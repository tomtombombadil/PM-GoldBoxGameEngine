// Lists what a JOURNAL.DAT the scan made holds (Claude's check of the
// journals: every game, both releases - `jlist <games dir> [folder]`).
// Prints numbers only (a named entry: its first and last words) - output
// stays in scratch.
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "engine/journal.h"
#include "platform/file.h"

#include "host.h"
#include "library.h"

int main(int argc, char** argv)
{
    if (argc < 2) return 2;
    pm::games_root() = argv[1];
    std::vector<library::GameDir> games;
    library::scan(games);
    for (const auto& g : games) {
        if (argc > 2 && g.folder != argv[2]) continue;
        const std::string path = library::cache_dir(g) + "/JOURNAL.DAT";
        plat::File f = plat::open(path.c_str());
        printf("%s (%s):", g.folder.c_str(), games::title(g.game));
        if (!f) {
            printf(" NO JOURNAL.DAT\n");
            continue;
        }
        plat::FileSource src(f);
        journal::Info info;
        if (!journal::read_info(src, info)) {
            printf(" unreadable\n");
            continue;
        }
        printf(" %d entries\n", info.entries);
        // (jlist <games> <folder> <kind><number>: that entry's text pieces' ends - scratch use, to see
        // where a section stops)
        if (argc > 3) {
            int first = 0, count = 0;
            if (journal::find(src, info, argv[3][0], atoi(argv[3] + 1), &first, &count))
                for (int q = 0; q < count; ++q) {
                    journal::PieceInfo p;
                    if (!journal::piece(src, info, first + q, p) || !p.text()) continue;
                    std::string t(static_cast<size_t>(p.h), ' ');
                    src.read_at(p.offset, reinterpret_cast<uint8_t*>(&t[0]), t.size());
                    printf("  [%d letters] %.90s ... %s\n", p.h, t.c_str(), t.size() > 160 ? t.c_str() + t.size() - 160 : "");
                }
        }
        for (const char kind : {'I', 'J', 'T', 'P'}) {
            std::string have, gaps;
            int n = 0, last = 0, text = 0, shortest = 1 << 30, short_at = 0;
            long letters = 0;
            for (int k = 1; k <= 400; ++k) {
                int first = 0, count = 0;
                if (!journal::find(src, info, kind, k, &first, &count) || count <= 0) continue;
                journal::PieceInfo p;
                if (journal::piece(src, info, first, p) && p.text()) {
                    ++text;
                    int len = 0;
                    for (int q = 0; q < count; ++q)
                        if (journal::piece(src, info, first + q, p) && p.text()) len += p.h;
                    letters += len;
                    if (len < shortest) shortest = len, short_at = k;
                }
                for (int m = last + 1; m < k; ++m) gaps += " " + std::to_string(m);
                last = k;
                ++n;
            }
            printf("  %c: %d (1-%d; %d as text", kind, n, last, text);
            if (text) printf(", %ld letters, the shortest %d: number %d", letters, shortest, short_at);
            printf(")%s%s\n", gaps.empty() ? "" : "  MISSING:", gaps.c_str());
        }
        f.close();
    }
    return 0;
}
