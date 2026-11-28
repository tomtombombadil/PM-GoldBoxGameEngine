#include "cluemaps.h"

#include <cstdio>
#include <cstring>
#include <strings.h>

#include "engine/cluebook.h"
#include "jour.h"
#include "platform/file.h"
#include "platform/platform.h"

namespace cluemaps {

namespace {

constexpr char kList[] = "CLUEMAPS.TXT";
std::string names_key, names_text;      // (CLUEMAPS.TXT as last read: name())

// A PDF on the device's own disc (jour::documents gives the host's paths)
struct HostSource : dax::ByteSource {
    FILE* f = nullptr;
    uint32_t n = 0;
    explicit HostSource(const std::string& path)
    {
        f = fopen(path.c_str(), "rb");
        if (f && fseek(f, 0, SEEK_END) == 0) n = static_cast<uint32_t>(ftell(f));
    }
    ~HostSource() override
    {
        if (f) fclose(f);
    }
    size_t read_at(uint32_t pos, uint8_t* buf, size_t k) override
    {
        if (!f || fseek(f, pos, SEEK_SET) != 0) return 0;
        return fread(buf, 1, k, f);
    }
    uint32_t size() const override { return n; }
};

std::string geo_name(const cluebook::Map& m)
{
    char b[16];
    if (m.geo) snprintf(b, sizeof b, "GEO%d.DAX", m.geo);
    else snprintf(b, sizeof b, "GEO.DAX");
    return b;
}

std::string pic_path(const std::string& cache, const char* file, int block)
{
    std::string f = file;
    const size_t dot = f.find('.');
    if (dot != std::string::npos) f.resize(dot);
    char b[24];
    snprintf(b, sizeof b, "_%d.IMG", block);
    return cache + "/CM_" + f + b;
}

struct Writer : cluebook::Sink {
    std::string cache;
    std::string list;
    Say say;
    int last = -1;
    bool picture(const cluebook::Map& m, int square, int w, int h, const uint8_t* rgb) override
    {
        const std::string g = geo_name(m);
        plat::File f = plat::open(pic_path(cache, g.c_str(), m.block).c_str(), "w");
        if (!f) return true;
        uint8_t head[10] = {'C', 'M', 'I', '1', static_cast<uint8_t>(square), static_cast<uint8_t>(square >> 8),
                            static_cast<uint8_t>(w), static_cast<uint8_t>(w >> 8), static_cast<uint8_t>(h),
                            static_cast<uint8_t>(h >> 8)};
        f.write(head, sizeof head);
        f.write(rgb, static_cast<size_t>(w) * h * 3);
        f.close();
        char line[48];
        snprintf(line, sizeof line, "%s\t%d\t", g.c_str(), m.block);
        list += line;
        list += "\n";
        return true;
    }
    void name(const cluebook::Map& m, const char* text) override
    {
        // (the name goes on the picture's line, written just before)
        char key[48];
        snprintf(key, sizeof key, "%s\t%d\t\n", geo_name(m).c_str(), m.block);
        const size_t at = list.rfind(key);
        if (at != std::string::npos) list.insert(at + strlen(key) - 1, text);
    }
    void progress(int n, int total) override
    {
        if (n == last) return;
        last = n;
        char line[48];
        snprintf(line, sizeof line, "    map %d of %d", n < total ? n + 1 : total, total);
        say(line, true, nullptr);
    }
};

void write_list(const std::string& cache, const std::string& text)
{
    plat::File f = plat::open((cache + "/" + kList).c_str(), "w");
    if (!f) return;
    f.write(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    f.close();
    names_key.clear();
}

bool contains_nocase(const std::string& s, const char* what)
{
    const size_t n = strlen(what);
    for (size_t i = 0; i + n <= s.size(); ++i)
        if (strncasecmp(s.c_str() + i, what, n) == 0) return true;
    return false;
}

} // namespace

bool ready(const std::string& cache) { return plat::exists((cache + "/" + kList).c_str()); }

std::string find(const library::GameDir& g)
{
    for (const jour::Document& d : jour::documents(g))
        if (contains_nocase(d.label, "clue")) return d.path;
    return "";
}

void none(const std::string& cache) { write_list(cache, "none\tno cluebook\n"); }

void make(const library::GameDir& g, const std::string& cache, Say say)
{
    const std::string pdf = find(g);
    if (pdf.empty()) {
        none(cache);
        return;
    }
    say("  Processing the cluebook's maps:", false, nullptr);
    HostSource src(pdf);
    Writer w;
    w.cache = cache;
    w.say = say;
    w.list = "maps 1\n";
    const int made = src.f ? cluebook::make(src, g.game, w) : -1;
    if (made > 0) {
        write_list(cache, w.list);
        char line[64];
        snprintf(line, sizeof line, "  Its cluebook's maps are ready (%d).", made);
        say(line, false, nullptr);
    } else {
        write_list(cache, made == -1 ? "none\tunknown edition\n" : "none\tnot made\n");
        say(made == -1 ? "  No cluebook maps: this edition isn't known." : "  No cluebook maps: they couldn't be made.",
            false, nullptr);
    }
    plat::log("[cluemaps] %s: %d maps from %s\n", g.folder.c_str(), made, pdf.c_str());
}

const Picture* picture(const std::string& cache, const char* file, int block)
{
    static Picture pic;
    static std::string key;
    const std::string p = pic_path(cache, file, block);
    if (p == key) return pic.rgb.empty() ? nullptr : &pic;
    key = p;
    pic = Picture();
    plat::File f = plat::open(p.c_str());
    if (!f) return nullptr;
    uint8_t head[10];
    if (f.read(head, sizeof head) == 10 && memcmp(head, "CMI1", 4) == 0) {
        pic.square = head[4] | head[5] << 8;
        pic.w = head[6] | head[7] << 8;
        pic.h = head[8] | head[9] << 8;
        pic.rgb.resize(static_cast<size_t>(pic.w) * pic.h * 3);
        if (f.read(pic.rgb.data(), pic.rgb.size()) != static_cast<int>(pic.rgb.size())) pic.rgb.clear();
    }
    f.close();
    return pic.rgb.empty() ? nullptr : &pic;
}

std::string name(const std::string& cache, const char* file, int block)
{
    std::string& key = names_key;
    std::string& text = names_text;
    if (key != cache) {
        key = cache;
        text.clear();
        plat::File f = plat::open((cache + "/" + kList).c_str());
        if (f) {
            text.resize(f.size());
            f.read(reinterpret_cast<uint8_t*>(&text[0]), text.size());
            f.close();
        }
    }
    char want[48];
    snprintf(want, sizeof want, "%s\t%d\t", file, block);
    size_t at = 0;
    while ((at = text.find(want, at)) != std::string::npos) {
        if (at == 0 || text[at - 1] == '\n') {
            const size_t s = at + strlen(want), e = text.find('\n', s);
            return text.substr(s, e == std::string::npos ? std::string::npos : e - s);
        }
        ++at;
    }
    return "";
}

} // namespace cluemaps
