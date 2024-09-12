# UnitXP Service Pack 3
WoW Vanilla 1.12.1 mod adding additional functions to UnitXP()

It is intended to be loaded via [vanilla-dll-sideloader](https://github.com/allfoxwy/vanilla-dll-sideloader) and also serve as an example about how to write a memory mod.

USE AT YOUR OWN RISK.



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



### Tell if UnitXP_SP3 functions available
When mod loads, it adds some globals to LUA:
- Vanilla1121mod.UnitXP_SP3
- Vanilla1121mod.UnitXP_SP3_inSight
- Vanilla1121mod.UnitXP_SP3_distanceBetween

You could check their existance to tell if certain function is available.



### Some notes for compiling the code
- I used MS VS 2022. There are elder post says GCC won't work.
- I staticly link [MinHook](https://github.com/TsudaKageyu/minhook). So UnitXP_SP3 also have "C/C++ > Code Generation > Runtime Library > Multi-threaded(/MT)"
- It's an elder game so we don't use advanced instructions from modern CPU: "C/C++ > Code Generation > Runtime Library > Enable Enhanced Instruction Set > No Enhanced Instructions (/arch:IA32)"
- x32 has a limited memory space. I think it would be better our program use less memory: "C/C++ > Optimization > Optimization /O1 and Favor small code /Os"
- I used C++ 17 features so "C/C++ > Language > C++ Language Standard > ISO C++ 17 Standard(/std:c++17)"

