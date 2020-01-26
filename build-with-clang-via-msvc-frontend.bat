set WINDOWS_SDK=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0

@echo Generate C++/WinRT header files
cppwinrt.exe @cppwinrt_plat.rsp

@echo Create output folder
if not exist build mkdir build

@echo Compile with Clang >= 9.0.0
clang -v

@echo Create object
clang-cl.exe /c /I"Generated Files\\" /D _WINDOWS /D _UNICODE /D UNICODE /EHsc /std:c++latest /Fo"build\\" main.cpp

@echo Link objects
lld-link.exe /OUT:"build\cppwinrt-clang.exe" /INCREMENTAL:NO /LIBPATH:"%WINDOWS_SDK%\um\x64" /LIBPATH:"%WINDOWS_SDK%\ucrt\x64" /LIBPATH:"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.24.28314\lib\x64" kernel32.lib user32.lib windowsapp.lib build\main.obj

@echo Embed manifest
:: https://docs.microsoft.com/en-us/cpp/build/how-to-embed-a-manifest-inside-a-c-cpp-application?view=vs-2019
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64\mt.exe" -manifest cppwinrt.manifest -outputresource:build\cppwinrt-clang.exe;1