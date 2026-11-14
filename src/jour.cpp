#include "jour.h"

#include "engine/games.h"
#include "engine/journal.h"
#include "engine/pdf.h"
#include "game/play.h"
#include "platform/file.h"
#include "platform/platform.h"

#include "video.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <strings.h>
#include <vector>

namespace jour {
namespace {

// ---------------------------------------------------------------------------
// Making the journal's files
// ---------------------------------------------------------------------------

std::string card(const std::string& rel) { return std::string(games::kRootDir) + "/" + rel; }

bool ends_with_nocase(const std::string& s, const char* tail)
{
    const size_t n = strlen(tail);
    return s.size() >= n && strcasecmp(s.c_str() + s.size() - n, tail) == 0;
}

bool contains_nocase(const std::string& s, const char* word)
{
    const size_t n = strlen(word);
    for (size_t i = 0; i + n <= s.size(); ++i)
        if (strncasecmp(s.c_str() + i, word, n) == 0) return true;
    return false;
}

// The journal PDF in games/<rel>: a .pdf with "journal" in its name ("": none)
std::string find_pdf(const std::string& rel)
{
    plat::File d = plat::open(card(rel).c_str());
    std::string found;
    if (!d || !d.isDirectory()) return found;
    for (plat::File f = d.openNextFile(); f; f = d.openNextFile()) {
        const std::string name = f.name();
        if (found.empty() && !f.isDirectory() && ends_with_nocase(name, ".pdf") && contains_nocase(name, "journal"))
            found = rel + "/" + name;
        f.close();
    }
    return found;
}

// The Gold Box Companion's folder for the game, inside its own folder (the
// Steam / SNEG releases: <folder>/GBC/Games/<nn. Title>/Game.dat) - "": none
std::string find_gbc(const std::string& folder)
{
    const std::string base = folder + "/GBC/Games";
    plat::File d = plat::open(card(base).c_str());
    std::string sub;
    if (!d || !d.isDirectory()) return sub;
    for (plat::File f = d.openNextFile(); f && sub.empty(); f = d.openNextFile()) {
        if (f.isDirectory()) sub = f.name();
        f.close();
    }
    if (sub.empty() || !plat::exists(card(base + "/" + sub + "/Game.dat").c_str())) return "";
    return base + "/" + sub;
}

uint32_t file_size(const std::string& card_path)
{
    plat::File f = plat::open(card_path.c_str());
    const uint32_t n = f ? static_cast<uint32_t>(f.size()) : 0;
    if (f) f.close();
    return n;
}

struct FileOut : journal::Output {
    plat::File f;
    bool write_at(uint32_t pos, const uint8_t* d, size_t n) override { return f && f.seek(pos) && f.write(d, n) == n; }
};

// The Companion's JE_NNN.jpg pictures from a folder
struct Jpegs : journal::GbcPictures {
    std::string                        dir;         // a card path
    plat::File                         f;
    std::unique_ptr<plat::FileSource>  src;
    ~Jpegs() override { close(); }
    void close() override
    {
        src.reset();
        if (f) f.close();
    }
    dax::ByteSource* open(int number) override
    {
        close();
        if (dir.empty()) return nullptr;
        char name[24];
        snprintf(name, sizeof name, "/JE_%03d.jpg", number);
        f = plat::open((dir + name).c_str());
        if (!f) return nullptr;
        src.reset(new plat::FileSource(f));
        return src.get();
    }
};

struct SectText : journal::ExtraText {
    std::string                        path;
    plat::File                         f;
    std::unique_ptr<plat::FileSource>  src;
    ~SectText() override { close(); }
    dax::ByteSource* open() override
    {
        close();
        f = plat::open(path.c_str());
        if (!f) return nullptr;
        src.reset(new plat::FileSource(f));
        return src.get();
    }
    void close() override
    {
        src.reset();
        if (f) f.close();
    }
};

struct NoGameDat : dax::ByteSource {
    uint32_t size() const override { return 0; }
    size_t read_at(uint32_t, uint8_t*, size_t) override { return 0; }
};

struct Prog {
    Say         say;
    const char* title;
    const char* what;
};

void progress(int done, int total, void* ctx)
{
    const Prog* p = static_cast<const Prog*>(ctx);
    if (!p->say || (done % 4 && done != total)) return;
    char line[120];
    snprintf(line, sizeof line, "%s the %s journal: page %d of %d", p->what, p->title, done, total);
    p->say(line, true, nullptr);
}

// The id JOURNAL.DAT has ("" when there's none)
std::string made_id(const std::string& dat_path, journal::Info* info_out = nullptr)
{
    plat::File f = plat::open(dat_path.c_str());
    if (!f) return "";
    plat::FileSource src(f);
    journal::Info info;
    const bool ok = journal::read_info(src, info);
    f.close();
    if (info_out) *info_out = info;
    return ok ? info.pdf_id : "";
}

bool book_ready(const std::string& cache)
{
    plat::File f = plat::open((cache + "/BOOK.TXT").c_str());
    if (!f) return false;
    plat::FileSource src(f);
    const bool ok = journal::book_pages(src) > 0;
    f.close();
    return ok;
}

// The PDF's text into BOOK.TXT, its sections' into SECT.TMP
bool read_book(const std::string& pdf, const std::string& cache, journal::Section* secs, journal::BookInfo& info,
               const Prog& pr)
{
    plat::File pf = plat::open(card(pdf).c_str());
    if (!pf) return false;
    FileOut bo, so;
    bo.f = plat::open((cache + "/BOOK.TXT").c_str(), "w");
    so.f = plat::open((cache + "/SECT.TMP").c_str(), "w");
    plat::FileSource src(pf);
    Prog p = pr;
    p.what = "Reading";
    bool ok = bo.f && so.f && journal::make_book(src, bo, so, secs, journal::kMaxSections, info, progress, &p);
    if (bo.f) bo.f.close();
    if (so.f) so.f.close();
    pf.close();
    plat::log("[journal] the book's text, games/%s: %s - %d page(s) read, %d failed, %u letters, %d section(s)%s\n", pdf.c_str(),
              ok ? "read" : "NOT read", info.pages, info.failed, static_cast<unsigned>(info.chars), info.sections,
              info.no_memory ? ", out of memory" : "");
    if (ok && info.chars < 400) ok = false;         // (a scanned book has no text)
    if (!ok) {
        plat::remove((cache + "/BOOK.TXT").c_str());
        plat::remove((cache + "/SECT.TMP").c_str());
    }
    return ok;
}

// Steam / SNEG with the Gold Box Companion: its entries' text and pictures,
// plus the PDF's own sections (tavern tales and the like)
void make_from_gbc(const library::GameDir& g, const std::string& cache, const std::string& gbc, const std::string& pdf,
                   Say say)
{
    const char* title = games::title(g.game);
    const std::string dat = card(gbc + "/Game.dat"), out_path = cache + "/JOURNAL.DAT";
    char id[48];
    snprintf(id, sizeof id, "gbc5:%u:%u", file_size(dat), pdf.empty() ? 0u : file_size(card(pdf)));
    if (made_id(out_path) == id && (pdf.empty() || book_ready(cache))) return;
    Prog pr{say, title, "Processing"};
    std::vector<journal::Section> secs(pdf.empty() ? 0 : journal::kMaxSections);
    journal::BookInfo binfo;
    SectText sect;
    bool have_book = false;
    if (!pdf.empty()) {
        have_book = read_book(pdf, cache, secs.data(), binfo, pr);
        sect.path = cache + "/SECT.TMP";
        sect.items = secs.data();
        sect.n = have_book ? binfo.sections : 0;
    }
    plat::File f = plat::open(dat.c_str());
    FileOut out;
    out.f = plat::open(out_path.c_str(), "w");
    Jpegs pics;
    pics.dir = card(gbc);
    bool ok = f && out.f;
    if (ok) {
        plat::FileSource src(f);
        ok = journal::make_gbc(src, pics, id, out, progress, &pr, have_book ? &sect : nullptr);
    }
    pics.close();
    sect.close();
    if (out.f) out.f.close();
    if (f) f.close();
    plat::remove((cache + "/SECT.TMP").c_str());
    if (!ok) plat::remove(out_path.c_str());
    plat::log("[journal] %s: from the Gold Box Companion%s - %s\n", title, have_book ? " and the PDF's text" : "",
              ok ? "made" : "FAILED");
}

// A typeset PDF alone: its numbered sections as text entries
void make_from_text(const library::GameDir& g, const std::string& cache, const std::string& pdf, Say say)
{
    const char* title = games::title(g.game);
    const std::string out_path = cache + "/JOURNAL.DAT";
    char id[32];
    snprintf(id, sizeof id, "text2:%u", file_size(card(pdf)));
    if (made_id(out_path) == id && book_ready(cache)) return;
    Prog pr{say, title, "Processing"};
    std::vector<journal::Section> secs(journal::kMaxSections);
    journal::BookInfo binfo;
    const bool have_book = read_book(pdf, cache, secs.data(), binfo, pr);
    bool ok = have_book && binfo.sections > 0;
    if (ok) {
        SectText sect;
        sect.path = cache + "/SECT.TMP";
        sect.items = secs.data();
        sect.n = binfo.sections;
        FileOut out;
        out.f = plat::open(out_path.c_str(), "w");
        NoGameDat none;
        Jpegs pics;
        ok = out.f && journal::make_gbc(none, pics, id, out, progress, &pr, &sect);
        sect.close();
        if (out.f) out.f.close();
        if (!ok) plat::remove(out_path.c_str());
    }
    plat::remove((cache + "/SECT.TMP").c_str());
    plat::log("[journal] %s: from its PDF's text (%d pages, %d sections) - %s\n", title, binfo.pages, binfo.sections,
              ok ? "made" : have_book ? "no entries found in it" : "the PDF's text couldn't be read");
}

} // namespace

void prepare(const library::GameDir& g, const std::string& cache, Say say)
{
    if (g.game == games::Game::Unknown) return;
    const char* title = games::title(g.game);
    // The PDF: in the game's folder, where its files are, or (Steam / SNEG) Documentation
    std::string pdf = find_pdf(g.folder);
    if (pdf.empty() && g.data_dir != g.folder) pdf = find_pdf(g.data_dir);
    if (pdf.empty()) pdf = find_pdf(g.folder + "/Documentation");
    const std::string gbc = find_gbc(g.folder);
    if (pdf.empty() && gbc.empty()) {
        plat::log("[journal] %s: no journal PDF in games/%s\n", title, g.folder.c_str());
        return;
    }
    if (!gbc.empty()) {
        make_from_gbc(g, cache, gbc, pdf, say);
        return;
    }
    plat::File f = plat::open(card(pdf).c_str());
    if (!f) return;
    plat::FileSource src(f);
    std::unique_ptr<pdf::Doc> doc(new pdf::Doc);
    const bool readable = pdf::open(src, *doc);
    const journal::Table* t = readable ? journal::find_table(src.size(), doc->id) : nullptr;
    if (!t) {
        f.close();
        if (readable) make_from_text(g, cache, pdf, say);
        else plat::log("[journal] %s: games/%s can't be read as a PDF\n", title, pdf.c_str());
        return;
    }
    // A scanned edition the engine's tables know: the entries cut from its
    // pages; maps from the Companion's sharper pictures when the player
    // has it in games/GBC
    char gbc_dir[200];
    snprintf(gbc_dir, sizeof gbc_dir, "%s/GBC/Games/%02d. %s", games::kRootDir, games::list_order(g.game), title);
    int maps = 0;
    for (int i = 0; i < t->n_entries; ++i) {
        if (!(t->entries[i].flags & journal::kMap)) continue;
        char je[240];
        snprintf(je, sizeof je, "%s/JE_%03d.jpg", gbc_dir, t->entries[i].number);
        maps += plat::exists(je) ? 1 : 0;
    }
    const std::string out_path = cache + "/JOURNAL.DAT";
    journal::Info info;
    if (made_id(out_path, &info) == t->pdf_id && info.entries == t->n_entries && info.revision >= journal::kRevision &&
        info.maps == maps) {
        f.close();
        return;
    }
    Prog pr{say, title, "Processing"};
    FileOut out;
    out.f = plat::open(out_path.c_str(), "w");
    Jpegs pics;
    pics.dir = gbc_dir;
    const uint32_t t0 = plat::now_ms();
    const bool ok = out.f && journal::make(src, *t, out, progress, &pr, maps ? &pics : nullptr);
    pics.close();
    if (out.f) out.f.close();
    f.close();
    if (!ok) plat::remove(out_path.c_str());
    plat::log("[journal] %s: %s, %d entries from the PDF's pages - %s (%u s)\n", title, t->name, t->n_entries,
              ok ? "made" : "FAILED", static_cast<unsigned>((plat::now_ms() - t0) / 1000));
}

// ---------------------------------------------------------------------------
// Showing an entry
// ---------------------------------------------------------------------------
namespace {

bool                  on = false;
bool                  game_look = false;        // text on the game's own text screen
// Text (the game's look): the entry's text, its lines
std::string           text;
std::vector<uint32_t> line_at, line_len;        // (len 0: a paragraph's gap)
int                   top = 0;                  // the first line shown / the page's top row
constexpr int         big = 0;                  // (text entries: one size - Tom, 2026-11-14: "it doesn't need a zoom")
std::vector<uint8_t>  saved;                    // the game screen meanwhile
uint8_t               saved_bpp = 4;
// Pictures: the entry as one tall page, video::kBackW wide a zoom
std::vector<uint32_t> doc;
int                   doc_w = 0, doc_h = 0, pan_x = 0;
// A picture entry's zoom (Tom, 2026-11-14): Width - the picture as wide as
// the screen (the start); Double - twice that; Whole - all of it on the
// screen (only when it is taller than the screen); Turned - a very long
// one on its side, its length across the screen (only for those)
enum class Zoom : uint8_t { Width, Double, Whole, Turned };
Zoom                  pic_zoom = Zoom::Width;
bool                  can_whole = false, can_turn = false;
bool                  entry_map = false;        // the entry is a map (the engine's table says so; a Steam entry's picture)
std::vector<uint32_t> view;                     // the backdrop's pixels
char                  title[48];
// (for building the page again at another zoom)
std::string           open_cache;
char                  open_kind = 'J';
int                   open_number = 0;

int cols() { return 38 / (big + 1); }
int rows() { return 22 / (big + 1); }

// The text into lines of at most cols() characters, broken at words
void layout_text()
{
    line_at.clear();
    line_len.clear();
    const int w = cols();
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '\n') {
            line_at.push_back(static_cast<uint32_t>(i));
            line_len.push_back(0);
            ++i;
            continue;
        }
        while (i < text.size() && text[i] == ' ') ++i;
        size_t end = i, brk = i;
        while (end < text.size() && text[end] != '\n' && end - i < static_cast<size_t>(w)) {
            if (text[end] == ' ') brk = end;
            ++end;
        }
        if (end < text.size() && text[end] != '\n' && text[end] != ' ' && brk > i) end = brk;
        if (end > i) {
            line_at.push_back(static_cast<uint32_t>(i));
            line_len.push_back(static_cast<uint32_t>(end - i));
        }
        i = end;
        // (a paragraph's end: its '\n' taken with its last line - the gap
        // comes from the '\n' that follows, if the text has a blank line)
        if (i < text.size() && text[i] == '\n') {
            ++i;
            line_at.push_back(static_cast<uint32_t>(i));
            line_len.push_back(0);
        }
    }
    while (!line_len.empty() && line_len.back() == 0) {
        line_at.pop_back();
        line_len.pop_back();
    }
}

int text_max_top()
{
    const int n = static_cast<int>(line_len.size()) - rows();
    return n > 0 ? n : 0;
}

// A text piece's letters onto the picture page (white paper's ink)
void doc_text(int y, const char* s, size_t n)
{
    int x = 16;
    for (size_t k = 0; k < n; ++k, x += 16) {
        const uint8_t* g = video::glyph(s[k]);
        for (int j = 0; j < 8; ++j)
            for (int i = 0; i < 8; ++i)
                if (g[j] & (0x80 >> i))
                    for (int b = 0; b < 4; ++b) {
                        const int px = x + i * 2 + (b & 1), py = y + j * 2 + (b >> 1);
                        if (px < doc_w && py < doc_h) doc[static_cast<size_t>(py) * doc_w + px] = 0xFF202020u;
                    }
    }
}

// The entry's pieces stacked on one page doc_w wide: pictures scaled to
// its width (the rows squeezed by how the screen will stretch them), text
// in the port's letters
bool build_doc(dax::ByteSource& src, const journal::Info& info, int first, int count, int width, int aspect)
{
    doc_w = width;
    struct Part {
        journal::PieceInfo p;
        int                y, h;
        std::string        t;
        std::vector<uint32_t> at, len;
    };
    std::vector<Part> parts(static_cast<size_t>(count));
    int y = 8;
    const int text_cols = doc_w / 16 - 2 > 8 ? doc_w / 16 - 2 : 8;
    for (int i = 0; i < count; ++i) {
        Part& q = parts[static_cast<size_t>(i)];
        if (!journal::piece(src, info, first + i, q.p)) return false;
        q.y = y;
        if (q.p.text()) {
            const int n = q.p.h < 16000 ? q.p.h : 16000;
            q.t.resize(static_cast<size_t>(n));
            if (src.read_at(q.p.offset, reinterpret_cast<uint8_t*>(&q.t[0]), static_cast<size_t>(n)) != static_cast<size_t>(n))
                return false;
            // (wrapped as the text screen's, at this page's width)
            size_t k = 0;
            while (k < q.t.size()) {
                if (q.t[k] == '\n') {
                    q.at.push_back(static_cast<uint32_t>(k));
                    q.len.push_back(0);
                    ++k;
                    continue;
                }
                while (k < q.t.size() && q.t[k] == ' ') ++k;
                size_t end = k, brk = k;
                while (end < q.t.size() && q.t[end] != '\n' && end - k < static_cast<size_t>(text_cols)) {
                    if (q.t[end] == ' ') brk = end;
                    ++end;
                }
                if (end < q.t.size() && q.t[end] != '\n' && q.t[end] != ' ' && brk > k) end = brk;
                q.at.push_back(static_cast<uint32_t>(k));
                q.len.push_back(static_cast<uint32_t>(end - k));
                k = end;
                if (k < q.t.size() && q.t[k] == '\n') ++k;
            }
            q.h = static_cast<int>(q.at.size()) * 20 + 8;
        } else {
            if (q.p.w <= 0 || q.p.w > 4000 || q.p.h <= 0) return false;
            // (never bigger than twice its scan: a small picture stays sharp)
            int w = doc_w - 16;
            if (w > q.p.w * 2) w = q.p.w * 2;
            q.h = static_cast<int>(static_cast<long long>(q.p.h) * w * 1000 / (static_cast<long long>(q.p.w) * aspect));
            if (q.h < 1) q.h = 1;
        }
        y += q.h + 8;
    }
    doc_h = y;
    if (doc_h > 40000) return false;
    doc.assign(static_cast<size_t>(doc_w) * doc_h, 0xFFFFFFFFu);
    std::vector<uint8_t> row;
    std::vector<uint32_t> acc;
    for (const Part& q : parts) {
        if (q.p.text()) {
            for (size_t l = 0; l < q.at.size(); ++l)
                if (q.len[l]) doc_text(q.y + static_cast<int>(l) * 20, q.t.c_str() + q.at[l], q.len[l]);
            continue;
        }
        int w = doc_w - 16;
        if (w > q.p.w * 2) w = q.p.w * 2;
        const int x0 = (doc_w - w) / 2;
        row.resize(static_cast<size_t>(q.p.w));
        const int black = journal::piece_black(src, q.p, row.data());
        uint8_t lut[256][3];
        for (int v = 0; v < 256; ++v) journal::level_rgb(v, black, &lut[v][0], &lut[v][1], &lut[v][2]);
        // Each page row: the scan rows it covers, averaged (print stays
        // readable scaled down), each of those averaged across too
        acc.assign(static_cast<size_t>(w) * 4, 0);
        int out_y = 0;
        for (int sy = 0; sy < q.p.h; ++sy) {
            if (src.read_at(q.p.offset + static_cast<uint32_t>(sy) * q.p.w, row.data(), static_cast<size_t>(q.p.w)) !=
                static_cast<size_t>(q.p.w))
                return false;
            for (int x = 0; x < w; ++x) {
                int a = static_cast<int>(static_cast<long long>(x) * q.p.w / w);
                int b = static_cast<int>(static_cast<long long>(x + 1) * q.p.w / w);
                if (b <= a) b = a + 1;
                if (b > q.p.w) b = q.p.w;
                uint32_t r = 0, g = 0, bl = 0;
                for (int k = a; k < b; ++k) {
                    r += lut[row[k]][0];
                    g += lut[row[k]][1];
                    bl += lut[row[k]][2];
                }
                const uint32_t n = static_cast<uint32_t>(b - a);
                uint32_t* c = &acc[static_cast<size_t>(x) * 4];
                c[0] += r / n;
                c[1] += g / n;
                c[2] += bl / n;
                c[3] += 1;
            }
            // (the page rows this scan row finishes)
            const int until = static_cast<int>(static_cast<long long>(sy + 1) * q.h / q.p.h);
            if (until <= out_y && sy + 1 < q.p.h) continue;
            for (int oy = out_y; oy < (until > out_y ? until : out_y + 1) && oy < q.h; ++oy) {
                uint32_t* to = &doc[static_cast<size_t>(q.y + oy) * doc_w + x0];
                for (int x = 0; x < w; ++x) {
                    const uint32_t* c = &acc[static_cast<size_t>(x) * 4];
                    const uint32_t n = c[3] ? c[3] : 1;
                    to[x] = 0xFF000000u | ((c[0] / n) << 16) | ((c[1] / n) << 8) | (c[2] / n);
                }
            }
            out_y = until > out_y ? until : out_y + 1;
            acc.assign(static_cast<size_t>(w) * 4, 0);
        }
    }
    return true;
}

constexpr int kViewRows = (video::kH - 10) * 2;         // the page's part of the backdrop (a hint line under it)

// The page made smaller to fit the screen's height: each new pixel the
// average of those it covers
void shrink_doc(int to_h)
{
    if (doc_h <= to_h || doc_h <= 0) return;
    const int nw = static_cast<int>(static_cast<long long>(doc_w) * to_h / doc_h) > 0
                       ? static_cast<int>(static_cast<long long>(doc_w) * to_h / doc_h) : 1;
    std::vector<uint32_t> out(static_cast<size_t>(nw) * to_h);
    for (int y = 0; y < to_h; ++y) {
        const int y0 = static_cast<int>(static_cast<long long>(y) * doc_h / to_h);
        int y1 = static_cast<int>(static_cast<long long>(y + 1) * doc_h / to_h);
        if (y1 <= y0) y1 = y0 + 1;
        for (int x = 0; x < nw; ++x) {
            const int x0 = static_cast<int>(static_cast<long long>(x) * doc_w / nw);
            int x1 = static_cast<int>(static_cast<long long>(x + 1) * doc_w / nw);
            if (x1 <= x0) x1 = x0 + 1;
            uint32_t rr = 0, gg = 0, bb = 0, n = 0;
            for (int j = y0; j < y1 && j < doc_h; ++j)
                for (int i = x0; i < x1 && i < doc_w; ++i) {
                    const uint32_t c = doc[static_cast<size_t>(j) * doc_w + i];
                    rr += (c >> 16) & 0xFF;
                    gg += (c >> 8) & 0xFF;
                    bb += c & 0xFF;
                    ++n;
                }
            if (!n) n = 1;
            out[static_cast<size_t>(y) * nw + x] = 0xFF000000u | ((rr / n) << 16) | ((gg / n) << 8) | (bb / n);
        }
    }
    doc.swap(out);
    doc_w = nw;
    doc_h = to_h;
}

// The page a quarter turn clockwise (its top to the right)
void turn_doc()
{
    std::vector<uint32_t> out(static_cast<size_t>(doc_w) * doc_h);
    const int nw = doc_h, nh = doc_w;
    for (int y = 0; y < nh; ++y)
        for (int x = 0; x < nw; ++x) out[static_cast<size_t>(y) * nw + x] = doc[static_cast<size_t>(doc_h - 1 - x) * doc_w + y];
    doc.swap(out);
    doc_w = nw;
    doc_h = nh;
}

// The entry's page at a zoom
bool make_doc(dax::ByteSource& src, const journal::Info& info, int first, int count, Zoom z)
{
    const int aspect = video::pixel_aspect() > 0 ? video::pixel_aspect() : 1000;
    if (z == Zoom::Turned) {
        // On its side its rows run across the screen: built with the
        // pixels' shape the other way round, kBackW rows tall, then turned
        if (!build_doc(src, info, first, count, video::kBackW, aspect)) return false;
        const long long h_fit = doc_h;        // (its height at the screen's width)
        int w = static_cast<int>(static_cast<long long>(video::kBackW) * video::kBackW * 1000000LL /
                                 (h_fit * aspect * aspect));
        if (w < 48) w = 48;
        if (w > video::kBackW * 2) w = video::kBackW * 2;
        if (!build_doc(src, info, first, count, w, 1000000 / aspect)) return false;
        turn_doc();
        return true;
    }
    if (!build_doc(src, info, first, count, z == Zoom::Double ? video::kBackW * 2 : video::kBackW, aspect)) return false;
    if (z == Zoom::Width) {
        can_whole = doc_h > kViewRows;
        can_turn = entry_map && doc_h > kViewRows * 2;      // (a very long map: twice the screen and more)
    }
    if (z == Zoom::Whole) shrink_doc(kViewRows);
    return true;
}

int doc_max_top() { return doc_h > kViewRows ? doc_h - kViewRows : 0; }

void clamp()
{
    if (game_look) {
        if (top > text_max_top()) top = text_max_top();
    } else {
        if (top > doc_max_top()) top = doc_max_top();
        const int max_pan = doc_w > video::kBackW ? doc_w - video::kBackW : 0;
        if (pan_x > max_pan) pan_x = max_pan;
        if (pan_x < 0) pan_x = 0;
    }
    if (top < 0) top = 0;
}

bool load(pic::Canvas& game, std::string& why)
{
    char named[40];
    journal::mention_name(open_kind, open_number, named, sizeof named);
    if (open_kind == 'I') snprintf(named, sizeof named, "the Introduction");
    snprintf(title, sizeof title, "%s", named);
    if (title[0] >= 'a' && title[0] <= 'z') title[0] = static_cast<char>(title[0] - 32);
    why = std::string("Read ") + named + " in your Adventurer's Journal.";
    plat::File f = plat::open((open_cache + "/JOURNAL.DAT").c_str());
    if (!f) {
        why += " (The port found no journal it can read in this game's folder: copy the game's whole folder, its "
               "journal PDF included, into games.)";
        return false;
    }
    plat::FileSource src(f);
    journal::Info info;
    int first = 0, count = 0;
    if (!journal::read_info(src, info)) {
        f.close();
        return false;
    }
    if (!journal::find(src, info, open_kind, open_number, &first, &count) || count <= 0 || count > 64) {
        why = std::string(title) + " isn't in this edition's journal.";
        f.close();
        return false;
    }
    bool all_text = true;
    size_t len = 0;
    std::vector<journal::PieceInfo> ps(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        if (!journal::piece(src, info, first + i, ps[static_cast<size_t>(i)])) {
            f.close();
            return false;
        }
        if (ps[static_cast<size_t>(i)].text()) len += static_cast<size_t>(ps[static_cast<size_t>(i)].h) + 2;
        else all_text = false;
    }
    bool ok = true;
    game_look = false;
    if (all_text) {
        text.clear();
        text.reserve(len);
        for (const auto& p : ps) {
            const size_t n = static_cast<size_t>(p.h < 16000 ? p.h : 16000), at = text.size();
            text.resize(at + n);
            ok = ok && src.read_at(p.offset, reinterpret_cast<uint8_t*>(&text[at]), n) == n;
            text += "\n\n";
        }
        // The game's own text screen, the game screen kept meanwhile
        if (ok && saved.empty()) {
            saved.assign(game.px, game.px + static_cast<size_t>(video::kW) * video::kH);
            saved_bpp = game.bpp;
        }
        ok = ok && play::text_screen(game);
        game_look = ok;
        if (ok) layout_text();
    } else {
        // A map? The scanned editions' tables mark them; a Steam entry's
        // picture (the Companion's JE_nnn.jpg) is one
        const journal::Table* t = journal::find_table(info.pdf_id);
        entry_map = t ? journal::is_map(*t, open_kind, open_number) : true;
        ok = make_doc(src, info, first, count, Zoom::Width);
        if (ok) view.assign(static_cast<size_t>(video::kBackW) * video::kBackH, 0xFF000000u);
    }
    f.close();
    if (!ok) why = std::string("There was a problem showing ") + named + ". Read it in your Adventurer's Journal.";
    return ok;
}

} // namespace

bool open(const std::string& cache_dir, char kind, int number, pic::Canvas& game, std::string& why)
{
    open_cache = cache_dir;
    open_kind = kind;
    open_number = number;
    top = 0;
    pan_x = 0;
    pic_zoom = Zoom::Width;
    on = load(game, why);
    if (!on && !saved.empty()) close(game);
    return on;
}

bool active() { return on; }

void close(pic::Canvas& game)
{
    if (!saved.empty()) {
        // (the game's screen back as it was)
        game.bpp = saved_bpp;
        memcpy(game.px, saved.data(), saved.size());
        saved.clear();
    }
    on = false;
    game_look = false;
    doc.clear();
    doc.shrink_to_fit();
    view.clear();
    video::set_backdrop(nullptr);
}

void scroll(int lines)
{
    top += game_look ? lines : lines * 24;
    clamp();
}

void page(int pages)
{
    top += game_look ? pages * (rows() - 1) : pages * (kViewRows - 32);
    clamp();
}

void pan(int step)
{
    if (game_look) return;
    pan_x += step * 64;
    clamp();
}

bool at_end() { return game_look ? top >= text_max_top() : top >= doc_max_top(); }

// X: a text entry's next page, round to the top after the last (it has
// one size); a picture entry's next zoom
void zoom()
{
    if (game_look) {
        if (top >= text_max_top()) top = 0;
        else top += rows() - 1;
        clamp();
        return;
    }
    const Zoom was = pic_zoom;
    const int was_h = doc_h > 0 ? doc_h : 1, was_top = top;
    Zoom z = was;
    for (int k = 0; k < 4; ++k) {
        z = static_cast<Zoom>((static_cast<int>(z) + 1) % 4);
        if ((z == Zoom::Whole && !can_whole) || (z == Zoom::Turned && !can_turn)) continue;
        break;
    }
    plat::File f = plat::open((open_cache + "/JOURNAL.DAT").c_str());
    if (!f) return;
    plat::FileSource src(f);
    journal::Info info;
    int first = 0, count = 0;
    const bool found = journal::read_info(src, info) && journal::find(src, info, open_kind, open_number, &first, &count);
    if (found && make_doc(src, info, first, count, z)) {
        pic_zoom = z;
        // (the same part of it stays in view; a turned one starts at its start)
        top = z == Zoom::Turned || was == Zoom::Turned ? 0 : static_cast<int>(static_cast<long long>(was_top) * doc_h / was_h);
        pan_x = doc_w > video::kBackW ? (doc_w - video::kBackW) / 2 : 0;
    } else if (found) {
        make_doc(src, info, first, count, was);
    }
    f.close();
    clamp();
}

bool draw(pic::Canvas& game)
{
    video::clear();
    char hint[41];
    if (game_look) {
        // The text: rows() lines from the top one, in the game's font
        play::text_screen(game);
        const int sc = big + 1;
        for (int r = 0; r < rows(); ++r) {
            const size_t k = static_cast<size_t>(top + r);
            if (k >= line_len.size()) break;
            if (line_len[k]) play::text_line(game, text.c_str() + line_at[k], static_cast<int>(line_len[k]), 8, 8 + r * 8 * sc, sc);
        }
        const int pages = text_max_top() ? (text_max_top() + rows() - 2) / (rows() - 1) + 1 : 1;
        const int at = top >= text_max_top() ? pages : top / (rows() - 1) + 1;
        snprintf(hint, sizeof hint, "%.20s  %d of %d", title, at % 1000, pages % 1000);
        video::fill(0, video::kH - 8, video::kW, 8, video::kBlack);
        video::text(0, 24, hint, video::kGold);
        video::text(26, 24, "X Page  B Back", video::kMuted);
        return true;
    }
    // The picture page: the part of it in view, the backdrop
    for (int y = 0; y < video::kBackH; ++y) {
        uint32_t* to = &view[static_cast<size_t>(y) * video::kBackW];
        const int sy = top + y;
        if (y >= kViewRows || sy >= doc_h) {
            for (int x = 0; x < video::kBackW; ++x) to[x] = y >= kViewRows ? 0xFF000000u : 0xFFFFFFFFu;
            continue;
        }
        if (doc_w >= video::kBackW) {
            memcpy(to, &doc[static_cast<size_t>(sy) * doc_w + pan_x], static_cast<size_t>(video::kBackW) * 4);
        } else {
            // (narrower than the screen: in its middle, white beside it)
            const int x0 = (video::kBackW - doc_w) / 2;
            for (int x = 0; x < video::kBackW; ++x) to[x] = 0xFFFFFFFFu;
            memcpy(to + x0, &doc[static_cast<size_t>(sy) * doc_w], static_cast<size_t>(doc_w) * 4);
        }
    }
    video::set_backdrop(view.data(), 0);
    video::fill(0, video::kH - 10, video::kW, 10, video::kBlack);
    const int pct = doc_max_top() ? top * 100 / doc_max_top() : 100;
    snprintf(hint, sizeof hint, "%.18s %d%%", title, pct % 1000);
    video::text_at(0, video::kH - 9, hint, video::kGold);
    static const char* const kZoom[4] = {"Width", "Double", "Whole", "Turned"};
    char keys[24];
    snprintf(keys, sizeof keys, "X %s  B Back", kZoom[static_cast<int>(pic_zoom)]);
    video::text_at(video::kW - static_cast<int>(strlen(keys)) * 8, video::kH - 9, keys, video::kMuted);
    return false;
}

} // namespace jour
