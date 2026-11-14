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
        const int v = atoi(eq + 1);
        if (!strcmp(line, "fit")) s.fit = static_cast<uint8_t>(v < 0 || v > 2 ? 0 : v);
        else if (!strcmp(line, "sound")) s.sound = static_cast<uint8_t>(v < 1 || v > 6 ? 1 : v);
        else if (!strcmp(line, "volume")) s.volume = static_cast<uint8_t>(v < 0 ? 0 : v > 100 ? 100 : v);
        else if (!strcmp(line, "font")) s.font = static_cast<uint8_t>(v < 0 || v > 3 ? 0 : v);
        else if (!strcmp(line, "swap_ab")) s.swap_ab = v != 0;
        else if (!strcmp(line, "pad_moves")) s.pad_moves = v != 0;
        else if (!strcmp(line, "music")) s.music = v != 0;
        else if (!strcmp(line, "comforts_off")) s.comforts_off = static_cast<uint16_t>(v & kOffAll);
        else if (!strcmp(line, "max_hp")) s.max_hp = v != 0;
        else if (!strcmp(line, "reply_asked")) s.reply_asked = v != 0;
    }
    fclose(f);
    return true;
}

bool settings_save(const std::string& dir, const Settings& s)
{
    mkdir(dir.c_str(), 0755);
    FILE* f = fopen(file_of(dir).c_str(), "w");
    if (!f) return false;
    fprintf(f, "fit=%d\nsound=%d\nvolume=%d\nfont=%d\nswap_ab=%d\npad_moves=%d\n", s.fit, s.sound, s.volume, s.font,
            s.swap_ab ? 1 : 0, s.pad_moves ? 1 : 0);
    fprintf(f, "music=%d\ncomforts_off=%d\nmax_hp=%d\nreply_asked=%d\nreply_email=%s\n", s.music ? 1 : 0, s.comforts_off,
            s.max_hp ? 1 : 0, s.reply_asked ? 1 : 0, s.reply_email.c_str());
    return fclose(f) == 0;
}
