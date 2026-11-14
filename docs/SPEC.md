# Gold Box Game Engine for PortMaster - design

Started 2026-11-14. The engine's own design (formats, rules, the games) is
the CYD repo's `docs/SPEC.md`; this document covers only what the port adds.

## 1. Goal
A PortMaster port that loads on the common Linux retro gaming handhelds.
The player copies their original game files in; the port plays the Gold Box
games from them. Nothing from the games is distributed (CLAUDE.md, hard
rules).

## 2. How the engine is used
The CYD engine was written to be ported: its game code (`cyd/src/engine`,
`cyd/src/game`) is plain C++ that asks the machine only for what
`cyd/src/platform/*.h` declares, and shows a 320 x 200 canvas that a front
end presents. This port:

| Engine asks for | Port gives |
|--|--|
| `plat::now_ms`, `log`, `random_seed` | SDL's clock, stdout (the launch script keeps it in `log.txt`), the time |
| `plat::open / exists / remove / rename / mkdir` | stdio under `games/`, names matched ignoring case |
| `plat::free_heap / largest_block`, `memlog::` | "can't tell" / plenty - no memory limits here |
| `plat::vga_screen` | true: VGA games in 256 colours |
| `audio_play / stop / hold ...` | an SDL audio device pulling from the engine's filler |
| a front end: `play::open / tick / act / nav / tap / input_key ...` | `src/main.cpp` |

The engine is a git submodule pinned to a commit, built unchanged. What the
CYD front end (`cyd/src/ui`, LovyanGFX) does for the boards, this port does
again for handhelds; nothing under `cyd/src/ui`, `hal`, `boards` is built.

## 3. Files on the device
    ports/Gold Box Game Engine.sh      launch script
    ports/goldbox/
        goldbox.aarch64 / .x86_64      the port
        games/<game folder>/           the player's games (GOG / Steam folders as they are)
        games/_CYD/                    the engine's own (made on the device; may be deleted)
        conf/settings.cfg              the port's settings
        log.txt                        the last run's log
The game folders are found as the CYD's card scan finds them: any folder
holding .DAX files (in it, or one or two folders down) or .TLB / .GLB
libraries; the game from the folder's name. Saves go where the originals
put them, inside the game's folder.

## 4. Controls
Rule (Tom, 2026-11-14): everything with D-Pad, A, B, X, Y, L1, L2, R1, R2,
Start, Select - most handhelds have no touch screen, many no sticks. Sticks
/ touch / mouse / keyboard are extras.

The engine already takes input the way a button device needs: movement
keys (`play::act`), a highlight that moves over every menu and list
(`play::nav`) with Select acting on it (`play::nav_point`), a fight's
direction keys (`play::fight_dir`, `fight_select`), typed keys
(`play::input_key`).

| Button | Exploring | Menus, lists, text | A fighter moving / aiming | Keyboard up |
|--|--|--|--|--|
| D-Pad | forward, turn left / right, turn around | move the highlight | step / aim; two together = diagonal | move over the keys |
| A | choose the highlighted menu word | choose / continue | end the move / Target | type the key |
| B | Back (below) | Back (below) | - | delete |
| L1 / R1 | menu word left / right | highlight left / right | up-left / up-right | - |
| L2 / R2 | list line up / down (the party list) | list line up / down | down-left / down-right | - |
| X | Area map | - | - | space |
| Y | the Companion | the Companion | the Companion | Done |
| Select | Pointer Mode | Pointer Mode | Pointer Mode | Pointer Mode |
| Start | the port's menu | the port's menu | the port's menu | the port's menu |

- **Back** (B; Tom, 2026-11-14: "Using B as an Exit button is a great
  idea"): first the engine's own Esc (`play::back` - out of View, Modify,
  a shop, Load Which Game, a Yes / No as No ...); when that has nothing to
  back out of, a tap on the menu line's **Exit** word if it has one
  (`play::exit_point`, added to the engine for this in CYD v0.196.52).
  Never the party menu's "Exit to DOS" - leaving a game is the Start
  menu's Exit Game.
- **L2 / R2** (Tom, 2026-11-14): no side-steps on the buttons - L2 / R2
  move the list's line while L1 / R1 move the menu's word, so both of a
  screen's highlights are at hand and the D-pad keeps walking. Look is on
  the game's menu (no button).
- **The Companion** (Y; Tom, 2026-11-14; `src/panel.*`): slides in from
  the left over the game screen (208 of 320 pixels), the game going on
  under it - the D-pad still walks and the map follows. Y again: the next
  view, then away. Exploring: Map (walls, doors gold, locked red, secret
  doors S, the party's arrow), Party; in a fight: Battle, Party. Party
  and Battle are the engine's own views (`game/companion.*`) through its
  Painter, stretched across by 4 / 3 for 8-pixel letters.
- "Press a key" text goes on with A only - not the D-pad (a held Forward
  would skip the story).
- **D-Pad Walks** (menu switch, on): off = the D-Pad always moves the
  highlight (walking then needs the menu words).
- **Pointer Mode** (Select; acts when Select is let go, so Select + Start
  stays PortMaster's quit): an arrow on the game screen, the D-Pad moves it
  (slow at first, faster held), A taps there (`play::tap`, exactly a CYD
  stylus tap), B or Select ends it. It is the guarantee that anything a
  tap can do on a CYD can be done with buttons, even where the highlight
  doesn't go.
- A fight's diagonals from the D-Pad: a direction waits 70 ms for a second
  one before the step is taken.
- Held D-Pad repeats (380 ms, then every 110 ms).
- **Confirm Button** (menu): A / B swapped, for handhelds whose labels are
  the other way round.
- Extras: either stick moves the pointer; a mouse or touch screen taps
  directly (also the on-screen keyboard's keys); a PC keyboard: arrows,
  Enter, Esc, Tab / F1 = Start, PgUp / PgDn = L1 / R1, Home / End = L2 / R2,
  F2 = X, F3 = Y, F4 = Select, and typing when the game asks for text.

## 5. The screen
The game's 320 x 200 canvas, with the port's overlay on top (same size; a
40 x 25 grid of 8 x 8 cells in the engine's own Clear font - the game's
font isn't known before a game is open). Fit (menu: Display):
- **Original 4:3** (default) - as a DOS monitor showed it; fills a 4:3
  panel (640 x 480, 480 x 320 nearly, 720 x 720 letterboxed).
- **Square Pixels** - the largest whole multiple.
- **Stretch** - the whole panel.
Uneven scaling goes through a whole-multiple copy first, then smoothed, so
pixels stay even. Every frame redraws the whole canvas (no dirty rows - a
handheld has the speed).

## 6. The port's own screens
- **Game list** (as the CYD's library): a game a page - its box art from
  the player's Steam `Launcher` folder in games/ (read at a quarter size by
  the engine's `jpeg_thumb`, fitted to the page at twice the canvas's
  sharpness, `src/art.*`, `video::set_backdrop`), the name big across the
  foot; Left / Right (or L1 / R1) turn the pages, round; A plays. No
  Launcher folder: the name on black. With no games, how to add them.
- **Music** (the engine's, v0.197): a later game's songs are made into
  files in `_CYD/<folder>/` at that game's first start ("Making the game's
  music" on the loading screen - the CYD does it in its card scan), then
  played by the engine; Start - Music: On / Off.
- **Game Sound**: Tandy, PC Speaker, then Darkness / Gateway / Treasures
  (that game's recordings lent to the older games, when its folder is in
  games/), Off; those three games always their own (or Off).
- **Loading** message at once when a game is chosen.
- **Menu** (Start): Back to Game, Controls, Display, Game Sound (Tandy /
  PC Speaker / Off), Volume, Game Font (Original / Clear / Darkness / Clear
  Caps), D-Pad Walks, Confirm Button, Look for Games Again, Check for
  Updates, Send Logs, Log Server, Exit Game, Quit. The game is paused while it is open.
- **On-screen keyboard** over the top of the game screen (the game's own
  text area stays visible) while `play::input()` asks.
- **Messages** stay until a button.
- **Journal mentions**: for now a message naming the entry to read in the
  player's own journal (section 8).

## 7. Building and packaging
CMake. CI (GitHub Actions) on every push: a check build with sanitizers +
`tools/smoke_test.py`; release builds for aarch64 (cross-compiled) and
x86_64 in an Ubuntu 20.04 container (glibc 2.31, SDL 2.0.10 headers);
`tools/package.py` makes `goldbox.zip` in PortMaster's layout. The port
reads the controller through SDL's game controller API with PortMaster's
mapping (`SDL_GAMECONTROLLERCONFIG`); gptokeyb runs only for the quit
hotkey.

## 7b. Installing, updating, logs (as Pocket Curator)
- **Releases**: GitHub releases of this repo, pre-releases while in
  development. Assets `goldbox_port-vX.Y.Z.zip`, `.sha256`,
  `GoldBox.Installer.sh`.
- **Installer**: one file copied into `roms/ports` and run from Ports;
  finds the newest release (pre-releases included), downloads, checks the
  SHA-256 and the zip, unpacks into the ports folder.
- **Check for Updates** (Start menu): `tools/update.sh check` (curl, the
  releases list; the first `goldbox_port-v*.zip` is the newest) -> a newer
  version is offered -> `update.sh fetch` stages it in `.update/staged`,
  READY last -> Restart Now (or the next launch): the launch script copies
  the staged port over the installed one, replaces itself last, removes
  `.update`, starts the new version. `update.log` records it.
- **Send Logs** (Start menu): Log Server (typed on the port's keyboard,
  kept in settings) + `tools/sendlogs.sh` (curl -T to WebDAV).
- The helper scripts run on a thread (`src/task.*`); the screen shows a
  notice until they end. Buttons only: all of it from the Start menu.

## 8. Roadmap
1. **v0.1 - first run** (this): engine built in, game list, play, buttons,
   keyboard, menu, package, CI.
2. **First play on a device** (Tom): controller mapping, sound, speed,
   scaling on his handhelds; fix what that shows.
3. **Play-through with game files** in the session (headless `--script`
   runs): every screen kind reached by buttons - party creation, the icon
   editor, shops, camp and spells, fights and aiming, saving / loading.
4. **The engine's own features the CYD front end hosts** (each needs a
   button-driven screen here): the journal (entries and the book from the
   player's PDF / Gold Box Companion - the engine's `journal::` code is
   portable, the viewer is not), the auto-map and the Companion's Party /
   Battle views (`game/companion.*` through its Painter; room beside the
   game on wide panels), Cheats, Quick Actions, the comfort switches
   (Game Fixes, Show Rolls, ...), later games' recorded sounds lent to the
   older games, logs.
5. **Game art in the list** (the player's GOG icon / Steam box art, made
   on the device as the CYD scan does).
6. **PortMaster submission** (Tom's call): screenshot, testing thread,
   their checks.
