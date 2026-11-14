// The Companion: what the CYD's bigger boards show beside the game - the
// auto-map, the party at a glance, a fight's whole battlefield - here a
// panel that slides in from the left over the game screen (Tom,
// 2026-11-14; the Y button). The game goes on under it: the D-pad still
// walks, and the map follows.
//
// The Party and Battle views are the engine's own (cyd/src/game/
// companion.*), drawn through its Painter; its 6 x 8 text cells become
// this port's 8 x 8 letters by stretching across by 4 / 3 - which is also
// about what a 4:3 screen does to the rows, so squares stay square.
#pragma once

#include <cstdint>

namespace panel {

// Y: closed -> the first view -> the next ... -> closed. Exploring: Map,
// Party; in a fight: Battle, Party.
void next();
void close();
bool open();
// Each frame: slides, redraws when what it shows changed, and puts itself
// into the overlay (nothing when closed)
void draw(uint32_t now_ms);

} // namespace panel
