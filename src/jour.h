// The Adventurer's Journal on the handheld (Tom, 2026-11-14: "a very big
// deal to being able to play these games without a physical print-out").
//
// Making: as the CYD engine's card scan does it, with the engine's own
// code (cyd/src/engine/journal.*, pdf.*, pdf_text.*) - from the player's
// journal PDF in the game's folder (GOG: scanned pages, cut into entries
// by the engine's tables; Steam / SNEG: typeset, its text read) and the
// Gold Box Companion's files where the release has them. The result is
// JOURNAL.DAT (and BOOK.TXT) in games/_CYD/<game>/, made at the game's
// first start. Nothing of a journal is in the port.
//
// Showing: entry N when the game mentions it (and again from the Start
// menu's Journal list). Text entries in the game's own look - its border,
// its font, on the borrowed game canvas - as on the CYD; entries with
// scanned pictures on a page of their own at twice the canvas's sharpness.
// Buttons only: Up / Down a line, L2 / R2 a page, X the size, Left /
// Right across a zoomed picture, B (or A at the end) back.
#pragma once

#include <string>

#include "engine/picture.h"

#include "library.h"

namespace jour {

using Say = void (*)(const char* line, bool replace, void* user);

// Makes the game's journal files if they aren't there (or are an older
// make's). Quick when there is nothing to do.
void prepare(const library::GameDir& g, const std::string& cache_dir, Say say);

// Opens entry (kind 'J' journal entry, 'T' tavern tale, 'P' proclamation,
// 'I' the introduction; number). False: it can't be shown - why says what
// to tell the player.
bool open(const std::string& cache_dir, char kind, int number, pic::Canvas& game, std::string& why);
bool active();
void close(pic::Canvas& game);

// The viewer's keys
void scroll(int lines);         // a line up / down (pictures: a few rows)
void page(int pages);
void zoom();
void pan(int step);
bool at_end();

// Draws the viewer (the overlay's hint line; the picture page as the
// backdrop). True: the game canvas is to be shown under the overlay (a
// text entry in the game's look); false: the backdrop.
bool draw(pic::Canvas& game);

} // namespace jour
