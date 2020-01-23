@echo Generate C++/WinRT header files
cppwinrt.exe @cppwinrt_plat.rsp

@echo Create output folder
if not exist build mkdir build

@echo Compile with Clang 9.0.0
clang -v
clang main.cpp -D_CONSOLE -DWIN32_LEAN_AND_MEAN -DWINRT_LEAN_AND_MEAN -DUNICODE -I "gen" -L "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64" -Wl,kernel32.lib,user32.lib,windowsapp.lib -Xclang -std=c++2a -Xclang -Wno-delete-non-virtual-dtor -o build\app.exe
