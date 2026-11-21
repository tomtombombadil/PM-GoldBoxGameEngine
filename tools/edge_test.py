#!/usr/bin/env python3
"""Pools of Darkness, Phlan's two maps (v0.12.8; Tom: the gate by the Temple of Tempus put the party in the water
on the far side of the same map). By the port's buttons: the game's own saved game, Rolf's tour refused, the walk
from 11,2 to the west gate at 0,4, a step out - the place must be map 17's 15,4, not this map's 15,4 - and back.

  tools/edge_test.py <binary> <games copy> <out dir>     (a COPY: scanned by this version; real game files)
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt

def main():
    exe, games, out = sys.argv[1:4]
    pt.EXE, pt.OUT = exe, out
    os.makedirs(out, exist_ok=True)
    r = pt.Run(games, 4, "edge")
    p = r.p
    bad = 0
    try:
        if not pt.start(r): print("!! no start"); return 1
        for i in range(12):                       # (Rolf's question: B = No; then his goodbye)
            if p.st.get("walking") == "1": break
            r.do("b,wait:700")
        def where(): return p.st.get("pos", p.st.get("desc", ""))
        print("  after the tour refused:", p.st)
        for step in "up up right".split() + ["up"] * 11:
            r.do(step + ",wait:350")
        def on():                                 # (a place's text waits for a press)
            for i in range(6):
                if p.st.get("walking") == "1": return
                p.show(); r.do("a,wait:700")
        on()
        print("  at the gate:", p.st.get("pos"), p.st.get("desc"))
        at_gate = dict(p.st)
        r.do("up,wait:900"); r.do("wait:400"); on()
        print("  a step west:", p.st.get("pos"), p.st.get("desc"))
        pt.shot(r, "edge_west")
        out_st = dict(p.st)
        r.do("down,wait:500"); r.do("up,wait:900"); r.do("wait:400"); on()     # (turned round, a step east)
        print("  and back east:", p.st.get("pos"), p.st.get("desc"))
        back_st = dict(p.st)
        pt.shot(r, "edge_back")
        if not (out_st.get("pos", "").startswith("15,4,") and "Map 17" in out_st.get("desc", "")):
            print("!! the step west did not come out at map 17's 15,4"); bad += 1
        if not (back_st.get("pos", "").startswith("0,4,") and "Map 16" in back_st.get("desc", "")):
            print("!! the step back did not come out at map 16's 0,4"); bad += 1
    finally:
        p.quit()
    print("edge test:", "FAILED" if bad else "passed")
    return bad

if __name__ == "__main__":
    sys.exit(main())
