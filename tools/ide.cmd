@echo off
rem Abre el IDE (ide\FenXEdit2) con simr: compila en memoria y ejecuta, sin dejar ficheros.
rem   tools\ide.cmd            usa build\zig-probe\simurgh\simr.exe (de tools\zig-probe-simurgh.sh)
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
cd /d "%ROOT%\ide\FenXEdit2"
"%EXE%" -c FenXEdit2.prg
if errorlevel 1 (
    echo.
    echo simr ha terminado con codigo %errorlevel%
    pause
)
endlocal
