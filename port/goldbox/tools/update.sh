#!/bin/bash
# Gold Box Game Engine: the updater's network half (the port's menu runs
# it: Check for Updates). As Pocket Curator's updater: everything goes
# through the firmware's own curl, nothing touches the live install - a
# release is downloaded, verified and unpacked into <gamedir>/.update/
# staged/, READY written last; the launch script applies it at the start
# of the next launch.
#
#   update.sh check                        -> TAG v0.3.0 / ZIP <url> / SHA <url> / PRE 0|1
#   update.sh fetch <tag> <zip> <sha> <gamedir>   -> STAGED v0.3.0
# A problem is one line: ERROR <what to tell the player>.
REPO="tomtombombadil/PM-GoldBoxGameEngine"
API="${GOLDBOX_RELEASES_URL:-https://api.github.com/repos/$REPO/releases?per_page=20}"

fail() { echo "ERROR $*"; exit 1; }
command -v curl >/dev/null 2>&1 || fail "curl not found on this firmware."

excuse() {
  if [ "$(date +%Y)" -lt 2026 ]; then
    echo "Can't reach GitHub: the device's clock is wrong ($(date +%F)). Connect Wi-Fi, wait a minute, try again."
  else
    echo "Can't reach GitHub. Is Wi-Fi connected?"
  fi
}

case "$1" in
check)
  json="$(curl -sfL --connect-timeout 10 -m 25 -H 'Accept: application/vnd.github+json' "$API")" || fail "$(excuse)"
  # The list is newest first, pre-releases included; GitHub prints a
  # field a line. The first port zip named is the newest release's.
  zip="$(printf '%s\n' "$json" | grep -o '"browser_download_url": *"[^"]*goldbox_port-v[0-9.]*\.zip"' | head -1 | sed -E 's/.*"(https?:[^"]+|file:[^"]+)"$/\1/')"
  [ -n "$zip" ] || fail "No release found on GitHub yet."
  tag="$(printf '%s' "$zip" | sed -E 's/.*goldbox_port-(v[0-9.]+)\.zip$/\1/')"
  sha="$(printf '%s\n' "$json" | grep -o "\"browser_download_url\": *\"[^\"]*goldbox_port-$tag\.zip\.sha256\"" | head -1 | sed -E 's/.*"([a-z]+:[^"]+)"$/\1/')"
  # (is that release a pre-release? its "prerelease" line follows its tag's)
  pre="$(printf '%s\n' "$json" | grep -E '"tag_name"|"prerelease"' | grep -A1 "\"$tag\"" | grep -c 'true')"
  echo "TAG $tag"
  echo "ZIP $zip"
  echo "SHA $sha"
  echo "PRE $pre"
  ;;
fetch)
  tag="$2"; zip_url="$3"; sha_url="$4"; game="$5"
  [ -n "$tag" ] && [ -n "$zip_url" ] && [ -d "$game" ] || fail "update.sh fetch: bad arguments."
  command -v unzip >/dev/null 2>&1 || fail "unzip not found on this firmware."
  free_kb=$(df -Pk "$game" 2>/dev/null | awk 'NR==2 {print $4}')
  [ -z "$free_kb" ] || [ "$free_kb" -ge 30720 ] || fail "Not enough free space ($((free_kb / 1024)) MB free, 30 MB needed)."
  up="$game/.update"
  rm -rf "$up"; mkdir -p "$up" || fail "Can't write to the card."
  curl -sfL --connect-timeout 10 -m 600 -o "$up/update.zip" "$zip_url" \
    || { rm -rf "$up"; fail "Download failed. Check Wi-Fi and try again."; }
  if [ -n "$sha_url" ] && command -v sha256sum >/dev/null 2>&1; then
    want="$(curl -sfL --connect-timeout 10 -m 20 "$sha_url" | awk '{print $1}')"
    if [ -n "$want" ]; then
      got="$(sha256sum "$up/update.zip" | awk '{print $1}')"
      [ "$got" = "$want" ] || { rm -rf "$up"; fail "The download failed its integrity check. Try again."; }
    fi
  fi
  unzip -tqq "$up/update.zip" >/dev/null 2>&1 || { rm -rf "$up"; fail "The downloaded file is damaged. Try again."; }
  unzip -oqq "$up/update.zip" -d "$up/staged" || { rm -rf "$up"; fail "Unpacking failed (card full?)."; }
  [ -f "$up/staged/goldbox/VERSION" ] || { rm -rf "$up"; fail "That release isn't a Gold Box Game Engine package."; }
  rm -f "$up/update.zip"
  echo "$tag" > "$up/READY"       # last: its presence says the staged copy is whole
  sync 2>/dev/null
  echo "STAGED $tag"
  ;;
*) fail "update.sh: check | fetch" ;;
esac
