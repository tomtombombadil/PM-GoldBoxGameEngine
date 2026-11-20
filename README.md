# Gold Box Game Engine for PortMaster

Plays SSI's Gold Box AD&D role-playing games on Linux retro gaming handhelds
through [PortMaster](https://portmaster.games) - from **your own copies** of
the original DOS game files ("bring your own game").

This is the handheld version of the
[CYD Gold Box Game Engine](https://github.com/tomtombombadil/CYD-GoldBoxGameEngine):
the same engine (it is built in from that repository, unchanged) with a new
front end made for a handheld's buttons and screen.

**Status: pre-release, playable.** All nine games start and play from the
buttons alone. What that rests on:

- **Tested Handhelds:** Anbernic RG40XX H and V, Powkiddy RGB10 MAX3 and X55,
  R36S, R36H, & TrimUI Brick Pro
- **Tested Firmwares:** Rocknix, Knulli, & dArkOS
- **Tested Game Versions:** Windows versions of the Steam & GOG releases. In theory the original game files from the original floppy disks would work as well, so long as they were placed in the correct folders inside /roms/ports/goldbox/games. This has not been tried yet because I no longer have the original floppies.

Known rough edges are in [docs/TODO.md](docs/TODO.md); each version's
changes in [CHANGELOG.md](CHANGELOG.md).

## The games
- Pool of Radiance
- Curse of the Azure Bonds
- Secret of the Silver Blades
- Pools of Darkness
- Gateway to the Savage Frontier
- Treasures of the Savage Frontier
- Champions of Krynn
- Death Knights of Krynn
- The Dark Queen of Krynn

## B.Y.O.G! (Bring Your Own Games) Nothing from the original games is in this repo

This repository and the port's package contain no game files, pictures, text
or code from the games - and never will. You need your own copy of each game
(the GOG or Steam releases); the port reads them on your device.

## Installing

You need PortMaster on the handheld, and Wi-Fi.

1. From the newest entry on this repository's
   [Releases page](https://github.com/tomtombombadil/PM-GoldBoxGameEngine/releases)
   download **`GoldBox.Installer.sh`** (one small file).
2. Copy it into the handheld's **`roms/ports`** folder.
3. On the handheld: Start - Game Settings - Update Gamelists, then open
   **Ports** and run **GoldBox.Installer**. It downloads and installs the
   newest release.
4. Copy each game's whole install folder into `roms/ports/goldbox/games/` -
   one folder a game, keeping the game's name in the folder's name:

       roms/ports/goldbox/games/Pool of Radiance/
       roms/ports/goldbox/games/Curse of the Azure Bonds/
       roms/ports/goldbox/games/Secret of the Silver Blades/
       roms/ports/goldbox/games/Pools of Darkness/
       roms/ports/goldbox/games/Gateway to the Savage Frontier/
       roms/ports/goldbox/games/Treasures of the Savage Frontier/
       roms/ports/goldbox/games/Champions of Krynn/
       roms/ports/goldbox/games/Death Knights of Krynn/
       roms/ports/goldbox/games/The Dark Queen of Krynn/
   

   Copy the **whole** folder: the journal PDF in it (GOG) or its
   `Documentation` and `GBC` folders (Steam) are what the in-game
   Adventurer's Journal is made from.

   With the Steam release, copy its `Launcher` folder there too
   (`games/Launcher/`): the game list then shows each game's box art, and
   the Pools of Darkness, Gateway and Treasures folders give the older
   games their recorded sound effects (Start - Settings - Game Sound).
   Without it (GOG) the list shows each game's own logo from its folder.

6. Start **Gold Box Game Engine** from Ports. The first time it sees a
   game it makes that game's journal and music from your files - it lists
   what it does and can take a few minutes, once. (Start -
   Rescan Game Files makes them anew.)

That is the only time a file has to be carried over for the port itself.
From then on: **Start - Check for Updates** in the port finds a newer
release, downloads it and installs it when the port restarts. Games, saves
and settings are kept. While the port is in development the releases are
pre-releases; the installer and the updater take the newest one.

Saved games go in each game's own folder, as the originals wrote them.

**Copying files to the handheld from Windows.** Many firmwares (ROCKNIX
among them) format their card as ext4, which Windows can't read. With the
handheld on your Wi-Fi, type `\\` and the handheld's IP address into File
Explorer's address bar (on ROCKNIX: Start - Network Settings shows the
address; the `roms` folder is the `games-roms` share, so step 2's folder is
`games-roms\ports`).

**Sending logs.** When something goes wrong, Start - Send Logs sends the
port's logs to the developer over the internet (the handheld needs Wi-Fi).
The first time it asks for your email address so the developer can write
back about the problem - optional; X says No Thanks. Only text logs and
the port's settings are sent (what the port did, your device, firmware and
PortMaster versions) - never your game files or saves.

## Controls

Everything works with the buttons alone - D-Pad, A, B, X, Y, L1, L2, R1, R2,
Start, Select. Sticks and touch screens are optional extras.

| Button | Action |
|--|--|
| D-Pad | Walk (forward, turn left / right, turn around) / move the highlight in menus and lists |
| A | Choose the highlighted thing / continue |
| B | Back: out of a screen or question, or the menu's Exit / delete a letter |
| L1 / R1 | Menu word left / right while walking |
| L2 / R2 | List line up / down (the party list while walking) |
| X | Area map |
| Y | The Companion, from the right: auto-map, party, a fight's battlefield (press again for the next, then away; on a wide screen it stands beside the game - Y hides it) |
| Select | Pointer Mode: the D-Pad moves a pointer, A taps there, B ends it |
| Start | The port's menu: display, sound, font, controls, updates, logs, exit game |
| Select + Start | Quit (PortMaster's usual) |

On an overland map with a marker (Champions of Krynn, Death Knights of
Krynn, Pools of Darkness): choose **Move**, then the D-Pad steps; B ends it.

In a fight, while a fighter moves or aims: the D-Pad steps (two directions
together = a diagonal), L1 / R1 / L2 / R2 are the four diagonals, A ends the
move or picks the target. When a game asks for a name or a number an
on-screen keyboard appears (D-Pad, A types, B deletes, X space, Y done).

## Building

The builds are made by GitHub Actions on every push (aarch64 and x86_64, in
an Ubuntu 20.04 container so they run on older firmwares); a release is the
same build published with its installer. To build it yourself on Linux:

    git clone --recursive https://github.com/tomtombombadil/PM-GoldBoxGameEngine.git
    cd PM-GoldBoxGameEngine
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

It needs a C++17 compiler, CMake and SDL2's development files. The result
also runs on a Linux PC: `build/goldbox --window 960x720 --games <folder>`.

## How it is put together

| Where | What |
|--|--|
| `cyd/` | The engine: the CYD repository as a git submodule, pinned to a commit. Never edited here. |
| `src/` | This port: the platform layer on SDL2 (`platform_sdl.cpp`, `audio_sdl.cpp`) and the front end (`main.cpp`, `video.cpp`, `library.cpp`). |
| `port/` | The PortMaster package: launch script, `port.json`, the `games/` folder's readme. |
| `tools/` | The CI's build, package and check scripts. |
| `docs/` | [SPEC.md](docs/SPEC.md) (design), [TODO.md](docs/TODO.md) (open items). |

MIT License. Third-party notices: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
