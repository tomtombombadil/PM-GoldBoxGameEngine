// The player's games: the folders under games/ (the engine's /GOLDBOX)
// holding a Gold Box game's original DOS files. The same rules as the CYD
// engine's card scan: cyd/src/game/game_find.h, shared with the boards.
#pragma once

#include <string>
#include <vector>

#include "engine/games.h"

namespace library {

struct GameDir {
    std::string folder;         // the folder under games/
    std::string data_dir;       // where its game files are, relative to games/ (the folder itself,
                                // or one or two folders inside it - a whole GOG / Steam install
                                // copied as it is); an HLIB game (The Dark Queen of Krynn): the
                                // folder holding DISK1-3
    games::Game game = games::Game::Unknown;
    bool        hlib = false;   // .TLB / .GLB libraries instead of .DAX files
    int         files = 0;      // game data files found
};

// The engine's own folder inside games/: what it makes from the player's
// files (the name is the engine's - its game code writes there too)
constexpr const char* kCacheDir = "_CYD";

// The game folders, in the engine's library order. False: there is no
// games folder at all.
bool scan(std::vector<GameDir>& out);

// "/GOLDBOX/_CYD/<folder>" (made if missing)
std::string cache_dir(const GameDir& g);

} // namespace library
