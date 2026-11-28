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

#include <cctype>
#include <algorithm>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "engine/geo.h"
#include "engine/journal.h"
#include "game/music_make.h"
#include "engine/profile.h"
#include "engine/quickacts.h"
#include "engine/rules.h"
#include "game/play.h"
#include "game/tidy.h"
#include "platform/audio.h"
#include "platform/file.h"
#include "platform/platform.h"

#include "host.h"
#include "jour.h"
#include "cluemaps.h"
#include "art.h"
#include "library.h"
#include "panel.h"
#include "settings.h"
#include "task.h"
#include "touchpad.h"

#ifdef __ANDROID__
#include <jni.h>
#endif
#include "shell.h"
#include "video.h"

#ifndef PM_VERSION
#define PM_VERSION "dev"
#endif

// --ab-reversed (the launch script, on a firmware whose A and B come out the
// other way round - Knulli; every launch): Confirm starts on SDL's B, once -
// the player's own choice in Settings stands afterwards - and the buttons'
// names on screen are the device's printed letters (confirm_name)
bool ab_reversed = false;

// The on-screen buttons (touchpad.h): Android's always; a PC's with
// --touchpad (the mouse acting as a finger), to try them out
#ifdef __ANDROID__
bool touch_ui = true;
#else
bool touch_ui = false;
#endif

namespace {

// ---- Buttons ---------------------------------------------------------------
enum Btn : uint8_t { BUp, BDown, BLeft, BRight, BA, BB, BX, BY, BL1, BR1, BL2, BR2, BStart, BSelect, BCount };
const char* const kBtnNames[BCount] = {"up", "down", "left", "right", "a", "b", "x", "y",
                                       "l1", "r1", "l2", "r2", "start", "select"};

enum class Screen : uint8_t { Games, Play, Message, Scan };

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

// The journal: the entries the game just mentioned, shown one after the
// other; the Start menu's list of those met so far
struct JRef {
    char kind;
    int  number;
};
std::vector<JRef> journal_queue;
std::string       cur_cache;            // the game's folder in games/_CYD (a card path)
bool              jlist_open = false;
int               jlist_sel = 0;
std::vector<jour::Document> jlist_docs;   // (Android: the game's PDFs, listed under its entries)

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

// ---- A touch screen (Android; a PC with --touchpad) -------------------------
// (Tom, 2026-11-23: "compatible with touch and/or a controller (via bluetooth
// or USB) or an onscreen overlay representing the controller"). Three ways,
// any mix: a controller; the on-screen buttons (touchpad.h); or touching the
// port's own screens - a tap or a swipe is the button a player would press
// there (touch_tap / touch_swipe), so nothing is touch's alone.
const char* const kTouchPad[3] = {"Auto", "Always", "Never"};
bool touch_last = true;     // the last thing used was the screen (Auto: the buttons shown)

void touch_pad_apply()
{
    if (!touch_ui) return;
    if (shell::on()) {
        // (the Android app's own pad round the game: no overlay - shell.h)
        touchpad::set_shown(false);
        return;
    }
    touchpad::set_shown(cfg.touch_pad == 1 || (cfg.touch_pad == 0 && touch_last));
}

// Controls - Button Size / Button Opacity (Tom, 2026-11-23: "give the user options, don't use fixed values")
const char* const kPadSize[4] = {"Small", "Medium", "Large", "Huge"};
void pad_style_apply()
{
    static const int kSize[4] = {75, 100, 125, 150};
    touchpad::set_style(kSize[cfg.pad_size % 4], 20 + 20 * (cfg.pad_alpha % 5));
}

// A folder as the player's phone and PC name it: "Internal storage/GoldBox"
// (Android's /storage/emulated/0), "SD card/..." (/storage/XXXX-XXXX)
std::string shown_path(const std::string& p)
{
#ifdef __ANDROID__
    const char* own = "/storage/emulated/0";
    if (p.compare(0, strlen(own), own) == 0) return "Internal storage" + p.substr(strlen(own));
    if (p.compare(0, 9, "/storage/") == 0) {
        const size_t end = p.find('/', 9);
        return "SD card" + (end == std::string::npos ? std::string() : p.substr(end));
    }
#endif
    return p;
}

// Display - Screen Rotation (Android; Tom, 2026-11-23: "an option under Display
// on the menu to force one rotation or the other"): Auto follows the device
// (and its rotation lock), Landscape / Portrait hold either way round
const char* const kRotation[3] = {"Auto", "Landscape", "Portrait"};

void rotation_apply()
{
#ifdef __ANDROID__
    // (Android's ActivityInfo: FULL_USER 13, USER_LANDSCAPE 11, USER_PORTRAIT 12)
    static const int kWant[3] = {13, 11, 12};
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, "setRequestedOrientation", "(I)V");
    if (m) env->CallVoidMethod(act, m, kWant[cfg.rotation % 3]);
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
#endif
}

// A phone's camera cutout / notch (Tom's Pixel 10a, 2026-11-23): its safe
// margins from the activity (GoldBoxActivity.cutoutInsets), asked about once
// a second - they turn with the device and can come late - nothing drawn in
// them (video::set_safe)
void safe_check(Uint32 now)
{
#ifdef __ANDROID__
    static Uint32 next = 0;
    if (static_cast<Sint32>(now - next) < 0) return;
    next = now + 1000;
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, "cutoutInsets", "()[I");
    if (m) {
        jintArray a = static_cast<jintArray>(env->CallObjectMethod(act, m));
        if (a && env->GetArrayLength(a) == 4) {
            jint v[4];
            env->GetIntArrayRegion(a, 0, 4, v);
            video::set_safe(v[0], v[1], v[2], v[3]);
        }
        if (a) env->DeleteLocalRef(a);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
#else
    (void)now;
#endif
}

#ifdef __ANDROID__
// The Journal list's PDFs: the app's own viewer (Java, GoldBoxActivity.openPdf - a page at a time, a book's
// spread split in two)
void android_open_pdf(const std::string& path, const std::string& title)
{
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, "openPdf", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (m) {
        jstring p = env->NewStringUTF(path.c_str()), t = env->NewStringUTF(title.c_str());
        env->CallVoidMethod(act, m, p, t);
        env->DeleteLocalRef(p);
        env->DeleteLocalRef(t);
    } else {
        env->ExceptionClear();
    }
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
}

// Report a Problem (Tom, 2026-11-28 - the Android app's alone; the handhelds keep Send Logs): the app's dialog
// (Java, GoldBoxActivity.reportProblem -> ReportDialog) asks what happened, then the player's own email app opens
// with the answers and one zip - the log, settings, the engine's notes, the device's details and, if ticked, a save
// of the game as it was (play::snapshot, made here first). `info`: "key\tvalue" lines (android_report)
// Device Info's lines of the device (Java, GoldBoxActivity.deviceInfo - Report's own, as DEVICE.TXT): "name\tvalue"
std::string android_device_info()
{
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return "";
    std::string out;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, "deviceInfo", "()Ljava/lang/String;");
    if (m) {
        jstring r = static_cast<jstring>(env->CallObjectMethod(act, m));
        if (r && !env->ExceptionCheck()) {
            const char* p = env->GetStringUTFChars(r, nullptr);
            if (p) {
                out = p;
                env->ReleaseStringUTFChars(r, p);
            }
        }
        if (r) env->DeleteLocalRef(r);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
    return out;
}

void android_report(int game);     // (below, beside android_op: it reads the screens' state)

void android_report_call(const std::string& info)
{
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, "reportProblem", "(Ljava/lang/String;)V");
    if (m) {
        jstring t = env->NewStringUTF(info.c_str());
        env->CallVoidMethod(act, m, t);
        env->DeleteLocalRef(t);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
}

// Start - System - Games Folder: the app's folder screen (Java,
// GoldBoxActivity.chooseFolder) opens; this program then ends
void android_choose_folder()
{
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, "chooseFolder", "()V");
    if (m) env->CallVoidMethod(act, m);
    else env->ExceptionClear();
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
}
#endif

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

// The game's title screens are up (its pictures and the Play / Demo line -
// not its demo, nor its party menu): no Companion beside them (Tom)
bool in_title() { return playing && play::titles(); }

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

// The letters printed on the device's buttons: on a firmware that reports A
// and B the other way round (--ab-reversed, Knulli) SDL's "B" is the button
// marked A - so the names follow the marks, not SDL's (Tom, 2026-11-17, TrimUI
// Brick Pro on Knulli: the Quick page said "B" for the button marked A)
const char* confirm_name() { return cfg.swap_ab != ab_reversed ? "B" : "A"; }
const char* back_name() { return cfg.swap_ab != ab_reversed ? "A" : "B"; }

// The game list: a game a page - its box art (the player's own, from the
// Steam release's Launcher folder in games/) under the header, its name
// across the foot; Left / Right turn the pages, round. As the CYD's.
constexpr int kHeaderH = 20;

// How the game list wants a game's picture (the scan makes it so): the
// page under the header at twice the canvas's sharpness; a logo above the
// name's one or two lines
art::Want art_want(const library::GameDir& g)
{
    const char* title = g.game == games::Game::Unknown ? g.folder.c_str() : games::title(g.game);
    const bool two = strlen(title) > 19;
    return art::Want{(video::kH - kHeaderH) * 2, ((two ? 144 : 162) - 4 - kHeaderH) * 2};
}

void draw_games()
{
    // (the whole panel: m more columns each side of the 320 on a wide one)
    const int m = video::margin(), left = -m, wide = video::kW + 2 * m, right = video::kW + m;
    video::clear(video::kClear);
    video::set_backdrop(nullptr);
    video::fill(left, 0, wide, kHeaderH, video::kNavy);
    video::text_at(left + 8, 6, "Gold Box Game Engine", video::kGold);
    video::text_at(right - 8 - static_cast<int>(strlen("v" PM_VERSION)) * 8, 6, "v" PM_VERSION, video::kMuted);
    if (games_found.empty()) {
        video::fill(left, kHeaderH, wide, video::kH - kHeaderH, video::kBlack);
        int row = 4;
        centred(row, have_root ? "No games found" : "No games folder", video::kWarn);
        row = wrapped(2, row + 2, 36,
#ifdef __ANDROID__
                      "This app plays SSI's Gold Box games from your own copies. Copy each game's whole folder "
                      "(from GOG or Steam) into its games folder:",
#else
                      "This port plays SSI's Gold Box games from your own copies. Copy each game's whole folder "
                      "(from GOG or Steam) into the port's games folder:",
#endif
                      video::kCream);
#ifdef __ANDROID__
        // (the folder the app was given, as the phone's file manager and a PC
        // over USB name it - "Internal storage/GoldBox/games/")
        {
            std::string at = shown_path(pm::games_root()) + "/";
            ++row;
            while (!at.empty()) {
                size_t cut = at.size();
                if (cut > 36) {
                    cut = at.rfind('/', 35);
                    cut = cut == std::string::npos || cut == 0 ? 36 : cut + 1;
                }
                centred(row++, at.substr(0, cut).c_str(), video::kGold);
                at.erase(0, cut);
            }
            row = wrapped(2, row + 1, 36, "for example games/Curse of the Azure Bonds/ - then tap the screen (Look Again).",
                          video::kCream);
            wrapped(2, row + 1, 36, "Another folder: Start - System - Games Folder.", video::kMuted);
        }
#else
        centred(row + 1, "ports/goldbox/games/", video::kGold);
        wrapped(2, row + 3, 36, "for example games/Curse of the Azure Bonds/ - then start the port again.",
                video::kCream);
#endif
        char line[41];
        snprintf(line, sizeof line, "Start: Menu   %s: Look Again", confirm_name());
        centred(23, line, video::kMuted);
        return;
    }
    const int n = static_cast<int>(games_found.size());
    const library::GameDir& g = games_found[game_sel];
    const bool can = play::available(g.game);
    // The name: big, across the foot (two lines when it's long)
    const char* title = g.game == games::Game::Unknown ? g.folder.c_str() : games::title(g.game);
    std::string l1 = title, l2;
    if (l1.size() > 19) {
        size_t cut = l1.rfind(' ', 19);
        if (cut == std::string::npos) cut = 19;
        l2 = l1.substr(cut + (l1[cut] == ' ' ? 1 : 0), 19);
        l1 = l1.substr(0, cut);
    }
    // Its picture, as the scan made and saved it (box art, else the game's
    // own logo above the name) - only read here, never decoded
    const uint32_t* picture = art::load(g, library::cache_dir(g), art_want(g));
    if (picture) video::set_backdrop(picture, kHeaderH);
    else video::fill(left, kHeaderH, wide, video::kH - kHeaderH, video::kBlack);
    const int ty = picture ? (l2.empty() ? 162 : 144) : (l2.empty() ? 92 : 82);
    video::text_big((video::kW - static_cast<int>(l1.size()) * 16) / 2, ty, l1.c_str(), can ? video::kCream : video::kMuted);
    if (!l2.empty())
        video::text_big((video::kW - static_cast<int>(l2.size()) * 16) / 2, ty + 18, l2.c_str(), can ? video::kCream : video::kMuted);
    if (!can) {
        video::fill(104, ty - 14, 112, 11, video::kBlack);
        video::text_at(112, ty - 12, "Not playable", video::kWarn);
    }
    // The pages' clue: a small arrowhead at each side (no "2 of 9" counter - Tom, 2026-11-14)
    if (n > 1) {
        for (int k = 0; k < 7; ++k) {
            video::fill(left + 4 + k, 106 - k, 1, 2 * k + 1, video::kBlack);
            video::fill(right - 5 - k, 106 - k, 1, 2 * k + 1, video::kBlack);
        }
        for (int k = 0; k < 5; ++k) {
            video::fill(left + 6 + k, 106 - k, 1, 2 * k + 1, video::kCream);
            video::fill(right - 7 - k, 106 - k, 1, 2 * k + 1, video::kCream);
        }
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

void        draw_jlist();
void        draw_scan();
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

// Send Logs (Tom, 2026-11-14): the logs go by the internet to the developer
// (tools/sendlogs.sh -> the log relay -> email). The first time it asks for
// a reply address - the player may say No Thanks - which goes with the logs
// so the developer can write back; remembered, changed in Settings.
void prompt_begin(bool then_send);

void send_logs_now()
{
    if (!tools_ready("sendlogs.sh")) return;
    menu_open = false;
    fflush(stdout);
    run_job(Job::Logs,
            tool("sendlogs.sh") + " " + task::quoted(port_dir()) + " " PM_VERSION " " + task::quoted(cfg.reply_email),
            "Send Logs", "Sending the logs to the developer...");
}

void send_logs()
{
    if (cfg.reply_email.empty() && !cfg.reply_asked) {
        prompt_begin(true);
        return;
    }
    send_logs_now();
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
    if (was == Job::Logs) {
        const std::string sent = task::field(out, "SENT");
        std::string body = "The logs were sent to the developer. Thank you!";
        if (sent.find("thanks") != std::string::npos)
            body += " A note is on its way to " + cfg.reply_email + " (look in your spam folder too) - a reply saying what you "
                    "were doing helps a lot.";
        show_notice("Send Logs", body);
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
// The CYD's menus by Tom's tree (2026-11-17; engine 0.209.0, its CLAUDE.md
// "The menus"), by buttons: Start opens the top page - in a game Display,
// Audio, Game Play, Journal, Cheats, Quick Actions, Controls, System, Exit
// Game; at the game list the same less the game-only ones. Game Play:
// Original Games, Enhancements, AD&D 1E Rules, Game Fixes (each fix and each
// rule its own switch - names and words from the engine's rules::fix_info /
// adnd_info, so a new one shows here by itself). Every line says what it
// does at the page's foot. The port's own: Controls, Quit, and System's
// Check for Updates / Reply Address.
enum MenuItem : uint8_t {
    MResume,
    // the pages
    MDisplayPg, MAudioPg, MPlayPg, MJournal, MCheats, MQuickPg, MControlsPg, MSystemPg, MLeaveGame, MExit,
    MDisplay, MClueDark, MRotation, MChooser, MJFont,  // Display (MRotation, MChooser, MJFont: Android's)
    MSound, MMusic, MVolume,                // Audio
    MOriginal, MEnhancePg, MRulesPg, MFixGamesPg,       // Game Play
    MFont, MClueMaps, MRolls, MSaveInfo, MAutoJoin, MKeepMem, MCampFix, MCoins, MOil, MAddInfo, MMoreGuys, MPortrait, MGallery,    // Enhancements
    MRule,                                  // AD&D 1E Rules (arg: rules::adnd_info's row)
    MFixGame,                               // Game Fixes (arg: games::Game; 0 Every Game)
    MFix,                                   // a game's fixes (arg: rules::Fix)
    MHealer, MMerchant, MSanctuary, MTrainer, MMaxHp, MNoRace,   // Cheats
    MQuickOn, MQuick,                       // Quick Actions
    MControls, MPadMoves, MSwap, MTouchPad, MPadSize, MPadAlpha,   // Controls (the last three: a touch screen's)
    MRescan, MClean, MDevice, MUpdate, MSendLogs, MReplyAddr, MFolder, MAbout,   // System (MFolder, MAbout: Android's)
    MBack
};
enum class Page : uint8_t { Top, Display, Audio, Play, Enhance, Rules, FixGames, Fixes, Cheats, Quick, Controls, System };
struct MItem {
    MenuItem m;
    int      arg;
};
std::vector<MItem> menu_items;
Page               menu_page = Page::Top;
int                menu_top = 0;        // the first line shown
int                fix_game = 0;        // the Fixes page's game (games::Game; 0: Every Game)
bool               device_open = false; // Device Info is up
bool               about_open = false;  // (with device_open) About instead - the Android app's
std::string        app_version;         // the Android app's (--app-version)

Page page_parent(Page p)
{
    switch (p) {
    case Page::Enhance:
    case Page::Rules:
    case Page::FixGames: return Page::Play;
    case Page::Fixes: return Page::FixGames;
    default: return Page::Top;
    }
}

// The page a line opens (Top: none)
Page page_of(MenuItem m)
{
    switch (m) {
    case MDisplayPg: return Page::Display;
    case MAudioPg: return Page::Audio;
    case MPlayPg: return Page::Play;
    case MCheats: return Page::Cheats;
    case MQuickPg: return Page::Quick;
    case MControlsPg: return Page::Controls;
    case MSystemPg: return Page::System;
    case MEnhancePg: return Page::Enhance;
    case MRulesPg: return Page::Rules;
    case MFixGamesPg: return Page::FixGames;
    case MFixGame: return Page::Fixes;
    default: return Page::Top;
    }
}

const char* page_title(Page p)
{
    switch (p) {
    case Page::Display: return "Display";
    case Page::Audio: return "Audio";
    case Page::Play: return "Game Play";
    case Page::Enhance: return "Enhancements";
    case Page::Rules: return "AD&D 1E Rules";
    case Page::FixGames: return "Game Fixes";
    case Page::Cheats: return "Cheats";
    case Page::Quick: return "Quick Actions";
    case Page::Controls: return "Controls";
    case Page::System: return "System";
    default: return "Menu";
    }
}

// The Game Fixes' games, in the series' order (0: Every Game) - the CYD's
constexpr games::Game kFixGameList[] = {
    games::Game::Unknown, games::Game::PoolOfRadiance, games::Game::CurseOfTheAzureBonds,
    games::Game::SecretOfTheSilverBlades, games::Game::PoolsOfDarkness, games::Game::GatewayToTheSavageFrontier,
    games::Game::TreasuresOfTheSavageFrontier, games::Game::ChampionsOfKrynn, games::Game::DeathKnightsOfKrynn,
    games::Game::DarkQueenOfKrynn};
const char* fix_game_name(int g)
{
#ifdef __ANDROID__
    // (the app's Game Fixes: the games' full names, wrapped - Tom, 2026-11-28)
    if (g) return games::title(static_cast<games::Game>(g));
#endif
    switch (static_cast<games::Game>(g)) {
    case games::Game::PoolOfRadiance: return "PoolRad";
    case games::Game::CurseOfTheAzureBonds: return "Curse";
    case games::Game::SecretOfTheSilverBlades: return "Secret";
    case games::Game::PoolsOfDarkness: return "Darkness";
    case games::Game::GatewayToTheSavageFrontier: return "Gateway";
    case games::Game::TreasuresOfTheSavageFrontier: return "Treasures";
    case games::Game::ChampionsOfKrynn: return "Champions";
    case games::Game::DeathKnightsOfKrynn: return "Knights";
    case games::Game::DarkQueenOfKrynn: return "Queen";
    default: return "Every Game";
    }
}
uint16_t every_game()
{
    uint16_t all = 0;
    for (games::Game g : kFixGameList)
        if (g != games::Game::Unknown) all = static_cast<uint16_t>(all | 1u << static_cast<int>(g));
    return all;
}
bool fix_on_page(const rules::FixInfo& f, int g)
{
    if (g == 0) return f.games == every_game();
    return f.games != every_game() && (f.games >> g & 1);
}

// The Enhancement a line switches (0: not one)
uint16_t comfort_bit(MenuItem m)
{
    switch (m) {
    case MRolls: return kOffRolls;
    case MAutoJoin: return kOffAutoJoin;
    case MSaveInfo: return kOffSaveInfo;
    case MKeepMem: return kOffKeepMem;
    case MCoins: return kOffCoins;
    case MOil: return kOffOil;
    case MAddInfo: return kOffAddInfo;
    case MCampFix: return kOffCampFix;
    case MGallery: return kOffGallery;
    case MPortrait: return kOffPortrait;
    case MMoreGuys: return kOffMoreGuys;
    case MQuickOn: return kOffQuick;
    default: return 0;
    }
}

void menu_build()
{
    menu_items.clear();
    auto add = [](MenuItem m, int arg = 0) { menu_items.push_back({m, arg}); };
    switch (menu_page) {
    case Page::Top:
        if (playing) add(MResume);
        add(MDisplayPg);
        add(MAudioPg);
        add(MPlayPg);
        if (playing) add(MJournal);
        add(MCheats);
        if (playing) add(MQuick);
        add(MControlsPg);
        add(MSystemPg);
#ifndef __ANDROID__
        // (the port's own, Tom 2026-11-17: on the main menu, "not buried" - a handheld's; the Android app has them
        // under System, Tom 2026-11-25)
        add(MRescan);
        add(MClean);
        add(MUpdate);
#endif
        if (playing) add(MLeaveGame);
        add(MExit);
        break;
    case Page::Display:
        add(MDisplay);      // (in the Android app's drawer: 4:3, Square Pixels, Stretch - the app places the Companion)
        add(MClueDark);
#ifdef __ANDROID__
        add(MRotation);
        if (shell::on()) add(MChooser);
        if (shell::on()) add(MJFont);
#endif
        break;
    case Page::Audio:
        add(MSound);
        add(MMusic);
        add(MVolume);
        break;
    case Page::Play:
        add(MOriginal);
        add(MEnhancePg);
        add(MRulesPg);
        add(MFixGamesPg);
        break;
    case Page::Enhance:
        for (MenuItem m : {MFont, MClueMaps, MRolls, MSaveInfo, MAutoJoin, MKeepMem, MCampFix, MCoins, MOil, MAddInfo, MMoreGuys, MPortrait, MGallery}) add(m);
        break;
    case Page::Rules:
        for (int k = 0; k < rules::kAdndCount; ++k) add(MRule, k);
        break;
    case Page::FixGames:
        // (in a game: Every Game and that game's; at the game list every game's)
        add(MFixGame, 0);
        for (games::Game g : kFixGameList) {
            if (g == games::Game::Unknown) continue;
            if (playing && g != games_found[game_sel].game) continue;
            add(MFixGame, static_cast<int>(g));
        }
        break;
    case Page::Fixes:
        for (int k = 0; k < rules::kFixCount; ++k)
            if (fix_on_page(rules::fix_info(k), fix_game)) add(MFix, k);
        break;
    case Page::Cheats:
        if (playing)
            for (MenuItem m : {MHealer, MMerchant, MSanctuary, MTrainer}) add(m);
        add(MMaxHp);
        add(MNoRace);
        break;
    case Page::Controls:
        add(MControls);
        add(MPadMoves);
        add(MSwap);
        if (touch_ui) {
            // (the Android app's own pad: its size and how solid it is - shown or not is its handle's and pin's)
            // (Button Opacity: the overlay's - the app's pad has its own room, never over the game)
            if (!shell::on()) add(MTouchPad);
            add(MPadSize);
            if (!shell::on()) add(MPadAlpha);
        }
        break;
    case Page::System:
#ifdef __ANDROID__
        add(MFolder);
        add(MRescan);
        add(MClean);
#endif
        add(MDevice);
#ifdef __ANDROID__
        add(MAbout);
#endif
        add(MSendLogs);         // (the app's: Report a Problem - its dialog and the player's email; no reply address)
#ifndef __ANDROID__
        add(MReplyAddr);
#endif
        break;
    }
    if (menu_page != Page::Top) add(MBack);
    if (menu_sel >= static_cast<int>(menu_items.size())) menu_sel = 0;
}

// To a page; coming back up, the highlight is on the line that led down
void menu_go(Page p)
{
    const Page from = menu_page;
    const int from_game = fix_game;
    menu_page = p;
    menu_sel = 0;
    menu_top = 0;
    menu_build();
    if (page_parent(from) == p && from != p)
        for (size_t i = 0; i < menu_items.size(); ++i)
            if (page_of(menu_items[i].m) == from && (menu_items[i].m != MFixGame || menu_items[i].arg == from_game))
                menu_sel = static_cast<int>(i);
}

// The switches' state as the engine gets it
uint16_t adnd_on() { return static_cast<uint16_t>(rules::kAdndAll & ~cfg.rules_off); }

void menu_label(const MItem& it, char* out, size_t cap)
{
#ifdef __ANDROID__
    static const char* const kFont[] = {"Original", "Clear", "Darkness", "Clear CAPS"};     // (Tom's spelling)
#else
    static const char* const kFont[] = {"Original", "Clear", "Darkness", "Clear Caps"};
#endif
    const MenuItem m = it.m;
    if (const uint16_t bit = comfort_bit(m)) {
        const char* name = m == MRolls     ? "Show Rolls"
                         : m == MAutoJoin  ? "Auto Join Items"
                         : m == MSaveInfo  ? "Savegame Info"
                         : m == MKeepMem   ? "Keep Memorizing"
                         : m == MCoins     ? "Coin Exchange"
                         : m == MOil       ? "Flasks of Oil"
                         : m == MAddInfo   ? "Character Info"
                         : m == MCampFix   ? "Camp Fix"
                         : m == MGallery   ? "Icon Gallery"
                         : m == MPortrait  ? "Icon Portrait"
                         : m == MMoreGuys  ? "More Characters"
                                           : "Quick Actions";
        snprintf(out, cap, "%s: %s", name, cfg.comforts_off & bit ? "Off" : "On");
        return;
    }
    switch (m) {
    case MResume: snprintf(out, cap, "Back to Game"); break;
    case MDisplayPg: snprintf(out, cap, "Display..."); break;
    case MAudioPg: snprintf(out, cap, "Audio..."); break;
    case MPlayPg: snprintf(out, cap, "Game Play..."); break;
    case MJournal: snprintf(out, cap, "Journal"); break;
    case MCheats: snprintf(out, cap, "Cheats..."); break;
    case MQuickPg: snprintf(out, cap, "Quick Actions..."); break;
    case MControlsPg: snprintf(out, cap, "Controls..."); break;
    case MSystemPg: snprintf(out, cap, "System..."); break;
    case MEnhancePg: snprintf(out, cap, "Enhancements..."); break;
    case MRulesPg: snprintf(out, cap, "AD&D 1E Rules..."); break;
    case MFixGamesPg: snprintf(out, cap, "Game Fixes..."); break;
    case MOriginal: snprintf(out, cap, "Original Games: %s", cfg.original ? "On" : "Off"); break;
    case MRule: snprintf(out, cap, "%s: %s", rules::adnd_info(it.arg).name, adnd_on() & rules::adnd_info(it.arg).bit ? "On" : "Off"); break;
    case MFixGame: snprintf(out, cap, "%s...", fix_game_name(it.arg)); break;
    case MFix: snprintf(out, cap, "%s: %s", rules::fix_info(it.arg).name, cfg.fixes_off >> it.arg & 1 ? "Off" : "On"); break;
    case MQuick: snprintf(out, cap, "Quick Actions"); break;
    case MControls: snprintf(out, cap, "Button Guide"); break;
    case MDisplay:
        // (the Android app's layout: its own Companion - Right / Below read as 4:3)
        snprintf(out, cap, "Picture: %s",
                 video::fit_name(static_cast<video::Fit>(shell::on() && cfg.fit > 2 ? 0 : cfg.fit)));
        break;
    case MRotation: snprintf(out, cap, "Screen Rotation: %s", kRotation[cfg.rotation % 3]); break;
    case MChooser: {
        static const char* const kChooser[3] = {"Auto", "Tiles", "Cards"};
        snprintf(out, cap, "Game Chooser: %s", kChooser[cfg.chooser % 3]);
        break;
    }
    case MSound: snprintf(out, cap, "Game Sound: %s", sound_name(sound_shown())); break;
    case MMusic: snprintf(out, cap, "AdLib Music: %s", cfg.music ? "On" : "Off"); break;
    case MVolume: snprintf(out, cap, "Volume: %d%%", cfg.volume); break;
    case MFont: snprintf(out, cap, "Game Font: %s", kFont[cfg.font]); break;
    case MPadMoves: snprintf(out, cap, "D-Pad Walks: %s", cfg.pad_moves ? "On" : "Off"); break;
    case MSwap: snprintf(out, cap, "Confirm Button: %s", confirm_name()); break;
    case MTouchPad: snprintf(out, cap, "On-Screen Buttons: %s", kTouchPad[cfg.touch_pad % 3]); break;
    case MPadSize: snprintf(out, cap, "Button Size: %s", kPadSize[cfg.pad_size % 4]); break;
    case MPadAlpha: snprintf(out, cap, "Button Opacity: %d%%", 20 + 20 * (cfg.pad_alpha % 5)); break;
    case MFolder: snprintf(out, cap, "Games Folder..."); break;
    case MMaxHp: snprintf(out, cap, "Max Hit Points: %s", cfg.max_hp ? "On" : "Off"); break;
    case MNoRace: snprintf(out, cap, "Disable Race Level Limits: %s", cfg.no_race_limits ? "On" : "Off"); break;
    case MClueMaps: snprintf(out, cap, "Cluebook Maps: %s", cfg.clue_maps ? "On" : "Off"); break;
    case MClueDark: snprintf(out, cap, "Cluebook Map Colors: %s", cfg.clue_dark ? "Dark" : "As Printed"); break;
    case MRescan: snprintf(out, cap, "Rescan Game Files"); break;
    case MClean: snprintf(out, cap, "Clean Up Game Files"); break;
    case MDevice: snprintf(out, cap, "Device Info"); break;
    case MAbout: snprintf(out, cap, "About"); break;
    case MJFont: {
        static const char* const kFont[4] = {"Journal", "Book", "Serif", "Plain"};
        snprintf(out, cap, "Journal Font: %s", kFont[cfg.journal_font % 4]);
        break;
    }
    case MUpdate: snprintf(out, cap, "%s", update_staged.empty() ? "Check for Updates" : "Restart to Update"); break;
#ifdef __ANDROID__
    case MSendLogs: snprintf(out, cap, "Report a Problem"); break;
#else
    case MSendLogs: snprintf(out, cap, "Send Logs"); break;
#endif
    case MReplyAddr: snprintf(out, cap, "Reply Address: %.14s", cfg.reply_email.empty() ? "Not Set" : cfg.reply_email.c_str()); break;
    case MLeaveGame: snprintf(out, cap, "Exit Game"); break;
    case MExit: snprintf(out, cap, "Quit"); break;
    case MHealer: snprintf(out, cap, "Healer"); break;
    case MMerchant: snprintf(out, cap, "Merchant"); break;
    case MSanctuary: snprintf(out, cap, "Sanctuary"); break;
    case MTrainer: snprintf(out, cap, "Trainer"); break;
    case MBack: snprintf(out, cap, "Back"); break;
    default: out[0] = 0; break;
    }
}

// What the highlighted line does (the page's foot)
const char* menu_help(const MItem& it)
{
    switch (it.m) {
    case MResume: return "Close this menu.";
    case MDisplayPg: return "How the picture fills the screen.";
    case MAudioPg: return "Sound effects, music and volume.";
    case MPlayPg: return "Original Games, the Enhancements, the AD&D 1E Rules and the Game Fixes.";
    case MJournal: return "The journal entries the game has told you to read so far.";
    case MCheats: return playing ? "Healer, Merchant, Sanctuary, Trainer, Max Hit Points, Race Level Limits." : "Max Hit Points, Race Level Limits. The other cheats are here while a game is played.";
    case MQuickPg: return "What Quick may do for each member of the party in a fight.";
    case MControlsPg: return "What each button does, and the buttons' settings.";
#ifdef __ANDROID__
    case MSystemPg: return "The games folder, Rescan and Clean Up, Device Info, About, Report a Problem.";
#else
    case MSystemPg: return "Device Info, Send Logs and its reply address.";
#endif
    case MLeaveGame: return "Back to the list of games. Save in the game first.";
    case MExit: return "Leaves the port.";
    case MOriginal:
        return "On: the games as the originals play them - every Enhancement, AD&D 1E Rule and Game Fix off, the game's "
               "own font, the two cheat switches off. Any can still be turned on by itself. Off: your own choices come back.";
    case MEnhancePg: return "The engine's comforts the original games didn't have, each with its switch.";
    case MRulesPg: return "Rules of the AD&D 1st edition books the originals left out, each with its switch.";
    case MFixGamesPg: return "Mistakes in the original games' programs put right, each with its switch, by game.";
    case MFixGame: return it.arg ? "The fixes of this game's own mistakes." : "The fixes that touch every game.";
    case MRule: return rules::adnd_info(it.arg).help;
    case MFix: return rules::fix_info(it.arg).help;
    case MDisplay:
#ifdef __ANDROID__
        if (shell::on())
            return "4:3 as the games' monitors showed them, Square Pixels, or Stretch to all of the game's room.";
        return "4:3 as the games' monitors showed them (the Companion slides out on Y), 4:3 + Companion Right or "
               "Below (it stays there), Square Pixels, or Stretch to the whole screen.";
#else
        return "Original 4:3 as the games' monitors showed them, Square Pixels, or Stretch to the whole screen.";
#endif
    case MSound:
        return "The games' sound effects: Tandy or PC Speaker, or the Darkness / Gateway / Treasures recordings "
               "when that game is here.";
    case MMusic: return "The later games' own music (made from your game files when the games are found).";
    case MVolume: return "How loud. Left / Right change it.";
    case MFont: return "The game's letters: its own, the engine's Clear (easier to read), Darkness's, or Clear Caps.";
    case MPadMoves: return "On: exploring, the D-pad walks and L1 R1 L2 R2 move the highlights. Off: the D-pad moves them.";
    case MSwap: return "Which of A and B chooses; the other goes back.";
    case MChooser:
        return "How the game chooser shows the games: Tiles - all at once; Cards - one at a time, swiped through; "
               "Auto - tiles on a tablet, cards on a phone.";
    case MRotation:
        return "Auto: turns with the device (as its own rotation lock allows). Landscape or Portrait: stays that way "
               "however the device is held.";
    case MPadSize:
        if (shell::on()) return "How big the controls' keys are: Small, Medium, Large or Huge.";
        return "How big the on-screen buttons are: Small, Medium, Large or Huge.";
    case MPadAlpha:
        if (shell::on()) return "How solid the controls look: less shows more of what is behind them.";
        return "How solid the on-screen buttons look - they lie over the game, so less shows more of it. A touch on a "
               "button is the button's, anywhere else the game's.";
    case MTouchPad:
        return "Auto: shown when the screen is touched, hidden while a controller or keyboard is used. Without them: "
               "tap a line to choose it; swipe to scroll; two fingers together = Start; the phone's Back = B.";
    case MFolder: {
        static std::string words;
        words = "Your games: " + shown_path(pm::games_root()) + "/";
        if (shell::on()) {
            // (the app's drawer: as the phone's file manager names it - "Internal storage › GoldBox › games")
            std::string w;
            for (char ch : words) w += ch == '/' ? std::string(" \xE2\x80\xBA ") : std::string(1, ch);
            words = w.substr(0, w.size() - 5) + ".";
        } else {
            words += " -";
        }
        words += " Choose another folder: the game list closes and the folder screen opens.";
        return words.c_str();
    }
    case MControls: return "What each button does.";
    case MRolls: return "Fights show the dice behind each message: to hit, needed, damage, saves.";
    case MAutoJoin: return "An item taken from a treasure joins the same thing its taker already carries. Off: Join by hand.";
    case MSaveInfo: return "Save / Load Which Game shows the highlighted slot's save: when, where, the party, its map. Empty slots grey.";
    case MCoins: return "Any mix of coins pays a price, with exact change, and money pooled or shared out is kept in the highest coins. Off: as the originals. PoolRad, Curse, Secret and Gateway.";
    case MOil: return "A thrown flask of oil bursts into flames: fire damage. Off: as the originals, a plain blow.";
    case MAddInfo: return "While Add Character to Party lists your characters, the empty half of the screen shows the highlighted one as View Character does: name, race, class and levels, the abilities, armour class, hit points, THAC0, weapon, armour and icon. Off: as the original.";
    case MKeepMem: return "A rest that is interrupted keeps the spells still to memorize for the next rest. Off: the list is forgotten.";
    case MCampFix: return "Pool of Radiance's camp gets Fix, as the later games have it.";
    case MGallery: return "The icon editor opens on pages of ready-made combat icons. Off: the game's editor.";
    case MPortrait: return "View Character shows the combat icon large where a portrait would go.";
    case MMoreGuys: return "Add Character also lists your saved games' members and the earlier game's characters.";
    case MQuickOn: return "On: each member's own Quick Actions. Off: Quick as the originals play it.";
    case MQuick: return "What Quick may do for each member of the party in a fight; its On / Off switch.";
    case MMaxHp: return "Training gives the highest hit points the dice allow. A cheat: off unless you turn it on.";
    case MNoRace:
        return "No race stops a class's levels: an elf fighter trains past 7, a new character starts at the levels its "
               "experience gives. Each game's own last level stays. A cheat: off unless you turn it on.";
    case MClueDark:
        return "Dark: a cluebook map printed on white paper is shown light on dark, as the game's screens are. "
               "As Printed: as the book has it.";
    case MClueMaps:
        return "On: the Companion shows the map from your cluebook, the party's arrow on it, where the scan found "
               "the map in your cluebook PDF. Off: the map drawn by the game's own walls.";
    case MReplyAddr: return "Your email address, sent with Send Logs so the developer can write back. Optional.";
    case MRescan: return "Looks through the games folder again and makes every game's picture, journal and music anew.";
    case MClean:
        return "Frees space: takes out of each game's folder what the engine never reads (DOSBox, installers, the "
               "Steam launcher's files). Game files, saves and every PDF stay. A report: games/_CYD/CLEANUP.TXT.";
#ifdef __ANDROID__
    case MDevice: return "The versions, this device, and each game: its version and what the scan made of its art, journal, cluebook and music.";
#else
    case MDevice: return "This port's version, the device, and each game found with its release (Steam or GOG).";
#endif
    case MAbout: return "The app's, the front end's and the engine's versions, where they are made, the licence and credits.";
    case MJFont:
        return "The type the journal's text is shown in: Journal - like the printed Adventurer's Journal's hand; Book - "
               "an italic book face; Serif - like Times; Plain - the app's own.";
    case MUpdate: return "Looks for a newer version of the port and installs it (needs Wi-Fi).";
#ifdef __ANDROID__
    case MSendLogs: return "Tell the developer what went wrong: your email app opens with your answers and the logs - "
                           "no personal information.";
#else
    case MSendLogs: return "Sends the port's logs to the developer by the internet (needs Wi-Fi).";
#endif
    case MHealer: return "The game's temple, wherever you are: cures, all free.";
    case MMerchant: return "All the game's shops' goods in one shop; Id free.";
    case MSanctuary: return "The game's camp: rests are never interrupted.";
    case MTrainer: return "The game's training hall: every class, free; Modify for anyone.";
    case MBack: return "Back a page.";
    default: return "";
    }
}

// Device Info (the CYD's, Tom 2026-11-17): this software, the device as the
// launch script found it (device.txt), each game found and its release
const char* release_name(const library::GameDir& g)
{
    // (Steam / SNEG: the game files in <folder>/GAME/<SHORT>; GOG: its goggame-<id> files)
    std::string d = g.data_dir;
    for (char& ch : d) ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    if (d.find("/GAME/") != std::string::npos) return "Steam";
    bool gog = false;
    if (DIR* dir = opendir((pm::games_root() + "/" + g.folder).c_str())) {
        while (const dirent* e = readdir(dir))
            if (!strncasecmp(e->d_name, "goggame-", 8)) gog = true;
        closedir(dir);
    }
    return gog ? "GOG" : "unknown release";
}

// Device Info's lines for the Android app's drawer (name, value): what draw_device shows
const char* release_name(const library::GameDir& g);
#ifdef __ANDROID__
std::string android_device_info();
#ifndef PM_FRONT_VERSION
#define PM_FRONT_VERSION PM_VERSION
#endif
#endif
std::vector<std::pair<std::string, std::string>> device_lines_cache;     // (Android: Device Info's, while open)
std::string app_art_path(const library::GameDir& g);
std::vector<std::pair<std::string, std::string>> device_lines()
{
    std::vector<std::pair<std::string, std::string>> out;
#ifdef __ANDROID__
    if (!device_lines_cache.empty()) return device_lines_cache;
    // The app's Device Info (Tom, 2026-11-28): the versions, each with its v; what the app sees of the device - the
    // same as a report's DEVICE.TXT (GoldBoxActivity.deviceInfo); each of the nine games: its version or Not Present
    out.push_back({"Version", "App v" PM_VERSION ", front end v" PM_FRONT_VERSION ", engine v" CYD_VERSION});
    out.push_back({"\x01This Device", ""});
    {
        const std::string info = android_device_info();
        size_t p = 0;
        while (p < info.size()) {
            size_t e = info.find('\n', p);
            if (e == std::string::npos) e = info.size();
            const std::string l = info.substr(p, e - p);
            const size_t t = l.find('\t');
            if (t != std::string::npos) out.push_back({l.substr(0, t), l.substr(t + 1)});
            p = e + 1;
        }
    }
    using G = games::Game;
    static const G kSeries[] = {G::PoolOfRadiance,   G::CurseOfTheAzureBonds,       G::SecretOfTheSilverBlades,
                                G::PoolsOfDarkness,  G::GatewayToTheSavageFrontier, G::TreasuresOfTheSavageFrontier,
                                G::ChampionsOfKrynn, G::DeathKnightsOfKrynn,        G::DarkQueenOfKrynn};
    // (each game a heading - "\x01" before a name makes it one - its version, then what the scan made of its journal,
    // its cluebook and, where the game has it, its music: Tom, 2026-11-28 10:56)
    auto game_lines = [&out](const library::GameDir& g, const char* title) {
        const std::string r = release_name(g);
        out.push_back({std::string("\x01") + title, ""});
        out.push_back({"Version", r == "Steam" ? "Steam Version" : r == "GOG" ? "GOG Version" : "Unknown Version"});
        const std::string cache = library::cache_dir(g);
        // (its picture on the chooser: Steam's box art from the Launcher folder, else the game's own logo - the
        // scan's APPART.IMG; empty: none to be had - Tom, 2026-11-28 11:22)
        {
            struct stat st;
            std::string box;
            const bool made = stat(pm::host_path(app_art_path(g).c_str()).c_str(), &st) == 0;
            out.push_back({"Art", !made          ? "Not processed yet - Rescan Game Files"
                                  : st.st_size == 0 ? "Not found (no box art or logo)"
                                  : art::box_file(g, box) ? "Box art found and processed (Steam's launcher)"
                                                          : "Logo found and processed (the game's icon)"});
        }
        bool book = false;
        for (const jour::Document& d : jour::documents(g)) {
            std::string l = d.label;
            for (char& ch : l) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
            if (l.find("journal") != std::string::npos) book = true;
        }
        out.push_back({"Journal", plat::exists((cache + "/JOURNAL.DAT").c_str()) ? "Found, recognised and processed"
                                  : book ? "Found, but not processed - Rescan Game Files, or Report a Problem"
                                         : "Not found (its PDF isn't in the game's folder)"});
        // (the cluebook: CLUEMAPS.TXT - "maps 3" and a line a map, or "none 3<tab>why")
        std::string clue = "Not found";
        if (FILE* f = fopen(pm::host_path((cache + "/CLUEMAPS.TXT").c_str()).c_str(), "r")) {
            char l[256];
            int maps = -1;
            std::string why;
            while (fgets(l, sizeof l, f)) {
                l[strcspn(l, "\r\n")] = 0;
                if (maps < 0 && !strncmp(l, "maps", 4)) maps = 0;
                else if (maps < 0 && !strncmp(l, "none", 4)) {
                    const char* t = strchr(l, '\t');
                    why = t ? t + 1 : "";
                    break;
                } else if (maps >= 0 && l[0]) ++maps;
            }
            fclose(f);
            if (maps > 0) clue = "Found, recognised and processed (" + std::to_string(maps) + " maps)";
            else if (why == "unknown edition") clue = "Found, but this edition isn't recognised yet";
            else if (why == "not made") clue = "Found and recognised, but its maps couldn't be made";
            else if (!cluemaps::find(g).empty()) clue = "Found, not processed yet - Rescan Game Files";
        } else if (!cluemaps::find(g).empty()) {
            clue = "Found, not processed yet - Rescan Game Files";
        }
        out.push_back({"Cluebook", clue});
        const profile::Profile* prof = profile::first(g.game);
        const bool has = g.game == G::PoolsOfDarkness || g.game == G::ChampionsOfKrynn ||
                         g.game == G::DeathKnightsOfKrynn || g.game == G::DarkQueenOfKrynn ||
                         g.game == G::GatewayToTheSavageFrontier || g.game == G::TreasuresOfTheSavageFrontier;
        const uint32_t songs = musicfiles::songs(g.data_dir.c_str(), prof);
        if (songs) {
            int n = 0;
            for (uint32_t m = songs; m; m &= m - 1) ++n;
            out.push_back({"Music", musicfiles::ready(g.data_dir.c_str(), cache.c_str(), prof)
                                        ? "Found and processed (" + std::to_string(n) + " songs)"
                                        : "Found, not processed yet - Rescan Game Files"});
        } else if (has) {
            out.push_back({"Music", "Not found (its music files aren't in the folder)"});
        }
    };
    for (G k : kSeries) {
        bool any = false;
        for (const library::GameDir& g : games_found)
            if (g.game == k) {
                game_lines(g, games::title(k));
                any = true;
            }
        if (!any) {
            out.push_back({std::string("\x01") + games::title(k), ""});
            out.push_back({"Version", "Not Present"});
        }
    }
    for (const library::GameDir& g : games_found)
        if (g.game == G::Unknown || g.game == G::UnlimitedAdventures) game_lines(g, g.folder.c_str());
    device_lines_cache = out;
    return out;
#endif
    out.push_back({"Version", "App v" PM_VERSION ", engine " CYD_VERSION});
    if (FILE* f = fopen((port_dir() + "/device.txt").c_str(), "r")) {
        char l[200];
        while (fgets(l, sizeof l, f)) {
            l[strcspn(l, "\r\n")] = 0;
            char* eq = strchr(l, '=');
            if (!eq || !strcmp(l, "version")) continue;
            *eq = 0;
            l[0] = static_cast<char>(toupper(static_cast<unsigned char>(l[0])));
            out.push_back({l, eq + 1});
        }
        fclose(f);
    }
    if (games_found.empty()) out.push_back({"Games found", "None"});
    for (const library::GameDir& g : games_found) {
        const char* title = g.game == games::Game::Unknown ? g.folder.c_str() : games::title(g.game);
        out.push_back({title, release_name(g)});
    }
    return out;
}

// About (the Android app's, System - About; Tom, 2026-11-25). (The app builds PM_VERSION as its own version; the
// front end's own comes as PM_FRONT_VERSION, this repo's VERSION.)
#ifndef PM_FRONT_VERSION
#define PM_FRONT_VERSION PM_VERSION
#endif
std::vector<std::pair<std::string, std::string>> about_lines()
{
    return {
        {"Gold Box\x1fGame Engine", "An engine for SSI's Gold Box AD&D games - play your own GOG or Steam copies."},
        {"App", "v" + (app_version.empty() ? std::string("?") : app_version) +
                    " - github.com/tomtombombadil/APK-GoldBoxGameEngine"},
        {"Front end", "v" PM_FRONT_VERSION " - github.com/tomtombombadil/PM-GoldBoxGameEngine"},
        {"Engine", "v" CYD_VERSION " - github.com/tomtombombadil/CYD-GoldBoxGameEngine"},
        {"Author", "tomtombombadil"},
        {"Licence", "MIT - free and open source. No game data is included."},
        {"SDL2", "Simple DirectMedia Layer, zlib licence"},
        {"Fonts", "Fondamento (Astigmatic), Alegreya (Huerta Tipografica), Liberation Serif (Red Hat) - SIL Open Font "
                  "Licence; Roboto - Android's own"},
        {"Games", "Pool of Radiance and the other Gold Box games are SSI's and their owners' - not part of this app."},
    };
}

void draw_device()
{
    box(1, 1, 38, 23);
    centred(1, "Device Info", video::kGold);
    int row = 3;
    char line[40];
    snprintf(line, sizeof line, "Port v" PM_VERSION "  Engine " CYD_VERSION);
    video::text(2, row++, line, video::kCream);
    if (FILE* f = fopen((port_dir() + "/device.txt").c_str(), "r")) {
        char l[160];
        while (row < 9 && fgets(l, sizeof l, f)) {
            l[strcspn(l, "\r\n")] = 0;
            char* eq = strchr(l, '=');
            if (!eq || !strcmp(l, "version")) continue;
            *eq = 0;
            l[0] = static_cast<char>(toupper(static_cast<unsigned char>(l[0])));
            snprintf(line, sizeof line, "%.10s: %.24s", l, eq + 1);
            video::text(2, row++, line, video::kMuted);
        }
        fclose(f);
    }
    ++row;
    video::text(2, row++, games_found.empty() ? "No games found." : "Games found:", video::kGold);
    for (size_t i = 0; i < games_found.size() && row < 22; ++i) {
        const library::GameDir& g = games_found[i];
        const char* title = g.game == games::Game::Unknown ? g.folder.c_str() : fix_game_name(static_cast<int>(g.game));
        snprintf(line, sizeof line, "%-12.12s %s", title, release_name(g));
        video::text(3, row++, line, video::kCream);
    }
    snprintf(line, sizeof line, "%s: Back", back_name());
    centred(22, line, video::kMuted);
}

// The Button Guide's lines (key, what it does) - drawn here, and the Android app's drawer shows them
struct HelpLine { const char* key; const char* what; };
std::vector<HelpLine> help_lines()
{
    const char* a = confirm_name();
    const char* b = back_name();
    return {
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
}

void draw_help()
{
    box(2, 2, 36, 21);
    centred(2, "Controls", video::kGold);
    const char* b = back_name();
    int row = 4;
    for (const HelpLine& l : help_lines()) {
        video::text(3, row, l.key, video::kGold);
        video::text(l.what[0] && strlen(l.key) > 6 ? 15 : 10, row, l.what, video::kCream);
        ++row;
    }
    char line[41];
    snprintf(line, sizeof line, "Select + Start: quit   %s: Back", b);
    centred(21, line, video::kMuted);
}

// Text wrapped into cols columns at pixel (x, y), lines 9 apart, max_lines at most
void wrap_at(int x, int y, int cols, const char* text, uint32_t fg, int max_lines)
{
    const std::string s = text;
    size_t i = 0;
    for (int line = 0; line < max_lines && i < s.size(); ++line) {
        while (i < s.size() && s[i] == ' ') ++i;
        size_t end = i + static_cast<size_t>(cols);
        if (end >= s.size()) end = s.size();
        else {
            size_t brk = s.rfind(' ', end);
            if (brk != std::string::npos && brk > i) end = brk;
        }
        video::text_at(x, y + line * 9, s.substr(i, end - i).c_str(), fg);
        i = end;
    }
}

constexpr int kMenuPitch = 11;
int kMenuLines = 12;            // the list lines that fit above the highlighted line's words (draw_menu)

// The lines wrap_at would take for the text
int wrap_lines(int cols, const char* text)
{
    const std::string s = text;
    size_t i = 0;
    int lines = 0;
    while (i < s.size()) {
        while (i < s.size() && s[i] == ' ') ++i;
        if (i >= s.size()) break;
        size_t end = i + static_cast<size_t>(cols);
        if (end >= s.size()) end = s.size();
        else {
            const size_t brk = s.rfind(' ', end);
            if (brk != std::string::npos && brk > i) end = brk;
        }
        i = end;
        ++lines;
    }
    return lines;
}

void draw_menu()
{
    if (help_open) {
        draw_help();
        return;
    }
    if (device_open) {
        draw_device();
        return;
    }
    // The page: the list above, the highlighted line's words below - as many
    // lines as they need (the engine's words for a fix or a rule run long),
    // the list taking the rest
    const int n = static_cast<int>(menu_items.size());
    const int x0 = 8, w = 304, cols = 36;
    const char* help = menu_help(menu_items[menu_sel]);
    int hl = wrap_lines(cols, help);
    if (hl < 3) hl = 3;
    if (hl > 13) hl = 13;
    kMenuLines = (176 - 6 - hl * 9) / kMenuPitch;
    if (kMenuLines < 4) kMenuLines = 4;
    if (kMenuLines > n) kMenuLines = n;
    if (menu_sel < menu_top) menu_top = menu_sel;
    if (menu_sel >= menu_top + kMenuLines) menu_top = menu_sel - kMenuLines + 1;
    if (menu_top > n - kMenuLines) menu_top = n > kMenuLines ? n - kMenuLines : 0;
    video::fill(x0, 2, w, 196, video::kNavy);
    video::frame(x0, 2, w, 196, video::kGold);
    char title_buf[24];
    const char* title = page_title(menu_page);
    if (menu_page == Page::Fixes) {
        snprintf(title_buf, sizeof title_buf, "Fixes: %s", fix_game_name(fix_game));
        title = title_buf;
    }
    video::text_at(x0 + (w - static_cast<int>(strlen(title)) * 8) / 2, 7, title, video::kGold);
    const int list_h = kMenuLines * kMenuPitch;
    if (n > kMenuLines) {
        // (more above / below: small arrowheads at the right)
        for (int k = 0; k < 4; ++k) {
            if (menu_top > 0) video::fill(x0 + w - 14 - k, 20 + k, 2 * k + 1, 1, video::kCream);
            if (menu_top + kMenuLines < n) video::fill(x0 + w - 14 - k, 19 + list_h - 2 - k, 2 * k + 1, 1, video::kCream);
        }
    }
    for (int i = menu_top; i < n && i < menu_top + kMenuLines; ++i) {
        char line[48];
        menu_label(menu_items[i], line, sizeof line);
        const int y = 19 + (i - menu_top) * kMenuPitch;
        const bool sel = i == menu_sel;
        if (sel) video::fill(x0 + 4, y, w - 24, kMenuPitch, video::kLit);
        video::text_at(x0 + 10, y + 2, line, sel ? video::kCream : video::kMuted);
    }
    const int hy = 196 - hl * 9 - 3;
    video::fill(x0 + 4, hy - 4, w - 8, 1, video::kEdge);
    wrap_at(x0 + 8, hy, cols, help, video::kCream, hl);
}

// ---- Quick Actions ---------------------------------------------------------
// The CYD's Quick tab by buttons: the party down the left, the chosen
// member's eight switches down the right - Up / Down a switch, L1 / R1 (or
// Left / Right) a member, Confirm flips it (kept at once), Back leaves
bool quick_open = false;
int  q_rows = 0, q_row = 0, q_col = 0;

void draw_quick()
{
    // (Tom, 2026-11-17: one line at the top switches Quick Actions on or off,
    // the rest of the screen is the party's settings - no menu in between)
    const bool on = !(cfg.comforts_off & kOffQuick);
    video::fill(8, 2, 304, 196, video::kNavy);
    video::frame(8, 2, 304, 196, video::kGold);
    video::text_at(108, 7, "Quick Actions", video::kGold);
    char top[40];
    snprintf(top, sizeof top, "Quick Actions: %s", on ? "On" : "Off");
    if (q_col < 0) video::fill(12, 19, 296, 11, video::kLit);
    video::text_at(16, 21, top, q_col < 0 ? video::kCream : video::kMuted);
    const uint32_t dim = video::kEdge;
    for (int r = 0; r < q_rows && r < 9; ++r) {
        char nm[24], line[24];
        if (!play::quick_name(r, nm, sizeof nm)) nm[0] = 0;
        snprintf(line, sizeof line, "%.15s", nm);
        const int y = 34 + r * 11;
        if (r == q_row && on) video::fill(12, y, 148, 11, video::kKey);
        video::text_at(16, y + 2, line, !on ? dim : r == q_row ? video::kGold : video::kMuted);
        if (play::quick_npc(r)) video::text_at(136, y + 2, "NPC", on ? video::kMuted : dim);
    }
    const uint8_t mask = q_rows > 0 ? play::quick_get(q_row) : 0;
    for (int k = 0; k < quickacts::kCols; ++k) {
        const int y = 34 + k * 11;
        const bool sel = k == q_col && on;
        if (sel) video::fill(164, y, 144, 11, video::kLit);
        // (the switch: a box, filled when ticked)
        video::frame(168, y + 1, 9, 9, on ? video::kCream : dim);
        if (mask >> k & 1) video::fill(170, y + 3, 5, 5, on ? video::kGold : dim);
        video::text_at(182, y + 2, quickacts::kName[k], !on ? dim : sel ? video::kCream : video::kMuted);
    }
    video::fill(12, 136, 296, 1, video::kEdge);
    const char* words = q_col < 0 || !on ? "On: each member's own Quick Actions, set below. Off: Quick plays every member as the "
                                           "original games do."
                                         : quickacts::kDesc[q_col];
    wrap_at(16, 140, 36, words, video::kCream, 4);
    char hint[44];
    if (q_col < 0) snprintf(hint, sizeof hint, "%s: On / Off   %s: Back", confirm_name(), back_name());
    else snprintf(hint, sizeof hint, "L1 R1: Member  %s: Flip  %s: Back", confirm_name(), back_name());
    video::text_at((video::kW - static_cast<int>(strlen(hint)) * 8) / 2, 186, hint, video::kMuted);
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

// The reply address's keyboard (Send Logs' first time: then_send - the logs
// go when it closes; Settings' Reply Address: just the address)
bool        prompt_sends = false;
const char* prompt_note = nullptr;

void prompt_begin(bool then_send)
{
    keys.clear();
    key_sel = 0;
    static const char* const kRows[4] = {"abcdefghij", "klmnopqrst", "uvwxyz.@_-", "0123456789"};
    int row = 2;
    for (int r = 0; r < 4; ++r, row += 2)
        for (int i = 0; kRows[r][i]; ++i) keys.push_back(Key{kRows[r][i], nullptr, 5 + i * 3, row, 3});
    keys.push_back(Key{'\b', "Delete", 5, row, 9});
    keys.push_back(Key{'\x18', then_send ? "No Thanks" : "Clear", 14, row, 11});
    keys.push_back(Key{'\n', "Done", 25, row, 10});
    prompt_text = cfg.reply_email;
    prompt_sends = then_send;
    prompt_note = nullptr;
    prompt_open = true;
    menu_open = false;
}

bool email_like(const std::string& t)
{
    const size_t at = t.find('@');
    return at != std::string::npos && at > 0 && t.find('@', at + 1) == std::string::npos && t.find('.', at + 2) != std::string::npos &&
           t.back() != '.';
}

void prompt_key(char ch)
{
    if (ch == '\b') {
        if (!prompt_text.empty()) prompt_text.pop_back();
        prompt_note = nullptr;
    } else if (ch == '\n' || ch == '\x18') {
        // (Done with nothing typed is No Thanks / Clear)
        if (ch == '\x18') prompt_text.clear();
        if (!prompt_text.empty() && !email_like(prompt_text)) {
            prompt_note = "That isn't an email address yet.";
            return;
        }
        cfg.reply_email = prompt_text;
        cfg.reply_asked = true;
        settings_save(conf_dir, cfg);
        prompt_open = false;
        keys.clear();
        keys_for = play::Input::None;       // (the game's own keyboard, if it was up, is built again)
        if (prompt_sends) send_logs_now();
    } else if (prompt_text.size() < 60 && ch > ' ' && ch < 127) {
        prompt_text += (ch >= 'A' && ch <= 'Z') ? static_cast<char>(ch + 32) : ch;
        prompt_note = nullptr;
    }
}

void draw_prompt()
{
    draw_keys();
    box(2, 15, 36, 9);
    video::text(3, 15, "Your email address (optional):", video::kGold);
    const size_t n = prompt_text.size();
    video::text(3, 17, (prompt_text.substr(n > 33 ? n - 33 : 0) + "_").c_str(), video::kCream);
    if (prompt_note) video::text(3, 19, prompt_note, video::kWarn);
    else wrapped(3, 19, 34, "It goes with the logs to the developer, who may write back about the problem.", video::kMuted);
    char line[41];
    snprintf(line, sizeof line, "%s: Type  %s: Delete  X: %s  Y: Done", confirm_name(), back_name(), prompt_sends ? "No" : "Clear");
    video::text(3, 23, line, video::kMuted);
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
    // The port's own pages fill the panel; a game is 320 across
    video::set_wide(!playing);
    // (the Android app shows the port's own screens in its own views: nothing of them drawn here - shell.h)
    const bool app = shell::on();
    if (app && (screen == Screen::Games || screen == Screen::Scan || screen == Screen::Message)) {
        video::clear(video::kBlack);
        video::set_backdrop(nullptr);
    } else if (screen == Screen::Games) {
        draw_games();
    } else if (screen == Screen::Scan) {
        draw_scan();
    } else if (screen == Screen::Message) {
        draw_message();
    } else {
        video::clear();
        panel::hold(in_title());           // (no Companion beside the title screens: Tom)
        // (Save Slot Info: with the Companion beside or under the game its Party view shows the
        // highlighted slot's party; without, the engine draws it in the game screen's right half)
        play::set_side_panel(panel::docked_on());
        panel::clue(cur_cache, cfg.clue_maps, cfg.clue_dark);
        panel::draw(SDL_GetTicks());
        if (!menu_open && !prompt_open) draw_keys();
        if (ptr_on && !menu_open) video::pointer(static_cast<int>(ptr_x), static_cast<int>(ptr_y));
    }
    if (jour::active() && !menu_open) {
        // (an entry: the viewer's own frame - the game's text screen or a page; the Android app's viewer is over the
        // game, which stays as it was)
        const bool canvas = shell::on() ? true : jour::draw(cv);
        if (notice.open && !app) draw_notice();
        video::present(canvas ? &cv : nullptr);
        return;
    }
    if (menu_open && !app) draw_menu();
    if (quick_open && !app) draw_quick();
    if (jlist_open && !app) draw_jlist();
    if (prompt_open) draw_prompt();
    if (notice.open && !app) draw_notice();
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

void apply_options()
{
    play::Options o;
    const uint16_t off = cfg.comforts_off;
    o.gallery = !(off & kOffGallery);
    o.demo_tap = true;                          // (no switch any more - Tom, 2026-11-17, the CYD's menus)
    o.more_guys = !(off & kOffMoreGuys);
    o.portrait = !(off & kOffPortrait);
    o.item_fixes = true;                        // (each fix its own switch: fixes_off)
    o.fixes_off = cfg.fixes_off;
    o.adnd = adnd_on();
    o.rolls = !(off & kOffRolls);
    o.camp_fix = !(off & kOffCampFix);
    o.cheats = true;                            // (the Cheats page is always there)
    o.quick_acts = !(off & kOffQuick);
    o.read_time = true;                         // (no switch any more)
    o.auto_join = !(off & kOffAutoJoin);
    o.save_info = !(off & kOffSaveInfo);
    o.keep_mem = !(off & kOffKeepMem);
    o.coin_exchange = !(off & kOffCoins);
    o.oil_burns = !(off & kOffOil);
    o.add_info = !(off & kOffAddInfo);
    o.max_hp = cfg.max_hp;
    o.no_race_limits = cfg.no_race_limits;
    o.font = cfg.font;
    play::set_options(o);
}

// ---- Finding the games -----------------------------------------------------
// As the CYD's card scan (Tom, 2026-11-14): what the engine makes from a
// game's files - its Adventurer's Journal, its music - is made when the
// game is FOUND (the first run that sees its folder), never at a game's
// start; the menu's Rescan Game Files makes everything anew. The screen
// lists what it does and stays until Continue.
unsigned shape_seen = ~0u;      // (the picture shape the game list's pictures were last checked for: pictures_follow)
constexpr char kScanMark[] = "PMSCAN.TXT";      // in the game's _CYD folder: found and its files made
constexpr char kScanTag[] = "scan 8";       // (2: engine 0.198.1 - journals read anew, music 16-bit; 3: 0.199 - Queen's and Champions' music; 4: 0.200.4 - journal sections read anew; 5: 0.200.6 / 0.201.0 - the Steam Treasures journal, GOG Secret entry 45: journals made anew; 6: 0.221.3 - the Steam journals' text: standard fonts' widths, word gaps; 7: 0.221.6 - the Steam journals' text corrected; 8: 0.221.7 - the Dark Queen's letter a picture)
std::vector<std::string> scan_log;
bool                     scan_done = false, scan_progress = false;
bool                     scan_from_menu = false;  // (the Android app: Rescan / Clean Up - Continue back to the menu)
#ifdef __ANDROID__
#define SCAN_TITLE "Processing Game Files"      // (the app's - Tom, 2026-11-28; the handhelds keep theirs)
#else
#define SCAN_TITLE "Looking for Games"
#endif
const char*              scan_title = SCAN_TITLE;     // (Clean Up Game Files: its own)

// The scan's page: its lines in the smaller letters (12 x 12 on the
// backdrop's sharper picture), as many across as the panel has room for
std::vector<uint32_t> scan_page;

void draw_scan()
{
    const int m = video::margin(), bw = video::back_w(), rows_px = (video::kH - kHeaderH) * 2;
    video::clear(video::kClear);
    video::fill(-m, 0, video::kW + 2 * m, kHeaderH, video::kNavy);
    video::text_at(-m + 8, 6, scan_title, video::kGold);
    video::fill(-m, video::kH - 14, video::kW + 2 * m, 14, video::kBlack);
    char line[41];
    if (scan_done) snprintf(line, sizeof line, "%s: Continue", confirm_name());
    else snprintf(line, sizeof line, "Working - this can take minutes.");
    video::text_at((video::kW - static_cast<int>(strlen(line)) * 8) / 2, video::kH - 11, line, scan_done ? video::kGold : video::kMuted);
    // The newest lines that fit, wrapped
    const size_t cols = static_cast<size_t>((bw - 24) / 12);
    std::vector<std::string> rows;
    for (const std::string& l : scan_log) {
        size_t i = 0;
        do {
            size_t end = i + cols - (i ? 2 : 0);
            if (end >= l.size()) end = l.size();
            else {
                const size_t brk = l.rfind(' ', end);
                if (brk != std::string::npos && brk > i) end = brk;
            }
            rows.push_back((i ? "  " : "") + l.substr(i, end - i));
            i = end;
            while (i < l.size() && l[i] == ' ') ++i;
        } while (i < l.size());
    }
    const int pitch = 15, fit = (rows_px - 28 - 12) / pitch;
    const size_t from = rows.size() > static_cast<size_t>(fit) ? rows.size() - fit : 0;
    scan_page.assign(static_cast<size_t>(bw) * video::kBackH, 0xFF000000u);
    for (size_t k = from; k < rows.size(); ++k)
        video::small_text(scan_page.data(), bw, rows_px, 12, 10 + static_cast<int>(k - from) * pitch, rows[k].c_str(),
                          rows[k].compare(0, 5, "Found") == 0 ? video::kGold : video::kCream);
    video::set_backdrop(scan_page.data(), kHeaderH);
}

std::string android_scan_text();
std::string android_menu_text();

// A line onto the scan's list (replace: over the last one), shown at once
void scan_put(const char* line, bool replace)
{
    if (replace && !scan_log.empty()) scan_log.back() = line;
    else scan_log.push_back(line);
    plat::log("[scan] %s\n", line);
    if (shell::on()) {
        // (the Android app's page: its lines go to it - and the drawer told it is out of the way: the scan runs
        // here without the frame loop, so a menu left open over it stayed, its taps unheard - Tom, 2026-11-28)
        shell::text(shell::ChDrawer, android_menu_text());
        shell::text(shell::ChScan, android_scan_text());
        video::clear(video::kBlack);
        video::set_backdrop(nullptr);
    } else {
        draw_scan();
    }
    video::present(nullptr);
    SDL_PumpEvents();           // (minutes may pass: the window stays alive)
}

bool ends_with(const std::string& s, const char* tail)
{
    const size_t n = strlen(tail);
    return s.size() >= n && s.compare(s.size() - n, n, tail) == 0;
}

// The scan's lines (Tom's layout, 2026-11-23: "Found <game>" at the left, what
// is made for it indented two, a step's counter under its heading indented
// four and counting in place). replace: a counter - it takes the place of the
// counter before it. A journal step's heading ("  Processing the journal:")
// takes the place of the step before it and its counter ("  Reading ...").
void scan_say(const char* line, bool replace, void*)
{
    const bool counter = replace && scan_progress;
    if (!replace && scan_progress && scan_log.size() >= 2 && ends_with(line, " the journal:") &&
        ends_with(scan_log[scan_log.size() - 2], " the journal:")) {
        scan_log.pop_back();
        scan_put(line, true);
    } else {
        scan_put(line, counter);
    }
    scan_progress = replace;
}

// The engine's music lines put into the scan's layout: "  Rendering the
// <game> music:" once, then a line a song - "    song 2 of 8..." while it is
// made, its length added when it is ("... (24 s of music, 1 s to make)").
bool        music_head = false;
std::string music_song;

void scan_music(const char* line, bool, void*)
{
    char name[48] = "", rest[120] = "";
    int  n = 0, of = 0;
    scan_progress = false;
    if (sscanf(line, "Rendering the %47[^,], song %d of %d", name, &n, &of) == 3) {
        if (!music_head) {
            scan_put(("  Rendering the " + std::string(name) + ":").c_str(), false);
            music_head = true;
        }
        char row[64];
        snprintf(row, sizeof row, "    song %d of %d...", n, of);
        music_song = row;
        scan_put(row, false);
        return;
    }
    const char* open = strrchr(line, '(');
    if (strncmp(line, "Rendered the ", 13) == 0 && open && !music_song.empty()) {
        scan_put((music_song + " " + open).c_str(), true);
        music_song.clear();
        return;
    }
    const char* why = strstr(line, "wasn't rendered: ");
    if (why && !music_song.empty()) {
        snprintf(rest, sizeof rest, "%s", why + 17);
        if (char* end = strstr(rest, ". The next")) *end = 0;
        scan_put((music_song + " not made: " + rest).c_str(), true);
        music_song.clear();
        return;
    }
    scan_put(("    " + std::string(line)).c_str(), false);
}

bool scan_marked(const std::string& cache)
{
    plat::File f = plat::open((cache + "/" + kScanMark).c_str());
    if (!f) return false;
    char buf[16] = {};
    f.read(reinterpret_cast<uint8_t*>(buf), sizeof kScanTag - 1);
    f.close();
    return strcmp(buf, kScanTag) == 0;
}

void scan_begin(bool& any)
{
    if (any) return;
    any = true;
    scan_title = SCAN_TITLE;
    scan_log.clear();
    scan_done = scan_progress = false;
    screen = Screen::Scan;
    video::set_wide(true);
    scan_say("Searching for Gold Box games...", false, nullptr);
}

// The Android app's picture of a game (its game chooser, Tom's redesign stage 3): made by the scan as
// everything else made from a game's files - the box art's JPEG decoded whole by Android, or the logo -
// into _CYD/<folder>/APPART.IMG; the chooser only reads it. Nothing of it on a handheld.
std::string app_art_path(const library::GameDir& g) { return library::cache_dir(g) + "/APPART.IMG"; }

bool app_art_make(const library::GameDir& g)
{
#ifdef __ANDROID__
    const std::string out = pm::host_path(app_art_path(g).c_str());
    std::string box;
    if (art::box_file(g, box) && shell::save_art(pm::host_path(box.c_str()), nullptr, 0, 0, out)) return true;
    std::vector<uint8_t> rgba;
    int w = 0, h = 0;
    if (art::logo_rgba(g, 256, rgba, w, h) && shell::save_art("", rgba.data(), w, h, out)) return true;
    // (none to be had: an empty file, so the next start doesn't look again)
    plat::File f = plat::open(app_art_path(g).c_str(), "w");
    if (f) f.close();
#else
    (void)g;
#endif
    return false;
}

void scan_picture(const library::GameDir& g, const std::string& cache)
{
    app_art_make(g);
    const int kind = art::make(g, cache, art_want(g));
    scan_say(kind == 2 ? "  Its box art is ready." : kind == 1 ? "  Its logo is ready." : "  No picture: no box art or icon found.",
             false, nullptr);
}

// The scan: EVERYTHING the port makes from a game's files is made here,
// once - the program's unpacked copy, the game list's picture, the journal,
// the music - so that nothing is processed when a game starts or a page is
// first shown (all: every game's, anew). A game found before whose picture
// isn't this screen's (another Display shape, a Launcher folder copied in
// since) gets its picture made again. True: something was done - the
// scan's screen is up, waiting for Continue.
bool discover(bool all, bool pictures = true)
{
    bool any = false, found_new = false;
    for (const library::GameDir& g : games_found) {
        if (!play::available(g.game)) continue;
        const std::string cache = library::cache_dir(g);
        char line[120];
        if (!all && scan_marked(cache)) {
            // (found by a version whose scan didn't make these yet)
            if (!g.hlib && !plat::exists((cache + "/PROGRAM.BIN").c_str()))
                play::prepare_program(g.data_dir.c_str(), g.game, cache.c_str());
#ifdef __ANDROID__
            // (found by a version before the app's own pictures: made now, once - none: an empty file says so)
            if (!plat::exists(app_art_path(g).c_str())) {
                scan_begin(any);
                snprintf(line, sizeof line, "Preparing the picture for %s", games::title(g.game));
                scan_say(line, false, nullptr);
                app_art_make(g);
            }
#endif
            // (found by a version before the cluebook maps: made now, once)
            if (!cluemaps::ready(cache) && cluemaps::find(g).empty()) cluemaps::none(cache);
            if (!cluemaps::ready(cache)) {
                scan_begin(any);
                snprintf(line, sizeof line, "Preparing the cluebook maps for %s", games::title(g.game));
                scan_say(line, false, nullptr);
                cluemaps::make(g, cache, scan_say);
            }
            if (pictures && !art::ready(g, cache, art_want(g))) {
                scan_begin(any);
                snprintf(line, sizeof line, "Preparing the picture for %s", games::title(g.game));
                scan_say(line, false, nullptr);
                scan_picture(g, cache);
            }
            continue;
        }
        scan_begin(any);
        found_new = true;
        snprintf(line, sizeof line, "Found %s", games::title(g.game));
        scan_say(line, false, nullptr);
        // Its program's tables, unpacked (the game's start reads this copy)
        if (!g.hlib) play::prepare_program(g.data_dir.c_str(), g.game, cache.c_str());
        scan_picture(g, cache);
        // Its Adventurer's Journal, from the player's own PDF
        // (anew on Rescan, and when an older version's scan made it - a new
        //  scan tag means the journal making changed, here or in the engine)
        if (all || plat::exists((cache + "/" + kScanMark).c_str())) {
            plat::remove((cache + "/JOURNAL.DAT").c_str());
            plat::remove((cache + "/BOOK.TXT").c_str());
        }
        jour::prepare(g, cache, scan_say);
        snprintf(line, sizeof line, "%s", plat::exists((cache + "/JOURNAL.DAT").c_str())
                                              ? "  Its journal is ready."
                                              : "  No journal: its PDF isn't in the game's folder.");
        scan_say(line, false, nullptr);
        // Its cluebook's maps, from the player's own PDF (the Companion's Cluebook Maps)
        cluemaps::make(g, cache, scan_say);
        // Its music (the later games), from its own files
        // (engine 0.199: the Dark Queen's and Champions' too - by the game's profile, as the CYD's scan)
        const profile::Profile* prof = profile::first(g.game);
        if (musicfiles::songs(g.data_dir.c_str(), prof) && (all || !musicfiles::ready(g.data_dir.c_str(), cache.c_str(), prof))) {
            music_head = false;
            music_song.clear();
            const int failed =
                musicfiles::make(g.data_dir.c_str(), cache.c_str(), prof, games::short_title(g.game), all, scan_music, nullptr);
            scan_say(failed ? "  Some of its music couldn't be made." : "  Its music is ready.", false, nullptr);
        } else if (!musicfiles::songs(g.data_dir.c_str(), prof) &&
                   (g.game == games::Game::PoolsOfDarkness || g.game == games::Game::ChampionsOfKrynn ||
                    g.game == games::Game::DeathKnightsOfKrynn || g.game == games::Game::DarkQueenOfKrynn ||
                    g.game == games::Game::GatewayToTheSavageFrontier ||
                    g.game == games::Game::TreasuresOfTheSavageFrontier)) {
            // (a copy without the game's AdLib music files - a player's log, v0.12.7: a silent Dark Queen
            // and no word why)
            scan_say("  No music: its music files aren't in the folder.", false, nullptr);
        }
        plat::File f = plat::open((cache + "/" + kScanMark).c_str(), "w");
        if (f) {
            f.write(reinterpret_cast<const uint8_t*>(kScanTag), sizeof kScanTag - 1);
            f.close();
        }
    }
    if (any) {
        scan_done = true;
        scan_say("Done.", false, nullptr);
        // (pictures alone: nothing to read - straight back to the list)
        if (!found_new) screen = Screen::Games;
    }
    if (pictures) shape_seen = video::shape_serial();
    return any;
}

std::string card_dir(const library::GameDir& g) { return std::string(games::kRootDir) + "/" + g.data_dir; }

// Continue on the Android app's chooser (Tom, 2026-11-24): the game's newest
// saved game, noted in _CYD/<folder>/CONTINUE.TXT whenever a game opens and
// closes - the chooser shows Continue for a game with one; the slot itself is
// looked for anew at the tap (play::newest_save). Android only.
void continue_note()
{
#ifdef __ANDROID__
    if (!playing || games_found.empty()) return;
    const std::string f = library::cache_dir(games_found[game_sel]) + "/CONTINUE.TXT";
    const char slot = play::newest_save();
    if (!slot) {
        plat::remove(f.c_str());
        return;
    }
    if (plat::File out = plat::open(f.c_str(), "w")) {
        out.write(reinterpret_cast<const uint8_t*>(&slot), 1);
        out.close();
    }
#endif
}

void leave_game()
{
    if (!playing) return;
    continue_note();
    if (jour::active()) {
        jour::close(cv);
        shell::text(shell::ChJournal, "");
    }
    journal_queue.clear();
    jlist_open = false;
    audio_stop();
    play::close();
    panel::leave();
    quick_open = false;
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
    // (no loading screen: the scan made everything a start needs)
    video::set_backdrop(nullptr);
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
    cur_cache = cache;
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
    continue_note();
}

void message(const char* title, const std::string& body);

// The next entry waiting to be shown (one that can't be: said instead)
void journal_next()
{
    while (!journal_queue.empty()) {
        const JRef r = journal_queue.front();
        journal_queue.erase(journal_queue.begin());
        std::string why;
        if (jour::open(cur_cache, r.kind, r.number, cv, why)) {
            // (the Android app shows it in its own viewer - Tom, 2026-11-25)
            if (shell::on()) shell::text(shell::ChJournal, jour::app_entry(cfg.journal_font));
            return;
        }
        message("Adventurer's Journal", why);
        return;
    }
}

// The game mentioned journal entries (its screen stays up to be read; the
// next press comes here): they open, one after the other
bool journal_due()
{
    char kind;
    int number;
    if (!play::journal_waiting() || !play::journal_request(&kind, &number)) return false;
    do journal_queue.push_back(JRef{kind, number});
    while (play::journal_request(&kind, &number));
    journal_next();
    return true;
}

// The entries met so far (the Start menu's Journal)
void draw_jlist()
{
    const int n = play::journal_seen_count(), rows = 16;
    box(8, 3, 24, rows + 3);
    centred(3, "Journal", video::kGold);
    int first = jlist_sel - rows / 2;
    if (first > n - rows) first = n - rows;
    if (first < 0) first = 0;
    for (int i = first; i < n && i < first + rows; ++i) {
        char kind, name[40];
        int number;
        if (!play::journal_seen(i, &kind, &number)) continue;
        journal::mention_name(kind, number, name, sizeof name);
        const bool sel = i == jlist_sel;
        if (sel) video::fill(8 * 8 - 2, (5 + i - first) * 8 - 1, 24 * 8 + 4, 10, video::kLit);
        video::text(9, 5 + i - first, name, sel ? video::kCream : video::kMuted);
    }
    char line[41];
    snprintf(line, sizeof line, "%s: Read   %s: Back", confirm_name(), back_name());
    centred(22, line, video::kMuted);
}

void jlist_press(int b)
{
    const int n = play::journal_seen_count();
    if (b == BUp && n) jlist_sel = (jlist_sel + n - 1) % n;
    else if (b == BDown && n) jlist_sel = (jlist_sel + 1) % n;
    else if (b == BB || b == BStart) jlist_open = false;
    else if (b == BA && n) {
        char kind;
        int number;
        if (play::journal_seen(jlist_sel, &kind, &number)) {
            jlist_open = false;
            menu_open = false;
            journal_queue.push_back(JRef{kind, number});
            journal_next();
        }
    }
}

void journal_press(int b)
{
    switch (b) {
    case BUp: jour::scroll(-1); break;
    case BDown: jour::scroll(1); break;
    case BLeft: jour::pan(-1); break;
    case BRight: jour::pan(1); break;
    case BL1:
    case BL2: jour::page(-1); break;
    case BR1:
    case BR2: jour::page(1); break;
    case BX:
    case BY: jour::zoom(); break;
    case BA:
        if (!jour::at_end()) {
            jour::page(1);
            break;
        }
        // fall through
    case BB:
        jour::close(cv);
        shell::text(shell::ChJournal, "");
        journal_next();         // (the next one mentioned, if there was one)
        break;
    default: break;
    }
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
        // (an overland where the keys step - PoolRad's wilderness, Champions' and the Dark Queen's maps, the
        // engine's play::move_keys, 0.214.0: Left / Right step west / east. Until v0.12.12 they were the turn
        // keys there - nothing on PoolRad's wilderness, north-west / north-east on the Krynn maps: no way
        // straight east or west)
        const bool steps = play::move_keys() != play::MoveKeys::Turns;
        play::act(b == BUp ? play::Act::Forward : b == BDown ? play::Act::TurnAround
                  : b == BLeft ? (steps ? play::Act::StepLeft : play::Act::TurnLeft)
                  : (steps ? play::Act::StepRight : play::Act::TurnRight), cv);
        return;
    }
    play::nav(b == BUp ? play::Nav::Up : b == BDown ? play::Nav::Down : b == BLeft ? play::Nav::Left : play::Nav::Right, cv);
}

// The game list's pictures are made for the picture's shape on this panel.
// Whenever that changes - the panel's real size arriving after the start
// (Tom's RGB10 MAX3, v0.6.2: no pictures until a rescan), Settings -
// Display, a PC window resized - the list checks them again and makes the
// ones that don't fit (the scan's screen, no wait)
void pictures_follow()
{
    // (only once the shape has stood still a moment: a panel can change
    // it more than once as it starts)
    static unsigned last = ~0u;
    static Uint32   since = 0;
    const unsigned now_serial = video::shape_serial();
    if (now_serial != last) {
        last = now_serial;
        since = SDL_GetTicks();
    }
    if (screen != Screen::Games || playing || menu_open || notice.open) return;
    // (the Android app's chooser has pictures of its own, made once: none follow the screen's shape)
    if (shell::on()) return;
    if (now_serial == shape_seen || SDL_GetTicks() - since < 300) return;
    shape_seen = now_serial;
    discover(false);
}

// Original Games (the CYD's, Tom 2026-11-17): on - every Enhancement, AD&D 1E
// Rule and Game Fix off, the game's own font, Max Hit Points off, the
// player's own set kept and given back when it goes off; meanwhile any
// switch can still be turned on by itself. Picture, sound, buttons: untouched
void toggle_original()
{
    if (!cfg.original) {
        cfg.saved_comforts_off = cfg.comforts_off;
        cfg.saved_rules_off = cfg.rules_off;
        cfg.saved_fixes_off = cfg.fixes_off;
        cfg.saved_max_hp = cfg.max_hp;
        cfg.saved_no_race_limits = cfg.no_race_limits;
        cfg.saved_font = cfg.font;
        cfg.comforts_off = kOffSwitches;
        cfg.rules_off = rules::kAdndAll;
        cfg.fixes_off = ~0ull;
        cfg.max_hp = false;
        cfg.no_race_limits = false;
        cfg.font = 0;
        cfg.original = true;
    } else {
        cfg.comforts_off = cfg.saved_comforts_off;
        cfg.rules_off = cfg.saved_rules_off;
        cfg.fixes_off = cfg.saved_fixes_off;
        cfg.max_hp = cfg.saved_max_hp;
        cfg.no_race_limits = cfg.saved_no_race_limits;
        cfg.font = cfg.saved_font;
        cfg.original = false;
    }
}

// (menu_dry: the value changed in cfg alone - nothing applied or saved; menu_choices works the list out so)
bool menu_dry = false;

void menu_change(int step)
{
    const MItem it = menu_items[menu_sel];
    const MenuItem m = it.m;
    switch (m) {
    case MRule:
        cfg.rules_off ^= rules::adnd_info(it.arg).bit;
        if (playing) apply_options();
        break;
    case MFix:
        cfg.fixes_off ^= 1ull << it.arg;
        if (playing) apply_options();
        break;
    case MOriginal:
        toggle_original();
        if (playing) apply_options();
        break;
    case MDisplay:
#ifdef __ANDROID__
    {
        // (the app's drawer: 4:3, Square Pixels, Stretch - the app places the Companion itself)
        if (shell::on()) {
            const int at = cfg.fit > 2 ? 0 : cfg.fit;
            cfg.fit = static_cast<uint8_t>((at + (step < 0 ? 2 : 1)) % 3);
            if (!menu_dry) video::set_fit(static_cast<video::Fit>(cfg.fit));
            break;
        }
        // (Android: 4:3, 4:3 + Companion Right, 4:3 + Companion Below, Square Pixels, Stretch)
        static const uint8_t kOrder[5] = {0, 3, 4, 1, 2};
        int at = 0;
        while (at < 5 && kOrder[at] != cfg.fit) ++at;
        cfg.fit = kOrder[((at < 5 ? at : 0) + (step < 0 ? 4 : 1)) % 5];
    }
#else
        // (the handhelds: Original 4:3, Square Pixels, Stretch - Tom, 2026-11-23: the Companion choices are
        // Android's, "The Port is working and displaying great")
        cfg.fit = static_cast<uint8_t>((cfg.fit + (step < 0 ? 2 : 1)) % 3);
#endif
        if (!menu_dry) video::set_fit(static_cast<video::Fit>(cfg.fit));
        // (the game list's pictures follow the shape: pictures_follow)
        break;
    case MSound:
        sound_cycle(step);
        if (playing && !menu_dry) apply_sound();
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
        if (playing && !menu_dry) apply_options();
        break;
    case MPadMoves: cfg.pad_moves = !cfg.pad_moves; break;
    case MRotation:
        cfg.rotation = static_cast<uint8_t>((cfg.rotation + (step < 0 ? 2 : 1)) % 3);
        if (!menu_dry) rotation_apply();
        break;
    case MChooser: cfg.chooser = static_cast<uint8_t>((cfg.chooser + (step < 0 ? 2 : 1)) % 3); break;
    case MJFont: cfg.journal_font = static_cast<uint8_t>((cfg.journal_font + (step < 0 ? 3 : 1)) % 4); break;
    case MTouchPad:
        cfg.touch_pad = static_cast<uint8_t>((cfg.touch_pad + (step < 0 ? 2 : 1)) % 3);
        if (!menu_dry) touch_pad_apply();
        break;
    case MPadSize:
        cfg.pad_size = static_cast<uint8_t>((cfg.pad_size + (step < 0 ? 3 : 1)) % 4);
        if (!menu_dry) pad_style_apply();
        break;
    case MPadAlpha:
        cfg.pad_alpha = static_cast<uint8_t>((cfg.pad_alpha + (step < 0 ? 4 : 1)) % 5);
        if (!menu_dry) pad_style_apply();
        break;
    case MSwap: cfg.swap_ab = !cfg.swap_ab; cfg.ab_set = true; break;
    case MMaxHp:
        cfg.max_hp = !cfg.max_hp;
        if (playing) apply_options();
        break;
    case MNoRace:
        cfg.no_race_limits = !cfg.no_race_limits;
        if (playing) apply_options();
        break;
    case MClueMaps: cfg.clue_maps = !cfg.clue_maps; break;
    case MClueDark: cfg.clue_dark = !cfg.clue_dark; break;
    default:
        if (const uint16_t bit = comfort_bit(m)) {
            cfg.comforts_off ^= bit;
            if (playing) apply_options();
            break;
        }
        return;
    }
    if (!menu_dry) settings_save(conf_dir, cfg);
}

void rescan(bool kept = false)
{
    have_root = kept ? library::scan_kept(games_found) : library::scan(games_found);
#ifdef __ANDROID__
    // (the app's chooser by series, Tom 2026-11-24: the Forgotten Realms four, the Savage Frontier two, then
    // Krynn's three - a tablet's grid has Krynn as its whole last row)
    auto rank = [](games::Game g) {
        switch (g) {
        case games::Game::GatewayToTheSavageFrontier: return 5;
        case games::Game::TreasuresOfTheSavageFrontier: return 6;
        case games::Game::ChampionsOfKrynn: return 7;
        case games::Game::DeathKnightsOfKrynn: return 8;
        case games::Game::DarkQueenOfKrynn: return 9;
        case games::Game::Unknown: return 100;
        default: return static_cast<int>(g) < 5 ? static_cast<int>(g) : 10 + static_cast<int>(g);
        }
    };
    std::stable_sort(games_found.begin(), games_found.end(),
                     [&](const library::GameDir& a, const library::GameDir& b) { return rank(a.game) < rank(b.game); });
#endif
    if (game_sel >= static_cast<int>(games_found.size())) game_sel = 0;
}

// Clean Up Game Files (Tom, 2026-11-24; the engine's game/tidy.h has the
// rules): each game found loses what the engine never reads - DOSBox, the
// installers, the Steam launcher's and the Gold Box Companion's own files;
// its game files, saves and every PDF stay - and the Launcher folder is cut
// to the box art. Shown on the scan's page (a game a line), every file in
// games/_CYD/CLEANUP.TXT (sent with Send Logs).
FILE* clean_log = nullptr;

void clean_up()
{
    rescan();
    scan_log.clear();
    scan_done = scan_progress = false;
    scan_title = "Cleaning Up Game Files";
    screen = Screen::Scan;
    video::set_wide(true);
    const std::string top = std::string(games::kRootDir) + "/" + library::kCacheDir;
    plat::mkdir(top.c_str());
    clean_log = fopen(pm::host_path((top + "/CLEANUP.TXT").c_str()).c_str(), "w");
    if (clean_log) fprintf(clean_log, "Gold Box Game Engine v%s - Clean Up Game Files\n\n", PM_VERSION);
    const tidy::Say detail = [](const char* line, void*) {
        if (clean_log) fprintf(clean_log, "  %s\n", line);
    };
    tidy::Report all;
    char line[160];
    for (const library::GameDir& g : games_found) {
        if (g.game == games::Game::Unknown) continue;           // (a folder not named after a game: left alone)
        snprintf(line, sizeof line, "Cleaning up %s...", games::title(g.game));
        scan_put(line, false);
        if (clean_log) fprintf(clean_log, "%s\n", line);
        tidy::Report r;
        tidy::clean_game(g.folder.c_str(), g.data_dir.c_str(), true, r, detail, nullptr);
        snprintf(line, sizeof line, "  %d file%s removed (%.1f MB)%s", r.files, r.files == 1 ? "" : "s",
                 r.bytes / 1048576.0, r.failed ? " - some couldn't be" : "");
        scan_put(line, false);
        all.files += r.files, all.bytes += r.bytes, all.folders += r.folders, all.left += r.left,
            all.failed += r.failed;
    }
    {
        tidy::Report r;
        if (clean_log) fprintf(clean_log, "The Launcher folder\n");
        tidy::clean_launcher(true, r, detail, nullptr);
        if (r.files) {
            snprintf(line, sizeof line, "The Launcher folder: %d files removed (%.1f MB) - its box art kept", r.files,
                     r.bytes / 1048576.0);
            scan_put(line, false);
        }
        all.files += r.files, all.bytes += r.bytes, all.folders += r.folders, all.failed += r.failed;
    }
    snprintf(line, sizeof line, "Done: %d file%s and %d folder%s removed, %.1f MB freed. Game files, saves and "
             "PDFs are all still there.", all.files, all.files == 1 ? "" : "s", all.folders, all.folders == 1 ? "" : "s",
             all.bytes / 1048576.0);
    scan_put(line, false);
    if (clean_log) fprintf(clean_log, "\n%s\n", line);
    if (all.left) {
        snprintf(line, sizeof line, "%d file%s the engine doesn't know left alone (listed in games/_CYD/CLEANUP.TXT).",
                 all.left, all.left == 1 ? "" : "s");
        scan_put(line, false);
    }
    if (all.failed) scan_put("Some files couldn't be removed (listed in games/_CYD/CLEANUP.TXT).", false);
    if (clean_log) {
        fclose(clean_log);
        clean_log = nullptr;
    }
    scan_done = true;
    draw_scan();
    video::present(nullptr);
}

// A cheat's screen, on the game's own (only at the exploring menu)
void cheat_go(int which)
{
    if (!play::cheats_ready() || !play::cheat_open(which, cv)) {
        show_notice("Cheats", "The cheats open while the party is exploring, at the game's menu (Area, Cast, View ...) - "
                              "not in a fight, a shop, a camp or a conversation.");
        return;
    }
    menu_open = false;
    after_input();
}

void quick_begin()
{
    const bool off = (cfg.comforts_off & kOffQuick) != 0;
    q_rows = play::quick_ready() ? play::quick_begin() : 0;
    if (q_rows <= 0 && !off) {
        show_notice("Quick Actions", "Quick Actions need a party: load a saved game or make characters first.");
        return;
    }
    q_row = 0;
    q_col = cfg.comforts_off & kOffQuick ? -1 : 0;        // (off: on its switch)
    quick_open = true;
    // (the menu stays open under it: Back returns to it - Tom, 2026-11-28 - Start closes both)
}

void quick_press(int b)
{
    // (q_col -1: the On / Off line at the top)
    const bool on = !(cfg.comforts_off & kOffQuick);
    switch (b) {
    case BUp: q_col = !on ? -1 : q_col <= -1 ? quickacts::kCols - 1 : q_col - 1; break;
    case BDown: q_col = !on ? -1 : q_col >= quickacts::kCols - 1 ? -1 : q_col + 1; break;
    case BLeft:
    case BL1:
    case BL2:
        if (on) q_row = (q_row + q_rows - 1) % q_rows;
        break;
    case BRight:
    case BR1:
    case BR2:
        if (on) q_row = (q_row + 1) % q_rows;
        break;
    case BA:
        if (q_col < 0) {
            cfg.comforts_off ^= kOffQuick;
            apply_options();
            settings_save(conf_dir, cfg);
            // (switched on: the party's rows are there now; off: they are dimmed, as they stand)
            if (!(cfg.comforts_off & kOffQuick)) {
                const int n = play::quick_ready() ? play::quick_begin() : 0;
                if (n > 0) q_rows = n;
                else if (q_rows <= 0) {
                    quick_open = false;
                    show_notice("Quick Actions", "Quick Actions are on. Their settings need a party: load a saved game or make characters first.");
                }
                if (q_row >= q_rows) q_row = 0;
            }
        } else if (!play::quick_toggle(q_row, q_col))
            show_notice("Quick Actions", "The change couldn't be kept: the games folder can't be written.");
        break;
    case BB: quick_open = false; break;                     // (back to the menu)
    case BStart:
        quick_open = false;
        menu_open = false;
        break;
    default: break;
    }
}

void menu_press(int b)
{
    if (help_open || device_open) {
        if (b == BA || b == BB || b == BStart) help_open = device_open = false;
        return;
    }
    const int n = static_cast<int>(menu_items.size());
    switch (b) {
    case BUp: menu_sel = (menu_sel + n - 1) % n; break;
    case BDown: menu_sel = (menu_sel + 1) % n; break;
    case BL2: menu_sel = menu_sel >= kMenuLines ? menu_sel - kMenuLines : 0; break;
    case BR2: menu_sel = menu_sel + kMenuLines < n ? menu_sel + kMenuLines : n - 1; break;
    case BLeft: menu_change(-1); break;
    case BRight: menu_change(1); break;
    case BB:
        if (menu_page != Page::Top) menu_go(page_parent(menu_page));
        else menu_open = false;
        break;
    case BStart: menu_open = false; break;
    case BA: {
        const MItem it = menu_items[menu_sel];
        switch (it.m) {
        case MResume: menu_open = false; break;
        case MControls: help_open = true; break;
        case MDevice:
            device_open = true;
            about_open = false;
            device_lines_cache.clear();     // (worked out anew each time it opens - not each frame)
            break;
        case MAbout: device_open = about_open = true; break;
        case MJournal:
#ifdef __ANDROID__
            // (the app's list: the entries, then the game's PDFs - Tom, 2026-11-27)
            jlist_docs = jour::documents(games_found[game_sel]);
            if (play::journal_seen_count() <= 0 && !jlist_docs.empty()) {
                jlist_open = true;
                jlist_sel = 0;
                break;
            }
#endif
            if (play::journal_seen_count() <= 0) {
                show_notice("Journal", "The game hasn't told you to read a journal entry yet. Each one it mentions is listed here.");
                break;
            }
            jlist_open = true;
            jlist_sel = play::journal_seen_count() - 1;         // (the newest)
            break;
        case MQuick: quick_begin(); break;
        case MFixGame:
            fix_game = it.arg;
            menu_go(Page::Fixes);
            break;
        case MBack: menu_go(page_parent(menu_page)); break;
        case MHealer: cheat_go(0); break;
        case MMerchant: cheat_go(1); break;
        case MSanctuary: cheat_go(2); break;
        case MTrainer: cheat_go(3); break;
        case MRescan:
            if (playing) leave_game();          // (in a game: leaves it first - the CYD's)
            menu_open = false;
#ifdef __ANDROID__
            scan_from_menu = true;              // (its Continue back to the menu - Tom, 2026-11-28)
#endif
            rescan();
            if (!discover(true)) {
                screen = Screen::Games;
                if (scan_from_menu) menu_open = true;
                scan_from_menu = false;
            }
            break;
        case MClean:
            if (playing) leave_game();          // (in a game: leaves it first - its files are gone through)
            menu_open = false;
#ifdef __ANDROID__
            scan_from_menu = true;
#endif
            clean_up();
            break;
        case MUpdate: update_check(); break;
        case MFolder:
#ifdef __ANDROID__
            if (playing) leave_game();
            android_choose_folder();
            quit = true;
#endif
            break;
#ifdef __ANDROID__
        case MSendLogs: android_report(-1); break;
#else
        case MSendLogs: send_logs(); break;
#endif
        case MReplyAddr: prompt_begin(false); break;
        case MLeaveGame: leave_game(); break;
        case MExit: quit = true; break;
        default:
            if (page_of(it.m) != Page::Top) menu_go(page_of(it.m));
            else menu_change(0);
            break;
        }
        break;
    }
    default: break;
    }
}

// A button went down (b: with Confirm / Back already as the player set them)
// ---- The Android app's own screens (shell.h; the redesign's stages 2 and 3)
// The port's own screens - the menu's pages, Device Info, the Button Guide,
// the journal list, Quick Actions, notices and messages, the game chooser and
// the scan's page - are shown by the app in Android's views when it has laid
// the screen out; their lines go to it as text (android_*_text, sent when
// changed), its taps come back here as the buttons would act (android_op).
// The screens stay this front end's: a controller still drives each.

constexpr int kOpChooserPlay = 10, kOpLookAgain = 11, kOpChooserSelect = 12, kOpChooserContinue = 13,
              kOpChooserMode = 14, kOpJournalClose = 15, kOpReport = 18, kOpPick = 19;
void menu_pick(int i, int k);
void press(int b);

#ifdef __ANDROID__
// Report a Problem: what the app's dialog fills in for the player - the game (the one open, else `game`, the
// chooser's; -1 none), where in it, the games found (its list), the versions - and, a game in play, a save of it as
// it is (play::snapshot into games/_CYD/_REPORT/SAVE, nothing of the game's own touched; the app zips it if the
// player ticks it, then empties the folder)
void android_report(int game)
{
    std::string t = "report\n";
    auto line = [&t](const char* k, const std::string& v) {
        t += k;
        t += '\t';
        for (char ch : v) t += ch == '\n' || ch == '\t' ? ' ' : ch;
        t += '\n';
    };
    auto title = [](const library::GameDir& g) {
        return std::string(g.game == games::Game::Unknown ? g.folder.c_str() : games::title(g.game));
    };
    const int n = static_cast<int>(games_found.size());
    if (playing && game_sel >= 0 && game_sel < n) game = game_sel;
    line("game", game >= 0 && game < n ? title(games_found[game]) : "");
    std::string games;
    for (const auto& g : games_found) games += (games.empty() ? "" : "\x1f") + title(g);
    line("games", games);
    std::string area, square, snap;
    const std::string report = std::string(games::kRootDir) + "/" + library::kCacheDir + "/_REPORT";
    if (playing) {
        if (play::titles()) area = "The game's title screens";
        else if (play::map()) {
            char file[16] = "";
            const int block = play::map_source(file, sizeof file);
            if (block >= 0) {
                area = cluemaps::name(cur_cache, file, block);
                if (area.empty()) area = std::string(file) + " map " + std::to_string(block);
            }
            char sq[48];
            snprintf(sq, sizeof sq, "%d, %d facing %s", play::pos_x(), play::pos_y(), geo::dir_name(play::dir()));
            square = sq;
        }
        // (a save of the game as it is, only for the report - an earlier report's left there goes first)
        mkdir(pm::host_path(report.c_str()).c_str(), 0755);
        const std::string host = pm::host_path((report + "/SAVE").c_str());
        if (DIR* dir = opendir(host.c_str())) {
            while (dirent* e = readdir(dir))
                if (e->d_name[0] != '.') remove((host + "/" + e->d_name).c_str());
            closedir(dir);
        }
        if (play::snapshot((report + "/SAVE").c_str())) snap = pm::host_path((report + "/SAVE").c_str());
    } else {
        area = screen == Screen::Games ? "The game chooser" : "";
    }
    line("area", area);
    line("square", square);
    line("save", snap);
    line("front", PM_FRONT_VERSION);       // (the app builds PM_VERSION as its own)
    line("engine", CYD_VERSION);
    fflush(stdout);         // (the log as it is now)
    android_report_call(t);
}
#endif

void android_op(int op, int index, int value)
{
    if (op == kOpChooserPlay) {
        const int n = static_cast<int>(games_found.size());
        if (screen == Screen::Games && !menu_open && !notice.open && index >= 0 && index < n) {
            game_sel = index;
            press(BA);
        }
        return;
    }
    if (op == kOpChooserContinue) {
        // Continue: the game opened and its newest save loaded, past the titles and the party menu
        const int n = static_cast<int>(games_found.size());
        if (screen != Screen::Games || menu_open || notice.open || index < 0 || index >= n) return;
        game_sel = index;
        start_game();
        if (!playing) return;
        const char slot = play::newest_save();
        const char* why = nullptr;
        if (!slot) show_notice("Continue", "This game has no saved game yet - start it and save from Encamp.");
        else if (!play::continue_game(slot, cv, &why) && why) show_notice("Continue", why);
        else plat::log("[port] continued from saved game %c\n", slot);
        return;
    }
    if (op == kOpChooserMode) {
        // (the chooser's own toggle - Tiles 1 / Cards 2; the Display page's line shows the same)
        cfg.chooser = static_cast<uint8_t>(value < 0 || value > 2 ? 0 : value);
        settings_save(conf_dir, cfg);
        return;
    }
    if (op == kOpJournalClose) {
        // (the app's journal viewer closed - its close button, Back, a controller's B)
        if (jour::active()) journal_press(BB);
        return;
    }
    if (op == kOpReport) {
        // (the chooser's hold menu: Report a Problem about that game)
#ifdef __ANDROID__
        if (index >= 0 && index < static_cast<int>(games_found.size())) android_report(index);
#endif
        return;
    }
    if (op == kOpChooserSelect) {
        if (screen == Screen::Games && index >= 0 && index < static_cast<int>(games_found.size())) game_sel = index;
        return;
    }
    if (op == kOpLookAgain) {
        if (screen == Screen::Games && !menu_open && !notice.open && games_found.empty()) press(BA);
        return;
    }
    if (notice.open || prompt_open) return;
    // Device Info, the Button Guide (over the menu): Back or close
    if (menu_open && (help_open || device_open)) {
        // (Back a page; the X closes the menu - back to the game: Tom, 2026-11-28)
        if (op == shell::MenuBack) menu_press(BB);
        else if (op == shell::MenuClose) {
            help_open = device_open = about_open = false;
            menu_open = false;
        }
        return;
    }
    if (jlist_open) {
        const int n = play::journal_seen_count();
        const int d0 = n + (n == 0 ? 1 : 0) + 1;         // (the first PDF's row: after the entries, a note, the heading)
        if (op == shell::MenuChoose && index >= 0 && index < n) {
            jlist_sel = index;
            jlist_press(BA);
        } else if (op == shell::MenuChoose && !jlist_docs.empty() && index >= d0 &&
                   index < d0 + static_cast<int>(jlist_docs.size())) {
#ifdef __ANDROID__
            const jour::Document& d = jlist_docs[static_cast<size_t>(index - d0)];
            android_open_pdf(d.path, d.label);
#endif
        } else if (op == shell::MenuBack) {
            jlist_press(BB);
        } else if (op == shell::MenuClose) {
            jlist_open = false;
            menu_open = false;
        }
        return;
    }
    if (quick_open) {
        // (its lines: 0 the On / Off line, 1 the member, 2 - 9 the member's switches)
        if (op == shell::MenuChoose && index == 0) {
            q_col = -1;
            quick_press(BA);
        } else if (op == shell::MenuChoose && index >= 2 && index < 2 + quickacts::kCols) {
            q_col = index - 2;
            quick_press(BA);
        } else if (op == shell::MenuChange && index == 1) {
            quick_press(value < 0 ? BLeft : BRight);
        } else if (op == kOpPick && index == 1) {
            // (the member list: value - how many members on from the one shown)
            if (q_rows > 0) q_row = (q_row + value) % q_rows;
        } else if (op == shell::MenuBack) {
            quick_press(BB);                // (back to the menu - Tom, 2026-11-28)
        } else if (op == shell::MenuClose) {
            quick_press(BStart);
        }
        return;
    }
    if (!menu_open) return;
    const int n = static_cast<int>(menu_items.size());
    const bool line = index >= 0 && index < n;
    if (op == kOpPick) {
        // (a choice line's list: value - how many places on from the value set now)
        if (line) menu_pick(index, value);
        return;
    }
    switch (op) {
    case shell::MenuClose: menu_open = false; break;
    case shell::MenuBack: menu_press(BB); break;
    case shell::MenuChoose:
        if (!line) break;
        menu_sel = index;
        menu_press(BA);
        break;
    case shell::MenuChange:
        if (!line) break;
        menu_sel = index;
        menu_change(value < 0 ? -1 : 1);
        break;
    case shell::MenuSet:
        if (!line) break;
        menu_sel = index;
        if (menu_items[index].m == MVolume) {
            cfg.volume = static_cast<uint8_t>(value < 0 ? 0 : value > 100 ? 100 : value);
            if (playing) apply_sound();
            settings_save(conf_dir, cfg);
        }
        break;
    default: break;
    }
}

// (tabs and line ends out of a field)
void field(std::string& out, const char* s)
{
    for (; s && *s; ++s) out += (*s == '\t' || *s == '\n' || *s == '\r') ? ' ' : *s;
}

void row(std::string& t, char kind, const char* name, const char* value, const char* help)
{
    t += '\n';
    t += kind;
    t += '\t';
    field(t, name);
    t += '\t';
    field(t, value);
    t += '\t';
    field(t, help);
}

// A choice line's value as its line shows it ("Picture: Original 4:3" -> "Original 4:3")
std::string menu_value(int i)
{
    char l[160];
    menu_label(menu_items[i], l, sizeof l);
    const std::string n = l;
    const size_t c = n.rfind(": ");
    return c == std::string::npos ? n : n.substr(c + 2);
}

// A choice line's values for the app's list (Tom, 2026-11-28: "a drop-down list ... not tapping right and left"),
// each with whether it can be chosen here: Game Sound and Game Font in Tom's order (Off, PC Speaker, Tandy, Gateway,
// Treasures, Darkness - a recording greyed without its game; Original, Clear, Clear CAPS, Darkness), the others in
// their values' own order (Small ... Huge) - worked out by stepping in cfg alone (menu_dry: nothing applied or saved)
std::vector<std::pair<std::string, bool>> menu_choice_list(int i)
{
    std::vector<std::pair<std::string, bool>> out;
    if (i < 0 || i >= static_cast<int>(menu_items.size())) return out;
    const MenuItem m = menu_items[i].m;
    if (m == MSound && !own_sound(sound_game())) {
        for (uint8_t v : {kSndOff, kSndPc, kSndTandy, kSndGateway, kSndTreasures, kSndDarkness})
            out.push_back({sound_name(v), sound_ok(v)});
        return out;
    }
    if (m == MFont) {
        bool dark = false;
        for (const auto& g : games_found)
            if (g.game == games::Game::PoolsOfDarkness && !g.hlib) dark = true;
        for (const char* v : {"Original", "Clear", "Clear CAPS", "Darkness"})
            out.push_back({v, strcmp(v, "Darkness") != 0 || dark});
        return out;
    }
    const int keep = menu_sel;
    const Settings was = cfg;
    menu_sel = i;
    menu_dry = true;
    switch (m) {
    case MPadSize: cfg.pad_size = 0; break;
    case MPadAlpha: cfg.pad_alpha = 0; break;
    case MRotation: cfg.rotation = 0; break;
    case MChooser: cfg.chooser = 0; break;
    case MJFont: cfg.journal_font = 0; break;
    case MTouchPad: cfg.touch_pad = 0; break;
    case MDisplay: cfg.fit = 0; break;
    case MClueDark: cfg.clue_dark = true; break;
    default: break;
    }
    out.push_back({menu_value(i), true});
    for (int k = 0; k < 12; ++k) {
        menu_change(1);
        const std::string v = menu_value(i);
        if (v == out[0].first) break;
        out.push_back({v, true});
    }
    menu_dry = false;
    cfg = was;
    menu_sel = keep;
    return out;
}

// The list's text: 0x1F between, a value that can't be chosen here after a 0x02
std::string menu_choices(int i)
{
    std::string t;
    bool first = true;
    for (const auto& c : menu_choice_list(i)) {
        if (!first) t += '\x1f';
        first = false;
        if (!c.second) t += '\x02';
        field(t, c.first.c_str());
    }
    return t;
}

// The app's list chose the value `k` places on from the one set now, in the list's order: the line stepped Right
// to it - in cfg alone but the last step, which applies and saves as a tap on > does
void menu_pick(int i, int k)
{
    const auto list = menu_choice_list(i);
    const int n = static_cast<int>(list.size());
    if (n == 0 || k <= 0) return;
    const std::string now = menu_value(i);
    int cur = 0;
    for (int j = 0; j < n; ++j)
        if (list[j].first == now) cur = j;
    const auto& want = list[(cur + k) % n];
    if (!want.second) return;
    menu_sel = i;
    const Settings was = cfg;
    menu_dry = true;
    int steps = 0;
    for (int s = 1; s <= 12 && !steps; ++s) {
        menu_change(1);
        if (menu_value(i) == want.first) steps = s;
    }
    cfg = was;
    if (!steps) {
        menu_dry = false;
        return;
    }
    for (int s = 1; s < steps; ++s) menu_change(1);
    menu_dry = false;
    menu_change(1);
}

// The drawer's page: a head line - open / busy (the port's own screen is over
// it: the drawer steps aside) / closed, the title, the highlighted line, a
// page above it (1 / 0) - then a line each: kind (P a page or screen, S a
// switch, C a choice - its < and >, L a level 0 - 100, A an action, T a line of
// text only (a name and its words), H a heading, G a group's title (Device Info's games: T lines under it
// indented), X none of the drawer's), its name, its value, its words. Tab-separated.
std::string android_menu_text()
{
    std::string t;
    const bool busy = prompt_open || notice.open || screen == Screen::Scan || screen == Screen::Message;
    auto head = [&](const char* title, int sel, bool above) {
        t = busy ? "busy\t" : "open\t";
        field(t, title);
        t += "\t" + std::to_string(sel) + "\t" + (above ? "1" : "0");
    };
    if (quick_open) {
        const bool on = !(cfg.comforts_off & kOffQuick);
        head("Quick Actions", q_col < 0 ? 0 : q_col + 2, true);      // (its back arrow - Tom, 2026-11-28)
        row(t, 'S', "Quick Actions", on ? "On" : "Off",
            "On: each member's own Quick Actions, set below. Off: Quick plays every member as the original games do.");
        if (on && q_rows > 0) {
            char nm[32] = "", who[48];
            if (!play::quick_name(q_row, nm, sizeof nm)) nm[0] = 0;
            snprintf(who, sizeof who, "%s%s", nm, play::quick_npc(q_row) ? " (NPC)" : "");
            row(t, 'C', "Member", who, "Whose Quick Actions the switches below are.");
            t += '\t';
            for (int r = 0; r < q_rows; ++r) {
                char m[32] = "";
                if (!play::quick_name(r, m, sizeof m)) m[0] = 0;
                if (r) t += '\x1f';
                field(t, m);
                if (play::quick_npc(r)) t += " (NPC)";
            }
            const uint8_t mask = play::quick_get(q_row);
            for (int k = 0; k < quickacts::kCols; ++k)
                row(t, 'S', quickacts::kName[k], mask >> k & 1 ? "On" : "Off", quickacts::kDesc[k]);
        }
        return t;
    }
    if (jlist_open) {
        head("Journal", jlist_sel, true);
        const int n = play::journal_seen_count();
        for (int i = 0; i < n; ++i) {
            char kind, name[48];
            int number;
            if (!play::journal_seen(i, &kind, &number)) continue;
            journal::mention_name(kind, number, name, sizeof name);
            row(t, 'A', name, "", "Opens the entry.");
        }
        if (!jlist_docs.empty()) {
            if (n == 0) row(t, 'H', "No entries yet: each one the game tells you to read is listed here.", "", "");
            row(t, 'H', "Documents", "", "");
            for (const jour::Document& d : jlist_docs)
                row(t, 'A', d.label.c_str(), "", "Opens the PDF, a page at a time.");
        }
        return t;
    }
    if (!menu_open) return "closed";
    if (help_open) {
        head("Button Guide", -1, true);
        // (a line with no key goes on the line before it: "Pointer: D-Pad moves it, Confirm taps, Back ends";
        // a key with no words is a heading - "In a fight:")
        std::vector<std::pair<std::string, std::string>> rows;
        for (const HelpLine& l : help_lines()) {
            if (!l.key[0] && !rows.empty()) rows.back().second += std::string(" ") + l.what;
            else rows.push_back({l.key, l.what});
        }
        for (const auto& r : rows) row(t, r.second.empty() ? 'H' : 'T', r.first.c_str(), r.second.c_str(), "");
        return t;
    }
    if (device_open && about_open) {
        head("About", -1, true);
        for (const auto& l : about_lines()) row(t, 'T', l.first.c_str(), l.second.c_str(), "");
        return t;
    }
    if (device_open) {
        head("Device Info", -1, true);
        for (const auto& l : device_lines()) {
            // (a game's title - or This Device - across the drawer in gold, its lines indented under it: kind G; Tom,
            // 2026-11-28)
            if (!l.first.empty() && l.first[0] == '\x01') row(t, 'G', l.first.c_str() + 1, "", "");
            else row(t, 'T', l.first.c_str(), l.second.c_str(), "");
        }
        return t;
    }
    char title[48];
    snprintf(title, sizeof title, "%s", page_title(menu_page));
    if (menu_page == Page::Fixes) snprintf(title, sizeof title, "Fixes: %s", fix_game_name(fix_game));
    head(title, menu_sel, menu_page != Page::Top);
    for (const MItem& it : menu_items) {
        char label[160];
        menu_label(it, label, sizeof label);
        std::string name = label, value;
        char kind = 'A';
        const size_t colon = name.rfind(": ");
        const bool dots = name.size() > 3 && name.compare(name.size() - 3, 3, "...") == 0;
        if (it.m == MBack) kind = 'X';                    // (the drawer's back arrow)
        else if (it.m == MReplyAddr) kind = 'A';          // (opens the keyboard)
        else if (it.m == MJournal || it.m == MQuick) kind = 'P';   // (open a page of their own)
        else if (it.m == MVolume) {
            kind = 'L';
            name = "Volume";
            value = std::to_string(cfg.volume);
        } else if (page_of(it.m) != Page::Top || dots) {
            kind = 'P';
            if (dots) name.resize(name.size() - 3);
        } else if (colon != std::string::npos) {
            value = name.substr(colon + 2);
            name.resize(colon);
            kind = value == "On" || value == "Off" ? 'S' : 'C';
        }
        row(t, kind, name.c_str(), value.c_str(), menu_help(it));
        if (kind == 'C') t += "\t" + menu_choices(static_cast<int>(&it - menu_items.data()));
    }
    return t;
}

// A notice (or a message page): "1", its title, Confirm's word ("" none),
// Back's word, waiting (1: a job runs - no buttons yet); then its words. "0": none.
std::string android_notice_text()
{
    std::string t;
    if (notice.open) {
        t = "1\t";
        field(t, notice.title.c_str());
        t += "\t";
        field(t, notice.yes ? notice.yes : "");
        t += "\t";
        field(t, notice.no ? notice.no : (notice.yes ? "" : "Close"));
        t += job != Job::None ? "\t1\n" : "\t0\n";
        t += notice.body;
    } else if (screen == Screen::Message) {
        t = "1\t";
        field(t, msg_title.c_str());
        t += "\tOK\t\t0\n";
        t += msg_body;
    } else {
        t = "0";
    }
    return t;
}

// The game chooser (screen Games): "games", the chosen one, then a line a
// game - its name, short name, playable (1 / 0), its picture's file as the
// phone has it ("" none), the picture's kind (2 box art, 1 logo, 0 none), its
// release, Continue (1: it has a saved game - CONTINUE.TXT); with none found: "none", the folder as the phone shows it. "off":
// not on the chooser.
std::string android_chooser_text()
{
    if (screen != Screen::Games || playing) return "off";
    std::string t;
    if (games_found.empty()) {
        t = have_root ? "none\t" : "nofolder\t";
        field(t, (shown_path(pm::games_root()) + "/").c_str());
        return t;
    }
    t = "games\t" + std::to_string(game_sel) + "\t" + std::to_string(cfg.chooser % 3);
    for (const library::GameDir& g : games_found) {
        const std::string art = app_art_path(g);
        struct stat st;
        const bool there = stat(pm::host_path(art.c_str()).c_str(), &st) == 0 && st.st_size > 0;
        std::string dummy;
        const int kind = !there ? 0 : art::box_file(g, dummy) ? 2 : 1;
        t += '\n';
        field(t, g.game == games::Game::Unknown ? g.folder.c_str() : games::title(g.game));
        t += '\t';
        field(t, g.game == games::Game::Unknown ? g.folder.c_str() : games::short_title(g.game));
        t += play::available(g.game) ? "\t1\t" : "\t0\t";
        if (there) field(t, pm::host_path(art.c_str()).c_str());
        t += "\t" + std::to_string(kind) + "\t";
        field(t, release_name(g));
        t += plat::exists((library::cache_dir(g) + "/CONTINUE.TXT").c_str()) ? "\t1" : "\t0";
        t += '\t';
        field(t, g.folder.c_str());     // (its folder: the app's home-screen shortcut opens it - --game-folder)
    }
    return t;
}

// The scan's page: "scan", its title, done (1 / 0); then its lines. "off": not up.
std::string android_scan_text()
{
    if (screen != Screen::Scan) return "off";
    std::string t = "scan\t";
    field(t, scan_title);
    t += scan_done ? "\t1" : "\t0";
    for (const std::string& l : scan_log) {
        t += '\n';
        field(t, l.c_str());
    }
    return t;
}

void android_ui_tick()
{
    if (!shell::on()) return;
    shell::text(shell::ChDrawer, android_menu_text());
    shell::text(shell::ChNotice, android_notice_text());
    shell::text(shell::ChChooser, android_chooser_text());
    shell::text(shell::ChScan, android_scan_text());
    shell::pad_style(cfg.pad_size % 4, 100);
}

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
        else if (b == BX) prompt_key('\x18');
        else if (b <= BRight) keys_move(b == BLeft ? -1 : b == BRight ? 1 : 0, b == BUp ? -1 : b == BDown ? 1 : 0);
        return;
    }
    if (jlist_open) {
        jlist_press(b);
        return;
    }
    if (quick_open) {
        quick_press(b);
        return;
    }
    if (screen == Screen::Scan) {
        if (scan_done && (b == BA || b == BB || b == BStart)) {
            screen = Screen::Games;
            // (the Android app: Rescan / Clean Up came from the menu - back to it, on the page it was on)
            if (scan_from_menu) {
                menu_open = true;
                menu_build();
            }
            scan_from_menu = false;
        }
        return;
    }
    if (menu_open) {
        menu_press(b);
        return;
    }
    if (jour::active() && b != BStart) {
        journal_press(b);
        return;
    }
    if (b == BStart) {
        ptr_mode = false;
        menu_page = Page::Top;
        menu_sel = 0;
        menu_top = 0;
        menu_build();
        menu_open = true;
        help_open = device_open = false;
        return;
    }
    if (screen == Screen::Message) {
        if (b == BA || b == BB) screen = playing ? Screen::Play : msg_back == Screen::Play ? Screen::Games : msg_back;
        return;
    }
    if (screen == Screen::Games) {
        const int n = static_cast<int>(games_found.size());
        if (b == BA && n == 0) {
            rescan();
            discover(false);
        } else if (b == BA) start_game();
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
    case BY:
        if (shell::on()) shell::companion();        // (the Android app's Companion: out / away)
        else panel::next();
        break;
    // (Champions' / the Dark Queen's overland, walking: north-west / north-east, as in a fight)
    case BL1:
        if (fight) play::fight_dir(7, cv);
        else if (cfg.pad_moves && play::walking() && play::move_keys() == play::MoveKeys::Steps6) play::act(play::Act::TurnLeft, cv);
        else play::nav(play::Nav::Left, cv);
        break;
    case BR1:
        if (fight) play::fight_dir(1, cv);
        else if (cfg.pad_moves && play::walking() && play::move_keys() == play::MoveKeys::Steps6) play::act(play::Act::TurnRight, cv);
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
        if (!down) {
            // A tap let go before its step was taken (a fight waits 70 ms for a
            // diagonal's second direction; a tap can be shorter than that, or
            // than a frame): the step is taken now, with what was held
            const bool pointing = ptr_mode && screen == Screen::Play && !menu_open;
            if (held[b] && last_dir >= 0 && !fired_once && !pointing) {
                fired_once = true;
                press(last_dir);
            }
            held[b] = false;
            if (last_dir == b) last_dir = -1;
            return;
        }
        held[b] = true;
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

// ---- Touching the port's screens ------------------------------------------
// A tap at (cx, cy) - canvas pixels, a wide page's margins outside 0..319 -
// as the buttons the player would press there: a line of a list is picked
// and chosen at once (Tom, 2026-11-23: "It should only take one tap - you
// know, be more Android-friendly"); outside a page's box is Back. In the game a
// tap is the game's own (pointer_tap), as on the CYD boards.
void presses(int b, int n)
{
    for (int i = 0; i < n; ++i) press(b);
}

void touch_tap(int cx, int cy)
{
    if (cy < 0 || cy >= video::kH) return;
    const bool in_page = cx >= 8 && cx < 312 && cy >= 2 && cy < 198;     // (the menus' box)
    if (notice.open) {
        if (job != Job::None) return;
        if (!notice.yes) press(BA);
        else if (cx < 20 || cx >= 300 || cy < 44 || cy >= 156) press(BB);      // (outside the box: No)
        else if (cy >= 136) press(cx < video::kW / 2 ? BA : BB);                // (its foot: Yes at the left, No at the right)
        return;
    }
    if (prompt_open) {
        for (size_t i = 0; i < keys.size(); ++i) {
            const Key& k = keys[i];
            if (cx >= k.col * 8 && cx < (k.col + k.w) * 8 && cy >= k.row * 8 - 4 && cy < k.row * 8 + 12) {
                key_sel = static_cast<int>(i);
                press(BA);
                return;
            }
        }
        return;
    }
    if (jlist_open) {
        if (cx < 60 || cx >= 260 || cy < 20 || cy >= 180) {
            press(BB);
            return;
        }
        const int n = play::journal_seen_count(), rows = 16;
        int first = jlist_sel - rows / 2;
        if (first > n - rows) first = n - rows;
        if (first < 0) first = 0;
        const int i = first + (cy + 1) / 8 - 5;
        if (cy + 1 >= 40 && i >= first && i < n && i < first + rows) {
            jlist_sel = i;
            press(BA);
        }
        return;
    }
    if (quick_open) {
        if (!in_page) {
            press(BB);
            return;
        }
        if (cy >= 19 && cy < 30) {
            q_col = -1;
            press(BA);
            return;
        }
        if (cfg.comforts_off & kOffQuick) return;           // (off: the rows are dimmed)
        const int r = (cy - 34) / 11;
        if (cy < 34 || r < 0) return;
        if (cx < 162) {
            if (r < q_rows && r < 9) q_row = r;
        } else if (r < quickacts::kCols) {
            q_col = r;
            press(BA);
        }
        return;
    }
    if (screen == Screen::Scan) {
        press(BA);
        return;
    }
    if (menu_open) {
        if (help_open || device_open || !in_page || cy < 17) {
            press(BB);          // (a page of words, outside the box, or its title: back)
            return;
        }
        const int n = static_cast<int>(menu_items.size());
        if (cy >= 19 && cy < 19 + kMenuLines * kMenuPitch) {
            const int i = menu_top + (cy - 19) / kMenuPitch;
            if (i < n) {
                menu_sel = i;
                press(BA);
            }
        }
        return;
    }
    if (jour::active()) {
        press(cx < video::kW / 3 ? BL1 : BA);       // (the left third: a page back; else on, or closed at its end)
        return;
    }
    if (screen == Screen::Message) {
        press(BA);
        return;
    }
    if (screen == Screen::Games) {
        const int n = static_cast<int>(games_found.size());
        if (cy < kHeaderH) press(BStart);                       // (the title bar: the menu)
        else if (n > 1 && cx < 48) press(BLeft);                // (the arrowheads' sides: the next / last game)
        else if (n > 1 && cx >= video::kW - 48) press(BRight);
        else press(BA);
        return;
    }
    if (screen == Screen::Play && cx >= 0 && cx < video::kW) {
        ptr_on = false;
        pointer_tap(cx, cy);
    }
}

// A swipe by (dx, dy) canvas pixels: lists scroll (the finger up = on down
// the list), across = Left / Right (a setting's value, the next game, a
// journal page)
void touch_swipe(int dx, int dy)
{
    const bool across = (dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy);
    const int ady = dy < 0 ? -dy : dy;
    if (notice.open || prompt_open || screen == Screen::Scan) return;
    if (jlist_open) {
        if (!across) presses(dy < 0 ? BDown : BUp, ady / 8 > 0 ? ady / 8 : 1);
        return;
    }
    if (quick_open) {
        if (across) press(dx < 0 ? BR1 : BL1);
        else presses(dy < 0 ? BDown : BUp, ady / 11 > 0 ? ady / 11 : 1);
        return;
    }
    if (menu_open) {
        if (help_open || device_open) return;
        if (across) press(dx > 0 ? BRight : BLeft);
        else presses(dy < 0 ? BDown : BUp, ady / kMenuPitch > 0 ? ady / kMenuPitch : 1);
        return;
    }
    if (jour::active()) {
        if (across) press(dx < 0 ? BR1 : BL1);
        else presses(dy < 0 ? BDown : BUp, ady / 8 > 0 ? ady / 8 : 1);
        return;
    }
    if (screen == Screen::Games && across) press(dx < 0 ? BRight : BLeft);
}

// The fingers not on the on-screen buttons: one = a tap or a swipe as it
// lifts; two together, lifted soon = Start
struct {
    int          free = 0;          // fingers down, not on the buttons
    bool         on = false;        // the first one's
    SDL_FingerID id = 0;
    float        x = 0, y = 0;      // where it went down (0..1 of the window)
    Uint32       t = 0;
    bool         two = false;
} touch;

void touch_event(const SDL_Event& e)
{
    int ow, oh;
    video::output_size(ow, oh);
    if (e.type == SDL_FINGERDOWN) {
        if (!touch_last) {
            touch_last = true;
            touch_pad_apply();
        }
    }
    if (touchpad::finger(e, ow, oh)) return;        // (an on-screen button's)
    if (e.type == SDL_FINGERDOWN) {
        if (touch.free++ == 0) {
            touch.on = true;
            touch.id = e.tfinger.fingerId;
            touch.x = e.tfinger.x;
            touch.y = e.tfinger.y;
            touch.t = SDL_GetTicks();
            touch.two = false;
        } else {
            touch.two = true;
        }
        return;
    }
    if (e.type != SDL_FINGERUP) return;
    if (touch.free > 0) --touch.free;
    if (touch.two) {
        if (touch.free == 0) {
            touch.on = false;
            if (SDL_GetTicks() - touch.t < 700) press(BStart);
        }
        return;
    }
    if (!touch.on || e.tfinger.fingerId != touch.id) return;
    touch.on = false;
    int ww = 1, wh = 1;
    video::window_size(ww, wh);
    const int x0 = static_cast<int>(touch.x * ww), y0 = static_cast<int>(touch.y * wh);
    const int x1 = static_cast<int>(e.tfinger.x * ww), y1 = static_cast<int>(e.tfinger.y * wh);
    const int shortest = ww < wh ? ww : wh;
    const int moved = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    int cx0, cy0, cx1, cy1;
    video::to_canvas(x0, y0, cx0, cy0, false);
    if (moved < (shortest / 25) * (shortest / 25)) {
        touch_tap(cx0, cy0);
        return;
    }
    video::to_canvas(x1, y1, cx1, cy1, false);
    touch_swipe(cx1 - cx0, cy1 - cy0);
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
#ifdef __ANDROID__
    case SDLK_AC_BACK: return BB;       // (a phone's Back button / gesture)
#endif
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

// A button of the Android app's pad (shell.h): as the controller's, A / B
// never swapped (the pad's OK is OK whatever Settings - Confirm Button says)
void shell_press(int b, bool down)
{
    if (b < 0 || b >= BCount) return;
    button(mapped(b), down);
}

void handle(const SDL_Event& e)
{
    if (shell::take(e, shell_press)) {
        touch_pad_apply();
        return;
    }
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
    // A touch screen: the on-screen buttons (touchpad.h) - shown at any touch,
    // hidden while a controller is in use
    case SDL_FINGERDOWN:
    case SDL_FINGERMOTION:
    case SDL_FINGERUP:
        if (touch_ui) touch_event(e);
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        // (a phone: the app put away - nothing left held down, the settings kept)
        touchpad::release_all();
        memset(held, 0, sizeof held);
        settings_save(conf_dir, cfg);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP: {
        if (e.type == SDL_CONTROLLERBUTTONDOWN && touch_last) {
            touch_last = false;
            touch_pad_apply();
        }
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
        if (down && touch_last && e.key.keysym.sym != SDLK_AC_BACK) {
            touch_last = false;                 // (a real keyboard)
            touch_pad_apply();
        }
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
        // (a touch screen: the finger's own events say it - touch_event; a PC's
        // --touchpad: the mouse is the finger)
        if (touch_ui) {
#ifdef __ANDROID__
            if (e.button.which == SDL_TOUCH_MOUSEID) break;
#else
            break;
#endif
        }
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

// ---- Driven from outside (--drive; tools/play_test.py) -----------------------
// Steps come on stdin, a line at a time (the script's steps, commas between);
// when a line's steps are done the port answers with what it is showing: a
// "[state]" line and the game screen READ AS TEXT - each 8 x 8 cell matched
// against the engine's Clear font (the test sets Game Font: Clear), with a
// second line saying how each cell is lit (H: boxed / inverse - the
// highlight; else its ink's colour as a hex digit). A test program can so
// find a menu's words and see where the highlight is, as a player would.
bool        drive = false, drive_owes = false;
std::string drive_in;

void drive_text()
{
    // (the glyphs to match: every printable character the font has)
    static uint64_t glyphs[127];
    static bool     made = false;
    if (!made) {
        made = true;
        for (int ch = 33; ch < 127; ++ch) {
            const uint8_t* g = video::glyph(static_cast<char>(ch));
            uint64_t v = 0;
            for (int j = 0; j < 8; ++j) v = v << 8 | g[j];
            glyphs[ch] = v;
        }
    }
    // A cell at any pixel place: its letter ('?' unread, ' ' empty) and its mark
    auto cell = [&](int x0, int y0, char& out, char& mark) {
        // The cell's paper: the colour most of it has; its ink: the rest
        int count[256] = {};
        uint8_t px[64];
        for (int j = 0; j < 8; ++j)
            for (int i = 0; i < 8; ++i) ++count[px[j * 8 + i] = cv.get(x0 + i, y0 + j)];
        int paper = 0, ink = -1;
        for (int v = 1; v < 256; ++v)
            if (count[v] > count[paper]) paper = v;
        uint64_t bits = 0;
        for (int k = 0; k < 64; ++k) {
            bits = bits << 1 | (px[k] != paper ? 1u : 0u);
            if (px[k] != paper) ink = px[k];
        }
        char ch = bits == 0 ? ' ' : '?';
        for (int c = 33; c < 127 && ch == '?'; ++c)
            if (glyphs[c] == bits) ch = static_cast<char>(c);
        // (a heavy letter - M, N, W - fills more than half its cell: the ink is the common colour)
        for (int c = 33; c < 127 && ch == '?'; ++c)
            if (glyphs[c] == ~bits) {
                ch = static_cast<char>(c);
                const int was = paper;
                paper = ink;
                ink = was;
            }
        // (text over a picture - Treasures, the Dark Queen: the paper is many
        //  colours, the letter one. A colour whose pixels alone are a letter's.)
        bool over = false;
        for (int v = 0; v < 256 && ch == '?'; ++v) {
            if (count[v] < 8 || count[v] > 40) continue;
            uint64_t b2 = 0;
            for (int k = 0; k < 64; ++k) b2 = b2 << 1 | (px[k] == v ? 1u : 0u);
            for (int c = 33; c < 127; ++c)
                if (glyphs[c] == b2) {
                    ch = static_cast<char>(c);
                    ink = v;
                    over = true;
                    break;
                }
        }
        // (white paper: a boxed word - the highlight. The Dark Queen's keys have a paper of their own.)
        const uint32_t rgb = video::palette(paper);
        const bool white = (rgb >> 16 & 255) + (rgb >> 8 & 255) + (rgb & 255) > 600;
        const bool boxed = !over && white && (ch != '?' || bits == 0);
        out = ch;
        mark = boxed ? 'H' : ch == ' ' || ink < 0 ? '.' : "0123456789abcdef"[ink & 15];
    };
    int unread = 0;
    for (int row = 0; row < 25; ++row) {
        char text[41], lit[41];
        for (int col = 0; col < 40; ++col) {
            cell(col * 8, row * 8, text[col], lit[col]);
            if (text[col] == '?') ++unread;
        }
        text[40] = lit[40] = 0;
        printf("T|%s|\nL|%s|\n", text, lit);
    }
    // Text that isn't on the 8-pixel grid (the Dark Queen's party menu: its
    // keys stand where its picture has them): lines read at any pixel place,
    // after the 25 - only on a screen the grid left mostly unread
    if (unread < 200) return;
    for (int y = 0; y <= 192; ++y) {
        int  best = 0;
        char bt[41], bl[41];
        for (int xo = 0; xo < 8; ++xo) {
            if (y % 8 == 0 && xo == 0) continue;
            char text[41], lit[41];
            int  got = 0, run = 0, longest = 0;
            for (int col = 0; col < 40; ++col) {
                text[col] = '?';
                lit[col] = '.';
                if (col * 8 + xo + 8 <= 320) cell(col * 8 + xo, y, text[col], lit[col]);
                const bool letter = text[col] != '?' && text[col] != ' ';
                // (a boxed space belongs to a boxed word; a plain one over a picture reads '?')
                run = letter || (text[col] == ' ' && run > 0) ? run + 1 : 0;
                if (letter) ++got;
                if (run > longest) longest = run;
            }
            if (longest >= 4 && got > best) {
                best = got;
                text[40] = lit[40] = 0;
                memcpy(bt, text, 41);
                memcpy(bl, lit, 41);
            }
        }
        if (best >= 4) {
            printf("T|%s|\nL|%s|\n", bt, bl);
            y += 7;
        }
    }
}

// (the game screen's pixels as one number: has the picture changed?)
uint32_t canvas_sum()
{
    uint32_t h = 2166136261u;
    for (int y = 0; y < 200; ++y)
        for (int x = 0; x < 320; ++x) h = (h ^ cv.get(x, y)) * 16777619u;
    return h;
}

void drive_answer()
{
    char l1[64] = "", l2[64] = "";
    if (playing) play::describe(l1, l2, sizeof l1);
    printf("[state] screen=%s playing=%d walking=%d fighting=%d pad=%d input=%d journal=%d jlist=%d menu=%d quick=%d notice=%d "
           "panel=%d dock=%d title=%d canvas=%08x pos=%d,%d,%d desc=%s|%s\n",
           screen == Screen::Games ? "games" : screen == Screen::Play ? "play" : screen == Screen::Scan ? "scan" : "message",
           playing ? 1 : 0, playing && play::walking() ? 1 : 0, playing && play::fighting() ? 1 : 0,
           playing && play::fight_pad() ? 1 : 0, playing ? static_cast<int>(play::input()) : 0, jour::active() ? 1 : 0,
           jlist_open ? 1 : 0, menu_open ? 1 : 0, quick_open ? 1 : 0, notice.open ? 1 : 0, panel::open() ? 1 : 0, panel::docked_on() ? 1 : 0, in_title() ? 1 : 0, canvas_sum(),
           playing ? play::pos_x() : 0, playing ? play::pos_y() : 0, playing ? play::dir() : 0, l1, l2);
    {
        // (what the ENGINE has switched on now - asked of the engine, not of the port's settings:
        // tools/switch_test.py flips a menu line in a game and reads these)
        unsigned long long fx = 0;
        for (int k = 0; k < rules::kFixCount; ++k)
            if (rules::fix(static_cast<rules::Fix>(k))) fx |= 1ull << k;
        unsigned ad = 0, en = 0;
        for (int k = 0; k < 16; ++k)
            if (rules::adnd(static_cast<uint16_t>(1u << k))) ad |= 1u << k;
        for (int k = 0; k < 8; ++k)
            if (rules::enh(static_cast<uint8_t>(1u << k))) en |= 1u << k;
        printf("[engine] fix=%llx adnd=%x enh=%x\n", fx, ad, en);
    }
    for (size_t i = 0; menu_open && i < menu_items.size(); ++i) {
        char label[40];
        menu_label(menu_items[i], label, sizeof label);
        printf("M|%c%s\n", static_cast<int>(i) == menu_sel ? '>' : ' ', label);
    }
    if (playing) drive_text();
    printf("[end]\n");
    fflush(stdout);
}

// More steps from stdin (never waits)
void drive_read()
{
    char buf[512];
    for (;;) {
        const ssize_t n = read(0, buf, sizeof buf);
        if (n <= 0) {
            if (n == 0 && drive_in.empty() && script_at >= script.size() && !drive_owes) quit = true;     // (stdin closed)
            break;
        }
        drive_in.append(buf, static_cast<size_t>(n));
    }
    size_t nl;
    while (script_at >= script.size() && !drive_owes && (nl = drive_in.find('\n')) != std::string::npos) {
        const std::string line = drive_in.substr(0, nl);
        drive_in.erase(0, nl + 1);
        script.clear();
        script_at = 0;
        size_t p0 = 0;
        while (p0 <= line.size()) {
            const size_t c = line.find(',', p0);
            const std::string t = line.substr(p0, c == std::string::npos ? std::string::npos : c - p0);
            if (!t.empty()) script.push_back(t);
            if (c == std::string::npos) break;
            p0 = c + 1;
        }
        drive_owes = true;
    }
}

void script_tick(Uint32 now)
{
    if (drive) {
        drive_read();
        if (drive_owes && script_at >= script.size() && static_cast<Sint32>(now - script_next) >= 0 && job == Job::None) {
            drive_owes = false;
            drive_answer();
        }
    }
    if (script_at >= script.size() || static_cast<Sint32>(now - script_next) < 0) return;
    if (job != Job::None) return;                   // (a network job runs: the script waits for it)
    const std::string& t = script[script_at++];
    script_next = now + 150;
    if (t.compare(0, 5, "wait:") == 0) {
        script_next = now + static_cast<Uint32>(atoi(t.c_str() + 5));
    } else if (t.compare(0, 5, "shot:") == 0) {
        draw_frame();
        if (!video::save_bmp(t.c_str() + 5)) plat::log("[script] couldn't write %s\n", t.c_str() + 5);
    } else if (t.compare(0, 3, "op:") == 0) {
        // (op:OP:INDEX:VALUE - an Android app's tap on its views, as nativeMenu sends it)
        int op = 0, i = 0, v = 0;
        if (sscanf(t.c_str() + 3, "%d:%d:%d", &op, &i, &v) >= 2) android_op(op, i, v);
    } else if (t == "snapshot") {
        // (the Android app's Report a Problem's save, checked anywhere: into games/_CYD/_REPORT/SAVE)
        const std::string dir = std::string(games::kRootDir) + "/" + library::kCacheDir + "/_REPORT/SAVE";
        mkdir(pm::host_path((std::string(games::kRootDir) + "/" + library::kCacheDir + "/_REPORT").c_str()).c_str(), 0755);
        plat::log("[script] snapshot: %s\n", playing && play::snapshot(dir.c_str()) ? "made" : "none");
    } else if (t.compare(0, 4, "tap:") == 0) {
        int x = 0, y = 0;
        if (sscanf(t.c_str() + 4, "%d:%d", &x, &y) == 2) pointer_tap(x, y);
    } else if (t.compare(0, 6, "touch:") == 0) {
        // (touch:x:y - a finger's tap on the port's screens, canvas pixels; swipe:dx:dy)
        int x = 0, y = 0;
        if (sscanf(t.c_str() + 6, "%d:%d", &x, &y) == 2) touch_tap(x, y);
    } else if (t.compare(0, 6, "swipe:") == 0) {
        int x = 0, y = 0;
        if (sscanf(t.c_str() + 6, "%d:%d", &x, &y) == 2) touch_swipe(x, y);
    } else if (t.compare(0, 5, "type:") == 0) {
        for (const char* p = t.c_str() + 5; *p; ++p) {
            if (prompt_open) prompt_key(*p);
            else if (playing) play::input_key(*p == '_' ? ' ' : *p, cv);
        }
    } else if (t.compare(0, 8, "journal:") == 0 && t.size() > 10 && playing) {
        // (journal:J:12 - that entry opened, as if the game had just mentioned it)
        journal_queue.push_back(JRef{t[8], atoi(t.c_str() + 10)});
        journal_next();
    } else if (t == "continue") {
        // (the Android chooser's Continue in an open game: its newest save, past the titles - to check it here)
        const char slot = play::newest_save();
        const char* why = nullptr;
        const bool ok = slot && play::continue_game(slot, cv, &why);
        fprintf(stderr, "[continue] slot %c: %s%s\n", slot ? slot : '-', ok ? "in the game" : "not: ", ok ? "" : why ? why : "no save");
    } else if (t == "menutext") {
        // (the Android drawer's view of the open menu page, to the log - to check it off the phone)
        fprintf(stderr, "[menutext] %s\n[/menutext]\n", android_menu_text().c_str());
    } else if (t.compare(0, 7, "layout:") == 0) {
        // (layout:gx:gy:gw:gh:cx:cy:cw:ch:view - the Android app's layout (shell.h), to look at off the phone)
        int v[9] = {};
        if (sscanf(t.c_str() + 7, "%d:%d:%d:%d:%d:%d:%d:%d:%d", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7],
                   &v[8]) == 9)
            video::set_shell(SDL_Rect{v[0], v[1], v[2], v[3]}, SDL_Rect{v[4], v[5], v[6], v[7]}, v[8]);
    } else if (t.compare(0, 7, "resize:") == 0) {
        int w = 0, h = 0;
        if (sscanf(t.c_str() + 7, "%dx%d", &w, &h) == 2) video::set_window(w, h);
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
    } else if (t.find('+') != std::string::npos) {
        // two directions together (a fight's diagonal): up+right
        int one = -1, two = -1;
        const std::string x = t.substr(0, t.find('+')), y = t.substr(t.find('+') + 1);
        for (int b = 0; b <= BRight; ++b) {
            if (x == kBtnNames[b]) one = b;
            if (y == kBtnNames[b]) two = b;
        }
        if (one >= 0 && two >= 0) {
            held[one] = held[two] = true;
            press(two);
            held[one] = held[two] = false;
        } else plat::log("[script] unknown step '%s'\n", t.c_str());
    } else {
        bool found = false;
        for (int b = 0; b < BCount; ++b)
            if (t == kBtnNames[b]) {
                if (b == BSelect) {
                    button(b, true);
                    button(b, false);
                } else if (b <= BRight) {
                    // (a direction: held while it acts, as a real press - a fight's pad reads what is held)
                    select_alone = false;
                    held[b] = true;
                    press(mapped(b));
                    held[b] = false;
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
           "  --ab-reversed    this firmware gives A and B reversed: Confirm starts on B (once)\n"
           "  --touchpad       on-screen buttons (as on a phone; the mouse is a finger)\n"
           "  --script <steps> inputs to play back (checks)\n",
           PM_VERSION);
}

} // namespace


int main(int argc, char** argv)
{
    int win_w = 0, win_h = 0, auto_game = 0;
    std::string auto_folder;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        const char* v = i + 1 < argc ? argv[i + 1] : nullptr;
        if (a == "--games" && v) pm::games_root() = argv[++i];
        else if (a == "--conf" && v) conf_dir = argv[++i];
        else if (a == "--app-version" && v) app_version = argv[++i];
        else if (a == "--tools" && v) tools_dir = argv[++i];
        else if (a == "--window" && v) sscanf(argv[++i], "%dx%d", &win_w, &win_h);
        else if (a == "--game" && v) auto_game = atoi(argv[++i]);
        else if (a == "--game-folder" && v) auto_folder = argv[++i];  // (the Android app's home-screen shortcut)
        else if (a == "--drive") drive = true;
        else if (a == "--ab-reversed") ab_reversed = true;
        else if (a == "--touchpad") touch_ui = true;
        else if (a == "--log" && v) {
            // (Android: no console - the log goes in a file beside the games)
            const char* path = argv[++i];
            if (freopen(path, "w", stdout)) setvbuf(stdout, nullptr, _IOLBF, 0);
            if (!freopen(path, "a", stderr)) {}
        }
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

#ifdef __ANDROID__
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");        // (Back is the B button, not "close the app")
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");       // (not a controller)
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait");
#endif
    if (touch_ui) SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");     // (a PC's mouse as a finger)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL: %s\n", SDL_GetError());
        return 1;
    }
    shell::begin();
    const Uint32 t_sdl = SDL_GetTicks();     // (the start's times, in the log: where a slow start goes)
    shell::set_menu(android_op);
#ifdef __ANDROID__
    plat::log("[port] Gold Box Game Engine for Android: app %s, front end %s, engine %s; games in %s\n", PM_VERSION,
              PM_FRONT_VERSION, CYD_VERSION, pm::games_root().c_str());
#else
    plat::log("[port] Gold Box Game Engine for PortMaster %s; games in %s\n", PM_VERSION, pm::games_root().c_str());
#endif
    settings_load(conf_dir, cfg);
#ifdef __ANDROID__
    if (!cfg.fit_set) {
        // (Tom, 2026-11-23: the Companion beside the game "by default, but changeable in the display settings" -
        // set once; the player's own choice stands after)
        cfg.fit = static_cast<uint8_t>(video::Fit::Right);
        cfg.fit_set = true;
        settings_save(conf_dir, cfg);
    }
#endif
    if (ab_reversed && !cfg.ab_set) {
        cfg.swap_ab = cfg.ab_set = true;
        settings_save(conf_dir, cfg);
    }
    if (!video::begin(win_w, win_h)) {
        fprintf(stderr, "SDL video: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    video::set_fit(static_cast<video::Fit>(cfg.fit));
    const Uint32 t_video = SDL_GetTicks();
    video::set_wide(true);
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) pm::audio_begin();
    else plat::log("[port] no sound: %s\n", SDL_GetError());
    open_pad();
    const Uint32 t_sound = SDL_GetTicks();
    touchpad::set_emit(button);
    touch_last = pad == nullptr;
    pad_style_apply();
    touchpad::set_hidden(cfg.pad_hidden);
    touch_pad_apply();
    rotation_apply();       // (after the window: SDL sets its own as it makes one)
    rescan(true);       // (what the last scan found - Tom, 2026-11-27: no scanning when the app opens)
    const Uint32 t_scan = SDL_GetTicks();
    plat::log("[port] %d game folder(s) found\n", static_cast<int>(games_found.size()));
    for (const auto& g : games_found)
        plat::log("[port]   %s: %s (%d files in games/%s)%s\n", g.folder.c_str(), games::title(g.game), g.files,
                  g.data_dir.c_str(), play::available(g.game) ? "" : " - not playable");
    // (a handheld's panel can report its real size only after the first
    // frames: let it settle before anything is made for its shape)
    // (the Android app: asked for its layout first, so the scan's page is its own - up to 1.5 s)
    shell::state(shell::State::Pages);
    for (int k = 0; k < 6 || (k < 75 && shell::wanted() && !shell::on()); ++k) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) handle(e);
        video::resized();
        video::clear(video::kBlack);
        video::present(nullptr);
        SDL_Delay(20);
    }
    // (new games now; the pictures of games found before are checked by the
    // game list once the panel's shape has stood still - pictures_follow)
    const Uint32 t_app = SDL_GetTicks();
    if (drive) fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);
    discover(false, false);
    plat::log("[port] start (ms): SDL %u, video %u, sound %u, the games found %u, the app's views %u, the games' files "
              "checked %u\n", t_sdl, t_video - t_sdl, t_sound - t_video, t_scan - t_sound, t_app - t_scan,
              SDL_GetTicks() - t_app);
    // (a game by its folder - a home-screen shortcut's: the game's start as Play's; gone, the game list)
    for (size_t g = 0; !auto_folder.empty() && g < games_found.size(); ++g)
        if (games_found[g].folder == auto_folder) auto_game = static_cast<int>(g) + 1;
    if (auto_game >= 1 && auto_game <= static_cast<int>(games_found.size())) {
        screen = Screen::Games;         // (--game: straight in, the scan's list not waited at)
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
        safe_check(now);
        if (touch_ui && touchpad::hidden() != cfg.pad_hidden) {
            cfg.pad_hidden = touchpad::hidden();        // (the arrow at the top left: kept for next time)
            settings_save(conf_dir, cfg);
        }
        // (the on-screen D-Pad's corners: diagonals while a fighter moves or aims)
        touchpad::set_diagonals(touch_ui && playing && screen == Screen::Play && !menu_open && keys.empty() &&
                                play::fight_pad());
        android_ui_tick();
        // (the Android app: what is on screen - its Companion and pad follow; nothing elsewhere)
        shell::state(!playing ? shell::State::Pages
                     : in_title() ? shell::State::Titles
                     : (screen == Screen::Play && !menu_open && keys.empty() && play::fight_pad()) ? shell::State::Fight
                                                                                                 : shell::State::Play);
        pointer_tick(dt > 0.1f ? 0.1f : dt);
        if (ptr_on && now - ptr_seen > 6000) ptr_on = false;        // (left alone: out of the way)
        script_tick(now);
        job_tick();
        pictures_follow();
        if (restart_wanted) quit = true;
        if (playing && screen == Screen::Play && !menu_open && !jour::active()) {
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
