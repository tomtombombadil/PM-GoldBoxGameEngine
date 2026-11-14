# Changelog

Every version of Gold Box Game Engine for PortMaster. A release's notes are
its part of this file.

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
