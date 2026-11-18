#!/usr/bin/env python3
"""A switch's EFFECT in a game, through the port's buttons (Tom, 2026-11-18: a menu line that
flips isn't a tested switch). Needs real game files: run by hand on a COPY of a scanned games
folder (scratch only).

    tools/switch_test.py <binary> <games copy> <game n> <out dir> coins

coins (Curse, its stock save): the Merchant cheat - Pool, Take 100 platinum, Buy the first item
(3000 gold), once with Coin Exchange on and once off (settings.cfg comforts_off 8192).
  On : the buyer's own coins go first, the counter pays the rest - buyer 0, counter 1200.
  Off: the originals, one purse or the other - buyer keeps 100, counter 1100.
"""
import os, re, shutil, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt

def platinum(lines):
    for t in lines:
        m = re.search(r"Platinum\s+(\d+)", t)
        if m: return int(m.group(1))
    return 0

def leave(r):
    for _ in range(4):
        if r.bottom().startswith("Buy View"): return
        r.do("b,wait:500")

def coins(src, gi, out, off):
    d = f"{out}/coins_{off}"
    shutil.rmtree(d, ignore_errors=True)
    shutil.copytree(src, d + "/games")
    pt.OUT = d
    pt.CFG_EXTRA = f"menus=2\ncomforts_off={off}\n"
    r = pt.Run(d + "/games", gi, "c"); p = r.p
    pt.start(r)
    r.do("start,wait:300,menu:Cheats,a,wait:400"); r.do("menu:Merchant,a,wait:1200")
    pt.goto_word(p, "Pool"); r.do("a,wait:800")
    pt.goto_word(p, "Take"); r.do("a,wait:800"); r.do("a,wait:800")
    r.do("type:100,wait:300"); r.do("y,wait:800")
    leave(r)
    pt.goto_word(p, "Buy"); r.do("a,wait:800"); r.do("a,wait:1200")
    leave(r)
    pt.goto_word(p, "View"); r.do("a,wait:800"); buyer = platinum(p.text); r.do("b,wait:500")
    pt.goto_word(p, "Take"); r.do("a,wait:800"); counter = platinum(p.text)
    p.quit()
    return buyer, counter

if __name__ == "__main__":
    pt.EXE, src, gi, out, what = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4], sys.argv[5]
    bad = 0
    if what == "coins":
        for off, want in ((0, (0, 1200)), (8192, (100, 1100))):
            got = coins(src, gi, out, off)
            ok = got == want
            bad += not ok
            print(f"Coin Exchange {'on ' if off == 0 else 'off'}: buyer {got[0]} platinum, counter {got[1]} - "
                  f"{'OK' if ok else 'FAILED, wanted %d / %d' % want}")
    sys.exit(1 if bad else 0)
