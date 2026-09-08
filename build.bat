@echo off
setlocal
chcp 65001 >nul
title Compilando Useful Side View Bundle...

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1"
set CODIGO_SALIDA=%ERRORLEVEL%

if %CODIGO_SALIDA% equ 0 (
    echo.
    echo ========================================================
    echo   [OK] Compilacion completada con exito.
    echo   Archivo generado en: dist\useful-sideview.wh.cpp
    echo ========================================================
) else (
    echo.
    echo ========================================================
    echo   [ERROR] La compilacion del bundle ha fallado.
    echo ========================================================
)

echo.
timeout /t 4 >nul 2>&1

exit /b %CODIGO_SALIDA%
