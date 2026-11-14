## Notes

Plays SSI's Gold Box AD&D role-playing games from **your own copies** of the original DOS game files. The port is an open-source engine written from scratch (the [CYD Gold Box Game Engine](https://github.com/tomtombombadil/CYD-GoldBoxGameEngine), here with a front end for handhelds); it contains nothing from the games.

Copy each game's whole install folder (the GOG or Steam release) into `ports/goldbox/games/`, one folder per game, keeping the game's name in the folder's name - for example `ports/goldbox/games/Curse of the Azure Bonds/`.

Thanks to SSI for the games, and to tomtombombadil for the engine and the packaging for PortMaster.

## Controls

Everything is done with the buttons; sticks are optional extras.

| Button | Action |
|--|--|
| D-Pad | Walk (forward, turn, turn around) / move the highlight in menus and lists |
| A | Choose the highlighted thing / continue |
| B | Back: out of a screen or question, or the menu's Exit / delete a letter |
| L1 / R1 | Menu word left / right while walking |
| L2 / R2 | Side-step left / right |
| X | Area map |
| Y | Look |
| Select | Pointer Mode: the D-Pad moves a pointer, A taps there, B ends it |
| Start | The port's menu (display, sound, font, controls, updates, logs, exit game) |
| Select + Start | Quit |

In a fight, while a fighter moves or aims: the D-Pad steps (two directions together = a diagonal), L1 / R1 / L2 / R2 are the four diagonals, A ends the move or picks the target.

When a game asks for a name or a number an on-screen keyboard appears: D-Pad to a key, A types it, B deletes, X is a space, Y is Done.

Confirm and Back can be swapped in the Start menu.

## Compile

```shell
git clone --recursive https://github.com/tomtombombadil/PM-GoldBoxGameEngine.git
cd PM-GoldBoxGameEngine
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Needs a C++17 compiler, CMake and SDL2's development files.
