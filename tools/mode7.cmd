@echo off
rem Prueba interactiva del modo 7: tests\simurgh\t_mode7.prg con el argumento -i (compila en memoria y ejecuta).
rem   tools\mode7.cmd b  ->  t_mode7b.prg: dos modos 7 uno sobre otro (pista sobre suelo, como F-Zero)
rem   W/S o flechas arriba/abajo: avanzar y retroceder      A/D: desplazarse a los lados
rem   flechas izquierda/derecha: girar                     R/F: altura   T/G: zoom   Y/H: horizonte   Esc: salir
rem Se ejecuta desde tests\simurgh, asi que los ficheros que se anadan alli (mapas, fpg...) se cargan por su nombre.
rem   tools\mode7.cmd            usa build\zig-probe\simurgh\simr.exe (de tools\zig-probe-simurgh.sh)
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
cd /d "%ROOT%\tests\simurgh"
set "PRG=t_mode7.prg"
if /i "%~1"=="b" set "PRG=t_mode7b.prg"
"%EXE%" %PRG% -i
if errorlevel 1 (
    echo.
    echo simr ha terminado con codigo %errorlevel%
    pause
)
endlocal
