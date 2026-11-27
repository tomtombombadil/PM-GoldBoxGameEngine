#!/usr/bin/env python3
"""tools/drive_steps.py <binary> <COPY of the games dir> <out dir> steps...
The first game of the folder to its party menu, then the steps by the port's buttons, the screen's text
printed at each "show" and at the end - for a quick look at one thing (a switch's effect, a list, a sheet).
Steps: a --script step (a, r2,wait:300, start, menu:Game_Play ...), line:<a menu line's start> /
word:<a menu word> (_ for a space) to put the highlight there, show, log (the port's log lines since the last). Needs real game files: scratch only.
   e.g. line:Load_Saved a,wait:800 a,wait:1500 line:BEGIN a,wait:1500 r2,wait:300 word:View a,wait:800 show"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt
pt.EXE, games, pt.OUT = sys.argv[1], sys.argv[2], sys.argv[3]
steps = sys.argv[4:]
r = pt.Run(games, int(os.environ.get("GAME_N", "1")), "sw", os.environ.get("WINDOW")); p = r.p   # (GAME_N=n: the folder's n-th game)
def show(tag):
    print("==", tag, "|", r.bottom(), "|", p.st.get("desc"))
    for t in p.text[:24]:
        s = re.sub(r"\?+", " ", t).rstrip()
        if s.strip(): print("   |", s)
print("party menu:", pt.to_party_menu(p, 70))
for s in steps:
    if s.startswith("line:"): print("goto_line", s[5:], pt.goto_line(p, s[5:].replace("_", " ")))
    elif s.startswith("word:"): print("goto_word", s[5:], pt.goto_word(p, s[5:].replace("_", " ")))
    elif s == "show": show("")
    elif s == "log":                    # (the port's own lines since the last "log" - the engine's [play] log)
        print("".join("   log: " + l for l in p.log)); p.log.clear()
    else: r.do(s)
show("end")
