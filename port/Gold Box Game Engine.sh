#!/bin/bash
# PortMaster launch script: Gold Box Game Engine

# Windows line ends, should they sneak in: run a cleaned copy
if LC_ALL=C grep -q $'\r' "$0" 2>/dev/null; then
  self_clean="$(dirname "$0")/.${0##*/}.lf"
  if tr -d '\r' < "$0" > "$self_clean" 2>/dev/null && [ -s "$self_clean" ]; then
    chmod +x "$self_clean" 2>/dev/null
    exec /bin/bash "$self_clean" "$@"
  fi
  echo "[goldbox] the launch script has Windows line ends" >&2
  exit 1
fi

{ # (the whole script read before it runs: an update replaces this file)

shopt -s expand_aliases     # (some firmwares define PortMaster's pm_* helpers as aliases)
XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls

PORTSDIR="/$directory/ports"
GAMEDIR="$PORTSDIR/goldbox"
CONFDIR="$GAMEDIR/conf"
BINARY="goldbox.${DEVICE_ARCH}"
SCRIPT="Gold Box Game Engine.sh"

# ---- An update the port downloaded (Start menu - Check for Updates) -------
# tools/update.sh left it whole in .update/staged with READY written last.
# It is put in place here, before anything of the port runs: the port's
# folder first, this script last (copy, then rename), .update removed only
# when all of it worked - a power cut on the way leaves READY and the next
# launch does it again. Settings (conf/) and the player's games (games/)
# are never in a release, so they stay.
UPD="$GAMEDIR/.update"
if [ -z "$GOLDBOX_SKIP_UPDATE" ] && [ -f "$UPD/READY" ] && [ -f "$UPD/staged/goldbox/VERSION" ]; then
  ULOG="$GAMEDIR/update.log"
  ulog() { echo "[goldbox-update] $*"; echo "$(date '+%F %T') $*" >> "$ULOG" 2>/dev/null; }
  ufail() {
    ulog "FAILED: $* - left staged, the next launch tries again"
    GOLDBOX_SKIP_UPDATE=1 exec /bin/bash "$PORTSDIR/$SCRIPT" "$@"
  }
  ulog "applying $(cat "$UPD/READY") over $(cat "$GAMEDIR/VERSION" 2>/dev/null)"
  cp -a "$UPD/staged/goldbox/." "$GAMEDIR/" 2>>"$ULOG" || cp -rf "$UPD/staged/goldbox/." "$GAMEDIR/" 2>>"$ULOG" \
    || ufail "copying the port's files"
  [ "$(cat "$GAMEDIR/VERSION" 2>/dev/null)" = "$(cat "$UPD/staged/goldbox/VERSION")" ] || ufail "the new files did not take"
  if [ -f "$UPD/staged/$SCRIPT" ]; then
    cp "$UPD/staged/$SCRIPT" "$PORTSDIR/.$SCRIPT.new" && chmod +x "$PORTSDIR/.$SCRIPT.new" \
      && mv -f "$PORTSDIR/.$SCRIPT.new" "$PORTSDIR/$SCRIPT" || ufail "replacing the launch script"
  fi
  rm -rf "$UPD"
  sync 2>/dev/null
  ulog "now $(cat "$GAMEDIR/VERSION" 2>/dev/null); starting it"
  exec /bin/bash "$PORTSDIR/$SCRIPT" "$@"
fi

# The player's own game folders go in games/; settings in conf/
mkdir -p "$CONFDIR" "$GAMEDIR/games"
cd "$GAMEDIR"

# This run's log; the run before is kept beside it (Send Logs takes both)
[ -s "$GAMEDIR/log.txt" ] && mv -f "$GAMEDIR/log.txt" "$GAMEDIR/log.prev.txt"
> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1

# Every log starts with what it ran on (Tom, 2026-11-14): this port's
# version, the device's make and model, the firmware and its version,
# PortMaster's version. device.txt holds the same for Send Logs.
GB_VERSION="$(cat "$GAMEDIR/VERSION" 2>/dev/null)"
GB_MODEL="$(tr -d '\0' < /sys/firmware/devicetree/base/model 2>/dev/null)"
[ -n "$GB_MODEL" ] || GB_MODEL="$(cat /sys/class/dmi/id/product_name 2>/dev/null)"
GB_DEVICE="${DEVICE_NAME:-unknown}"
[ -n "$GB_MODEL" ] && GB_DEVICE="$GB_DEVICE ($GB_MODEL)"
# (PortMaster's version: its "version" file, else the line in its program)
GB_PM="$(head -n 1 "$controlfolder/version" 2>/dev/null | tr -d '\r')"
[ -n "$GB_PM" ] || GB_PM="$(sed -n "s/^PORTMASTER_VERSION *= *['\"]\([^'\"]*\)['\"].*/\1/p" "$controlfolder/pugwash" 2>/dev/null | head -n 1)"
{
  echo "version=${GB_VERSION:-unknown}"
  echo "device=$GB_DEVICE"
  echo "cpu=${DEVICE_CPU:-unknown} ${DEVICE_ARCH}, RAM ${DEVICE_RAM:-?} GB"
  echo "firmware=${CFW_NAME:-unknown} ${CFW_VERSION}"
  echo "portmaster=${GB_PM:-unknown}"
  echo "screen=${DISPLAY_WIDTH}x${DISPLAY_HEIGHT}"
  echo "sticks=${ANALOG_STICKS}"
} > "$GAMEDIR/device.txt" 2>/dev/null
echo "[goldbox] ==== $(date '+%F %T') ===="
echo "[goldbox] Gold Box Game Engine: v${GB_VERSION:-unknown}"
echo "[goldbox] Device:               $GB_DEVICE - ${DEVICE_CPU:-unknown cpu}, ${DEVICE_ARCH}, RAM ${DEVICE_RAM:-?} GB"
echo "[goldbox] Firmware:             ${CFW_NAME:-unknown} ${CFW_VERSION}"
echo "[goldbox] PortMaster:           ${GB_PM:-unknown}"
echo "[goldbox] Screen:               ${DISPLAY_WIDTH}x${DISPLAY_HEIGHT}, sticks ${ANALOG_STICKS}"

export LD_LIBRARY_PATH="$GAMEDIR/libs.${DEVICE_ARCH}:$LD_LIBRARY_PATH"
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

if [ ! -f "$GAMEDIR/$BINARY" ]; then
  echo "[goldbox] this port has no build for this device ($DEVICE_ARCH)"
  pm_finish
  exit 1
fi
$ESUDO chmod +x "$GAMEDIR/$BINARY" "$GAMEDIR"/tools/*.sh 2>/dev/null

# The port reads the controller itself; gptokeyb only gives the firmware's
# Select + Start quit
$GPTOKEYB "$BINARY" &
pm_platform_helper "$GAMEDIR/$BINARY"

./"$BINARY" --games "$GAMEDIR/games" --conf "$CONFDIR" --tools "$GAMEDIR/tools" $GOLDBOX_ARGS
echo "[goldbox] the port ended with code $?"

pm_finish

# "Restart Now" after an update was downloaded: straight into it
if [ -z "$GOLDBOX_SKIP_UPDATE" ] && [ -f "$UPD/READY" ] && [ -f "$GAMEDIR/restart" ]; then
  rm -f "$GAMEDIR/restart"
  exec /bin/bash "$PORTSDIR/$SCRIPT" "$@"
fi
rm -f "$GAMEDIR/restart"
exit 0

} # (end of the read-it-all brace)
