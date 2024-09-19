# UnitXP Service Pack 3
WoW Vanilla 1.12.1 mod adding additional functions to UnitXP().

It is intended to be loaded via [vanilla-dll-sideloader](https://github.com/allfoxwy/vanilla-dll-sideloader) and also serve as an example about how to write a memory mod.

I don't take ANY responsibility if this mod is originate in Burning Legion, or it would crash you game, or some Turtle ban your account. USE AT YOUR OWN RISK. 



### Targeting function

TAB key by default could select anything except your wish.


This mod adds 2 targeting function:
- `/script UnitXP("target", "nearestEnemy);`
- `/script UnitXP("target", "randomEnemy);`

Return TRUE when found a target.


Target nearest enemy is by its name, the one and the only one nearest enemy. No bullshit. And it follows rules:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- When player is in-combat, it only target in-combat enemy.
- No range limit, as long as we could see the enemy in eyes.


Target random enemy despite its name, is a well-defined targeting function I proposed for replacing TAB key:
- Only target attackable enemy.
- Only target livings.
- Only target enemy in line of sight.
- When player is in-combat, it only target in-combat enemy.
- Max range is 41 yards. Enemy further than that is ignored.
- Attack range is divided into 3 parts (0-5, 5-25, 25-41). If there is enemy in near range part, further range parts would be ignored.
- In 0 to 5 yards. All ememy would be targeted randomly.
- In 5 to 25 yards. Only the nearest 3 enemies would be targeted, randomly.
- In 25 to 41 yards. Only the nearest 5 enemies would be targeted, randomly.
- When there is an alternative choice, it won't target the same enemy as last time.



### Hide nameplate behind wall

Vanilla client only check distance for nameplates. This would make mobs behind wall/door also show up their nameplates.

This mod changes Vanilla behaviour:
- In 8 to MAX range, only those mobs in player's sight would receive a nameplate
- In 0 to 8 range, every mob receive a nameplate
- When a mob with nameplate move into shadow behind wall, its nameplate would disappear within 2 seconds. However this refresh feature could be disabled because it causes flash.

By default:
- Modern Nameplate Distance feature is Enabled

- Refresh enemy nameplate every 2 seconds.

- Friendly nameplate would NOT refresh.

 ...you could control it by

- `/script UnitXP("modernNameplateDistance", "enable");`

- `/script UnitXP("modernNameplateDistance", "disable");`

- `/script UnitXP("modernNameplateDistance", "enableEnemyRefresh");`

- `/script UnitXP("modernNameplateDistance", "disableEnemyRefresh");`

- `/script UnitXP("modernNameplateDistance", "enableFriendRefresh");`

- `/script UnitXP("modernNameplateDistance", "disableFriendRefresh");`

- `/script local query = UnitXP("modernNameplateDistance", "anything else");print(query);`

LUA nameplate addon would work out-of-box, no need to change anything.

I wish I could make a perfect "refresh to disappear" program with NO flash, however I lack 2 pieces of critical information:
- I can't find address of CGUnit_C::RemoveUnitNamePlate()
- There is a linked list of nameplates at 0xc4d92c, it could be iterated by NODE_ADDR + 0x4e0 . However I can't find unit GUID out of it.



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



### GetTickCount64() WinAPI

- `/script local time = UnitXP("getTickCount", "anything would do");print(time);`

This makes the mod require Windows Vista or newer to operate. I know there is GetTickCount() but I think I won't compromise for now.



### Tell if UnitXP_SP3 functions available

These only work with included [vanilla-dll-sideloader](https://github.com/allfoxwy/vanilla-dll-sideloader), VanillaFixes loader skip this.

When mod loads, it adds some globals to LUA:
- Vanilla1121mod.UnitXP_SP3
- Vanilla1121mod.UnitXP_SP3_inSight
- Vanilla1121mod.UnitXP_SP3_distanceBetween
- Vanilla1121mod.UnitXP_SP3_modernNameplateDistance
- Vanilla1121mod.UnitXP_SP3_target

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


