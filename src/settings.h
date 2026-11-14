// The port's own settings (conf/settings.cfg: "name=value" lines)
#pragma once

#include <cstdint>
#include <string>

struct Settings {
    uint8_t fit = 0;            // video::Fit
    uint8_t sound = 1;          // 1 Tandy, 2 PC Speaker, 3 Off (the engine's numbers)
    uint8_t volume = 70;        // percent
    uint8_t font = 0;           // the game's text: 0 its own, 1 Clear, 2 Darkness's, 3 Clear Caps
    bool    swap_ab = false;    // Confirm / Back the other way round
    bool    pad_moves = true;   // exploring: the D-pad walks (off: it always moves the highlight)
    std::string log_url;        // Send Logs' WebDAV server ("" none)
};

bool settings_load(const std::string& dir, Settings& s);
bool settings_save(const std::string& dir, const Settings& s);
