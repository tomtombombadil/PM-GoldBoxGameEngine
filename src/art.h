// The game list's pictures, as the CYD engine's library has them (Tom,
// 2026-11-14: "that ALL gets pre-processed during the initial game
// discovery ... no loading or processing screens"): MADE once by the scan
// from the player's own files and saved in the game's _CYD folder; the
// game list only reads that file. Nothing of them is in the port.
//
// The picture: the game's box art from the Steam / SNEG collection's
// Launcher folder (games/Launcher), else the game's own logo - the icon in
// its folder's goggame-<id>.ico / .dll (GOG) or game.ico (Steam), found
// and drawn by the engine's icon reader - centred above the name.
#pragma once

#include <cstdint>
#include <string>

#include "engine/games.h"

#include "library.h"

namespace art {

// How the page wants it: video::kBackW wide and `rows` tall; a logo within
// the top `icon_rows`
struct Want {
    int rows = 0, icon_rows = 0;
};

// The scan's step: decodes the picture and saves it (ART.BIN in cache_dir;
// with no picture to be had, a file that says so). Returns 2 box art,
// 1 logo, 0 none.
int make(const library::GameDir& g, const std::string& cache_dir, Want w);
// The saved file is this screen's (its shape, this size) and what the
// folders now offer (a Launcher folder copied in since: not ready)
bool ready(const library::GameDir& g, const std::string& cache_dir, Want w);
// The saved picture (0xAARRGGBB, rows x video::kBackW), read from its
// file - nothing is decoded; nullptr: none. Kept until the next call for
// another game.
const uint32_t* load(const library::GameDir& g, const std::string& cache_dir, Want w);

} // namespace art
