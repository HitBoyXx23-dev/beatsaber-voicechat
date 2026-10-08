@echo off
setlocal EnableDelayedExpansion
REM Builds the voice chat mod. Downloads any missing requirements into .\tools
REM   1. CMake, Ninja (host build tools)   -> downloaded if not on PATH
REM   2. Android NDK r26b (Quest build)     -> downloaded if ANDROID_NDK_HOME is not set
REM   3. qpm (mod dependency manager)       -> downloaded if not on PATH
REM Then: host tests, Quest build, qpm restore, and .qmod packing.
cd /d "%~dp0"
set "TOOLS=%CD%\tools"
if not exist "%TOOLS%" mkdir "%TOOLS%"

set "CMAKE_VER=3.30.5"
set "NINJA_VER=1.12.1"
set "NDK_ZIP=android-ndk-r26b-windows.zip"
set "NDK_URL=https://dl.google.com/android/repository/%NDK_ZIP%"
set "QPM_URL=https://github.com/QuestPackageManager/QPM.CLI/releases/latest/download/qpm-rust.exe"

REM ---- CMake ----
where cmake >nul 2>nul || (
    if not exist "%TOOLS%\cmake\bin\cmake.exe" (
        call :download "https://github.com/Kitware/CMake/releases/download/v%CMAKE_VER%/cmake-%CMAKE_VER%-windows-x86_64.zip" "%TOOLS%\cmake.zip" || exit /b 1
        call :unzip "%TOOLS%\cmake.zip" "%TOOLS%" || exit /b 1
        for /d %%D in ("%TOOLS%\cmake-*") do ren "%%D" cmake
    )
    set "PATH=%TOOLS%\cmake\bin;!PATH!"
)

REM ---- Ninja ----
where ninja >nul 2>nul || (
    if not exist "%TOOLS%\ninja\ninja.exe" (
        call :download "https://github.com/ninja-build/ninja/releases/download/v%NINJA_VER%/ninja-win.zip" "%TOOLS%\ninja.zip" || exit /b 1
        call :unzip "%TOOLS%\ninja.zip" "%TOOLS%\ninja" || exit /b 1
    )
    set "PATH=%TOOLS%\ninja;!PATH!"
)

REM ---- qpm ----
where qpm >nul 2>nul || (
    if not exist "%TOOLS%\qpm\qpm.exe" (
        mkdir "%TOOLS%\qpm" 2>nul
        call :download "%QPM_URL%" "%TOOLS%\qpm\qpm.exe" || (
            echo [error] Could not download qpm. Install it manually from https://github.com/QuestPackageManager/QPM.CLI/releases
            echo         then put qpm.exe on PATH and run build.bat again.
            exit /b 1
        )
    )
    set "PATH=%TOOLS%\qpm;!PATH!"
)

REM ---- Host tests (always) ----
echo.
echo === Host tests ===
cmake -S . -B build\host || exit /b 1
cmake --build build\host --config Release || exit /b 1
ctest --test-dir build\host -C Release --output-on-failure || exit /b 1

REM ---- Android NDK (Quest build) ----
if "%ANDROID_NDK_HOME%"=="" (
    if not exist "%TOOLS%\ndk\build\cmake\android.toolchain.cmake" (
        echo.
        echo === Downloading Android NDK r26b (about 1 GB) ===
        call :download "%NDK_URL%" "%TOOLS%\%NDK_ZIP%" || exit /b 1
        call :unzip "%TOOLS%\%NDK_ZIP%" "%TOOLS%" || exit /b 1
        for /d %%D in ("%TOOLS%\android-ndk-*") do ren "%%D" ndk
    )
    set "ANDROID_NDK_HOME=%TOOLS%\ndk"
)

echo.
echo === Restoring mod dependencies (qpm) ===
qpm restore || (echo [error] qpm restore failed. Check the versions in qpm.json. & exit /b 1)

echo.
echo === Quest build ===
cmake -S . -B build\quest -G Ninja -DANDROID=ON ^
  -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake" ^
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build build\quest || exit /b 1

REM ---- Pack .qmod (a zip with mod.json and the library) ----
echo.
echo === Pack qmod ===
set "STAGE=%TOOLS%\stage"
if exist "%STAGE%" rmdir /s /q "%STAGE%"
mkdir "%STAGE%"
copy /y mod\mod.json "%STAGE%\mod.json" >nul
copy /y build\quest\libvoicechat.so "%STAGE%\libvoicechat.so" >nul || exit /b 1
set "QMOD=site\downloads\VoiceChat-1.40.8.qmod"
powershell -NoProfile -Command "Compress-Archive -Force -Path '%STAGE%\mod.json','%STAGE%\libvoicechat.so' -DestinationPath '%CD%\%QMOD%.zip'; Move-Item -Force '%CD%\%QMOD%.zip' '%CD%\%QMOD%'" || exit /b 1
echo.
echo Done: %QMOD%
exit /b 0

REM ---- helpers ----
:download
echo Downloading %~nx2 ...
powershell -NoProfile -Command "$ProgressPreference='SilentlyContinue'; try { Invoke-WebRequest -UseBasicParsing -Uri '%~1' -OutFile '%~2' } catch { Write-Host $_.Exception.Message; exit 1 }"
exit /b %errorlevel%

:unzip
echo Extracting %~nx1 ...
powershell -NoProfile -Command "Expand-Archive -Force -Path '%~1' -DestinationPath '%~2'"
exit /b %errorlevel%
