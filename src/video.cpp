#include "video.h"

#include "engine/font.h"

#include <SDL.h>

#include <cstring>

namespace video {
namespace {

SDL_Window*   win = nullptr;
SDL_Renderer* ren = nullptr;
SDL_Texture*  tex = nullptr;        // 320 x 200, the frame as composed
SDL_Texture*  big = nullptr;        // the frame at a whole multiple (sharp pixels, then smoothed to the panel)
SDL_Texture*  side[2] = {nullptr, nullptr};     // the Companion beside the game (wide panels)
bool          side_on = false;
SDL_Texture*  below = nullptr;                  // the Companion under the game (square panels)
bool          below_on = false;
SDL_Rect      strip{0, 0, 0, 0};    // the room beside the game
SDL_Texture*  back = nullptr;       // the backdrop picture
bool          back_on = false;
int           back_top = 0;
int           big_k = 0, big_uw = 0;
unsigned      serial = 0;           // counts the picture's changes of shape
int           shape_uw = 0;
Fit           fit = Fit::Original;
uint32_t      pal[256];
// The port's own pages (the game list, the scan) fill a wide panel: the
// overlay is then `uw` across instead of 320, the 320 everything is laid
// out in standing in its middle (`ox` in from its left)
bool          wide = false;
int           uw = kW, ox = 0;
uint32_t      over[kMaxW * kH];     // the overlay (uw across)
uint32_t      shown[kMaxW * kH];    // the composed frame
SDL_Rect      dst{0, 0, kW, kH};    // where the picture lands in the window

void layout()
{
    int ow = kW, oh = kH;
    SDL_GetRendererOutputSize(ren, &ow, &oh);
    int w, h;
    if (fit == Fit::Stretch) {
        w = ow;
        h = oh;
    } else if (fit == Fit::Square) {
        int k = ow / kW < oh / kH ? ow / kW : oh / kH;
        if (k < 1) k = 1;
        w = kW * k;
        h = kH * k;
        if (w > ow || h > oh) {     // (a panel under 320 x 200)
            w = ow;
            h = ow * kH / kW;
        }
    } else {
        // 4:3: the 320 x 200 screen filled a 4:3 monitor (pixels 1.2 tall)
        w = ow;
        h = ow * 3 / 4;
        if (h > oh) {
            h = oh;
            w = oh * 4 / 3;
        }
    }
    // A page of the port's own on a panel wider than 4:3: the whole panel,
    // the same 1.2-tall pixels, more of them across
    uw = kW;
    if (wide && oh > 0) {
        int wu = static_cast<int>(static_cast<long>(kH) * 6 * ow / (5L * oh)) & ~1;
        if (wu > kMaxW) wu = kMaxW;
        if (wu > kW) {
            uw = wu;
            h = oh;
            w = static_cast<int>(static_cast<long>(oh) * uw * 5 / (kH * 6L));
            if (w > ow) w = ow;
        }
    }
    ox = (uw - kW) / 2;
    const SDL_Rect was = dst;
    const int was_uw = shape_uw;
    dst = SDL_Rect{(ow - w) / 2, (oh - h) / 2, w, h};
    shape_uw = uw;
    if (was.w != dst.w || was.h != dst.h || was_uw != uw) {
        ++serial;
        SDL_Log("[video] panel %d x %d: picture %d x %d, %d across%s", ow, oh, dst.w, dst.h, uw, wide ? " (a page of the port's)" : "");
    }
    strip = SDL_Rect{0, 0, 0, 0};
    if (uw == kW && side_on && fit == Fit::Original && ow - w >= 200) {
        dst.x = 0;                              // (the game at the left, the Companion in the rest)
        strip = SDL_Rect{w, 0, ow - w, oh};
    }
    if (uw == kW && below_on && fit == Fit::Original && oh - h >= kBelowH && ow >= kBelowW) {
        dst.y = 0;                              // (the game at the top, the Companion under it)
        strip = SDL_Rect{0, h, ow, oh - h};
    }
    // The in-between texture: the next whole multiple at or above the
    // picture's size, so uneven scaling doesn't make uneven pixels
    int k = (dst.w + uw - 1) / uw;
    const int kv = (dst.h + kH - 1) / kH;
    if (kv > k) k = kv;
    if (k < 1) k = 1;
    if (k > 6) k = 6;
    if (k != big_k || uw != big_uw || !big) {
        if (big) SDL_DestroyTexture(big);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        big = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, uw * k, kH * k);
        big_k = big ? k : 0;
        big_uw = uw;
    }
}

} // namespace

const uint8_t* glyph(char ch)
{
    const unsigned char c = static_cast<unsigned char>(ch);
    if (c >= 'A' && c <= 'Z') return font::kClearUpper[c - 'A'];
    if (c >= 'a' && c <= 'z') return font::kClearLower[c - 'a'];
    if (c >= 32 && c <= 63) return font::kClearText[c - 32];
    // (the two an email address needs that the engine's font hasn't - the port's own)
    static const uint8_t kAt[8] = {0x7C, 0xC6, 0xDE, 0xDE, 0xDE, 0xC0, 0x78, 0x00};
    static const uint8_t kUnder[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00};
    if (c == '@') return kAt;
    if (c == '_') return kUnder;
    return font::kClearText[0];
}

uint32_t palette(int value) { return pal[value & 0xFF]; }

void blit(int x, int y, int w, int h, const uint32_t* argb)
{
    for (int j = 0; j < h; ++j) {
        const int py = y + j;
        if (py < 0 || py >= kH) continue;
        for (int i = 0; i < w; ++i) {
            const int px = x + ox + i;
            const uint32_t c = argb[j * w + i];
            if (px >= 0 && px < uw && (c >> 24)) over[py * uw + px] = c;
        }
    }
}

void text_big(int x, int y, const char* s, uint32_t fg)
{
    // (the edge first: the letters' shape a pixel out every way)
    for (int pass = 0; pass < 2; ++pass) {
        int cx = x;
        for (const char* p = s; *p; ++p, cx += 16) {
            const uint8_t* g = glyph(*p);
            for (int j = 0; j < 8; ++j)
                for (int i = 0; i < 8; ++i)
                    if (g[j] & (0x80 >> i)) {
                        if (pass == 0) fill(cx + i * 2 - 1, y + j * 2 - 1, 4, 4, kBlack);
                        else fill(cx + i * 2, y + j * 2, 2, 2, fg);
                    }
        }
    }
}

void set_wide(bool on)
{
    if (on == wide) return;
    wide = on;
    if (ren) layout();
}

unsigned shape_serial() { return serial; }

int margin() { return ox; }
int back_w() { return uw * 2; }

bool side_room()
{
    if (!ren || fit != Fit::Original || wide) return false;
    int ow = 0, oh = 0;
    SDL_GetRendererOutputSize(ren, &ow, &oh);
    const int w = ow * 3 / 4 > oh ? oh * 4 / 3 : ow;
    return ow - w >= 200;
}

void set_side(const uint32_t* top, const uint32_t* bottom)
{
    const bool want = top != nullptr && side_room();
    if (want) {
        const uint32_t* px[2] = {top, bottom};
        for (int i = 0; i < 2; ++i) {
            if (!side[i]) {
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
                side[i] = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kSideW, kSideH);
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
            }
            if (side[i] && px[i]) SDL_UpdateTexture(side[i], nullptr, px[i], kSideW * 4);
        }
    }
    if (want != side_on) {
        side_on = want;
        layout();
    }
}

bool below_room()
{
    if (!ren || fit != Fit::Original || wide) return false;
    int ow = 0, oh = 0;
    SDL_GetRendererOutputSize(ren, &ow, &oh);
    const int h = ow * 3 / 4 > oh ? oh : ow * 3 / 4;
    return oh - h >= kBelowH && ow >= kBelowW && !side_room();
}

void set_below(const uint32_t* px)
{
    const bool want = px != nullptr && below_room();
    if (want) {
        if (!below) {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
            below = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kBelowW, kBelowH);
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        }
        if (below) SDL_UpdateTexture(below, nullptr, px, kBelowW * 4);
    }
    if (want != below_on) {
        below_on = want;
        layout();
    }
}

// The Companion's two pictures in the strip: as wide as it is, rows 1.2
// tall as the game's (so its squares stay square), both fitting its height
void draw_side()
{
    if (below_on && strip.w > 0 && below) {
        // (square pixels, as large as fits, in the strip's middle)
        int w = strip.w, h = w * kBelowH / kBelowW;
        if (h > strip.h) {
            h = strip.h;
            w = h * kBelowW / kBelowH;
        }
        const SDL_Rect to{strip.x + (strip.w - w) / 2, strip.y + (strip.h - h) / 2, w, h};
        SDL_RenderCopy(ren, below, nullptr, &to);
        return;
    }
    if (!side_on || strip.w <= 0) return;
    int w = strip.w, h = w * kSideH * 6 / (kSideW * 5);
    if (h * 2 > strip.h) {
        h = strip.h / 2;
        w = h * kSideW * 5 / (kSideH * 6);
    }
    for (int i = 0; i < 2; ++i) {
        if (!side[i]) continue;
        const SDL_Rect to{strip.x + (strip.w - w) / 2, strip.y + i * h, w, h};
        SDL_RenderCopy(ren, side[i], nullptr, &to);
    }
}

int pixel_aspect()
{
    if (dst.w <= 0 || dst.h <= 0) return 1000;
    return static_cast<int>(static_cast<long>(dst.h) * uw * 1000 / (static_cast<long>(dst.w) * kH));
}

void set_backdrop(const uint32_t* argb, int top)
{
    back_on = false;
    if (!ren || !argb) return;
    if (!back) {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        back = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kMaxW * 2, kBackH);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        if (!back) return;
    }
    const SDL_Rect part{0, 0, uw * 2, kBackH};
    SDL_UpdateTexture(back, &part, argb, uw * 2 * 4);
    back_top = top;
    back_on = true;
}

const char* fit_name(Fit f) { return f == Fit::Square ? "Square Pixels" : f == Fit::Stretch ? "Stretch" : "Original 4:3"; }

bool begin(int window_w, int window_h)
{
    const bool whole = window_w <= 0 || window_h <= 0;
    win = SDL_CreateWindow("Gold Box Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           whole ? 640 : window_w, whole ? 480 : window_h,
                           whole ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE);
    if (!win) return false;
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if (!ren) return false;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kMaxW, kH);
    if (!tex) return false;
    SDL_ShowCursor(SDL_DISABLE);
    set_ega_palette();
    clear();
    layout();
    return true;
}

void end()
{
    for (auto& t : side) {
        if (t) SDL_DestroyTexture(t);
        t = nullptr;
    }
    side_on = false;
    if (below) SDL_DestroyTexture(below);
    below = nullptr;
    below_on = false;
    if (back) SDL_DestroyTexture(back);
    back = nullptr;
    back_on = false;
    if (big) SDL_DestroyTexture(big);
    if (tex) SDL_DestroyTexture(tex);
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    big = tex = nullptr;
    ren = nullptr;
    win = nullptr;
}

void set_fit(Fit f)
{
    fit = f < Fit::Count ? f : Fit::Original;
    if (ren) layout();
}

void resized()
{
    if (ren) layout();
}

void set_window(int w, int h)
{
    if (!win || w <= 0 || h <= 0) return;
    SDL_SetWindowSize(win, w, h);
    resized();
}

void set_palette(int i, const pic::Rgb& c)
{
    pal[i & 0xFF] = 0xFF000000u | (static_cast<uint32_t>(c.r) << 16) | (static_cast<uint32_t>(c.g) << 8) | c.b;
}

void set_ega_palette()
{
    for (int i = 0; i < 16; ++i) set_palette(i, pic::kEga[i]);
    pal[pic::kTransparent] = 0xFF000000u;       // (EGA "see-through": black)
}

void clear(uint32_t colour)
{
    for (auto& p : over) p = colour;
}

void fill(int x, int y, int w, int h, uint32_t colour)
{
    x += ox;
    for (int j = y < 0 ? 0 : y; j < y + h && j < kH; ++j)
        for (int i = x < 0 ? 0 : x; i < x + w && i < uw; ++i) over[j * uw + i] = colour;
}

void frame(int x, int y, int w, int h, uint32_t colour)
{
    fill(x, y, w, 1, colour);
    fill(x, y + h - 1, w, 1, colour);
    fill(x, y, 1, h, colour);
    fill(x + w - 1, y, 1, h, colour);
}

void text_at(int x, int y, const char* s, uint32_t fg, uint32_t bg)
{
    x += ox;
    for (; *s; ++s, x += 8) {
        const uint8_t* g = glyph(*s);
        for (int j = 0; j < 8; ++j) {
            const int py = y + j;
            if (py < 0 || py >= kH) continue;
            for (int i = 0; i < 8; ++i) {
                const int px = x + i;
                if (px < 0 || px >= uw) continue;
                if (g[j] & (0x80 >> i)) over[py * uw + px] = fg;
                else if (bg != kClear) over[py * uw + px] = bg;
            }
        }
    }
}

int text(int col, int row, const char* s, uint32_t fg, uint32_t bg)
{
    text_at(col * 8, row * 8, s, fg, bg);
    return col + static_cast<int>(strlen(s));
}

// A letter at one and a half times its size (12 x 12), smoothed: each of
// its pixels the average of the 2 x 2 it covers of the letter at 3 x
void small_text(uint32_t* buf, int bw, int bh, int x, int y, const char* s, uint32_t fg)
{
    for (; *s; ++s, x += 12) {
        const uint8_t* g = glyph(*s);
        for (int j = 0; j < 12; ++j) {
            const int py = y + j;
            if (py < 0 || py >= bh) continue;
            for (int i = 0; i < 12; ++i) {
                const int px = x + i;
                if (px < 0 || px >= bw) continue;
                int cover = 0;
                for (int b = 0; b < 2; ++b)
                    for (int a = 0; a < 2; ++a)
                        if (g[(2 * j + b) / 3] & (0x80 >> ((2 * i + a) / 3))) ++cover;
                if (!cover) continue;
                const uint32_t rr = ((fg >> 16) & 0xFF) * cover / 4, gg = ((fg >> 8) & 0xFF) * cover / 4, bb = (fg & 0xFF) * cover / 4;
                buf[static_cast<size_t>(py) * bw + px] = 0xFF000000u | (rr << 16) | (gg << 8) | bb;
            }
        }
    }
}

void pointer(int x, int y)
{
    // (an arrow, 7 tall: cream with a black edge)
    static const char* const kShape[9] = {"#", "##", "#.#", "#..#", "#...#", "#....#", "#..###", "#.#", "##"};
    for (int j = 0; j < 9; ++j)
        for (int i = 0; kShape[j][i]; ++i) {
            const int px = x + ox + i, py = y + j;
            if (px < 0 || px >= uw || py < 0 || py >= kH) continue;
            over[py * uw + px] = kShape[j][i] == '#' ? kBlack : kCream;
        }
}

void present(const pic::Canvas* game)
{
    if (!ren) return;
    const bool picture = back_on && !game;      // (the backdrop shows through the overlay's gaps)
    for (int y = 0; y < kH; ++y)
        for (int x = 0; x < uw; ++x) {
            const uint32_t o = over[y * uw + x];
            const int gx = x - ox;
            shown[y * uw + x] = (o >> 24) ? o : game && game->px && gx >= 0 && gx < kW ? pal[game->get(gx, y)]
                              : picture && y >= back_top ? 0u : 0xFF000000u;
        }
    const SDL_Rect part{0, 0, uw, kH};
    SDL_UpdateTexture(tex, &part, shown, uw * 4);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
    if (picture) {
        SDL_RenderClear(ren);
        const SDL_Rect to{dst.x, dst.y + dst.h * back_top / kH, dst.w, dst.h - dst.h * back_top / kH};
        const SDL_Rect from{0, 0, uw * 2, kBackH - back_top * 2};
        SDL_RenderCopy(ren, back, &from, &to);
        if (big) {
            SDL_SetTextureBlendMode(big, SDL_BLENDMODE_BLEND);
            SDL_SetRenderTarget(ren, big);
            SDL_RenderCopy(ren, tex, &part, nullptr);     // (alpha copied as it is)
            SDL_SetRenderTarget(ren, nullptr);
            SDL_RenderCopy(ren, big, nullptr, &dst);
            SDL_SetTextureBlendMode(big, SDL_BLENDMODE_NONE);
        } else {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_RenderCopy(ren, tex, &part, &dst);
        }
        draw_side();
        SDL_RenderPresent(ren);
        return;
    }
    if (big) {
        SDL_SetRenderTarget(ren, big);
        SDL_RenderCopy(ren, tex, &part, nullptr);
        SDL_SetRenderTarget(ren, nullptr);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, big, nullptr, &dst);
    } else {
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, &part, &dst);
    }
    draw_side();
    SDL_RenderPresent(ren);
}

bool to_canvas(int wx, int wy, int& cx, int& cy)
{
    if (!ren) return false;
    // (window points -> the renderer's pixels: they differ on high-DPI screens)
    int ww = 1, wh = 1, ow = 1, oh = 1;
    SDL_GetWindowSize(win, &ww, &wh);
    SDL_GetRendererOutputSize(ren, &ow, &oh);
    const int x = wx * ow / (ww ? ww : 1), y = wy * oh / (wh ? wh : 1);
    if (x < dst.x || y < dst.y || x >= dst.x + dst.w || y >= dst.y + dst.h) return false;
    cx = (x - dst.x) * uw / dst.w - ox;
    if (cx < 0 || cx >= kW) return false;
    cy = (y - dst.y) * kH / dst.h;
    return true;
}

bool save_bmp(const char* path)
{
    // What the panel got: the frame as scaled, the backdrop included
    if (!ren) return false;
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(ren, &w, &h);
    SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!s) return false;
    bool ok = SDL_RenderReadPixels(ren, nullptr, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch) == 0;
    ok = ok && SDL_SaveBMP(s, path) == 0;
    SDL_FreeSurface(s);
    return ok;
}

} // namespace video
