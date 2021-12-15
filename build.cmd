call "%VS100COMNTOOLS%\vsvars32.bat"

cl ForceReboot.cpp ^
    /D "_UNICODE" ^
    /D "UNICODE" ^
    /Zc:wchar_t ^
    /MT ^
    /link Advapi32.lib User32.lib
