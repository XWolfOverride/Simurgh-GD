@echo off
rem Lanza SPEED, el juego de DIV 2 de ejemplo reunido en build\zig-probe\div2\speed (speed.prg y la carpeta speed\ con sus FPG, FNT y PCM).
rem Se compila en memoria y se ejecuta; el codigo esta en CP850 (de ahi -c).
rem   tools\speed.cmd            usa build\zig-probe\simurgh\simr.exe (de tools\zig-probe-simurgh.sh)
rem   set SIMURGH_BIN=ruta\simr.exe  para usar otro ejecutable
setlocal
set "ROOT=%~dp0.."
set "EXE=%SIMURGH_BIN%"
if "%EXE%"=="" set "EXE=%ROOT%\build\zig-probe\simurgh\simr.exe"
if not exist "%EXE%" (
    echo No se encuentra %EXE%
    echo Construyelo antes con: sh tools/zig-probe-simurgh.sh
    pause
    exit /b 2
)
if not exist "%ROOT%\build\zig-probe\div2\speed\speed.prg" (
    echo No se encuentra build\zig-probe\div2\speed\speed.prg
    pause
    exit /b 2
)
cd /d "%ROOT%\build\zig-probe\div2\speed"
"%EXE%" -c speed.prg
if errorlevel 1 (
    echo.
    echo simr ha terminado con codigo %errorlevel%
    pause
)
endlocal
