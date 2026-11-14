// Gold Box Game Engine for PortMaster: the SDL2 front end.
//
// The games themselves are the CYD engine's code (the cyd/ submodule:
// cyd/src/engine, cyd/src/game), run from the player's own DOS game files
// in games/. This file is what the CYD's touch screen front end
// (cyd/src/ui) is on the boards: it shows the game's 320 x 200 canvas,
// turns a handheld's buttons into the game's input (play::act / nav / tap
// / input_key), and draws the port's own screens - the game list, the
// menu, the on-screen keyboard.
#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "game/music_make.h"
#include "game/play.h"
#include "platform/audio.h"
#include "platform/file.h"
#include "platform/platform.h"

#include "host.h"
#include "art.h"
#include "library.h"
#include "panel.h"
#include "settings.h"
#include "task.h"
#include "video.h"

#ifndef PM_VERSION
#define PM_VERSION "dev"
#endif

namespace {

// ---- Buttons ---------------------------------------------------------------
enum Btn : uint8_t { BUp, BDown, BLeft, BRight, BA, BB, BX, BY, BL1, BR1, BL2, BR2, BStart, BSelect, BCount };
const char* const kBtnNames[BCount] = {"up", "down", "left", "right", "a", "b", "x", "y",
                                       "l1", "r1", "l2", "r2", "start", "select"};

enum class Screen : uint8_t { Games, Play, Message };

Settings                      cfg;
std::string                   conf_dir = "conf";
std::string                   tools_dir = "tools";    // the helper scripts (update.sh, sendlogs.sh)
std::vector<library::GameDir> games_found;
bool                          have_root = false;
int                           game_sel = 0;
Screen                        screen = Screen::Games;
bool                          quit = false;

// The game screen
std::vector<uint8_t> pixels(video::kW * video::kH);
pic::Canvas          cv{pixels.data(), video::kW, video::kH, 4};
bool                 playing = false;
int                  fight_colours = 0;
int                  flash_index = -1, flash_colour = -1;

// A message over everything, until a button (errors, journal mentions)
std::string msg_title, msg_body;
Screen      msg_back = Screen::Games;

// The menu (Start)
bool menu_open = false;
int  menu_sel = 0;
bool help_open = false;

// The on-screen keyboard (while the game asks for a number or text)
struct Key {
    char        ch;
    const char* label;
    int         col, row, w;    // text cells
};
std::vector<Key> keys;
play::Input      keys_for = play::Input::None;
int              key_sel = 0;
// The same keyboard for a line the port itself asks for (the log server's address)
bool        prompt_open = false;
std::string prompt_text;

// The pointer: a tap anywhere on the game screen, for the rare thing the
// highlight doesn't reach. Buttons alone do it (most handhelds have no
// sticks and no touch screen): Select turns Pointer Mode on - the D-pad
// moves the arrow, Confirm taps, Back or Select ends it. A stick, a mouse
// or a touch screen move / tap it too where there is one (extras only).
float  ptr_x = video::kW / 2, ptr_y = video::kH / 2;
bool   ptr_on = false;
bool   ptr_mode = false;        // the D-pad moves it
Uint32 ptr_seen = 0;
Uint32 ptr_held_since = 0;
bool   select_alone = false;    // Select went down and nothing else since (Select + Start is the firmware's quit)

// Held D-pad directions and their repeat
bool   held[4] = {};
int    last_dir = -1;
Uint32 next_fire = 0;
bool   fired_once = false;

SDL_GameController* pad = nullptr;
bool                trig[2] = {};

// ---- The palette (as the CYD front end keeps its panel's colours) ----------
void show_flash(int index, int colour)
{
    if (colour == flash_colour) return;
    if (flash_index >= 0) video::set_palette(flash_index, pic::kEga[flash_index]);
    flash_colour = colour;
    flash_index = colour >= 0 ? index : -1;
    if (colour >= 0) video::set_palette(index, pic::kEga[colour]);
}

void sync_palette(bool all)
{
    const pic::Rgb* p = nullptr;
    if (play::palette(&p, all))
        for (int i = 0; i < 256; ++i) video::set_palette(i, p[i]);
    // A fight swaps colours 0 and 8 (the field's black shows dark grey);
    // Gateway's shows 8 black, under water blue
    if (play::fight_colours() != fight_colours) {
        fight_colours = play::fight_colours();
        video::set_ega_palette();
        if (fight_colours == 1) {
            video::set_palette(0, pic::kEga[8]);
            video::set_palette(8, pic::kEga[0]);
        } else if (fight_colours == 2) {
            video::set_palette(8, pic::kEga[0]);
        } else if (fight_colours == 3) {
            video::set_palette(8, pic::kEga[1]);
        }
    }
    int index = 0;
    const int colour = play::flash(index);
    show_flash(index, colour);
}

// ---- Drawing the port's own screens ----------------------------------------
void centred(int row, const char* s, uint32_t fg, uint32_t bg = video::kClear)
{
    int col = (40 - static_cast<int>(strlen(s))) / 2;
    if (col < 0) col = 0;
    video::text(col, row, s, fg, bg);
}

// Text wrapped at word ends into cols columns from (col, row); returns the
// row after the last line
int wrapped(int col, int row, int cols, const std::string& s, uint32_t fg)
{
    size_t i = 0;
    int y = row * 8;                    // (lines 10 pixels apart: the font fills its 8)
    while (i < s.size() && y < video::kH - 8) {
        while (i < s.size() && s[i] == ' ') ++i;
        size_t end = i, brk = i;
        bool nl = false;
        while (end < s.size() && end - i < static_cast<size_t>(cols)) {
            if (s[end] == '\n') {
                nl = true;
                break;
            }
            if (s[end] == ' ') brk = end;
            ++end;
        }
        if (end < s.size() && !nl && s[end] != ' ' && brk > i) end = brk;
        video::text_at(col * 8, y, s.substr(i, end - i).c_str(), fg);
        y += 10;
        i = end + (nl ? 1 : 0);
    }
    return (y + 7) / 8;
}

void box(int col, int row, int cols, int rows)
{
    video::fill(col * 8 - 4, row * 8 - 4, cols * 8 + 8, rows * 8 + 8, video::kNavy);
    video::frame(col * 8 - 4, row * 8 - 4, cols * 8 + 8, rows * 8 + 8, video::kGold);
}

const char* confirm_name() { return cfg.swap_ab ? "B" : "A"; }
const char* back_name() { return cfg.swap_ab ? "A" : "B"; }

// The game list: a game a page - its box art (the player's own, from the
// Steam release's Launcher folder in games/) under the header, its name
// across the foot; Left / Right turn the pages, round. As the CYD's.
constexpr int kHeaderH = 20;

void draw_games()
{
    video::clear(video::kClear);
    video::set_backdrop(nullptr);
    video::fill(0, 0, video::kW, kHeaderH, video::kNavy);
    video::text_at(8, 6, "Gold Box Game Engine", video::kGold);
    video::text_at(video::kW - 8 - static_cast<int>(strlen(PM_VERSION)) * 8, 6, PM_VERSION, video::kMuted);
    if (games_found.empty()) {
        video::fill(0, kHeaderH, video::kW, video::kH - kHeaderH, video::kBlack);
        int row = 4;
        centred(row, have_root ? "No games found" : "No games folder", video::kWarn);
        row = wrapped(2, row + 2, 36,
                      "This port plays SSI's Gold Box games from your own copies. Copy each game's whole folder "
                      "(from GOG or Steam) into the port's games folder:",
                      video::kCream);
        centred(row + 1, "ports/goldbox/games/", video::kGold);
        wrapped(2, row + 3, 36, "for example games/Curse of the Azure Bonds/ - then start the port again.",
                video::kCream);
        char line[41];
        snprintf(line, sizeof line, "Start: Menu   %s: Look Again", confirm_name());
        centred(23, line, video::kMuted);
        return;
    }
    const int n = static_cast<int>(games_found.size());
    const library::GameDir& g = games_found[game_sel];
    const bool can = play::available(g.game);
    const uint32_t* picture = art::box(g.game, (video::kH - kHeaderH) * 2);
    if (picture) video::set_backdrop(picture, kHeaderH);
    else video::fill(0, kHeaderH, video::kW, video::kH - kHeaderH, video::kBlack);
    // The name: big, across the foot (two lines when it's long)
    const char* title = g.game == games::Game::Unknown ? g.folder.c_str() : games::title(g.game);
    std::string l1 = title, l2;
    if (l1.size() > 19) {
        size_t cut = l1.rfind(' ', 19);
        if (cut == std::string::npos) cut = 19;
        l2 = l1.substr(cut + (l1[cut] == ' ' ? 1 : 0), 19);
        l1 = l1.substr(0, cut);
    }
    const int ty = picture ? (l2.empty() ? 162 : 144) : (l2.empty() ? 92 : 82);
    video::text_big((video::kW - static_cast<int>(l1.size()) * 16) / 2, ty, l1.c_str(), can ? video::kCream : video::kMuted);
    if (!l2.empty())
        video::text_big((video::kW - static_cast<int>(l2.size()) * 16) / 2, ty + 18, l2.c_str(), can ? video::kCream : video::kMuted);
    if (!can) {
        video::fill(104, ty - 14, 112, 11, video::kBlack);
        video::text_at(112, ty - 12, "Not playable", video::kWarn);
    }
    // The pages' clue: a small arrowhead at each side, which game of how many
    if (n > 1) {
        for (int k = 0; k < 7; ++k) {
            video::fill(4 + k, 106 - k, 1, 2 * k + 1, video::kBlack);
            video::fill(video::kW - 5 - k, 106 - k, 1, 2 * k + 1, video::kBlack);
        }
        for (int k = 0; k < 5; ++k) {
            video::fill(6 + k, 106 - k, 1, 2 * k + 1, video::kCream);
            video::fill(video::kW - 7 - k, 106 - k, 1, 2 * k + 1, video::kCream);
        }
        char page[24];
        snprintf(page, sizeof page, "%d of %d", (game_sel + 1) % 1000, n % 1000);
        video::fill(video::kW - 12 - static_cast<int>(strlen(page)) * 8, kHeaderH + 3, static_cast<int>(strlen(page)) * 8 + 8, 12, video::kBlack);
        video::text_at(video::kW - 8 - static_cast<int>(strlen(page)) * 8, kHeaderH + 5, page, video::kMuted);
    }
}

void draw_message()
{
    video::set_backdrop(nullptr);
    video::clear(playing ? video::kClear : video::kBlack);
    box(3, 6, 34, 13);
    centred(7, msg_title.c_str(), video::kGold);
    wrapped(4, 9, 32, msg_body, video::kCream);
    char line[41];
    snprintf(line, sizeof line, "%s: Continue", confirm_name());
    centred(18, line, video::kMuted);
}

uint8_t     sound_shown();
const char* sound_name(uint8_t m);
void        sound_cycle(int step);

// ---- The network jobs (tools/update.sh, tools/sendlogs.sh) ------------------
enum class Job : uint8_t { None, Check, Fetch, Logs };
Job         job = Job::None;
std::string update_tag, update_zip, update_sha;     // the release a check found
std::string update_staged;                          // downloaded: this version starts at the next launch
bool        restart_wanted = false;

// A box over everything: a job's progress, then its result and choices
struct Notice {
    bool        open = false;
    std::string title, body;
    const char* yes = nullptr;      // Confirm's label (nullptr: none)
    const char* no = "Close";       // Back's
    void (*on_yes)() = nullptr;
};
Notice notice;

void show_notice(const char* title, const std::string& body, const char* yes = nullptr, void (*on_yes)() = nullptr,
                 const char* no = "Close")
{
    notice.open = true;
    notice.title = title;
    notice.body = body;
    notice.yes = yes;
    notice.no = no;
    notice.on_yes = on_yes;
}

std::string tool(const char* name) { return task::quoted(tools_dir + "/" + name); }

// The folder the port lives in (tools/ is inside it)
std::string port_dir()
{
    const size_t k = tools_dir.rfind('/');
    return k == std::string::npos ? "." : tools_dir.substr(0, k);
}

bool tools_ready(const char* name)
{
    if (FILE* f = fopen((tools_dir + "/" + name).c_str(), "r")) {
        fclose(f);
        return true;
    }
    show_notice("Not Available", "This build has no tools folder (it isn't an installed port).");
    return false;
}

void run_job(Job j, const std::string& command, const char* title, const char* text)
{
    if (!task::start("/bin/bash " + command + " 2>&1")) return;
    job = j;
    show_notice(title, text, nullptr, nullptr, nullptr);
}

void update_restart()
{
    // (the launch script puts the staged version in place and starts it)
    if (FILE* f = fopen((port_dir() + "/restart").c_str(), "w")) fclose(f);
    restart_wanted = true;
}

void update_fetch()
{
    run_job(Job::Fetch,
            tool("update.sh") + " fetch " + task::quoted(update_tag) + " " + task::quoted(update_zip) + " " +
                task::quoted(update_sha) + " " + task::quoted(port_dir()),
            "Updating", "Downloading the new version...");
}

void update_check()
{
    if (!update_staged.empty()) {
        show_notice("Update Ready", "Version " + update_staged.substr(1) + " is downloaded. It starts the next time the port does.",
                    "Restart Now", update_restart, "Later");
        return;
    }
    if (!tools_ready("update.sh")) return;
    run_job(Job::Check, tool("update.sh") + " check", "Updates", "Looking for a new version...");
}

void send_logs()
{
    if (cfg.log_url.empty()) {
        show_notice("Send Logs", "Set the log server's address first: the menu's Log Server line.");
        return;
    }
    if (!tools_ready("sendlogs.sh")) return;
    fflush(stdout);
    run_job(Job::Logs,
            tool("sendlogs.sh") + " " + task::quoted(cfg.log_url) + " " + task::quoted(port_dir()) + " " PM_VERSION,
            "Send Logs", "Sending the logs...");
}

// "v0.10.2" / "0.10.2" as one number to compare
long version_number(const std::string& v)
{
    long n[3] = {0, 0, 0};
    sscanf(v.c_str() + (v[0] == 'v' || v[0] == 'V' ? 1 : 0), "%ld.%ld.%ld", &n[0], &n[1], &n[2]);
    return (n[0] * 1000 + n[1]) * 1000 + n[2];
}

void job_tick()
{
    std::string out;
    int code = 0;
    if (job == Job::None || !task::finished(out, code)) return;
    const Job was = job;
    job = Job::None;
    plat::log("[port] job %d ended (%d): %s", static_cast<int>(was), code, out.c_str());
    const std::string err = task::field(out, "ERROR");
    if (code != 0 || !err.empty()) {
        show_notice(was == Job::Logs ? "Send Logs" : "Updates", err.empty() ? "It didn't work (see log.txt)." : err);
        return;
    }
    if (was == Job::Check) {
        update_tag = task::field(out, "TAG");
        update_zip = task::field(out, "ZIP");
        update_sha = task::field(out, "SHA");
        const bool pre = task::field(out, "PRE") == "1";
        if (update_tag.empty() || version_number(update_tag) <= version_number(PM_VERSION)) {
            show_notice("Updates", "This is the newest version (" PM_VERSION ").");
            return;
        }
        show_notice("Update Found",
                    "Version " + update_tag.substr(1) + (pre ? " (a pre-release)" : "") +
                        " is out. You have " PM_VERSION ".",
                    "Download", update_fetch, "Later");
    } else if (was == Job::Fetch) {
        update_staged = task::field(out, "STAGED");
        show_notice("Update Ready", "Version " + update_staged.substr(1) + " is downloaded and checked. It starts the next time the port does.",
                    "Restart Now", update_restart, "Later");
    } else {
        std::string n = task::field(out, "SENT");
        show_notice("Send Logs", "Sent to the log server: " + n.substr(0, n.find(' ')) + " file(s).");
    }
}

void draw_notice()
{
    box(3, 6, 34, 13);
    centred(7, notice.title.c_str(), video::kGold);
    wrapped(4, 9, 32, notice.body, video::kCream);
    char line[41] = "";
    if (job != Job::None) snprintf(line, sizeof line, "Please wait...");
    else if (notice.yes) snprintf(line, sizeof line, "%s: %s   %s: %s", confirm_name(), notice.yes, back_name(), notice.no);
    else snprintf(line, sizeof line, "%s: %s", confirm_name(), notice.no ? notice.no : "Close");
    centred(18, line, video::kMuted);
}

// ---- The menu --------------------------------------------------------------
enum MenuItem : uint8_t {
    MResume, MControls, MDisplay, MSound, MMusic, MVolume, MFont, MPadMoves, MSwap, MRescan, MUpdate, MSendLogs, MLogServer,
    MLeaveGame, MExit
};
std::vector<MenuItem> menu_items;

void menu_build()
{
    menu_items.clear();
    if (playing) menu_items.push_back(MResume);
    menu_items.push_back(MControls);
    menu_items.push_back(MDisplay);
    menu_items.push_back(MSound);
    menu_items.push_back(MMusic);
    menu_items.push_back(MVolume);
    menu_items.push_back(MFont);
    menu_items.push_back(MPadMoves);
    menu_items.push_back(MSwap);
    if (!playing) menu_items.push_back(MRescan);
    menu_items.push_back(MUpdate);
    menu_items.push_back(MSendLogs);
    menu_items.push_back(MLogServer);
    if (playing) menu_items.push_back(MLeaveGame);
    menu_items.push_back(MExit);
    if (menu_sel >= static_cast<int>(menu_items.size())) menu_sel = 0;
}

void menu_label(MenuItem m, char* out, size_t cap)
{
    static const char* const kFont[] = {"Original", "Clear", "Darkness", "Clear Caps"};
    switch (m) {
    case MResume: snprintf(out, cap, "Back to Game"); break;
    case MControls: snprintf(out, cap, "Controls"); break;
    case MDisplay: snprintf(out, cap, "Display: %s", video::fit_name(static_cast<video::Fit>(cfg.fit))); break;
    case MSound: snprintf(out, cap, "Game Sound: %s", sound_name(sound_shown())); break;
    case MMusic: snprintf(out, cap, "Music: %s", cfg.music ? "On" : "Off"); break;
    case MVolume: snprintf(out, cap, "Volume: %d%%", cfg.volume); break;
    case MFont: snprintf(out, cap, "Game Font: %s", kFont[cfg.font]); break;
    case MPadMoves: snprintf(out, cap, "D-Pad Walks: %s", cfg.pad_moves ? "On" : "Off"); break;
    case MSwap: snprintf(out, cap, "Confirm Button: %s", confirm_name()); break;
    case MRescan: snprintf(out, cap, "Look for Games Again"); break;
    case MUpdate: snprintf(out, cap, "%s", update_staged.empty() ? "Check for Updates" : "Restart to Update"); break;
    case MSendLogs: snprintf(out, cap, "Send Logs"); break;
    case MLogServer: snprintf(out, cap, "Log Server: %.14s", cfg.log_url.empty() ? "Not Set" : cfg.log_url.c_str()); break;
    case MLeaveGame: snprintf(out, cap, "Exit Game"); break;
    case MExit: snprintf(out, cap, "Quit"); break;
    }
}

void draw_help()
{
    box(2, 2, 36, 21);
    centred(2, "Controls", video::kGold);
    const char* a = confirm_name();
    const char* b = back_name();
    struct Line { const char* key; const char* what; };
    const Line lines[] = {
        {"D-Pad", "Walk / move the highlight"},
        {a, "Choose the highlighted thing"},
        {b, "Back / the menu's Exit"},
        {"L1 R1", "Menu word left / right"},
        {"L2 R2", "List line up / down"},
        {"X", "Area map"},
        {"Y", "Companion: map, party"},
        {"Select", "Pointer: D-Pad moves it,"},
        {"", "Confirm taps, Back ends"},
        {"Start", "This menu"},
        {"In a fight:", ""},
        {"D-Pad", "Move / aim (two = diagonal)"},
        {"L1 R1", "Up-left / up-right"},
        {"L2 R2", "Down-left / down-right"},
        {a, "End the move / Target"},
    };
    int row = 4;
    for (const Line& l : lines) {
        video::text(3, row, l.key, video::kGold);
        video::text(l.what[0] && strlen(l.key) > 6 ? 15 : 10, row, l.what, video::kCream);
        ++row;
    }
    char line[41];
    snprintf(line, sizeof line, "Select + Start: quit   %s: Back", b);
    centred(21, line, video::kMuted);
}

void draw_menu()
{
    if (help_open) {
        draw_help();
        return;
    }
    const int n = static_cast<int>(menu_items.size());
    const int pitch = n > 11 ? 12 : 14, h = n * pitch + 8, y0 = (video::kH - h) / 2;
    video::fill(52, y0, 216, h, video::kNavy);
    video::frame(52, y0, 216, h, video::kGold);
    for (int i = 0; i < n; ++i) {
        char line[40];
        menu_label(menu_items[i], line, sizeof line);
        const int y = y0 + 4 + i * pitch;
        const bool sel = i == menu_sel;
        if (sel) video::fill(54, y, 212, pitch, video::kLit);
        video::text_at(62, y + (pitch - 8) / 2, line, sel ? video::kCream : video::kMuted);
    }
}

// ---- The on-screen keyboard ------------------------------------------------
void keys_build(play::Input in)
{
    keys.clear();
    keys_for = in;
    key_sel = 0;
    if (in == play::Input::None) return;
    static const char* const kRows[4] = {"ABCDEFGHIJ", "KLMNOPQRST", "UVWXYZ'-.,", "0123456789"};
    int row = 2;
    for (int r = in == play::Input::Number ? 3 : 0; r < 4; ++r, row += 2)
        for (int i = 0; kRows[r][i]; ++i) keys.push_back(Key{kRows[r][i], nullptr, 5 + i * 3, row, 3});
    if (in == play::Input::Text) keys.push_back(Key{' ', "Space", 5, row, 9});
    keys.push_back(Key{'\b', "Delete", in == play::Input::Text ? 15 : 5, row, 10});
    keys.push_back(Key{'\n', "Done", in == play::Input::Text ? 26 : 16, row, 9});
}

void draw_keys()
{
    if (keys.empty()) return;
    const int rows = keys.back().row + 2 - keys.front().row;
    box(5, keys.front().row, 30, rows - 1);
    for (size_t i = 0; i < keys.size(); ++i) {
        const Key& k = keys[i];
        const bool sel = static_cast<int>(i) == key_sel;
        video::fill(k.col * 8 + 1, k.row * 8 - 3, k.w * 8 - 2, 14, sel ? video::kLit : video::kKey);
        if (sel) video::frame(k.col * 8 + 1, k.row * 8 - 3, k.w * 8 - 2, 14, video::kGold);
        char one[2] = {k.ch, 0};
        const char* s = k.label ? k.label : one;
        video::text_at(k.col * 8 + (k.w * 8 - static_cast<int>(strlen(s)) * 8) / 2, k.row * 8, s, video::kCream);
    }
}

void prompt_begin()
{
    keys.clear();
    key_sel = 0;
    static const char* const kRows[4] = {"abcdefghij", "klmnopqrst", "uvwxyz.:/-", "0123456789"};
    int row = 2;
    for (int r = 0; r < 4; ++r, row += 2)
        for (int i = 0; kRows[r][i]; ++i) keys.push_back(Key{kRows[r][i], nullptr, 5 + i * 3, row, 3});
    keys.push_back(Key{'\b', "Delete", 5, row, 15});
    keys.push_back(Key{'\n', "Done", 20, row, 15});
    prompt_text = cfg.log_url.empty() ? "http://" : cfg.log_url;
    prompt_open = true;
}

void prompt_key(char ch)
{
    if (ch == '\b') {
        if (!prompt_text.empty()) prompt_text.pop_back();
    } else if (ch == '\n') {
        cfg.log_url = prompt_text == "http://" ? "" : prompt_text;
        settings_save(conf_dir, cfg);
        prompt_open = false;
        keys.clear();
        keys_for = play::Input::None;       // (the game's own keyboard, if it was up, is built again)
    } else if (prompt_text.size() < 60) {
        prompt_text += ch;
    }
}

void draw_prompt()
{
    draw_keys();
    box(2, 15, 36, 8);
    video::text(3, 15, "Log server (WebDAV) address:", video::kGold);
    const size_t n = prompt_text.size();
    video::text(3, 17, (prompt_text.substr(n > 33 ? n - 33 : 0) + "_").c_str(), video::kCream);
    video::text(3, 19, "e.g. http://192.168.1.20:5005", video::kMuted);
    char line[41];
    snprintf(line, sizeof line, "%s: Type  %s: Delete  Y: Done", confirm_name(), back_name());
    video::text(3, 21, line, video::kMuted);
}

// The key nearest to the chosen one's middle, a row up (dy -1) or down
void keys_move(int dx, int dy)
{
    if (keys.empty()) return;
    const Key& c = keys[key_sel];
    if (dx) {
        const int n = static_cast<int>(keys.size());
        for (int s = 1; s < n; ++s) {
            const int i = (key_sel + dx * s + n * s) % n;
            if (keys[i].row == c.row) {
                key_sel = i;
                return;
            }
        }
        return;
    }
    const int mid = c.col * 2 + c.w;
    int best = -1, best_d = 1 << 30, want_row = -1;
    for (int pass = 0; pass < 2 && best < 0; ++pass) {
        // (the next row that way; past the last, round to the other end)
        want_row = -1;
        for (const Key& k : keys) {
            const bool ahead = pass == 0 ? (dy > 0 ? k.row > c.row : k.row < c.row) : true;
            if (!ahead || k.row == c.row) continue;
            if (want_row < 0 || (pass == 0 ? (dy > 0 ? k.row < want_row : k.row > want_row)
                                           : (dy > 0 ? k.row < want_row : k.row > want_row)))
                want_row = k.row;
        }
        for (size_t i = 0; i < keys.size() && want_row >= 0; ++i)
            if (keys[i].row == want_row) {
                const int d = abs(keys[i].col * 2 + keys[i].w - mid);
                if (d < best_d) {
                    best_d = d;
                    best = static_cast<int>(i);
                }
            }
    }
    if (best >= 0) key_sel = best;
}

// ---- Frames ----------------------------------------------------------------
void draw_frame()
{
    if (screen == Screen::Games) {
        draw_games();
    } else if (screen == Screen::Message) {
        draw_message();
    } else {
        video::clear();
        panel::draw(SDL_GetTicks());
        if (!menu_open && !prompt_open) draw_keys();
        if (ptr_on && !menu_open) video::pointer(static_cast<int>(ptr_x), static_cast<int>(ptr_y));
    }
    if (menu_open) draw_menu();
    if (prompt_open) draw_prompt();
    if (notice.open) draw_notice();
    video::present(playing ? &cv : nullptr);
}

// (the game's "show this now": a message up before slow work, as saving)
void present_now()
{
    if (!playing) return;
    sync_palette(false);
    draw_frame();
}

void message(const char* title, const std::string& body)
{
    msg_title = title;
    msg_body = body;
    if (screen != Screen::Message) msg_back = screen;
    screen = Screen::Message;
}

// ---- The game --------------------------------------------------------------
// ---- Game Sound (as the CYD's): Tandy, PC Speaker, then Darkness /
// Gateway / Treasures - that game's own recordings lent to the older
// games' effects - when that game is in games/; Off. Darkness, Gateway
// and Treasures themselves always play their own.
constexpr uint8_t kSndTandy = 1, kSndPc = 2, kSndOff = 3, kSndDarkness = 4, kSndGateway = 5, kSndTreasures = 6;

uint8_t own_sound(games::Game g)
{
    return g == games::Game::PoolsOfDarkness              ? kSndDarkness
         : g == games::Game::GatewayToTheSavageFrontier   ? kSndGateway
         : g == games::Game::TreasuresOfTheSavageFrontier ? kSndTreasures
                                                          : 0;
}

games::Game sound_game() { return games_found.empty() ? games::Game::Unknown : games_found[game_sel].game; }

bool sound_ok(uint8_t m)
{
    if (m == kSndTandy || m == kSndPc || m == kSndOff) return true;
    for (const auto& g : games_found)
        if (!g.hlib && own_sound(g.game) == m) return true;
    return false;
}

// The set in use: the game's own, else the setting (Tandy when the
// setting's game isn't there any more)
uint8_t sound_shown()
{
    const uint8_t own = own_sound(sound_game());
    if (own && cfg.sound != kSndOff) return own;
    return sound_ok(cfg.sound) ? cfg.sound : kSndTandy;
}

const char* sound_name(uint8_t m)
{
    switch (m) {
    case kSndPc: return "PC Speaker";
    case kSndOff: return "Off";
    case kSndDarkness: return "Darkness";
    case kSndGateway: return "Gateway";
    case kSndTreasures: return "Treasures";
    default: return "Tandy";
    }
}

void sound_cycle(int step)
{
    static const uint8_t kOrder[6] = {kSndTandy, kSndPc, kSndDarkness, kSndGateway, kSndTreasures, kSndOff};
    // (in Darkness, Gateway or Treasures: its own or Off)
    const uint8_t own = own_sound(sound_game());
    if (own) {
        cfg.sound = cfg.sound == kSndOff ? own : kSndOff;
        return;
    }
    int at = 0;
    for (int i = 0; i < 6; ++i)
        if (kOrder[i] == sound_shown()) at = i;
    for (int k = 0; k < 6; ++k) {
        at = (at + (step < 0 ? 5 : 1)) % 6;
        if (sound_ok(kOrder[at])) break;
    }
    cfg.sound = kOrder[at];
}

void apply_sound()
{
    const uint8_t m = sound_shown();
    uint8_t mode = m == kSndOff ? kSndOff : m == kSndPc ? kSndPc : kSndTandy;
    std::string rec_dir;
    games::Game rec = games::Game::Unknown;
    if (m >= kSndDarkness && !own_sound(sound_game()))
        for (const auto& g : games_found)
            if (!g.hlib && own_sound(g.game) == m) {
                mode = 4;                       // (recorded: that game's ZOUNDS lent)
                rec_dir = g.data_dir;
                rec = g.game;
                break;
            }
    play::set_sound(mode, static_cast<uint8_t>(cfg.volume * 255 / 100), rec_dir.empty() ? nullptr : rec_dir.c_str(), rec);
    play::set_music(cfg.music);
}

// A line under "Loading ..." while a game's first start makes its files
void loading_line(const char* line, bool, void*)
{
    video::fill(0, 14 * 8 - 2, video::kW, 40, video::kBlack);
    wrapped(2, 14, 36, line, video::kMuted);
    video::present(nullptr);
    SDL_PumpEvents();           // (minutes may pass: the window stays alive)
}

void apply_options()
{
    play::Options o;
    o.font = cfg.font;
    play::set_options(o);
}

std::string card_dir(const library::GameDir& g) { return std::string(games::kRootDir) + "/" + g.data_dir; }

void leave_game()
{
    if (!playing) return;
    audio_stop();
    play::close();
    panel::close();
    playing = false;
    menu_open = false;
    keys_build(play::Input::None);
    video::set_ega_palette();
    screen = Screen::Games;
}

void start_game()
{
    if (games_found.empty()) return;
    const library::GameDir& g = games_found[game_sel];
    if (!play::available(g.game)) {
        message("Not Playable", g.game == games::Game::Unknown
                                    ? "These look like Gold Box game files, but the folder's name doesn't say which "
                                      "game. Name the folder after the game."
                                    : "The engine can't play this game yet.");
        return;
    }
    // (on screen at once: the first load can take a few seconds)
    video::set_backdrop(nullptr);
    video::clear(video::kBlack);
    char line[80];
    snprintf(line, sizeof line, "Loading %.28s...", games::title(g.game));
    wrapped(2, 10, 36, line, video::kGold);
    centred(14, "This can take a few seconds.", video::kMuted);
    video::present(nullptr);

    video::set_ega_palette();
    fight_colours = 0;
    flash_index = flash_colour = -1;
    apply_sound();
    apply_options();
    // The player's Pools of Darkness folder (its font: a Game Font choice)
    play::set_darkness_dir("");
    for (const auto& o : games_found)
        if (o.game == games::Game::PoolsOfDarkness && !o.hlib) play::set_darkness_dir(card_dir(o).c_str());
    // The earlier game's folder: its characters can be added
    play::set_import_dir("");
    using G = games::Game;
    const G here = g.game;
    const G earlier = here == G::CurseOfTheAzureBonds          ? G::PoolOfRadiance
                    : here == G::SecretOfTheSilverBlades      ? G::CurseOfTheAzureBonds
                    : here == G::PoolsOfDarkness              ? G::SecretOfTheSilverBlades
                    : here == G::TreasuresOfTheSavageFrontier ? G::GatewayToTheSavageFrontier
                    : here == G::DeathKnightsOfKrynn          ? G::ChampionsOfKrynn
                    : here == G::DarkQueenOfKrynn             ? G::DeathKnightsOfKrynn
                                                              : here;
    for (const auto& o : games_found)
        if (earlier != here && o.game == earlier && !o.hlib) {
            const std::string pd = card_dir(o), sv = pd + "/SAVE";
            play::set_import_dir(plat::exists(sv.c_str()) ? sv.c_str() : pd.c_str());
            break;
        }
    const int bpp = play::canvas_depth(here);
    cv.bpp = static_cast<uint8_t>(bpp == 8 ? 8 : 4);
    cv.vga = cv.bpp == 8;
    cv.bg = nullptr;
    cv.bg_rows = 0;
    cv.paper = -1;
    memset(pixels.data(), 0, pixels.size());
    const std::string cache = library::cache_dir(g);
    // The game's music (the later games): made once from its own files -
    // the CYD's card scan does this; here a game's first start
    if (!g.hlib && musicfiles::songs(g.data_dir.c_str()) && !musicfiles::ready(g.data_dir.c_str(), cache.c_str())) {
        loading_line("Making the game's music (once; it can take a few minutes)...", false, nullptr);
        const int failed = musicfiles::make(g.data_dir.c_str(), cache.c_str(), games::short_title(g.game), false, loading_line, nullptr);
        plat::log("[port] music made for %s (%d failed)\n", games::title(here), failed);
    }
    const char* err = play::open(g.data_dir.c_str(), here, cv, cache.c_str());
    if (err) {
        play::close();
        message("Can't Start the Game", err);
        return;
    }
    play::set_present_now(present_now);
    playing = true;
    screen = Screen::Play;
    sync_palette(true);
    plat::log("[port] playing %s from games/%s\n", games::title(here), g.data_dir.c_str());
}

// The game mentioned journal entries: named until the port has a journal
// viewer of its own
bool journal_due()
{
    char kind;
    int number;
    if (!play::journal_waiting() || !play::journal_request(&kind, &number)) return false;
    std::string what;
    do {
        char one[48];
        snprintf(one, sizeof one, "%s%s %d", what.empty() ? "" : ", ",
                 kind == 'T' ? "Tavern Tale" : kind == 'P' ? "Proclamation" : kind == 'I' ? "the Introduction" : "Journal Entry",
                 number);
        if (kind == 'I') snprintf(one, sizeof one, "%sthe Introduction", what.empty() ? "" : ", ");
        what += one;
    } while (play::journal_request(&kind, &number));
    message("Adventurer's Journal", "Read " + what + " in the journal that came with your game (its PDF).");
    return true;
}

void after_input()
{
    if (playing && play::exit_requested()) leave_game();
}

// A tap on the game screen: what it acts on lights up first
void canvas_tap(int x, int y)
{
    int y0, y1;
    if (play::tap_highlight(x, y, cv, &y0, &y1)) {
        draw_frame();
        if (play::tap_highlight_blink(cv)) {
            SDL_Delay(70);
            draw_frame();
        }
    }
    play::tap(x, y, cv);
    play::tap_highlight_end(cv);
    after_input();
}

void confirm()
{
    if (ptr_on) {
        if (journal_due()) return;
        canvas_tap(static_cast<int>(ptr_x), static_cast<int>(ptr_y));
        return;
    }
    if (play::fight_pad()) {
        play::fight_select(cv);
        return;
    }
    if (journal_due()) return;
    int x, y;
    if (play::nav_point(&x, &y)) canvas_tap(x, y);
    else canvas_tap(2, 2);          // (nothing highlighted: "press a key" - any tap goes on)
}

// A direction in a fight from the D-pad's held buttons (0 north, clockwise)
int fight_dir_held()
{
    const bool u = held[BUp], d = held[BDown], l = held[BLeft], r = held[BRight];
    if (u && r) return 1;
    if (d && r) return 3;
    if (d && l) return 5;
    if (u && l) return 7;
    if (u) return 0;
    if (r) return 2;
    if (d) return 4;
    if (l) return 6;
    return -1;
}

void game_dir(int b)
{
    if (!keys.empty()) {
        keys_move(b == BLeft ? -1 : b == BRight ? 1 : 0, b == BUp ? -1 : b == BDown ? 1 : 0);
        return;
    }
    if (play::fight_pad()) {
        const int d = fight_dir_held();
        if (d >= 0) play::fight_dir(d, cv);
        return;
    }
    if (cfg.pad_moves && play::walking()) {
        play::act(b == BUp ? play::Act::Forward : b == BDown ? play::Act::TurnAround
                  : b == BLeft ? play::Act::TurnLeft : play::Act::TurnRight, cv);
        return;
    }
    play::nav(b == BUp ? play::Nav::Up : b == BDown ? play::Nav::Down : b == BLeft ? play::Nav::Left : play::Nav::Right, cv);
}

void menu_change(int step)
{
    const MenuItem m = menu_items[menu_sel];
    switch (m) {
    case MDisplay:
        cfg.fit = static_cast<uint8_t>((cfg.fit + (step < 0 ? 2 : 1)) % 3);
        video::set_fit(static_cast<video::Fit>(cfg.fit));
        break;
    case MSound:
        sound_cycle(step);
        if (playing) apply_sound();
        break;
    case MMusic:
        cfg.music = !cfg.music;
        play::set_music(cfg.music);
        break;
    case MVolume: {
        int v = cfg.volume + (step < 0 ? -10 : 10);
        if (step == 0 && v > 100) v = 0;
        cfg.volume = static_cast<uint8_t>(v < 0 ? 0 : v > 100 ? 100 : v);
        if (playing) apply_sound();
        break;
    }
    case MFont:
        cfg.font = static_cast<uint8_t>((cfg.font + (step < 0 ? 3 : 1)) % 4);
        if (playing) apply_options();
        break;
    case MPadMoves: cfg.pad_moves = !cfg.pad_moves; break;
    case MSwap: cfg.swap_ab = !cfg.swap_ab; break;
    default: return;
    }
    settings_save(conf_dir, cfg);
}

void rescan()
{
    have_root = library::scan(games_found);
    if (game_sel >= static_cast<int>(games_found.size())) game_sel = 0;
}

void menu_press(int b)
{
    if (help_open) {
        if (b == BA || b == BB || b == BStart) help_open = false;
        return;
    }
    const int n = static_cast<int>(menu_items.size());
    switch (b) {
    case BUp: menu_sel = (menu_sel + n - 1) % n; break;
    case BDown: menu_sel = (menu_sel + 1) % n; break;
    case BLeft: menu_change(-1); break;
    case BRight: menu_change(1); break;
    case BB:
    case BStart: menu_open = false; break;
    case BA:
        switch (menu_items[menu_sel]) {
        case MResume: menu_open = false; break;
        case MControls: help_open = true; break;
        case MRescan:
            rescan();
            menu_open = false;
            break;
        case MUpdate: update_check(); break;
        case MSendLogs: send_logs(); break;
        case MLogServer: prompt_begin(); break;
        case MLeaveGame: leave_game(); break;
        case MExit: quit = true; break;
        default: menu_change(0); break;
        }
        break;
    default: break;
    }
}

// A button went down (b: with Confirm / Back already as the player set them)
void press(int b)
{
    if (b <= BRight && !ptr_mode) ptr_on = false;
    if (notice.open) {
        if (job != Job::None) return;               // (working: nothing to press yet)
        if (b == BA || b == BB || b == BStart) {
            void (*go)() = b == BA ? notice.on_yes : nullptr;
            notice.open = false;
            if (go) go();
        }
        return;
    }
    if (prompt_open) {
        if (b == BA) prompt_key(keys[key_sel].ch);
        else if (b == BB) prompt_key('\b');
        else if (b == BY || b == BStart) prompt_key('\n');
        else if (b <= BRight) keys_move(b == BLeft ? -1 : b == BRight ? 1 : 0, b == BUp ? -1 : b == BDown ? 1 : 0);
        return;
    }
    if (menu_open) {
        menu_press(b);
        return;
    }
    if (b == BStart) {
        ptr_mode = false;
        menu_build();
        menu_open = true;
        help_open = false;
        return;
    }
    if (screen == Screen::Message) {
        if (b == BA || b == BB) screen = playing ? Screen::Play : msg_back == Screen::Play ? Screen::Games : msg_back;
        return;
    }
    if (screen == Screen::Games) {
        const int n = static_cast<int>(games_found.size());
        if (b == BA && n == 0) rescan();
        else if (b == BA) start_game();
        else if ((b == BUp || b == BLeft || b == BL1) && n) game_sel = (game_sel + n - 1) % n;
        else if ((b == BDown || b == BRight || b == BR1) && n) game_sel = (game_sel + 1) % n;
        return;
    }
    // ---- in the game
    if (ptr_mode) {
        if (b == BA) confirm();
        else if (b == BB) ptr_mode = ptr_on = false;
        after_input();
        return;
    }
    if (!keys.empty() && !ptr_on) {
        switch (b) {
        case BA: play::input_key(keys[key_sel].ch, cv); break;
        case BB: play::input_key('\b', cv); break;
        case BX: if (keys_for == play::Input::Text) play::input_key(' ', cv); break;
        case BY: play::input_key('\n', cv); break;
        default: if (b <= BRight) game_dir(b); break;
        }
        after_input();
        return;
    }
    const bool fight = play::fight_pad();
    switch (b) {
    case BUp:
    case BDown:
    case BLeft:
    case BRight: game_dir(b); break;
    case BA: confirm(); break;
    case BB: {
        // Back: out of a question or screen (the engine's Esc); else the
        // menu line's Exit word
        int x, y;
        if (!play::back(cv) && play::exit_point(&x, &y)) canvas_tap(x, y);
        break;
    }
    case BX: play::act(play::Act::Area, cv); break;
    case BY: panel::next(); break;
    case BL1:
        if (fight) play::fight_dir(7, cv);
        else play::nav(play::Nav::Left, cv);
        break;
    case BR1:
        if (fight) play::fight_dir(1, cv);
        else play::nav(play::Nav::Right, cv);
        break;
    // (Tom, 2026-11-14: with L1 / R1 the two highlights - the menu's word
    // and the list's line - are both at hand while the D-pad walks)
    case BL2:
        if (fight) play::fight_dir(5, cv);
        else play::nav(play::Nav::Up, cv);
        break;
    case BR2:
        if (fight) play::fight_dir(3, cv);
        else play::nav(play::Nav::Down, cv);
        break;
    default: break;
    }
    after_input();
}

// A physical button -> the player's Confirm / Back
int mapped(int b)
{
    if (cfg.swap_ab && b == BA) return BB;
    if (cfg.swap_ab && b == BB) return BA;
    return b;
}

void button(int b, bool down)
{
    b = mapped(b);
    if (b == BSelect) {
        // Pointer Mode on / off as Select comes up again (with nothing
        // pressed meanwhile)
        if (down) {
            select_alone = true;
        } else if (select_alone && playing && screen == Screen::Play && !menu_open) {
            ptr_mode = !ptr_mode;
            ptr_on = ptr_mode;
            ptr_seen = SDL_GetTicks();
        }
        return;
    }
    if (down) select_alone = false;
    if (b <= BRight) {
        held[b] = down;
        if (!down) {
            if (last_dir == b) last_dir = -1;
            return;
        }
        last_dir = b;
        fired_once = false;
        if (held[0] + held[1] + held[2] + held[3] == 1) ptr_held_since = SDL_GetTicks();
        // In a fight two directions together are a diagonal: a moment for
        // the second one to come down before the step is taken
        const bool wait = playing && screen == Screen::Play && !menu_open && keys.empty() && play::fight_pad();
        next_fire = SDL_GetTicks() + (wait ? 70 : 0);
        return;
    }
    if (down) press(b);
}

void dpad_tick(Uint32 now)
{
    if (ptr_mode && screen == Screen::Play && !menu_open) return;     // (the pointer's: pointer_tick)
    if (last_dir < 0 || !held[last_dir]) {
        // (another direction may still be down)
        last_dir = -1;
        for (int i = 0; i < 4; ++i)
            if (held[i]) last_dir = i;
        if (last_dir < 0) return;
    }
    if (static_cast<Sint32>(now - next_fire) < 0) return;
    press(last_dir);
    next_fire = now + (fired_once ? 110 : 380);
    fired_once = true;
}

// ---- The pointer -----------------------------------------------------------
void pointer_tap(int cx, int cy)
{
    if (menu_open || screen != Screen::Play || !playing) return;
    ptr_x = static_cast<float>(cx);
    ptr_y = static_cast<float>(cy);
    if (!keys.empty()) {
        // (a touch / click on the on-screen keyboard's keys)
        for (size_t i = 0; i < keys.size(); ++i) {
            const Key& k = keys[i];
            if (cx >= k.col * 8 && cx < (k.col + k.w) * 8 && cy >= k.row * 8 - 4 && cy < k.row * 8 + 12) {
                key_sel = static_cast<int>(i);
                play::input_key(k.ch, cv);
                after_input();
                return;
            }
        }
    }
    if (journal_due()) return;
    canvas_tap(cx, cy);
}

void pointer_tick(float dt)
{
    if (menu_open || screen != Screen::Play) {
        ptr_mode = false;
        return;
    }
    if (ptr_mode) {
        // The D-pad: slow at first (a menu word is 8 pixels tall), faster held
        const float v = SDL_GetTicks() - ptr_held_since < 300 ? 35.0f : 130.0f;
        ptr_x += (held[BRight] - held[BLeft]) * v * dt;
        ptr_y += (held[BDown] - held[BUp]) * v * dt;
    }
    float x = 0, y = 0;
    if (pad) {
    const SDL_GameControllerAxis ax[4] = {SDL_CONTROLLER_AXIS_LEFTX, SDL_CONTROLLER_AXIS_LEFTY,
                                          SDL_CONTROLLER_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTY};
    for (int i = 0; i < 4; ++i) {
        float v = SDL_GameControllerGetAxis(pad, ax[i]) / 32767.0f;
        if (v > -0.25f && v < 0.25f) v = 0;
        if (i & 1) y += v;
        else x += v;
    }
    }
    if (x == 0 && y == 0 && !ptr_mode) return;
    const float speed = 150.0f;         // (canvas pixels a second at full tilt)
    ptr_x += x * (x < 0 ? -x : x) * speed * dt;
    ptr_y += y * (y < 0 ? -y : y) * speed * dt;
    if (ptr_x < 0) ptr_x = 0;
    if (ptr_y < 0) ptr_y = 0;
    if (ptr_x > video::kW - 1) ptr_x = video::kW - 1;
    if (ptr_y > video::kH - 1) ptr_y = video::kH - 1;
    ptr_on = true;
    ptr_seen = SDL_GetTicks();
}

// ---- SDL events ------------------------------------------------------------
int pad_button(Uint8 b)
{
    switch (b) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP: return BUp;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return BDown;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return BLeft;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return BRight;
    case SDL_CONTROLLER_BUTTON_A: return BA;
    case SDL_CONTROLLER_BUTTON_B: return BB;
    case SDL_CONTROLLER_BUTTON_X: return BX;
    case SDL_CONTROLLER_BUTTON_Y: return BY;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return BL1;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return BR1;
    case SDL_CONTROLLER_BUTTON_START: return BStart;
    case SDL_CONTROLLER_BUTTON_BACK: return BSelect;
    default: return -1;
    }
}

// A PC's keyboard (and a handheld's, when its firmware sends keys)
int key_button(SDL_Keycode k)
{
    switch (k) {
    case SDLK_UP: return BUp;
    case SDLK_DOWN: return BDown;
    case SDLK_LEFT: return BLeft;
    case SDLK_RIGHT: return BRight;
    case SDLK_RETURN:
    case SDLK_KP_ENTER: return BA;
    case SDLK_ESCAPE: return BB;
    case SDLK_F1:
    case SDLK_TAB: return BStart;
    case SDLK_PAGEUP: return BL1;
    case SDLK_PAGEDOWN: return BR1;
    case SDLK_HOME: return BL2;
    case SDLK_END: return BR2;
    case SDLK_F2: return BX;
    case SDLK_F3: return BY;
    case SDLK_F4: return BSelect;
    default: return -1;
    }
}

void open_pad()
{
    if (pad) return;
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
        if (SDL_IsGameController(i)) {
            pad = SDL_GameControllerOpen(i);
            if (pad) {
                plat::log("[port] controller: %s\n", SDL_GameControllerName(pad));
                return;
            }
        }
}

void handle(const SDL_Event& e)
{
    switch (e.type) {
    case SDL_QUIT: quit = true; break;
    case SDL_WINDOWEVENT:
        if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) video::resized();
        break;
    case SDL_CONTROLLERDEVICEADDED: open_pad(); break;
    case SDL_CONTROLLERDEVICEREMOVED:
        if (pad && e.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad))) {
            SDL_GameControllerClose(pad);
            pad = nullptr;
            memset(held, 0, sizeof held);
            open_pad();
        }
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP: {
        const int b = pad_button(e.cbutton.button);
        if (b >= 0) button(b, e.type == SDL_CONTROLLERBUTTONDOWN);
        break;
    }
    case SDL_CONTROLLERAXISMOTION:
        if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
            const int t = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
            const bool down = e.caxis.value > (trig[t] ? 8000 : 16000);
            if (down != trig[t]) {
                trig[t] = down;
                button(t ? BR2 : BL2, down);
            }
        }
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
        const bool down = e.type == SDL_KEYDOWN;
        if (down && e.key.repeat) break;
        // Typing on a real keyboard while the game asks for text
        if (down && playing && screen == Screen::Play && !menu_open && play::input() != play::Input::None) {
            const SDL_Keycode k = e.key.keysym.sym;
            char ch = 0;
            if (k == SDLK_BACKSPACE) ch = '\b';
            else if (k == SDLK_RETURN || k == SDLK_KP_ENTER) ch = '\n';
            else if (k >= SDLK_a && k <= SDLK_z) ch = static_cast<char>('A' + (k - SDLK_a));
            else if (k >= SDLK_0 && k <= SDLK_9) ch = static_cast<char>('0' + (k - SDLK_0));
            else if (k == SDLK_SPACE || k == SDLK_MINUS || k == SDLK_PERIOD || k == SDLK_COMMA || k == SDLK_QUOTE)
                ch = static_cast<char>(k);
            if (ch) {
                play::input_key(ch, cv);
                after_input();
                break;
            }
        }
        const int b = key_button(e.key.keysym.sym);
        if (b >= 0) button(b, down);
        break;
    }
    case SDL_MOUSEBUTTONDOWN: {
        int cx, cy;
        if (e.button.button == SDL_BUTTON_LEFT && video::to_canvas(e.button.x, e.button.y, cx, cy)) {
            ptr_on = e.button.which != SDL_TOUCH_MOUSEID;
            ptr_seen = SDL_GetTicks();
            pointer_tap(cx, cy);
        }
        break;
    }
    case SDL_MOUSEMOTION: {
        int cx, cy;
        // (a real move only: some drivers report a pointer at 0, 0 as they start)
        if (e.motion.which != SDL_TOUCH_MOUSEID && (e.motion.xrel || e.motion.yrel) && (e.motion.x || e.motion.y) &&
            video::to_canvas(e.motion.x, e.motion.y, cx, cy)) {
            ptr_x = static_cast<float>(cx);
            ptr_y = static_cast<float>(cy);
            ptr_on = true;
            ptr_seen = SDL_GetTicks();
        }
        break;
    }
    default: break;
    }
}

// ---- A script of inputs (checks without a person: --script "a,wait:500,
// shot:/tmp/x.bmp,quit"; names as kBtnNames, tap:x:y, type:TEXT) ----------
std::vector<std::string> script;
size_t                   script_at = 0;
Uint32                   script_next = 0;

void script_tick(Uint32 now)
{
    if (script_at >= script.size() || static_cast<Sint32>(now - script_next) < 0) return;
    if (job != Job::None) return;                   // (a network job runs: the script waits for it)
    const std::string& t = script[script_at++];
    script_next = now + 150;
    if (t.compare(0, 5, "wait:") == 0) {
        script_next = now + static_cast<Uint32>(atoi(t.c_str() + 5));
    } else if (t.compare(0, 5, "shot:") == 0) {
        draw_frame();
        if (!video::save_bmp(t.c_str() + 5)) plat::log("[script] couldn't write %s\n", t.c_str() + 5);
    } else if (t.compare(0, 4, "tap:") == 0) {
        int x = 0, y = 0;
        if (sscanf(t.c_str() + 4, "%d:%d", &x, &y) == 2) pointer_tap(x, y);
    } else if (t.compare(0, 5, "type:") == 0) {
        for (const char* p = t.c_str() + 5; *p; ++p) {
            if (prompt_open) prompt_key(*p);
            else if (playing) play::input_key(*p == '_' ? ' ' : *p, cv);
        }
    } else if (t.compare(0, 5, "menu:") == 0) {
        // (menu:Send Logs - the open menu's line that starts so, '_' for a space)
        std::string want = t.substr(5);
        for (char& ch : want)
            if (ch == '_') ch = ' ';
        bool found = false;
        for (size_t i = 0; menu_open && i < menu_items.size(); ++i) {
            char label[40];
            menu_label(menu_items[i], label, sizeof label);
            if (strncmp(label, want.c_str(), want.size()) == 0) {
                menu_sel = static_cast<int>(i);
                found = true;
            }
        }
        if (!found) plat::log("[script] no menu line '%s'\n", want.c_str());
    } else if (t == "quit") {
        quit = true;
    } else {
        bool found = false;
        for (int b = 0; b < BCount; ++b)
            if (t == kBtnNames[b]) {
                if (b == BSelect) {
                    button(b, true);
                    button(b, false);
                } else {
                    select_alone = false;
                    press(mapped(b));
                }
                found = true;
            }
        if (t.compare(0, 5, "hold:") == 0 || t.compare(0, 4, "let:") == 0) {
            // (hold:right / let:right - a D-pad direction kept down)
            const bool down = t[0] == 'h';
            const char* name = t.c_str() + (down ? 5 : 4);
            for (int b = 0; b <= BRight; ++b)
                if (!strcmp(name, kBtnNames[b])) {
                    button(b, down);
                    found = true;
                }
        }
        if (!found) plat::log("[script] unknown step '%s'\n", t.c_str());
    }
    if (notice.open) plat::log("[script] %-12s notice: %s / %s\n", t.c_str(), notice.title.c_str(), notice.body.c_str());
    if (playing)
        plat::log("[script] %-12s walking=%d fight=%d input=%d\n", t.c_str(), play::walking() ? 1 : 0,
                  play::fighting() ? 1 : 0, static_cast<int>(play::input()));
}

void usage()
{
    printf("Gold Box Game Engine for PortMaster %s\n"
           "  --games <dir>    the folder holding the player's game folders (default: games)\n"
           "  --conf <dir>     where settings are kept (default: conf)\n"
           "  --tools <dir>    the port's helper scripts (default: tools)\n"
           "  --window <WxH>   a window instead of the whole screen\n"
           "  --game <n>       start game n of the list at once (from 1)\n"
           "  --script <steps> inputs to play back (checks)\n",
           PM_VERSION);
}

} // namespace

int main(int argc, char** argv)
{
    int win_w = 0, win_h = 0, auto_game = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        const char* v = i + 1 < argc ? argv[i + 1] : nullptr;
        if (a == "--games" && v) pm::games_root() = argv[++i];
        else if (a == "--conf" && v) conf_dir = argv[++i];
        else if (a == "--tools" && v) tools_dir = argv[++i];
        else if (a == "--window" && v) sscanf(argv[++i], "%dx%d", &win_w, &win_h);
        else if (a == "--game" && v) auto_game = atoi(argv[++i]);
        else if (a == "--script" && v) {
            std::string s = argv[++i];
            size_t p = 0;
            while (p <= s.size()) {
                const size_t c = s.find(',', p);
                const std::string t = s.substr(p, c == std::string::npos ? std::string::npos : c - p);
                if (!t.empty()) script.push_back(t);
                if (c == std::string::npos) break;
                p = c + 1;
            }
        } else {
            usage();
            return a == "--help" ? 0 : 2;
        }
    }
    // (strip a trailing slash: paths are joined with one)
    while (pm::games_root().size() > 1 && pm::games_root().back() == '/') pm::games_root().pop_back();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL: %s\n", SDL_GetError());
        return 1;
    }
    plat::log("[port] Gold Box Game Engine for PortMaster %s; games in %s\n", PM_VERSION, pm::games_root().c_str());
    settings_load(conf_dir, cfg);
    if (!video::begin(win_w, win_h)) {
        fprintf(stderr, "SDL video: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    video::set_fit(static_cast<video::Fit>(cfg.fit));
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) pm::audio_begin();
    else plat::log("[port] no sound: %s\n", SDL_GetError());
    open_pad();
    rescan();
    plat::log("[port] %d game folder(s) found\n", static_cast<int>(games_found.size()));
    for (const auto& g : games_found)
        plat::log("[port]   %s: %s (%d files in games/%s)%s\n", g.folder.c_str(), games::title(g.game), g.files,
                  g.data_dir.c_str(), play::available(g.game) ? "" : " - not playable");
    if (auto_game >= 1 && auto_game <= static_cast<int>(games_found.size())) {
        game_sel = auto_game - 1;
        start_game();
    }

    Uint32 last = SDL_GetTicks();
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) handle(e);
        const Uint32 now = SDL_GetTicks();
        const float dt = (now - last) / 1000.0f;
        last = now;
        dpad_tick(now);
        pointer_tick(dt > 0.1f ? 0.1f : dt);
        if (ptr_on && now - ptr_seen > 6000) ptr_on = false;        // (left alone: out of the way)
        script_tick(now);
        job_tick();
        if (restart_wanted) quit = true;
        if (playing && screen == Screen::Play && !menu_open) {
            play::tick(plat::now_ms(), cv);
            after_input();
        }
        if (playing) {
            sync_palette(false);
            const play::Input in = play::input();
            if (in != keys_for && !prompt_open) keys_build(in);
            int y0, y1;
            play::take_dirty(y0, y1);       // (every frame shows the whole canvas)
        }
        draw_frame();
        // (vsync paces a real screen; a software or dummy one needs a rest)
        const Uint32 took = SDL_GetTicks() - now;
        if (took < 8) SDL_Delay(8 - took);
    }
    leave_game();
    pm::audio_end();
    video::end();
    SDL_Quit();
    return 0;
}
