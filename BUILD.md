# Building the mod locally

## Windows (build.bat)

Just double-click `build.bat` or run it from a terminal in the repo folder. It downloads what's missing into `tools\` (git-ignored):

| Requirement | Used for | How it's obtained |
| --- | --- | --- |
| CMake 3.30.5 | Build system | Downloaded if not on PATH |
| Ninja 1.12.1 | Quest build | Downloaded if not on PATH |
| qpm | Mod dependencies | Downloaded from the QPM.CLI latest release if not on PATH |
| Android NDK r26b (~1 GB) | Quest C++ compiler | Downloaded if `ANDROID_NDK_HOME` is not set |

Then it runs the host tests, `qpm restore`, the Quest build, and packs `site\downloads\VoiceChat-1.40.8.qmod`.

Notes:
- Downloads need internet access. The NDK download is large, so the first run takes a while.
- You need a C++ compiler on PATH for the host tests (Visual Studio "Desktop development with C++", or MinGW).
- If the qpm download fails, install qpm manually from the QPM.CLI releases page and rerun.
- To use your own NDK, set `ANDROID_NDK_HOME` before running.

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
