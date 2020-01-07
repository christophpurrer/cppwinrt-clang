# cppwinrt-clang
A sample C++/WinRT app using the clang compiler

## Requirements
- Install LLVM >= 8.0.1 and it to your path

## Generate C++/WinRT header files
```
cppwinrt.exe @cppwinrt_plat.rsp
```

## Create output folder
```
if not exist build mkdir build
```

## Compile with Clang
```
clang main.cpp -D_CONSOLE -DWIN32_LEAN_AND_MEAN -DWINRT_LEAN_AND_MEAN -DUNICODE -I "gen" -L C:\tools\toolchains\vs2017_15.5\WindowsSdk\10.0.16299.91\Lib\10.0.16299.0\um\x64 -Wl,kernel32.lib,user32.lib,windowsapp.lib -Xclang -std=c++2a -Xclang -Wno-delete-non-virtual-dtor -o build\app.exe
```