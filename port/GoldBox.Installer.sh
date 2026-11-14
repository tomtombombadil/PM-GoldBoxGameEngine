#!/bin/bash
# ===========================================================================
# Gold Box Game Engine - one-file installer
# ===========================================================================
# Copy THIS FILE alone into your ports folder (roms/ports) and start it from
# the Ports menu. With Wi-Fi connected it downloads the newest Gold Box Game
# Engine release from GitHub, checks it and installs it.
#
# It installs the NEWEST release, pre-releases included. After that the
# port updates itself: Start menu - Check for Updates. Running this again
# reinstalls / repairs; your games, saves and settings are kept.
#
# (After Pocket Curator's installer - same author, MIT.)
# ===========================================================================
REPO="tomtombombadil/PM-GoldBoxGameEngine"
API="${GOLDBOX_RELEASES_URL:-https://api.github.com/repos/$REPO/releases?per_page=20}"

# Windows line ends, should they sneak in: run a cleaned copy
if LC_ALL=C grep -q $'\r' "$0" 2>/dev/null; then
  self_clean="$(dirname "$0")/.${0##*/}.lf"
  if tr -d '\r' < "$0" > "$self_clean" 2>/dev/null && [ -s "$self_clean" ]; then
    chmod +x "$self_clean" 2>/dev/null
    exec /bin/bash "$self_clean" "$@"
  fi
  echo "[goldbox-install] the installer has Windows line ends" >&2
  exit 1
fi

{ # (the whole script read before it runs)

shopt -s expand_aliases
XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}
SCRIPT_DIR="$(dirname "$0")"

controlfolder=""
for candidate in "/opt/system/Tools/PortMaster" "/opt/tools/PortMaster" "$XDG_DATA_HOME/PortMaster" \
                 "$SCRIPT_DIR/PortMaster" "/roms/ports/PortMaster" "/roms2/ports/PortMaster"; do
  if [ -f "$candidate/control.txt" ]; then
    controlfolder="$candidate"
    break
  fi
done
if [ -z "$controlfolder" ]; then
  echo "[goldbox-install] PortMaster isn't installed - install it first (https://portmaster.games)" >&2
  exit 1
fi
source "$controlfolder/control.txt"
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
type pm_message >/dev/null 2>&1 || pm_message() { echo "[pm_message] $*"; }

PORTS_DIR="/$directory/ports"
GAMEDIR="$PORTS_DIR/goldbox"
WORK_ZIP="$PORTS_DIR/.goldbox_install.zip"
LOGF="$PORTS_DIR/goldbox_install.log"
> "$LOGF" 2>/dev/null

say() {
  echo "[goldbox-install] $*"
  { echo "$(date '+%F %T') $*" >> "$LOGF"; sync; } 2>/dev/null
  pm_message "Gold Box Game Engine: $*"
}
die() {
  say "FAILED: $*"
  sleep "${GOLDBOX_PAUSE:-6}"
  rm -f "$WORK_ZIP"
  exit 1
}

say "installer starting on $CFW_NAME"
command -v curl  >/dev/null 2>&1 || die "curl not found on this firmware."
command -v unzip >/dev/null 2>&1 || die "unzip not found on this firmware."
if [ "$(date +%Y)" -lt 2026 ]; then
  say "the device's clock looks wrong ($(date +%F)) - if this fails: connect Wi-Fi, wait a minute, run me again."
fi
free_kb=$(df -Pk "$PORTS_DIR" 2>/dev/null | awk 'NR==2 {print $4}')
if [ -n "$free_kb" ] && [ "$free_kb" -lt 30720 ]; then
  die "not enough free space ($((free_kb / 1024)) MB free, 30 MB needed)."
fi

say "looking up the newest release..."
json="$(curl -sfL --connect-timeout 10 -m 25 -H 'Accept: application/vnd.github+json' "$API")" \
  || die "can't reach GitHub. Is Wi-Fi connected?"
zip_url="$(printf '%s\n' "$json" | grep -o '"browser_download_url": *"[^"]*goldbox_port-v[0-9.]*\.zip"' | head -1 | sed -E 's/.*"([a-z]+:[^"]+)"$/\1/')"
[ -n "$zip_url" ] || die "no release found on GitHub."
tag="$(printf '%s' "$zip_url" | sed -E 's/.*goldbox_port-(v[0-9.]+)\.zip$/\1/')"
sha_url="$(printf '%s\n' "$json" | grep -o "\"browser_download_url\": *\"[^\"]*goldbox_port-$tag\.zip\.sha256\"" | head -1 | sed -E 's/.*"([a-z]+:[^"]+)"$/\1/')"

say "found $tag - downloading..."
rm -f "$WORK_ZIP"
curl -sfL --connect-timeout 10 -m 600 -o "$WORK_ZIP" "$zip_url" >> "$LOGF" 2>&1 \
  || die "download failed. Check Wi-Fi and try again."
if [ -n "$sha_url" ] && command -v sha256sum >/dev/null 2>&1; then
  want="$(curl -sfL --connect-timeout 10 -m 20 "$sha_url" | awk '{print $1}')"
  if [ -n "$want" ]; then
    got="$(sha256sum "$WORK_ZIP" | awk '{print $1}')"
    [ "$got" = "$want" ] || die "the download failed its integrity check. Try again."
    say "checksum verified."
  fi
fi
unzip -tqq "$WORK_ZIP" >/dev/null 2>&1 || die "the downloaded zip is damaged. Try again."

say "installing $tag..."
# (a release holds no settings, saves or games: nothing of the player's is replaced)
unzip -oqq "$WORK_ZIP" -d "$PORTS_DIR" || die "unpacking failed (card full or read-only?)."
rm -f "$WORK_ZIP"
chmod +x "$PORTS_DIR/Gold Box Game Engine.sh" "$GAMEDIR"/goldbox.* "$GAMEDIR"/tools/*.sh 2>/dev/null
# The folder the player's games go in must be there the moment this ends
# (Tom, 2026-11-14: he copies the games over before the first start)
mkdir -p "$GAMEDIR/games" "$GAMEDIR/conf"
[ -d "$GAMEDIR/games" ] || die "couldn't make $GAMEDIR/games."
# (the log began in the ports folder, before the port's own existed)
mv -f "$LOGF" "$GAMEDIR/install.log" 2>/dev/null && LOGF="$GAMEDIR/install.log"

# The new entry shows once the games list is read again
if command -v curl >/dev/null 2>&1; then
  curl -s -m 3 -o /dev/null "http://127.0.0.1:1234/reloadgames" 2>/dev/null
fi
sync 2>/dev/null
say "done! $tag is installed. Copy your games into ports/goldbox/games. If 'Gold Box Game Engine' isn't in Ports yet: Start - Game Settings - Update Gamelists."
sleep "${GOLDBOX_PAUSE:-6}"
exit 0

} # (end of the read-it-all brace)
