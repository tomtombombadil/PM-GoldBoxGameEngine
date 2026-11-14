// The game list's pictures: each game's box art from the player's own copy
// of the Steam / SNEG collection's Launcher folder (games/Launcher - as the
// CYD engine's library uses it), read at a quarter of its size by the
// engine's own JPEG reader and fitted to the list's page. Nothing of it is
// in the port.
#pragma once

#include <cstdint>

#include "engine/games.h"

#include "library.h"

namespace art {

// The game's picture, video::kBackW wide and `rows` tall (0xAARRGGBB), its
// top kept; nullptr: none (no Launcher folder, or a file it can't read).
// Kept until the next call for another game or size.
const uint32_t* box(games::Game g, int rows);

// A game without box art: its own logo - the icon in the game folder's
// goggame-<id>.ico / .dll (GOG) or game.ico (Steam), found and drawn by
// the engine's icon reader as the CYD's library does (Tom, 2026-11-14) -
// as big as fits in the top `icon_rows` of a black picture video::kBackW
// wide and `rows` tall, centred, square on the screen. nullptr: no icon.
const uint32_t* icon(const library::GameDir& g, int rows, int icon_rows);

} // namespace art
