# UnitXP Service Pack 3
WoW Vanilla 1.12.1 mod adding additional functions to UnitXP().

It is intended to be loaded via [vanilla-dll-sideloader](https://github.com/allfoxwy/vanilla-dll-sideloader) and also serve as an example about how to write a memory mod.

I don't take ANY responsibility if this mod is originate in Burning Legion, or it would crash you game, or some Turtle ban your account. USE AT YOUR OWN RISK. 



### Check if two units are in line of sight to each other

`local result = UnitXP("inSight", UNIT_ID, UNIT_ID);`

UNIT_ID could be "player", "target"... also GUID string `"0x12345"`

Return TRUE for in sight, FALSE for NOT in sight, NIL for error.

This function has limitation:
- It works locally, no server communication, not precisely accurate.
- I don't know how to read object's height, so this function treat everything as Human height.



### Measure distance between two units

`local result = UnitXP("distanceBetween", UNIT_ID, UNIT_ID);`

UNIT_ID could be "player", "target"... also GUID string `"0x12345"`

Return a number, or NIL for error.


### GetTickCount64() WinAPI

`local time = UnitXP("getTickCount", "anything would do");`

This makes the mod require Windows Vista or newer to operate. I know there is GetTickCount() but I think I won't compromise for now.



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

`UnitXP("modernNameplateDistance", "enable");`

`UnitXP("modernNameplateDistance", "disable");`

`UnitXP("modernNameplateDistance", "enableEnemyRefresh");`

`UnitXP("modernNameplateDistance", "disableEnemyRefresh");`

`UnitXP("modernNameplateDistance", "enableFriendRefresh");`

`UnitXP("modernNameplateDistance", "disableFriendRefresh");`

`local query = UnitXP("modernNameplateDistance", "anything else");`

LUA nameplate addon would work out-of-box, no need to change anything.

I wish I could make a perfect "refresh to disappear" program with NO flash, however I lack 2 pieces of critical information:
- I can't find address of CGUnit_C::RemoveUnitNamePlate()
- There is a linked list of nameplates at 0xc4d92c, it could be iterated by NODE_ADDR + 0x4e0 . However I can't find unit GUID out of it.



### Tell if UnitXP_SP3 functions available
When mod loads, it adds some globals to LUA:
- Vanilla1121mod.UnitXP_SP3
- Vanilla1121mod.UnitXP_SP3_inSight
- Vanilla1121mod.UnitXP_SP3_distanceBetween
- Vanilla1121mod.UnitXP_SP3_modernNameplateDistance

You could check their existance to tell if certain function is available.



### Some notes for compiling the code
- I used MS VS 2022. There are elder post says GCC won't work.
- I staticly link [MinHook](https://github.com/TsudaKageyu/minhook). So UnitXP_SP3 also have "C/C++ > Code Generation > Runtime Library > Multi-threaded(/MT)"
- It's an elder game so we don't use advanced instructions from modern CPU: "C/C++ > Code Generation > Runtime Library > Enable Enhanced Instruction Set > No Enhanced Instructions (/arch:IA32)"
- x32 has a limited memory space. I think it would be better our program use less memory: "C/C++ > Optimization > Optimization /O1 and Favor small code /Os"
- I used C++ 17 features so "C/C++ > Language > C++ Language Standard > ISO C++ 17 Standard(/std:c++17)"



### zlib/libpng license

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

- The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software
  in a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

- Altered source versions must be plainly marked as such, and must not
  be misrepresented as being the original software.

- This notice may not be removed or altered from any source distribution.


