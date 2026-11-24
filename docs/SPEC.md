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



*The Android app* (Tom, 2026-11-23) is this front end built into an APK in its own repo,
`tomtombombadil/APK-GoldBoxGameEngine` (this repo a submodule there). Its parts here are `#ifdef __ANDROID__`
(the Games Folder line, Back = B, no update / log scripts) and the touch below.
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
  At "Pick Character" (the party menu's Drop / Modify / View / Remove
  ... in every game but PoolRad and Curse) B goes back to the
  party menu, as the originals' Esc (CYD 0.200.4). At a Yes / No, B is No
  (ours - the originals' Esc does nothing there).
- **Overland maps with a marker** (Champions, Death Knights, Darkness;
  v0.9.1): the menu is "Move Encamp" - Move, then the D-pad steps (the
  engine's `walking()` is true in Move mode since CYD 0.200.1), B = Exit.
  The Dark Queen's and PoolRad's / Gateway's overland: the D-pad steps at
  once. Where the engine's keys step instead of turning (`play::move_keys`,
  v0.12.12): PoolRad's wilderness - Up / Down / Left / Right = north /
  south / west / east; Champions' and the Dark Queen's maps the same, and
  L1 / R1 = north-west / north-east.
- **L2 / R2** (Tom, 2026-11-14): no side-steps on the buttons - L2 / R2
  move the list's line while L1 / R1 move the menu's word, so both of a
  screen's highlights are at hand and the D-pad keeps walking. Look is on
  the game's menu (no button).
- **The Companion** (Y; Tom, 2026-11-14; `src/panel.*`): slides in from
  the **right** over the game screen (208 of 320 pixels; from the right
  the 3D view and a fight's field, at the screen's left, stay in sight -
  Tom), the game going on
  under it - the D-pad still walks and the map follows. Y again: the next
  view, then away. Exploring: Map (walls, doors gold, locked red, secret
  doors S - the engine's `play::map_secret` - the party's arrow; on
  PoolRad's wilderness the lines are where a step is refused and the
  scripts' places are orange squares, the legend "Place"), Party; in a fight: Battle, Party. Party
  and Battle are the engine's own views (`game/companion.*`) through its
  Painter, stretched across by 4 / 3 for 8-pixel letters.
  **Wide panels** (Tom, 2026-11-14: "16:9 devices and similar ... to the
  right of the regular game area, similar to how it works on the higher
  resolution CYD screens"): when the Original 4:3 picture leaves 200
  device pixels or more beside it (`video::side_room` - 16:9, 16:10, 3:2
  at 1.5 or wider), the game stands at the left and the Companion fills
  the rest all the time - Map (a fight: Battle) above Party
  (`video::set_side`, two 208 x 200 pictures smoothed into the strip). Y
  there takes it away (the game centred) and brings it back; no slide-out.
  **Not on the game's title screens** (Tom, 2026-11-14, v0.9.5: a blank
  Companion beside the intro "look bad"): until the party menu (Begin
  Adventuring) is reached the game stays centred with nothing beside it
  and Y does nothing, on every panel (`panel::hold`; the engine's
  `play::titles()`). A demo (Play / Demo) has the Companion.
  **Square panels** (Tom, 2026-11-15; 1:1, 720 x 720 - RG CubeXX, RGB30):
  when the Original 4:3 picture leaves 150 device pixels or more UNDER it
  on a panel 600 or more across (`video::below_room`), the game stands at
  the top and the Companion fills the room under it all the time - one
  picture of 600 x 150 square pixels as large as fits (1.2 x on 720 x
  720): the map at 9 pixels a square with its legend beside it (a fight:
  the battlefield) at the left, the party's rows at the right
  (`panel.cpp` `draw_below`; the engine's rows through a Painter that
  stretches only past the icons, so those keep their shape). Y takes it
  away (the game centred) and brings it back, as beside the game.
  The views carry no "Map Party" heading and the map's legend is the words
  "Locked" (red) and "Secret" (cyan, as the S on the map) - the CYD's
  (0.200.7 / .8; Tom, 2026-11-14); the party's names white (the chosen
  one) / cyan.
  Other fits (Square Pixels, Stretch) and 4:3 panels: the slide-out.
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
  On **Knulli** (A and B reported the other way round - Tom's RG40XX V) it
  starts on B: the launch script finds the firmware (`CFW_NAME`,
  `/etc/os-release`, `/usr/share/knulli`) and passes `--ab-reversed`; applied
  once (`ab_set` in settings.cfg), the player's own choice stands after.
  Every hint and the Confirm Button line name the button as it is MARKED
  on the device (v0.10.9): there the program's B is the button marked A,
  so the line reads "Confirm Button: A".
- Extras: either stick moves the pointer; a mouse or touch screen taps
  directly (also the on-screen keyboard's keys); a PC keyboard: arrows,
  Enter, Esc, Tab / F1 = Start, PgUp / PgDn = L1 / R1, Home / End = L2 / R2,
  F2 = X, F3 = Y, F4 = Select, and typing when the game asks for text.

### Touch (Android; a PC with `--touchpad`) - Tom, 2026-11-23
"Compatible with touch and/or a controller (via bluetooth or USB) or an onscreen overlay representing the
controller". All three, any mix; nothing is touch's alone (the rule above stands):
- **A controller**: as everywhere.
- **The on-screen buttons** (`src/touchpad.*`) - **an overlay** (Tom, 2026-11-23: "the controls need to be an
  overlay, and the game needs to be maximized"): the picture keeps all its room, the buttons lie over it,
  see-through. Landscape: the D-Pad and the face buttons in the free band beside the game when it is wide enough,
  else over its sides, their foot above the game's menu line (canvas rows 23-24 stay clear); L2 L1 and MOUSE
  (Select) top left after the hide / show arrow, R1 R2 and MENU (Start) top right. Portrait: the game at the
  top, the buttons at the foot. **A touch on a button is the button's, anywhere else the game's** - only the
  drawn shape (the D-Pad's corners are the game's outside a fight). **The arrow at the top left** hides /
  shows them (`pad_hidden`). Start - Controls - **On-Screen Buttons: Auto / Always / Never** (`touch_pad`),
  **Button Size** Small / Medium / Large / Huge (`pad_size`), **Button Opacity** 20 - 100% (`pad_alpha`).
- **Touching the port's own screens** (`touch_tap` / `touch_swipe` in main.cpp - each the buttons a player
  would press there): a list's line tapped = picked and chosen at once (A) - one tap (Tom, 2026-11-23: "be
  more Android-friendly"); outside a page's box or its title = B;
  a swipe up / down scrolls (the finger up = on down the list), across = Left / Right (a setting's value; the
  game list's next game; a journal page). The game list: its sides = Left / Right, the middle = A, the title
  bar = Start. A notice: tapped = A; a Yes / No one: its foot's left = Yes, right = No, outside = No. The
  journal: the left third a page back, else A. Quick Actions: a member; a switch (flipped); the top line.
  Two fingers tapped together = Start. Android's Back = B. In the game a tap is the game's (pointer_tap: the
  highlight, the on-screen keyboard), as on the CYD boards.
- **The on-screen D-Pad's diagonals** (Tom, 2026-11-23): while `play::fight_pad()` (a fighter moving or aiming)
  its four corners are diagonals - both directions held, as a controller's two buttons; arrowheads in the
  corners show it. Elsewhere a corner is the nearer arm. Start is labelled MENU, Select MOUSE (Tom).
- In the game a tap moves the game's highlight to what it picks before acting (engine 0.215.1), as the
  buttons would.
- **Display - Screen Rotation** (Android): Auto (the device's, its rotation lock too) / Landscape / Portrait
  (`rotation` in settings.cfg; `rotation_apply`, the activity's requested orientation). The window is made
  resizable there, else SDL locks the screen to its first shape.
- Script steps `touch:x:y` / `swipe:dx:dy` (canvas pixels) for the smoke test.
- **A camera cutout / notch** (Android; Tom's Pixel 10a, 2026-11-23): its safe margins (`video::set_safe`,
  from the activity's `cutoutInsets()`, asked about once a second by `safe_check`) are left black; the picture,
  the on-screen buttons and the Companion are laid out in the rest (`usable()` in video.cpp).

## 5. The screen
**Display - Picture** (Tom, 2026-11-23): the handhelds - Original 4:3 / Square Pixels / Stretch (`fit` 0 / 1 / 2),
as always. **Android only**: 4:3 / **4:3 + Companion Right** / **4:3 + Companion Below** / Square Pixels / Stretch
(`fit` 0 / 3 / 4 / 1 / 2). **The Companion as large as its room allows, at its own
shape** (Tom, 2026-11-23): Below on a tall panel uses the two pictures beside the game's, side by side under it,
down to the on-screen buttons (`below_pair`, `touchpad::foot`) - when they come out 1.3 times the wide strip's
area or more; else the wide 600 x 150 strip. Original docks the Companion by itself
where there is room on a handheld (`kAutoDock`), never on Android (the slide-out on Y). Right / Below dock it
there whatever the panel: the game made smaller if it must (Right: the pair's width is 4/3 + 0.43 of the
game's height; Below: the game's width + 1/4 of it in height); with the on-screen buttons the Right pair goes
to the right, the left over room for the D-Pad. Android starts on Right (`fit_set`, once).


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

**The port's own pages fill the panel** (Tom, 2026-11-14; `video::set_wide`):
the game list and the scan (and the menu over them) aren't held to 4:3 -
on a panel wider than 4:3 the overlay is wider than 320 (the same 1.2-tall
pixels; 426 across on 16:9, up to 576) and the backdrop twice that; pages
are laid out in the 320 columns standing in the middle, with
`video::margin()` more each side for what spans the panel (header, box
art, arrowheads). The game list's picture is made by the scan at that
width (`ART.BIN` keeps its width; another panel: made again). With a game
on screen the overlay is 320 again.

## 6. The port's own screens
- **Game list** (as the CYD's library): a game a page - its box art from
  the player's Steam `Launcher` folder in games/ (read at a quarter size by
  the engine's `jpeg_thumb`, fitted to the page at twice the canvas's
  sharpness, `src/art.*`, `video::set_backdrop`), the name big across the
  foot; Left / Right (or L1 / R1) turn the pages, round; A plays. No
  Launcher folder (a GOG player; Tom, 2026-11-14: "the logo from the DLL,
  just like the CYD version"): the game's own logo from its folder - the
  best of `goggame-*.ico`, `goggame-*.dll`, another `.ico` (Steam's
  `game.ico`; the CYD library's order), read by the engine's
  `icon::find` / `render` (`art::icon`), square on the screen, centred
  above the name. Neither: the name on black. With no games, how to add
  them.
- **Finding the games** (the CYD's card scan; Tom, 2026-11-14: "built on
  the first time the game is discovered ... rebuilt if the user chooses
  to rediscover the games"; "That ALL gets pre-processed during the
  initial game discovery ... no loading or processing screens"):
  everything made from a game's files - the program's unpacked copy
  (`play::prepare_program`), the game list's picture (`art::make` ->
  `ART.BIN`), its journal, its music - is made when the game is
  **found**: at the
  start, every playable game folder without the mark
  `_CYD/<folder>/PMSCAN.TXT` (`discover()` in main.cpp). The screen lists
  what it does in Tom's layout (2026-11-23, v0.12.11: "Found <game>" at the
  left, what is made for it indented two, a step's heading ending in a colon
  with its counter indented four under it and counting in place -
  "  Processing the journal:" / "    page n of m", "  Rendering the <game>
  music:" / "    song n of m... (s of music, s to make)"; what a copy lacks
  as "  No picture / No journal / No music: ...") in smaller letters (12 x 12, smoothed, on the
  backdrop's sharper picture: 52 across on 4:3, 69 on 16:9) and stays
  until A. **Start - Rescan Game Files** does every game anew (journal files removed first, music
  forced). Never at a game's start or on first use: a game starts with no
  loading screen, and the game list only reads `ART.BIN` (`art::load`).
  The picture is made for the screen's shape: a game found before whose
  `ART.BIN` isn't this shape's gets just its picture made again on the
  scan's screen ("Preparing the picture for ...", no wait), as the CYD's
  "Preparing Pictures". **When**: not at the start - a handheld's panel
  can report its real size only after the first frames (Tom's RGB10 MAX3,
  v0.6.2: pictures made for the wrong size at every start, then not
  shown) - but by the game list (`pictures_follow`), once the picture's
  shape (`video::shape_serial`) has stood still 300 ms and whenever it
  changes after (Settings - Display, a window resized). The log has a
  "[video] panel W x H" line for every change.
- **Music** (the engine's, v0.197): a later game's songs are made into
  files in `_CYD/<folder>/` by that scan, then
  played by the engine; Settings - Music: On / Off.
- **Game Sound**: Tandy, PC Speaker, then Darkness / Gateway / Treasures
  (that game's recordings lent to the older games, when its folder is in
  games/), Off; those three games always their own (or Off).
- **Menu** (Start; `menu_*` in main.cpp; since v0.12.0 the CYD's tree -
  Tom, 2026-11-17, engine 0.209.0): the game is paused while it is open.
  Up / Down a line, L2 / R2 a page, Left / Right change a value, A choose
  or open a page, B back a page (from the top page: close). Every line's
  meaning is under the list, in as many lines as it needs (the list
  scrolls in the rest).
  - **Top**, in a game: Back to Game, Display, Audio, Game Play, Journal,
    Cheats, Quick Actions, Controls, System, Rescan Game Files, Clean Up Game
    Files, Check for Updates, Exit Game, Quit. At the game list: Display, Audio, Game Play,
    Cheats (Max Hit Points and Disable Race Level Limits only), Controls, System, Rescan Game Files,
    Clean Up Game Files, Check for Updates, Quit. (Rescan and Check for Updates on the top page
    are the port's own - Tom, 2026-11-17: "not buried".)
  - **Display**: Picture (the fit, section 5).
  - **Audio**: Game Sound, AdLib Music, Volume.
  - **Game Play**: Original Games (a switch: on - every Enhancement, AD&D
    1E Rule and Game Fix off, the game's own font, the cheat switches off, the
    player's set kept in settings.cfg's `saved_*` and given back when it
    goes off; any switch can be turned on by itself meanwhile),
    Enhancements, AD&D 1E Rules, Game Fixes.
  - **Enhancements** (`comforts_off` bits): Game Font, Show Rolls,
    Savegame Info, Auto Join Items, Keep Memorizing, Camp Fix, Coin
    Exchange, Flasks of Oil (engine 0.210.0: `coin_exchange`, `oil_burns`), Character Info
    (0.211.0: `add_info`), More
    Characters, Icon Portrait, Icon Gallery. (Not here: the CYD's
    Companion Views and Large Icon - the boards'.)
  - **AD&D 1E Rules** (`rules_off`, rules::Adnd bits) and **Game Fixes**
    (`fixes_off`, bit = rules::Fix; pages Every Game and one a game - in a
    game Every Game and that game's): one switch each, names / games /
    help from the engine's `rules::adnd_info` / `fix_info`.
  - **Cheats**: Healer, Merchant, Sanctuary, Trainer (`play::cheat_open` -
    the game's own screens; only at the exploring menu, else a message says
    so), Max Hit Points, Disable Race Level Limits (v0.12.9: no race stops a class's levels). Always there (no switch).
  - **Quick Actions**: one screen (`play::quick_*`; Tom, 2026-11-17 - no
    page in between): the top line Quick Actions On / Off, under it the
    party down the left (NPCs marked) and the chosen member's eight
    switches down the right, dimmed when off. Up / Down: the top line and
    the switches; L1 / R1 or Left / Right a member; A flips (a switch is
    kept at once in `_CYD/QUICK.TXT` by the engine); B back.
  - **Controls** (the port's own): Button Guide, D-Pad Walks, Confirm
    Button.
  - **Rescan Game Files** (top page; in a game: leaves it first).
  - **Clean Up Game Files** (v0.13.0, Tom 2026-11-24; top page, beside
    Rescan; in a game: leaves it first): the engine's `game/tidy.*` on each
    game found and the Launcher folder - what the engine never reads goes
    (DOSBox, installer / Galaxy and Steam launcher files, the Companion's
    program and pictures, DOS drivers / tools, the Dark Queen's non-AdLib
    music, the Launcher but its box art); game files, saves, SAVE* folders,
    icons, the Companion's journal files, every PDF and unknown files stay.
    Its lines on the scan's page ("Cleaning Up Game Files"), every file in
    games/_CYD/CLEANUP.TXT (Send Logs takes it). A folder holding hidden
    ".name" files (the listing skips them) is left. Checked: smoke test
    (made folders), and the Steam set from the Project: 582 files / 83 MB out,
    then Rescan's lines the same as the first scan's.
  - **System**: Device
    Info (the port's and the engine's version, device.txt's lines, each
    game found and its release - Steam: files under `<folder>/GAME/`; GOG:
    a `goggame-*` file; else unknown), Send Logs, Reply Address.
  - Always on, no switch (as the CYD): Readable Messages, a button stops
    the demo, the Cheats page. settings.cfg: `menus=2`; an older file's
    Game Fixes off = every fix off.
- **On-screen keyboard** over the top of the game screen (the game's own
  text area stays visible) while `play::input()` asks.
- **Messages** stay until a button.
- **The Adventurer's Journal** (`src/jour.*`; Tom, 2026-11-14: "a very
  big deal"):
  - *Made* when the game is found (Finding the games, above), with the
    engine's own code and by the CYD scan's three routes: a scanned PDF
    the engine's tables know (GOG: the PDF in the game's folder) ->
    `journal::make`, the entries cut from the pages' pictures; the Steam /
    SNEG release (`<folder>/GBC/Games/<nn. Title>/Game.dat`, the PDF in
    `<folder>/Documentation`) -> `make_gbc` + `make_book`; a typeset PDF
    alone -> its sections' text. Result: `_CYD/<folder>/JOURNAL.DAT`
    (+ `BOOK.TXT`), made again only when its source changed.
  - *Shown* when the game mentions an entry: its screen stays up; the next
    A opens the entry (several: one after the other); closing returns to
    the game, still waiting. Text entries in the game's look - its text
    screen's border and font on the borrowed canvas (`play::text_screen` /
    `text_line`), 38 x 22 letters, one size. Entries with pictures
    on a page of their own on the backdrop layer (scan rows averaged
    down).
  - **X** (Tom, 2026-11-14): a text entry has no zoom - X is the next
    page, round to the top after the last. A picture entry's X steps its
    zoom: **Width** (the start: the picture as wide as the screen),
    **Double** (twice that; Left / Right pan), **Whole** (all of it on
    the screen - only when it is taller than the screen), **Turned** (a
    quarter turn, its length across the screen - only for a map more than
    twice the screen's height: the engine's table marks the scanned
    editions' maps, a Steam entry's picture is one). The key's line names
    the zoom shown.
  - Keys: Up / Down, L2 / R2 or L1 / R1 a page, A a page then close at
    the end, X as above, B close. Start - Journal: the entries met so
    far (`play::journal_seen`).
  - An entry that can't be shown is named in a message instead, with why.

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
- **The log's head** (Tom, 2026-11-14): every `log.txt` starts with this
  port's version, the device (PortMaster's name for it and the board's
  own model line - its make and model), the firmware and its version,
  PortMaster's version (its `version` file), the screen. The launch
  script writes them, and the same as `device.txt` (name=value) for Send
  Logs.
- **Send Logs** (Start menu; Tom, 2026-11-14 - replaces the WebDAV
  server of v0.3 - v0.7): `tools/sendlogs.sh` posts the logs with curl,
  as plain form fields (`file:<name>` = its text - no zip or base64 tool
  needed on the firmware; each file's end, 600 KB of log.txt at most),
  with `app`, `version`, `device`, `firmware`, `portmaster`, `screen`,
  `reply`, to the **log relay**: a Google Apps Script web app in the
  logs mailbox's account (`tools/logrelay/Code.gs`; its address is in
  sendlogs.sh, `GOLDBOX_RELAY_URL` overrides it for tests). The relay
  zips the files and emails them to the mailbox - subject: version,
  device, firmware; body: the details and log.txt's last 40 lines -
  with the player's address as Reply-To, and sends that address one
  fixed thank-you note asking what they were doing. Its limits: 3 MB an
  upload, 60 reports and 20 notes a day, a note an address a day. It
  answers "OK n files" / "ERROR why" (after a redirect: curl -L).
  - **Reply address**: asked for the first time Send Logs is used (the
    port's keyboard, with @ . _ -; X = No Thanks), kept in settings
    (`reply_email`, `reply_asked`), changed or cleared at Settings -
    Reply Address. Never required.
  - Sent: log.txt, log.prev.txt, device.txt, update.log, settings.cfg,
    the engine's `_CYD/*.TXT` logs - never BOOK.TXT (a journal's text),
    game files or saves.
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
   button-driven screen here): done - the journal's entries, the
   Companion (map, party, battle; beside the game on wide panels), the
   sound sets, music, Cheats, Quick Actions, the comfort switches (v0.6);
   still to do - the journal as a whole book.
5. **Game art in the list** - done (Steam box art v0.4, the GOG logo
   v0.6).
6. **PortMaster submission** (Tom's call): screenshot, testing thread,
   their checks.
