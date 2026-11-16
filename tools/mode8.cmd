@echo off
rem Prueba interactiva del modo 8 (mapas de sectores): tests\simurgh\t_mode8.prg con el argumento -i (compila en memoria y ejecuta).
rem   tools\mode8.cmd div  ->  visor de un mapa de DIV 2: build\zig-probe\div2\wld\view.prg (con WLD_VIEW.WLD y WLD_VIEW.FPG al lado)
rem   W/S o flechas arriba/abajo: avanzar y retroceder      A/D: paso lateral      flechas izquierda/derecha: girar
rem   Q/Z: subir y bajar     R/F: mirar arriba y abajo     (visor de DIV: 1-6 saltan a las banderas, Mayus corre, N niebla, C cielo)     Esc: salir
rem Se ejecuta desde la carpeta del programa, asi que los mapas y texturas que se anadan alli se cargan por su nombre.
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
set "DIR=%ROOT%\tests\simurgh"
set "PRG=t_mode8.prg"
if /i "%~1"=="div" (
    set "DIR=%ROOT%\build\zig-probe\div2\wld"
    set "PRG=view.prg"
)
if not exist "%DIR%\%PRG%" (
    echo No se encuentra %DIR%\%PRG%
    pause
    exit /b 2
)
cd /d "%DIR%"
"%EXE%" %PRG% -i
if errorlevel 1 (
    echo.
    echo simr ha terminado con codigo %errorlevel%
    pause
)
endlocal
