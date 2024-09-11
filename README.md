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