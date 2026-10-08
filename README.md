# Beat Saber Voice Chat (1.40.8)

Voice chat for Beat Saber multiplayer lobbies, with a mic mute button.

## How it works

- **Voice does not go through a server.** Audio is sent as packets between players in the same lobby, over the game's own multiplayer connection. Anyone in the lobby who has the mod can hear and talk.
- **The Vercel project only serves the download page and small JSON endpoints:**
  - `GET /api/version`: the latest mod and game version, and the qmod path.
  - `GET /api/health`: a status check.
  These endpoints are stateless, so they work on Vercel's serverless functions without a database.

## Repository layout

| Path | What it is |
| --- | --- |
| `site/index.html` | Download page |
| `site/downloads/` | Put the built `.qmod` here |
| `api/` | Vercel serverless functions |
| `mod/mod.json` | qmod manifest (game 1.40.8) |
| `scripts/pack-qmod.sh` | Packs a built `.so` into a `.qmod` |
| `vercel.json` | Vercel config |

## Deploy to Vercel

1. Go to vercel.com, click **Add New → Project**, and import this repo.
2. Leave the build command empty. Vercel reads `vercel.json`, serves `site/`, and deploys `api/`.
3. Click **Deploy**. The site is then at `https://<project>.vercel.app`.
4. Check `https://<project>.vercel.app/api/health` returns `{"ok": true, ...}`.

## Build the .qmod

A `.qmod` is a zip file with `mod.json` and your compiled native library at the top level.

You need:
- The Quest modding toolchain for Beat Saber 1.40.8: Android NDK, CMake, and the `qpm` package manager (or the template your mod was made from).
- The mod's compiled library, for example `libvoicechat.so`.

Steps:

1. Build the library for `arm64-v8a` with your toolchain. The output is `libvoicechat.so`.
2. From the repo root, run:
   ```bash
   scripts/pack-qmod.sh path/to/libvoicechat.so
   ```
   This writes `site/downloads/VoiceChat-1.40.8.qmod`.
3. Check the archive:
   ```bash
   unzip -l site/downloads/VoiceChat-1.40.8.qmod
   ```
   It should list `mod.json` and `libvoicechat.so`.
4. Commit and push the `.qmod` so Vercel redeploys with it.

## Before you publish

- Check the dependency IDs and versions in `mod/mod.json` against the current QuestPatcher or qpm mod index. They are placeholders.
- The mod's C++/C# source is not in this repo yet. Without it there is no `.so` to pack, so the download link returns 404 until it's built.

## Privacy

Voice is not encrypted end to end. Only talk with people you trust.
