@echo off
setlocal
chcp 65001 >nul

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo [ОШИБКА] Visual Studio не найдена!
    pause
    exit /b 1
)

call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"

echo [Сборка] Компиляция MyCS2Module.dll...
cl /LD /std:c++17 /EHsc /O2 /I include /I src src\dllmain.cpp /Fe:MyCS2Module.dll user32.lib gdi32.lib

if exist dllmain.obj del dllmain.obj
if exist MyCS2Module.exp del MyCS2Module.exp
if exist MyCS2Module.lib del MyCS2Module.lib

if exist MyCS2Module.dll (
    echo [УСПЕХ] Файл MyCS2Module.dll успешно собран!
) else (
    echo [ОШИБКА] Ошибка при сборке.
)
