#!/usr/bin/env python3
"""tools/lair_test.py <binary> <games copy> <game n> <out dir> <steps>

Plays a save's last stretch by the port's buttons alone: the first saved game on the Load list loaded, then the steps (button names,
comma-separated - e.g. up,up,right,up) with every event after each played through - fights on Quick
("continue battle?" No), Share then Exit at a treasure, No at other questions, A at a page - until the
exploring menu is back. Prints the engine's [fight] lines, where the party stands at the end and its rows.
Made for Treasures' lair and ending (2026-11-26; the save before the lair the only save in a COPY of
the games folder). A stuck screen (60 presses, nothing changing) is shot and the run ends.
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt
import hand_test as ht
pt.EXE = sys.argv[1]; pt.OUT = sys.argv[4]; os.makedirs(pt.OUT, exist_ok=True)
games = sys.argv[2]
r = pt.Run(games, int(sys.argv[3]), "end"); p = r.p
if not pt.start(r): print("\n".join(r.notes)); sys.exit(1)
print("\n".join(r.notes)); print("at:", p.st["desc"])
def texts(): return " ".join(t.strip() for t in p.text[15:24] if t.strip())
seen = []; fights = 0; healed = False
def run_events(maxn=400):
    global fights, healed
    for i in range(maxn):
        _w = r.words()
        if p.st["fighting"] == "1" and not ("Take" in _w and "Exit" in _w):
            fights += 1; n = 0
            while p.st["fighting"] == "1" and n < 4000:
                w = ht.fmw(p)
                if "Take" in w and "Exit" in w: break
                if "Quick" in w and ht.fgoto(p, "Quick"): r.do("a,wait:250")
                elif "Yes" in w and "No" in w:
                    if not globals().get("cb"): globals()["cb"] = 1; pt.shot(r, "continue"); print("   continue battle? asked at press", n, flush=True)
                    ht.fgoto(p, "No"); r.do("a,wait:250")
                else: r.do("a,wait:400")
                n += 1
                sig = (p.st.get("canvas"), r.bottom())
                if sig == globals().get("lastsig"): globals()["same"] = globals().get("same", 0) + 1
                else: globals()["same"] = 0; globals()["lastsig"] = sig
                if globals()["same"] == 60:
                    pt.shot(r, "stuck"); print("   STUCK at press", n, p.st, flush=True); p.show()
                    for key in ["b", "x", "y", "start", "b", "select"]:
                        r.do(key + ",wait:500"); print("    after", key, ":", r.bottom(), "|", p.st.get("canvas"), p.st["desc"], flush=True)
                    pt.shot(r, "stuck2"); open(pt.OUT + "/log.txt", "w").write("".join(p.log)); p.quit(); sys.exit(0)
                if n % 150 == 0: print("   ..", n, "|", r.bottom()[:50], "|", " / ".join(t.strip() for t in p.text[17:24] if t.strip())[:110], flush=True)
            if fights == 1: pt.shot(r, "fight1end")
            print("".join(l for l in p.log if "[fight]" in l), flush=True)
            print("  fight", fights, "over after", n, "presses:", p.st["desc"], "|", r.bottom(), flush=True); continue
        if r.exploring(): return True
        if p.st["desc"].startswith("Party") and "|0 char" in p.st["desc"]: print("  PARTY FELL"); return False
        t = texts()
        if t and (not seen or seen[-1] != t): seen.append(t)
        if "HEALED" in t.upper() and not healed:
            healed = True; pt.shot(r, "healed"); print("  HEALED page:", t[-120:], "| bottom:", r.bottom())
        w = r.words()
        if "Share" in w and globals().get("sh", 0) < 2 and pt.goto_word(p, "Share"): globals()["sh"] = globals().get("sh", 0) + 1; r.do("a,wait:500"); continue
        if "Take" in w and "Exit" in w and pt.goto_word(p, "Exit"): globals()["sh"] = 0; r.do("a,wait:500"); continue
        if "Yes" in w and "No" in w and pt.goto_word(p, "No"): r.do("a,wait:500"); continue
        r.do("a,wait:500")
    return False
for k in sys.argv[5].split(","):
    r.do(k + ",wait:700")
    if not run_events(): break
    if healed: break
run_events(300)
pt.shot(r, "after")
print("".join(l for l in p.log if "[fight]" in l or "[play] script" in l or "[play] map" in l))
print("fights", fights, "healed page seen", healed, "| now:", p.st["desc"], "|", r.bottom())
print("party rows:"); [print("   ", t) for t in p.text[2:12] if t.strip()]
open(pt.OUT + "/log.txt", "w").write("".join(p.log))
open(pt.OUT + "/texts.txt", "w").write("\n".join(seen))
p.quit()
