# TODO - Tom's and Claude's open items

Tick off with the version that did it. Never ask Tom the same thing twice.

## Tom
- [ ] **First run on the RG40XX H (ROCKNIX)**: README "Installing" -
      `GoldBox.Installer.sh` from the newest release into `roms/ports`,
      run it from Ports, add a game. Report - each problem its own
      message: did the installer work; does the port start; are A / B the
      right way round (Start menu - Confirm Button swaps them); L2 / R2;
      sound; speed; how the picture fits.
- [ ] **Log Server**: start your dev WebDAV server (writable), enter its
      address under Start - Log Server, try Send Logs; pass me the files
      whenever something goes wrong.
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
- [ ] While "press a key" text waits only A continues; the D-pad does
      nothing there. Decide after Tom's first play whether any button
      should go on.
- [ ] Game selection screen with the player's box art (Steam Launcher) /
      GOG icon, made on the device (roadmap 5) - Tom wants this.
- [ ] Journal: process the player's journal PDFs on the device and show
      entries with buttons (roadmap 4; the GOG and Steam PDFs differ).
- [ ] Auto-map / Companion views; Cheats; Quick Actions; comfort
      switches; lent sounds (roadmap 4).
- [ ] Updater: a stable / pre-release choice (as Pocket Curator's A / Y)
      once there is a stable release; show the changelog of the version
      offered.
- [ ] Not tried on a device: the installer's games-list refresh, the
      firmware's curl / unzip / sha256sum, Restart Now under ROCKNIX.
- [ ] Before a PortMaster submission: `port.json` against their
      generator / checks (`availability`, `store`, `min_glibc` unverified),
      the gameplay screenshot (Tom: later), an armhf build if wanted.
- [ ] Keep the `cyd` pin current (built on CYD 0.196.52, commit a1ee0ea).

## Done
- [x] v0.3.0 (2026-11-14): releases as Pocket Curator's - installer,
      Check for Updates, pre-releases; Send Logs to a WebDAV server.
- [x] v0.2.0 (2026-11-14): B = Back / the menu's Exit (CYD v0.196.52
      `play::exit_point`); first real game run through the port (Curse,
      GOG files, headless); stray pointer at start fixed;
      `tools/play_shots.py`. Tom: name OK, test device RG40XX H / ROCKNIX,
      game files in the Project.
- [x] v0.1.0 (2026-11-14): repo set up; engine as submodule; SDL2 platform
      layer and front end; buttons-only controls incl. Pointer Mode;
      PortMaster package; CI.
