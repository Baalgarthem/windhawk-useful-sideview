@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1"
if errorlevel 1 exit /b 1
echo.
echo Build completado: dist\useful-sideview.wh.cpp
