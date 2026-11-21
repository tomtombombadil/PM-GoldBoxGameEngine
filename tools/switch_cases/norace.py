"""Disable Race Level Limits (the Cheats' switch; Tom, 2026-11-21) - its EFFECT by the port's buttons.
Darkness, the Steam release's second saved game: Oratisi, an elf fighter / magic-user / thief 5 / 9 / 12 with
500,000 experience, Strength 18 and Intelligence 18, trained at the Cheats' Trainer until it says no. Off: the
elf's limits stop it at 7 / 11. On: the fighter goes on (9 with 500,000).

  GAMES=<a scanned copy of the Steam folders> OUT=<scratch> EXE=build/goldbox  norace.py <game n> tag=cfg ...
     e.g. norace.py 4 off= on=no_race_limits=1
"""
import os, sys, shutil, re, glob
OUT = os.environ.get('OUT', '/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import play_test as pt
pt.EXE = os.environ.get("EXE", "build/goldbox"); GAMES = os.environ["GAMES"]

def run(n, cfg, tag):
    D = OUT + f"/norace_{tag}"; shutil.rmtree(D, ignore_errors=True); shutil.copytree(GAMES, D + "/games")
    for f in glob.glob(D + "/games/*ark*/**/SAVE/*", recursive=True):       # (the second saved game as the first)
        b = os.path.basename(f)
        m = re.match(r"(SAVGAM|CHRDAT)B(.*)", b, re.I)
        if m: shutil.copyfile(f, os.path.join(os.path.dirname(f), m.group(1) + "A" + m.group(2)))
    pt.OUT = D; pt.CFG_EXTRA = cfg + "\n"
    r = pt.Run(D + "/games", n, "t"); p = r.p; pt.start(r)
    for i in range(12):                                   # (Rolf's question: No)
        if p.st.get("walking") == "1": break
        r.do("b,wait:700")
    r.do("start,wait:300,menu:Cheats,a,wait:400"); r.do("menu:Trainer,a,wait:1200")
    said = []
    for t in range(6):
        if not pt.goto_line(p, "Train"): said.append("no Train line"); break
        r.do("a,wait:700")
        if "Pick" in r.bottom() or "Select" in r.bottom():
            if not pt.goto_line(p, "ORATISI", "down", 8): said.append("Oratisi not chosen")
            r.do("a,wait:900")
        for k in range(14):
            txt = " ".join(x.strip("? ") for x in p.text[1:23] if x.strip("? ")) if not p.has("Create New") else ""
            b = r.bottom()
            if txt and (not said or said[-1] != txt) and ("a level" in txt.lower() or "experience" in txt.lower() or "train" in b.lower() and "not" in b.lower()): said.append(txt + " | " + b.strip())
            if p.has("Create New") and "Pick" not in b: break       # (the hall's menu again)
            if "Yes" in b: pt.goto_word(p, "Yes")
            r.do("a,wait:700")
    p.quit(); shutil.rmtree(D)
    return said

n = int(sys.argv[1])
for tag, cfg in [(a.split("=")[0], a.split("=", 1)[1].replace("\\n", "\n")) for a in sys.argv[2:]]:
    print(tag + ":")
    for s in run(n, cfg, tag): print("   ", re.sub(r"\s+", " ", s)[:150])
