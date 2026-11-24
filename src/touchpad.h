// On-screen buttons for a touch screen with no controller (Android phones
// and tablets). The same buttons the handhelds have - D-Pad, A, B, X, Y,
// L1, R1, L2, R2, Start, Select - drawn beside the game (landscape) or under
// it (portrait), so everything the port does with buttons works by touch.
// The game picture keeps the room between them; a tap on the picture is a
// tap on the game, as before.
//
// Shown while no controller is in use: a controller's button hides them, a
// touch on the screen brings them back. An overlay (Tom, 2026-11-23): the
// picture is as large as the panel lets it be; a touch on a button is the
// button's, anywhere else the game's; the arrow at the top left puts the
// buttons away and brings them back.
#pragma once

#include <SDL.h>

namespace touchpad {

// The buttons' numbers are main.cpp's Btn order:
// Up, Down, Left, Right, A, B, X, Y, L1, R1, L2, R2, Start, Select
constexpr int kCount = 14;

// A button went down / up
using Emit = void (*)(int button, bool down);
void set_emit(Emit e);

// Shown or not (the picture's room changes: video lays itself out again)
void set_shown(bool on);
bool shown();

// The D-Pad's corners are diagonals (two directions at once): on while a
// fight's fighter moves or aims (main.cpp sets it each frame)
void set_diagonals(bool on);

// Lays the buttons out over the panel (W x H pixels): `use` the part inside
// its safe margins, `pic` where the game's picture is, `strip` the docked
// Companion's room (w 0: none). The picture's room is the picture's - the
// buttons go over it (video.cpp's layout calls this when it is done)
void place(int W, int H, const SDL_Rect& use, const SDL_Rect& pic, const SDL_Rect& strip);

// The height the buttons take at the foot of an upright panel (`use`: its
// part inside the safe margins) - the room a Companion under the game leaves them
int foot(const SDL_Rect& use);

// Controls - Button Size (percent of the standard) and Button Opacity (percent)
void set_style(int size_pct, int opacity_pct);

// The hide / show arrow (top left): the buttons put away, the arrow kept
void set_hidden(bool hidden);
bool hidden();

// Draws the buttons over the frame (renderer output pixels)
void draw(SDL_Renderer* ren);

// A finger event: true when it was the buttons' (not a tap on the game)
bool finger(const SDL_Event& e, int ow, int oh);

// Lets go of everything (the app paused, the buttons hidden)
void release_all();

} // namespace touchpad
