# Beat Saber Voice Chat (Quest, 1.40.8)

Voice chat for Beat Saber multiplayer lobbies on standalone Quest, with a mic mute button.
Players talk directly through the game's multiplayer packets (via MultiplayerCore), so no voice server is needed.

## Build the mod

Requirements: [qpm](https://github.com/QuestPackageManager/QPM.CLI), CMake, Ninja, and the Android NDK (qpm installs the version listed in `qpm.json`).

```bash
qpm restore          # downloads beatsaber-hook, bs-cordl, multiplayer-core, bsml, ...
qpm s build          # builds build/libvoicechat.so
qpm qmod zip         # packs VoiceChat.qmod from mod.template.json
```

Install the `.qmod` with [MBF](https://mbf.bsquest.xyz), then also install MultiplayerCore from the same page.

## Host tests (no Quest toolchain)

The packet format, mute state, and jitter buffer are plain C++ and tested on PC:

```bash
cmake -S . -B build/host && cmake --build build/host && ctest --test-dir build/host
```

## Layout

| Path | What it is |
| --- | --- |
| `src/voice_core.*`, `src/jitter_buffer.*` | Platform-independent logic (tested) |
| `src/Voice/` | Voice packet and the controller (mic capture, send, receive, playback) |
| `src/Hooks/VoiceHooks.cpp` | Attaches the controller when a lobby session starts |
| `src/UI/VoiceMuteButton.*` | In-lobby mute button |
| `include/hooking.hpp` | Auto-install hook macros |
| `mod.template.json`, `qpm.json` | qmod manifest template and dependencies |
| `site/`, `api/`, `vercel.json` | Download page and version API for Vercel |

## Deploy the site (Vercel)

Import the repo in Vercel with no build command. Put the built `.qmod` in `site/downloads/VoiceChat-1.40.8.qmod` (the download link points there) and commit it. `GET /api/version` and `GET /api/health` are stateless.

## Status

Not yet verified on a headset. Known risks: Unity audio calls from the capture thread, per-chunk playback quality, and a few API signatures (`BSML::Lite::CreateUIButton`, `NetDataWriter::Put`) that depend on the exact dependency versions.

## Privacy

Voice is not encrypted end to end. Only talk with people you trust.
