// The Companion: what the CYD's bigger boards show beside the game - the
// auto-map, the party at a glance, a fight's whole battlefield - here a
// panel that slides in from the RIGHT over the game screen (Tom,
// 2026-11-14; the Y button - from the right the 3D view and the fight's
// field, at the screen's left, stay in sight). The game goes on under it:
// the D-pad still walks, and the map follows.
//
// A wide panel (16:9 and the like; video::side_room) has room beside the
// 4:3 game picture: there the Companion stands to the game's right all
// the time, as on the CYD's bigger boards - the map (a fight: the
// battlefield) above the party. Y there takes it away and brings it back.
// Not during the game's title screens (hold): the game stays centred there.
//
// The Party and Battle views are the engine's own (cyd/src/game/
// companion.*), drawn through its Painter; its 6 x 8 text cells become
// this port's 8 x 8 letters by stretching across by 4 / 3 - which is also
// about what a 4:3 screen does to the rows, so squares stay square.
#pragma once

#include <cstdint>
#include <string>

namespace panel {

// Y: closed -> the first view -> the next ... -> closed. Exploring: Map,
// Party; in a fight: Battle, Party. Beside the game (a wide panel): on / off.
void next();
void close();
bool open();
// It stands beside the game (a wide panel) or under it (a square one) now
bool docked_on();
// The game screen is left: nothing beside it either
void leave();
// Each frame: slides, redraws when what it shows changed, and puts itself
// into the overlay (nothing when closed)
void draw(uint32_t now_ms);
// The game's title screens are up (Tom, 2026-11-14: a blank Companion
// beside them "look bad"): no Companion, the game centred, Y does nothing -
// until the game's party menu (Begin Adventuring) is reached
void hold(bool on);
// The Companion's map from the cluebook (Tom, 2026-11-27: Cluebook Maps): the open game's _CYD folder (the
// scan's pictures there) and whether the switch is on - each frame
void clue(const std::string& cache, bool on);

} // namespace panel
