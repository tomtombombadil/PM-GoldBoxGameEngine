# TODO - Tom's and Claude's open items

Tick off with the version that did it. Never ask Tom the same thing twice.

## Tom
- [ ] **Game files for testing here**: this Project has no files yet, so
      Claude has only tested the port's own screens (no real game has been
      run through it). Add the game zips to this Project (as in the CYD
      Project: one zip a game, GOG files) - Curse or PoolRad first.
- [ ] **First run on a handheld**: install the `goldbox-portmaster`
      artifact from the latest Build run, add a game, and report (each
      problem its own message, as with the CYD): does it start; are A / B
      the right way round (Start menu - Confirm Button swaps them); do
      L2 / R2 work; sound; speed; how the picture fits; which device and
      firmware.
- [ ] Which handhelds / firmwares do you have to test on? (PortMaster asks
      for AmberELEC, ArkOS-family, muOS, ROCKNIX, Knulli and 640 x 480 +
      other resolutions before a submission.)
- [ ] The port's name: `goldbox` / "Gold Box Game Engine" - OK?
- [ ] Do you want it submitted to PortMaster's catalogue eventually? It
      needs a gameplay screenshot in the package - a picture of a real game
      screen, which the CYD rules keep out of the repo. Your call how to
      square that (a screenshot only in the PortMaster PR, not here?).
- [ ] B button: the engine has no "Esc" (the CYD has none on purpose); B
      only backs out of questions (`play::back`). Should B also pick a
      menu's Exit word? That would be a new `play::` call in the CYD repo -
      not a bug fix, so it needs your OK.
- [ ] `cyd/src/app/library.cpp` (the game scan) is tied to Arduino's
      files, so the port has its own copy of its rules (`src/library.cpp`).
      Moving the CYD's onto the platform layer would let both share it -
      not a bug fix; your OK needed.

## Claude
- [ ] Play a real game through the port headless once game files are
      here (SPEC roadmap 3); record which screens were reached by buttons.
- [ ] Journal viewer; auto-map / Companion views; Cheats; Quick Actions;
      comfort switches; lent sounds (SPEC roadmap 4).
- [ ] Game art in the game list (roadmap 5).
- [ ] Check `port.json` against PortMaster's generator / `build_release.py
      --do-check` before any submission (`availability`, `store`,
      `min_glibc` values are from their packaging page, unverified).
- [ ] An armhf build if any of Tom's devices need one.
- [ ] Keep the `cyd` pin current (built on CYD 0.196.51, commit 57259df).

## Done
- [x] v0.1.0 (2026-11-14): repo set up; engine as submodule; SDL2 platform
      layer and front end; buttons-only controls incl. Pointer Mode;
      PortMaster package; CI.
