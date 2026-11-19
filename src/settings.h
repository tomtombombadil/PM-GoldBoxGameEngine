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
    bool    ab_set = false;     // the firmware's default was applied, or the player chose (never applied again)
    bool    music = true;       // the games' own music (the later games)
    bool    pad_moves = true;   // exploring: the D-pad walks (off: it always moves the highlight)
    uint16_t comforts_off = 0;  // the engine's Enhancements switched off (kOff* bits; 0: all on)
    bool    max_hp = false;     // the Max Hit Points cheat
    std::string reply_email;    // Send Logs: the player's reply address ("" none) - goes with the logs
    bool    reply_asked = false; // Send Logs asked for it once (the player may have said No Thanks)
    // ---- the menus by Tom's tree (v0.12.0, engine 0.209.0)
    uint16_t rules_off = 0;     // the AD&D 1E Rules switched off (rules::Adnd bits; 0: all on)
    uint64_t fixes_off = 0;     // the Game Fixes switched off one by one (bit = rules::Fix; 0: all on)
    bool    original = false;   // Original Games on: everything above off, the player's own set kept below
    uint16_t saved_comforts_off = 0, saved_rules_off = 0;
    uint64_t saved_fixes_off = 0;
    bool    saved_max_hp = false;
    uint8_t saved_font = 0;
};

// comforts_off's bits - play::Options' switches. Not switches any more (Tom, 2026-11-17, the CYD's
// menus: always on): kOffDemoTap, kOffReadTime, kOffCheats; kOffFixes only in files from before
// v0.12.0 (read as every Game Fix off)
enum : uint16_t {
    kOffGallery = 1, kOffDemoTap = 2, kOffMoreGuys = 4, kOffPortrait = 8, kOffFixes = 16, kOffRolls = 32,
    kOffCampFix = 64, kOffCheats = 128, kOffQuick = 256, kOffReadTime = 512, kOffAutoJoin = 1024, kOffSaveInfo = 2048,
    kOffKeepMem = 4096, kOffCoins = 8192, kOffOil = 16384, kOffAddInfo = 32768,
    kOffSwitches = kOffGallery | kOffMoreGuys | kOffPortrait | kOffRolls | kOffCampFix | kOffQuick | kOffAutoJoin |
                   kOffSaveInfo | kOffKeepMem | kOffCoins | kOffOil | kOffAddInfo
};

bool settings_load(const std::string& dir, Settings& s);
bool settings_save(const std::string& dir, const Settings& s);
