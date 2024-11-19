# UnitXP Service Pack 3
WoW Vanilla 1.12.1 mod giving proper Classic-style nameplate hiding behavior, TAB key targeting functions, background notifications, and more.

I don't take ANY responsibility if this mod is originate in Burning Legion, or it would crash you game, or some Turtle ban your account. USE AT YOUR OWN RISK. 



### How to install

1. Download  `UnitXP-SP3-with-loader.zip`  from https://github.com/allfoxwy/UnitXP_SP3/releases/latest
2. Unzip everything into same place with `WoW.exe`
3. Move `UnitXP_SP3_Addon` into WoWclient/Interface/AddOns/
4. Run `Enable sideload-DLL.exe`
5. Start game with `WoW_sideload-DLL.exe`

Additional tips:
- You could delete `Enable sideload-DLL.exe` after installation, it's a huge file because of Node.js
- Currently Node.js needs Windows 8.1 to operate. `UnitXP_SP3.dll` itself works on elder Windows, you might try using VanillaFixes loader for it on elder Windows.



### How to use it in game

- With [the Lua Addon](https://github.com/allfoxwy/UnitXP_SP3_Addon) installed, you could acccess it from game's ESC menu.

![menu](https://github.com/user-attachments/assets/f31dd4b4-7e47-4e42-9fc4-cf02c5b59fc1)





### Targeting function (for TAB key)

This mod adds a few targeting functions to help you have a better TAB.

Currently when continuously trigger these functions, you may experience a small lag between switching target. I believe this is because game needs a server communication to obtain Target of Target information. I wish I could find a better way in future.

Functions could be accessed via Key Bindings menu, or via macro.

![keybindings](https://github.com/user-attachments/assets/61ddc189-7fd5-4a6b-bd5d-23ddd51d4895)




#### Nearest targeting

- `/script UnitXP("target", "nearestEnemy");`

Return TRUE when found a target.

Target nearest enemy. It is the one and the only one nearest enemy. No bullshit. And it follows rules:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- Only target enemy in front of player camera.
- Maximum range is 200 yards.
- In PvP, it ignores Pets and Totems.
- In PvE, when player is in-combat, it only target in-combat enemy.



#### Raid mark targeting

- `/script UnitXP("target", "nextMarkedEnemyInCycle");`
- `/script UnitXP("target", "previousMarkedEnemyInCycle");`

Return TRUE when found a target.

These functions only target mobs with a mark icon in order:
1. Skull
1. Red X cross
1. Blue square
1. Moon
1. Green triangle
1. Purple diamond
1. Orange circle
1. Yellow star

With following rules:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- Only target enemy in front of player camera.
- In PvP, it ignores Pets and Totems.
- Maximum range is 200 yards.
- In PvE, when player is in-combat, it only target in-combat enemy.
- When continuously triggered, it guarantees that every marked mob in range would be targeted for once.




#### Melee targeting

- `/script UnitXP("target", "nextEnemyConsideringDistance");`
- `/script UnitXP("target", "previousEnemyConsideringDistance");`

Return TRUE when found a target.

These functions are designed for ***melee***:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- Only target enemy in front of player camera.
- In PvP, it ignores Pets and Totems.
- In PvE, when player is in-combat, it only target in-combat enemy.
- Max range is 41 yards. Enemy further than that is ignored.
- Attack range is divided into 3 parts (0-8, 8-25, 25-41). If there is enemy in near range part, further range parts would be ignored.
- In 0 to 8 yards. It cycles all enemies.
- In 8 to 25 yards. Only the nearest 3 enemies would be cycled.
- In 25 to 41 yards. Only the nearest 5 enemies would be cycled.
- When no target, it selects the nearest.



#### Ranged targeting

- `/script UnitXP("target", "nextEnemyInCycle");`
- `/script UnitXP("target", "previousEnemyInCycle");`

Return TRUE when found a target.

These functions are designed for ***ranged***:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- Only target enemy in front of player camera.
- In PvP, it ignores Pets and Totems.
- In PvE, when player is in-combat, it only target in-combat enemy.
- Max range is 41 yards. Enemy further than that is ignored.
- When continuously triggered, it guarantees that every mob in range would be targeted for once.
- When no target, it selects the nearest.



#### World boss targeting

- `/script UnitXP("target", "worldBoss");`

Return TRUE when found a world boss.

World boss needs special attention:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- Only target enemy in front of player camera.
- Maximum range is 200 yards.
- In PvE, when player is in-combat, it only target in-combat enemy.
- When continuously triggered, it guarantees that every world boss in range would be targeted for once.



#### Range cone

"In front of player camera" is defined by a factor which could be adjusted with: 

- `/script UnitXP("target","rangeCone", 2.2);`

When this range cone factor in its minimum value 2, the cone is same as game's Field of View.

By default it's 2.2 . Increasing the factor would narrow the cone, so that only mobs in the center of vision would be targeted.




#### Using multiple targeting function together

For example: "We target raid mark first. However when no mark, we cycle in magic range":

- `/script local _=(UnitXP("target","nextMarkedEnemyInCycle") or UnitXP("target","nextEnemyInCycle"));`

This code works because targeting functions return TRUE or FALSE indicating if they got a target. Lua logic operators support short-cut evaluation, that is, they evaluate their second operand only when necessary. 




### Proper nameplates

Vanilla client only check distance for nameplates. This makes mobs behind wall/door also show up their nameplates.

This mod changes Vanilla behavior to Classic style:
- Only those mobs in player's sight would receive a nameplate
- If you move your camera really close, you could see through a wall for a short distance

Lua nameplate addon would work out-of-box, no need to change anything.

By default this feature is enabled. You could toggle its switch:

- `/script UnitXP("modernNameplateDistance", "enable");`

- `/script UnitXP("modernNameplateDistance", "disable");`




### Check if two units are in line of sight to each other

- `/script local result = UnitXP("inSight", UNIT_ID, UNIT_ID);print(result);`

UNIT_ID could be "player", "target"... also GUID string `"0x12345"`

Return TRUE for in sight, FALSE for NOT in sight, NIL for error.

This function has limitation:
- This is an expensive function. Basically it is raytracing on CPU. So it is recommended to check distance (or apply other filters such as friend/foe) before calling this function.
- It works locally, no server communication, not precisely accurate.
- I don't know how to read object's height, so this function treat everything as Human height.




### Measure distance between two units

- `/script local result = UnitXP("distanceBetween", UNIT_ID, UNIT_ID);print(result);`

UNIT_ID could be "player", "target"... also GUID string `"0x12345"`

Return a number, or NIL for error.




### Flash operating system's taskbar icon when game is in background

- `/script UnitXP("notify", "taskbarIcon");`

Flash would stop when game back to foreground.

To make this function link with certain game events like whisper/trade/invitation etc, we need [the Lua Addon](https://github.com/allfoxwy/UnitXP_SP3_Addon).




### Play operating system's event sound when game is in background

- `/script UnitXP("notify", "systemSound", "SystemDefault");`

Only work when game is in background.

To make this function link with certain game events like whisper/trade/invitation etc, we need [the Lua Addon](https://github.com/allfoxwy/UnitXP_SP3_Addon).


### Timer

- `/script local timerID = UnitXP("timer", "arm", 1000, 3000, "callbackFunctionNameString");`
- `/script UnitXP("timer", "disarm", timerID);`

Vanilla way doing periodic work is to use GetTime() in an OnUpdate() function and check if the time is come. This is basically doing [busy waiting](https://en.wikipedia.org/wiki/Busy_waiting). And because of the game is single-threaded, these timer pulling call would cost FPS. Mostly these function call are useless. For example on a 60 Hz display, we need triggering an event every second, then there would be 59 useless function call before every 1 useful call. Blizz later added C_Timer facility in patch 6.0.2 to solve this problem.

This mod adding a new timer facility to the game. These timers are running in a serperated thread so that their pulling call would not block game thread. When a timer triggers, it would call the corresponding Lua callback in game thread. The callback is passed with a single parameter which is `timer ID`. It is safe to `arm` or to `disarm` timers in callbacks.

The `arm` method in above example has 2 numberic parameter: The first `1000` means the timer would goes off when 1000ms after the `arm` method. The second `3000` means the timer would repeatly run every 3000ms after first trigger. If we pass a `0` to second numberic parameter, the timer would only goes off for once then disarm itself.

Beware that the timer is running in a seperated thread so game's `/reload` would NOT disarm a repeating timer. AddOns need to take care of their own repeating timer in `PLAYER_LOGOUT` or `PLAYER_LEAVING_WORLD` event and call `disarm` method to shut down cleanly.

Timer accuracy is decided by FPS: There would be at most 1 callback for each timer per frame. It is possible that there could be no timer callback during a frame.



### OnUpdate() and Timer

As Timer requires Lua AddOns forming a different structure to make full use of it. This usually is not a trivial work.

However, even simply link AddOn's OnUpdate() function with a Timer instead of UIFrame:OnUpdate should provide benefits:
- AddOn might not need a FPS speed repeating OnUpdate(). We could use a slower Timer for it.
- UnitXP_SP3 would line triggered Timers in a FIFO queue. For each callback in queue, UnitXP_SP3 would check time before execution. If callbacks already used up 1/80 second during a single rendering frame, those remaining callbacks in queue would be delayed to next frame. This behavior should smooth some graphical stutter, as repeating Lua code now have a loose time threshold to follow.




### Tell if UnitXP_SP3 functions available

Currently you could check `if UnitXP("inSight", "player", "player")` is true or not to know if client has UnitXP_SP3 installed.



### Some notes for compiling the code

- I used MS VS 2022. Note that GCC has a different calling convention than MS compiler, Blizz used MS compiler so we couldn't use GCC (nor MinGW).
- I staticly link [MinHook](https://github.com/TsudaKageyu/minhook). So UnitXP_SP3 also have "C/C++ > Code Generation > Runtime Library > Multi-threaded(/MT)"
- It's an elder game so we don't use advanced instructions from modern CPU: "C/C++ > Code Generation > Runtime Library > Enable Enhanced Instruction Set > No Enhanced Instructions (/arch:IA32)"
- x32 has a limited memory space. I think it would be better our program use less memory: "C/C++ > Optimization > Optimization /O1 and Favor small code /Os"
- I used C++ 17 features so "C/C++ > Language > C++ Language Standard > ISO C++ 17 Standard(/std:c++17)"
- Linked with [libMinHook.x86.lib](https://github.com/TsudaKageyu/minhook) and Winmm.lib


### MIT License


Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


