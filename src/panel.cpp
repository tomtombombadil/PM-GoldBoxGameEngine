#include "panel.h"

#include <functional>

#include "cluemaps.h"
#include "engine/cluebook.h"

#include "engine/geo.h"
#include "game/companion.h"
#include "game/play.h"

#include "shell.h"
#include "video.h"

#include <cstdio>
#include <cstring>

namespace panel {
namespace {

constexpr int kVW = 156;                    // the engine's views' width (6-wide cells)
constexpr int kW = kVW * 4 / 3;             // on the screen: 208
constexpr int kH = video::kH;

enum class View : uint8_t { None, Map, Party, Battle };
View     view = View::None;
uint32_t buf[2][kW * kH];                   // the panel (0), drawn only when it changes; 1: the party under it, beside the game
uint32_t below[video::kBelowW * video::kBelowH];   // under the game on a square panel: map / battle | party
uint32_t* pixels = buf[0];                  // the one being drawn
int      pw = kW, ph = kH;                  // (its size)
uint32_t sig = 0, sig2 = 0;
bool     side_wanted = true;                // beside the game on a wide panel (Y: off / on)
int      map_rows = kH;                     // the map's picture: its rows in use from the top (to the legend)
bool     side_shown = false;
bool     held = false;                      // the game's title screens: no Companion
float    shown = 0;                         // how far out it is: 0 closed, 1 fully
uint32_t last_ms = 0;

constexpr uint32_t rgb(uint16_t c)
{
    return 0xFF000000u | (static_cast<uint32_t>((c >> 11) * 255 / 31) << 16) |
           (static_cast<uint32_t>(((c >> 5) & 63) * 255 / 63) << 8) | static_cast<uint32_t>((c & 31) * 255 / 31);
}

void put(int x, int y, int w, int h, uint32_t c)
{
    for (int j = y < 0 ? 0 : y; j < y + h && j < ph; ++j)
        for (int i = x < 0 ? 0 : x; i < x + w && i < pw; ++i) pixels[j * pw + i] = c;
}

void letters(int x, int y, const char* s, uint32_t c)
{
    for (; *s; ++s, x += 8) {
        const uint8_t* g = video::glyph(*s);
        for (int j = 0; j < 8; ++j)
            for (int i = 0; i < 8; ++i)
                if (g[j] & (0x80 >> i)) put(x + i, y + j, 1, 1, c);
    }
}

// The engine's views draw through this: across stretched by 4 / 3 (its 6-wide
// text cells become 8-pixel letters) - from `knee` on; left of it as it is
// (square pixels under the game: the party's icons keep their shape), all
// of it `off` to the right
struct Paint : companion::Painter {
    int knee = 0, off = 0;
    int sx(int x) const { return off + (x <= knee ? x : knee + (x - knee) * 4 / 3); }
    void fill(int x, int y, int w, int h, uint16_t c) override { put(sx(x), y, sx(x + w) - sx(x), h, rgb(c)); }
    void text(int x, int y, const char* s, uint16_t c) override { letters(sx(x), y, s, rgb(c)); }
    uint16_t colour(int value) override
    {
        const uint32_t p = video::palette(value);
        return static_cast<uint16_t>((((p >> 16) & 0xF8) << 8) | (((p >> 8) & 0xFC) << 3) | ((p & 0xFF) >> 3));
    }
};

constexpr uint32_t kPlace = 0xFFFF8C00;       // (the CYD's style::kPlace)

View first() { return play::fighting() ? View::Battle : View::Map; }

// The auto-map: walls a pixel wide, doors a band on the wall line (gold;
// red: locked; a one-way door half of one and an arrow), a secret door a cyan S, the party a green arrow (the CYD's
// colours and legend - Tom, 2026-11-14: no heading, the words alone).
// In the zone zx, zy, zw x zh. square: the buffer's pixels are square
// (under the game; the legend to the map's right) - else squares 12 x 10,
// about square once a 4:3 screen has stretched the rows, the legend under.
std::string clue_cache;
bool clue_on = false;
bool clue_dark = true;

// The party's arrowhead in a square at (sx, sy), cw x ch: fitted between its walls a pixel clear of each (Tom,
// 2026-11-27: balanced every way round) - its tip two pixels wide where the room across is even
void party_arrow(int sx, int sy, int cw, int ch, uint32_t col)
{
    const int dir = play::dir(), fx = geo::dx(dir), fy = geo::dy(dir);
    const int na = (fx ? cw : ch) - 1, np = (fx ? ch : cw) - 1, len = na - 2;
    const int base = np % 2 == 0 ? 2 : 1;
    for (int i = 0; i < len; ++i) {
        int wd = base + 2 * (i / 2);
        if (wd > np - 2) wd = np - 2;
        if (wd < 1) wd = 1;
        const int a = 1 + (np - wd) / 2, along = (fx > 0 || fy > 0) ? na - 1 - i : 2 + i;
        if (fx) put(sx + along, sy + a, 1, wd, col);
        else put(sx + a, sy + along, wd, 1, col);
    }
}

// The cluebook's own picture of the map (Cluebook Maps on and the scan made one for this map): as large as the zone
// lets it be, the area's name above it, the party's arrow on its square. False: none - the drawn map then.
bool draw_clue_map(int zx, int zy, int zw, int zh, bool square)
{
    if (!clue_on || clue_cache.empty()) return false;
    char file[16];
    const int block = play::map_source(file, sizeof file);
    if (block < 0) return false;
    const cluemaps::Picture* pic = cluemaps::picture(clue_cache, file, block);
    if (!pic) return false;
    const std::string name = cluemaps::name(clue_cache, file, block);
    const int head = name.empty() ? 0 : 11;
    // (beside the game the buffer's pixels are 1.2 times as tall as wide on the screen - the drawn map's 12 x 10
    // squares: the picture 1.2 times as wide as tall here; under the game the pixels are square)
    const double stretch = square ? 1.0 : 1.2;
    int ph = square ? zh - 4 - head : zh - 6 - head;
    if ((square ? zw - 4 : zw - 6) < ph * stretch) ph = static_cast<int>((square ? zw - 4 : zw - 6) / stretch);
    const int pw = static_cast<int>(ph * stretch);
    if (ph < 40) return false;
    const int mx = square ? zx + 2 : zx + (zw - pw) / 2, my = zy + 2 + head;
    if (head) letters(square ? zx + 2 : zx + 3, zy + 2, name.substr(0, (square ? pw : zw - 6) / 8).c_str(), video::kGold);
    // (Dark: a book printed on light paper shown light on dark - each colour's lightness turned over, its hue kept;
    // a book printed light on dark - Steam Gateway's - as it is)
    bool flip = false;
    if (clue_dark) {
        uint64_t sum = 0;
        for (size_t i = 0; i < pic->rgb.size(); i += 3 * 7) sum += (pic->rgb[i] * 3 + pic->rgb[i + 1] * 6 + pic->rgb[i + 2]) / 10;
        flip = sum * 3 * 7 / pic->rgb.size() > 128;
    }
    // (each screen pixel the mean of the picture's pixels under it)
    const double kx = static_cast<double>(pic->w) / pw, ky = static_cast<double>(pic->h) / ph;
    for (int y = 0; y < ph; ++y) {
        const int y0 = static_cast<int>(y * ky), y1 = static_cast<int>((y + 1) * ky) > y0 ? static_cast<int>((y + 1) * ky) : y0 + 1;
        for (int x = 0; x < pw; ++x) {
            const int x0 = static_cast<int>(x * kx), x1 = static_cast<int>((x + 1) * kx) > x0 ? static_cast<int>((x + 1) * kx) : x0 + 1;
            uint32_t r = 0, g = 0, b = 0, n = 0;
            for (int j = y0; j < y1 && j < pic->h; ++j)
                for (int i = x0; i < x1 && i < pic->w; ++i) {
                    const uint8_t* q = &pic->rgb[(static_cast<size_t>(j) * pic->w + i) * 3];
                    r += q[0];
                    g += q[1];
                    b += q[2];
                    ++n;
                }
            if (!n) continue;
            int cr = static_cast<int>(r / n), cg = static_cast<int>(g / n), cb = static_cast<int>(b / n);
            if (flip) {
                const int l = (cr * 3 + cg * 6 + cb) / 10, shift = 255 - 2 * l;
                cr = cr + shift < 0 ? 0 : cr + shift > 255 ? 255 : cr + shift;
                cg = cg + shift < 0 ? 0 : cg + shift > 255 ? 255 : cg + shift;
                cb = cb + shift < 0 ? 0 : cb + shift > 255 ? 255 : cb + shift;
            }
            put(mx + x, my + y, 1, 1, 0xFF000000u | static_cast<uint32_t>(cr) << 16 | static_cast<uint32_t>(cg) << 8 | static_cast<uint32_t>(cb));
        }
    }
    // The party: its square on the straightened grid (half a square round it)
    const double sqx = pic->square / kx, sqy = pic->square / ky;
    const int ax = mx + static_cast<int>(cluebook::kMargin * sqx + play::pos_x() * sqx),
              ay = my + static_cast<int>(cluebook::kMargin * sqy + play::pos_y() * sqy);
    const int cw = static_cast<int>(sqx + 0.5) + 1, ch = static_cast<int>(sqy + 0.5) + 1;
    // (dark under the green so it shows on the paper)
    party_arrow(ax - 1, ay, cw, ch, 0xFF004000u);
    party_arrow(ax + 1, ay, cw, ch, 0xFF004000u);
    party_arrow(ax, ay - 1, cw, ch, 0xFF004000u);
    party_arrow(ax, ay + 1, cw, ch, 0xFF004000u);
    party_arrow(ax, ay, cw, ch, 0xFF28FF28u);
    map_rows = my + ph + 2;
    return true;
}

void draw_map(int zx, int zy, int zw, int zh, bool square)
{
    put(zx, zy, zw, zh, video::kBlack);
    const geo::Map* m = play::map();
    if (!m) {
        letters(zx + 8, zy + (square ? 8 : 40), "No map here.", video::kCream);
        map_rows = zy + 52;
        return;
    }
    if (draw_clue_map(zx, zy, zw, zh, square)) return;
    // (the area's name from the cluebook above the drawn map too)
    std::string area;
    if (!clue_cache.empty()) {
        char file[16];
        const int block = play::map_source(file, sizeof file);
        if (block >= 0) area = cluemaps::name(clue_cache, file, block);
    }
    const int head = area.empty() ? 0 : 11;
    if (head) {
        letters(zx + 3, zy + 2, area.substr(0, (zw - 6) / 8).c_str(), video::kGold);
        zy += head;
        zh -= head;
    }
    int cw = square ? (zh - 6) / m->h : (zw - 10) / m->w, ch = square ? cw : (zh - 24) / m->h;
    const int cap_w = square ? 9 : 12, cap_h = square ? 9 : 10;
    if (cw > cap_w) cw = cap_w;
    if (ch > cap_h) ch = cap_h;
    if (cw < 2) cw = 2;
    if (ch < 2) ch = 2;
    const int mx = square ? zx + 4 : zx + (zw - cw * m->w) / 2, my = zy + (square ? (zh - ch * m->h) / 2 : 4);
    const uint16_t plain = geo::plain_walls(*m);
    static const uint8_t kS[7] = {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E};
    for (int pass = 0; pass < 2; ++pass)
        for (int y = 0; y < m->h; ++y)
            for (int x = 0; x < m->w; ++x) {
                const int sx = mx + x * cw, sy = my + y * ch;
                for (int d = 0; d < 8; d += 2) {
                    const int w = geo::wall(*m, x, y, d);
                    if (!w) continue;
                    const int door = geo::door(*m, x, y, d);
                    if ((door != 0) != (pass == 1)) continue;
                    const bool across = d == 0 || d == 4;
                    const int lx = d == 2 ? sx + cw : sx, ly = d == 4 ? sy + ch : sy;
                    if (across) put(lx, ly, cw + 1, 1, video::kCream);
                    else put(lx, ly, 1, ch + 1, video::kCream);
                    if (!door) continue;
                    // (the map look, Tom 2026-11-27: every door leaves a little wall at each end; L the side's
                    // length, P the square's depth across it)
                    const int L = across ? cw : ch, P = across ? ch : cw, q = L / 4, n = L - 2 * q + 1;
                    if (play::map_arch(x, y, d)) {
                        // An archway (the engine's play::map_arch - its wall's picture see-through): an opening,
                        // a serif across the line where each piece of wall ends, a pixel of wall clear of the
                        // corner (Tom, 2026-11-27); too small for that, a plain opening
                        const int a = (L + 3) / 4, g = L - 2 * a + 1;
                        if (a < 3 || g < 2) {
                            if (across) put(lx + q, ly, n, 1, video::kBlack);
                            else put(lx, ly + q, 1, n, video::kBlack);
                        } else if (across) {
                            put(lx + a, ly, g, 1, video::kBlack);
                            put(lx + a - 1, ly - 1, 1, 3, video::kCream);
                            put(lx + a + g, ly - 1, 1, 3, video::kCream);
                        } else {
                            put(lx, ly + a, 1, g, video::kBlack);
                            put(lx - 1, ly + a - 1, 3, 1, video::kCream);
                            put(lx - 1, ly + a + g, 3, 1, video::kCream);
                        }
                        continue;
                    }
                    // A one-way door (Tom, 2026-11-27): passable from this square, the same wall solid from the
                    // next - half a door on this side, an arrow into the next square pointing away from the wall
                    const int nx = x + geo::dx(d), ny = y + geo::dy(d), back = (d + 4) & 7;
                    const bool oneway = nx >= 0 && ny >= 0 && nx < m->w && ny < m->h && geo::wall(*m, nx, ny, back) &&
                                        !geo::door(*m, nx, ny, back);
                    // (which side of the line this square is: +1 below / right of it, -1 above / left)
                    const int here = d == 0 || d == 6 ? 1 : -1;
                    auto arrow = [&]() {
                        if (!oneway || P < 7) return;
                        const int len = P / 2 > 3 ? P / 2 : 3, mid = L / 2, out = -here;
                        for (int k = 0; k < len; ++k) {
                            const int t = 2 + k;
                            if (across) put(lx + mid, ly + out * t, 1, 1, video::kGold);
                            else put(lx + out * t, ly + mid, 1, 1, video::kGold);
                        }
                        for (int k = 1; k <= 2; ++k) {
                            const int t = 1 + len - k;
                            if (across) {
                                put(lx + mid - k, ly + out * t, 1, 1, video::kGold);
                                put(lx + mid + k, ly + out * t, 1, 1, video::kGold);
                            } else {
                                put(lx + out * t, ly + mid - k, 1, 1, video::kGold);
                                put(lx + out * t, ly + mid + k, 1, 1, video::kGold);
                            }
                        }
                    };
                    if (door == 1 && (plain >> w & 1)) {
                        // Drawn as the area's wall: a secret door only where the game lets the party through
                        // (the engine's play::map_secret, 0.214.2 - before it is found; the rest is wall). The
                        // S turned on a wall running up and down (Tom, 2026-11-27)
                        if (!play::map_secret(x, y, d) || cw < 8) continue;
                        for (int r = 0; r < 7; ++r)
                            for (int c = 0; c < 5; ++c)
                                if (kS[r] >> (4 - c) & 1) {
                                    if (across) put(lx + cw / 2 - 2 + c, ly - 3 + r, 1, 1, video::kCyan);
                                    else put(lx - 3 + (6 - r), ly + ch / 2 - 2 + c, 1, 1, video::kCyan);
                                }
                        arrow();
                        continue;
                    }
                    // A door: 3 pixels across the wall line, a little wall left at each end (locked: red - codes
                    // 2 and 3 alike, Tom); a one-way door the half on this square's side
                    const uint32_t col = door == 1 ? video::kGold : video::kWarn;
                    if (oneway) {
                        if (across) put(lx + q, here > 0 ? ly : ly - 1, n, 2, col);
                        else put(here > 0 ? lx : lx - 1, ly + q, 2, n, col);
                        arrow();
                    } else if (across) put(lx + q, ly - 1, n, 3, col);
                    else put(lx - 1, ly + q, 3, n, col);
                }
            }
    // The places (PoolRad's wilderness, engine 0.213.0: where its scripts have something): an orange square
    bool places = false;
    for (int y = 0; y < m->h; ++y)
        for (int x = 0; x < m->w; ++x)
            if (play::map_place(x, y)) {
                const int pw = cw / 2 > 2 ? cw / 2 : 2, ph = ch / 2 > 2 ? ch / 2 : 2;
                put(mx + x * cw + (cw - pw + 1) / 2, my + y * ch + (ch - ph + 1) / 2, pw, ph, kPlace);
                places = true;
            }
    party_arrow(mx + play::pos_x() * cw, my + play::pos_y() * ch, cw, ch, 0xFF28FF28u);
    if (square) {
        const int lx = mx + m->w * cw + 10;
        if (places) letters(lx, my, "Place", kPlace);
    } else {
        const int ly = my + m->h * ch + 6;
        // (no words for the doors - Tom, 2026-11-27: the symbols say it; an overland's places keep theirs)
        map_rows = places ? ly + 10 : my + m->h * ch + 3;
        if (places) letters(zx + 4, ly, "Place", kPlace);
    }
}

void redraw(View v, bool blink_on, bool edge)
{
    Paint p;
    const companion::Area a{0, 0, kVW, kH};
    if (v == View::Map) draw_map(0, 0, kW, kH, false);
    else if (v == View::Party) companion::draw_party(p, a, true, true);
    else companion::draw_battle(p, a, blink_on, false, true);
    if (edge) put(0, 0, 1, kH, video::kGold);
}

// What a view shows, as a number: it is drawn again only when this changes
uint32_t signature(View v, bool blink_on)
{
    uint32_t s = static_cast<uint32_t>(v) * 0x9E3779B1u;
    if (v == View::Map) {
        const geo::Map* m = play::map();
        s ^= static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m)) ^ static_cast<uint32_t>(play::pos_x() << 8) ^
             static_cast<uint32_t>(play::pos_y() << 16) ^ static_cast<uint32_t>(play::dir() << 24) ^ (m ? 1u : 0u);
        // (the cluebook's picture or the drawn map: Cluebook Maps, and which map it is)
        char file[16];
        s = s * 31 + (clue_on ? 7u : 3u) + (clue_dark ? 11u : 0u) + static_cast<uint32_t>(play::map_source(file, sizeof file)) * 131u +
            static_cast<uint32_t>(std::hash<std::string>()(clue_cache + file));
        // (a door opened shows too: the map's own bytes)
        if (m)
            for (int i = 0; i < m->squares(); ++i) s = s * 31 + m->pl(3)[i];
        // (the secret doors the engine has worked out, an overland's places)
        if (m)
            for (int y = 0; y < m->h; ++y)
                for (int x = 0; x < m->w; ++x) {
                    uint32_t b = play::map_place(x, y) ? 16u : 0u;
                    for (int d = 0; d < 8; d += 2) b |= play::map_secret(x, y, d) ? 1u << (d / 2) : 0u;
                    s = s * 31 + b;
                }
        // (Save Slot Info: the highlighted slot's map stands here - another slot, another map in the same place)
        if (m && play::saves_shown())
            for (int k = 0; k < 3; ++k)
                for (int i = 0; i < m->squares(); ++i) s = s * 31 + m->pl(k)[i];
    } else if (v == View::Party) {
        // (both: the icons' and the lines' - hit points, levels, the ready colour. Until v0.12.7 only the
        // icons' was asked, so a member trained kept its magenta name until something else redrew the view)
        s ^= play::panel_party_sig(true) * 31u ^ play::panel_party_sig(false);
    } else {
        s ^= play::panel_field_sig() ^ (companion::battle_blinks() && blink_on ? 0x55u : 0u);
    }
    return s | 1;
}

// Beside the game: the map / battlefield above, the party below
void draw_side(uint32_t now_ms)
{
    const bool blink_on = (now_ms / 400) & 1;
    const View top = first();
    const uint32_t s = signature(top, blink_on), s2 = signature(View::Party, blink_on);
    const bool fresh = !side_shown;
    if (fresh || s != sig) {
        sig = s;
        pixels = buf[0];
        redraw(top, blink_on, false);
    }
    if (fresh || s2 != sig2) {
        sig2 = s2;
        pixels = buf[1];
        redraw(View::Party, blink_on, false);
    }
    pixels = buf[0];
    side_shown = true;
    // (the rows the party's members take: the engine's Party view, rows of up to 26 from the top)
    const int n = play::panel_members();
    const int pitch = n ? ((kH - 1) / n > 26 ? 26 : (kH - 1) / n) : 0;
    // (the map to its legend; a fight's field stands at its picture's foot - all of it)
    video::set_side_used(top == View::Map ? map_rows : kH, n ? 1 + n * pitch : 12);
    // (the app lays the pictures out itself: what they hold - the map's rows, the party's members and their pitch,
    // a fight's field - Tom, 2026-11-25 evening)
    char comp[64];
    snprintf(comp, sizeof comp, "comp\t%d\t%d\t%d\t%d", top == View::Map ? map_rows : kH, n, pitch, top == View::Map ? 0 : 1);
    shell::text(shell::ChCompanion, comp);
    video::set_side(buf[0], buf[1]);
}

// Under the game (a square panel - Tom, 2026-11-15): the map (a fight: the
// battlefield) at the left, the party at the right, in square pixels
constexpr int kLeftW = 262, kPartyX = 268, kPartyKnee = 29;   // (the knee: past the party's icons)
void draw_below(uint32_t now_ms)
{
    const bool blink_on = (now_ms / 400) & 1;
    const View left = first();
    const uint32_t s = signature(left, blink_on), s2 = signature(View::Party, blink_on);
    const bool fresh = !side_shown;
    pixels = below;
    pw = video::kBelowW;
    ph = video::kBelowH;
    if (fresh) put(0, 0, pw, ph, video::kBlack);
    if (fresh || s != sig) {
        sig = s;
        if (left == View::Map) {
            draw_map(0, 0, kLeftW, ph, true);
        } else {
            put(0, 0, kLeftW, ph, video::kBlack);
            Paint p;
            companion::draw_battle(p, companion::Area{0, 0, kLeftW * 3 / 4, ph}, blink_on, false, true);
        }
        put(kLeftW + 2, 4, 1, ph - 8, 0xFF3C3A32u);
    }
    if (fresh || s2 != sig2) {
        sig2 = s2;
        Paint p;
        p.off = kPartyX;
        p.knee = kPartyKnee;
        const int w = kPartyKnee + (pw - kPartyX - kPartyKnee) * 3 / 4;
        companion::draw_party(p, companion::Area{0, 0, w, ph}, true, true);
    }
    pixels = buf[0];
    pw = kW;
    ph = kH;
    side_shown = true;
    video::set_below(below);
}

bool docked() { return video::side_room() || video::below_room(); }

} // namespace

bool open() { return view != View::None; }
bool docked_on() { return side_shown; }
void close() { view = View::None; }

void leave()
{
    close();
    shown = 0;
    if (side_shown) {
        video::set_side(nullptr, nullptr);
        video::set_below(nullptr);
    }
    side_shown = false;
}

void clue(const std::string& cache, bool on, bool dark)
{
    clue_cache = cache;
    clue_on = on;
    clue_dark = dark;
}

void hold(bool on)
{
    if (on && !held) close();
    held = on;
}

void next()
{
    if (held) return;
    sig = sig2 = 0;
    if (docked()) {
        side_wanted = !side_wanted;
        view = View::None;
        return;
    }
    if (view == View::None) view = first();
    else if (view == View::Party) view = View::None;
    else view = View::Party;
}

void draw(uint32_t now_ms)
{
    if (docked() && side_wanted && !held) {
        view = View::None;
        shown = 0;
        // (the panel's shape changed under it - a window resized: the other picture, drawn anew)
        static bool was_beside = false;
        const bool beside = video::side_room();
        if (side_shown && beside != was_beside) {
            video::set_side(nullptr, nullptr);
            video::set_below(nullptr);
            side_shown = false;
        }
        was_beside = beside;
        if (beside) draw_side(now_ms);
        else draw_below(now_ms);
        return;
    }
    if (side_shown) {
        video::set_side(nullptr, nullptr);
        video::set_below(nullptr);
        side_shown = false;
        sig = 0;
    }
    const uint32_t dt = now_ms - last_ms;
    last_ms = now_ms;
    const float step = (dt > 100 ? 100 : dt) / 140.0f;        // (out or in within 140 ms)
    shown += open() ? step : -step;
    if (shown < 0) shown = 0;
    if (shown > 1) shown = 1;
    if (shown <= 0) return;
    if (open()) {
        // A fight starts or ends under it: the view that fits
        if (view == View::Map && play::fighting()) view = View::Battle;
        if (view == View::Battle && !play::fighting()) view = View::Map;
        const bool blink_on = (now_ms / 400) & 1;
        const uint32_t s = signature(view, blink_on);
        if (s != sig) {
            sig = s;
            redraw(view, blink_on, true);
        }
    }
    // From the right: the 3D view and the fight's field stay in sight
    video::blit(video::kW - static_cast<int>(kW * shown), 0, kW, kH, pixels);
}

} // namespace panel
