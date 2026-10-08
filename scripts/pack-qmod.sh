#!/usr/bin/env bash
# Packs a built Quest mod into a .qmod (a zip) and copies it into site/downloads.
# Usage: scripts/pack-qmod.sh path/to/libvoicechat.so
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SO="${1:?usage: pack-qmod.sh path/to/libvoicechat.so}"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
cp "$ROOT/mod/mod.json" "$STAGE/mod.json"
cp "$SO" "$STAGE/libvoicechat.so"
OUT="$ROOT/site/downloads/VoiceChat-1.40.8.qmod"
(cd "$STAGE" && zip -q -r "$OUT" mod.json libvoicechat.so)
echo "Wrote $OUT"
