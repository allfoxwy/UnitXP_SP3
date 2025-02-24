# UnitXP Service Pack 3

An attempt to make Vanilla 1.12 modern...
- Adjust camera
- Background notifications
- Check line of sight in Lua
- Debug interface for in-game Lua
- Measure distance in Lua
- Performance tweak of disabling TCP delayed ACK
- Proper nameplates
- Screenshot produce JPEG file
- Better Tab targeting functions


Refer to [wiki](https://github.com/allfoxwy/UnitXP_SP3/wiki) for detail.

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

- With [the Lua Addon](https://github.com/allfoxwy/UnitXP_SP3_Addon) installed, you could acccess it from minimap icon.

![menu](https://github.com/user-attachments/assets/f31dd4b4-7e47-4e42-9fc4-cf02c5b59fc1)




### MIT License


Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


