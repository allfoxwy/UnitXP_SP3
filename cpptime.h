#ifndef CPPTIME_H_
#define CPPTIME_H_

/** This file is modified from https://github.com/eglimi/cpptime
 *
 * The MIT License (MIT)
 *
 * Origin from Copyright (c) 2015 Michael Egli
 * Modified for UnitXP_SP3 by allfox and the thankful community
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
 */

 // Includes
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <set>
#include <unordered_set>
#include <deque>
#include <stack>
#include <thread>
#include <unordered_map>
#include <string>
#include <atomic>
#include <sstream>

#include "Vanilla1121_functions.h"
#include "utf8_to_utf16.h"

namespace CppTime
{

    // Public types
    using timer_id = uint32_t;
    using handler_t = std::string; // Lua function name in string
    using clock = std::chrono::high_resolution_clock;
    using timestamp = std::chrono::time_point<clock>;
    using duration = std::chrono::microseconds;
    const auto timeslice = duration(1000 * 1000 / 80); // Expected execution timeslice (it's a loose expectation)

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

        inline bool operator<(const Time_event& l, const Time_event& r)
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
        std::timed_mutex trigger;

        // Use to terminate the timer thread.
        std::atomic_bool done = false;

        // Holds all active events.
        std::unordered_map<timer_id, detail::Event> events;
        // Sorted queue that has the next timeout at its top.
        std::multiset<detail::Time_event> time_events;
        // FIFO queue for execution
        std::deque< std::pair<CppTime::timer_id, std::string> > execution_fifo;
        // A hash list for quick check if a timer is already in queue
        std::unordered_set<CppTime::timer_id> already_in_fifo;

        // Next usable timer id
        std::atomic_uint32_t next_id = 1;

        // According to https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices
        // Creating thread in global object's constructor is risky.
        // So here is a flag to delay it to a later stage.
        std::atomic_bool threadIsRunning = false;

    public:
        Timer() : m{}, cond{}, worker{}, events{}, time_events{}, execution_fifo{}, already_in_fifo{}
        {
            done = false;
            threadIsRunning = false;
            next_id = 1;
        }

        /*
        * End of Timer thread
        * We need this function because we can't join thread in destructor, because of DLL loader lock
        * We would call it in gameQuit detoured function
        */
        void end() {
            done = true;

            if (threadIsRunning) {
                cond.notify_all();
                if (worker.joinable()) {
                    worker.join();

                    // After join, threadIsRunning should be false already
                }
                else {
                    MessageBoxW(NULL, utf8_to_utf16(u8"Somehow, Timer thread is not joinable.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                }


                // We are not cleaning queues
                // Because I don't know how WoW quit its main thread, is it gracefully quit or a kill?
                // Anyway, these queues won't be executed anymore as game is no longer rendering, so leaving them for OS cleaning should be fine.
            }
        }

        /*
         * Add a new timer.
         *
         * \param when The time at which the handler is invoked.
         * \param handler The callable that is invoked when the timer fires.
         * \param period The periodicity at which the timer fires. Only used for periodic timers.
         */
        timer_id add(const timestamp when, const handler_t handler, const duration period = duration::zero())
        {
            if (done) {
                return 0;
            }

            if (handler.empty()) {
                return 0;
            }

            scoped_m lock(m);

            if (threadIsRunning == false) {
                worker = std::thread([this] { run(); });

                // It may look like this line is redundant.
                // However the newly created thread need to wait OS for scheduling, so tuning flag in it would be delayed.
                threadIsRunning = true;
            }

            timer_id id = next_id++; // I think we won't use up uint32_t
            detail::Event e{ id, when, period, handler, true };
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
        inline timer_id add(const std::chrono::duration<Rep, Period> when, handler_t handler, const duration period = duration::zero())
        {
            return add(clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(when), handler, period);
        }

        /**
         * Overloaded `add` function that uses a uint64_t instead of a `time_point` for
         * the first timeout and the period.
         */
        inline timer_id add(const uint64_t when, handler_t handler, const uint64_t period = 0)
        {
            // We expecting time in milliseconds
            return add(duration(when * 1000), handler, duration(period * 1000));
        }

        /**
         * Removes the timer with the given id.
         */
        bool remove(timer_id id)
        {
            if (done) {
                return false;
            }

            scoped_m lock(m);

            if (events.erase(id) == 0) {
                return false;
            }

            auto it = std::find_if(time_events.begin(), time_events.end(),
                [&](const detail::Time_event& te) { return te.ref == id; });
            if (it != time_events.end()) {
                time_events.erase(it);
            }

            lock.unlock();
            cond.notify_all();
            return true;
        }

        /*
        * Tell how many timers are running.
        */
        size_t size()
        {
            if (done) {
                return 0;
            }

            scoped_m lock(m);

            return events.size();
        }

        /*
        * Execute the callback queue.
        */
        void execute()
        {
            if (done || threadIsRunning == false)
            {
                return;
            }

            auto start = clock::now();
            if (trigger.try_lock_for(timeslice))
            {
                if (done || threadIsRunning == false)
                {
                    trigger.unlock();
                    return;
                }

                void* L = GetContext();
                while (!done && threadIsRunning && clock::now() - start <= timeslice && execution_fifo.size() > 0)
                {
                    // We do a copy here, because execution_fifo would pop_front() soon. Reference is error-prone.
                    auto i = execution_fifo.front();

                    std::stringstream ss{};
                    ss << i.second << "(" << i.first << ");";
                    vanilla1121_runScript(ss.str());

                    already_in_fifo.erase(i.first);
                    execution_fifo.pop_front();
                }
                trigger.unlock();
            }
        }

    private:
        void run()
        {
            threadIsRunning = true;

            scoped_m lock(m);

            while (!done) {

                if (time_events.empty()) {
                    // Wait for work
                    cond.wait(lock);
                }
                else {
                    detail::Time_event te = *time_events.begin();
                    if (CppTime::clock::now() >= te.next) {

                        // Remove time event
                        time_events.erase(time_events.begin());

                        // Copy the handler before releasing the lock
                        handler_t handler{ events.at(te.ref).handler };

                        lock.unlock();
                        {
                            std::lock_guard lg{ trigger };

                            // Add to execution queue
                            // We only accept 1 callback per timer, discard the rest.
                            if (already_in_fifo.insert(te.ref).second == true) {
                                execution_fifo.push_back({ te.ref, handler });
                            }
                        }
                        lock.lock();

                        /*It may look like the following check if te.ref still in events is unnecessary,
                        * HOWEVER remember that we are working in threads, betweeen trigger releasing and lock.lock(),
                        * it is possible that the operating system schedule an execute() and Lua callback remove the timer.
                        */
                        if (events.find(te.ref) != events.end() && events.at(te.ref).period.count() > 0) {
                            // The event is valid and a periodic timer.
                            te.next += events.at(te.ref).period;
                            time_events.insert(te);
                        }
                        else {
                            // The event is either no longer valid because it was removed in the
                            // callback, or it is a one-shot timer.
                            events.erase(te.ref);
                        }
                    }
                    else {
                        cond.wait_until(lock, te.next);
                    }
                }
            }

            threadIsRunning = false;
        }
    };

} // end namespace CppTime

#endif // CPPTIME_H_
