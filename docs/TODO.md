# TODO - Tom's and Claude's open items

Tick off with the version that did it. Never ask Tom the same thing twice.

## Tom
- [ ] Send Logs (v0.8.0) on each handheld: does it say "The logs were
      sent", does the email arrive with the device / firmware / PortMaster
      lines filled in (not "unknown"), and does the thank-you note reach
      the address you typed? Whatever reads "unknown" in the log's head on
      a device: tell Claude which line.
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
- [ ] v0.6.0, checked headless with Curse (GOG): the scan's screen and
      its mark, the logo page, the Companion from the right and beside
      the game at 1280 x 720, Quick Actions (a switch flipped, kept),
      Cheats - Healer and Trainer opened, every Settings line.
      **Not yet**: Merchant / Sanctuary used through, Max Hit Points at a
      real level-up, the Battle view beside a fight, the other games'
      logos (each GOG folder's .ico / .dll), a wide panel on a device.
- [ ] Journals (v0.7.0): every game's made and listed in both releases
      (`build/jlist <games>`; engine 0.198.1 fixed the Steam Krynn PDFs,
      Champions' tavern tales, Treasures' Introduction). Opened and
      looked at: Curse GOG J 4 / 12 (every zoom), Curse Steam J 12 / 4,
      Treasures Steam's Introduction. **Not yet**: entries of the other
      games looked at one by one; the whole-book view (BOOK.TXT is made,
      no screen for it); a real mention reached by playing (the test hook
      `journal:K:N` so far). Known gaps (the CYD's TODO too): the Dark
      Queen's Introduction is only its heading lines; GOG's Dark Queen
      journal comes from the PDF's text (no table) and lacks entries 13
      and 26; Death Knights has no entry 13 in any source.
- [ ] Tom: after v0.7.0 on the RGB10 MAX3 - do the game list's pictures
      show at once, at every start? If not: Send Logs (the log's
      "[video] panel" lines say what sizes the panel reported).
- [ ] From the CYD, still missing here: the journal as a whole book; the
      icon gallery by buttons (unchecked); a logs screen (Send Logs
      instead); the Game menu's Sounds tab if it has one worth having.
- [ ] Updater: a stable / pre-release choice (as Pocket Curator's A / Y)
      once there is a stable release; show the changelog of the version
      offered.
- [ ] Not tried on a device: the installer's games-list refresh, the
      firmware's curl / unzip / sha256sum, Restart Now under ROCKNIX.
- [ ] Before a PortMaster submission: `port.json` against their
      generator / checks (`availability`, `store`, `min_glibc` unverified),
      the gameplay screenshot (Tom: later), an armhf build if wanted.
- [ ] Keep the `cyd` pin current (built on CYD 0.199.0). Tom, 2026-11-14:
      a minor Companion display change is coming in the CYD - not pushed
      yet at 0.199.0; pick it up when it lands.

## Done
- [x] v0.9.0: CYD 0.199.0 - the Dark Queen's and Champions' music, made
      by the scan in both releases (checked: 8 songs / 1 song each, the
      title song plays).
- [x] v0.8.1: Tom's first real Send Logs (R36S / ROCKNIX, v0.8.0) worked,
      every head line filled; the scan's PMSCAN.TXT marks no longer sent
      (they crowded the 12 files); the message points to the spam folder.
- [x] v0.8.0 (2026-11-14, Tom): Send Logs by the internet - the log
      relay (an Apps Script in the logs mailbox) emails them; the reply
      address prompt and the thank-you note; the WebDAV server and its
      setting gone; every log headed by the port's version, the device's
      make and model, firmware and PortMaster versions.
- [x] v0.7.0 (2026-11-14, Tom): journal viewer's X (text: next page;
      pictures: Width / Double / Whole / Turned); the game list's
      pictures checked once the panel's size has settled (RGB10 MAX3);
      Rescan Game Files on the game list's first menu; "v" before the
      version; the journal scan's misses fixed in the engine (CYD
      0.198.1) and every game's journal checked in both releases; music
      16-bit (CYD 0.198.0).
- [x] v0.6.2 (2026-11-14, Tom on the RGB10 MAX3): game list and scan
      fill a 16:9 panel; smaller scan letters; no game counter; Game
      Fixes' words.
- [x] v0.6.1 (2026-11-14, Tom: nothing processed at start-up or first
      use): the scan also makes the program's copy and the game list's
      picture (ART.BIN, only read afterwards); no loading screen.
- [x] v0.6.0 (2026-11-14, Tom): the Companion from the right, and beside
      the game on wide panels; journals and music made when a game is
      found (Look for Games Again: anew), not at its start; the GOG logo
      on the game list; Settings page with every engine comfort, Cheats,
      Quick Actions.
- [x] v0.5.0 (2026-11-14): the Adventurer's Journal - made on the device,
      shown in the game, the Start menu's list.
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
