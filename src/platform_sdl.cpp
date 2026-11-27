// The platform layer on SDL2 / POSIX (host.h)
#include "host.h"

#include "platform/file.h"
#include "platform/memlog.h"
#include "platform/platform.h"

#include <SDL.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <mutex>
#include <unordered_map>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

namespace pm {

std::string& games_root()
{
    static std::string r = "games";
    return r;
}

namespace {

// dir + "/" + name, with name as the folder really spells it
std::string step(const std::string& dir, const std::string& name)
{
    const std::string exact = dir + "/" + name;
    struct stat st;
    if (stat(exact.c_str(), &st) == 0) return exact;
    if (DIR* d = opendir(dir.c_str())) {
        while (dirent* e = readdir(d))
            if (strcasecmp(e->d_name, name.c_str()) == 0) {
                const std::string found = dir + "/" + e->d_name;
                closedir(d);
                return found;
            }
        closedir(d);
    }
    return exact;       // (not there: a file or folder about to be made)
}

} // namespace

std::string host_path(const char* p)
{
    if (strncmp(p, "/GOLDBOX", 8) != 0 || (p[8] != '/' && p[8] != 0)) return p;
    // (the folders on the way, once found, remembered: each look a slow step through Android's shared storage,
    // and the scan opens the same folders again and again - only a folder, never the last part, which may be a
    // file about to be made)
    static std::mutex m;
    static std::unordered_map<std::string, std::string> found;
    std::string out = games_root();
    const char* s = p + 8;
    while (*s) {
        while (*s == '/') ++s;
        const char* e = s;
        while (*e && *e != '/') ++e;
        if (e > s) {
            const bool last = *e == 0 || (*e == '/' && e[1] == 0);
            const std::string key = out + "/" + std::string(s, e);
            if (last) {
                out = step(out, std::string(s, e));
            } else {
                std::unique_lock<std::mutex> lock(m);
                auto it = found.find(key);
                if (it != found.end()) {
                    out = it->second;
                } else {
                    lock.unlock();
                    const std::string got = step(out, std::string(s, e));
                    lock.lock();
                    found[key] = got;
                    out = got;
                }
            }
        }
        s = e;
    }
    return out;
}

} // namespace pm

namespace plat {

uint32_t now_ms() { return SDL_GetTicks(); }

uint32_t random_seed()
{
    const uint32_t s = static_cast<uint32_t>(time(nullptr)) ^ (SDL_GetTicks() << 16);
    return s ? s : 1;
}

void log(const char* fmt, ...)
{
    va_list a;
    va_start(a, fmt);
    vprintf(fmt, a);
    va_end(a);
    fflush(stdout);
}

// (a handheld's memory is thousands of times the boards': not counted)
size_t free_heap() { return 0; }
size_t largest_block() { return 0; }
// The VGA games in their 256 colours
bool vga_screen() { return true; }

namespace {

class HostFile : public FileImpl {
public:
    explicit HostFile(FILE* f) : f_(f) {}
    HostFile(DIR* d, const std::string& path) : d_(d), path_(path), dir_(true) {}
    // A folder's entry, listed: opened only when it is used (Tom, 2026-11-27: the Android app slow to its tiles -
    // the listing had opened every file and folder in it, each a slow step through Android's shared storage, and
    // the engine's scan lists the folders it looks in again and again)
    static std::shared_ptr<HostFile> entry(const std::string& path, bool dir)
    {
        auto r = std::make_shared<HostFile>(static_cast<FILE*>(nullptr));
        r->path_ = path;
        r->dir_ = dir;
        r->lazy_ = true;
        return r;
    }
    ~HostFile() override { close(); }
    bool ok() const override { return f_ || d_ || lazy_; }
    size_t size() override
    {
        if (!file()) return 0;
        const long c = ftell(f_);
        fseek(f_, 0, SEEK_END);
        const long s = ftell(f_);
        fseek(f_, c, SEEK_SET);
        return static_cast<size_t>(s);
    }
    bool seek(uint32_t p) override { return file() && fseek(f_, p, SEEK_SET) == 0; }
    int read(uint8_t* b, size_t n) override { return file() ? static_cast<int>(fread(b, 1, n, f_)) : 0; }
    size_t write(const uint8_t* b, size_t n) override { return f_ ? fwrite(b, 1, n, f_) : 0; }
    void close() override
    {
        lazy_ = false;
        if (f_) {
            fclose(f_);
            f_ = nullptr;
        }
        if (d_) {
            closedir(d_);
            d_ = nullptr;
        }
    }
    bool is_dir() const override { return dir_ && (d_ || lazy_); }
    const char* name() const override { return name_.c_str(); }
    std::shared_ptr<FileImpl> next() override
    {
        if (!folder()) return nullptr;
        while (dirent* e = readdir(d_)) {
            if (e->d_name[0] == '.') continue;
            const std::string p = path_ + "/" + e->d_name;
            // (what it is from the folder's own listing; only when that doesn't say, a look at it)
            bool dir = e->d_type == DT_DIR, reg = e->d_type == DT_REG;
            if (!dir && !reg) {
                struct stat st;
                if (stat(p.c_str(), &st) != 0) continue;
                dir = S_ISDIR(st.st_mode);
            }
            auto r = entry(p, dir);
            r->name_ = e->d_name;
            return r;
        }
        return nullptr;
    }

private:
    // (an entry listed: opened at its first use)
    bool file()
    {
        if (!f_ && lazy_ && !dir_) {
            lazy_ = false;
            f_ = fopen(path_.c_str(), "rb");
        }
        return f_ != nullptr;
    }
    bool folder()
    {
        if (!d_ && lazy_ && dir_) {
            lazy_ = false;
            d_ = opendir(path_.c_str());
        }
        return d_ != nullptr;
    }

    FILE*       f_ = nullptr;
    DIR*        d_ = nullptr;
    std::string path_, name_;
    bool        dir_ = false, lazy_ = false;
};

} // namespace

bool card_ready()
{
    struct stat st;
    return stat(pm::games_root().c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

File open(const char* card_path, const char* mode)
{
    const std::string hp = pm::host_path(card_path);
    const bool writing = mode && (mode[0] == 'w' || mode[0] == 'a');
    struct stat st;
    if (!writing && stat(hp.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        DIR* d = opendir(hp.c_str());
        return d ? File(std::make_shared<HostFile>(d, hp)) : File();
    }
    FILE* f = fopen(hp.c_str(), mode && mode[0] == 'w' ? "wb" : mode && mode[0] == 'a' ? "ab" : "rb");
    if (!f) return File();
    return File(std::make_shared<HostFile>(f));
}

bool exists(const char* path)
{
    struct stat st;
    return stat(pm::host_path(path).c_str(), &st) == 0;
}
bool remove(const char* path) { return ::remove(pm::host_path(path).c_str()) == 0; }
// When the file was last written, as the player reads a date - the engine's
// Save Slot Info shows it beside each save ("Nov 17 2026 9:41 PM", the
// device's local time; the handhelds have a clock, the boards none)
uint32_t file_stamp(const char* path)
{
    struct stat st;
    if (stat(pm::host_path(path).c_str(), &st) != 0) return 0;
    return static_cast<uint32_t>(st.st_mtime);
}

bool file_time(const char* path, char* out, size_t cap)
{
    if (cap) out[0] = 0;
    struct stat st;
    if (!cap || stat(pm::host_path(path).c_str(), &st) != 0) return false;
    struct tm tmv;
    const time_t t = st.st_mtime;
    if (!localtime_r(&t, &tmv) || tmv.tm_year + 1900 < 2000) return false;   // (a device whose clock was never set: no date)
    static const char* const kMon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const int h12 = tmv.tm_hour % 12 ? tmv.tm_hour % 12 : 12;
    // (the save screen's column on a 4:3 panel is 18 letters and breaks a longer line at a space: "Nov 7 2026
    // 9:41 PM" fits; a longer one closes up to "9:41PM" - then it fits, or breaks after the year, never
    // leaving "PM" on a row of its own)
    char full[32];
    snprintf(full, sizeof full, "%s %d %d %d:%02d %s", kMon[tmv.tm_mon], tmv.tm_mday, tmv.tm_year + 1900, h12, tmv.tm_min,
             tmv.tm_hour < 12 ? "AM" : "PM");
    if (strlen(full) > 18)
        snprintf(full, sizeof full, "%s %d %d %d:%02d%s", kMon[tmv.tm_mon], tmv.tm_mday, tmv.tm_year + 1900, h12, tmv.tm_min,
                 tmv.tm_hour < 12 ? "AM" : "PM");
    snprintf(out, cap, "%s", full);
    return true;
}
bool rename(const char* from, const char* to)
{
    if (!exists(from)) return false;
    if (exists(to) && !remove(to)) return false;
    return ::rename(pm::host_path(from).c_str(), pm::host_path(to).c_str()) == 0;
}
bool rmdir(const char* path) { return ::rmdir(pm::host_path(path).c_str()) == 0; }
bool mkdir(const char* path)
{
    ::mkdir(pm::host_path(path).c_str(), 0755);
    return exists(path);
}

} // namespace plat

// The boards' memory log: here only the engine's own notes, into the port's
// log (GOLDBOX_MEMLOG=1) - memory is never short on these machines
namespace memlog {

void begin(const char*, const char*) {}
void note(const char* line)
{
    if (getenv("GOLDBOX_MEMLOG")) plat::log("[memlog] %s\n", line);
}
void tick(bool) {}
unsigned free_kb() { return 999; }
unsigned largest_kb() { return 999; }

} // namespace memlog
