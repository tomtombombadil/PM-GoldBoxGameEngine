# TODO - Tom's and Claude's open items

Tick off with the version that did it. Never ask Tom the same thing twice.

## Tom
- [ ] **Send Logs** couldn't find your WebDAV server (v0.3.1) - you're
      looking at your side. If it still fails: what address you typed,
      and what the server's own window shows when you try.
- [ ] After the v0.3.1 install the `goldbox` folder wasn't visible until
      the port had been started once. The release zip holds
      `goldbox/games/` and the installer makes it; say if v0.4's install
      (or a refresh of the Explorer window) still doesn't show it.
- [ ] Other handhelds / firmwares you can test on, when it matters (a
      PortMaster submission wants several).
- [ ] `cyd/src/app/library.cpp` (the game scan) is tied to Arduino's
      files, so the port has its own copy of its rules (`src/library.cpp`).
      Moving the CYD's onto the platform layer would let both share it -
      not a bug fix; your OK needed.

## Claude
- [ ] **Buttons through every screen kind** with the game files (SPEC
      roadmap 3). Checked so far, Curse (GOG), v0.2.0: titles, party
      menu, Load, Modify, Create Character to the name's keyboard (type,
      delete, Done), Add Character lists, Begin, story text and pictures,
      the exploring menu, Back out of each, Pointer Mode, the Start menu.
      **Not yet**: walking through doors, Area, camp / spells / items,
      shops, a fight (moving, aiming, diagonals), saving, the icon
      gallery and editor (may need D-pad support beyond Pointer Mode),
      and every other game; the Steam (SNEG) folders; the VGA games.
- [ ] Game list: the GOG icon when there is no Steam Launcher folder
      (the CYD shows it); the box art is decoded each time a page is first
      shown - cache it if that is slow on the device.
- [ ] Journal: process the player's journal PDFs on the device and show
      entries with buttons (roadmap 4; the GOG and Steam PDFs differ).
- [ ] From the CYD, still missing here: Cheats; Quick Actions; the comfort
      switches (Game Fixes, Show Rolls, Camp Fix, Max HP, Readable
      Messages ...); the icon gallery by buttons (unchecked); logs screen.
- [ ] Updater: a stable / pre-release choice (as Pocket Curator's A / Y)
      once there is a stable release; show the changelog of the version
      offered.
- [ ] Not tried on a device: the installer's games-list refresh, the
      firmware's curl / unzip / sha256sum, Restart Now under ROCKNIX.
- [ ] Before a PortMaster submission: `port.json` against their
      generator / checks (`availability`, `store`, `min_glibc` unverified),
      the gameplay screenshot (Tom: later), an armhf build if wanted.
- [ ] Keep the `cyd` pin current (built on CYD 0.197.1).

## Done
- [x] v0.4.0 (2026-11-14, Tom's first run on the RG40XX H: installer OK,
      A / B right, L2 / R2, sound, picture fit all fine): box-art game
      list, sound sets, the Companion on Y, L2 / R2 = list up / down,
      Modify's single highlight, the games' music (CYD 0.197.1).
- [x] v0.3.1: the installer's stray log. v0.3.0 (2026-11-14): releases as Pocket Curator's - installer,
      Check for Updates, pre-releases; Send Logs to a WebDAV server.
- [x] v0.2.0 (2026-11-14): B = Back / the menu's Exit (CYD v0.196.52
      `play::exit_point`); first real game run through the port (Curse,
      GOG files, headless); stray pointer at start fixed;
      `tools/play_shots.py`. Tom: name OK, test device RG40XX H / ROCKNIX,
      game files in the Project.
- [x] v0.1.0 (2026-11-14): repo set up; engine as submodule; SDL2 platform
      layer and front end; buttons-only controls incl. Pointer Mode;
      PortMaster package; CI.
