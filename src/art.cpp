#include "art.h"

#include "engine/icon.h"
#include "engine/inflate.h"
#include "engine/jpeg_thumb.h"
#include "platform/file.h"

#include "video.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <strings.h>
#include <vector>

namespace art {
namespace {

std::vector<uint32_t> pixels;
games::Game           have = games::Game::Unknown;
int                   have_rows = 0, have_aspect = 0;
bool                  have_ok = false;

struct Ctx {
    jthumb::Fit fit;
    int         rows;       // of the picture kept
    int         tall;       // of the picture fitted (rows as the screen will stretch them)
    int         last = -1;
};

// A fitted row: into the kept row it lands on (two on one: their average)
bool out_row(int y, const uint8_t* rgb, int w, void* ctx)
{
    Ctx* c = static_cast<Ctx*>(ctx);
    const int to_y = y * c->rows / c->tall;
    if (y < 0 || to_y >= c->rows) return true;
    uint32_t* to = pixels.data() + static_cast<size_t>(to_y) * video::kBackW;
    const bool mix = to_y == c->last;
    c->last = to_y;
    for (int x = 0; x < w && x < video::kBackW; ++x) {
        uint32_t r = rgb[x * 3], g = rgb[x * 3 + 1], b = rgb[x * 3 + 2];
        if (mix) {
            r = (r + ((to[x] >> 16) & 0xFF)) / 2;
            g = (g + ((to[x] >> 8) & 0xFF)) / 2;
            b = (b + (to[x] & 0xFF)) / 2;
        }
        to[x] = 0xFF000000u | (r << 16) | (g << 8) | b;
    }
    return true;
}

bool in_row(int, const uint8_t* rgb, int, void* ctx) { return static_cast<Ctx*>(ctx)->fit.add(rgb); }

// ---- The icon --------------------------------------------------------------
std::vector<uint32_t> ipixels;
std::string           ihave;
int                   ihave_rows = 0, ihave_aspect = 0;
bool                  ihave_ok = false;

bool ends_with(const char* s, const char* ext)
{
    const size_t n = strlen(s), m = strlen(ext);
    return n >= m && strcasecmp(s + n - m, ext) == 0;
}

// Which file to take the icon from (the CYD library's order): GOG's own
// .ico, then its .dll, then any other .ico but GOG's Support.ico
int icon_rank(const char* name)
{
    const bool gog = strncasecmp(name, "goggame-", 8) == 0;
    if (ends_with(name, ".ICO")) {
        if (gog) return 3;
        return strcasecmp(name, "Support.ico") == 0 ? 0 : 1;
    }
    if (gog && ends_with(name, ".DLL")) return 2;
    return 0;
}

void find_icon(const std::string& rel, std::string& out, int& best)
{
    const std::string dir = std::string(games::kRootDir) + "/" + rel;
    plat::File d = plat::open(dir.c_str());
    if (!d || !d.isDirectory()) return;
    for (plat::File f = d.openNextFile(); f; f = d.openNextFile()) {
        const char* nm = f.name();
        if (const char* slash = strrchr(nm, '/')) nm = slash + 1;
        const int r = f.isDirectory() ? 0 : icon_rank(nm);
        if (r > best) {
            best = r;
            out = dir + "/" + nm;
        }
        f.close();
    }
}

struct IconTo {
    int x, y;
};

// A row of the icon over the black picture (its see-through parts black)
void icon_row(int y, const uint8_t* rgba, int w, void* ctx)
{
    const IconTo* t = static_cast<const IconTo*>(ctx);
    const int to_y = t->y + y;
    if (to_y < 0 || to_y >= video::kBackH) return;
    uint32_t* to = ipixels.data() + static_cast<size_t>(to_y) * video::kBackW;
    for (int x = 0; x < w; ++x) {
        const int tx = t->x + x;
        if (tx < 0 || tx >= video::kBackW) continue;
        const uint32_t a = rgba[x * 4 + 3];
        to[tx] = 0xFF000000u | ((rgba[x * 4] * a / 255) << 16) | ((rgba[x * 4 + 1] * a / 255) << 8) | (rgba[x * 4 + 2] * a / 255);
    }
}

} // namespace

const uint32_t* box(games::Game g, int rows)
{
    const int aspect = video::pixel_aspect();
    if (g == have && rows == have_rows && aspect == have_aspect) return have_ok ? pixels.data() : nullptr;
    have = g;
    have_rows = rows;
    have_aspect = aspect;
    have_ok = false;
    const char* name = games::box_art_file(g);
    if (!name || rows <= 0 || rows > video::kBackH) return nullptr;
    char path[160];
    snprintf(path, sizeof path, "%s/%s/%s", games::kRootDir, games::kLauncherArt, name);
    plat::File f = plat::open(path);
    if (!f) return nullptr;
    plat::FileSource src(f);
    jthumb::Info info;
    if (!jthumb::probe(src, info)) return nullptr;
    pixels.assign(static_cast<size_t>(video::kBackW) * video::kBackH, 0xFF000000u);
    Ctx c;
    c.rows = rows;
    c.tall = rows * aspect / 1000;      // (shown stretched: fitted at the shape it will have, then squeezed)
    if (c.tall < rows) c.tall = rows;
    if (!c.fit.begin(info.out_w, info.out_h, video::kBackW, c.tall, out_row, &c, true)) return nullptr;
    have_ok = jthumb::decode(src, in_row, &c) && c.fit.finish();
    f.close();
    return have_ok ? pixels.data() : nullptr;
}

const uint32_t* icon(const library::GameDir& g, int rows, int icon_rows)
{
    const int aspect = video::pixel_aspect();
    if (g.folder == ihave && rows == ihave_rows && aspect == ihave_aspect) return ihave_ok ? ipixels.data() : nullptr;
    ihave = g.folder;
    ihave_rows = rows;
    ihave_aspect = aspect;
    ihave_ok = false;
    if (rows <= 0 || rows > video::kBackH || icon_rows < 32 || icon_rows > rows) return nullptr;
    std::string path;
    int best = 0;
    find_icon(g.folder, path, best);
    if (g.data_dir != g.folder) find_icon(g.data_dir, path, best);
    if (best == 0) return nullptr;
    plat::File f = plat::open(path.c_str());
    if (!f) return nullptr;
    plat::FileSource src(f);
    icon::Found found;
    if (!icon::find(src, found)) return nullptr;
    // Square on the screen: the picture's rows are shown `aspect` / 1000 tall
    int h = icon_rows - 16, w = h * aspect / 1000;
    if (w > icon::kMaxOut) {
        w = icon::kMaxOut;
        h = w * 1000 / aspect;
    }
    if (h > icon::kMaxOut) h = icon::kMaxOut;
    ipixels.assign(static_cast<size_t>(video::kBackW) * video::kBackH, 0xFF000000u);
    std::vector<uint8_t> window(inflate::kWindow);
    IconTo to{(video::kBackW - w) / 2, (icon_rows - h) / 2};
    ihave_ok = icon::render(src, found, w, h, window.data(), icon_row, &to);
    f.close();
    return ihave_ok ? ipixels.data() : nullptr;
}

} // namespace art
