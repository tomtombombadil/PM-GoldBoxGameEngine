# Changelog

Every version of Gold Box Game Engine for PortMaster. A release's notes are
its part of this file.

## [0.19.1] - 2026-11-26

Built on the CYD engine 0.217.3.

- **Treasures: a step on the overland no longer takes ten minutes of the game's clock by itself** (engine 0.217.2;
  the original's clock, read in DOSBox: only the map's own travel time counts).
- **Death Knights**: a script's "step ahead" and "3D view again" calls (engine 0.217.1).
- **The Dark Queen**: "This is recorded as journal entry n." is said and the entry offered; its saves with
  imported characters load in the original; indoor fights past a map's 16th column / row get their field
  (engine 0.217.2 / 0.217.3).

## [0.19.0] - 2026-11-25

Built on the CYD engine 0.217.0. **Android only - nothing changes on a handheld.**

- **The journal, Android's own** (Tom): the app shows an entry itself (channel 5, `jour::app_entry`) - a text
  entry's words, a picture entry's scans saved once as pictures (at most 2400 across) for the app's viewer. The
  game's own text screen, its "X Width  B Back" line and its entry number / % line are not drawn on Android. The
  app's close is op 15 (the journal's B).
- **Display › Journal Font: Journal / Book / Serif / Plain** (Android): the type the app shows a text entry in
  (`journal_font` in the settings).
- **System › About** (Android): the app's, the front end's and the engine's versions and repos, the licence, the
  credits (`--app-version` from the app).
- **The Companion placed by the app** (layout view 4, `nativePictures`): the app lays the map and the party out
  by rule for every screen - each picture's place and the rows of it shown; the front end says what the pictures
  hold (channel 4: the map's rows in use, the party's members and their pitch, a fight).

## [0.18.1] - 2026-11-25

Built on the CYD engine 0.217.0. **Android only - nothing changes on a handheld.**

- The Companion's party in the app's column (view 3; Tom): as wide as the map, starting right under the map's
  "Locked Secret", its members' rows only (the empty rows under the map's legend and under the last member cut
  off: `video::set_side_used`) - made smaller only when even those don't fit. Before, the whole picture was shrunk
  to the room left and centred.

## [0.18.0] - 2026-11-25

Built on the CYD engine 0.217.0. **Android only - nothing changes on a handheld.**

- The Android app's menu (Tom, 2026-11-25): **Rescan Game Files and Clean Up Game Files under System** (a
  handheld keeps them on the main menu); **Display › Game Chooser: Auto / Tiles / Cards** - all the games at
  once or one at a time (Auto: tiles on a tablet), also set by the chooser's own toggle (op 14).
- The Companion beside the game in the app's column: the map as wide as the column at its top, the party under it
  in what is left (layout view 3) - the map never made smaller to fit the party.
- System › Games Folder's words in the app name the folder as the phone does (Internal storage › GoldBox ›
  games).

## [0.17.1] - 2026-11-24

Built on the CYD engine 0.217.0. **Android only.**

- The Android app's game chooser lists the games by series (Tom): Gateway and Treasures before the Krynn three,
  so a tablet's last row is Krynn's.

## [0.17.0] - 2026-11-24

Built on the CYD engine 0.217.0 (871d27c). **Android only - nothing changes on a handheld.**

- **Continue** on the Android app's game chooser (Tom, 2026-11-24): straight into the game's newest saved game,
  past the titles and the party menu - the engine's new `play::newest_save` / `play::continue_game`. The chooser
  shows it for a game that has a saved game (`_CYD/<folder>/CONTINUE.TXT`, noted when a game opens and closes).
- From engine 0.216.3 / .4: a flight's runners can fight again; Resist Fire / Resist Cold halve their damage in
  Secret, Champions, Darkness, Treasures and Death Knights.
- Test step `continue` (the chooser's Continue in an open game).

Checked: the smoke test; `continue` with the nine GOG copies on the PC build - eight went into their game at
their newest save (Curse, PoolRad, Secret, Darkness, Champions, Death Knights, Dark Queen, Treasures; shots looked
at); Gateway's GOG copy has no saved game and said so.

## [0.16.0] - 2026-11-24

Built on the CYD engine 0.216.2. **Android only - nothing changes on a handheld.**

- **The Android app shows all of the port's own screens itself** (Tom's redesign, stage 3): besides the menu's pages
  now Device Info, the Button Guide, the journal list, Quick Actions, notices and messages, the game chooser and
  the scan's page - their lines go to the app, its taps come back as the buttons would act.
- **The chooser's pictures are made by the scan** (`_CYD/<folder>/APPART.IMG`: the box art decoded whole, or the
  logo), and no picture is remade when the device turns.
- What 0.15.0 left out of the app's menu (Tom: "you missed some menus and menu items"): the Display page with
  Picture (4:3, Square Pixels, Stretch) and Screen Rotation, Controls - Button Size (for the app's pad), Back to
  Game.
- Test steps `layout:` and `menutext` (the drawer's view of the open page, to stderr) to check these off the phone.

Checked: the smoke test on the PC build; every menu page's lines through `menutext` at the game list and in a game
(Curse); the app on the Android emulator (see the app's 0.4.0).

## [0.15.0] - 2026-11-24

Built on the CYD engine 0.216.2 (as v0.14.1). **Android only - nothing changes on a handheld.**

- **The Android app's menu drawer** (Tom's redesign, stage 2): the app shows the Start menu's pages in its own
  drawer from the left - Android's letters, switches, a slider for the volume with - / + and mute. The menu itself
  is the same one: its pages, lines and words come from here (`android_menu_tick`), its taps act as the buttons
  would (`android_menu`), and a controller still moves about it. Screen Rotation now stands on the menu's top
  page in the app (its Display page had nothing else left).

Checked: the smoke test on the PC build (nothing of it runs there); the app on the Android emulator (the drawer by
touch: the top page, Audio opened and its three lines shown, the volume muted and back, closed by a tap beside
it). Not tried there: the other pages, a controller in the drawer.

## [0.14.1] - 2026-11-24

Built on the CYD engine 0.216.2. One engine fault, found at the end of the Treasures play-through:

- **Treasures: the party is healed where the game heals it** - at the ending ("ice hunters tend your wounds, and
  all are healed") and after the demo's fight the original makes the whole party well (wounds, blindness,
  disease, poison, the dead, the stoned, drained levels) and waits for a button with "Press button or enter to
  continue". The engine did nothing there: the party stayed hurt and that page of the ending was replaced by the
  next before it could be read. From the original's program (read, not run in DOSBox - its two last fights would
  have to be won there).

Checked: in the engine's test program on the play-through's save - after the ending the eight members stand at
their full hit points, a member drained in the Ice Peak has the two levels back, and the page waits with its
prompt. **Not seen through the port's buttons** (it comes after two fights the party wins only with the test
program's help). The smoke test passes.

## [0.14.0] - 2026-11-24

Built on the CYD engine 0.216.1 (as v0.13.2). **Android only - nothing changes on a handheld.**

- **The Android app's own controls round the game** (Tom's redesign, stage 1): the app now draws the pad, the
  edge handles and the Companion's room in Android's own views; the front end draws the game and the Companion's
  pictures where the app says (`src/shell.*`). The old see-through overlay, Display - Picture and the Controls
  page's button lines are gone from the app's menu once the app has laid the screen out.

Checked: the smoke test on the PC build (the shell does nothing there); the front end's half of each layout with
Curse at 2280 x 1080 and 1080 x 2280 (the test step `layout:`). Not yet on a phone.

## [0.13.2] - 2026-11-24

Built on the CYD engine 0.216.1. Two engine faults found by the Treasures play-through, each set against the
original game (its program read, then the original run in DOSBox on the engine's own save):

- **Treasures: training stops at the game's last levels** - cleric 10; fighter, paladin, ranger, magic-user and
  thief 11. The engine had no such stop: a fighter with the experience trained on to 12 and beyond. In the
  original a fighter of 11th level with 1,200,000 experience is not trained in Mirabar's hall; the engine now
  refuses the same. (The journal's tables print a 12th level for fighters and thieves; the program stops at 11,
  and the program is followed.) Characters already trained past it keep their levels.
- **Darkness and Treasures: a Cloak of Protection gives nothing over magic armour** - as a Ring of Protection in
  every game. The engine counted it. The original listed two of my clerics at AC -7 and -8 where the engine
  showed -8 and -9. Saves are put right as they are loaded (those two games work the figures out at Load, as
  Treasures' own Load does).

Checked: the engine's test program and the original side by side from the same save (the eight members' AC
agree; the fighter stays 11); through the port's buttons on the PC build: that save loaded from the party menu
lists the same eight AC. Not done by buttons: the training hall. Darkness's cloak: by its program's code only,
not seen in DOSBox. Not on a device.

## [0.13.1] - 2026-11-24

Built on the CYD engine 0.216.0 at its newest commit (eb8cf7a; v0.13.0 was one commit behind).

- **Clean Up Game Files: the report names each folder it removed** ("Removed the folder Pool of Radiance/DOSBOX") - before, CLEANUP.TXT listed the files and only counted the folders. The engine's change; nothing else differs from v0.13.0.

Checked: the smoke test on the PC build (made-up game folders: the six unwanted files and the DOSBox folder go, the game's files, saves, PDFs and an unknown file stay, the report has the folder's line). Not run on real game folders or a device this time.

## [0.13.0] - 2026-11-24

Built on the CYD engine 0.216.0.

- **Start - Clean Up Game Files** (beside Rescan Game Files): frees space in the games folder. A whole GOG or
  Steam install brings DOSBox, the installer's and the Steam launcher's files, the Gold Box Companion program and
  DOS sound drivers the engine never reads; they go. Every game file the engine reads, every saved game and
  character, every PDF (the manuals, cluebooks, journals) and any file it doesn't know stay. The Steam
  collection's Launcher folder keeps only the box art. A report: games/_CYD/CLEANUP.TXT. On the nine Steam games
  it took out 582 files, 83 MB.

## [0.12.20] - 2026-11-23

Built on the CYD engine 0.215.2 (unchanged).

- **The handhelds' Display - Picture is back to its three choices** - Original 4:3, Square Pixels, Stretch - as
  before v0.12.17 (Tom: "The new display options are for Android, not handhelds. The Port is working and
  displaying great"). 4:3 + Companion Right / Below are the Android app's only. A settings file that picked one of
  them on a handheld reads as Original 4:3.

## [0.12.19] - 2026-11-23

Built on the CYD engine 0.215.2 (unchanged).

- Android: on the port's own pages (the game list, the menus over it) the on-screen buttons' top rows and the
  hide / show arrow start under the title bar - they covered "Gold Box Game Engine" and the version (seen on the
  Android 11 emulator, 0.12.18). In a game they stay at the top corners.

## [0.12.18] - 2026-11-23

Built on the CYD engine 0.215.2 (unchanged).

**The Companion as large as its room allows** (Tom, 2026-11-23: "make them as large as will fit in the area
available to them, keeping the same aspect ratio"):

- **4:3 + Companion Below on a tall panel** (a phone held upright): the map (in a fight, the battlefield) and the
  party side by side under the game, as large as the room down to the on-screen buttons lets them be - on a
  1080 x 2400 phone each about 540 x 620 (the wide strip was 1080 x 270 for both). Where the room is short and
  wide (a tablet upright, or Below held sideways) the wide strip stays - it comes out larger there; the larger of
  the two is chosen by itself.
- Beside the game (Companion Right) it already filled the game's height at its own shape - unchanged.

Checked: the PC build's smoke test; the handhelds' shapes as before (640 x 480, 1280 x 720, 720 x 720); the
layouts drawn by the real video code at 1080 x 2400, 1536 x 2048 and 2400 x 1080 (stand-in Companion pictures).

## [0.12.17] - 2026-11-23

Built on the CYD engine 0.215.2 (unchanged).

**Display - Picture: two new choices, everywhere** (Tom, 2026-11-23: "a 4:3+companion right, and a 4:3+companion
below"): **4:3 + Companion Right** and **4:3 + Companion Below** - the Companion stays there, the game made a little
smaller when the panel has no room beside / under a full-size one (a 4:3 tablet). The choices go 4:3, + Companion
Right, + Companion Below, Square Pixels, Stretch. On a handheld **Original 4:3** is as before (the Companion beside
or under the game where there is room); on Android plain **4:3** means the slide-out only, and the app starts on
4:3 + Companion Right (once - your own choice stands).

**For the Android app - the on-screen buttons are an overlay** (Tom's Galaxy Tab S2: the game was small in the
middle, the button columns took the sides):

- The game gets its whole room; the buttons lie over it, see-through. The D-Pad and the face buttons go in the
  free room beside the game when there is enough (a wide phone), else over its sides - above the game's menu line,
  which stays clear.
- **A touch on a button is the button's; anywhere else is the game's** - only the button's own shape counts
  (the D-Pad's empty corners too, outside a fight).
- **The arrow at the top left** puts the buttons away (pointing up) and brings them back (pointing down); kept for
  next time.
- **Start - Controls - Button Size** (Small, Medium, Large, Huge) and **Button Opacity** (20% - 100%).
- Upright, the game sits at the top and the buttons at the foot.

Checked: the PC build's smoke test (the Picture test now steps through the five choices); the handhelds' shapes as
before at 640 x 480, 1280 x 720 and 720 x 720; the overlay and the docked Companion drawn by the real video code at
2400 x 1080, 1920 x 1080, 2048 x 1536, 1080 x 2400 and 1536 x 2048 (stand-in Companion pictures - no game here).
Not on a phone or tablet yet.

## [0.12.16] - 2026-11-23

Built on the CYD engine 0.215.2 (was 0.215.1).

**Fixed (in the engine, 0.215.2)**

- **The 3D view comes back after a fight** (Tom, Curse's Fire Knives: their picture stayed after the fight until
  a step redrew the view). The engine forgot the picture shown before a fight only in PoolRad; now in every game.
  On every device - handhelds too.

**For the Android app** (Tom, 2026-11-23):

- **Diagonals on the on-screen D-Pad in a fight**: while a fighter moves or aims, the D-Pad's corners are the four
  diagonals - two directions at once, as two buttons on a controller's pad ("there's no real d-pad ... we need
  to add diagonal movement buttons during combat"). A thumb can slide round all eight.
- The on-screen **Start is labelled MENU, Select MOUSE** (Pointer Mode: an arrow the D-Pad moves, A taps where
  it points - in a game only).

Checked: the PC build's smoke test; the D-Pad's diagonals in a test harness (a corner holds both ways, a slide
lets one go, the drawing). The engine's change by code only - not played here (no game files in this session).

## [0.12.15] - 2026-11-23

Built on the CYD engine 0.215.1 (unchanged).

**For the Android app**: **a phone's camera cutout / notch is kept clear** (Tom's Pixel 10a: "the camera cutout
is right in the top center of the game display when in portrait"). The app tells the front end the cutout's safe
margins (Android's own, checked about once a second - they turn with the phone); the game picture, the
on-screen buttons and the Companion are laid out inside them, the cutout's strip left black (`video::set_safe`).
Nothing changes on a handheld or a PC.

## [0.12.14] - 2026-11-23

Built on the CYD engine 0.215.1 (was 0.214.10).

**For the Android app** (Tom's first tries on his phone):

- **One tap chooses** on the port's own pages - the Start menu's lines, the journal list, Quick Actions (was:
  the first tap picked, the second chose; Tom: "It should only take one tap - you know, be more
  Android-friendly").
- **The screen turns with the device** (was: held the way it was at the start). And **Display - Screen
  Rotation: Auto / Landscape / Portrait** (Android; Tom: "an option under Display on the menu to force one
  rotation or the other"). Auto follows the device, its rotation lock too.

**Fixed (in the engine, 0.215.1)**

- **A tap in the game moves the highlight to what it picks** before acting, as the D-pad and A do: the party
  menu's line (tapping Modify Character left the highlight on Drop Character while it asked for a character)
  and the menu line's word. On the CYD boards' touch screens too.

**From the engine (0.215.0)**

- AD&D 1E Rules - **Level Drain & Restoration** (the books' drain and Restoration): a new switch on Start -
  Game Play - AD&D 1E Rules, on by default, off under Original Games.

Checked: the PC build's smoke test (Original Games turns the new rule off too). The engine's tap change not
played here (no game files in this session): the state is the one the D-pad and A reach.

## [0.12.13] - 2026-11-23

Built on the CYD engine 0.214.10 (unchanged).

**For the Android app** (`tomtombombadil/APK-GoldBoxGameEngine` - Tom, 2026-11-23: the app is its own repo; it
builds this repo's front end). Nothing changes on a handheld.

- **Touch.** A touch screen works three ways, in any mix: a controller (Bluetooth / USB / a handheld's own),
  the **on-screen buttons** (the handhelds' D-Pad, A, B, X, Y, L1, R1, L2, R2, Start, Select beside the game in
  landscape, under it in portrait), or **touching the port's own screens**: a line tapped is picked, tapped
  again chosen; swipe to scroll; across = Left / Right (a setting, the next game, a journal page); the game
  list's sides page through the games, its middle starts one, its title bar opens the menu; outside a page's box
  or its title = Back; two fingers together = Start; the phone's Back = B. In the game a tap is the game's own,
  as on the CYD boards. Nothing is touch's alone (SPEC 4).
- **Start - Controls - On-Screen Buttons: Auto / Always / Never** (touch screens only). Auto: shown when the
  screen is touched, hidden while a controller or keyboard is used.
- **Start - System - Games Folder** (Android only): shows where the games are ("Internal storage/GoldBox/games/",
  as the phone and a PC name it) and opens the app's folder screen to choose another.
- The empty game list on Android names that folder the same way.
- Android leaves out Check for Updates, Send Logs and Reply Address (the handhelds' scripts).
- `--touchpad` on a PC: the same buttons and touch, the mouse as the finger.

Checked: the PC build - the smoke test (with a touch run: menu by the title bar, a line picked and chosen,
On-Screen Buttons cycled and saved, swipes, the game list's sides), screenshots at 2400 x 1080, 1080 x 2400,
2048 x 1536 and 1280 x 720; the on-screen buttons' fingers (slides along the D-Pad, two buttons at once, a tap on
the game passed through) in a test harness. The Android app (APK repo 0.1.0) on an Android 11 emulator: the folder
screen, Play, the empty game list naming "Internal storage/GoldBox/games/", the menu by a tap on the title bar,
System - Games Folder back to the folder screen. Not on a phone; no game played on Android yet.



Built on the CYD engine 0.214.10 (was 0.211.14).

**Fixed**

- **A shop's goods turned up in the next fight's treasure.** Found in the Treasures play-through (Ruathym): after a visit to a shop, the next fight's treasure listed the shop's whole stock as well - free. The engine left the shop's goods on its item list. The original was run from the same save (DOSBox: the shop, Exit, the guards' fight - two pages of the guards' own items, the same experience). Fixed in the engine (0.214.10); every game from Secret on had it. Curse's and PoolRad's shops work another way and were not checked.
- **The D-pad's Left / Right on the overland maps where the keys step.** On PoolRad's wilderness they did nothing - no way east or west by the D-pad; on Champions' and the Dark Queen's maps they went north-west / north-east, with no way straight east or west. Now Left / Right step west / east there (the engine's new `play::move_keys`), and on the two Krynn maps L1 / R1 are north-west / north-east. Everywhere else as before.

**From the engine (0.211.15 - 0.214.9)**

- The Companion's map on PoolRad's wilderness: where a step is refused (lines), the places its scripts know (orange squares, "Place"), the party's arrow.
- Secret doors on the Companion's map are the engine's own now: an S wherever the game lets the party through a wall, worked out before the party finds it - no S on a wall that only looks like one.
- Darkness: the ending's movie; training past 255 hit points (Game Fix "Hit Points Past 255"); level drains and Restoration by each original's own routine; the Cheats at Champions' / Death Knights' outposts; Champions' random treasure.

Checked: the PC build with the GOG files. PoolRad's wilderness by the port's buttons from a made save (Right: 0,0 -> 1,0 -> 2,0; Down a row; the Companion's map with its places looked at, 1280 x 720). Champions' overland: Right now faces east (was north-east) - the step itself ran into a fight, so the square's change was not seen. The play test on PoolRad (a fight, camp, the cheats). Not on a device; the Dark Queen's map not tried.

## [0.12.11] - 2026-11-23

Built on the CYD engine 0.211.14 (unchanged).

**The scan's list, tidied** (Tom's layout: "the indent changes, the shortening of the lines and over all neater appearance"):

- A game's "Found ..." line at the left (gold); everything made for it indented two under it; a step's counter indented four under its heading and counting in place.
- The journal: "  Processing the journal:" then "    page 12 of 99" - the game's name no longer repeated in every line. (Steam copies read the PDF's text first: "  Reading the journal:" with its own page count, replaced by Processing when that starts.)
- The music: "  Rendering the Pools of Darkness music:" once, then a line a song - "    song 2 of 8..." while it is made, "(24 s of music, 1 s to make)" added to the same line when it is. "Making its music..." and the separate "Rendered ..." lines are gone.
- **A game without its music files now says so**: "  No music: its music files aren't in the folder." (Darkness, Champions, Death Knights, the Dark Queen, Gateway, Treasures). Before, the step was passed over without a word - a player's log (v0.12.7) had a silent Dark Queen and no reason given.
- "No journal" shortened to "  No journal: its PDF isn't in the game's folder." so it fits one row on 640 x 480.
- The same lines go to log.txt.

Checked: the PC build's scan run on the Steam set (with the Launcher folder), the GOG set, and a cut-down copy (no icon, no PDF, no music files) - the lists read back from the log and the 640 x 480 screen looked at for the GOG set and the cut-down copy. Not on a device.

## [0.12.10] - 2026-11-21

Built on the CYD engine 0.211.14 (was 0.211.13).

**The Disable Race Level Limits cheat, tested in use** (Tom: "at, near, or past the level limit ... turn it on, level up and then turn the cheat back off ... across the games"). Two faults found and fixed, both in the engine:

- **Past its limit, with the cheat off again, a character went on training** - in PoolRad, Curse, Secret, Darkness, Gateway and Treasures. Their limits are the originals' "level equals the limit" tests (in the originals nobody is ever past it), so one level over and no limit held. Now a limit holds at or past it; the limits' values are unchanged.
- **The Dark Queen: the cheat also lifted a knight's order limit** (a knight of the Crown past 18, to 40). Now the cheat only takes the race's part: in the Krynn games the character is read as a human in the race tables.

What was checked, and how:
- **Every game, in the engine** (its new `racecap` check): every race x class x Strength / Intelligence 16, 17, 18, a character with experience for everything, a level at a time by the training hall's own choice - a level under the limit trains; at the limit it doesn't and isn't shown as ready; with the cheat on it trains, up to the game's own last level and no further; then at EVERY level past the limit with the cheat off again: no training, not ready, level and experience untouched; cheat on again: trains on. Limited combinations / checks: PoolRad 15 / 150, Curse 26 / 488, Secret 36 / 1,023, Darkness 30 / 3,114, Champions 19 / 193, Death Knights 87 / 2,742, the Dark Queen 90 / 8,478, Gateway 16 / 157, Treasures 30 / 3,114 - none wrong after the fixes.
- **Six games by the port's buttons** (new `tools/switch_cases/racecap.py`; PoolRad, Curse, Secret, Darkness, Gateway, Treasures - GOG): a saved game prepared with the first member an elf fighter with Strength 18 (limit 7) and plenty of experience, at level 6, 7 and 8; the Cheats' Trainer, Train, cheat off then switched on at Start - Cheats in the same game. All six the same: level 6 trains to 7 with the cheat off; level 7: "Not Enough Experience" off, "will become: a level 8 Fighter" on; level 8 (past): "Not Enough Experience" off, "a level 9 Fighter" on - and in PoolRad and Gateway, whose last fighter level is 8, nothing either way.
- **On, level up, off, save, start anew** (Darkness, the Steam save's Oratisi, by buttons): cheat on - 5/9/12 trained to 9/11/12; cheat off - "Not Enough Experience", still 9/11/12 with 500,000; saved; the port started again with the cheat off - the save loads as 9/11/12, no training; nothing is taken back.

Good to know: the games say "Not Enough Experience" when a race's limit stops the training - their own words, there is no other message. And a single-class character can train only once a visit whatever its experience (the originals cut it to one short of the level after) - so after one training with the cheat on, the next "Not Enough Experience" is that, not the limit.

Not done: the three Krynn games by the port's buttons (my save tool can't set a Krynn character's race; they are covered by the engine check only). Not on a device.

## [0.12.9] - 2026-11-21

Built on the CYD engine 0.211.13 (was 0.211.10).

- **New cheat: Disable Race Level Limits** (Tom: "I want to add a cheat ... which disables the limits on class level because of race"). Start - Cheats, off unless you turn it on; Original Games turns it off and gives your choice back. On: no race stops a class's levels - training goes on as far as the experience reaches, and a new character starts at the levels its experience gives. Each game's own last level of a class stays. **Seen by the port's buttons** (`tools/switch_cases/norace.py`, Darkness, the Steam release's second saved game, the Cheats' Trainer): off - Oratisi (an elf, Strength 18, Intelligence 18, 500,000 experience) trains to fighter 7 / magic-user 11 and no further; on - the fighter goes on to 8 and 9, then stops (10 needs 500,001). Not tried: the other eight games with it on; a new character made with it on.
- **The changing levels on a reroll are the originals'** (Tom: a new Darkness elf fighter / magic-user / thief showed 6/9/12, 5/9/12, 6/10/12 ...). The original in DOSBox, the same character, nine rolls: fighter 5 / 6 / 7 with Strength under 17 / 17 / 18, magic-user 9 / 10 with Intelligence under 17 / 17. Nothing changed - the new cheat is the way round it.
- **New Game Fix: Training Every Class** (Curse, Gateway; on by default; on their Game Fixes pages by itself). Their programs train ONE class of a multi-class a visit and cut the experience, so the level waiting in the other class can be lost. On: every ready class trains and the experience another class needs is kept. Off: as the originals. Checked in the engine (its `behave levels` hall check, all nine games, fix on and off) - **the switch's effect was not played through the port's buttons**.
- **Multi-class training seen by the port's buttons** (open since v0.12.8): Darkness's hall says "will become: a level 6 Fighter and a level 10 Magic-User", then 7 and 11.
- From the CYD session, with the pin: Darkness / Treasures - a fight called with no monsters sets the script's flags as the original (0.211.12).

Looked at and left: in Secret (and every game but PoolRad) B inside a treasure's "Take: Money Items Exit" asks "There is still treasure left ..." - that menu's Exit word leaves the treasure in those originals, and B is that word.

Not checked on a device. Checked here: smoke test; `norace.py` as above. Still open: the fight Speed test by buttons in Secret and Darkness (no fight reached by a test there yet - Steam Secret's opening fight plays by itself); the Treasures play-through (Daggerford next).

## [0.12.8] - 2026-11-20

Built on the CYD engine 0.211.10 (was 0.211.5).

- **Pools of Darkness: Phlan's gates** (Tom: the gate by the Temple of Tempus put the party in the water on the far side of the same map). Phlan is two maps side by side, and the game's script swaps them when a step leaves the map's edge - the engine never told the script, so the party wrapped round. Fixed in the engine (0.211.6, the CYD session's, found the same day in its play-through). Checked here: every opening on the line between the two maps (rows 4, 5, 8, 11, 12), both ways, in the engine's playsim - each comes out at the same row of the other map; and by the port's own buttons (new `tools/edge_test.py`): the walk to Traitor's Gate, a step west = the other map's 15,4, a step back = 0,4. The other edges of the two maps are walls or water; the gates out of the city (Kuto's Gate and the like) are the game's own events, not map edges.
- **Training a multi-class character** (Tom: Oratisi, Darkness's second saved game - an elf fighter / magic-user / thief 5 / 9 / 12 with 500,000 experience came out 7 / 9 / 12 with 125,000). The engine trained ONE class a visit and cut the experience - Curse's routine, wrongly used for the later games. Read from each game's program: Secret, Champions, Death Knights, Darkness and Treasures train **every class that is ready, a level each**; only Curse and Gateway train one. Now so. Darkness and Treasures also keep the experience where the old code cut it. Set against the original Darkness in DOSBox, the same save: "will become: a level 6 Fighter and a level 10 Magic-User", then 7 and 11, 500,000 experience kept, then "not enough" - the engine now does exactly that, with a spell learned each time. (Tom's figures were the fault's own leftovers: the save starts at 5 / 9 / 12 with 500,000.) **A game saved after the fault keeps what it lost** - load the stock save again for Oratisi.
- **The Companion's party view was not drawn again when only a line changed** - the port asked the engine for the icons' change mark alone, so a trained character's magenta name (and levels) stayed until something else redrew the view. Now both marks are asked. This was the port's fault, not the engine's.
- **Treasures: a fight on the overland starts two map squares apart** (was: the foes among the party - "not fixed" in v0.12.7). The cause: where there is no 3D view the originals' sight routine sets the encounter's range to 2 itself, whatever the script gave. Darkness's open-ground fights the same (its routine is the same). Seen in DOSBox before (nearest foe at Range 12) and in the playsim after (the foes off the first screen, nearest at 13).
- **Darkness / Treasures: a weapon's Aim** goes Next / Prev through the fighter and the foes it can attack, nearest first - never its own side (the original, DOSBox; ours stepped onto the ally beside it).
- Also from the CYD session's Darkness play-through, with this pin: a fight's end tells the scripts whether it was won (the Worldstone, the Tyrant's Flame and other places lost their texts after a win), DESTROY ITEMS' magic-only bit, a charmed-only party is lost.

Not checked on a device. Checked here: smoke test; `tools/edge_test.py` (Darkness, GOG, the port's buttons); the engine's navcheck and fightfuzz in Darkness and Treasures (0 problems), its host tests, `behave levels` in Curse, Darkness, Treasures. The training was checked in the engine's playsim against DOSBox, not by the port's buttons. Curse's and Gateway's one-class training loses experience as the originals do - a Game Fix for it is on the list.

## [0.12.7] - 2026-11-20

Built on the CYD engine 0.211.5 (was 0.211.4). The faults the tests of v0.12.6 turned up, fixed.

- **B inside a treasure's "Take: Money Items Exit" left the whole treasure** - and the next B answered "There is still treasure left" with No, so the treasure was gone. B now goes back one menu, to "View Take Pool Share Exit". Seen through the port's buttons in PoolRad after the fix (`hand_test`: "B back: OK"); the same fault had shown in Secret.
- **Treasures: "9 lucky paper of Leilons"** now reads "9 lucky papers of Leilon", as the original (seen in DOSBox).
- The engine's menu check (navcheck) had two false alarms - the light-grey box on an empty save slot, and a spell list scrolling under the box. Both were the check's mistakes, not the game's; with them put right all nine games pass it, the new empty-fight-menu rule included.

**Not fixed - the cause not found yet:** in Treasures a fight on the overland starts with the foes next to the party; in the original (same save, clean reading) they start twelve squares further off. The party's own formation matches the original square for square. The notes for the next try are in the CYD repo's TODO.

## [0.12.6] - 2026-11-20

Built on the CYD engine 0.211.4 (was 0.211.1).

- **A fight's Speed menu** (Tom, Gateway's first fight): after Faster the highlight jumped back to Slower, so the next A undid it - now it stays on the word pressed (Aim's Prev likewise, when Target comes and goes). And leaving Speed left an empty menu line until the D-pad was touched, in the games whose fight menu has Speed itself (Darkness, Gateway, Treasures, the Krynn games) - now the fight menu comes back, Speed boxed.
- **The Companion's party** (Tom): a character who can train has their name in magenta, as the games' own party list, and the class and level that are ready are magenta too (were yellow). The character whose turn it is / who is chosen stays white.
- **Treasures' overland** (found in this session's play-through, each set against the original in DOSBox): a game saved on the overland came back with a 3D view and a noise floor where the map belongs - and the original could not load such a save at all. Saves now keep the map; an old save mends itself as it loads. Fights on the overland had every square a river bank: now grass, rocks, bushes and trees, as the original. The overland's menu no longer has Area (the original's has none).

Checked here, GOG files, 640 x 480, the port's own buttons: `tools/hand_test.py <...> speed` (new: Speed opened, Faster twice, Slower twice, out by Exit and by B) - the build before fails it in Gateway (the highlight; the wrong menu after Exit), this one passes in PoolRad, Curse, Champions, the Dark Queen, Gateway and Treasures; Secret's test fight ended before the Speed menu was tried, Darkness and Death Knights had not finished when this was written (TODO). Treasures: a save made on the overland by v0.12.5 loaded - the map there, no Area - and an overland fight's picture looked at (grass, trees, rocks). The Companion's magenta was seen in the engine's own Companion picture (Treasures, two members given the experience), not yet on a port screenshot.

## [0.12.5] - 2026-11-18

Built on the CYD engine 0.211.1 (was 0.210.9).

- **Fights indoors had the wrong battlefield - in every game but Gateway** (Tom: Treasures' first fight, the floor black, a few wall pieces). An engine fault of this session's own making, since engine 0.203.12 (17 November, the Gateway arena work; in the port since v0.10.10): after the indoor field was made, the outdoor field was made over it. Treasures and Darkness showed black floors, Curse fought its dungeon fights in a wood; Gateway's wilderness fights got no field made at all. Fixed in the engine (0.211.1).
- **Character Info** (the engine's new Enhancement, 0.211.0): while Add Character to Party lists the characters, the screen's other half shows the highlighted one as View Character does. Game Play - Enhancements - Character Info, on by default.

Checked here, GOG files: Treasures' first street fight in Llorkh through the port's buttons at 640 x 480 - black floor before, the street's floor and walls after (screenshots); PoolRad's first fight through the port - its room's floor and wall; in the engine's test program Treasures and Curse before / after (Curse: a wood before, the dungeon room after). Character Info on / off in Curse: the highlighted character's sheet beside the list / the plain list. Darkness: one indoor fight in the engine test, its floor drawn. NOT checked after the fix: Secret, the Krynn games and Gateway (its wilderness fights) - by the same code, not looked at.

## [0.12.4] - 2026-11-18

Built on the CYD engine 0.210.9 (was 0.210.0). Nothing changed in the port's own code - the engine's fixes come with it:

- **Spell memorizing** (the engine's, each game set against the original in DOSBox there): the Krynn games' special spells (the god's, the moons' extras) memorized and counted on their own line; the classes' spells learned at a load; the Ring of Wizardry by each game's code; the Keep questions; the to-memorize list shown first (Curse, Secret, Gateway, Champions); spell lists alike in every game - Up / Down a line at a time with the window scrolling, Next / Prev a page.
- Modify Character as the originals (Krynn knights' 18/xx, the Dark Queen's own bounds); Death Knights' start camp and fight icons; the temple's Raise Dead by each original's routine (the Curse question v0.12.3 found).
- The title music starts with the title's first screen; music no longer drops out during long loads.

Checked here, GOG files, 640 x 480: the smoke test; Keep Memorizing on / off in Curse again (its steps follow the new to-memorize list); tools/hand_test.py `menus` (every word of Cast / View / Encamp - Magic / Rest / Alter by buttons) on all nine games: nothing flagged but the Dark Queen's View / Drop, which that test can't read (known); the Memorize screen was reached in eight games, not in the Dark Queen. The engine's memorizing fixes themselves were not re-proved here game by game - that was the CYD's DOSBox work.

## [0.12.3] - 2026-11-18

No change to what the player sees. The switches were tested in games (Tom: a menu line that flips isn't a tested switch).

- For the tests: the port's `--drive` output now says which switches the ENGINE has on (asked of the engine itself) and lists the open menu's lines. `tools/switch_test.py` (coins, wiring) and `tools/switch_cases/` hold the tests.
- **Seen in a game, on and off, through the buttons (GOG files)**: Coin Exchange (Curse), Game Font, Savegame Info, More Characters, Icon Gallery (PoolRad, Curse, Darkness, Treasures; some in Secret / Champions), Icon Portrait (Curse), Camp Fix (PoolRad), Show Rolls (PoolRad, a fight), Keep Memorizing (Curse), Max Hit Points (Curse, a level-up: +13 twice against +5 / +11 / +9 / +6), Multi-Class Hit Points (Curse: +3 on, +4 off), Raising the Dead (Darkness: Constitution 18 kept on, 17 off), Race Adjustments (Darkness, a dwarf's 80 rolls: Con 16.25 / Cha 14.11 on, 15.12 / 14.91 off).
- **Every line reaches the game**: in a loaded game of each of the nine games every Game Fix, AD&D 1E Rule, Coin Exchange and Flasks of Oil line was flipped by the buttons and the engine asked - each switched its own engine switch and only that; Original Games switched all off and gave the set back; a switch left off was still off after a restart.
- **NOT seen in a game through the port**: Auto Join Items, Flasks of Oil, Quick Actions On / Off; the rules Magic Missile, Thrown Weapons, Magic Items, Age with Time, Power Word Kill, Protection from Evil; every Game Fix but Race Adjustments. For those: the line reaches the engine (above), and what the engine then does is checked by the CYD's own toggle tests - not by this port.
- Found on the way, for the CYD side: in Curse the temple's Raise Dead takes no Constitution with Raising the Dead OFF either (the rule's help says the originals take a point "by spell or at a temple").

## [0.12.2] - 2026-11-18

Built on the CYD engine 0.210.0 (was 0.209.0).

- **Coin Exchange and Flasks of Oil are Enhancements now** (the engine's change): two new lines on Game Play - Enhancements, each its own switch, on by default. Coin Exchange is no longer on the AD&D 1E Rules page (a player who had it off there keeps it off); the Magic Items rule is PoolRad's Flame Tongue alone.
- With the engine: its switch-by-switch checks' fixes (Secret's outdoor fight field, the games each Game Fix lists, Treasures' flask of oil). The Game Fixes pages show the engine's corrected lists by themselves.
- Not brought over: the CYD's journal-book changes (single-column pages, Page Width) - the port has no whole-book view yet.

Checked: the smoke test (every menu page incl. the two new lines; Original Games on / off); tools/play_test.py on Curse (GOG) at 640 x 480 - menus, save, Journal, Quick Actions, the four cheats; no fight met.

## [0.12.1] - 2026-11-17

- **Quit hung the device on Knulli** (Tom's RG40XX V, v0.12.0, from his log: "pm_finish: command not found" after "the port ended with code 0"). The port itself had ended; the launch script then failed to run PortMaster's clean-up, and the hotkey helper it had started was left behind, so the firmware's menu never came back. Why: Knulli's PortMaster gives that clean-up as an alias, and the script - read whole before it runs, so an update can replace it - didn't know aliases defined while it ran. Now the helpers are called by name when they are needed, the hotkey helper is ended by hand afterwards in any case, and a firmware with no clean-up at all gets one. The smoke test now gives the clean-up as an alias and fails if it isn't run. Not tried on a device.
- **Rescan Game Files and Check for Updates are on the main menu** (Tom: "not buried"), in a game and at the game list. System keeps Device Info, Send Logs and Reply Address.
- **Quick Actions is one screen** (Tom: "needlessly complex"): the main menu's line opens it; its top line switches Quick Actions On / Off, the rest is the party and each member's eight settings. Off: the settings are dimmed (Quick as the originals); with no party yet the screen still opens to switch it.

Checked: the smoke test (its install / update run now with an alias clean-up); a screenshot of the main menu and of the Quick Actions screen in Curse at 640 x 480; tools/play_test.py on Curse for the menu's Journal, Quick Actions and cheats.

## [0.12.0] - 2026-11-17

Engine 0.209.0.

- **The menus follow the CYD's new tree.** Start opens: Display, Audio, Game Play, Journal, Cheats, Quick Actions, Controls, System, Exit Game, Quit (at the game list: without Journal, Quick Actions and Exit Game; Cheats there has Max Hit Points only). B goes back a page.
  - **Display**: Picture (Original 4:3 / Square Pixels / Stretch).
  - **Audio**: Game Sound, AdLib Music, Volume.
  - **Game Play**: Original Games, Enhancements, AD&D 1E Rules, Game Fixes.
    - **Original Games** - on: every Enhancement, AD&D 1E Rule and Game Fix off, the game's own font, Max Hit Points off; your own set is kept and comes back when you turn it off. Any switch can still be turned on by itself meanwhile.
    - **Enhancements**: Game Font, Show Rolls, Savegame Info, Auto Join Items, Keep Memorizing (new), Camp Fix, More Characters, Icon Portrait, Icon Gallery.
    - **AD&D 1E Rules**: nine switches (Magic Missile, Multi-Class Hit Points, Thrown Weapons, Magic Items, Coin Exchange, Age with Time, Power Word Kill, Protection from Evil, Raising the Dead).
    - **Game Fixes**: every fix its own switch - Every Game, and a page a game (in a game: Every Game and that game's).
  - **Quick Actions**: On / Off, and Each Member's Actions.
  - **Controls** (the port's own): Button Guide, D-Pad Walks, Confirm Button.
  - **System**: Rescan Game Files, Device Info (new: the port's and the engine's version, the device, each game found and whether it is the Steam or GOG release), Check for Updates, Send Logs, Reply Address.
- **Is it built into the engine?** Partly. What each rule and fix does, and their names and help words, come from the engine - a new one shows up here by itself. The pages, the switches and where they are kept are the port's, and were rewritten for this.
- **Gone, as in the CYD** (always on now): the Readable Messages, Button Stops Demo, Cheats and the single Game Fixes switches, As Original / All On (Original Games replaces them).
- **Rescan Game Files moved** from the game list's first menu into System, where the CYD's tree has it. You had asked for it on the first menu (v0.7.0) - say if you want it back there.
- **Your settings carry over.** An older settings file whose Game Fixes was Off reads as every fix off.
- The words under the list take as many lines as they need (the engine's explanations of a fix can be long); the list scrolls in what is left.
- **Treasures: Llorkh can be finished** (engine 0.208.4, found by the play-through). The fight with the giant skeleton in the keep's dungeon came again at every visit and Geildarr was never met: the original's fights tell the story whether the party ran, and the engine didn't pass that on. Read from the original's program; played in the engine's test program through Geildarr's defeat and journal entries 11 and 10. Not compared in DOSBox.

Checked: the port's smoke test (every page and switch by buttons, Original Games on and off, an old settings file read); screenshots of the pages at 640 x 480 and 1280 x 720 in Curse; tools/play_test.py on Curse (GOG, 640 x 480) for Journal, Quick Actions and the four cheats. Not checked: on a device; the other eight games' menus (the same code, their Game Fixes pages differ).

## [0.11.2] - 2026-11-17

Engine 0.208.1.

- **Treasures: a spell cast where the story waits for one now counts.** In Llorkh's dungeon two women both claim to be Siulajia and the game offers Attack / Leave / Cast. Choosing Cast and then casting Dispel Magic from the Cast menu did nothing - the party could never tell them apart, so Siulajia could not be freed or join. The original passes the spell just cast to the story; the engine didn't. Found in the Treasures play-through, read from the original's program, and played here in the engine's test program: the false one shows herself, her guards attack, the two journal entries are given, Siulajia joins. Not compared in DOSBox. Other places in Treasures that wait for a spell use the same code.

## [0.11.1] - 2026-11-17

Engine 0.208.0.

- **Save Slot Info: the save's map** (Tom: the map area was blank on 16:9 and 1:1). While Save or Load Which Game asks, the Companion's map is now the highlighted save's own - its map, the party's square and the way it faces. "No map here." only where the save has none: out of doors (an overland map), a game's stock save from before the adventure begins, and an empty slot. Looked at in a 1280 x 720 window with a save made in each game: PoolRad, Curse, Secret, Darkness, Knights and Treasures show their map and the party's arrow; Champions (overland) none; Curse also at 720 x 720. Not looked at: Gateway (the test didn't reach its Save), the Dark Queen (it has no Save Slot Info screen).
- **Treasures: a save in Llorkh said "Yartar".** The engine's place name for the game's first town was wrong.

## [0.11.0] - 2026-11-17

Engine 0.207.2.

- **Save Slot Info** (the engine's new option, on by default; Settings - Save Slot Info; As Original turns it off). "Save Which Game" shows the slots that hold a save in white and the empty ones in grey. While Save or Load Which Game asks, the game screen shows the highlighted slot's save: when it was saved (the handheld's clock - "Nov 7 2026 9:41 PM"), where the party is, the game's clock, and each member with icon, class, level and hit points. On a 4:3 panel the party is the right half of the game screen; on a wide or square panel the Companion beside / under the game shows that party and the words get the whole game screen. Looked at with Curse's own save at 640 x 480, 1280 x 720 and 720 x 720: Load Which Game and Save Which Game, a used slot and empty ones. The Companion's map stays empty for a save for now - the save's own map there (Tom's request) is the next version.
- **Treasures: characters brought from Gateway keep their things as in the original.** Their items read as other things ("Banded Mail" as "Dragon Breath Armor") and stayed readied; now the same names as in Gateway, nothing readied, as the original does it (compared in DOSBox: two members' lists).
- **Darkness and Treasures: Strength counted again.** A character loaded from a saved game lost the Strength bonus to hit and damage the first time anything was readied or changed (a fighter with 18/00: 3 worse to hit). Found in the Treasures play-through's first hour; the games' own stock saves now give back their own figures.

## [0.10.10] - 2026-11-17

Engine 0.205.1. Gateway to the Savage Frontier has been played to its end, and its last place and its ending were not as the original's - found by running the original in DOSBox beside the engine and reading its program.

- **Gateway: Ascore's arena is now the maze it was.** In the original these fights don't end when the last enemy falls: "All enemies destroyed. (more) / Continue through the (more) / maze off any map edge. (more)" - and the party walks off an edge of the battlefield; the side it leaves by decides where it goes next (north: the next fight; the last fight's north: the ending). Stepping off the edge asks no "Flee?" there, and the fight is over as the first member is out. The engine ended each fight at the last enemy and always went north. (Trolls get up again in the second fight, so there the way on is the edge.)
- **Gateway: the ending.** After the altar the engine showed six empty pages and dropped to the party menu. Now the original's six picture pages with its words, then "Victory is sweet", Krevish leaves, and the party is in Yartar - the game goes on, as in the original. All six pages were compared with the original's.
- **Gateway: Krevish is drawn in fights** (he was an empty square), and the fight view stops at the battlefield's edge instead of showing floor that isn't there.
- **Auto Join** (new in the engine, Settings): an item taken from a treasure joins the same thing its taker already carries - arrows with arrows - instead of taking a slot. On by default; As Original turns it off.

## [0.10.9] - 2026-11-17

- **Knulli: the screens named the wrong buttons.** Knulli reports A and B the other way round, and the port already fixes that so the button marked A chooses and B goes back. But the words on screen still went by what the firmware reported, so they told you to press B for what the A button does (on the Quick page: "B Flip, A Back" when it is the other way). The names on screen now follow the letters printed on the buttons. Seen on a TrimUI Brick Pro with Knulli.

## [0.10.8] - 2026-11-16

Engine 0.203.11. One more fault from the Gateway play-through, in the last place of the game.

- **Gateway: Ascore's arena took every flight for "north".** The arena sends the party on by the side of the battlefield it leaves: north to the next fight, any other way to Vaalgamon's "Run away, little ones!" and back out. The engine never told the game which way the party had left, so running from any fight - even the last - counted as north, the way to the ending. And the south way out of the first fight was walled up. Checked in DOSBox from the same save: there one fighter stepped off the south edge, the party stood facing south, and Vaalgamon laughed it out of the arena. The engine now does the same.

## [0.10.7] - 2026-11-16

Engine 0.203.10. Three faults found by playing Gateway from a new party (the play-through is at Luskan, the first statuette taken - about a third of the game).

- **Gateway: every Yes / No question took the No branch** unless something earlier happened to fit. At the start of the game the thugs who corner Krevish in Yartar could not be fought ("The little man continues to dodge his attackers" whatever was answered), so the first quest could not start. Fixed in the engine.
- **Gateway: the Kraken cell under Yartar could not be left, and each try killed someone.** The script asks who climbs through the grating and tests that member's Dexterity and thief level; the engine gave it 0 for both, so everyone fell - and the fall, 2 points in the original, did 130 to a random member. Checked against the original in DOSBox from the same save: there the thief gets through, and a fighter who fell lost 2 hit points. The engine now does the same.
- Engine 0.203.8 (the CYD session's): on Quick, Missile no longer swaps the melee weapon for a bow when no foe is in sight.

## [0.10.6] - 2026-11-16

Engine 0.203.7.

- **Pools of Darkness: a new game could not be played.** After Rolf's tour of New Phlan (or after saying No to it) the screen stayed on his last words with a black view and no menu, and no button did anything. Games loaded from a save were fine, which is why it wasn't seen before. Fixed in the engine (the CYD boards had the same fault). Found by a new test that starts every game the way a new player does: six characters made, armour and weapons bought, then out into the first map.
- Engine 0.203.4 - 0.203.6 (the CYD session's): Pool of Radiance - loading a save, the dice's order in fights, clerics on Quick.
- New test tool `tools/newgame_test.py`; `tools/explore_test.py` can keep the party alive with the Cheats.

## [0.10.5] - 2026-11-15

Engine 0.203.3.

- **Knulli: A confirms without touching Settings.** Knulli reports A and B the other way round, so the port now starts there with Confirm Button on B (which is the button marked A on the device). It does this once; whatever you set in Settings - Confirm Button afterwards stays. If you had already switched it by hand on Knulli, nothing changes for you. Other firmwares are untouched.

## [0.10.4] - 2026-11-15

Engine 0.203.3. Nothing in the port itself changed; this is the fourth test round's result and the newest engine.

- Engine 0.203.3 (the CYD session's): Pool of Radiance's wilderness places set against the original - the cave's status line, the first run's frame.
- Fourth test round (creating a character to the end, the icon screen, walking the maps with fights on Quick): no fault found in the port. What it reached and what it did not is in docs/TODO.md.
- Tom's reports: runs on an RG40XX V with Knulli (A and B come out reversed there - Settings - Confirm Button) and on an R36S with dArkOS.
- New test tool `tools/explore_test.py`.

## [0.10.3] - 2026-11-15

Engine 0.203.2. From the third test round: fights played by hand and the camp / spells / items menus walked through, by buttons.

- **A quick D-pad tap in a fight could be lost.** While a fighter moves or aims, the port waits 70 ms after a direction for a second one (two together = a diagonal). A tap let go sooner than that did nothing. Now the step is taken when the button comes up. (Found by reading the code after the test's own taps did nothing; on a device it would have shown as the odd tap being ignored.)
- Engine 0.203.x (the CYD session's): every game's demo runs to its end; the Dark Queen's area spells.
- New test tool `tools/hand_test.py`; the test scripts can press two directions together (`up+right`).

## [0.10.2] - 2026-11-15

Engine 0.202.1. Nothing a player would notice was wrong; this is the result of the second test round.

- One fault found by 2,500 random button presses a game on a checking build: the engine shifted -1 while working out whether the Companion's Battle view had changed (no effect on what was drawn; fixed in the engine, 0.202.1). No crash in any of the nine games, window resizes included.
- Save, quit the port, start it again, load: the party is where it was - all nine GOG games and the ten Steam folders, and all nine again with the handheld (ARM) program under emulation.
- Updating from v0.9.4 with its own Check for Updates to v0.10.1 (the real release files): settings and saves kept, journals made anew once.
- The button test at 1280 x 720 and 720 x 720 on the Steam folders and again on the GOG ones: clean.
- Engine 0.202.0 (the CYD session's): the Dark Queen's area spells' bursts.
- New test tools: `tools/save_test.py`, `tools/monkey_test.py`.

## [0.10.1] - 2026-11-15

Engine 0.201.2. From the button test of every game (below).

- **The Dark Queen of Krynn, Create Character:** the form showed two white boxes at once - a choice and "Done" - and A acted on the choice, so it wasn't clear how to finish. Now one box, as on Modify: Up / Down move over the choices and A picks one; L1 / R1 light "Done" (and "Exit"), and A takes it. (Fixed in the engine, so the CYD boards' cursor keys have it too.)
- **The Dark Queen's knights** read "Monk" on the Companion's Party view - now "Knight" (engine fix).
- Journals are made anew once after this update (the scan's screen at the first start) - v0.10.0 said so but only did it for games found new. With engine 0.201.0's corrected entry 45 of Secret's GOG journal and 0.200.6's Steam Treasures entries.
- Engine 0.201.0 (the CYD session's): caster levels, Death Knights' Heal / Remove Curse, PoolRad's camp Animate Dead, the Dark Queen's "Exit to DOS?" question.
- The button test (`tools/play_test.py`) now reads text over pictures and off the 8-pixel grid, so Treasures and the Dark Queen are played by it like the others.

## [0.10.0] - 2026-11-15

Engine 0.200.9.

- **Square screens (1:1, 720 x 720 - RG CubeXX, RGB30 and the like): the game moves to the top and the Companion fills the room under it** - the map (in a fight: the battlefield) at the left, the party at the right. Y takes it away (the game centred, as before) and brings it back. Checked in a 720 x 720 window only - not on such a device.
- The Companion follows the CYD's new look (engine 0.200.7 / 0.200.8): no "Map Party" heading, the party's names white (the chosen one) and cyan, secret doors a cyan S, the legend just "Locked" and "Secret".
- The title screens' "no Companion" now asks the engine properly (`play::titles()`, new in engine 0.200.9). The demo keeps the Companion.
- Engine 0.200.6: the Steam Treasures journal's entries 17, 20, 21 and 23 are found (the scan reads the journals anew).

## [0.9.5] - 2026-11-14

- **Wide screens (16:9): the title screens are centred, with no Companion beside them.** The Companion (map and party) comes in, and the game moves to the left, when the game reaches its party menu (the one with Begin Adventuring). Y does nothing during the title screens.
- If you choose Demo at a game's Play / Demo line, the Companion is beside the demo while it plays and goes again when the titles come back.

## [0.9.4] - 2026-11-14

Engine 0.200.5 - the rest of the party-menu check against the originals (a character made and added in each, since they ship no usable saved game):

- **Secret of the Silver Blades asks "Pick Character" first** (Select / Exit) for Drop, Modify, View and Remove, as the original does. B leaves it.
- **Secret's character-creation questions start on Yes** - "Reroll stats?", "Is this icon ok?", "Save NAME?" - as the original's. Careful: A on "Reroll stats?" now rerolls; move to No to keep the stats.
- Gateway to the Savage Frontier: checked, already the same as the original.
- Not run: Pool of Radiance and Curse of the Azure Bonds (the originals ask their code wheel question before the menu). Neither has a "Pick Character" prompt in its program.

## [0.9.3] - 2026-11-14

Engine 0.200.4. The party menus (the one with Begin Adventuring) were set beside the original games running in DOSBox, and three things brought into line:

- **B leaves "Pick Character"** in every game, back to the party menu with nobody picked - as Esc does in the originals. (B also still answers No at a Yes / No, and takes a menu's Exit.)
- **Pools of Darkness and Treasures of the Savage Frontier ask "Pick Character" first** for Drop, Modify, View and Remove, as the originals do. Before, only Darkness's Drop asked; the others acted on the highlighted character at once.
- **The Dark Queen of Krynn: Delete Character is the original's** - a list of your saved characters that aren't in the party ("delete a character:", Delete / Exit), then "Delete NAME forever?" and "Are you sure?". With none saved: "No characters to delete." (0.9.2 had it dropping a party member - wrong.)
- Journals: the engine reads two Introductions better (Steam Gateway's was its contents page). This version's first start runs the scan once more to rebuild the journals.
- Not rechecked against the originals this round: Pool of Radiance, Curse, Secret, Gateway.

## [0.9.2] - 2026-11-14

- Pre-release. **Death Knights of Krynn: the fight field is grey again**, with its dotted ground - the open squares were black and only the trees and rocks grey. Checked against the original game (engine 0.200.2).
- **The Dark Queen of Krynn: Delete Character works from the buttons.** At "Pick Character": Up / Down (or L2 / R2) choose the character, A is Select and asks "drop NAME forever" Yes / No, B is No. Before, A did nothing and Up / Down dropped out of it.

## [0.9.1] - 2026-11-14

- Pre-release. **Overland travel works from the D-Pad** in Champions of Krynn, Death Knights of Krynn and Pools of Darkness: choose Move, the D-Pad steps the party across the map, B ends it. Before this the party could not leave the square it stood on - Champions' own saved game starts on that map (fixed in the engine, 0.200.1).
- Engine 0.200.1: also the Dark Queen's ending pictures, and the classes a character can train in shown in yellow (0.200.0).
- **Every game was played by a scripted button test** before this release (see the README's Status): no taps, the screen read back after every press.
- Known: in The Dark Queen of Krynn, Delete Character's "Pick Character" can't be answered with A (L2 / R2 leave it; Pointer Mode works). "Pick Character" in the Krynn games has no B = back: pick one, then Exit. The Trainer cheat is left with Begin Adventuring, not B.

## [0.9.0] - 2026-11-14

- Pre-release. **Music for The Dark Queen of Krynn and Champions of Krynn** (engine 0.199.0): made from the game's own files by the scan, like the other games' music. This version's first start runs the scan once more to make it.

## [0.8.1] - 2026-11-14

- Pre-release. Send Logs no longer sends the scan's marker files (one per game, no use as logs - they were crowding out real ones).
- After sending, the message says to look in your spam folder for the note.

## [0.8.0] - 2026-11-14

- Pre-release. **Send Logs now sends the logs to the developer over the internet** - no server to set up. With Wi-Fi on, Start - Send Logs uploads them and they arrive by email. The Log Server setting and the WebDAV upload are gone.
- **Reply address.** The first time, Send Logs asks for your email address (optional - X is No Thanks). It goes with the logs so the developer can write back, and you get one note confirming the logs arrived and asking what you were doing. Change or clear it at Settings - Reply Address.
- **Every log starts with what it ran on**: the port's version, the device's make and model, the firmware's name and version, PortMaster's version, the screen size.
- Only text logs and the port's settings are sent - never game files, saves or journal text.

## [0.7.0] - 2026-11-14

- Pre-release. **Journals read properly from every game, both releases.** The scan missed things in several Steam journals: Treasures' Introduction, all of Champions' 57 tavern tales, and the whole text of the three Krynn journals (their PDFs were refused). Fixed in the engine (0.198.1) and checked game by game. This version's first start runs the scan once more to rebuild them (and the music, now 16-bit).
- **Journal viewer, X button.** Text entries have no zoom any more: X is the next page, and goes round to the top after the last. Picture entries (maps, GOG's scanned entries): X steps Width (the start - as wide as the screen), Double, Whole (all of it on screen), and for very long maps Turned (on its side, its length across the screen).
- **Game list pictures show at every start** without a rescan: they are now checked only once the screen has reported its real size (the RGB10 MAX3 problem).
- **Rescan Game Files** is on the game list's Start menu itself (was Look for Games Again, at the bottom of Settings).
- The game list shows the version with a "v" (v0.7.0).
- Known: the Dark Queen's Introduction comes out as its heading only; GOG's Dark Queen journal lacks entries 13 and 26 (the Steam release has them).

## [0.6.2] - 2026-11-14

- Pre-release. **The game list and the Looking for Games screen fill the whole screen** on 16:9 and other wide handhelds (box art across the full width) instead of sitting in a 4:3 box. This version's first start remakes the list pictures once for the screen's width.
- **Looking for Games uses smaller letters**, so its lines fit.
- The "2 of 9" counter is gone from the game list.
- Game Fixes' description: "Corrects game mistakes and uses AD&D 1E rules. Off: as original game."

## [0.6.1] - 2026-11-14

- Pre-release. **Everything is prepared when a game is found, nothing afterwards.** The scan now also makes each game's list picture (box art or logo) and its program tables, alongside the journal and music. After that the game list only shows saved pictures and a game starts straight away - no "Loading" screen, nothing decoded on first use.
- Because this version adds those steps, its first start runs the picture step once for games already found. Changing Settings - Display remakes the pictures for the new shape when the menu closes.

## [0.6.0] - 2026-11-14

- Pre-release. **The Companion comes in from the right** (Y), so the 3D view and a fight's field stay in sight.
- **Wide screens** (16:9 and the like): the Companion stands beside the game all the time - the map (in a fight, the battlefield) above the party. Y hides it and brings it back.
- **Journals and music are made when a game is found**, not when it starts: the first run that sees a game folder lists what it does ("Found ...", the journal's pages, the music) and waits for A. **Settings - Look for Games Again** makes everything anew.
- **GOG games show their logo** on the game list (from the game folder's own `goggame-*.ico` / `.dll`), when there is no Steam `Launcher` folder with box art.
- **The Start menu is tidied**: Settings is a page of its own, and every line says what it does at the foot of the page. L2 / R2 jump a page.
- **Settings has the engine's extras**, each On / Off: Game Fixes, Show Rolls, Readable Messages, Camp Fix, Icon Gallery, Icon Portrait, More Characters, Button Stops Demo, Quick Actions, Cheats; Max Hit Points; **As Original** turns them all off.
- **Cheats** (Start - Cheats, while exploring): Healer, Merchant, Sanctuary, Trainer, Max Hit Points.
- **Quick Actions** (Start - Quick Actions): for each member of the party, what Quick may do in a fight - Melee, Missile, Spells, Items, Bandage First, Move, Turn Undead, Guard. Up / Down a switch, L1 / R1 a member, A flips it.

## [0.5.0] - 2026-11-14

- Pre-release. **The Adventurer's Journal, in the game.** When a game tells you to read a journal entry, tavern tale or proclamation, the next press of A opens it. Made on the handheld from your own journal PDF the first time the game starts - GOG's scanned journal (the PDF in the game's folder) as pictures of the entry, the Steam release's (its `Documentation` folder and Gold Box Companion files) as text in the game's own look.
  - Up / Down scroll, L2 / R2 (or A) a page, X the size / zoom, Left / Right across a zoomed picture, B back.
  - Start - Journal lists the entries met so far, to read again.

## [0.4.0] - 2026-11-14

- Pre-release. **Game list with box art**: a game a page, its picture from the Steam release's `Launcher` folder (copy that folder into `goldbox/games/`), Left / Right to turn the pages.
- **Game Sound** now offers the Darkness, Gateway and Treasures sound sets for the older games when those games are in `games/`.
- **The Companion** on Y: slides in from the left - the auto-map, the party at a glance, a fight's battlefield. Y again for the next view, then away. The game goes on under it.
- **Music**: the later games' own music (Pools of Darkness, Death Knights of Krynn, Gateway, Treasures) - made from the game's files the first time that game starts (a few minutes, once), then played where the game plays it. Start - Music turns it off. (Engine 0.197.1.)
- **L2 / R2** move the list's line (the party list while walking); L1 / R1 still move the menu's word. The side-step and Look buttons are gone.
- Modify Character shows one highlight: the item being changed, or the menu's word once you're on the menu .
- The installer says where the games go and makes sure the folder is there.

## [0.3.1] - 2026-11-14

- Pre-release. The installer no longer leaves a `goldbox_install.log` behind in the ports folder (it is `goldbox/install.log`).

## [0.3.0] - 2026-11-14

- Pre-release. Installing and updating as Pocket Curator does it: a one-file installer (`GoldBox.Installer.sh`) that downloads and installs the newest release, and **Check for Updates** in the Start menu - the port finds a newer release, downloads and checks it, and installs it on restart. Games, saves and settings are kept.
- **Send Logs** in the Start menu: puts the port's logs on a WebDAV server (set its address once with **Log Server**).
- The log now starts with the port's version, the firmware and the device; the run before's log is kept as `log.prev.txt`.

## [0.2.0] - 2026-11-14

- B is Back: out of a screen or question, or the menu's Exit. Built on engine 0.196.52.
- First check with real game files (Curse of the Azure Bonds).

## [0.1.0] - 2026-11-14

- First version: the CYD Gold Box Game Engine with an SDL2 front end for handheld buttons - game list, play, on-screen keyboard, Pointer Mode, Start menu; the PortMaster package and its builds.
