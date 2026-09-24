@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul
cd /d "%~dp0"

echo ========================================================
echo  FastDirectX 11 — Raster Pattern Demo Launcher
echo ========================================================

:: 1. Build native DLL
echo [1/3] Compiling native FastDirectX.dll...
call compile.bat
if %errorlevel% neq 0 (
    echo [ERROR] Native build failed.
    exit /b 1
)

:: 2. Build Java code
echo.
echo [2/3] Building Java project...
call mvn -q -DskipTests test-compile
if %errorlevel% neq 0 (
    echo [ERROR] Maven build failed.
    exit /b 1
)

:: 3. Run Demo
echo.
echo [3/3] Launching FastDirectX Demo...
call mvn -q dependency:build-classpath -Dmdep.outputFile=target\cp.txt
set /p DEP_CP=<target\cp.txt
set FULL_CP=target\classes;target\test-classes;!DEP_CP!

java --enable-preview --enable-native-access=ALL-UNNAMED -cp "!FULL_CP!" fastdirectx.demo.DirectXDemoMain
if %errorlevel% neq 0 (
    echo [ERROR] Demo execution failed.
    exit /b 1
)

echo Demo finished.
pause
