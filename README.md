# Gold Box Game Engine for PortMaster

Plays SSI's Gold Box AD&D role-playing games on Linux retro gaming handhelds
through [PortMaster](https://portmaster.games) - from **your own copies** of
the original DOS game files ("bring your own game").

This is the handheld version of the
[CYD Gold Box Game Engine](https://github.com/tomtombombadil/CYD-GoldBoxGameEngine):
the same engine (it is built in from that repository, unchanged) with a new
front end made for a handheld's buttons and screen.

**Status: early development.** The port builds, starts, finds the games and
runs them through the engine; it has not been play-tested on a handheld yet.
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

1. Install PortMaster on your handheld if it isn't there already.
2. Get `goldbox.zip` (this repository's Actions builds / Releases) and
   install it as any PortMaster port: the zip's contents go in your
   `ports` folder, giving `ports/Gold Box Game Engine.sh` and
   `ports/goldbox/`.
3. Copy each game's whole install folder into `ports/goldbox/games/` - one
   folder a game, keeping the game's name in the folder's name:

       ports/goldbox/games/Pool of Radiance/
       ports/goldbox/games/Curse of the Azure Bonds/

4. Start **Gold Box Game Engine** from your Ports menu.

Saved games go in each game's own folder, as the originals wrote them.

## Controls

Everything works with the buttons alone - D-Pad, A, B, X, Y, L1, L2, R1, R2,
Start, Select. Sticks and touch screens are optional extras.

| Button | Action |
|--|--|
| D-Pad | Walk (forward, turn left / right, turn around) / move the highlight in menus and lists |
| A | Choose the highlighted thing / continue |
| B | Back / delete a letter |
| L1 / R1 | Menu word left / right while walking |
| L2 / R2 | Side-step left / right |
| X | Area map |
| Y | Look |
| Select | Pointer Mode: the D-Pad moves a pointer, A taps there, B ends it |
| Start | The port's menu: display, sound, font, controls, exit game |
| Select + Start | Quit (PortMaster's usual) |

In a fight, while a fighter moves or aims: the D-Pad steps (two directions
together = a diagonal), L1 / R1 / L2 / R2 are the four diagonals, A ends the
move or picks the target. When a game asks for a name or a number an
on-screen keyboard appears (D-Pad, A types, B deletes, X space, Y done).

## Building

The builds are made by GitHub Actions on every push (aarch64 and x86_64, in
an Ubuntu 20.04 container so they run on older firmwares): the
`goldbox-portmaster` artifact of the latest *Build* run is the installable
zip. To build it yourself on Linux:

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
