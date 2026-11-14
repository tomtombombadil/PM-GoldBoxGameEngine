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
  sees the folder, and anew on Look for Games Again. **No decoding,
  converting or "Loading" screen at a game's start, on a list page, or
  the first time a feature is used.** Anything new the port or the
  engine makes from game files gets a scan step in the same version -
  before writing such code, read how the CYD's card scan does it
  (`cyd/src/app/library.cpp`, CYD CLAUDE.md "Card scan") and do the same.
- **Game list pictures** (Tom, 2026-11-14): Steam box art when the
  Launcher folder is there, else the game's own logo from its folder's
  `goggame-*.ico` / `.dll` as the CYD. If a picture of our own is ever
  needed: mock it up for Tom first.
- **Every engine extra has its switch here** (Tom, 2026-11-14: "The
  cheats, quick actions, game fixes, and all that extra functionality
  needs to be added"): a new `play::Options` field or Game-menu tab in
  the CYD gets a Settings line / menu page in the same version that
  moves the pin, with its words at the page's foot.
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
- **Send Logs** (Tom, 2026-11-14): Start menu - Log Server (a WebDAV
  address, typed once, kept in conf/settings.cfg - never an address in the
  repo) and Send Logs (`tools/sendlogs.sh`: log.txt, log.prev.txt,
  update.log, settings.cfg, the engine's `_CYD/*.TXT` - text only, never
  game files) into `goldbox-logs/` on the server. Tom gets them on his PC
  that way and passes them to Claude. When asking Tom for a log, ask for
  Send Logs - not for files fetched off the card.
- Submitting to PortMaster's own catalogue (a PR to PortsMaster/PortMaster-New)
  is Tom's call and needs things only he can supply (TODO).
