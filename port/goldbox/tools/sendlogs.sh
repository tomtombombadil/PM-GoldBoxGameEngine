#!/bin/bash
# Gold Box Game Engine: Send Logs - uploads the port's logs (text only, never
# game files) with the firmware's curl to the developer's log relay, a small
# web script that emails them on (tools/logrelay/Code.gs in the repo). The
# player's reply address, if they gave one, goes with them.
#   sendlogs.sh <gamedir> <version> [reply address]
# -> SENT <n> [thanks]   or   ERROR <what to tell the player>
game="$1"; ver="$2"; reply="$3"
RELAY="${GOLDBOX_RELAY_URL:-https://script.google.com/macros/s/AKfycbwzwY0ThckpiiYTi0npBbaCzLvE5ppzWmilPtgH-w4uclPy28K9YXqqs8jolAJIm3bh/exec}"
fail() { echo "ERROR $*"; exit 1; }
[ -d "$game" ] || fail "The port's folder wasn't found."
command -v curl >/dev/null 2>&1 || fail "curl not found on this firmware."

# What the launch script wrote about this device (device.txt: name=value)
field() { sed -n "s/^$1=//p" "$game/device.txt" 2>/dev/null | head -n 1; }

tmp="$game/.sendlogs"
rm -rf "$tmp"; mkdir -p "$tmp" || fail "Couldn't write in the port's folder."
trap 'rm -rf "$tmp"' EXIT

args=(-sS -L --connect-timeout 10 -m 180
      --data-urlencode "app=goldbox" --data-urlencode "version=$ver"
      --data-urlencode "device=$(field device)" --data-urlencode "firmware=$(field firmware)"
      --data-urlencode "portmaster=$(field portmaster)" --data-urlencode "screen=$(field screen)"
      --data-urlencode "reply=$reply")
n=0
# <file> <name it goes under> <most bytes: its end is what matters>
add() {
  [ -s "$1" ] && [ "$n" -lt 12 ] || return 0
  tail -c "$3" "$1" > "$tmp/$2" 2>/dev/null || return 0
  args+=(--data-urlencode "file:$2@$tmp/$2")
  n=$((n + 1))
}
# This run's log, the run before, the updater's, the device, the settings,
# and what the engine wrote about each game
add "$game/log.txt" "log.txt" 600000
add "$game/log.prev.txt" "log.prev.txt" 300000
add "$game/device.txt" "device.txt" 4000
add "$game/update.log" "update.log" 60000
add "$game/conf/settings.cfg" "settings.cfg" 8000
for f in "$game"/games/_CYD/*.TXT "$game"/games/_CYD/*/*.TXT; do
  [ -f "$f" ] || continue
  case "$f" in */BOOK.TXT) continue ;; esac     # (a journal's text: the player's game, not a log)
  rel="${f#"$game"/games/_CYD/}"
  add "$f" "engine_$(printf '%s' "$rel" | tr -c 'A-Za-z0-9._-' '_')" 60000
done
[ "$n" -gt 0 ] || fail "There are no logs to send yet."

out="$(curl "${args[@]}" "$RELAY" 2>"$tmp/curl.err")" ||
  fail "Couldn't reach the log service. Is Wi-Fi connected? ($(head -c 80 "$tmp/curl.err" | tr '\n' ' '))"
case "$out" in
  "OK "*)    echo "SENT $n$(case "$out" in *thanks*) echo ' thanks' ;; esac)" ;;
  "ERROR "*) fail "The log service said: ${out#ERROR }" ;;
  *)         fail "The log service gave an answer the port didn't expect. Try again later." ;;
esac
