# Third-party notices

## The engine (the `cyd/` submodule)
- **CYD Gold Box Game Engine**, (c) 2026 tomtombombadil, MIT License -
  https://github.com/tomtombombadil/CYD-GoldBoxGameEngine. The games' logic
  (`cyd/src/engine`, `cyd/src/game`) is built into this port unchanged. Its
  own notices: `cyd/THIRD_PARTY_NOTICES.md`. Of the third-party code it
  includes, this port's binary contains:
  - **TJpgDec R0.03** (Tiny JPEG Decompressor), (C) ChaN 2021,
    http://elm-chan.org/fsw/tjpgd/ - "No restriction on use. You can use,
    modify and redistribute it for personal, non-profit or commercial
    products UNDER YOUR RESPONSIBILITY. Redistributions of source code must
    retain the above copyright notice."
  - **QR Code generator library (C)**, Copyright (c) Project Nayuki, MIT
    License, https://www.nayuki.io/page/qr-code-generator-library.

## Libraries (linked, not copied)
- **SDL2** - zlib license, https://www.libsdl.org. Linked dynamically: the
  handheld's own SDL2 is used.
- **libstdc++ / libgcc** (GCC Runtime Library Exception) - linked statically.

## Used on the handheld (not part of this repo or its package)
- **PortMaster** and its **gptokeyb** - the launch script uses them as every
  port does (https://portmaster.games).

## Used for reference only (nothing copied)
- PortMaster's packaging guide and example launch script
  (https://portmaster.games/packaging.html) - the launch script follows
  its required steps.

## Game data
None. The player supplies their own original game files in the port's
`games/` folder.
