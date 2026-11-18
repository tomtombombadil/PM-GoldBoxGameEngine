#include "settings.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

namespace {
std::string file_of(const std::string& dir) { return dir + "/settings.cfg"; }
} // namespace

bool settings_load(const std::string& dir, Settings& s)
{
    FILE* f = fopen(file_of(dir).c_str(), "r");
    if (!f) return false;
    char line[256];
    int menus = 1;              // (a file from before v0.12.0 has no "menus" line)
    while (fgets(line, sizeof line, f)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        if (!strcmp(line, "reply_email")) {
            s.reply_email = eq + 1;
            while (!s.reply_email.empty() && (s.reply_email.back() == '\n' || s.reply_email.back() == '\r' || s.reply_email.back() == ' '))
                s.reply_email.pop_back();
            continue;
        }
        if (!strcmp(line, "fixes_off")) { s.fixes_off = strtoull(eq + 1, nullptr, 10); continue; }
        if (!strcmp(line, "saved_fixes_off")) { s.saved_fixes_off = strtoull(eq + 1, nullptr, 10); continue; }
        const int v = atoi(eq + 1);
        if (!strcmp(line, "fit")) s.fit = static_cast<uint8_t>(v < 0 || v > 2 ? 0 : v);
        else if (!strcmp(line, "sound")) s.sound = static_cast<uint8_t>(v < 1 || v > 6 ? 1 : v);
        else if (!strcmp(line, "volume")) s.volume = static_cast<uint8_t>(v < 0 ? 0 : v > 100 ? 100 : v);
        else if (!strcmp(line, "font")) s.font = static_cast<uint8_t>(v < 0 || v > 3 ? 0 : v);
        else if (!strcmp(line, "swap_ab")) s.swap_ab = v != 0;
        else if (!strcmp(line, "ab_set")) s.ab_set = v != 0;
        else if (!strcmp(line, "pad_moves")) s.pad_moves = v != 0;
        else if (!strcmp(line, "music")) s.music = v != 0;
        else if (!strcmp(line, "comforts_off")) s.comforts_off = static_cast<uint16_t>(v);
        else if (!strcmp(line, "menus")) menus = v;
        else if (!strcmp(line, "rules_off")) s.rules_off = static_cast<uint16_t>(v);
        else if (!strcmp(line, "original")) s.original = v != 0;
        else if (!strcmp(line, "saved_comforts_off")) s.saved_comforts_off = static_cast<uint16_t>(v);
        else if (!strcmp(line, "saved_rules_off")) s.saved_rules_off = static_cast<uint16_t>(v);
        else if (!strcmp(line, "saved_max_hp")) s.saved_max_hp = v != 0;
        else if (!strcmp(line, "saved_font")) s.saved_font = static_cast<uint8_t>(v < 0 || v > 3 ? 0 : v);
        else if (!strcmp(line, "max_hp")) s.max_hp = v != 0;
        else if (!strcmp(line, "reply_asked")) s.reply_asked = v != 0;
    }
    fclose(f);
    // A file from before the menus by Tom's tree: its one Game Fixes switch off = every fix off (as
    // the CYD reads its older file); the switches that are gone read as on
    if (menus < 2 && (s.comforts_off & kOffFixes)) s.fixes_off = ~0ull;
    s.comforts_off &= kOffSwitches;
    s.saved_comforts_off &= kOffSwitches;
    // Coin Exchange was an AD&D 1E Rule (bit 0x20) until engine 0.210.0: an Enhancement since
    if (s.rules_off & 0x20) { s.comforts_off |= kOffCoins; s.rules_off &= ~0x20; }
    if (s.saved_rules_off & 0x20) { s.saved_comforts_off |= kOffCoins; s.saved_rules_off &= ~0x20; }
    return true;
}

bool settings_save(const std::string& dir, const Settings& s)
{
    mkdir(dir.c_str(), 0755);
    FILE* f = fopen(file_of(dir).c_str(), "w");
    if (!f) return false;
    fprintf(f, "fit=%d\nsound=%d\nvolume=%d\nfont=%d\nswap_ab=%d\npad_moves=%d\n", s.fit, s.sound, s.volume, s.font,
            s.swap_ab ? 1 : 0, s.pad_moves ? 1 : 0);
    fprintf(f, "music=%d\ncomforts_off=%d\nmax_hp=%d\nreply_asked=%d\nab_set=%d\nreply_email=%s\n", s.music ? 1 : 0, s.comforts_off,
            s.max_hp ? 1 : 0, s.reply_asked ? 1 : 0, s.ab_set ? 1 : 0, s.reply_email.c_str());
    fprintf(f, "menus=2\nrules_off=%d\nfixes_off=%llu\noriginal=%d\nsaved_comforts_off=%d\nsaved_rules_off=%d\n"
               "saved_fixes_off=%llu\nsaved_max_hp=%d\nsaved_font=%d\n",
            s.rules_off, static_cast<unsigned long long>(s.fixes_off), s.original ? 1 : 0, s.saved_comforts_off, s.saved_rules_off,
            static_cast<unsigned long long>(s.saved_fixes_off), s.saved_max_hp ? 1 : 0, s.saved_font);
    return fclose(f) == 0;
}
