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

} // namespace shell
