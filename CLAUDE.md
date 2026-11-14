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
- `src/main.cpp` - the screens (game list, play, menu, keyboard, messages)
  and the buttons. The game is driven only through `play.h`.
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
- `--script` steps: button names (`up down left right a b x y l1 r1 l2 r2
  start select`), `hold:<dir>` / `let:<dir>`, `wait:<ms>`, `tap:x:y`,
  `type:TEXT`, `shot:<file.bmp>`, `quit`. `--game <n>` starts a game at
  once. With Tom's game files in scratch this plays real games headless;
  look at the `shot:` pictures before saying something works.
- Never report "tested" without saying what was run against what. Without
  game files only the port's own screens are tested - say so.
- The device itself (controller mapping, sound, speed, each firmware) can
  only be checked by Tom on his handhelds: list what to try in TODO.

## Versions and releases
- Semantic versioning from `VERSION` (started 0.1.0, 2026-11-14): bump it
  in every push to main that changes the port (fix = Z, feature = Y).
- Every push to main: CI checks, builds aarch64 + x86_64 and the package
  (artifact `goldbox-portmaster`). Check CI's result after every push.
- `git fetch` and merge origin/main before every push; never force-push;
  never push over Tom's edits.
- **Don't publish a release unless Tom asks.** Releases: Actions -> Build ->
  Run workflow with `release_tag` = `v` + VERSION.
- Submitting to PortMaster's own catalogue (a PR to PortsMaster/PortMaster-New)
  is Tom's call and needs things only he can supply (TODO).
