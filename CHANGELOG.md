# Changelog

Every version of Gold Box Game Engine for PortMaster. A release's notes are
its part of this file.

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
