#!/usr/bin/env python3
"""tools/demo_shots.py <binary> <games dir> <game n> <out dir> [WxH]

A game's Demo through the port's buttons (the title's "Play Demo" line): the demo's fight is a fixed one,
so its battlefield can be LOOKED AT - a shot as the fight starts and four more through it (BMPs in <out>:
real game screens, scratch only). Says whether the fight came and whether the Companion stayed (Tom: the
demo keeps it). For the "a fight test looks at the battlefield's picture" rule: nine games, a look each.
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt

pt.EXE, games, n, pt.OUT = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
os.makedirs(pt.OUT, exist_ok=True)
r = pt.Run(games, n, "demo", sys.argv[5] if len(sys.argv) > 5 else None); p = r.p
p.do("wait:1500")
found = False
for i in range(40):
    w = r.words()
    if "Demo" in w:
        found = pt.goto_word(p, "Demo"); break
    if p.has("Load Saved") or p.has("Create New") or p.has("Create Character"): break
    r.do("a,wait:700")
print("game", n, "| Demo on the title's line:", "found" if found else "NOT FOUND", "|", r.bottom())
if found:
    r.do("a,wait:1500")
    seen = False
    for i in range(int(os.environ.get("DEMOWAIT", "60"))):
        if p.st["fighting"] == "1": seen = True; break
        r.do("wait:1000")
    print("  the demo's fight:", "started" if seen else "NOT SEEN", "after", i, "s | dock=" + p.st.get("dock", "?"), "title=" + p.st.get("title", "?"))
    for k in range(5):
        pt.shot(r, f"demo{k}")
        r.do("wait:4000")
    print("  after 20 s: fighting=" + p.st["fighting"], "|", p.st.get("desc", ""))
p.quit()
