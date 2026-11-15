#!/usr/bin/env python3
"""The button play test: plays a game with button presses alone and reads the screen back.

    tools/play_test.py <goldbox binary> <games dir> <game number> <out dir> [WxH]

The port is started with --drive (src/main.cpp): script steps come in on stdin, and after each
line it answers with its state and the game screen read back as text (each 8 x 8 cell matched
against the Clear font; pictures read as '?'). The test never taps: every step is a button a
handheld has. Per game it does: title -> party menu, Load Saved Game (or two characters made
and added where the release ships no saved game), View, Begin Adventuring, D-pad walking (an
overland map: Move, steps, B = Exit), X = Area, Y = the Companion, Encamp -> Save, the Start
menu's Journal, Quick Actions and the four Cheats, and a fight if one is met (Move's direction
pad, then Quick / Auto to its end, the treasure, back to exploring).

It needs real game files, so it is run by hand, not by CI; give each run a COPY of the games
folder (it saves over slot A). Notes go to <out>/<n>.notes as they are made, every press with
the menu line it led to to <out>/<n>.trail, screenshots (real game screens: scratch only, never
the repo) to <out>. The Dark Queen's screens are pictures the reader can't read: check that
game with --script and shot: steps (tools/play_shots.py).
"""
import os, random, re, subprocess, sys, time

EXE = "build/goldbox"
OUT = "."
class Port:
    def __init__(self, games, game, conf, window=None):
        os.makedirs(conf, exist_ok=True)
        with open(conf + "/settings.cfg", "w") as f:
            f.write("font=1\nvolume=0\nreply_asked=1\n")
        env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
        args = [EXE, "--games", games, "--conf", conf, "--game", str(game), "--drive"] + (["--window", window] if window else [])
        self.p = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env, text=True, bufsize=1)
        self.log = []
        self.st = {}; self.text = []; self.lit = []
    def do(self, steps="wait:150"):
        self.p.stdin.write(steps + "\n"); self.p.stdin.flush()
        text, lit = [], []
        while True:
            line = self.p.stdout.readline()
            if not line: raise RuntimeError("port ended\n" + "".join(self.log[-25:]))
            line = line.rstrip("\n")
            if line.startswith("[state]"):
                head, _, desc = line[8:].partition(" desc=")
                self.st = dict(kv.split("=", 1) for kv in head.split(" ") if "=" in kv)
                self.st["desc"] = desc
            elif line.startswith("T|"): text.append(line[2:42])
            elif line.startswith("L|"): lit.append(line[2:42])
            elif line == "[end]": break
            elif not line.startswith("[script]"): self.log.append(line + "\n")
        self.text, self.lit = text, lit
        return self
    def show(self):
        print(self.st)
        for t in self.text:
            if t.strip(): print("  |" + t + "|")
    def find(self, word):
        W = word.upper()
        for r, t in enumerate(self.text):
            c = t.upper().find(W)
            if c >= 0: return r, c
        # over a picture some cells can't be read ('?'): those match anything, most must be read
        for r, t in enumerate(self.text):
            T = t.upper()
            for c in range(0, 40 - len(W) + 1):
                seg = T[c:c + len(W)]
                if all(a == b or a == "?" for a, b in zip(seg, W)) and sum(a == b for a, b in zip(seg, W)) * 10 >= len(W) * 6:
                    return r, c
        return None
    def has(self, word): return self.find(word) is not None
    def lit_at(self, r, c): return self.lit[r][c] == "H"
    def quit(self):
        try:
            self.p.stdin.write("quit\n"); self.p.stdin.flush(); self.p.wait(10)
        except Exception: self.p.kill()

def to_party_menu(p, tries=30):
    p.do("wait:1500")
    # (the title screens: the port keeps the Companion off them - its title= must say so)
    if p.st.get("title") != "1": print("!! title=0 while the title screens are up")
    for i in range(tries):
        if p.has("Load Saved") or p.has("Create New"): return i
        p.do("a,wait:700")
    return -1

def goto_line(p, word, key="r2", tries=16):
    """The list's highlight moved (L2 / R2) onto the line holding word; True when it is lit"""
    for i in range(tries):
        at = p.find(word)
        if at is None: return False
        r, c = at
        if p.lit_at(r, c): return True
        p.do(key + ",wait:120")
    return False

def goto_word(p, word, key="r1", tries=14, row=None):
    """The menu line's highlight moved (L1 / R1) onto word"""
    for i in range(tries):
        rows = [row] if row is not None else [24, 23, 22]
        at = None
        for r in rows:
            c = p.text[r].upper().find(word.upper()) if r < len(p.text) else -1
            if c >= 0: at = (r, c); break
        if at is None: return False
        if p.lit_at(*at): return True
        p.do(key + ",wait:120")
    return False

def games_of(p):
    return [re.match(r"\[port\]   (.*?): ", l).group(1) for l in p.log if re.match(r"\[port\]   .*?: ", l)]


class Run:
    def __init__(self, games, n, tag, window=None):
        self.p = Port(games, n, f"{OUT}/conf-{n}", window)
        self.n, self.tag = n, tag
        self.notes, self.trail = [], []
    def note(self, *a):
        s = " ".join(str(x) for x in a); self.notes.append(s)
        open(f"{OUT}/{self.n}.notes", "a").write(s + "\n")
    def do(self, steps):
        self.p.do(steps); open(f"{OUT}/{self.n}.trail", "a").write(f"{steps:<22} | {self.bottom()[:60]}\n"); self.trail.append(f"{steps:<22} | {self.p.st.get('desc','')[:26]:<26} | {self.bottom()[:60]}")
        return self.p
    def bottom(self):
        return self.p.text[24].strip() if len(self.p.text) > 24 else ""
    def exploring(self):
        return "Encamp" in self.bottom() and self.p.st["fighting"] == "0" and self.p.st["journal"] == "0"
    def words(self):
        t = self.p.text
        return ((t[23] if len(t) > 23 and "?" not in t[23] else "") + " " + self.bottom()).split()
    def log_has(self, s, since=0):
        return any(s in l for l in self.p.log[since:])

    # One sensible press when the game wants something that isn't the exploring menu
    def step_on(self):
        b = self.bottom()
        if self.p.st["journal"] != "0":
            if not getattr(self, "jseen", False):
                self.jseen = True; shot(self, "jentry"); self.note("  the game named a journal entry and the viewer opened it; B closes it")
            return self.do("b,wait:500")
        if "?" in "".join(self.p.text[24:25]) and not b.replace("?", "").strip():
            return self.do("a,wait:600")
        if ("Press" in b or "press" in b or "continue" in b or len(self.words()) <= 1):
            return self.do("a,wait:500")
        for w in ("Exit", "Continue", "No", "Leave", "Done"):
            if w in self.words() and goto_word(self.p, w):
                return self.do("a,wait:500")
        return self.do("a,wait:500")

    def settle(self, tries=50):
        for i in range(tries):
            if self.exploring() or self.p.st["fighting"] == "1" or "battle" in self.bottom().lower(): return True
            self.step_on()
        return False

def shot(r, name):
    r.p.do(f"shot:{OUT}/{r.n}-{name}.bmp")

def count(p):
    m = re.search(r"\|(\d+) char", p.st["desc"])
    return int(m.group(1)) if m else -1

def create(r, name):
    """Create New Character with the buttons alone: every list's first choice, the stats kept, the name
    typed on the port's keyboard, the icon as it comes"""
    p = r.p
    before = count(p)
    if not goto_line(p, "Create New"): return False
    r.do("a,wait:600")
    for i in range(60):
        if p.has("Create New Character") and i > 3: return True
        b = r.bottom()
        w = r.words()
        if p.st["input"] != "0":
            r.do(f"type:{name},wait:200,y,wait:600")
        elif "Reroll" in b and goto_word(p, "No"): r.do("a,wait:600")
        elif "Keep" in w and goto_word(p, "Keep"): r.do("a,wait:600")
        elif "Done" in w and goto_word(p, "Done"): r.do("a,wait:600")
        elif "Yes" in w and goto_word(p, "Yes"): r.do("a,wait:600")
        else: r.do("a,wait:600")
    return p.has("Create New Character") and count(p) >= before

def add(r, name):
    p = r.p
    before = count(p)
    if count(p) > 0 and name[:5] in "".join(p.text[:12]): return True
    if not goto_line(p, "Add Character"): return False
    r.do("a,wait:600")
    for i in range(12):
        if count(p) > before: break
        at = p.find(name)
        if at and not p.lit_at(*at): r.do("r2,wait:150"); continue
        r.do("a,wait:600")
    ok = count(p) > before
    for i in range(6):
        if p.has("Create New Character") and "Select" in r.bottom(): break
        r.do("b,wait:500")
    return ok

def start(r):
    p = r.p
    k = to_party_menu(p, 70)
    r.name = games_of(p)[r.n - 1]
    r.note(f"== {r.tag}{r.n} {r.name}")
    r.note("  title -> party menu:", "OK" if k >= 0 else "FAILED", f"({k} presses)")
    if k < 0: return False
    if p.has("Load Saved") and goto_line(p, "Load Saved"):
        r.do("a,wait:600")
        if "from where" in r.bottom().lower(): r.do("a,wait:600")
        if "which game" in r.bottom().lower(): r.do("a,wait:1500")
    if p.st["desc"].startswith("Party") and "|0 char" in p.st["desc"]:
        # no saved game shipped: two characters made from scratch (the whole Create New Character path)
        for i in range(2):
            ok = create(r, f"TEST{chr(65 + i)}")
            r.note(f"  Create New Character #{i + 1} by buttons:", "OK" if ok else "FAILED", "->", p.st["desc"])
            if not ok: break
            ok = add(r, f"TEST{chr(65 + i)}")
            r.note(f"  Add Character to Party #{i + 1} by buttons:", "OK" if ok else "FAILED", "->", p.st["desc"])
    else:
        r.note("  Load Saved Game by buttons: OK ->", p.st["desc"])
    if p.st["desc"].startswith("Party"):
        if "|0 char" in p.st["desc"]:
            r.note("  NO PARTY - can't go on"); return False
        if not p.has("Begin Adventuring"):
            # (Treasures draws this menu over a picture the reader can't read through: the lines
            # by count from where the highlight stands after a load - Modify)
            r.do("r2,wait:200"); r.do("a,wait:700"); v = p.st["desc"].startswith("View"); r.do("b,wait:500")
            r.note("  View Character, B back (lines counted):", "OK" if v and p.st["desc"].startswith("Party") else "FAILED")
            for i in range(4): r.do("r2,wait:200")
            r.do("a,wait:1500")
        elif goto_line(p, "View Character"):
            r.do("a,wait:600")
            if "Pick Character" in r.bottom(): r.do("a,wait:600")
            v = p.st["desc"].startswith("View") or "Items" in r.bottom(); r.do("b,wait:500")
            r.note("  View Character, B back:", "OK" if v and p.has("Begin Adventuring") else "FAILED")
        if p.has("Begin Adventuring"):
            ok = goto_line(p, "Begin Adventuring")
            r.do("a,wait:1500")
            r.note("  Begin Adventuring lit by L2/R2:", "OK" if ok else "FAILED")
    ok = r.settle(70)
    r.note("  reached the exploring menu:", "OK" if ok else "FAILED", "-", r.bottom(), "| walking", p.st["walking"])
    return ok


def newest(games, since):
    out = []
    for root, d, files in os.walk(games):
        if "_CYD" in root: continue
        for f in files:
            pth = os.path.join(root, f)
            if os.path.getmtime(pth) > since and "SAV" in f.upper(): out.append(f)
    return sorted(out)

def back_out(r, tries=12):
    for i in range(tries):
        if r.exploring() or r.p.st["fighting"] == "1": return True
        r.do("b,wait:500")
    return r.settle(20)

def calm(r):
    p = r.p
    for i in range(8):
        if p.st["fighting"] == "1" or "battle" in r.bottom().lower():
            for k in range(20):
                if p.st["fighting"] == "1": break
                r.do("a,wait:500")
            fight(r)
        elif r.exploring(): return True
        else: r.settle(30)
    return r.exploring()

def phase2(r, games, t0):
    p = r.p
    calm(r)
    # --- walking with the D-pad
    pos0 = p.st["pos"]; seen = {pos0}
    for k in ["up", "right", "up", "left", "left", "up", "right", "right", "up", "down", "up"]:
        r.do(k + ",wait:450")
        seen.add(p.st["pos"])
        if not r.exploring(): break
    if r.words() == ["Move", "Encamp"]:
        # an overland map with a marker: Move, then the D-pad steps, B = Exit
        from PIL import Image, ImageChops
        def snap(n):
            shot(r, n); return Image.open(f"{OUT}/{r.n}-{n}.bmp").convert("RGB")
        goto_word(p, "Move"); r.do("a,wait:500"); a = snap("ov0"); mv = r.bottom()
        for k in ["right", "right", "down", "left", "up"]: r.do(k + ",wait:500")
        b = snap("ov1"); box = ImageChops.difference(a, b).getbbox()
        if not r.exploring() and "Exit" not in r.words(): calm(r)
        r.do("b,wait:500")
        r.note("  overland map: Move, D-pad steps, B = Exit:", "OK" if box and r.words() == ["Move", "Encamp"] else "FAILED", f"(menu [{mv}], picture changed {box}, after B [{r.bottom()}])")
    else:
        r.note("  D-pad walks / turns:", "OK" if len(seen) > 2 else "FAILED", f"({len(seen)} positions from {pos0})")
    calm(r)
    # --- X = Area
    a = "".join(p.text[:16]); r.do("x,wait:500"); b = "".join(p.text[:16]); shot(r, "area"); r.do("x,wait:500"); c = "".join(p.text[:16])
    r.note("  X = Area map and back:", "OK" if a != b and a == c else f"? (changed {a != b}, back {a == c})")
    # --- Y = Companion (beside / under the game on a wide / square panel: Y takes it away and brings it back)
    if p.st.get("dock") == "1":
        shot(r, "companion")
        r.do("y,wait:500"); off = p.st.get("dock"); r.do("y,wait:500"); on = p.st.get("dock")
        r.note("  Y = Companion beside / under the game off and on:", "OK" if (off, on) == ("0", "1") else f"? ({off}, {on})")
    v = []
    for i in range(0 if p.st.get("dock") == "1" else 4):
        r.do("y,wait:700"); v.append(p.st["panel"])
        if i == 0: shot(r, "companion")
        if p.st["panel"] in ("0", "none", "off"): break
    if v: r.note("  Y = Companion views:", " -> ".join(v))
    while p.st["panel"] not in ("0", "none", "off") and len(v) < 8: r.do("y,wait:500"); v.append(p.st["panel"])
    calm(r)
    # --- Encamp, Save
    ok = goto_word(p, "Encamp"); r.do("a,wait:800")
    if p.st["journal"] != "0":            # (a journal entry the game had just named opens on this press)
        r.do("b,wait:500"); calm(r); ok = goto_word(p, "Encamp"); r.do("a,wait:800")
    camp = r.bottom()
    saved = False
    if ok and goto_word(p, "Save"):
        r.do("a,wait:800"); q = r.bottom()
        r.do("a,wait:1500")
        for i in range(4):
            if "?" in r.bottom() or "verwrite" in r.bottom():
                goto_word(p, "Yes"); r.do("a,wait:1200")
        files = newest(games, t0)
        saved = len(files) > 0
        r.note("  Encamp -> Save -> slot A:", "OK" if saved else "FAILED", f"camp menu [{camp}] asked [{q}] wrote {files[:4]}")
    else:
        shot(r, "encamp")
        r.note("  Encamp -> Save:", "FAILED - camp menu:", camp, "|", " / ".join(t.strip() for t in p.text[16:23] if t.strip() and "?" not in t)[:200])
    back_out(r)
    r.note("  B leaves the camp:", "OK" if r.exploring() else "FAILED - " + r.bottom())
    calm(r)
    # --- the Start menu: Journal, Quick Actions, Cheats
    r.do("start,wait:300"); m = p.st["menu"]
    r.do("menu:Journal,a,wait:500"); j = p.st["jlist"] + "/" + p.st["notice"]; shot(r, "journal"); r.do("b,wait:400")
    if p.st["menu"] == "1": r.do("b,wait:300")
    if p.st["notice"] != "0": r.do("a,wait:300")
    r.note("  Start menu opens:", "OK" if m == "1" else "FAILED", "| Journal list/notice:", j)
    r.do("start,wait:300,menu:Quick,a,wait:500"); q = p.st["quick"]; shot(r, "quick")
    r.do("a,wait:200,a,wait:200,down,wait:200,right,wait:200,b,wait:400")
    while p.st["menu"] == "1" or p.st["quick"] == "1" or p.st["notice"] != "0": r.do("b,wait:300")
    r.note("  Quick Actions page:", "OK" if q == "1" else "FAILED")
    for name, want in (("Healer", ""), ("Merchant", ""), ("Sanctuary", ""), ("Trainer", "")):
        r.do("start,wait:300,menu:Cheats,a,wait:400")
        r.do(f"menu:{name},a,wait:1200")
        desc, bot, note = p.st["desc"], r.bottom(), p.st["notice"]
        shot(r, name.lower())
        if note != "0": r.do("a,wait:300")
        if name == "Trainer":
            tr = goto_line(p, "Begin"); r.do("a,wait:1200")
            r.note("    the Trainer's Begin goes back to the game:", "OK" if tr and r.exploring() else "FAILED - " + r.bottom())
        got = back_out(r, 14)
        r.note(f"  Cheats -> {name}:", "opened" if note == "0" and not ("Encamp" in bot) else "NOT OPENED", f"[{desc}] [{bot[:44]}]", "| back by B:", "OK" if got else "FAILED - " + r.bottom())
        calm(r)
    # --- a fight
    if not getattr(r, "fought", False):
        steps = 0
        rnd = random.Random(7)
        while p.st["fighting"] != "1" and steps < 260:
            r.do(rnd.choice(["up", "up", "up", "left", "right"]) + ",wait:260"); steps += 1
            if "battle" in r.bottom().lower(): break
            if not r.exploring() and p.st["fighting"] != "1": r.settle(25)
        if p.st["fighting"] == "1" or "battle" in r.bottom().lower(): calm(r)
        else: r.note(f"  no fight met in {steps} steps of wandering")

def fight(r):
    p = r.p
    r.fought = True
    shot(r, "fight")
    r.note("  A FIGHT:", r.bottom())
    moved = pad = False
    for i in range(40):                    # to the first menu of one of ours
        if "Move" in r.words() and ("Quick" in r.words() or i > 4): break
        if p.st["fighting"] != "1": break
        r.do("a,wait:400")
    if "Move" in r.words() and goto_word(p, "Move"):
        r.do("a,wait:400"); pad = p.st["pad"] == "1"
        t0 = "".join(p.text[:24]); r.do("up,wait:500"); r.do("left,wait:500"); r.do("r1,wait:500"); moved = "".join(p.text[:24]) != t0 or True
        shot(r, "fightmove"); r.do("a,wait:500")
    r.note("  fight: Move opens the direction pad:", "OK" if pad else "FAILED - " + r.bottom())
    q = False
    for i in range(30):
        if "Quick" in r.words() and goto_word(p, "Quick"): r.do("a,wait:400"); q = True; break
        if p.st["fighting"] != "1": break
        r.do("a,wait:400")
    r.note("  fight: Quick chosen by L1/R1 + A:", "OK" if q else "not reached - " + r.bottom())
    n = 0
    while p.st["fighting"] == "1" and n < 900:
        w = r.words()
        if "Quick" in w and goto_word(p, "Quick"): r.do("a,wait:300")
        elif "Auto" in w and goto_word(p, "Auto"): r.do("a,wait:300")
        elif "Move" in w and "Guard" in w and goto_word(p, "Guard"): r.do("a,wait:300")
        elif "Move" in w and "Done" in w and goto_word(p, "Done"): r.do("a,wait:300")
        else: r.do("a,wait:300")
        n += 1
    r.note("  fight ended:", "OK" if p.st["fighting"] != "1" else "STILL FIGHTING", f"after {n} presses ->", r.bottom())
    shot(r, "afterfight")
    ok = r.settle(60)
    r.note("  back to exploring after the fight / treasure:", "OK" if ok and r.exploring() else "FAILED - " + r.bottom() + " | " + p.st["desc"])


if __name__ == "__main__":
    if len(sys.argv) < 5:
        sys.exit(__doc__)
    EXE, G, n, OUT = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
    win = sys.argv[5] if len(sys.argv) > 5 else None
    os.makedirs(OUT, exist_ok=True)
    for f in (f"{OUT}/{n}.notes", f"{OUT}/{n}.trail"):
        if os.path.exists(f): os.remove(f)
    t0 = time.time()
    r = Run(G, n, "g", win)
    try:
        if start(r): phase2(r, G, t0)
    except Exception as e:
        r.note("  !! THE RUN STOPPED:", repr(e)[:300])
    finally:
        print("\n".join(r.notes))
        try: r.p.quit()
        except Exception: pass
    sys.exit(1 if any("FAILED" in s or "!!" in s for s in r.notes) else 0)
