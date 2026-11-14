#!/bin/bash
# Gold Box Game Engine: Send Logs - puts the port's logs on a WebDAV
# server (the developer's PC) with the firmware's curl.
#   sendlogs.sh <server address> <gamedir> <version>
# -> SENT <n> <folder on the server>   or   ERROR <what to tell the player>
url="${1%/}"; game="$2"; ver="$3"
fail() { echo "ERROR $*"; exit 1; }
[ -n "$url" ] && [ -d "$game" ] || fail "No log server set."
command -v curl >/dev/null 2>&1 || fail "curl not found on this firmware."
case "$url" in http://*|https://*) ;; *) url="http://$url" ;; esac

stamp="$(date '+%Y%m%d-%H%M%S')"
dir="goldbox-logs"
# (the folder may be there already: its result doesn't matter)
curl -s -o /dev/null --connect-timeout 6 -m 15 -X MKCOL "$url/$dir/" 2>/dev/null

# What goes: this run's log, the run before, the updater's, the settings,
# and what the engine wrote about each game (text only - never game files)
n=0; failed=""
send() {  # <file> <name on the server>
  [ -s "$1" ] || return 0
  if curl -sf -o /dev/null --connect-timeout 6 -m 60 -T "$1" "$url/$dir/${stamp}_v${ver}_$2"; then
    n=$((n + 1))
  else
    failed="$2"
  fi
}
send "$game/log.txt" "log.txt"
send "$game/log.prev.txt" "log.prev.txt"
send "$game/update.log" "update.log"
send "$game/conf/settings.cfg" "settings.cfg"
for f in "$game"/games/_CYD/*.TXT "$game"/games/_CYD/*/*.TXT; do
  [ -f "$f" ] || continue
  rel="${f#"$game"/games/_CYD/}"
  send "$f" "engine_$(printf '%s' "$rel" | tr '/ ' '__')"
done
if [ "$n" -eq 0 ]; then
  fail "Couldn't reach the log server at $url. Is it running, and is Wi-Fi connected?"
fi
[ -z "$failed" ] || fail "Sent $n file(s), but $failed failed."
echo "SENT $n $dir/${stamp}_v${ver}_*"
