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

## Using it

In a multiplayer lobby a small button appears off to your left. It shows `MIC ON` (green) or `MIC OFF` (red); press it to toggle your mic.

Open the settings with the Voice Chat button in the main menu's mod list (also under Settings > Mod Settings > Voice Chat). They are saved to `/sdcard/ModData/com.beatgames.beatsaber/Configs/VoiceChat.json`:

| Setting | Default | What it does |
| --- | --- | --- |
| Enable voice chat | On | Turns voice chat off completely (no panel, no mic, no playback) |
| Start muted in lobbies | On | Joins each lobby muted until you press Unmute |
| Push-to-talk | Off | Only transmits while the chosen button is held; the Mute button is hidden |
| Push-to-talk button | Left grip | Left grip, Right grip, X, Y, A, B, Left stick click, Right stick click |

## PC version

`pc/VoiceChat` is a BSIPA plugin for Beat Saber PC 1.40.8 with the same panel and settings. Quest and PC players in the same lobby can hear each other: both send a MultiplayerCore packet named `VoicePacket` containing an int index, an int length, and up to 640 bytes of 16 kHz mono 16-bit PCM.

Requirements: the .NET SDK, and Beat Saber PC 1.40.8 with BSIPA, BSML, SiraUtil and MultiplayerCore installed (the build compiles against the game's own DLLs).

```bash
dotnet build pc/VoiceChat/VoiceChat.csproj -c Release -p:BeatSaberDir="C:\Program Files (x86)\Steam\steamapps\common\Beat Saber"
```

Copy `pc/VoiceChat/bin/Release/net472/VoiceChat.dll` into the game's `Plugins` folder, or add `-p:CopyToPlugins=true` to copy it automatically. Settings are under Mod Settings > Voice Chat and are saved to `UserData/VoiceChat.json`. The microphone is the system default recording device.

Cross-play needs everyone on a server that supports MultiplayerCore (for example BeatTogether), with Voice Chat 1.0.0 on both Quest and PC.

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
| `src/UI/VoiceMuteButton.*` | In-lobby mic toggle button |
| `src/UI/VoiceSettings.*` | Mod settings menu |
| `src/Config/VoiceConfig.*` | Saved settings and push-to-talk button mapping |
| `pc/VoiceChat/` | PC (BSIPA) version, cross-play compatible with Quest |
| `include/hooking.hpp` | Auto-install hook macros |
| `mod.template.json`, `qpm.json` | qmod manifest template and dependencies |
| `site/`, `api/`, `vercel.json` | Download page and version API for Vercel |

## Deploy the site (Vercel)

Import the repo in Vercel with no build command. Put the built `.qmod` in `site/downloads/VoiceChat-1.40.8.qmod` (the download link points there) and commit it. `GET /api/version` and `GET /api/health` are stateless.

## Status

Boots on Quest with Beat Saber 1.40.8. In-lobby voice quality has not been fully verified yet; playback is per-chunk and may sound choppy.

## Privacy

Voice is not encrypted end to end. Only talk with people you trust.
