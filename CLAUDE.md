# CLAUDE.md - working rules for this repo

Gold Box Game Engine for PortMaster: the CYD Gold Box Game Engine
(`tomtombombadil/CYD-GoldBoxGameEngine`, the `cyd/` submodule) with an SDL2
front end, packaged as a PortMaster port for Linux retro gaming handhelds.
The player adds their own original game files ("bring your own game").
Design: `docs/SPEC.md`. Open items: `docs/TODO.md`.

Repo: `tomtombombadil/PM-GoldBoxGameEngine` (Tom, 2026-11-14: Claude manages
and maintains it; it is the primary output of the Claude Project
"PM-GoldBoxGameEngine").

## The developer
- Tom works on **Windows**. Never give Linux / bash commands in instructions
  for him; PowerShell or a GUI. (The handhelds and CI are Linux - their
  scripts are bash, but Tom never has to run them: CI builds the package.)
- If a Python tool is ever needed on his side: always a venv, and
  `python -m pip install`, never bare `pip install`.
- He does not want to repeat instructions. Anything decided goes in this
  file, `docs/SPEC.md` or `THIRD_PARTY_NOTICES.md` the moment it is decided.
- **The task list**: `docs/TODO.md` - Tom's and Claude's open items
  (questions for Tom, checks Claude wants, follow-ups). Add things as they
  come up, tick them off with the version that did it, never ask Tom the
  same thing twice. Sync SPEC and TODO to the Project (`claude/SPEC.md`,
  `claude/TODO.md`) with every push.
- Game names to Tom (as in the CYD repo): PoolRad, Curse, Secret, Darkness,
  Gateway, Treasures, Champions, Knights, Queen - never "Pool" alone.

## Decisions (Tom, 2026-11-14)
- The port's name is "Gold Box Game Engine", its folder `goldbox` - settled.
- B = Back / Exit (SPEC 4). L2 / R2 = list up / down, no side-step
  buttons; Y = the Companion slide-out; no Look button (SPEC 4).
- **The Companion slides in from the RIGHT** (Tom, 2026-11-14: he had
  said left and meant right - the 3D view / fight field stay in sight).
  On wide panels (16:9 and the like) it stands beside the game instead
  (SPEC 4) - Tom's idea, as the CYD's bigger boards.
- **EVERYTHING made from a game's files is made by the scan, when the
  game is FOUND - the CYD's logic, follow it** (Tom, 2026-11-14, said
  twice: "I don't want anything processed (such as journals, logos, etc.)
  at game start up or upon first use. That ALL gets pre-processed during
  the initial game discovery ... Do it once, and then after that
  everything is smooth and fast and professional. No pausing, no loading
  or processing screens"). The scan (`discover()` in main.cpp, SPEC 6):
  the program's unpacked copy, the game list's picture (saved file; the
  list only reads it), the journal, the music - at the first run that
  sees the folder, and anew on Rescan Game Files (Start - System). **No decoding,
  converting or "Loading" screen at a game's start, on a list page, or
  the first time a feature is used.** Anything new the port or the
  engine makes from game files gets a scan step in the same version -
  before writing such code, read how the CYD's card scan does it
  (`cyd/src/app/library.cpp`, CYD CLAUDE.md "Card scan") and do the same.
- **The port's own pages fill the panel** (Tom, 2026-11-14, on his
  PowKiddy RGB10 MAX3, 16:9: the game list and the scan pillarboxed to
  4:3 "looks very unprofessional"): the game list, the scan and any new
  page of the port's own are laid out for the panel's real shape
  (`video::set_wide`, `margin()`, `back_w()`; SPEC 5) - check each at
  1280 x 720 as well as 640 x 480 (`--window`) before a release. Only
  the game itself is 4:3. No "2 of 9" counter on the game list ("looks
  bad"). The scan's list uses the smaller letters (`video::small_text`).
- **Journal viewer's X** (Tom, 2026-11-14): text entries - no zoom, X =
  next page, round to the top; picture entries - Width (start), Double,
  Whole, and Turned for very long maps (SPEC 6).
- **Rescan Game Files**: Tom, 2026-11-14 - on the game list's first Start
  menu, "Don't bury it at the bottom of the Settings menu". Since v0.12.0 it
  is in Start - System, the first line there, as the CYD's tree has it (Tom
  asked to mimic that tree; asked in TODO whether he wants it back on the
  first menu). The game list's version reads "v0.6.2" (with the v).
- **Nothing made for the screen's shape before the shape has settled**
  (Tom's RGB10 MAX3, v0.6.2: pictures remade at every start and not
  shown): a panel's real size can arrive after the first frames - check
  `video::shape_serial` and wait (`pictures_follow`), never trust the
  size at start-up. The smoke test's `resize:` runs cover it.
- **Journals are checked for every game in both releases** (Tom,
  2026-11-14: "Check the other games to make sure they're detecting the
  journals properly"): after any change to the journal making - here or
  in the engine - scan all eighteen folders (GOG with its PDF in the game
  folder, Steam with it in `Documentation`) and compare `build/jlist
  <games>` (target `jlist`; counts by kind, gaps, the shortest section)
  with the last run; a section far shorter than its neighbours is a miss.
  A miss in the engine's reading is an engine bug: fixed in the CYD repo.
- **No Companion on a game's title screens** (Tom, 2026-11-14, v0.9.5:
  blank beside the intro on 16:9, "look bad"): the game centred and
  nothing beside it until the party menu (Begin Adventuring); then the
  Companion and the game at the left (`panel::hold`, SPEC 4; the engine's
  `play::titles()`, CYD 0.200.9 - Tom OK'd the call, 2026-11-15). **The
  demo keeps the Companion** (Tom, 2026-11-15: "don't turn off the
  companion for the demo").
- **Square panels** (Tom, 2026-11-15: 1:1, 720 x 720 - RG CubeXX, RGB30:
  "Could the game area be shifted up and the companion info fit at the
  bottom?"): the game at the top, the Companion in the room under it - map
  (a fight: the battlefield) left, party right (`video::below_room` /
  `set_below`, `panel.cpp` `draw_below`; SPEC 4). Tom has no such device:
  checked at `--window 720x720` only; check it there before a release.
- **Tom's devices**: RG40XX H / ROCKNIX (4:3, primary), PowKiddy RGB10
  MAX3 (16:9), an **R36H** on ROCKNIX (4:3, 640 x 480; it reports itself
  as "R33S (Game Console R33S)" - "those devices are sketchy"; logs
  headed R33S are this one. v0.8.0's Send Logs worked there). And an
  **RG40XX V on Knulli** (Tom, 2026-11-15, v0.10.x): runs; **A and B come
  out reversed there** - so on Knulli Confirm starts on B by itself (Tom,
  2026-11-15: "just go ahead and reverse the buttons in it by default";
  v0.10.5: the launch script finds Knulli and passes `--ab-reversed`,
  applied once - `ab_set` - then Settings - Confirm Button is the
  player's). **The hints name the button as MARKED on the device** (Tom,
  2026-11-17, TrimUI Brick Pro on Knulli: "it has them labeled
  incorrectly ... B is flip and A is back. But A is actually flip and B is
  back"): on a reversed firmware the program's B is the button marked A -
  `confirm_name` / `back_name` in main.cpp turn with `ab_reversed`; every
  hint goes through those two, never a letter typed into a string. Another firmware with the same fault gets the same line in
  the launch script - no first-start question. And an **R36S on dArkOS** (Tom, 2026-11-15):
  "It ran fine". And a **TrimUI Brick Pro on Knulli** (Tom, 2026-11-17, v0.10.8):
  runs, the A / B turn found it by itself.
- **The CYD repo keeps moving while Tom works on both** (2026-11-14: "I
  probably shouldn't have made them separate projects, but here we
  are"): when he says the CYD changed, fetch it, move the pin once its
  CI is green, adapt the port to any changed call, and wrap anything
  player-facing into this repo in the same version (new music -> the
  scan; a Companion change -> `src/panel.*` shows it).
- **B = the menu's Exit or a question's No, and out of "Pick Character"**
  (Tom, 2026-11-14: "use B to exit if there is an exit menu item or 'no'
  item ... only enhancing it with the B-button-exit feature, not altering
  the way the games work"). **The games' own menus follow the originals**:
  when a party-menu / prompt question comes up, run the original in
  DOSBox beside the engine (CYD CLAUDE.md "DOSBox here";
  `usescancodes=false`, arrows + Return, letters work in Queen) and make
  the engine match - in the CYD repo - before adding anything of ours.
- **No saved game is no reason not to test** (Tom, 2026-11-14: "create a
  party quickly and save them. There's your savegame. Don't let that keep
  you from testing") - in the engine and in the original under DOSBox
  alike (Create New Character with Returns, Add Character to Party).
  GOG's Curse / PoolRad take **any answer** at the code wheel (Tom);
  PoolRad in DOSBox needs its folder's parent mounted as C: and
  `cd POOLRAD`; Curse's and PoolRad's menus take letters, not Return.
- **Coming from the CYD** (Tom, 2026-11-14): Companion map / party-info
  changes, mostly for the small boards but with effects here, and maybe a
  rearranged party-info layout ("It's hard to read as is"). Don't
  redesign `src/panel.*`'s Party view meanwhile; when it lands, move the
  pin and make the port's panel show it in the same version.
- **"Exit to DOS" in a training hall leaves at once, unsaved - leave it**
  (Tom, 2026-11-15: "It's a port of an old DOS game, not a babysitter.
  It's from the era when actions had consequences"). No confirmations
  added where the originals have none.
- **The Trainer cheat is left with Begin Adventuring** (Tom, 2026-11-14:
  "B doesn't leave, and that's OK") - don't make B leave it.
- **A scan-tag bump (`kScanTag`) makes every journal anew** at the next
  start (v0.10.1) - bump it whenever the journal making changes, here or
  in the engine (its tables too: their ids don't change).
- **A highlighted word must act on A, and the list keys must not drop out
  of a prompt** (Tom, 2026-11-14, the Dark Queen's Pick Character): where
  they don't, it is an engine bug in `play::nav` / `nav_point` - fix it
  in the CYD repo.
- **Game list pictures** (Tom, 2026-11-14): Steam box art when the
  Launcher folder is there, else the game's own logo from its folder's
  `goggame-*.ico` / `.dll` as the CYD. If a picture of our own is ever
  needed: mock it up for Tom first.
- **Every engine extra has its switch here** (Tom, 2026-11-14: "The
  cheats, quick actions, game fixes, and all that extra functionality
  needs to be added"): a new `play::Options` field or Game-menu tab in
  the CYD gets a Settings line / menu page in the same version that
  moves the pin, with its words at the page's foot.
- **The menus mimic the CYD's** (Tom, 2026-11-17: "review them and make similar
  changes to the PortMaster version menus ... the toggles were reworked, many
  separated into individual toggles"; v0.12.0, engine 0.209.0 - its CLAUDE.md
  "The menus"): Start's top page and its pages as the CYD's tree (`Page`,
  `menu_build` in main.cpp; SPEC 6), plus the port's own Controls page, Quit,
  and System's Check for Updates / Reply Address. Each AD&D 1E Rule and each
  Game Fix is its own switch; their names, games and help come from the
  engine (`rules::adnd_info` / `fix_info`) - a new one needs nothing here.
  What the port keeps: `rules_off`, `fixes_off`, `original` + the saved set
  (settings.cfg, `menus=2`). When the CYD's tree changes, change this one in
  the version that moves the pin. Rescan Game Files went to System with it
  (it replaces "on the game list's first menu" unless Tom says otherwise).
- **Play-throughs check the journals and every text** (Tom, 2026-11-17: "This
  is a functionality test of the whole game, not just the parts you want to
  test"): keep every text of every leg from the first step of a place
  (transcript); per map a ledger of the journal entries its scripts mention
  (the ECL blocks' strings), each ticked only when the engine fired it there
  ("JOURNAL J n"), and each present in the journal the scan made, GOG and
  Steam. A place isn't "done" with an entry unticked or unexplained.
- **Save Slot Info** (Tom, 2026-11-17: "The CYD engine has created changes
  to the save/load menu screens ... review them and update this repo";
  v0.11.0, engine 0.207.x): the engine draws it; the port gives it three
  things - the Settings line (`kOffSaveInfo`), `plat::file_time` (the
  save's date from the file, the device's local time; 18 letters is the
  4:3 column - `platform_sdl.cpp` closes "9:41 PM" up to "9:41PM" rather
  than let it wrap badly), and `play::set_side_panel(panel::docked_on())`
  each frame so a Companion beside / under the game shows the slot's party
  and the words get the whole game screen. Since v0.11.1 (engine 0.208.0;
  Tom: "we may as well show fill in the map display too") the Companion's map
  is the highlighted save's meanwhile - the engine's `play::map()` / `pos_x` /
  `pos_y` / `dir` give the save's; `panel.cpp`'s signature takes the map's
  walls in so another slot redraws. Check it at 640 x 480, 1280 x
  720 and 720 x 720 after an engine change there.
- **What the CYD version has, this port must have** (Tom, 2026-11-14,
  after v0.3: the journals "a very big deal", the launcher's box art on
  the game chooser, the Darkness / Gateway / Treasures sound sets were all
  missing): before calling a version ready, go down the CYD's player-facing
  features (its CLAUDE.md / SPEC 11) and port each or list it in TODO as
  missing - never leave one out silently.
- The port's folder: one program file a device type (`goldbox.aarch64`) -
  the engine is compiled into it, there are no loose engine files; the
  player's games in `games/`.
- It is to be submitted to PortMaster's catalogue **eventually**; that
  needs a gameplay screenshot - "don't worry about that for now". Don't
  raise it again until the port is ready for submission.
- **Primary test device: Tom's Anbernic RG40XX H running ROCKNIX** (640 x
  480, every button and both sticks, no touch screen). ROCKNIX's card is
  ext4 - Windows can't read it. **Don't give Tom multi-step file-copy
  procedures for getting builds onto it** (2026-11-14: "I don't understand
  your install instructions and it's kinda a pain"): builds reach the
  device by the installer once, then Check for Updates.
- **Tom's game files are in this Project's files** (2026-11-14): the GOG
  release of each of the nine games ("<Game Name>.zip") and the Steam
  release ("<SHORT>_SNEG_VERSION.zip" - SNEG is the Steam publisher; **the
  two releases' files differ**, so check both), each game's journal PDFs
  in zips ("<SHORT> - ... Journal.zip" GOG, "<SHORT> SNEG - ....zip"
  Steam; zipped so the PDF's bytes come through - they are processed on
  the device), and "Launcher Steam.zip" (the Steam launcher: its box art
  is wanted on the game selection screen, made on the device from the
  player's copy as the CYD scan does). Fetch with `project_read`, unpack
  under `/tmp/claude-0/pmgames/` (never under a repo).

## Bring your own game - hard rules (the CYD project's, unchanged)
- The repo and the package contain nothing from the games: no DAX / TLB /
  GLB files, no extracted pictures, text, journal entries or code. Tests
  build their own synthetic files. **Screenshots of real game graphics
  stay out of the repo** (scratch only).
- Game files are read from the port's `games/` folder on the device. What
  the engine makes from them on the device (`games/_CYD/`) is fine.
- Tom's game files, when he provides them for testing, stay in the
  session's scratch space only - never under the repo (.gitignore and CI
  block the file types as a backstop; `tools/package.py` refuses them).
- Licensing: MIT. Only copy code from MIT / BSD / Apache / zlib /
  public-domain sources, keep their headers, update
  `THIRD_PARTY_NOTICES.md`. GPL projects and unlicensed ones (coab, the
  Gold Box Companion) are reference only. Other ports' launch scripts:
  follow PortMaster's documented steps, don't copy a port's script.

## The two repos
- **`cyd/` is the engine, a git submodule pinned to a CYD commit. Never
  edit files under `cyd/` from this repo.** This port builds
  `cyd/src/engine/*.cpp` and `cyd/src/game/play.cpp` as they are and uses
  only what `cyd/src/platform/*.h` and `cyd/src/game/play.h` declare.
- The CYD repo is still in development (Tom, 2026-11-14: "almost done";
  keep an eye on it). At the start of a session look at what changed there
  (`git -C cyd fetch`, the log since the pinned commit) and move the pin
  forward when its CI is green and this port still builds and passes its
  checks. Say which CYD version (its `VERSION`) the port is built on in
  the push's summary.
- Changes to the CYD repo itself (Tom, 2026-11-14): **only bug fixes or
  mistakes**. Anything this port would like from the engine that isn't a
  bug (a new `play::` call, moving code onto the platform layer) is a
  question for Tom in TODO, not a change. That repo has its own CLAUDE.md -
  follow it when working there (version bump, its TODO / SPEC, no force
  pushes, fetch and merge before pushing).
- Engine behaviour (rules, the games' screens, what differs from the
  originals) is decided in the CYD repo. This repo decides only the
  handheld front end.

## Controls - Tom's rule (2026-11-14)
- **Most handhelds have no touch screen and many have no sticks. The whole
  game and every function must be usable with only: D-Pad, A, B, X, Y, L1,
  L2, R1, R2, Start, Select.** Sticks, L3 / R3, touch and mouse may add
  conveniences but are never the only way to do something.
- A new screen or feature isn't done until it works from those buttons;
  say how in SPEC section 4.
- Select + Start is PortMaster's quit (gptokeyb) - never bind something to
  that pair, and Select's own action happens on its release.

## Architecture
- `src/platform_sdl.cpp` - `plat::` (clock, log, files) and `memlog::`.
  The engine's `/GOLDBOX/...` paths map onto the `games/` folder, **names
  matched ignoring case** (DOS files on ext4). No file-slot limit, memory
  figures 0 ("can't tell") - the boards' memory rules don't apply here.
  `plat::vga_screen()` is true: the VGA games run in 256 colours.
- `src/audio_sdl.cpp` - `audio_*` through an SDL audio device (8-bit
  unsigned, `kAudioHz`); the filler runs on SDL's audio thread, changed
  only under the device lock.
- `src/video.*` - the 320 x 200 canvas plus the port's overlay (same size,
  40 x 25 text cells in the engine's Clear font), scaled to the panel:
  Original 4:3 (default), Square Pixels, Stretch. Layout always from the
  panel's size - never assume a resolution.
- `src/library.*` - finds the games (the CYD scan's rules on `plat::`).
- `src/art.*` (the game list's pictures: made by the scan into
  `_CYD/<folder>/ART.BIN`, only read by the list), `src/panel.*` (the Companion),
  `src/jour.*` (the journal: made and shown), `src/task.*` (the helper
  scripts' thread).
- `src/main.cpp` - the screens (game list, the scan, play, the menu's
  pages, Quick Actions, keyboard, messages) and the buttons. The game is driven only through `play.h`.
- The engine's folder inside `games/` is `_CYD` (its game code writes
  there by that name) - don't rename it.
- `src/compat.*` - strlcpy / strlcat of the port's own, force-included
  into the engine's files (glibc has them only since 2.38; the engine's
  game code uses them). **Local builds on a new Linux hide this kind of
  thing - only the CI's Ubuntu 20.04 container build says whether a
  binary will start on old firmwares** ("newest glibc needed" in its log).
- Built with CMake; C++17 as GCC 9 knows it (the Ubuntu 20.04 container).
  SDL2 calls no newer than 2.0.10 (that container's, and old firmwares').
  libstdc++ linked statically, SDL2 dynamically (the device's own).
- PortMaster package: `port/` + binaries -> `tools/package.py` ->
  `dist/goldbox.zip` (port name `goldbox`, script "Gold Box Game
  Engine.sh"). The launch script must keep Unix line ends
  (`.gitattributes`; CI checks).

## Checking work
- `tools/smoke_test.py <binary>` runs the port with SDL's dummy drivers and
  a `--script` of button presses (no game files). CI runs it on an
  ASan / UBSan build. Extend it with every new screen.
- **`tools/play_test.py <binary> <games dir> <game n> <out dir> [WxH]`**
  (2026-11-14, before the Reddit announcement - Tom: "Test it. Test each
  game"): plays a game with button presses alone through `--drive` (steps
  on stdin; after each line the port prints its state and the game screen
  read back as text, `drive_text` in main.cpp) - party set-up, walking,
  overland Move, Area, the Companion, Encamp / Save, Journal, Quick
  Actions, the Cheats, a fight. Needs real game files, so it is run by
  hand on a COPY of the games folder before every release Tom is to
  announce, every game (since v0.10.1 the reader takes text over pictures
  and off the 8-pixel grid - lines after the 25 - so Treasures and the
  Dark Queen are played too; a boxed word = WHITE paper; `canvas=` in the
  state is the picture's sum, `title=` / `dock=` the title screens / the
  docked Companion). **The engine's `play::walking()` decides what the D-pad
  does** - a place where the boards' movement keys work but walking() is
  false is a place this port can't move (v0.9.1: the Krynn / Darkness
  overland). The ARM program can be run here too: qemu-user plus an arm64
  SDL2 sysroot unpacked from Ubuntu's ports debs (`qemu-aarch64 -L <root>`).
- `tools/save_test.py <binary> <games copy> <n> <out>` (v0.10.2): save,
  quit the port, start anew, load - the same place? The binary may be the
  ARM program's qemu wrapper. `tools/monkey_test.py <sanitizer build>
  <games copy> <n> <presses>`: random presses and resizes, looking for a
  crash or a sanitizer report (build with `-DCMAKE_BUILD_TYPE=RelWithDebInfo
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"`; a Debug build is too
  slow for the screen reader; its Send Logs / update addresses point
  nowhere). Use copies already scanned by this version (a scan under the
  sanitizer or qemu takes very long). The update across REAL releases was
  checked by hand once (v0.9.4 -> v0.10.1, scratch script).
- `tools/hand_test.py <binary> <games copy> <n> <out> [fight|menus|both]`
  (v0.10.3): `fight` walks the map through square by square until a fight
  starts, then on our fighters' turns tries Move (D-pad straight, two
  directions together, L1 / R1 / L2 / R2), Aim, Aim - Manual, Cast, Done's
  menu, then Quick and the treasure menu; `menus` takes every word of
  Cast / View / Encamp with A, notes its screen, B back, four deep. Its
  "!!" lines need a look - most so far were the test's own (a message a
  member that wants a press each, the icon gallery left by B then Yes,
  the Dark Queen's off-grid prompts). Script steps: a direction is HELD
  while it acts (a fight's pad reads what is held), `up+right` = two
  together.
- `tools/explore_test.py <binary> <games copy> <n> <out> [steps] [minutes]`
  (2026-11-15): a character made, the icon screen tried, then the maps
  walked through - fights on Quick (Encamp - Fix after each), shops /
  temples / halls walked through word by word, journal mentions counted,
  stuck places noted with a shot. The stock parties fall within a few
  fights: for a deep walk make the party strong first. **A training hall
  is the game's whole party menu - leave it by BEGIN Adventuring; its
  "Exit to DOS" leaves the game.** Never start two lane scripts on the
  same folders (a half-copied games folder gives nonsense).
- `tools/newgame_test.py <binary> <games copy> <n> <out> [steps] [minutes]`
  (2026-11-16, v0.10.6): a NEW game as a player starts one - six
  characters made and added, Modify, Begin, armour / shield / weapon
  bought at the Merchant cheat's shop and readied (View - Items), then
  explore_test's walk with `strong` on (the Healer cheat after fights,
  the Trainer's free training every third; Max Hit Points on). It found
  Darkness's new game hanging (engine 0.203.7). **Saves only show what a
  save holds - run this after any engine pin move that touches a game's
  start.** A fault that shows in the CYD's playsim too (`GAME=n
  NEWGAME=1`, tools/devkit/build.sh sim) is the engine's.
  **A test that says OK must look at the result** (2026-11-16: creation
  "OK" for a day while the test had left the race list by Exit - it only
  looked for the party menu; now the member count / the Add list). At
  most four runs at once here (2 cores); the walks' fights and temple
  visits are slow - 30 minutes a game goes a few hundred steps.
- **Play tests go where things are - no random wandering** (Tom,
  2026-11-16: "you have access to the actual game files themselves and
  the inner workings of the game. You already KNOW where the encounters /
  fights / inns / shops / trainers / etc are. Use that information. Don't
  wander randomly. That's a waste of time and effort. Be smart. Think like
  a player would"; game guides too). Before a walk: get the map's squares
  and what each one's script does from the engine (the CYD's sweep /
  playsim on the game files, `ecl::find_shops`, the map's walls for the
  way there), then send the party straight to the shop, the temple, the
  hall, the first fight, the way out - as a player with the cluebook.
  How (2026-11-16): the book = the CYD sweep's `events` run with EVMAP=1
  (`cyd/tools/devkit/build.sh sw "" tools/playsim/sweep.cpp`, then
  `GAME=n EVMAP=1 sw <folder> events`; the folder's parent stands for
  /GOLDBOX with a `_CYD` beside it; PoolRad's folder is its inner
  POOLRAD) - each block's walls and, by the square and facing a step was
  taken FROM, what its script shows. `tools/guide.py` reads it and plans
  the way; `BOOK=<file> tools/newgame_test.py ...` walks it. Books are
  made from game files: scratch only.
- **Play-throughs, start to finish, every game** (Tom, 2026-11-16, the
  same brief he gave the CYD session; its CLAUDE.md "Play-throughs" /
  "Function checks" hold here too). This session's order: **Gateway
  first** (the CYD session has PoolRad - done - and Curse), then the
  games it hasn't taken. **Gateway: done, 2026-11-17. Treasures next** (Tom,
  2026-11-17: "Go ahead with testing Treasures") - the finished Gateway
  party brought over (Add Character - Gateway), scratch `/tmp/claude-0/tplay`,
  notes the Project's `claude/playtest_treasures.md`. A new party rolled up, then adventuring to the
  end by a web walkthrough; the check is that every triggered event
  fires at the right time and place, every chain of events to the ending
  works, and a player can finish and enjoy the game. **Don't fail over
  and over - cheat**: no grinding (one or two random encounters to see
  they work, then grant the experience the guides give for the next
  area - never go in too low); no trips back to town to rest / sell /
  train - the Cheats' Healer, Sanctuary, Merchant, Trainer where the
  party stands; save often, reload after a level drain.
  Tom's party: 2 fighters, a fighter/thief, a cleric, a cleric/magic-
  user, a magic-user (two clerics matter). His fight advice: a tight
  line so only one or two foes reach each member, the best armour on the
  corners, casters behind, a doorway or hall when there is one; everyone
  with a missile weapon (slings for casters, composite bows for the
  strong); gang up on one foe at a time; missiles / Silence 15' / a wand
  of Magic Missiles on enemy casters (a hit stops a spell); Sleep to
  hold a group, then shoot their archers and casters first - later Hold
  Person, Stinking Cloud; undead: Protection from Evil against ghouls'
  touch, the level-drainers first, Turn Undead kept for them, magic or
  silver weapons.
  **Loot is a key part of the game - take it, every fight** (Tom,
  2026-11-16: "if you're not taking advantage of it, you're going to
  fail in a lot of fights, the big important fights"): after each fight
  Take the items; at the Cheats' Merchant try to Sell each - a high
  price means magic - and have the Merchant identify (ID) it; give it to
  the member who can make the best use of it and ready it, sell what is
  no use. Never leave a treasure by Exit (the kit's AUTOTAKE; a member
  with room and strength to carry takes - not one already loaded).
  **This is play-testing, not a marathon gaming session** (Tom,
  2026-11-16: "We're not looking for a challenge here, we're checking to
  see if the game works the way it is supposed to work"): the party
  should survive its fights and carry on without replaying things. So:
  the Merchant's ID instead of memorizing Detect Magic / Identify (the
  slots go to fight spells); spend the money won on better equipment,
  freely; Modify Character to raise the party's figures, freely.
  How: the CYD repo's play-through kit on the playsim
  (`cyd/tools/devkit/playthrough/README.txt`, a folder a game - Gateway's
  is `gateway/`), the engine at full speed; an engine fault found is
  fixed in the CYD repo, then the pin moves. What only the port can show
  (buttons, screens) stays with newgame_test's walks. Progress and
  findings: the Project's `claude/playtest_gateway.md` and TODO.
- **A direction let go before its step was taken still steps** (v0.10.3:
  a fight waits 70 ms for a diagonal's second direction; shorter taps
  were lost) - `button()` in main.cpp; keep it when touching the D-pad.
- `tools/play_shots.py <binary> <games dir> <out.png> steps...` plays a
  script and puts a screenshot per step on one sheet (scratch only - real
  game screens). Titles run on their own: give waits between presses
  (`a! wait:700!`), and keep sheets small - a shot per step that matters.
- `journal:K:N` (script step, in a game): opens that journal entry as if
  the game had mentioned it.
- `--script` steps: button names (`up down left right a b x y l1 r1 l2 r2
  start select`), `hold:<dir>` / `let:<dir>`, `wait:<ms>`, `tap:x:y`,
  `type:TEXT`, `menu:<line's start, _ for a space>` (the open Start
  menu's line - never count `down`s), `shot:<file.bmp>`, `quit`. `--game <n>` starts a game at
  once. With Tom's game files in scratch this plays real games headless;
  look at the `shot:` pictures before saying something works.
- Never report "tested" without saying what was run against what. Without
  game files only the port's own screens are tested - say so.
- The device itself (controller mapping, sound, speed, each firmware) can
  only be checked by Tom on his handhelds: list what to try in TODO.

## Versions and releases
- Semantic versioning from `VERSION` (started 0.1.0, 2026-11-14): bump it
  in every push to main that changes the port (fix = Z, feature = Y).
- Every push to main: CI checks (incl. the install / update / send-logs
  tests against pretend servers), builds aarch64 + x86_64 and the package.
  Check CI's result after every push.
- `git fetch` and merge origin/main before every push; never force-push;
  never push over Tom's edits.
- **Releases as Pocket Curator's** (Tom, 2026-11-14: "package up the file
  into a pre-release until we're ready for an actual release ... lookup a
  new version and download and install it right from the test device";
  his `tomtombombadil/PocketCurator` is the model - read it when in doubt):
  every version Tom is to test is published as a **pre-release** - Claude
  does it, without asking, once the push's CI is green: the Build workflow
  dispatched with `release_tag` = `v` + VERSION
  (`gh api -X POST repos/tomtombombadil/PM-GoldBoxGameEngine/actions/workflows/build.yml/dispatches
  -f ref=main -f 'inputs[release_tag]=vX.Y.Z'`), then check the release
  has its three assets. A **stable** release (`inputs[stable]=true`) only
  when Tom says the port is ready for one.
  - Assets: `goldbox_port-vX.Y.Z.zip` (the names the installer and updater
    look for - never rename), its `.sha256`, `GoldBox.Installer.sh`.
  - `port/GoldBox.Installer.sh`: copied once into `roms/ports`, installs
    the newest release (pre-releases included).
  - The port's **Check for Updates** (`port/goldbox/tools/update.sh` +
    `src/main.cpp`'s jobs): stages the newest release in `.update/`, READY
    last; the launch script applies it at the next launch (Restart Now
    does that at once). Network only through the firmware's `curl` from
    the helper scripts - no network code in the binary. conf/ and games/
    are never in a release.
  - **An update is installed by the OLD version's launch script and
    updater**: a mistake there strands testers on that version (Pocket
    Curator met this). Never publish a release whose smoke test's update
    path (`tools/smoke_test.py` update_test) fails, and keep that test
    covering install -> update -> restart.
  - `CHANGELOG.md`: a section a version, written for Tom - it is the
    release's notes.
- **Send Logs** (Tom, 2026-11-14, v0.8.0; replaces the WebDAV server of
  v0.3 - v0.7, which is gone - no server address anywhere): the Start
  menu's Send Logs uploads the logs with the firmware's curl
  (`tools/sendlogs.sh`: plain form fields, no zip tool needed) to the
  **log relay** - a Google Apps Script web app Tom deployed in the logs
  mailbox's account (cyd.classic.games.logs@gmail.com;
  `tools/logrelay/Code.gs` is its source - **a change there does nothing
  until Tom pastes it and deploys a new version**: give him the file and
  click-by-click steps, Manage deployments - Edit - New version, so the
  address stays the same). It zips and emails them to that mailbox, the
  player's address as Reply-To, and sends the player one fixed thank-you
  note. The relay's address is public by design (it can only do that);
  its limits are in Code.gs. No mail password, token or other secret
  ever goes in the port (Tom asked about SMTP, SSH keys, GitHub tokens:
  each needs a secret in a public package). Sent: log.txt, log.prev.txt,
  device.txt, update.log, settings.cfg, the engine's `_CYD/*.TXT` - text
  only, never game files, saves, BOOK.TXT or the scan's PMSCAN.TXT marks.
  The thank-you note lands in the player's spam folder (Tom's did,
  v0.8.0): the port's message says to look there.
  - The **reply address** is asked for the first time (optional, X = No
    Thanks), kept in conf/settings.cfg, changed at Settings - Reply
    Address.
  - **Every log's head** (the launch script): the port's version, the
    device's make and model, the firmware and its version, PortMaster's
    version - Tom wants all four in every log; `device.txt` has the same.
  - Tom reads the emails and passes the zips to Claude. When asking Tom
    for a log, ask for Send Logs - not for files fetched off the card.
- Submitting to PortMaster's own catalogue (a PR to PortsMaster/PortMaster-New)
  is Tom's call and needs things only he can supply (TODO).
