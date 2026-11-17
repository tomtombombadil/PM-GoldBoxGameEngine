# Changelog

Every version of Gold Box Game Engine for PortMaster. A release's notes are
its part of this file.

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
