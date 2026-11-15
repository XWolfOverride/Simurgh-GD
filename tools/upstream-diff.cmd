@echo off
rem Envoltorio de upstream-diff.sh para Windows (necesita el bash de Git para Windows).
rem Uso: tools\upstream-diff.cmd [--names ^| --stat ^| --patch ^| --check] [fichero-salida]
setlocal
set "BASH="
where bash >nul 2>nul && set "BASH=bash"
if not defined BASH if exist "%ProgramFiles%\Git\bin\bash.exe" set "BASH=%ProgramFiles%\Git\bin\bash.exe"
if not defined BASH if exist "%ProgramFiles(x86)%\Git\bin\bash.exe" set "BASH=%ProgramFiles(x86)%\Git\bin\bash.exe"
if not defined BASH (
    echo [X]  No se encuentra bash. Instala Git para Windows o MSYS2.
    exit /b 2
)
"%BASH%" "%~dp0upstream-diff.sh" %*
exit /b %errorlevel%
