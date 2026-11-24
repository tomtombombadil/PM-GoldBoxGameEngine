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

Emit emit_fn = nullptr;
bool on = false;
int  lay_w = 0, lay_h = 0;          // the size the buttons were laid out for
Key  keys[kCount];
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

// Lays the buttons out for a renderer of ow x oh; the picture's room back
void layout(int ow, int oh, int& ax, int& ay, int& aw, int& ah)
{
    lay_w = ow;
    lay_h = oh;
    if (ow >= oh) {
        // Landscape: a column each side
        int p = oh * 46 / 100;
        if (p > ow * 21 / 100) p = ow * 21 / 100;
        ax = p;
        ay = 0;
        aw = ow - 2 * p;
        ah = oh;
        int c = p * 28 / 100;
        if (c * 3 > oh * 45 / 100) c = oh * 15 / 100;
        const int cy = oh * 64 / 100;
        const int bh = oh * 9 / 100, top = oh * 5 / 100;
        dpad_at(p / 2, cy, c);
        place(L2, Shape::Rect, p * 8 / 100, top, p * 38 / 100, bh, "L2");
        place(L1, Shape::Rect, p * 54 / 100, top, p * 38 / 100, bh, "L1");
        place(Select, Shape::Rect, p * 22 / 100, top + bh * 3 / 2, p * 56 / 100, bh * 8 / 10, "MOUSE");
        const int rx = ow - p;
        face_at(rx + p / 2, cy, c);
        place(R1, Shape::Rect, rx + p * 8 / 100, top, p * 38 / 100, bh, "R1");
        place(R2, Shape::Rect, rx + p * 54 / 100, top, p * 38 / 100, bh, "R2");
        place(Start, Shape::Rect, rx + p * 22 / 100, top + bh * 3 / 2, p * 56 / 100, bh * 8 / 10, "MENU");
        return;
    }
    // Portrait: the picture at the top, the buttons under it
    ax = 0;
    ay = 0;
    aw = ow;
    ah = ow * 3 / 4;
    if (ah > oh * 58 / 100) ah = oh * 58 / 100;
    const int p = oh - ah;
    const int gap = ow / 60;
    int bh = p * 11 / 100;
    if (bh > ow * 9 / 100) bh = ow * 9 / 100;
    const int bw = (ow - gap * 7) / 6;
    const int row = ah + p * 4 / 100;
    const int order[6] = {L2, L1, Select, Start, R1, R2};
    const char* names[6] = {"L2", "L1", "MOUSE", "MENU", "R1", "R2"};
    for (int i = 0; i < 6; ++i) place(order[i], Shape::Rect, gap + i * (bw + gap), row, bw, bh, names[i]);
    int c = ow * 14 / 100;
    if (c * 34 > (p - bh) * 10 * 8 / 10) c = (p - bh) * 8 / 34;
    const int cy = row + bh + (oh - row - bh) / 2;
    dpad_at(ow * 25 / 100, cy, c);
    face_at(ow * 75 / 100, cy, c);
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
    const int reach = cell * 3 / 2 + (held ? cell : cell / 4);
    if (dx < -reach || dx > reach || dy < -reach || dy > reach) return -2;
    const int adx = dx < 0 ? -dx : dx, ady = dy < 0 ? -dy : dy;
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
    for (int b = A; b < kCount; ++b) {
        const Key& k = keys[b];
        if (k.shape == Shape::None) continue;
        if (k.shape == Shape::Round) {
            const int r = k.r.w * 62 / 100;
            const int dx = x - (k.r.x + k.r.w / 2), dy = y - (k.r.y + k.r.h / 2);
            if (dx * dx + dy * dy <= r * r) return b;
        } else if (inside(k.r, x, y, k.r.h / 6)) {
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
    video::resized();
}

bool shown() { return on; }

void set_diagonals(bool v)
{
    if (v == diagonals) return;
    diagonals = v;
    // (a finger already on the pad: what it holds is worked out at its next move)
}

void area(int x0, int y0, int w, int h, int& ax, int& ay, int& aw, int& ah)
{
    if (!on) {
        ax = x0;
        ay = y0;
        aw = w;
        ah = h;
        return;
    }
    layout(w, h, ax, ay, aw, ah);
    // (laid out from 0, 0: moved into the usable part)
    ax += x0;
    ay += y0;
    for (Key& k : keys) {
        k.r.x += x0;
        k.r.y += y0;
    }
    pad_cx += x0;
    pad_cy += y0;
}

void draw(SDL_Renderer* ren)
{
    if (!on) return;
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    int px = lay_w < lay_h ? lay_w / 220 : lay_h / 220;
    if (px < 2) px = 2;
    for (int b = 0; b < kCount; ++b) {
        const Key& k = keys[b];
        if (k.shape == Shape::None) continue;
        const bool lit = pressed[b] > 0;
        if (k.shape == Shape::Round) {
            const int cx = k.r.x + k.r.w / 2, cy = k.r.y + k.r.h / 2, r = k.r.w / 2;
            SDL_SetRenderDrawColor(ren, 0xF0, 0xE6, 0xC8, lit ? 120 : 70);
            disc(ren, cx, cy, r);
            SDL_SetRenderDrawColor(ren, 0x10, 0x18, 0x38, lit ? 200 : 140);
            disc(ren, cx, cy, r - px);
            SDL_SetRenderDrawColor(ren, 0xE8, 0xB8, 0x40, lit ? 255 : 190);
            label(ren, k.label, cx, cy, px * 2);
        } else {
            SDL_SetRenderDrawColor(ren, 0xF0, 0xE6, 0xC8, lit ? 120 : 70);
            SDL_RenderFillRect(ren, &k.r);
            const SDL_Rect in{k.r.x + px, k.r.y + px, k.r.w - 2 * px, k.r.h - 2 * px};
            SDL_SetRenderDrawColor(ren, 0x10, 0x18, 0x38, lit ? 200 : 140);
            SDL_RenderFillRect(ren, &in);
            SDL_SetRenderDrawColor(ren, 0xE8, 0xB8, 0x40, lit ? 255 : 190);
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
    SDL_SetRenderDrawColor(ren, 0x10, 0x18, 0x38, 140);
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
            SDL_SetRenderDrawColor(ren, 0xF0, 0xE6, 0xC8, lit ? 90 : 40);
            SDL_RenderFillRect(ren, &sq);
            SDL_SetRenderDrawColor(ren, 0xE8, 0xB8, 0x40, lit ? 255 : 190);
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
        const int d = dpad_dir(x, y, false);
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
