# TODO - Tom's and Claude's open items

Tick off with the version that did it. Never ask Tom the same thing twice.

## Tom
- [ ] v0.9.1 on a handheld: Champions' overland map - Start a game from its
      own save, choose Move, walk with the D-Pad, B to end. (Checked here by
      script and under ARM emulation, not on a device.)
- [ ] The CYD Project's claude/TODO.md needs CYD v0.200.1's two lines (the
      engine fix was pushed from the port's session, which can't write to
      that Project).
- [ ] Send Logs (v0.8.0) on each handheld: does it say "The logs were
      sent", does the email arrive with the device / firmware / PortMaster
      lines filled in (not "unknown"), and does the thank-you note reach
      the address you typed? Whatever reads "unknown" in the log's head on
      a device: tell Claude which line.
- [ ] After the v0.3.1 install the `goldbox` folder wasn't visible until
      the port had been started once. The release zip holds
      `goldbox/games/` and the installer makes it; say if v0.4's install
      (or a refresh of the Explorer window) still doesn't show it.
- [ ] v0.9.5 on the RGB10 MAX3: the title screens centred with nothing
      beside them, the Companion from the party menu on. (Checked here at
      1280 x 720 by script in all nine games, not on a device.)
- [ ] v0.10.0, if you know someone with a 1:1 handheld (RG CubeXX,
      RGB30): the Companion under the game - readable? (Checked here in a
      720 x 720 window: PoolRad's play test with a fight, Curse walking.
      Its letters are 9.6 device pixels there - smaller than the 14 of the
      RGB10 MAX3's Companion.)
- [ ] **Buck Rogers - SET ASIDE** (Tom, 2026-11-16: "disregard the Buck
      Rogers games for now"; don't raise it until he does). Notes kept: (Tom, 2026-11-16: "Consider the impact of adding the
      two Buck Rogers games (Countdown to Doomsday & Matrix Cubed)" - no
      GOG or Steam release, only the old DOS files; they run with
      Gateway's DOSBox settings; files to be uploaded to the Project).
      Claude's first reading, before the files: an ENGINE job (the CYD
      repo) - two new game profiles and the XXVc rules (skills, careers,
      ranged fights with guns, ship-to-ship combat, their own items) -
      then little here: the scan's folder names and logos (no GOG / Steam
      folder shape, no launcher art, no GOG journal PDF: what journal /
      manual files will players have?), the play / new-game tests. To do
      when the files are in: look at them (DAX or the newer format, the
      program's size, what Gateway's profile already reads) and give Tom
      a sized plan before any code.
      Rights and PortMaster (looked up 2026-11-16): not abandoned in any
      legal sense - the Buck Rogers name is fought over by the Nowlan and
      Dille heirs and claimed by studios; the games are sold nowhere.
      PortMaster's published packaging rules say nothing on games no
      longer sold; its two kinds are Ready to Run (free / licensed files
      shipped) and ports where the player supplies files they own - ours
      is the second kind, and it ships no game files. Its port.json has
      `store` (where to buy) and `availability` fields: for Buck Rogers
      there is no store to name. Not written down anywhere found: how
      its reviewers treat that - ask on their Discord at submission time
      (Tom's call). Docs to say only "your own original copy"; no
      download links; keep the Buck Rogers name out of the port's title.
- [ ] Other handhelds / firmwares you can test on, when it matters (a
      PortMaster submission wants several).
- [ ] `cyd/src/app/library.cpp` (the game scan) is tied to Arduino's
      files, so the port has its own copy of its rules (`src/library.cpp`).
      Moving the CYD's onto the platform layer would let both share it -
      not a bug fix; your OK needed.

## Claude
- [ ] Button test gaps (tools/play_test.py; v0.10.1's runs, 2026-11-15:
      all nine GOG games at 640 x 480, 1280 x 720 and 720 x 720, the ten
      Steam folders at 640 x 480): a fight was met and played to its end
      only in PoolRad, Treasures (GOG - the party fell), Champions and
      Knights (Steam); Curse, Secret, Darkness, Gateway and Queen met none
      in 260 steps. No game was played far past its start.
      v0.10.2's round (2026-11-15, the "cheap" five): save / quit / reload
      (9 GOG + 10 Steam; 9 GOG under ARM emulation), Steam + GOG at 1280 x
      720 and 720 x 720, 2,500 random presses a game on a sanitizer build,
      the update v0.9.4 -> v0.10.1 from the real zips - all clean but one
      engine shift of -1 (fixed, 0.202.1). Known test misses, not the
      port's: Steam Secret's opening fight is over before the fight steps;
      Champions' overland has no Area map.
      v0.10.3's round (tools/hand_test.py, GOG, 640 x 480): **fights by
      hand** - Move by D-pad, two directions together and L1 / R1 / L2 / R2
      all step the fighter in PoolRad, Curse, Champions, Queen, Gateway,
      Treasures; Aim (Next, Target) and Aim - Manual (the pad moves the
      aim) in PoolRad, Curse, Champions, Gateway, Treasures; Cast - spell
      - Target in PoolRad and Treasures; Done's menu (Delay) in PoolRad,
      Curse, Champions; the treasure menu in PoolRad, Curse, Champions.
      NOT fought: Secret (no fight in 2,000 steps, 203 squares, GOG and
      Steam), Darkness (4,000 steps), Knights (GOG 2,000 steps; the Steam
      run looped at a Yes / No square) - their saves start in quiet
      places; Queen's Aim / Cast (the test loses its menu after the
      moves). **Menus** (Cast, View - Items / Spells / Trade / Drop,
      Encamp - Magic / Rest / Alter / Fix, four deep, every word lit and
      taken, B back): PoolRad, Curse, Secret, Darkness, Knights, Gateway,
      Treasures whole; Champions its camp only (its save starts
      overland); Queen in part (the test can't read its coin prompt).
      Nothing found wrong in them.
      Phase 2 (2026-11-15, tools/explore_test.py, GOG, no port change):
      **Create New Character to its end** by buttons - all nine (WRONG,
      found 2026-11-16: the test left the race list by Exit in every game
      whose list is headed "Pick Race" and still said OK - only PoolRad
      and Queen were really made. Put right and done again: phase 3). **The
      icon screen** (Encamp - Alter - Icon): the D-pad and L2 / R2 change
      the picture, A on Heads / Weapons / Colors too, Edit opens, B asks
      "Is this icon ok?", Yes leaves - eight games (Champions' save starts
      overland, no Alter there; Queen has no Icon word). **Places met by
      walking**: Curse's temple (Heal / View / Pool / Appraise walked
      through) and training hall, Gateway's temple - no shop was reached
      in any game. **Journal mentions opened by playing**: PoolRad 1,
      Secret 2, Darkness 1, Queen 1, Gateway 2. **Playing further in**:
      Secret 3,330 steps / 203 squares, Darkness 4,120 / 62, Knights
      3,417 / 126 with nothing wrong and no fight; in the other six the
      stock party fell on Quick within 41 - 645 steps (1 - 8 fights).
      Nowhere did the walk get stuck through the port's fault.
      **Not reached**: shops; a second map in most games; fights in
      Secret, Darkness, Knights. A walk that goes deep needs a party that
      survives - made strong with the Trainer cheat first, or a save from
      further in (Tom's, if he has any).
      Phase 3 (2026-11-16, tools/newgame_test.py, GOG, 640 x 480; v0.10.6):
      **a NEW game as a player starts one**, buttons only - six characters
      made (Human Fighters where offered) and found in the party: all nine;
      Modify to the highest figures: PoolRad, Curse, Gateway (the Add /
      Subtract kind of Modify - Secret, Darkness, Krynn, Treasures - not
      driven yet by the test); Begin to the exploring menu: eight at once,
      **Darkness hung after Rolf's tour - an engine fault, fixed (0.203.7,
      v0.10.6)**; armour, shield and weapon **bought at the Merchant's and
      readied** through View - Items: PoolRad, Curse, Champions, Treasures
      (Secret's first tries were too dear - "Not enough Money."; Gateway's
      and Queen's shop not driven: the test's). Then the walk with the
      Healer after fights and free training:
      PoolRad 361 steps / 12 fights (fell, still level 1) - Curse 753 / 13
      fights, a temple walked through - Secret 4,112 steps, 203 squares,
      its tavern and vault met, no fight (the town's Yes / No answered No)
      - Darkness 2,123 steps, no fight - Champions: stopped in its second
      fight (the test met a fight menu without Quick; put right after) -
      Knights: the six new characters, unarmed, fell in the opening battle
      on Quick - Queen 1,000+ steps, 104 squares, 1 fight - Gateway 565
      steps, 260 squares on 5 maps, 18 fights, 36 trainings - Treasures
      206 steps, 6 fights, 6 trainings (one fight outlasted the test's
      1,200 presses). Nothing wrong found in the port's own handling.
      **Still not reached**: a shop walked INTO (only the Merchant's);
      later maps; fights in Secret and Darkness.
      Phase 4 (2026-11-16, Tom: "Don't wander randomly ... Think like a
      player would"; `tools/guide.py` + newgame_test's cluebook walk, GOG,
      new games, engine 0.203.7): the party sent straight to each place by
      the book made from the game's own scripts and walls. **A shop WALKED
      INTO and bought from**: PoolRad, Curse, Gateway, Treasures; Secret's
      shop walked into and its list opened (nothing cheap on the test's
      list). **A temple walked into, Heal's list opened**: PoolRad,
      Darkness, Gateway, Treasures. **A training hall walked into, Train
      tried**: Darkness, Gateway ("Training costs 1000 gp."). **A fight
      gone to and won**: PoolRad. Every place left again to the exploring
      menu. Not yet: Champions (its new game starts on the overland map -
      the walk doesn't drive Move there), Knights (the opening battle),
      Queen (book not made in time); fights on the start maps of Curse,
      Secret, Gateway ("no way there" - behind doors the walk found shut;
      Bash / Pick / the next map not driven yet); going from map to map.
- [ ] Next for the cluebook walk: doors (Bash / Pick), the way from one
      map to the next (the book's NEWECL squares), Champions' / Queen's
      overland, then each game's first quest step by step.
- [ ] Question for the CYD side (2026-11-16): Death Knights' NEW
      characters start high in level but with no armour or weapons (AC 6 -
      10) and meet the opening battle before any shop. Is that the
      original's way? Not checked in DOSBox.
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
      journal comes from the PDF's text (no table) and lacks entry 13 (26 found
      since engine 0.200.6); Death Knights has no entry 13 in any source.
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
- [ ] Keep the `cyd` pin current (built on CYD 0.203.7). Tom, 2026-11-14:
      Companion map / party-info changes are in work in the CYD (small
      boards first; the party-info layout may be rearranged) - when they
      land, move the pin and bring `src/panel.*` along.

## Done
- [x] 2026-11-16 (Tom: use the game files' own knowledge, no random walks): `tools/guide.py`, the CYD sweep's EVMAP; shops / temples / halls walked into. Coverage above.
- [x] v0.10.6 (Tom, 2026-11-15: "Go ahead with your testing ... using the cheats"; engine 0.203.7): a new game in every game by buttons;
      Pools of Darkness' new game hung after the tour - fixed in the engine. `tools/newgame_test.py`. Coverage above.
- [x] v0.10.5 on Tom's RG40XX V (his log, 2026-11-15): the firmware reports itself "knulli scarab", the launch script
      found it ("Knulli: Confirm starts on B"), `ab_set=1` written. His unit was already swapped by hand, so the flip
      itself was seen only in the smoke test - a fresh Knulli install by a tester would show it.
- [x] v0.10.5 (Tom, 2026-11-15): Knulli starts with Confirm on B; "Exit to DOS" in a training hall stays as the originals' (no question).
- [x] 2026-11-15, Tom on devices: RG40XX V / Knulli (A and B reversed - Confirm Button puts it right), R36S / dArkOS ("ran fine").
- [x] v0.10.3 (Tom, 2026-11-15: phase 1 - fights by hand, camp / spells / items; engine 0.203.2): a D-pad tap
      under 70 ms in a fight's Move / Aim was lost - fixed; `tools/hand_test.py`. Coverage above.
- [x] v0.10.2 (Tom, 2026-11-15: the five cheap tests; engine 0.202.1): results above; `tools/save_test.py`, `tools/monkey_test.py`.
- [x] v0.10.1 (Tom, 2026-11-15: "Go ahead with your testing"; engine 0.201.2): the button test on
      every game in three window shapes and both releases. Found and fixed: the Dark Queen's Create
      Character form's two highlights (engine 0.201.1), its knights "Monk" on the Party view
      (0.201.2), journals not made anew by v0.10.0's update. The test reads Treasures and the Dark
      Queen now (text over pictures, off the grid).
- [x] v0.10.0 (Tom, 2026-11-15; engine 0.200.9): `play::titles()` added to the engine and used; the demo keeps
      the Companion; square panels - the Companion under the game; the CYD's Companion look (0.200.7 / .8).
- [x] v0.9.5: no Companion beside a game's title screens on wide panels - the game centred until its party menu (Tom, 2026-11-14).
- [x] 2026-11-14: PoolRad's and Curse's party menus set beside the
      originals in DOSBox (any answer passes GOG's code wheel - Tom): the
      same as the engine, no "Pick Character" there. All nine done.
- [x] v0.9.4 (engine 0.200.5; DOSBox with a made party): Secret's Pick
      Character and its creation questions on Yes; Gateway checked, the
      same.
- [x] v0.9.3 (Tom, 2026-11-14; engine 0.200.4, the originals in DOSBox
      beside the engine): B leaves "Pick Character"; Darkness and
      Treasures ask it for Drop / Modify / View / Remove; Queen's Delete
      Character is the original's list of saved characters. Journals read
      anew (scan 4).
- [x] v0.9.2 (Tom on a handheld, 2026-11-14; engine 0.200.2): Death
      Knights' fight field grey with its ground (was black; checked
      against the original in DOSBox); the Dark Queen's Delete Character
      by buttons. Tom: the Trainer is left with Begin, B needn't.
- [x] v0.9.1: the D-pad couldn't move the party on the Krynn / Darkness
      overland maps (found by the button test; engine 0.200.1). CYD
      0.200.0's Queen ending pictures and yellow trainable classes came
      with the pin. `--drive` and tools/play_test.py added.
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
