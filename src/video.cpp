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
int           big_k = 0;
Fit           fit = Fit::Original;
uint32_t      pal[256];
uint32_t      over[kW * kH];        // the overlay
uint32_t      shown[kW * kH];       // the composed frame
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
    dst = SDL_Rect{(ow - w) / 2, (oh - h) / 2, w, h};
    // The in-between texture: the next whole multiple at or above the
    // picture's size, so uneven scaling doesn't make uneven pixels
    int k = (dst.w + kW - 1) / kW;
    const int kv = (dst.h + kH - 1) / kH;
    if (kv > k) k = kv;
    if (k < 1) k = 1;
    if (k > 6) k = 6;
    if (k != big_k || !big) {
        if (big) SDL_DestroyTexture(big);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        big = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, kW * k, kH * k);
        big_k = big ? k : 0;
    }
}

const uint8_t* glyph(char ch)
{
    const unsigned char c = static_cast<unsigned char>(ch);
    if (c >= 'A' && c <= 'Z') return font::kClearUpper[c - 'A'];
    if (c >= 'a' && c <= 'z') return font::kClearLower[c - 'a'];
    if (c >= 32 && c <= 63) return font::kClearText[c - 32];
    return font::kClearText[0];
}

} // namespace

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
    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kW, kH);
    if (!tex) return false;
    SDL_ShowCursor(SDL_DISABLE);
    set_ega_palette();
    clear();
    layout();
    return true;
}

void end()
{
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
    for (int j = y < 0 ? 0 : y; j < y + h && j < kH; ++j)
        for (int i = x < 0 ? 0 : x; i < x + w && i < kW; ++i) over[j * kW + i] = colour;
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
    for (; *s; ++s, x += 8) {
        const uint8_t* g = glyph(*s);
        for (int j = 0; j < 8; ++j) {
            const int py = y + j;
            if (py < 0 || py >= kH) continue;
            for (int i = 0; i < 8; ++i) {
                const int px = x + i;
                if (px < 0 || px >= kW) continue;
                if (g[j] & (0x80 >> i)) over[py * kW + px] = fg;
                else if (bg != kClear) over[py * kW + px] = bg;
            }
        }
    }
}

int text(int col, int row, const char* s, uint32_t fg, uint32_t bg)
{
    text_at(col * 8, row * 8, s, fg, bg);
    return col + static_cast<int>(strlen(s));
}

void pointer(int x, int y)
{
    // (an arrow, 7 tall: cream with a black edge)
    static const char* const kShape[9] = {"#", "##", "#.#", "#..#", "#...#", "#....#", "#..###", "#.#", "##"};
    for (int j = 0; j < 9; ++j)
        for (int i = 0; kShape[j][i]; ++i) {
            const int px = x + i, py = y + j;
            if (px < 0 || px >= kW || py < 0 || py >= kH) continue;
            over[py * kW + px] = kShape[j][i] == '#' ? kBlack : kCream;
        }
}

void present(const pic::Canvas* game)
{
    if (!ren) return;
    for (int y = 0; y < kH; ++y)
        for (int x = 0; x < kW; ++x) {
            const uint32_t o = over[y * kW + x];
            shown[y * kW + x] = (o >> 24) ? o : game && game->px ? pal[game->get(x, y)] : 0xFF000000u;
        }
    SDL_UpdateTexture(tex, nullptr, shown, kW * 4);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    if (big) {
        SDL_SetRenderTarget(ren, big);
        SDL_RenderCopy(ren, tex, nullptr, nullptr);
        SDL_SetRenderTarget(ren, nullptr);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, big, nullptr, &dst);
    } else {
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, nullptr, &dst);
    }
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
    cx = (x - dst.x) * kW / dst.w;
    cy = (y - dst.y) * kH / dst.h;
    return true;
}

bool save_bmp(const char* path)
{
    SDL_Surface* s = SDL_CreateRGBSurfaceWithFormatFrom(shown, kW, kH, 32, kW * 4, SDL_PIXELFORMAT_ARGB8888);
    if (!s) return false;
    const bool ok = SDL_SaveBMP(s, path) == 0;
    SDL_FreeSurface(s);
    return ok;
}

} // namespace video
