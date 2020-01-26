@echo Generate C++/WinRT header files
cppwinrt.exe @cppwinrt_plat.rsp

@echo Create output folder
if not exist build mkdir build

@echo Compile with Clang 9.0.0
clang -v

@echo Create binary
clang -I "Generated Files" -D_WINDOWS -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DWINRT_LEAN_AND_MEAN -DMDd main.cpp -Xclang -std=c++2a -Xclang -Wno-delete-non-virtual-dtor -o build\cppwinrt-clang.exe -L "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64" -Wl,kernel32.lib,user32.lib,windowsapp.lib -DMANIFEST -DMANIFESTUAC:"level='asInvoker' uiAccess='false'"

@echo Embed manifest
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64\mt.exe" -manifest cppwinrt.manifest -outputresource:build\cppwinrt-clang.exe;1