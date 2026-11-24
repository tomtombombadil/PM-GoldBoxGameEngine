// The Android app's own controls round the game (Tom, 2026-11-24, the
// redesign: "It needs a serious glow up ... not like a native Android app").
// The app (APK repo, GoldBoxActivity / Shell.java) draws the pad, the edge
// handles and the panels in Android's views over SDL's surface; this front
// end draws the game and the Companion's pictures in the rectangles the app
// gives it. Android only: everywhere else nothing here does anything, and
// on() stays false - the handhelds are untouched by it.
//
// The app -> here: JNI calls (any thread) become SDL events, taken on the
// game's thread by main's handle(). Here -> the app: what is on screen, so
// it shows the Companion and the pad when they mean something.
#pragma once

#include <SDL.h>

namespace shell {

// The app lays the screen out (it has sent a layout)
bool on();

// After SDL_Init: the event type the app's calls come as
void begin();

// An event of the app's: true when it was (taken here). A pad button goes to
// `press` (main's Btn numbers, down / up); a layout goes to video
bool take(const SDL_Event& e, void (*press)(int button, bool down));

// What is on screen (told to the app when it changes)
enum class State : int { Pages = 0, Titles = 1, Play = 2, Fight = 3 };
void state(State s);

// Y on a controller: the app's Companion out / away
void companion();

// The app's menu drawer (stage 2 of the redesign): the front end's menu stays
// the one that decides (its pages, lines and the buttons that move about it -
// a controller still drives it); the app shows it in Android's views and its
// taps come here as ops for `fn` (main.cpp): 0 close, 1 choose line i, 2 change
// line i by value (-1 / +1), 3 back a page, 4 set line i to value (a level)
enum MenuOp : int { MenuClose = 0, MenuChoose = 1, MenuChange = 2, MenuBack = 3, MenuSet = 4 };
void set_menu(void (*fn)(int op, int index, int value));
// What the menu shows now, as text for the app (sent only when it changed)
void menu_state(const char* text, size_t len);

} // namespace shell
