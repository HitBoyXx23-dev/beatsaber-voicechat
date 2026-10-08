# Building the mod locally

## Windows (build.bat)

1. Install **CMake** (https://cmake.org/download/) and a C++ compiler (Visual Studio "Desktop development with C++" or MinGW). Make sure `cmake` is on PATH.
2. Double-click `build.bat`, or run it from a terminal in the repo folder.
   - It always builds and runs the host tests (`src/voice_core` logic).
   - To also build the Quest library and `.qmod`, install the **Android NDK** and set `ANDROID_NDK_HOME` to its folder, for example `C:\Android\ndk\26.1.10909125`. Then run `build.bat` again.
3. The Quest build also needs **Ninja** on PATH and **bash** (Git Bash works) for `scripts/pack-qmod.sh`.
4. The `.qmod` is written to `site/downloads/VoiceChat-1.40.8.qmod`.

## Linux / macOS

```bash
cmake -S . -B build/host && cmake --build build/host && ctest --test-dir build/host
```

## Dependencies for the Quest build

`qpm.json` lists `beatsaber-hook` and `scotland2`. Install them with qpm before the Quest build:

```bash
qpm restore
```

The version ranges are placeholders. Check them against the current qpm package index.

## What is implemented

| Area | Status |
| --- | --- |
| Voice packet encode/decode (format from MultiplayerExtensions.VoiceChat) | Done, tested on PC |
| Mute state and transmit gating | Done, tested on PC |
| Quest entry points (`setup`, `load`) | Done, stub |
| Mic capture, sending/receiving packets in the lobby | Not done |
| Mute button in the lobby UI | Not done |

The Quest-only parts need the game's headers and the Quest toolchain, so they have not been compiled here. The stubs in `src/main.cpp` are marked with TODOs.
