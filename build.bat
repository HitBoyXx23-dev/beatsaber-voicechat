@echo off
setlocal
REM Builds the voice chat mod.
REM   1. Always: compiles and runs the host tests (needs CMake and a C++ compiler on PATH).
REM   2. If ANDROID_NDK_HOME is set: builds libvoicechat.so for Quest and packs the .qmod.
cd /d "%~dp0"

where cmake >nul 2>nul || (echo [error] CMake not found. Install from https://cmake.org/download/ & exit /b 1)

echo === Host tests ===
cmake -S . -B build\host || exit /b 1
cmake --build build\host --config Release || exit /b 1
ctest --test-dir build\host -C Release --output-on-failure || exit /b 1

if "%ANDROID_NDK_HOME%"=="" (
    echo.
    echo [skip] ANDROID_NDK_HOME is not set, so the Quest build was skipped.
    echo        Set it to your Android NDK folder and run build.bat again.
    exit /b 0
)

echo.
echo === Quest build ===
cmake -S . -B build\quest -G "Ninja" -DANDROID=ON ^
  -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake" ^
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build build\quest || exit /b 1

echo.
echo === Pack qmod ===
bash scripts/pack-qmod.sh build\quest\libvoicechat.so || exit /b 1
echo Done: site\downloads\VoiceChat-1.40.8.qmod
