// On-screen buttons (touchpad.h)
#include "touchpad.h"

#include <cstring>

#include "video.h"

namespace touchpad {

namespace {

enum { Up, Down, Left, Right, A, B, X, Y, L1, R1, L2, R2, Start, Select };

enum class Shape : uint8_t { None, Rect, Round };

struct Key {
    Shape       shape = Shape::None;
    SDL_Rect    r{0, 0, 0, 0};
    const char* label = "";
};

constexpr int kToggle = kCount;     // the hide / show arrow (no button of the game's)

Emit emit_fn = nullptr;
bool on = false;
bool hidden_ = false;               // the arrow put the buttons away (the arrow stays)
int  size_pct = 100, alpha_pct = 60;    // Controls - Button Size / Button Opacity
int  lay_w = 0, lay_h = 0;          // the size the buttons were laid out for
Key  keys[kCount + 1];
int  pressed[kCount];               // fingers on each button
// The D-Pad: one square, the direction from where the finger is in it
int  pad_cx = 0, pad_cy = 0, cell = 1;

// The D-Pad's corners are diagonals while a fight's fighter moves or aims
// (set_diagonals; Tom, 2026-11-23: "there's no real d-pad ... we need to add
// diagonal movement buttons during combat") - two directions held at once,
// as two buttons of a controller's pad
bool diagonals = false;

// Fingers on the buttons: which, and on what - a D-Pad finger holds a mask of
// directions (1 << Up ...; 0 its middle), any other finger one button
constexpr int kFingers = 10;
SDL_FingerID fid[kFingers];
int          fon[kFingers];
bool         fused[kFingers] = {};
bool         fpad[kFingers] = {};

void press(int b, bool down)
{
    if (b < 0 || b >= kCount) return;
    if (down) {
        if (pressed[b]++ == 0 && emit_fn) emit_fn(b, true);
    } else if (pressed[b] > 0) {
        if (--pressed[b] == 0 && emit_fn) emit_fn(b, false);
    }
}

void place(int b, Shape s, int x, int y, int w, int h, const char* label)
{
    keys[b].shape = s;
    keys[b].r = SDL_Rect{x, y, w, h};
    keys[b].label = label;
}

void round_at(int b, int cx, int cy, int d, const char* label) { place(b, Shape::Round, cx - d / 2, cy - d / 2, d, d, label); }

void dpad_at(int cx, int cy, int c)
{
    pad_cx = cx;
    pad_cy = cy;
    cell = c > 0 ? c : 1;
    place(Up, Shape::Rect, cx - c / 2, cy - c / 2 - c, c, c, "");
    place(Down, Shape::Rect, cx - c / 2, cy + c / 2, c, c, "");
    place(Left, Shape::Rect, cx - c / 2 - c, cy - c / 2, c, c, "");
    place(Right, Shape::Rect, cx + c / 2, cy - c / 2, c, c, "");
}

void face_at(int cx, int cy, int c)
{
    const int d = c * 105 / 100;
    round_at(X, cx, cy - c, d, "X");
    round_at(A, cx + c, cy, d, "A");
    round_at(B, cx, cy + c, d, "B");
    round_at(Y, cx - c, cy, d, "Y");
}

int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

// The buttons over the panel (Tom, 2026-11-23: "the controls need to be an
// overlay, and the game needs to be maximized in the screen area"): the
// picture keeps its whole room; the D-Pad and the face buttons go in the
// free room beside it when there is enough, else over its sides - above the
// game's menu line, which stays clear ("it might cover the menu line at the
// bottom"). L1 L2 and MOUSE top left, after the hide / show arrow; R1 R2 and
// MENU top right. Portrait: all at the foot of the panel.
void lay_out(int W, int H, const SDL_Rect& use, const SDL_Rect& pic, const SDL_Rect& strip, int top0)
{
    lay_w = W;
    lay_h = H;
    const int base = use.w < use.h ? use.w : use.h;
    int c = base * 10 / 100 * size_pct / 100;          // a D-Pad cell; the D-Pad is 3 x 3
    if (c < 20) c = 20;
    const int mg = c / 3;
    const int d = c * 105 / 100, span = 2 * c + d;     // the face buttons' diamond across
    const int sw = c * 3 / 2, sh = c * 7 / 10;         // a shoulder button
    const int tg = c * 3 / 4;                          // the hide / show arrow
    // (the hide / show arrow, top left - Tom: "a button in the upper left to toggle the overlay")
    const int y0 = top0 > use.y ? top0 : use.y;          // (the top rows' start)
    place(kToggle, Shape::Rect, use.x + mg, y0 + mg, tg, tg, "");
    const int menu_top = pic.y + pic.h * 184 / 200;    // (the game's menu line: canvas rows 23-24)
    if (use.w >= use.h) {
        // ---- Landscape
        const bool strip_right = strip.w > 0 && strip.x >= pic.x + pic.w;
        const int used_right = strip_right ? strip.x + strip.w : pic.x + pic.w;
        const int left_free = pic.x - use.x, right_free = use.x + use.w - used_right;
        const int need = 3 * c + 2 * mg, fneed = span + 2 * mg;
        const int rows_bottom = y0 + mg + sh + mg + sh + mg;        // (under the shoulder rows)
        const int low = use.y + use.h * 62 / 100;
        // The D-Pad: in the free band at the left, else over the game's left side, its foot above the menu line
        int pcx, pcy;
        if (left_free >= need) {
            pcx = use.x + left_free / 2;
            pcy = low;
        } else {
            pcx = use.x + mg + 3 * c / 2;
            pcy = menu_top - mg - 3 * c / 2 < low ? menu_top - mg - 3 * c / 2 : low;
        }
        pcy = clampi(pcy, rows_bottom + 3 * c / 2, use.y + use.h - mg - 3 * c / 2);
        dpad_at(pcx, pcy, c);
        // The face buttons: the free band at the right, else over the Companion (it has no menu line) or the game
        int fcx, fcy;
        if (right_free >= fneed) {
            fcx = use.x + use.w - right_free / 2;
            fcy = low;
        } else {
            fcx = use.x + use.w - mg - span / 2;
            fcy = strip_right ? low : (menu_top - mg - span / 2 < low ? menu_top - mg - span / 2 : low);
        }
        fcy = clampi(fcy, rows_bottom + span / 2, use.y + use.h - mg - span / 2);
        face_at(fcx, fcy, c);
        // The shoulders: in the bands when they fit, else over the picture's top corners
        const int lx = left_free >= need ? use.x + (left_free - 2 * sw - mg) / 2 : use.x + mg + tg + mg;
        const int top = y0 + mg;
        place(L2, Shape::Rect, lx < use.x + mg + tg + mg ? use.x + mg + tg + mg : lx, top, sw, sh, "L2");
        place(L1, Shape::Rect, keys[L2].r.x + sw + mg, top, sw, sh, "L1");
        place(Select, Shape::Rect, keys[L2].r.x, top + sh + mg, 2 * sw + mg, sh, "MOUSE");
        const int rx = right_free >= fneed ? use.x + use.w - right_free / 2 + (2 * sw + mg) / 2 - sw
                                           : use.x + use.w - mg - sw;
        place(R2, Shape::Rect, rx, top, sw, sh, "R2");
        place(R1, Shape::Rect, rx - mg - sw, top, sw, sh, "R1");
        place(Start, Shape::Rect, keys[R1].r.x, top + sh + mg, 2 * sw + mg, sh, "MENU");
        return;
    }
    // ---- Portrait: the D-Pad and the face buttons at the panel's foot, the shoulders in a row above them
    const int pcy = use.y + use.h - mg - 3 * c / 2;
    dpad_at(use.x + mg + 3 * c / 2, pcy, c);
    face_at(use.x + use.w - mg - span / 2, pcy, c);
    const int row = pcy - 3 * c / 2 - mg - sh;
    int bw = (use.w - 7 * mg) / 6;
    if (bw > sw * 3 / 2) bw = sw * 3 / 2;
    const int order[6] = {L2, L1, Select, Start, R1, R2};
    const char* names[6] = {"L2", "L1", "MOUSE", "MENU", "R1", "R2"};
    const int left0 = use.x + mg, right0 = use.x + use.w - mg - 3 * bw - 2 * mg;
    for (int i = 0; i < 6; ++i)
        place(order[i], Shape::Rect, (i < 3 ? left0 : right0) + (i % 3) * (bw + mg), row, bw, sh, names[i]);
}

bool inside(const SDL_Rect& r, int x, int y, int grow)
{
    return x >= r.x - grow && y >= r.y - grow && x < r.x + r.w + grow && y < r.y + r.h + grow;
}

// The D-Pad's directions at (x, y) as a mask (1 << Up ...): one, or two in a
// corner while the diagonals are on; 0 in its middle, -2 off it
int dpad_dir(int x, int y, bool held)
{
    const int dx = x - pad_cx, dy = y - pad_cy;
    // (a touch: only on what is drawn - the button wins there, anywhere else is the
    // game's, Tom 2026-11-23; a finger already on it may roll on past its edge)
    const int reach = cell * 3 / 2 + (held ? cell : 0);
    if (dx < -reach || dx > reach || dy < -reach || dy > reach) return -2;
    const int adx = dx < 0 ? -dx : dx, ady = dy < 0 ? -dy : dy;
    if (!held && !diagonals && adx > cell / 2 && ady > cell / 2) return -2;    // (an empty corner: the game's)
    if (adx < cell / 4 && ady < cell / 4) return 0;
    const int h = 1 << (dx < 0 ? Left : Right), v = 1 << (dy < 0 ? Up : Down);
    // (a corner: both ways - the smaller at least half the larger)
    if (diagonals && adx > cell / 3 && ady > cell / 3 && adx * 2 >= ady && ady * 2 >= adx) return h | v;
    return adx > ady ? h : v;
}

void press_mask(int was, int now)
{
    for (int b = Up; b <= Right; ++b) {
        if ((was >> b & 1) && !(now >> b & 1)) press(b, false);
        if (!(was >> b & 1) && (now >> b & 1)) press(b, true);
    }
}

int hit(int x, int y)
{
    if (inside(keys[kToggle].r, x, y, 0)) return kToggle;
    if (hidden_) return -2;
    for (int b = A; b < kCount; ++b) {
        const Key& k = keys[b];
        if (k.shape == Shape::None) continue;
        if (k.shape == Shape::Round) {
            const int r = k.r.w / 2;
            const int dx = x - (k.r.x + k.r.w / 2), dy = y - (k.r.y + k.r.h / 2);
            if (dx * dx + dy * dy <= r * r) return b;
        } else if (inside(k.r, x, y, 0)) {
            return b;
        }
    }
    return -2;
}

void disc(SDL_Renderer* ren, int cx, int cy, int r)
{
    for (int dy = -r; dy <= r; ++dy) {
        int dx = 0;
        while ((dx + 1) * (dx + 1) + dy * dy <= r * r) ++dx;
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void label(SDL_Renderer* ren, const char* s, int cx, int cy, int px)
{
    const int n = static_cast<int>(strlen(s));
    const int x0 = cx - n * 8 * px / 2, y0 = cy - 4 * px;
    for (int i = 0; i < n; ++i) {
        const uint8_t* g = video::glyph(s[i]);
        if (!g) continue;
        for (int row = 0; row < 8; ++row)
            for (int col = 0; col < 8; ++col)
                if (g[row] & (0x80 >> col)) {
                    const SDL_Rect r{x0 + (i * 8 + col) * px, y0 + row * px, px, px};
                    SDL_RenderFillRect(ren, &r);
                }
    }
}

// An arrow in a D-Pad cell, pointing out from the middle
void arrow(SDL_Renderer* ren, const SDL_Rect& r, int dir)
{
    const int cx = r.x + r.w / 2, cy = r.y + r.h / 2, s = r.w / 5;
    for (int i = 0; i <= s; ++i) {
        // (i: from the tip back)
        switch (dir) {
        case Up: SDL_RenderDrawLine(ren, cx - i, cy - s / 2 + i, cx + i, cy - s / 2 + i); break;
        case Down: SDL_RenderDrawLine(ren, cx - i, cy + s / 2 - i, cx + i, cy + s / 2 - i); break;
        case Left: SDL_RenderDrawLine(ren, cx - s / 2 + i, cy - i, cx - s / 2 + i, cy + i); break;
        default: SDL_RenderDrawLine(ren, cx + s / 2 - i, cy - i, cx + s / 2 - i, cy + i); break;
        }
    }
}

} // namespace

void set_emit(Emit e) { emit_fn = e; }

void set_shown(bool v)
{
    if (v == on) return;
    if (!v) release_all();
    on = v;
    video::resized();       // (a docked Companion moves over, leaving the D-Pad its band)
}

bool shown() { return on; }

void set_diagonals(bool v)
{
    if (v == diagonals) return;
    diagonals = v;
    // (a finger already on the pad: what it holds is worked out at its next move)
}

void place(int W, int H, const SDL_Rect& use, const SDL_Rect& pic, const SDL_Rect& strip, int top)
{
    lay_out(W, H, use, pic, strip, top);
}

int foot(const SDL_Rect& use)
{
    if (!on) return 0;
    const int base = use.w < use.h ? use.w : use.h;
    int c = base * 10 / 100 * size_pct / 100;
    if (c < 20) c = 20;
    const int mg = c / 3, sh = c * 7 / 10;
    return 3 * c + mg + sh + 2 * mg;        // (lay_out's portrait: the D-Pad, the shoulders' row, the margins)
}

void set_style(int size, int opacity)
{
    const bool changed = size != size_pct;
    size_pct = size < 50 ? 50 : size > 200 ? 200 : size;
    alpha_pct = opacity < 10 ? 10 : opacity > 100 ? 100 : opacity;
    if (changed) video::resized();
}

void set_hidden(bool v)
{
    if (v == hidden_) return;
    if (v) {
        // (the buttons away: nothing left held - the arrow's own finger stays)
        for (int i = 0; i < kFingers; ++i)
            if (fused[i] && fon[i] >= 0) {
                if (fpad[i]) press_mask(fon[i], 0);
                else press(fon[i], false);
                fused[i] = false;
            }
    }
    hidden_ = v;
}

bool hidden() { return hidden_; }

// A colour's see-through amount at the Button Opacity
Uint8 al(int a) { return static_cast<Uint8>(a * alpha_pct / 100); }

void chevron(SDL_Renderer* ren, const SDL_Rect& r, bool up)
{
    const int cx = r.x + r.w / 2, cy = r.y + r.h / 2, s = r.w / 3, t = r.w / 10 > 1 ? r.w / 10 : 2;
    for (int k = 0; k < t; ++k)
        for (int i = 0; i <= s; ++i) {
            const int y = up ? cy - s / 2 + i + k : cy + s / 2 - i - k;
            SDL_RenderDrawPoint(ren, cx - i, y);
            SDL_RenderDrawPoint(ren, cx + i, y);
        }
}

void draw(SDL_Renderer* ren)
{
    if (!on) return;
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    int px = lay_w < lay_h ? lay_w / 220 : lay_h / 220;
    if (px < 2) px = 2;
    {
        // The hide / show arrow: up puts the buttons away, down brings them back
        const SDL_Rect& r = keys[kToggle].r;
        SDL_SetRenderDrawColor(ren, 0x10, 0x18, 0x38, al(160));
        SDL_RenderFillRect(ren, &r);
        SDL_SetRenderDrawColor(ren, 0xE8, 0xB8, 0x40, al(230) > 120 ? al(230) : 120);
        chevron(ren, r, !hidden_);
    }
    if (hidden_) {
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
        return;
    }
    for (int b = 0; b < kCount; ++b) {
        const Key& k = keys[b];
        if (k.shape == Shape::None) continue;
        const bool lit = pressed[b] > 0;
        if (k.shape == Shape::Round) {
            const int cx = k.r.x + k.r.w / 2, cy = k.r.y + k.r.h / 2, r = k.r.w / 2;
            SDL_SetRenderDrawColor(ren, 0xF0, 0xE6, 0xC8, al(lit ? 120 : 70));
            disc(ren, cx, cy, r);
            SDL_SetRenderDrawColor(ren, 0x10, 0x18, 0x38, al(lit ? 200 : 140));
            disc(ren, cx, cy, r - px);
            SDL_SetRenderDrawColor(ren, 0xE8, 0xB8, 0x40, al(lit ? 255 : 190));
            label(ren, k.label, cx, cy, px * 2);
        } else {
            SDL_SetRenderDrawColor(ren, 0xF0, 0xE6, 0xC8, al(lit ? 120 : 70));
            SDL_RenderFillRect(ren, &k.r);
            const SDL_Rect in{k.r.x + px, k.r.y + px, k.r.w - 2 * px, k.r.h - 2 * px};
            SDL_SetRenderDrawColor(ren, 0x10, 0x18, 0x38, al(lit ? 200 : 140));
            SDL_RenderFillRect(ren, &in);
            SDL_SetRenderDrawColor(ren, 0xE8, 0xB8, 0x40, al(lit ? 255 : 190));
            if (b <= Right) arrow(ren, k.r, b);
            else {
                // (as big as fits the button: twice the line's thickness at most)
                const int n = static_cast<int>(strlen(k.label));
                int lp = px * 2;
                if (n > 0 && lp * 8 * n > k.r.w * 85 / 100) lp = k.r.w * 85 / 100 / (8 * n);
                if (lp * 8 > k.r.h * 70 / 100) lp = k.r.h * 70 / 100 / 8;
                if (lp < 1) lp = 1;
                label(ren, k.label, k.r.x + k.r.w / 2, k.r.y + k.r.h / 2, lp);
            }
        }
    }
    // (the D-Pad's middle, joining its arms)
    const SDL_Rect mid{pad_cx - cell / 2, pad_cy - cell / 2, cell, cell};
    SDL_SetRenderDrawColor(ren, 0x10, 0x18, 0x38, al(140));
    SDL_RenderFillRect(ren, &mid);
    if (diagonals) {
        // The corners: a diagonal's arrowhead each, lit while both its ways are held
        const int sz = cell / 3;
        for (int k = 0; k < 4; ++k) {
            const int sx = k & 1 ? 1 : -1, sy = k & 2 ? 1 : -1;
            const bool lit = pressed[sx < 0 ? Left : Right] > 0 && pressed[sy < 0 ? Up : Down] > 0;
            // (the corner's point, out from the middle; the arrowhead's edges back along both arms)
            const int px0 = pad_cx + sx * (cell * 3 / 2 - cell / 6), py0 = pad_cy + sy * (cell * 3 / 2 - cell / 6);
            const SDL_Rect sq{pad_cx + sx * cell - cell / 2 + cell / 8, pad_cy + sy * cell - cell / 2 + cell / 8,
                              cell * 3 / 4, cell * 3 / 4};
            SDL_SetRenderDrawColor(ren, 0xF0, 0xE6, 0xC8, al(lit ? 90 : 40));
            SDL_RenderFillRect(ren, &sq);
            SDL_SetRenderDrawColor(ren, 0xE8, 0xB8, 0x40, al(lit ? 255 : 190));
            for (int i = 0; i <= sz; ++i)
                SDL_RenderDrawLine(ren, px0, py0 - sy * i, px0 - sx * (sz - i), py0 - sy * i);
        }
    }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
}

bool finger(const SDL_Event& e, int ow, int oh)
{
    if (e.type != SDL_FINGERDOWN && e.type != SDL_FINGERMOTION && e.type != SDL_FINGERUP) return false;
    const int x = static_cast<int>(e.tfinger.x * ow), y = static_cast<int>(e.tfinger.y * oh);
    int slot = -1;
    for (int i = 0; i < kFingers; ++i)
        if (fused[i] && fid[i] == e.tfinger.fingerId) slot = i;
    if (e.type == SDL_FINGERDOWN) {
        if (!on || slot >= 0) return slot >= 0;
        for (int i = 0; i < kFingers && slot < 0; ++i)
            if (!fused[i]) slot = i;
        if (slot < 0) return false;
        const int d = hidden_ ? -2 : dpad_dir(x, y, false);
        if (d != -2) {
            fused[slot] = true;
            fid[slot] = e.tfinger.fingerId;
            fpad[slot] = true;
            fon[slot] = d;
            press_mask(0, d);
            return true;
        }
        const int b = hit(x, y);
        if (b < 0) return false;
        fused[slot] = true;
        fid[slot] = e.tfinger.fingerId;
        fpad[slot] = false;
        if (b == kToggle) {
            fon[slot] = -1;                     // (the finger is the arrow's until it lifts)
            set_hidden(!hidden_);
            return true;
        }
        fon[slot] = b;
        press(b, true);
        return true;
    }
    if (slot < 0) return false;
    if (e.type == SDL_FINGERMOTION) {
        if (fpad[slot]) {
            // (a thumb rolling round the D-Pad: the direction follows it)
            int d = dpad_dir(x, y, true);
            if (d == -2) d = 0;
            if (d != fon[slot]) {
                press_mask(fon[slot], d);
                fon[slot] = d;
            }
        }
        return true;
    }
    if (fpad[slot]) press_mask(fon[slot], 0);
    else press(fon[slot], false);
    fused[slot] = false;
    return true;
}

void release_all()
{
    for (int i = 0; i < kFingers; ++i)
        if (fused[i]) {
            if (fpad[i]) press_mask(fon[i], 0);
            else press(fon[i], false);
            fused[i] = false;
        }
    for (int b = 0; b < kCount; ++b)
        while (pressed[b] > 0) press(b, false);
}

} // namespace touchpad
