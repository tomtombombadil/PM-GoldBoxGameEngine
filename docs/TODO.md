# TODO - Tom's and Claude's open items

Tick off with the version that did it. Never ask Tom the same thing twice.

## Tom
- [ ] v0.13.0 **Tom: Clean Up Game Files** (Start menu, beside Rescan): run it on a handheld with GOG and Steam
      folders, then play a game of each; the report is games/_CYD/CLEANUP.TXT (also sent by Send Logs).
- [x] Handhelds' Picture: Tom, 2026-11-23 - the Companion choices are Android's; the handhelds' three back (v0.12.20).
- [ ] v0.12.17 (app 0.1.3) on the Tab S2 and the Pixel: the game larger, the buttons over it; the arrow at the top
      left; Controls - Button Size / Opacity; Display - Picture's two new choices, landscape and portrait.
- [ ] v0.12.16 (app 0.1.2) on your phone: a fight - move a fighter diagonally with the on-screen D-Pad's corners;
      after the fight (Curse's Fire Knives) the 3D view is back at once. The buttons read MENU and MOUSE.
- [ ] v0.12.14 (app 0.1.1) on your phone: tap Modify Character in the party menu - the highlight goes there;
      the Start menu chooses on one tap; turn the phone (Auto) and try Display - Screen Rotation.
- [ ] The Android app (APK-GoldBoxGameEngine) on your phone: the folder screen (default folder, then Choose
      Folder on one of your own), a game copied in over USB, the scan, then play by touch alone (On-Screen
      Buttons Auto), then with a Bluetooth controller (the buttons go away; a touch brings them back).
- [ ] The X55's red rclone line at Quit (ROCKNIX nightly; Tom: not after other ports): ROCKNIX's cloud backup
      after a game (CLAUDE.md has where). To settle why only here: quit this port with Wi-Fi off (no line
      expected), and another port with Wi-Fi on (the line expected). To be rid of it: ROCKNIX's cloud backup
      setting off, or rclone set up.
- [ ] v0.12.9 on a handheld: Start - Cheats - Disable Race Level Limits on, then train Oratisi (the fighter goes
      past 7) or roll a new elf (the levels no longer follow Strength / Intelligence).
- [ ] The CYD boards got the same cheat as a Cheats-page key "No Race Limits" (engine 0.211.13) - compiled by CI
      only, never seen on a board: a look on a CYD when you flash next.
- [ ] v0.12.8 on a handheld: Darkness - out of Phlan's first map by Traitor's Gate (by the Temple of Tempus)
      and back; Oratisi (the Steam release's second saved game, loaded afresh) trained at the hall - fighter and
      magic-user both rise, the experience stays 500,000, the Companion's name turns from magenta to cyan.
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

- [ ] Tom, v0.12.1 on the RG40XX V (Knulli): does Quit come back to EmulationStation now? If it hangs again, Send Logs
      after the next start (the log's last lines say what the launch script did). Also the new menus: anything hard to
      find or read?
## Claude
- [x] (done 2026-11-26, see v0.14.1's section) **Treasures play-through** (Tom, 2026-11-17; scratch `/tmp/claude-0/tplay`,
      the finished Gateway party brought over). Started: the party added
      from Gateway, the opening story, Llorkh's first fixed fight (the old
      tower's griffons and Lord's Men) won. Two engine faults found before
      the first step and fixed (engine 0.207.2, v0.11.0): Gateway's items at
      the transfer; Strength lost after a load (Darkness too). 2026-11-17 afternoon: **Llorkh
      town done** - all twelve fixed fights met and won (the taverns' dwarves
      helped, the wounded Zhentil's room, the Bane temple, the drider party),
      the keep's courtyard (the easy version, as the script
      picks after seven town fights), Geildarr's illusion and the efreet
      behind it, the stairs down. **The keep's dungeon**: journal 8, the barracks' and store rooms' fights, the
      caped man's meeting, **the two Siulajias - an engine fault (Cast did
      nothing), fixed: engine 0.208.1, v0.11.2** - then Dispel Magic, the
      false one's guards (five cockatrices: three members turned to stone,
      put right at the Healer cheat), journal 24 and 19, Siulajia joined.
      Aids used: hit points 220, the Healer after each fight, the magic-users'
      books filled by the test program (the Gateway party came with first-
      level spells only - my Gateway training picks). Walked again 2026-11-17 evening with every text kept and
      the loot taken: **Llorkh's journal entries all fired where the scripts put
      them - 1, 3, 5 (town), 8, 24, 19, 11, 10 (dungeon)**; the GOG journal
      the scan made has entries 1 - 83. **A second engine fault: Llorkh could
      not be finished (the skeleton's fight came again, Geildarr never met) -
      fixed, engine 0.208.4.** Geildarr defeated, Milzorr's thanks. NOT yet: loot taken
      and sold (the walks so far left the treasure - to put right), Geildarr
      himself (north of the dungeon), Loudwater on.
- [ ] **Switches' effects in a game, through the port** (Tom, 2026-11-18; `tools/switch_test.py`, `tools/switch_cases/`;
      v0.12.3's CHANGELOG has the table). SEEN on and off: Coin Exchange, Game Font, Savegame Info, More Characters,
      Icon Gallery, Icon Portrait (Curse only - PoolRad shows its own portrait first; Secret / Darkness / Champions /
      Treasures showed no change: right for those games, or the test's View? to check), Camp Fix, Show Rolls, Keep
      Memorizing, Max Hit Points, Multi-Class Hit Points, Raising the Dead (Darkness), Race Adjustments (Darkness).
      Every Game Play line reaches the engine in all nine games (`wiring`). NOT SEEN yet: Auto Join Items (needs a
      treasure with a pile the taker holds), Flasks of Oil (a fight with a troll, a flask thrown), Quick Actions On /
      Off, Magic Missile (PoolRad, a magic-user of level 3+), Thrown Weapons (Show Rolls' to-hit sum), Magic Items
      (PoolRad's Flame Tongue), Age with Time, Power Word Kill, Protection from Evil, and 45 Game Fixes - each needs a
      prepared save (the CYD playsim's RECSET / SETXP / MEMSET) and its steps. The tour is unsteady in Knights, Queen
      and Gateway (their screens change by themselves between runs): those games' Enhancements not shown yet.
- [x] v0.12.8: Treasures' overland fights start two map squares apart, as the original (engine 0.211.8 - its sight
      routine sets the range on open ground).
- [ ] Speed test (`tools/hand_test.py ... speed`) by the port's buttons: passed in PoolRad, Curse, Champions, Knights,
      Queen, Gateway, Treasures. Secret and Darkness: no fight reached by the test's walk - seen in the engine
      instead (Darkness: Speed's picks and Exit by the playsim; both: navcheck's empty-fight-menu rule, 0 failures).
- [ ] **Tom to try on a device (v0.12.6)**: Gateway's fight Speed (Faster stays lit; Exit / B bring the fight menu
      back); the Companion's magenta name and class when someone can train; Treasures: load a save made on the
      overland (the map must be there), a fight on the overland (grass, not river banks).
- [x] (done 2026-11-26) **Treasures play-through** (the Project's claude/playtest_treasures.md): Llorkh, Loudwater, Secomber, Leilon,
      Waterdeep's caverns, Waterdeep Village's dragon done; Daggerford next. Open: the Leilon potion; ELWYN level-drained (to restore); loot not yet ID'd / sold.
- [ ] **After v0.12.5: every game's battlefields looked at** - the fix was seen in Treasures, PoolRad (port) and Curse (engine
      test) only. To do: a fight indoors and one outdoors in each of the nine games through the port, the pictures looked
      at (the CYD's tools/devkit/fieldsurvey for the engine side); Gateway's wilderness fights above all.
      **Lesson**: the play-throughs and button tests read TEXT - none of them looked at a fight's picture, so two days of
      wrong battlefields went unseen. A fight test must look at the field (a shot, or the ground's tiles counted).
- [ ] Tom, v0.11.0 on a handheld: Save Slot Info - Encamp - Save and the
      party menu's Load: the highlighted slot's date (is the device's clock
      right?), place and party; on the RGB10 MAX3 the party and (v0.11.1) the save's map in the
      Companion.
- [ ] **Gateway play-through** (Tom, 2026-11-16; the CYD kit's `gateway/`
      folder, engine at full speed, a new party of Tom's shape; notes in
      the Project's claude/playtest_gateway.md). 2026-11-16, v0.10.7: from
      the party menu to Luskan with the Statuette of the East - Yartar
      (Krevish, the mission), Nesme (the Banite shrine), Silverymoon (Erek;
      Broadhand's forge on the festival day), Everlund (Amanitas, the
      ring), Yartar again (the capture at the gate, the Kraken base and its
      captain), Neverwinter (Lord Nasher, a garden), Port Llast (the ship's
      password door), Luskan's Host Tower (Brinshaar, the statuette). Every
      event on that chain fired where the script puts it once the three
      faults below were fixed. Found and fixed (engine 0.203.9 / .10): Yes /
      No always No; the cell's grating (Dex / thief level read as 0; the
      fall's damage). 2026-11-16 evening, no new engine fault proved: on
      through Tuern (the ore), Gundarlun (Jagaerda), the Purple Rocks,
      Trisk, Neverwinter's sword-smith (the Sword of Stonecutting after 3
      days), Secomber, Loudwater, Llorkh, the Star Mounts and Sundabar -
      **all four statuettes, the party at the arena gate in Ascore**.
      Random overland encounters: met, about ten. **The arena's exits:
      an engine fault, proved in DOSBox and fixed (engine 0.203.11,
      v0.10.8)** - the party now comes out of a fight it ran from facing
      the way it left, and the field ends where the map does. Checked for
      the south way out only (one fighter, the first arena fight, the
      same save in both); east / west / north use the same code, not
      tried in the original. **2026-11-17, v0.10.10 (engine 0.205.1): THE
      END REACHED - the arena's four fights, the altar, the ending's six
      pages, "Victory is sweet", Yartar.** On the way three more engine
      faults, each proved against the original (DOSBox from the same save,
      and its program read) and fixed: the arena's fights are maze fights
      (no end at the last enemy, walk off an edge, no Flee question, over
      as the first member is out); the ending was six empty pages and the
      party menu; Krevish wasn't drawn in fights. Compared with DOSBox: the
      first fight's field and view (same picture), the south way out, the
      experience after a fight (20 a member with nobody out, 24 with one of
      seven out - the same in both), all six ending pages (same pictures
      and words; the original's colours are its own VGA shades). How the
      end was reached: the party's hit points set to 220 (Tom: Modify
      freely) and everyone put back up between the arena's fights - a
      play-through aid in the test program only; the last fight was got
      through with it, not won by tactics. In the original the ending was
      watched by a scratch copy's script made to jump to it (its fights
      skipped: the second one's trolls never stay down). NOT done: the
      ending played with the port's buttons on a device (the same key path
      as Curse's ending); the arena's east / west ways and its other
      rooms; Neverwinter's other gardens and the optional places. (3) the
      magic-users got no new spells from the Trainer's level-ups (only
      first-level ones at level 6) - the game's way (scrolls) or a miss?
      (4) after entering a town from the overland's Move, the menu line
      still reads "Move (Use arrow keys): Exit" until the next step -
      compare with the original.
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
- [ ] Before a PortMaster submission (their packaging page, read
      2026-11-16; Tom asked how we stand): HAVE - folder / script names,
      port.json, README (notes, controls, compile, thanks), gameinfo.xml,
      licenses, control.txt / get_controls / mod_<CFW> / gptokeyb /
      pm_platform_helper / pm_finish / log.txt, aarch64, no game files.
      NEED - (1) a testing thread in their Discord's #testing-n-dev with
      runs on AmberELEC, dArkOS, muOS, ROCKNIX (Libmali and Panfrost) and
      Knulli, 640 x 480 and a higher resolution - "Pull Requests submitted
      without documented testing ... will not be accepted" (have: ROCKNIX,
      Knulli, dArkOS; not AmberELEC, muOS; which ROCKNIX variant?);
      (2) screenshot.png - gameplay, 4:3, 640 x 480 or more (Tom: later;
      a real game screen in their repo - his call); (3) port.json through
      their generator: `store` (GOG / Steam links), `min_glibc`, the arch
      name for x86 ("x64"?); (4) Claude's guess, not in their rules: a
      catalogue build without Check for Updates / the installer (their
      app updates ports) and a word with them about Send Logs (a port
      that uploads logs) - ask in the testing thread; (5) the PR itself
      from Tom's fork of PortsMaster/PortMaster-New, branch `goldbox`.
- [ ] Keep the `cyd` pin current (built on CYD 0.221.2). Tom, 2026-11-14:
      Companion map / party-info changes are in work in the CYD (small
      boards first; the party-info layout may be rearranged) - when they
      land, move the pin and bring `src/panel.*` along.

## Done
- [x] v0.12.7 (Tom, 2026-11-20: "fix the faults you found during testing"; engine 0.211.5): B inside a treasure's Take menu left the
      treasure - fixed, seen by buttons in PoolRad; Treasures' lucky papers' plural; navcheck's two false alarms (all nine games clean).
- [x] v0.12.6 (Tom, 2026-11-20; engine 0.211.4): a fight's Speed menu (the highlight stays on Faster / Slower; Exit and B bring the
      fight menu back); the Companion's ready-to-train magenta; Treasures' overland (saves keep the map, fights outdoors, no Area) -
      the last three found by the play-through and set against the original in DOSBox. `tools/hand_test.py ... speed`.
- [x] v0.12.5 (Tom, 2026-11-18: Treasures' fight field black; engine 0.211.1): indoor fights got an outdoor field in every game
      but Gateway since engine 0.203.12 - this session's own Gateway change. Fixed in the CYD repo. Character Info switch (0.211.0).
- [x] v0.12.4 (Tom, 2026-11-18; engine 0.210.9): the engine's spell-memorizing fixes; the pin moved. Curse's temple Raise Dead
      (found by v0.12.3's switch test) was put right in the engine, 0.210.1, by each original's routine.
      hand_test `menus`, nine GOG games: 22 - 140 words each taken by buttons, Encamp - Magic - Memorize reached in eight;
      flagged only the Dark Queen's View / Drop (the test can't read its coin prompt - known); its Memorize not reached.
- [x] v0.12.3 (Tom, 2026-11-18): switches tested in games - see the open item for what was and wasn't seen.
- [x] v0.12.2 (engine 0.210.0): Coin Exchange and Flasks of Oil as Enhancement switches; the pin moved.
- [x] v0.12.1 (Tom, 2026-11-17): Quit hung on Knulli (pm_finish an alias the launch script didn't know) - fixed; Rescan Game Files and
      Check for Updates on the main menu; Quick Actions one screen with its On / Off line at the top.
- [x] v0.12.0 (Tom, 2026-11-17; engine 0.209.0): the menus as the CYD's tree - Display, Audio, Game Play (Original Games, Enhancements,
      AD&D 1E Rules, Game Fixes - each rule and fix its own switch), Cheats, Quick Actions, Controls, System (Device Info new).
- [x] v0.11.2 (engine 0.208.1): Treasures - a spell cast where a script waits for one (the two Siulajias in Llorkh) reaches the script.
- [x] v0.11.1 (Tom, 2026-11-17; engine 0.208.0): Save Slot Info shows the highlighted save's map, square and heading in the Companion (16:9, 1:1).
      Seen at 1280 x 720: PoolRad, Curse, Secret, Darkness, Knights, Treasures; Curse at 720 x 720. Not seen: Gateway. Stock saves / overland: "No map here."
- [x] v0.11.0 (Tom, 2026-11-17; engine 0.207.2): Save Slot Info - Settings line, the saves' dates, the docked Companion shows the slot's party.
- [x] 2026-11-16 (Tom: use the game files' own knowledge, no random walks): `tools/guide.py`, the CYD sweep's EVMAP; shops / temples / halls walked into. Coverage above.
- [x] v0.10.6 (Tom, 2026-11-15: "Go ahead with your testing ... using the cheats"; engine 0.203.7): a new game in every game by buttons;
      Pools of Darkness' new game hung after the tour - fixed in the engine. `tools/newgame_test.py`. Coverage above.
- [x] v0.10.5 on Tom's RG40XX V (his log, 2026-11-15): the firmware reports itself "knulli scarab", the launch script
      found it ("Knulli: Confirm starts on B"), `ab_set=1` written. His unit was already swapped by hand, so the flip
      itself was seen only in the smoke test - a fresh Knulli install by a tester would show it.
- [x] v0.10.9 (Tom, 2026-11-17, TrimUI Brick Pro on Knulli): the screens' button names follow the printed
      letters on a reversed firmware (`confirm_name` / `back_name` with `ab_reversed`). Tom to check on the Brick Pro.
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

## v0.12.8
- [x] Darkness, Phlan's gates (Tom): engine 0.211.6; `tools/edge_test.py`.
- [x] Multi-class training (Tom, Oratisi): engine 0.211.10 - every ready class but in Curse / Gateway.
- [x] The Companion's party view redrawn on a line's change (the port's signature).
- [x] Treasures' overland fight distance (engine 0.211.8); Aim's Next / Prev foes only (Darkness-type).
- [x] v0.12.9: Game Fix Training Every Class (Curse, Gateway; engine 0.211.11). Its effect by the port's buttons: not played.
- [x] v0.12.9: the training by the port's buttons (norace.py, Darkness). [ ] The Speed test by buttons in Secret /
      Darkness still open - Steam Secret's opening fight plays by itself (the save's party on Quick?): find out.
- [x] (done 2026-11-26; the Leilon potion never found among the loot) Treasures' play-through: Daggerford next; Elwyn's lost levels; the loot since Secomber; the Leilon potion.

## v0.12.10
- [x] The level-limit cheat tested in use (Tom, 2026-11-21): engine 0.211.14's two fixes; `racecap.py` by buttons in
      PoolRad, Curse, Secret, Darkness, Gateway, Treasures; the on / train / off / save / reload cycle in Darkness.
- [ ] The same by buttons in Champions, Knights, Queen - needs a prepared save with a Krynn race (the playsim's RECSET
      sets record bytes only: an EXTSET for the Krynn fields, or a character made in the game).
- [ ] Restoration (a drained character past its limit, the cheat off): its own limits still "level == limit" - look.

- [x] v0.12.11 (Tom, 2026-11-23): the scan's list in his layout (indents, a heading + a counter in place, short lines);
      "No music: ..." when a copy lacks its music files (a player's v0.12.7 log: a silent Dark Queen, no reason given).
      Checked on the PC build (Steam set, GOG set, a cut-down copy; 640 x 480 looked at) - not on a device.
- [ ] Tom: look at the new scan list on a device (Start - Rescan Game Files), 4:3 and the 16:9 MAX3.
- [ ] Tom's edited list had "Making its music..." under five games but not under Darkness - taken as: gone everywhere.
      Say if it should stay.

## v0.12.12
- [x] A shop's goods in the next fight's treasure (engine 0.214.10; found at Ruathym, the original run in DOSBox).
- [x] The D-pad's Left / Right on PoolRad's wilderness and the Krynn overlands (`play::move_keys`); the Companion's
      wilderness map and the engine's secret doors.
- [ ] Tom, on a handheld: PoolRad's wilderness (leave Phlan by boat) - walk all four ways, the Companion's map there;
      Champions' overland - Move, then Left / Right (west / east) and L1 / R1 (the diagonals).
- [ ] Not tried by buttons: the Dark Queen's overland; Champions' step itself (the test's square ran into a fight).
- [ ] Curse / PoolRad shops: do their goods stay on the item list after the shop? (the CYD's TODO has it.)

## v0.13.1
- [x] The pin at the engine's newest commit (eb8cf7a: Clean Up's report names each folder removed). v0.12.13 - v0.13.0
      were the Android app's session's (touch, the on-screen buttons, Clean Up Game Files); the handhelds' side of
      them looked at here by the play test (PoolRad) and shots at 640 x 480 and 1280 x 720 only.
- [ ] Tom, on a handheld: Start - Clean Up Game Files on a card with a whole GOG / Steam copy (it deletes - the games
      still start after? the report: games/_CYD/CLEANUP.TXT).

## v0.13.2
- [x] Treasures' last levels (cleric 10, the others 11) and Darkness's / Treasures' cloaks of protection with magic
      armour (engine 0.216.1) - found by the play-through, each run in the original (DOSBox) from the engine's save.
- [ ] Tom, on a handheld: a Treasures or Darkness party with a Cloak of Protection over magic armour - its AC is one
      (the cloak's plus) worse after loading; that is the original's figure.
- [ ] Darkness's cloak rule seen in DOSBox (by its code so far); what the original's hall says to a member at the
      last level (nothing was seen; the engine: "Not Enough Experience").
- [x] (done 2026-11-26) Treasures play-through: Bjorn's Hold done (journals 2, 9, 78, 79; 69 only by running its script branch - a
      5 % path encounter); Icewolf, the Ice Peak, Freezefire next. The two blackrobed fights were won with the test
      program's WINFIGHT (lost on 18 of 18 dice by Quick play).

## v0.14.1
- [x] Engine pin 0.216.2: Treasures' PROGRAM 5 (the party made well, then a key - the ending, the demo's fight).
      Seen in the engine's test program; not through the port's buttons.
- [x] **The Treasures play-through reached the ending** (2026-11-24; notes: the Project's
      `claude/playtest_treasures_2.md`). The lair's two fights were won with the test program's help, not by play.
- [x] Treasures' journal ledger closed (2026-11-25): every entry a script cites has fired by play - 4 (the old
      sage on the road), 26 (Waterdeep's gate before Leilon), 47 (the tour's lifeboat) and 59 (Wimeg before Froal)
      played from earlier saves; 17 and 39 had fired (my hand list was wrong - now counted from the transcript).
      No script names 6, 13, 14, 15, 82. Also done: the old tunnel at 54;10,8 (the Room of Three Doors, in and
      out), Fireshear's mine pocket (under the shaft at 12,8). In the engine's test program; no engine fault found.
- [x] Treasures' open list worked off (2026-11-26; notes: the Project's `claude/playtest_treasures_3.md`). In the
      engine's test program on the GOG files unless said: every fired journal entry's wording read beside its
      scene (the Steam scan's text) - each fits; the Crossroads (its centre, the portals, four of its eight
      fights, the way out), the Hall of Ten Doors; the ending with Gauhok alone and with both NPCs; the lair
      won by Quick play without help on 6 of 8 dice. In the original (DOSBox) beside the engine, the same in
      both: the "&7" line, the training hall's "Not Enough Experience" at a class's last level, "The water is too
      deep!" by Yartar, Fireshear's pocket fight (4 loaded, 3 on the field).
- [x] v0.19.1 (engine 0.217.2): **Treasures' overland step added ten minutes of its own** - the original adds none
      (DOSBox, the same save and steps). Not checked: Search off in the original; Gateway's overland.
- [x] **The lair and the ending through the port's buttons** (2026-11-26, the x86_64 build here, GOG files): two
      runs from the save before the lair to the overland after the ending, two more as far as the rewards - both
      fights by Quick, the party made well (seen as full hit points on the next screen; the page's own words not read by the test).
- [x] v0.19.2 / v0.19.3 (engine 0.219.1) **The unexplained Treasures run - explained and fixed**: the originals' fight end
      says "fled" only when a member ran AND nobody is left okay; the engine said it when any one member had run
      (an NPC's morale). Treasures, Darkness; the Dark Queen had no fight-end flags at all (97 fights ask). Read
      from the three programs (Treasures GAME.OVR 0x5884 / 0x59D9, Darkness 0x4D97, DQK.EXE 0x4B67C); seen in the
      engine's test program (the lair with Kriiador made to run: on to the ending). Not run in DOSBox; the first
      run's own member-who-ran was never seen (its log was not kept) - the cause fits, it is not proved for it.
- [ ] Treasures, small things left: the Steam scan's journal entry 42 lacks its heading line; the GOG journal's
      picture entries not looked at one by one; the Crossroads' four other fight cells; the tunnel rooms and the
      ending's healing not set against the original.
- [x] v0.19.1: the pin at engine 0.217.3 (the other sessions' Death Knights and Dark Queen fixes with it).

## Worked through on 2026-11-26 (Tom away: "grind through as much of that list as you can")
- [x] Fight-end flags in every game: an IF straight after a COMBAT only in Darkness (215 of 468), Treasures (63 of
      272) and the Dark Queen (97 of 283) - the CYD sweep's new `combatif`; the other six have none.
- [x] The rule checks that failed (Curse's halfling fighters, PoolRad's 9 training cases): the checks, not the engine -
      they expected the originals' "level == limit"; the engine stops at or past it since 0.211.14 (the cheat off
      again). The checks now expect that; SPEC 11 says it. Curse 6,004 pass (1 without its oracle file), PoolRad 430.
- [x] The menu-check item (Save Which Game, Choose Spell) was the check's own false alarm, put right in 0.211.5; run
      again on Curse and Darkness: 0.
- [x] Treasures' overland clock with Search off, in the original (DOSBox): no minutes added either (13:07 -> 00:07 ->
      12:07). Gateway's overland steps by its own routine (12 hours a step) - never the walking step's minutes.
- [x] **Battlefields looked at through the port** (v0.19.3's build, GOG files, `tools/demo_shots.py`): the Demo's
      fight in Curse (outdoors, trees), Secret, Champions, Knights, Darkness, the Dark Queen, Treasures (indoors),
      Gateway (the sea) - each field right. PoolRad has no Demo (its fields seen before, v0.12.5). **Found: Secret's
      demo party is three black figures** - the original (DOSBox) draws a wizard and two fighters; the engine's, in
      the CYD TODO, not fixed (where the original takes those pictures from was not found).
- [x] Speed test by buttons in Secret: a save set one step before a fight (the playsim), `WALK=up tools/hand_test.py
      ... speed` - the menu, Exit and B back, all right; its indoor field looked at.
- [x] v0.19.8: Speed test by buttons in Darkness - passed (a save one step before the hill giants' room, made by
      the playsim: AREA=1 BLOCK=20 TELE=2,1,0; `WALK=up tools/hand_test.py ... speed`). All nine games done.
- [ ] Never played to an end by the port's buttons: Gateway's ending. Seen by buttons: Treasures'.
- [ ] The CYD TODO does not tick Death Knights' play-through as complete (its kit's notes reach Soth's floor) -
      the CYD session's to confirm.
- [x] v0.19.4 (engine 0.219.3): spell names cut at 23 letters (Treasures' grimoire) - fixed; the original (DOSBox)
      and the port's screen after, the same names whole. Found while checking the old "RADIU" note.
- [x] Gateway's menu line after a town is entered from the overland's Move ("Move (Use arrow keys): Exit" stays):
      the original does the same (DOSBox, Port Llast from the overland) - no fault.
- [x] v0.19.5 (engine 0.220.0): Curse's Girdle of the Dwarves cutting an imported PoolRad character's hit points
      (Tom's Thorin, 56 -> 40) - the original's own (DOSBox: a PoolRad dwarf fighter 8 added by Add Character -
      Pool, the Girdle traded and readied: 40); now Game Fixes' Imported Hit Points (Curse, on by default; playsim:
      on 64, off 40). The port's menu shows the line (menutext); its effect by the port's buttons not yet seen.
      Secret's demo party seen drawn through the port (demo_shots, game 3).
- [x] v0.19.6 (engine 0.221.2): Treasures' new characters at 22,600 experience (they got Darkness's 1,500,001 -
      found when the engine's rule checks were run for every game); Game Fixes' Poison Save Bonus. newgame_test on
      Treasures by the port's buttons: six made, added, modified, Begin, the Merchant, 41 steps, 0 "!!".
      Not compared with DOSBox: a new character's level in Darkness, Champions, Knights, Queen (Curse, Secret,
      Gateway, Treasures were).
- [x] 2026-11-26: Imported Hit Points' EFFECT seen by the port's buttons (v0.19.6's program, Curse GOG, a save
      with the PoolRad dwarf fighter and the Girdle unreadied; tools/drive_steps.py): View - Items - Ready - with
      the switch on 56 -> 64 hit points, with it turned off at Start - Game Play - Game Fixes - Curse 56 -> 40
      (the original's, as DOSBox showed). New characters' level and experience compared with DOSBox in Curse,
      Secret, Darkness, Gateway, Treasures, Champions, Knights - all the same now (the Dark Queen not compared).
- [x] 2026-11-26: the engine's own checks run in all nine games on 0.221.2 (the CYD repo's behave suites - own,
      matrix, levels, economy, features, rules4, toggles - and fightfuzz, campfuzz, navcheck): 0 failures once
      the checks' missing oracle files were made again and three checks of their own were put right. This run is
      what found Treasures' starting experience. Steam's and GOG's Treasures GAME.OVR are the same file.
- [ ] Still open from the day's list: its Scroll of Protection from Paralyzation
      (what the original's does - not found in its code yet), its Dust of Disappearance in a fight; Poison Save
      Bonus's effect by the port's buttons (saves aren't shown on any screen - only the engine's checks saw it);
      the other Game Fixes' effects by buttons; the Dark Queen's new characters against DOSBox.
- [x] v0.19.8 (engine 0.221.4): Exit / B in a treasure's "Take: Money Items Exit" left the whole treasure in all
      games but PoolRad - found by the Darkness Speed test's treasure step, the original seen in DOSBox (Treasures),
      fixed in the engine; by buttons in Darkness: B back at the treasure's menu. Curse's fight by buttons after it:
      its treasure (items only) unchanged. Not seen in DOSBox: Curse, Secret, Gateway, the Krynn games.
- [x] 2026-11-26: Treasures' new characters' hit points are rolled in the original too (three rerolls in DOSBox:
      50, 34, 40 for an elf fighter 5) - no difference.
- [x] v0.19.13 (engine 0.221.11): Curse's camp on the travel map - the camp's own screen and the map back after it,
      as the original (DOSBox, the edge of Tilverton). Seen by the port's buttons (drive_steps, a save at the map).
- [x] 2026-11-27: hand_test `fight` again on v0.19.8 by buttons (GOG; PoolRad, Gateway, Treasures, Champions,
      Knights): Move by D-pad / two together / L1 R1 L2 R2 in all five; Aim and Aim - Manual in Gateway and
      Treasures; Cast in PoolRad and Treasures; Done's menu and Delay in PoolRad; PoolRad's treasure: Take, then B
      back at the treasure's menu. Champions' fight outlasted the test's 900 presses (the test's limit).
- [x] 2026-11-27: **Gateway's ending by the port's buttons** (v0.19.13's program, GOG files): a save made by the playsim
      at the arena's altar (the ending's script, ECL6 block 28; slot H of the play-through's party) loaded by buttons,
      every page pressed through with A - the ending's pictures and words (Vaalgamon dragged down, the desert's
      army, Krevish's ring) - to Yartar's exploring menu. The arena's four fights before it were played in the
      engine's test program (2026-11-17), not by buttons.
- [x] 2026-11-27: **the Dark Queen's new character set beside the original** (DOSBox): a human fighter - level 12,
      1,000,001 experience, 200 steel, THAC0 10, the same sheet and form - the same through the port's buttons.
- [x] 2026-11-27: Curse's Scroll of Prot. Paralyzation set beside the original (DOSBox): Used readied in camp it gives
      effect 0x6D in both (the saves' .FX files the same); 0x6D stops paralysis given by spells, not a monster's
      paralysing touch - in the original's code and the engine alike. Used unreadied: nothing, in both.
