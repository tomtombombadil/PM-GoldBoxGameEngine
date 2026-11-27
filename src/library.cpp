#include "library.h"

#include "game/game_find.h"
#include "platform/file.h"
#include "host.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <strings.h>

namespace library {

namespace {

// The games folder's own folders, by name (one listing - none looked into)
bool top_folders(std::vector<std::string>& folders)
{
    folders.clear();
    plat::File root = plat::open(games::kRootDir);
    if (!root || !root.isDirectory()) return false;
    for (plat::File d = root.openNextFile(); d; d = root.openNextFile()) {
        if (d.isDirectory()) folders.push_back(d.name());
        d.close();
    }
    root.close();
    std::sort(folders.begin(), folders.end());
    return true;
}

std::string kept_path() { return std::string(games::kRootDir) + "/" + kCacheDir + "/GAMES.TXT"; }

// What a scan found, kept for the starts after it (Tom, 2026-11-27: "there should be NO scanning when the app
// opens. It should all already be prepared and waiting") - the folders it saw, then each game: its folder, where
// its files are, which game, HLIB or not, how many files
void keep(const std::vector<std::string>& folders, const std::vector<GameDir>& found)
{
    plat::mkdir((std::string(games::kRootDir) + "/" + kCacheDir).c_str());
    FILE* f = fopen(pm::host_path(kept_path().c_str()).c_str(), "w");
    if (!f) return;
    fprintf(f, "games 1\n");
    for (const std::string& n : folders) fprintf(f, "top\t%s\n", n.c_str());
    for (const GameDir& g : found)
        fprintf(f, "game\t%s\t%s\t%d\t%d\t%d\n", g.folder.c_str(), g.data_dir.c_str(), static_cast<int>(g.game),
                g.hlib ? 1 : 0, g.files);
    fclose(f);
}

} // namespace

bool scan_kept(std::vector<GameDir>& out)
{
    std::vector<std::string> folders, seen;
    if (!top_folders(folders)) {
        out.clear();
        return false;
    }
    FILE* f = fopen(pm::host_path(kept_path().c_str()).c_str(), "r");
    if (!f) return scan(out);
    std::vector<GameDir> got;
    bool good = false;
    char line[600];
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strcmp(line, "games 1") == 0) good = true;
        else if (strncmp(line, "top\t", 4) == 0) seen.push_back(line + 4);
        else if (strncmp(line, "game\t", 5) == 0) {
            // folder \t data_dir \t game \t hlib \t files
            char* fld[5] = {};
            char* p = line + 5;
            for (int i = 0; i < 5 && p; ++i) {
                fld[i] = p;
                p = strchr(p, '\t');
                if (p) *p++ = 0;
            }
            if (!fld[4]) {
                good = false;
                break;
            }
            GameDir g;
            g.folder = fld[0];
            g.data_dir = fld[1];
            g.game = static_cast<games::Game>(atoi(fld[2]));
            g.hlib = atoi(fld[3]) != 0;
            g.files = atoi(fld[4]);
            got.push_back(g);
        }
    }
    fclose(f);
    std::sort(seen.begin(), seen.end());
    // (a game folder added or taken away, or a game's files gone: looked for again)
    bool same = good && seen == folders;
    for (size_t i = 0; same && i < got.size(); ++i)
        same = plat::exists((std::string(games::kRootDir) + "/" + got[i].data_dir).c_str());
    if (!same) return scan(out);
    out = got;
    return true;
}

bool scan(std::vector<GameDir>& out)
{
    out.clear();
    std::vector<std::string> folders;
    if (!top_folders(folders)) return false;
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
    keep(folders, out);
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
