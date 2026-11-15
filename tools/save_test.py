#!/usr/bin/env python3
"""Save, quit the port, start it again, load: is the party where it was?
   save_test.py <binary> <COPY of the games dir> <game n> <out dir> [WxH]
Button presses only, through play_test.py's driver (needs real game files; scratch only).
Session 1: party set-up, a few steps, Encamp -> Save -> slot A, the port quit. Session 2: a new
process - Load Saved Game, Begin Adventuring: the same members, square and facing.
The binary may be a wrapper (the ARM program under qemu-user): presses wait on the port's answers."""
import sys, os, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt

def main():
    exe, games, n, out = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
    win = sys.argv[5] if len(sys.argv) > 5 else None
    pt.EXE, pt.OUT = exe, out
    os.makedirs(out, exist_ok=True)
    t0 = time.time()
    r = pt.Run(games, n, "s", win)
    p = r.p
    bad = False
    try:
        if not pt.start(r): raise RuntimeError("no exploring menu")
        for k in ("up", "up", "right", "up"):
            r.do(k + ",wait:350")
            if not r.exploring(): pt.calm(r)
        ok = pt.goto_word(p, "Encamp"); r.do("a,wait:800")
        if p.st["journal"] != "0":
            r.do("b,wait:500"); pt.calm(r); ok = pt.goto_word(p, "Encamp"); r.do("a,wait:800")
        for again in range(3):            # (a fight or a story can come up on the way into camp: through it, again)
            if ok and pt.goto_word(p, "Save"): break
            if p.st["fighting"] == "1": pt.fight(r)
            pt.back_out(r); pt.calm(r)
            ok = pt.goto_word(p, "Encamp"); r.do("a,wait:800")
        else: raise RuntimeError("no Save in the camp menu: " + r.bottom())
        r.do("a,wait:800"); r.do("a,wait:1500")
        for i in range(4):
            if "?" in r.bottom() or "verwrite" in r.bottom():
                pt.goto_word(p, "Yes"); r.do("a,wait:1200")
        files = pt.newest(games, t0)
        pt.back_out(r); pt.calm(r)
        was = (p.st["pos"], p.st["desc"].split("|")[0])
        r.note(f"  session 1: saved {len(files)} files at {was}")
        if not files: bad = True
    except Exception as e:
        r.note("  !! session 1 stopped:", repr(e)[:200]); bad = True
    finally:
        try: p.quit()
        except Exception: pass
    if not bad:
        r2 = pt.Run(games, n, "s", win)
        p = r2.p
        try:
            ok = pt.start(r2)
            now = (p.st["pos"], p.st["desc"].split("|")[0])
            r2.note("  session 2 (the port started anew, Load, Begin):", "the SAME place - OK" if ok and now == was else f"FAILED - now {now}, was {was}")
        except Exception as e:
            r2.note("  !! session 2 stopped:", repr(e)[:200])
        finally:
            try: p.quit()
            except Exception: pass
    notes = open(f"{out}/{n}.notes").read()
    print(notes)
    sys.exit(1 if "FAILED" in notes or "!!" in notes else 0)

main()
