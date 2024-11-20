#ifndef CPPTIME_H_
#define CPPTIME_H_

/** This file is modified from https://github.com/eglimi/cpptime
 * 
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Michael Egli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \author    Michael Egli
 * \copyright Michael Egli
 * \date      11-Jul-2015
 *
 * \file cpptime.h
 *
 * C++11 timer component
 * =====================
 *
 * A portable, header-only C++11 timer component.
 *
 * Overview
 * --------
 *
 * This component can be used to manage a set of timeouts. It is implemented in
 * pure C++11. It is therefore very portable given a compliant compiler.
 *
 * A timeout can be added with one of the `add()` functions, and removed with
 * the `remove()` function. A timeout can be set to be either one-shot or
 * periodic. If it is one-shot, the callback is invoked once and the timeout
 * event is then automatically removed. If the timeout is periodic, it is
 * always renewed and never automatically removed.
 *
 * When a timeout is removed or when a one-shot timeout expires, the handler
 * will be deleted to clean-up any resources.
 *
 * Removing a timeout is possible from within the callback. In this case, you
 * must be careful not to access any captured variables, if any, after calling
 * `remove()`, because they are no longer valid.
 *
 * Timeout Units
 * -------------
 *
 * The preferred functions for adding timeouts are those that take a
 * `std::chrono::...` argument. However, for convenience, there is also an API
 * that takes a uint64_t. When using this API, all values are expected to be
 * given in microseconds (us).
 *
 * For periodic timeouts, a separate timeout can be specified for the initial
 * (first) timeout, and the periodicity after that.
 *
 * To avoid drifts, times are added by simply adding the period to the initially
 * calculated (or provided) time. Also, we use `wait until` type of API to wait
 * for a timeout instead of a `wait for` API.
 *
 * Data Structure
 * --------------
 *
 * Internally, a std::vector is used to store timeout events. The timer_id
 * returned from the `add` functions are used as index to this vector.
 *
 * In addition, a std::multiset is used that holds all time points when
 * timeouts expire.
 *
 * The timer_id is uint32_t and have no rollover check. They never be reused.
 * This should be safe as Lua by default use double-precision float which could
 * hold up to 52bits in its fraction part.
 *
 * Examples
 * --------
 *
 * More examples can be found in the `tests` folder.
 *
 * ~~~
 * CppTime::Timer t;
 * t.add(std::chrono::seconds(1), [](CppTime::timer_id){ std::cout << "got it!"; });
 * std::this_thread::sleep_for(std::chrono::seconds(2));
 * ~~~
 */

// Includes
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <set>
#include <stack>
#include <thread>
#include <unordered_map>

namespace CppTime
{

// Public types
using timer_id = uint32_t;
using handler_t = std::function<void(timer_id)>;
using clock = std::chrono::high_resolution_clock;
using timestamp = std::chrono::time_point<clock>;
using duration = std::chrono::microseconds;

// Private definitions. Do not rely on this namespace.
namespace detail
{

// The event structure that holds the information about a timer.
struct Event {
	timer_id id;
	timestamp start;
	duration period;
	handler_t handler;
	bool valid;
};

// A time event structure that holds the next timeout and a reference to its
// Event struct.
struct Time_event {
	timestamp next;
	timer_id ref;
};

inline bool operator<(const Time_event &l, const Time_event &r)
{
	return l.next < r.next;
}

} // end namespace detail

class Timer
{
	using scoped_m = std::unique_lock<std::mutex>;

	// Thread and locking variables.
	std::mutex m;
	std::condition_variable cond;
	std::thread worker;

	// Use to terminate the timer thread.
	bool done = false;

	// Holds all active events.
	std::unordered_map<timer_id, detail::Event> events;
	// Sorted queue that has the next timeout at its top.
	std::multiset<detail::Time_event> time_events;

	// Next usable timer id
	uint32_t next_id = 0;

public:
	Timer() : m{}, cond{}, worker{}, events{}, time_events{}
	{
		scoped_m lock(m);
		done = false;
		next_id = 0;
		worker = std::thread([this] { run(); });
	}

	~Timer()
	{
		scoped_m lock(m);
		done = true;
		lock.unlock();
		cond.notify_all();
		worker.join();
		events.clear();
		time_events.clear();
	}

	/**
	 * Add a new timer.
	 *
	 * \param when The time at which the handler is invoked.
	 * \param handler The callable that is invoked when the timer fires.
	 * \param period The periodicity at which the timer fires. Only used for periodic timers.
	 */
	timer_id add(
		const timestamp& when, handler_t&& handler, const duration& period = duration::zero())
	{
		scoped_m lock(m);
		timer_id id = next_id;
		next_id++; // I think we won't use up uint32_t
		detail::Event e{ id, when, period, std::move(handler), true };
		events.insert({ id,e });
		time_events.insert(detail::Time_event{ when, id });
		lock.unlock();
		cond.notify_all();
		return id;
	}

	/**
	 * Overloaded `add` function that uses a `std::chrono::duration` instead of a
	 * `time_point` for the first timeout.
	 */
	template <class Rep, class Period>
	inline timer_id add(const std::chrono::duration<Rep, Period> &when, handler_t &&handler,
	    const duration &period = duration::zero())
	{
		return add(clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(when),
		    std::move(handler), period);
	}

	/**
	 * Overloaded `add` function that uses a uint64_t instead of a `time_point` for
	 * the first timeout and the period.
	 */
	inline timer_id add(const uint64_t when, handler_t &&handler, const uint64_t period = 0)
	{
		return add(duration(when), std::move(handler), duration(period));
	}

	/**
	 * Removes the timer with the given id.
	 */
	bool remove(timer_id id)
	{
		scoped_m lock(m);
		
		if (events.erase(id) == 0) {
			return false;
		}

		auto it = std::find_if(time_events.begin(), time_events.end(),
		    [&](const detail::Time_event &te) { return te.ref == id; });
		if(it != time_events.end()) {
			time_events.erase(it);
		}

		lock.unlock();
		cond.notify_all();
		return true;
	}

private:
	void run()
	{
		scoped_m lock(m);

		while(!done) {

			if(time_events.empty()) {
				// Wait for work
				cond.wait(lock);
			} else {
				detail::Time_event te = *time_events.begin();
				if(CppTime::clock::now() >= te.next) {

					// Remove time event
					time_events.erase(time_events.begin());

					// Patch from https://github.com/eglimi/cpptime/pull/9
					// VS would metion this line doing a copy and suggest using a reference. But we should keep the copy as that is the intention of the patch.
					auto tempHandler = events.at(te.ref).handler;

					// Invoke the handler
					lock.unlock();
					tempHandler(te.ref);
					lock.lock();

					if(events.find(te.ref) != events.end() && events.at(te.ref).period.count() > 0) {
						// The event is valid and a periodic timer.
						te.next += events.at(te.ref).period;
						time_events.insert(te);
					} else {
						// The event is either no longer valid because it was removed in the
						// callback, or it is a one-shot timer.
						events.erase(te.ref);
					}
				} else {
					cond.wait_until(lock, te.next);
				}
			}
		}
	}
};

} // end namespace CppTime

#endif // CPPTIME_H_
