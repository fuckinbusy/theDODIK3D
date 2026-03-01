@echo off
:: Скрипт быстрой сборки theDODIK3D через MSBuild (Debug|x64)
:: Запускать из корня проекта.

set CONFIG=Debug
set PLATFORM=x64
set SLN=theDODIK3D.sln

:: Ищем MSBuild в стандартных местах VS 2022
set MSBUILD=
for %%P in (
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
) do (
    if exist %%P set MSBUILD=%%P
)

if "%MSBUILD%"=="" (
    echo [ERROR] MSBuild.exe не найден. Откройте Developer Command Prompt или исправьте пути в build.bat.
    exit /b 1
)

echo [BUILD] %CONFIG%^|%PLATFORM%  -  %SLN%
%MSBUILD% %SLN% /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /nologo /m /v:minimal
if %ERRORLEVEL% neq 0 (
    echo [FAIL] Сборка завершилась с ошибками.
    exit /b %ERRORLEVEL%
)

echo [OK] Сборка успешна. Бинарник: game\%PLATFORM%\%CONFIG%\theDODIK3D.exe
