// The game list's pictures: each game's box art from the player's own copy
// of the Steam / SNEG collection's Launcher folder (games/Launcher - as the
// CYD engine's library uses it), read at a quarter of its size by the
// engine's own JPEG reader and fitted to the list's page. Nothing of it is
// in the port.
#pragma once

#include <cstdint>

#include "engine/games.h"

namespace art {

// The game's picture, video::kBackW wide and `rows` tall (0xAARRGGBB), its
// top kept; nullptr: none (no Launcher folder, or a file it can't read).
// Kept until the next call for another game or size.
const uint32_t* box(games::Game g, int rows);

} // namespace art
