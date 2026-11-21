"""Disable Race Level Limits in real use, by the port's buttons (Tom, 2026-11-21: "at, near, or past the level
limit ... turn it on ... and then turn the cheat back off ... across the games").

A prepared saved game (the CYD playsim's RECSET + SAVESLOT on the game's own save: member 1 an elf fighter with
Strength 18 - its limit 7 - and experience for everything; scratch, made from game files) is loaded, the Cheats'
Trainer opened and Train taken: first with the cheat OFF, then - in the same game, switched at Start - Cheats -
with it ON. What the game says each time is printed.

  GAMES=<folder: the game's folder + _CYD, scanned> OUT=<scratch> EXE=build/goldbox  racecap.py <slot letter> ...
"""
import os, sys, shutil, re, glob
OUT = os.environ.get('OUT', '/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import play_test as pt
pt.EXE = os.environ.get("EXE", "build/goldbox"); GAMES = os.environ["GAMES"]

def attempt(r):
    """Train, member 1: every screen's words until the hall's menu is back"""
    p = r.p; said = []
    if not pt.goto_line(p, "Train"): return ["no Train line"]
    r.do("a,wait:800")
    if "Pick" in r.bottom() or "Select" in r.bottom(): r.do("a,wait:900")
    for k in range(16):
        b = r.bottom().strip()
        if p.has("Create New") and "Pick" not in b and k: break
        txt = re.sub(r"\s+", " ", " ".join(x.strip("? ") for x in p.text[1:23] if x.strip("? "))) if not p.has("Create New") else ""
        line = (txt + " | " + b).strip(" |")
        if line and (not said or said[-1] != line) and re.search(r"level|experience|train|cannot|can't|not ", line, re.I): said.append(line)
        if "Yes" in b: pt.goto_word(p, "Yes")
        r.do("a,wait:800")
    return said or ["(nothing said: the hall's menu again)"]

def cheat(r):
    """Start - Cheats - Disable Race Level Limits flipped, back to the game"""
    r.do("start,wait:300,menu:Cheats,a,wait:400"); r.do("menu:Disable_Race,wait:200,a,wait:300"); r.do("b,wait:200,b,wait:300")

def run(slot):
    D = OUT + f"/racecap_{slot}"; shutil.rmtree(D, ignore_errors=True); shutil.copytree(GAMES, D + "/games")
    for f in glob.glob(D + "/games/**/*", recursive=True):
        b = os.path.basename(f)
        m = re.match(r"(SAVGAM|CHRDAT)" + slot + r"(.*)", b, re.I)
        if m and "/_CYD/" not in f: shutil.copyfile(f, os.path.join(os.path.dirname(f), m.group(1) + "A" + m.group(2)))
    pt.OUT = D; pt.CFG_EXTRA = "no_race_limits=0\n"
    r = pt.Run(D + "/games", 1, "t"); p = r.p; pt.start(r)
    for i in range(12):
        if p.st.get("walking") == "1": break
        r.do("b,wait:700")
    r.do("start,wait:300,menu:Cheats,a,wait:400"); r.do("menu:Trainer,a,wait:1200")
    who = next((t.strip("? ")[:16].strip() for t in p.text[3:10] if t.strip("? ")), "?")
    print(f"  slot {slot} ({who}) the cheat OFF:")
    for s in attempt(r): print("      ", s[:150])
    cheat(r)
    print(f"  slot {slot} the cheat ON:")
    for s in attempt(r): print("      ", s[:150])
    p.quit(); shutil.rmtree(D)

for slot in sys.argv[1:]: run(slot.upper())
