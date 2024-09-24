# UnitXP Service Pack 3
WoW Vanilla 1.12.1 mod adding additional functions to UnitXP().

It is intended to be loaded via [vanilla-dll-sideloader](https://github.com/allfoxwy/vanilla-dll-sideloader) and also serve as an example about how to write a memory mod.

I don't take ANY responsibility if this mod is originate in Burning Legion, or it would crash you game, or some Turtle ban your account. USE AT YOUR OWN RISK. 


### How to use them in game

- You could create a new macro in game.

- Write function in macro, example: `/script local time = UnitXP("getTickCount", "anything would do");print(time);`

- Pull the macro icon onto Action bar

- Click that Action bar button



### Targeting function (for TAB key)

This mod adds a few targeting functions to help you have a better TAB.

Currently when continuously trigger these functions, you may experience a small lag between switching target. I believe this is because game needs a server communication to obtain Target of Target information. I wish I could find a better way in future.


#### Nearest targeting

- `/script UnitXP("target", "nearestEnemy");`

Return TRUE when found a target.

Target nearest enemy. It is the one and the only one nearest enemy. No bullshit. And it follows rules:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- In PvP, it ignores Pets and Totems.
- When player is in-combat, it only target in-combat enemy.
- No range limit, as long as we could see the enemy in eyes.


#### Melee targeting

- `/script UnitXP("target", "nextEnemyConsideringDistance");`
- `/script UnitXP("target", "previousEnemyConsideringDistance");`

Return TRUE when found a target.

These functions are designed for ***melee***:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- In PvP, it ignores Pets and Totems.
- When player is in-combat, it only target in-combat enemy.
- Max range is 41 yards. Enemy further than that is ignored.
- Attack range is divided into 3 parts (0-5, 5-25, 25-41). If there is enemy in near range part, further range parts would be ignored.
- In 0 to 5 yards. It cycles all enemies.
- In 5 to 25 yards. Only the nearest 3 enemies would be cycled.
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
- In PvP, it ignores Pets and Totems.
- When player is in-combat, it only target in-combat enemy.
- Max range is 41 yards. Enemy further than that is ignored.
- When continuously triggered, it guarantees that every mob in range would be targeted for once.
- When no target, it selects the nearest.


#### World boss targeting

- `/script UnitXP("target", "worldBoss");`

Return TRUE when found a world boss.

World boss needs special attention:
- Only target attackable enemy.
- Only target livings.
- ***It ignores line of sight.*** For example NAXX 4HM fight has a stone platform in the center of battlefield, it would block line of sight.
- When player is in-combat, it only target in-combat enemy.
- No range limit, as long as we could see the enemy in eyes.
- When continuously triggered, it guarantees that every world boss in range would be targeted for once.
- When no target, it selects the nearest.




### Hide nameplate behind wall (for V key)

Vanilla client only check distance for nameplates. This makes mobs behind wall/door also show up their nameplates.

This mod changes Vanilla behaviour to Classic style:
- Only those mobs in player's sight would receive a nameplate
- If you move your camera really close, you could see through a wall for a short distance

LUA nameplate addon would work out-of-box, no need to change anything.

By default this feature is enabled. You could toggle its switch:

- `/script UnitXP("modernNameplateDistance", "enable");`

- `/script UnitXP("modernNameplateDistance", "disable");`




### Check if two units are in line of sight to each other

- `/script local result = UnitXP("inSight", UNIT_ID, UNIT_ID);print(result);`

UNIT_ID could be "player", "target"... also GUID string `"0x12345"`

Return TRUE for in sight, FALSE for NOT in sight, NIL for error.

This function has limitation:
- It works locally, no server communication, not precisely accurate.
- I don't know how to read object's height, so this function treat everything as Human height.




### Measure distance between two units

- `/script local result = UnitXP("distanceBetween", UNIT_ID, UNIT_ID);print(result);`

UNIT_ID could be "player", "target"... also GUID string `"0x12345"`

Return a number, or NIL for error.




### Flash WoW window and its taskbar icon to notify player

- `/script UnitXP("flashNotifyOS", 10);`

Above command would attmpt flashing 10 times, and stop as soon as WoW window is back to foreground.





### Tell if UnitXP_SP3 functions available

These only work with included [vanilla-dll-sideloader](https://github.com/allfoxwy/vanilla-dll-sideloader), VanillaFixes loader skip them.

When mod loads, it adds some globals to LUA:
- Vanilla1121mod.UnitXP_SP3
- Vanilla1121mod.UnitXP_SP3_inSight
- Vanilla1121mod.UnitXP_SP3_distanceBetween
- Vanilla1121mod.UnitXP_SP3_modernNameplateDistance
- Vanilla1121mod.UnitXP_SP3_target
- Vanilla1121mod.UnitXP_SP3_flashNotifyOS

You could check their existance to tell if certain function is available.



### Some notes for compiling the code

- I used MS VS 2022. There are elder post says GCC won't work.
- I staticly link [MinHook](https://github.com/TsudaKageyu/minhook). So UnitXP_SP3 also have "C/C++ > Code Generation > Runtime Library > Multi-threaded(/MT)"
- It's an elder game so we don't use advanced instructions from modern CPU: "C/C++ > Code Generation > Runtime Library > Enable Enhanced Instruction Set > No Enhanced Instructions (/arch:IA32)"
- x32 has a limited memory space. I think it would be better our program use less memory: "C/C++ > Optimization > Optimization /O1 and Favor small code /Os"
- I used C++ 17 features so "C/C++ > Language > C++ Language Standard > ISO C++ 17 Standard(/std:c++17)"



### MIT License


Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


