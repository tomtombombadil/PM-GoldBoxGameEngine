// The screen: the game's 320 x 200 canvas (palette indexes) and the port's
// own overlay (menus, the on-screen keyboard, messages - 320 x 200 too, a
// 40 x 25 grid of 8 x 8 text cells like the games'), scaled to whatever
// the handheld's panel is.
#pragma once

#include <cstdint>

#include "engine/picture.h"

struct SDL_Rect;

namespace video {

constexpr int kW = pic::kScreenW, kH = pic::kScreenH;
constexpr int kMaxW = 576;      // the overlay at its widest (a 21:9 panel)

// How the 320 x 200 picture fills the panel
enum class Fit : uint8_t {
    Original,       // 4:3, as a DOS monitor showed it (the default)
    Square,         // square pixels at the largest whole multiple
    Stretch,        // the whole panel
    // (Tom, 2026-11-23: "a 4:3+companion right, and a 4:3+companion below. In
    // 4:3 the companion slides out. On the 4:3+ options, the companion is
    // where specified")
    Right,          // 4:3, the Companion beside it at the right (the game made smaller when it must)
    Below,          // 4:3, the Companion under it
    Count
};
const char* fit_name(Fit f);

// window_w / h: 0 = the whole screen (a handheld); else a window (a PC)
bool begin(int window_w, int window_h);
void end();
void set_fit(Fit f);
// The window changed size (a PC)
void resized();
// Makes the window that size (the test script's "resize:WxH": a panel
// whose real size arrives after the start)
void set_window(int w, int h);

// The palette the canvas's values are shown in
void set_palette(int index, const pic::Rgb& c);
void set_ega_palette();

// ---- The port's own pages on a wide panel (Tom, 2026-11-14: the game list
// and the scan "resize to match the screen" - 16:9 and the like)
// On: the overlay and the backdrop cover the WHOLE panel - the same
// 1.2-tall pixels, more of them across. Everything is still laid out in
// the 320 columns, which now stand in the middle: x runs from -margin()
// to kW + margin(). Off (a game on screen): 320 across, as the games are.
void set_wide(bool on);
// Goes up whenever the picture's size or shape on the panel changes (the
// panel's real size can arrive after the start; a PC window resized): what
// was made for a shape - the game list's pictures - is checked again then
unsigned shape_serial();
int margin();           // the columns beside the 320, each side (0: a 4:3 panel, or not wide)
int back_w();           // the backdrop picture's width now: twice (kW + 2 * margin())
// Smaller letters for a backdrop picture (12 x 12, smoothed; black behind):
// into buf (bw x bh of 0xAARRGGBB) at (x, y)
void small_text(uint32_t* buf, int bw, int bh, int x, int y, const char* s, uint32_t fg);

// ---- The overlay (colours: 0xAARRGGBB; alpha 0 = the game shows through)
constexpr uint32_t kClear = 0;
constexpr uint32_t kBlack = 0xFF000000, kNavy = 0xFF101838, kKey = 0xFF182652, kEdge = 0xFF465C96;
constexpr uint32_t kCream = 0xFFF0E6C8, kCyan = 0xFF55FFFF, kMuted = 0xFF96917F, kGold = 0xFFE8B840, kWarn = 0xFFDC5A46;
constexpr uint32_t kLit = 0xFF96701E;
void clear(uint32_t colour = kClear);
void fill(int x, int y, int w, int h, uint32_t colour);
void frame(int x, int y, int w, int h, uint32_t colour);
// Text by cell (col 0-39, row 0-24) in the engine's Clear font; returns
// the column after it. bg kClear: ink only.
int text(int col, int row, const char* s, uint32_t fg, uint32_t bg = kClear);
// The same from a pixel position
void text_at(int x, int y, const char* s, uint32_t fg, uint32_t bg = kClear);
// Twice the size (16 x 16 a letter), a black edge round it: names over pictures
void text_big(int x, int y, const char* s, uint32_t fg);
// A letter's 8 rows (bit 7 the left pixel)
const uint8_t* glyph(char ch);
// A picture into the overlay (w x h of 0xAARRGGBB, alpha 0 skipped)
void blit(int x, int y, int w, int h, const uint32_t* argb);
// The colour a canvas value shows as now
uint32_t palette(int value);

// A picture behind the overlay where it is see-through, in place of the
// game (the game list's box art): back_w() x kBackH pixels - twice the
// canvas's, for the panel's sharpness (kBackW across unless set_wide) -
// shown from canvas row `top` down. nullptr: none.
constexpr int kBackW = kW * 2, kBackH = kH * 2;
// How tall a canvas pixel is shown, in thousandths of its width (1200 on
// a 4:3 fit): a picture made for the backdrop is squeezed by it
int pixel_aspect();
void set_backdrop(const uint32_t* argb, int top = 0);

// A wide panel (16:9 and the like): the room beside the 4:3 game picture
// shows the Companion - two pictures of kSideW x kSideH (0xAARRGGBB), one
// above the other, the game moved to the left. side_room: the panel has
// that room (Original 4:3 fit only). set_side(nullptr, nullptr): none.
constexpr int kSideW = 208, kSideH = kH;
bool side_room();
void set_side(const uint32_t* top, const uint32_t* bottom);
// How many of the party picture's rows its members use (from the top; the rest is empty) - the app's column
// shows it as wide as the map, cut below them (Tom, 2026-11-25)
void set_side_used(int rows);

// A square or tall panel (1:1, 720 x 720 and the like - Tom, 2026-11-15):
// the 4:3 game picture leaves room UNDER it; the game moves to the top and
// the Companion fills that room - one picture of kBelowW x kBelowH square
// pixels (0xAARRGGBB), as large as fits. below_room: the panel has that
// room (Original 4:3 fit only). set_below(nullptr): none.
constexpr int kBelowW = 600, kBelowH = 150;
bool below_room();
void set_below(const uint32_t* px);

// The pointer's arrow, its tip at (x, y)
void pointer(int x, int y);

// Shows a frame: the canvas (nullptr: black) under the overlay
void present(const pic::Canvas* game);

// The panel's safe margins in pixels (a phone's camera cutout - Android):
// the picture, the on-screen buttons and the Companion keep out of them
void set_safe(int left, int top, int right, int bottom);

// The Android app lays the screen out (shell.*, Tom's redesign 2026-11-24):
// the game's room and the Companion's (renderer pixels; comp w 0: none),
// view - the Companion's pictures: 0 both, 1 the map / battlefield, 2 the
// party. Once given, these decide where everything goes (the safe margins,
// Display - Picture's Right / Below, the on-screen buttons no longer do)
void set_shell(const SDL_Rect& game, const SDL_Rect& comp, int view);

// The renderer's size in pixels (the whole panel); the window's in its points
void output_size(int& w, int& h);
void window_size(int& w, int& h);

// A point of the window -> the canvas's pixel (clip false: any point, the
// canvas's scale carried on past its edges - a wide page's margins, a swipe) (false: outside the picture)
bool to_canvas(int wx, int wy, int& cx, int& cy, bool clip = true);
// Saves what the panel shows (as scaled) as a BMP
bool save_bmp(const char* path);

} // namespace video
