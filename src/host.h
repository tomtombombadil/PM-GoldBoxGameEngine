// The platform layer for PortMaster handhelds (and any Linux PC): what the
// engine's game code (cyd/src/engine, cyd/src/game) asks of the machine -
// cyd/src/platform/*.h - done with stdio and SDL2.
//
// The engine's "card" is a folder: paths under /GOLDBOX are files under
// pm::games_root() (the port's games/ folder, where the player copies
// their own game folders). Names are matched without regard to case, as
// on the DOS / FAT disks the games were written for - a handheld's ports
// partition is often ext4, where "SAVE" and "Save" differ.
#pragma once

#include <string>

namespace pm {

// The folder that stands for /GOLDBOX
std::string& games_root();
// The host path of a card path (each part matched ignoring case when the
// exact name isn't there)
std::string host_path(const char* card_path);

// Sound: opens SDL's audio device (false: none - the game runs silent)
bool audio_begin();
void audio_end();

} // namespace pm
