# TODO - Tom's and Claude's open items

Tick off with the version that did it. Never ask Tom the same thing twice.

## Tom
- [ ] **First run on the RG40XX H (ROCKNIX)**: install the
      `goldbox-portmaster` artifact of the latest Build run (README
      "Installing"), add a game, and report - each problem its own
      message: does it start; are A / B the right way round (Start menu -
      Confirm Button swaps them); do L2 / R2 work; sound; speed; how the
      picture fits.
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
- [ ] Before a PortMaster submission: `port.json` against their
      generator / checks (`availability`, `store`, `min_glibc` unverified),
      the gameplay screenshot (Tom: later), an armhf build if wanted.
- [ ] Keep the `cyd` pin current (built on CYD 0.196.52, commit a1ee0ea).

## Done
- [x] v0.2.0 (2026-11-14): B = Back / the menu's Exit (CYD v0.196.52
      `play::exit_point`); first real game run through the port (Curse,
      GOG files, headless); stray pointer at start fixed;
      `tools/play_shots.py`. Tom: name OK, test device RG40XX H / ROCKNIX,
      game files in the Project.
- [x] v0.1.0 (2026-11-14): repo set up; engine as submodule; SDL2 platform
      layer and front end; buttons-only controls incl. Pointer Mode;
      PortMaster package; CI.
