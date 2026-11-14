# Gold Box Game Engine for PortMaster

Plays SSI's Gold Box AD&D role-playing games on Linux retro gaming handhelds
through [PortMaster](https://portmaster.games) - from **your own copies** of
the original DOS game files ("bring your own game").

This is the handheld version of the
[CYD Gold Box Game Engine](https://github.com/tomtombombadil/CYD-GoldBoxGameEngine):
the same engine (it is built in from that repository, unchanged) with a new
front end made for a handheld's buttons and screen.

**Status: early development.** The port builds, finds the games and plays
them (checked on a PC with scripted button presses); it has not been
play-tested on a handheld yet.
See [docs/TODO.md](docs/TODO.md).

## The games

Pool of Radiance, Curse of the Azure Bonds, Secret of the Silver Blades,
Pools of Darkness, Gateway to the Savage Frontier, Treasures of the Savage
Frontier, Champions of Krynn, Death Knights of Krynn, The Dark Queen of Krynn -
whichever the engine plays (its
[game list](https://tomtombombadil.github.io/CYD-GoldBoxGameEngine/) has
each one's status).

## Nothing from the games is in here

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

5. Start **Gold Box Game Engine** from Ports.

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

**Sending logs.** Start - Log Server: the address of a WebDAV server on
your network (for example rclone on a PC:
`rclone serve webdav "C:\GoldBoxLogs" --addr :5006`, address
`http://<the PC's IP>:5006`). Then Start - Send Logs puts the port's logs
there, in a `goldbox-logs` folder.

## Controls

Everything works with the buttons alone - D-Pad, A, B, X, Y, L1, L2, R1, R2,
Start, Select. Sticks and touch screens are optional extras.

| Button | Action |
|--|--|
| D-Pad | Walk (forward, turn left / right, turn around) / move the highlight in menus and lists |
| A | Choose the highlighted thing / continue |
| B | Back: out of a screen or question, or the menu's Exit / delete a letter |
| L1 / R1 | Menu word left / right while walking |
| L2 / R2 | Side-step left / right |
| X | Area map |
| Y | Look |
| Select | Pointer Mode: the D-Pad moves a pointer, A taps there, B ends it |
| Start | The port's menu: display, sound, font, controls, updates, logs, exit game |
| Select + Start | Quit (PortMaster's usual) |

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
