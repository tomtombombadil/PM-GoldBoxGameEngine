#include "art.h"

#include "engine/jpeg_thumb.h"
#include "platform/file.h"

#include "video.h"

#include <cstdio>
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

} // namespace art
