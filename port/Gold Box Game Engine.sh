#!/bin/bash
# PortMaster launch script: Gold Box Game Engine

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

GAMEDIR="/$directory/ports/goldbox"
CONFDIR="$GAMEDIR/conf"
BINARY="goldbox.${DEVICE_ARCH}"

# The player's own game folders go in games/; settings in conf/
mkdir -p "$CONFDIR" "$GAMEDIR/games"
cd "$GAMEDIR"

# Log this run (overwritten each launch)
> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1

export LD_LIBRARY_PATH="$GAMEDIR/libs.${DEVICE_ARCH}:$LD_LIBRARY_PATH"
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

if [ ! -f "$GAMEDIR/$BINARY" ]; then
  echo "This port has no build for this device ($DEVICE_ARCH)."
  pm_finish
  exit 1
fi
$ESUDO chmod +x "$GAMEDIR/$BINARY"

# The port reads the controller itself; gptokeyb only gives the firmware's
# Select + Start quit
$GPTOKEYB "$BINARY" &
pm_platform_helper "$GAMEDIR/$BINARY"

./"$BINARY" --games "$GAMEDIR/games" --conf "$CONFDIR"

pm_finish
