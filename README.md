# cppwinrt-clang
A sample C++/WinRT app using the clang compiler

## Requirements
- Install LLVM >= 9.0.0 and it to your path
- Install Windows 10 1903 SDK to  C:\Program Files (x86)\Windows Kits\10\References\10.0.18362.0

## A.) Build with Visual Studio, using clang
- Open cppwinrt-clang.sln

## B.) Build from command line, using clang
**ISSUE**: It does not leverage cppwinrt.manifest and the app crashes at runtime

- Running *build.bat* will:

1.) Generate C++/WinRT header files
```
cppwinrt.exe @cppwinrt_plat.rsp
```

2.) Create output folder
```
if not exist build mkdir build
```

3.) Compile with Clang
```
clang main.cpp -D_CONSOLE -DWIN32_LEAN_AND_MEAN -DWINRT_LEAN_AND_MEAN -DUNICODE -I "gen" -L "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64" -Wl,kernel32.lib,user32.lib,windowsapp.lib -Xclang -std=c++2a -Xclang -Wno-delete-non-virtual-dtor -o build\app.exe
```
