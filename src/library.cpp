#include "library.h"

#include "game/game_find.h"
#include "platform/file.h"

#include <algorithm>
#include <strings.h>

namespace library {

bool scan(std::vector<GameDir>& out)
{
    out.clear();
    plat::File root = plat::open(games::kRootDir);
    if (!root || !root.isDirectory()) return false;
    std::vector<std::string> folders;
    for (plat::File d = root.openNextFile(); d; d = root.openNextFile()) {
        if (d.isDirectory()) folders.push_back(d.name());
        d.close();
    }
    root.close();
    for (const auto& name : folders) {
        // Where its game files are: the engine's own rules (cyd/src/game/game_find.h, the boards' scan's too)
        if (game_find::passed_over(name.c_str())) continue;
        game_find::Found f;
        if (game_find::look(name.c_str(), f) != game_find::Look::Game) continue;
        GameDir g;
        g.folder = name;
        g.data_dir = f.data_dir;
        g.hlib = f.hlib;
        g.files = f.files;
        g.game = f.game;
        out.push_back(g);
    }
    std::sort(out.begin(), out.end(), [](const GameDir& a, const GameDir& b) {
        const int oa = games::list_order(a.game), ob = games::list_order(b.game);
        return oa != ob ? oa < ob : strcasecmp(a.folder.c_str(), b.folder.c_str()) < 0;
    });
    return true;
}

std::string cache_dir(const GameDir& g)
{
    const std::string top = std::string(games::kRootDir) + "/" + kCacheDir;
    plat::mkdir(top.c_str());
    const std::string dir = top + "/" + g.folder;
    plat::mkdir(dir.c_str());
    return dir;
}

} // namespace library
