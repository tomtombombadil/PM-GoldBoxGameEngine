// The port's own settings (conf/settings.cfg: "name=value" lines)
#pragma once

#include <cstdint>
#include <string>

struct Settings {
    uint8_t fit = 0;            // video::Fit
    uint8_t sound = 1;          // 1 Tandy, 2 PC Speaker, 3 Off, 4 Darkness, 5 Gateway, 6 Treasures (their recordings)
    uint8_t volume = 70;        // percent
    uint8_t font = 0;           // the game's text: 0 its own, 1 Clear, 2 Darkness's, 3 Clear Caps
    bool    swap_ab = false;    // Confirm / Back the other way round
    bool    music = true;       // the games' own music (the later games)
    bool    pad_moves = true;   // exploring: the D-pad walks (off: it always moves the highlight)
    uint16_t comforts_off = 0;  // the engine's comforts switched off (kOff* bits; 0: all on)
    bool    max_hp = false;     // the Max Hit Points cheat
    std::string reply_email;    // Send Logs: the player's reply address ("" none) - goes with the logs
    bool    reply_asked = false; // Send Logs asked for it once (the player may have said No Thanks)
};

// comforts_off's bits - play::Options' switches
enum : uint16_t {
    kOffGallery = 1, kOffDemoTap = 2, kOffMoreGuys = 4, kOffPortrait = 8, kOffFixes = 16, kOffRolls = 32,
    kOffCampFix = 64, kOffCheats = 128, kOffQuick = 256, kOffReadTime = 512, kOffAll = 1023
};

bool settings_load(const std::string& dir, Settings& s);
bool settings_save(const std::string& dir, const Settings& s);
