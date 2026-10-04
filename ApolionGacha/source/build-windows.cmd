@echo off
rem Run in a Visual Studio x64 Native Tools Command Prompt.
cd /d "%~dp0"
rc /nologo /fo resources.res resources.rc
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /utf-8 /EHsc /O2 /MT /DUNICODE /D_UNICODE main.cpp resources.res /Fe:..\ApolionGacha.exe /link /SUBSYSTEM:WINDOWS /MANIFEST:NO gdiplus.lib comctl32.lib ole32.lib uuid.lib shell32.lib comdlg32.lib propsys.lib bcrypt.lib user32.lib gdi32.lib
if errorlevel 1 exit /b 1
del main.obj resources.res
