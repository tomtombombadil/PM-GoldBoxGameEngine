#include "library.h"

#include "platform/file.h"

#include <algorithm>
#include <cstring>
#include <strings.h>

namespace library {
namespace {

bool ends_with(const char* s, const char* ext)
{
    const size_t n = strlen(s), m = strlen(ext);
    return n >= m && strcasecmp(s + n - m, ext) == 0;
}
bool is_dax(const char* n) { return ends_with(n, ".DAX"); }
bool is_hlib(const char* n) { return ends_with(n, ".TLB") || ends_with(n, ".GLB"); }

std::string card(const std::string& rel) { return std::string(games::kRootDir) + "/" + rel; }

// A folder's entries: its sub-folders' names, and how many of its files
// pass `want`
int list(const std::string& rel, bool (*want)(const char*), std::vector<std::string>* dirs)
{
    plat::File d = plat::open(card(rel).c_str());
    if (!d || !d.isDirectory()) return 0;
    int n = 0;
    for (plat::File f = d.openNextFile(); f; f = d.openNextFile()) {
        if (f.isDirectory()) {
            if (dirs) dirs->push_back(f.name());
        } else if (want(f.name())) {
            ++n;
        }
        f.close();
    }
    return n;
}

// The first folder exactly `depth` levels below rel holding .DAX files
int dax_at_depth(const std::string& rel, int depth, std::string& where)
{
    std::vector<std::string> dirs;
    const int n = list(rel, is_dax, &dirs);
    if (depth == 0) {
        if (n > 0) where = rel;
        return n;
    }
    std::sort(dirs.begin(), dirs.end());
    for (const auto& s : dirs) {
        const int k = dax_at_depth(rel + "/" + s, depth - 1, where);
        if (k > 0) return k;
    }
    return 0;
}

int count_hlib(const std::string& rel, int depth)
{
    std::vector<std::string> dirs;
    int n = list(rel, is_hlib, &dirs);
    if (depth > 0)
        for (const auto& s : dirs) n += count_hlib(rel + "/" + s, depth - 1);
    return n;
}

// The first folder up to `depth` levels below rel holding a DISK1 folder
bool disk_root(const std::string& rel, int depth, std::string& where)
{
    if (plat::exists(card(rel + "/DISK1").c_str())) {
        where = rel;
        return true;
    }
    if (depth == 0) return false;
    std::vector<std::string> dirs;
    list(rel, is_dax, &dirs);
    std::sort(dirs.begin(), dirs.end());
    for (const auto& s : dirs)
        if (disk_root(rel + "/" + s, depth - 1, where)) return true;
    return false;
}

const char* base_name(const std::string& p)
{
    const size_t k = p.rfind('/');
    return k == std::string::npos ? p.c_str() : p.c_str() + k + 1;
}

} // namespace

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
        // (the engine's own folder; the Steam / SNEG collection's Launcher
        // and Gold Box Companion folders hold no game)
        if (strcasecmp(name.c_str(), kCacheDir) == 0 || strcasecmp(name.c_str(), "Launcher") == 0 ||
            strcasecmp(name.c_str(), "GBC") == 0)
            continue;
        GameDir g;
        g.folder = name;
        g.data_dir = name;
        // A whole install copied as it is: the game files may sit one
        // folder down (GOG) or two (Steam / SNEG: <folder>/GAME/<SHORT>);
        // the shallowest folder with them wins
        int n = 0;
        for (int depth = 0; n == 0 && depth <= 2; ++depth) n = dax_at_depth(name, depth, g.data_dir);
        if (n == 0) {
            n = count_hlib(name, 3);
            g.hlib = true;
            if (n > 0) disk_root(name, 2, g.data_dir);
        }
        if (n == 0) continue;
        g.files = n;
        g.game = games::from_folder_name(name.c_str());
        if (g.game == games::Game::Unknown && g.data_dir != name) g.game = games::from_folder_name(base_name(g.data_dir));
        if (g.game == games::Game::UnlimitedAdventures) continue;       // (not one of the engine's games)
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
