// The cluebooks' maps on the Companion (Tom, 2026-11-27: "a toggle in the Enhancement that enables the Cluebook's
// map images ... I love the fact that you can accurately place the party's arrow on the image of the map"): made by
// the scan from the player's own cluebook PDF (the engine's cluebook::make - each known edition's maps cut out and
// straightened, the area's name read), kept in _CYD/<folder>/ as CM_<geo>_<block>.IMG and CLUEMAPS.TXT; the
// Companion only reads them.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "library.h"

namespace cluemaps {

using Say = void (*)(const char* line, bool replace, void* ctx);

// The scan's step: the game's cluebook found (its folder, Documentation) and its maps made; its lines through say
// in the scan's layout. CLUEMAPS.TXT is written whatever comes of it, so a start doesn't try again.
void make(const library::GameDir& g, const std::string& cache, Say say);
// The game's cluebook PDF (its folder, Documentation - jour::documents'; the host's path), "" when none
std::string find(const library::GameDir& g);
// Without making anything: the list that says the game has no cluebook (so a start doesn't look again)
void none(const std::string& cache);
// The step done for this game (by this version's scan or one since)
bool ready(const std::string& cache);

// A map's picture: square pixels a square, half a square round the grid, RGB888
struct Picture {
    int square = 0, w = 0, h = 0;
    std::vector<uint8_t> rgb;
};
// The picture of map (file, block) of the game whose cache this is - nullptr: none (kept: the last one asked)
const Picture* picture(const std::string& cache, const char* file, int block);
// The area's name the cluebook prints for that map ("" when none)
std::string name(const std::string& cache, const char* file, int block);

} // namespace cluemaps
