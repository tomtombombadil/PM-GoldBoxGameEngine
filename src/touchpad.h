// On-screen buttons for a touch screen with no controller (Android phones
// and tablets). The same buttons the handhelds have - D-Pad, A, B, X, Y,
// L1, R1, L2, R2, Start, Select - drawn beside the game (landscape) or under
// it (portrait), so everything the port does with buttons works by touch.
// The game picture keeps the room between them; a tap on the picture is a
// tap on the game, as before.
//
// Shown while no controller is in use: a controller's button hides them, a
// touch on the screen brings them back.
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

// The room the game picture may use, within the renderer's ow x oh
// (everything when not shown); the buttons are laid out for that size
void area(int ow, int oh, int& ax, int& ay, int& aw, int& ah);

// Draws the buttons over the frame (renderer output pixels)
void draw(SDL_Renderer* ren);

// A finger event: true when it was the buttons' (not a tap on the game)
bool finger(const SDL_Event& e, int ow, int oh);

// Lets go of everything (the app paused, the buttons hidden)
void release_all();

} // namespace touchpad
