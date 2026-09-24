@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ========================================================
echo  Compiling FastDirectX Native DLL (D3D11)
echo ========================================================

:: 1. Detect JAVA_HOME
if "%JAVA_HOME%"=="" (
    for %%d in (
        "C:\Program Files\Java\jdk-21.0.12.1"
        "C:\Program Files\Java\jdk-21"
        "C:\Program Files\Java\jdk-17"
        "C:\Program Files\Java\jdk-25"
    ) do (
        if exist %%d (
            set "JAVA_HOME=%%~d"
            goto :found_java
        )
    )
)
:found_java
if "%JAVA_HOME%"=="" (
    echo [ERROR] JAVA_HOME not found.
    exit /b 1
)
echo Using JAVA_HOME: %JAVA_HOME%

:: 2. Detect Visual Studio vcvars64.bat
where cl.exe >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    set "VS_DIR="
    for %%v in (
        "C:\Program Files\Microsoft Visual Studio\18\Community"
        "C:\Program Files\Microsoft Visual Studio\2026\Community"
        "C:\Program Files\Microsoft Visual Studio\2022\Community"
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
    ) do (
        if exist "%%~v\VC\Auxiliary\Build\vcvars64.bat" (
            set "VS_DIR=%%~v"
            goto :found_vs
        )
    )
    :found_vs
    if not defined VS_DIR (
        echo [ERROR] Visual Studio compiler not found.
        exit /b 1
    )
    echo Found Visual Studio at: !VS_DIR!
    call "!VS_DIR!\VC\Auxiliary\Build\vcvars64.bat" >nul
)

if not exist "target\classes\native" mkdir "target\classes\native"
if not exist "src\main\resources\native" mkdir "src\main\resources\native"

cl.exe /O2 /W3 /std:c++17 /MD /EHsc /LD ^
   /I "%JAVA_HOME%\include" ^
   /I "%JAVA_HOME%\include\win32" ^
   /Fe:target\classes\FastDirectX.dll ^
   native\DirectXBackend.cpp ^
   d3d11.lib dxgi.lib d3dcompiler.lib ^
   /link /DLL /MACHINE:X64

if %ERRORLEVEL% EQU 0 (
    copy /y target\classes\FastDirectX.dll . >nul 2>&1
    copy /y target\classes\FastDirectX.dll src\main\resources\FastDirectX.dll >nul 2>&1
    copy /y target\classes\FastDirectX.dll src\main\resources\native\FastDirectX.dll >nul 2>&1
    copy /y target\classes\FastDirectX.dll target\classes\native\FastDirectX.dll >nul 2>&1
    echo.
    echo ========================================================
    echo [SUCCESS] FastDirectX.dll compiled successfully
    echo ========================================================
) else (
    echo.
    echo [ERROR] Native compilation failed.
    exit /b 1
)
endlocal
