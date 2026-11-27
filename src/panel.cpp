#include "panel.h"

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
// red: locked), a secret door a cyan S, the party a green arrow (the CYD's
// colours and legend - Tom, 2026-11-14: no heading, the words alone).
// In the zone zx, zy, zw x zh. square: the buffer's pixels are square
// (under the game; the legend to the map's right) - else squares 12 x 10,
// about square once a 4:3 screen has stretched the rows, the legend under.
void draw_map(int zx, int zy, int zw, int zh, bool square)
{
    put(zx, zy, zw, zh, video::kBlack);
    const geo::Map* m = play::map();
    if (!m) {
        letters(zx + 8, zy + (square ? 8 : 40), "No map here.", video::kCream);
        map_rows = zy + 52;
        return;
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
                    if (play::map_arch(x, y, d)) {
                        // An archway (the engine's play::map_arch - its wall's picture see-through; Tom,
                        // 2026-11-27): an opening in the wall line, its middle half away
                        const int q = (across ? cw : ch) / 4;
                        if (across) put(lx + q, ly, cw - 2 * q + 1, 1, video::kBlack);
                        else put(lx, ly + q, 1, ch - 2 * q + 1, video::kBlack);
                        continue;
                    }
                    if (door == 1 && (plain >> w & 1)) {
                        // Drawn as the area's wall: a secret door only where the game lets the party through
                        // (the engine's play::map_secret, 0.214.2 - before it is found; the rest is wall)
                        if (!play::map_secret(x, y, d) || cw < 8) continue;
                        const int ox = across ? lx + cw / 2 - 2 : lx - 2, oy = across ? ly - 3 : ly + ch / 2 - 3;
                        for (int r = 0; r < 7; ++r)
                            for (int c = 0; c < 5; ++c)
                                if (kS[r] >> (4 - c) & 1) put(ox + c, oy + r, 1, 1, video::kCyan);
                        continue;
                    }
                    const uint32_t col = door == 1 ? video::kGold : video::kWarn;
                    if (across) put(lx + 1, ly - 1, cw - 1, 3, col);
                    else put(lx - 1, ly + 1, 3, ch - 1, col);
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
    // The party: an arrowhead the way it faces
    const int cx = mx + play::pos_x() * cw + cw / 2, cy = my + play::pos_y() * ch + ch / 2;
    const int dir = play::dir(), fx = geo::dx(dir), fy = geo::dy(dir);
    const int len = (fx ? cw : ch) / 2 - 1;
    for (int k = -len; k <= len; ++k) {
        // (k along the facing; the head narrows to the front)
        const int half = (len - k) / 2;
        for (int s = -half; s <= half; ++s) put(cx + fx * k + (fx ? 0 : s), cy + fy * k + (fy ? 0 : s), 1, 1, 0xFF28FF28u);
    }
    if (square) {
        const int lx = mx + m->w * cw + 10;
        if (places) letters(lx, my, "Place", kPlace);
        else {
            letters(lx, my, "Locked", video::kWarn);
            letters(lx, my + 12, "Secret", video::kCyan);
        }
    } else {
        const int ly = my + m->h * ch + 6;
        map_rows = ly + 10;
        if (places) letters(zx + 4, ly, "Place", kPlace);
        else {
            letters(zx + 4, ly, "Locked", video::kWarn);
            letters(zx + 68, ly, "Secret", video::kCyan);
        }
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
