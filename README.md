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
| `src/` | Mod source (voice core and Quest entry point) |
| `tests/` | Host tests for the core logic |
| `vercel.json` | Vercel config |

## Deploy to Vercel

1. Go to vercel.com, click **Add New → Project**, and import this repo.
2. Leave the build command empty. Vercel reads `vercel.json`, serves `site/`, and deploys `api/`.
3. Click **Deploy**. The site is then at `https://<project>.vercel.app`.
4. Check `https://<project>.vercel.app/api/health` returns `{"ok": true, ...}`.

## Build the .qmod

A `.qmod` is a zip containing `mod.json` and the compiled `libvoicechat.so`, placed in `site/downloads/`. The build is handled separately.

## Before you publish

- Check the dependency IDs and versions in `mod/mod.json` against the current QuestPatcher or qpm mod index. They are placeholders.
- The Quest entry point is a stub. Mic capture, lobby packets, and the mute button are not implemented yet .

## Privacy

Voice is not encrypted end to end. Only talk with people you trust.
