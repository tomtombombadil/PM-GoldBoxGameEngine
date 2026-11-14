// The screen: the game's 320 x 200 canvas (palette indexes) and the port's
// own overlay (menus, the on-screen keyboard, messages - 320 x 200 too, a
// 40 x 25 grid of 8 x 8 text cells like the games'), scaled to whatever
// the handheld's panel is.
#pragma once

#include <cstdint>

#include "engine/picture.h"

namespace video {

constexpr int kW = pic::kScreenW, kH = pic::kScreenH;

// How the 320 x 200 picture fills the panel
enum class Fit : uint8_t {
    Original,       // 4:3, as a DOS monitor showed it (the default)
    Square,         // square pixels at the largest whole multiple
    Stretch,        // the whole panel
    Count
};
const char* fit_name(Fit f);

// window_w / h: 0 = the whole screen (a handheld); else a window (a PC)
bool begin(int window_w, int window_h);
void end();
void set_fit(Fit f);
// The window changed size (a PC)
void resized();

// The palette the canvas's values are shown in
void set_palette(int index, const pic::Rgb& c);
void set_ega_palette();

// ---- The overlay (colours: 0xAARRGGBB; alpha 0 = the game shows through)
constexpr uint32_t kClear = 0;
constexpr uint32_t kBlack = 0xFF000000, kNavy = 0xFF101838, kKey = 0xFF182652, kEdge = 0xFF465C96;
constexpr uint32_t kCream = 0xFFF0E6C8, kMuted = 0xFF96917F, kGold = 0xFFE8B840, kWarn = 0xFFDC5A46;
constexpr uint32_t kLit = 0xFF96701E;
void clear(uint32_t colour = kClear);
void fill(int x, int y, int w, int h, uint32_t colour);
void frame(int x, int y, int w, int h, uint32_t colour);
// Text by cell (col 0-39, row 0-24) in the engine's Clear font; returns
// the column after it. bg kClear: ink only.
int text(int col, int row, const char* s, uint32_t fg, uint32_t bg = kClear);
// The same from a pixel position
void text_at(int x, int y, const char* s, uint32_t fg, uint32_t bg = kClear);
// The pointer's arrow, its tip at (x, y)
void pointer(int x, int y);

// Shows a frame: the canvas (nullptr: black) under the overlay
void present(const pic::Canvas* game);

// A point of the window -> the canvas's pixel (false: outside the picture)
bool to_canvas(int wx, int wy, int& cx, int& cy);
// Saves what was last presented (320 x 200, before scaling) as a BMP
bool save_bmp(const char* path);

} // namespace video
