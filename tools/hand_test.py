#!/usr/bin/env python3
"""A fight played by hand, and the camp / spells / items menus walked through - buttons only.
   hand_test.py <binary> <COPY of the games dir> <game n> <out dir> [fight|menus|both] [WxH]
Through play_test.py's driver (needs real game files; scratch only).
fight: walks until a fight starts, then on our fighters' turns tries, one a turn: Move by the
  D-pad (a step straight, a step diagonal - two directions together, and L1 / R1 / L2 / R2), Aim
  (Next, Target), Aim - Manual (the pad moves the aim, Target), Cast (the first spell, Target),
  Guard, Done's own menu; then Quick to the end and the treasure menu.
menus: from the exploring menu every word is lit and taken with A, its screen noted (words, a lit
  list line, L2 / R2 moving it), B back - to a depth of 4. Flags what a player would trip on: a
  word that can't be lit, an A that shows nothing, a B that doesn't come back.
A note a line; "!!" marks what needs a look."""
import sys, os, re, random, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt

def mw(p):
    """The menu line's words (row 24; "Aim:Next" -> Next)"""
    out = []
    for t in p.text[24].replace("?", " ").split():
        t = t.split(":")[-1].strip(".,")
        if t and t[0].isalpha(): out.append(t)
    return out

def litword(p):
    """The menu line's boxed word, or None (a message has none)"""
    row = p.text[24]
    for m in re.finditer(r"[A-Za-z][A-Za-z']*", row):
        if p.lit_at(24, m.start(), len(m.group(0))): return m.group(0)
    return None

def litlines(p):
    return [i for i in range(min(24, len(p.lit))) if re.search("H{4}", p.lit[i])]

def view(p):
    return (p.st["desc"], p.text[24].strip(), p.st["input"], p.st["pad"], p.st.get("canvas"))

def frows(p):
    # (two rows in Darkness, Gateway and the Krynn games; the Dark Queen's stand off the grid -
    #  the reader's lines after the 25)
    return list(range(25, len(p.text))) if len(p.text) > 25 else [23, 24]

def fmw(p):
    """A fight's menu words"""
    out = []
    for row in frows(p):
        for t in p.text[row].replace("?", " ").split():
            t = t.split(":")[-1].strip(".,")
            if t and t[0].isalpha(): out.append(t)
    return out

def flit(p):
    for row in frows(p):
        for m in re.finditer(r"[A-Za-z][A-Za-z']*", p.text[row]):
            if p.lit_at(row, m.start(), len(m.group(0))): return m.group(0)
    return None

def fgoto(p, word):
    for row in frows(p):
        if word.upper() in p.text[row].upper(): return pt.goto_word(p, word, row=row)
    return pt.goto_word(p, word)

def at_menu(p):
    # (a fighter's menu: Move among its words and one of them boxed - the move's own line has none)
    return "Move" in fmw(p) and p.st["pad"] == "0" and flit(p) is not None and not any("=" in p.text[x] for x in frows(p))

def rest(r, tries=10):
    """A step's or a blow's pictures run on: until the pad or a fighter's menu is back"""
    p = r.p
    for i in range(tries):
        if p.st["fighting"] != "1" or p.st["pad"] == "1" or at_menu(p): return
        r.do("wait:250")

def to_menu(r, tries=60):
    p = r.p
    for i in range(tries):
        if p.st["fighting"] != "1": return False
        if at_menu(p): return True
        if p.st["pad"] == "1" or (flit(p) is not None and "Move" not in fmw(p) and "Yes" not in fmw(p)):
            r.do("b,wait:350")            # (left in a move, an aim or another menu: out of it by B, nothing chosen)
        else: r.do("a,wait:350")
    return False

def moves_left(p):
    m = re.search(r"=\s*(\d+)", p.text[24])
    return int(m.group(1)) if m else None

def end_turn(r):
    p = r.p
    if p.st["pad"] == "1": r.do("a,wait:400")
    rest(r)
    if p.st["fighting"] == "1" and at_menu(p):
        for w in ("Guard", "Done"):
            if w in fmw(p) and fgoto(p, w):
                r.do("a,wait:400")
                if "Guard" in fmw(p) and "Move" not in fmw(p) and fgoto(p, "Guard"): r.do("a,wait:400")
                break

def try_move(r, keys, what):
    p = r.p
    if not fgoto(p, "Move"): return r.note(f"  !! fight {what}: Move can't be lit")
    r.do("a,wait:400")
    if p.st["pad"] != "1": return r.note(f"  !! fight {what}: A on Move opens no pad - [{p.text[24].strip()}]")
    n0 = moves_left(p); got = []
    for k in keys:
        if p.st["pad"] != "1": break
        before = (moves_left(p), p.st.get("canvas")); r.do(k + ",wait:350"); rest(r)
        if p.st["pad"] != "1": got.append(k + "=a blow / the turn over"); break
        if moves_left(p) != before[0]: got.append(k)
    r.note(f"  fight {what}: moves left {n0} ->", moves_left(p), "| steps taken by", got if got else "NONE", "" if got else "!! no key moved the fighter")
    end_turn(r)

def try_aim(r, manual):
    p = r.p; what = "Aim - Manual" if manual else "Aim"
    if "Aim" not in fmw(p): return r.note(f"  fight {what}: no Aim word for this fighter")
    fgoto(p, "Aim"); r.do("a,wait:500")
    w = fmw(p)
    if "Next" not in w and "Target" not in w: return r.note(f"  !! fight {what}: A on Aim -> [{p.text[24].strip()}]") or end_turn(r)
    if "Target" not in w and not manual:
        r.note("  fight Aim: menu", w, "- no Target word (no foe in this fighter's reach)")
        if fgoto(p, "Exit"): r.do("a,wait:400")
        return end_turn(r)
    if manual:
        if not fgoto(p, "Manual"): r.note("  !! fight Aim: Manual can't be lit")
        r.do("a,wait:400"); pad = p.st["pad"]; c0 = p.st.get("canvas"); moved = []
        for k in ("up", "up+right", "left", "down"):
            c = p.st.get("canvas"); r.do(k + ",wait:350")
            if p.st.get("canvas") != c: moved.append(k)
        r.note(f"  fight Aim - Manual: pad {pad}; the aim moved by", moved if moved else "NONE !!", "| menu", fmw(p))
    else:
        c = p.st.get("canvas"); ok = fgoto(p, "Next"); r.do("a,wait:400")
        r.note("  fight Aim: menu", w, "| Next", "moves the aim" if p.st.get("canvas") != c else "shows no change (one foe?)")
    if "Target" not in fmw(p):
        r.note(f"  fight {what}: no Target word where the aim stands now (nothing to hit there) - menu", fmw(p))
        if fgoto(p, "Exit"): r.do("a,wait:400")
        if p.st["pad"] == "1": r.do("b,wait:400")
        return end_turn(r)
    lit = fgoto(p, "Target"); r.do("a,wait:600")
    after = p.text[24].strip()
    if "Yes" in fmw(p): r.do("b,wait:400"); after += " -> B = No"
    r.note(f"  fight {what}: Target", "lit," if lit else "!! not lit,", "A ->", f"[{after[:50]}]", "pad", p.st["pad"])
    if "Target" in fmw(p) and "Exit" in fmw(p) and fgoto(p, "Exit"): r.do("a,wait:400")
    if p.st["pad"] == "1": r.do("b,wait:400")
    end_turn(r)

def try_cast(r):
    p = r.p
    fgoto(p, "Cast"); r.do("a,wait:500")
    w = fmw(p); ll = litlines(p)
    names = [re.sub(r"\?+", " ", p.text[i]).strip()[:24] for i in ll[:1]]
    r.note("  fight Cast: menu", w, "| lit spell line", names if names else "NONE !!")
    if len(ll) and "Cast" in w: fgoto(p, "Cast")
    r.do("a,wait:600")
    w2 = fmw(p)
    if "Target" in w2:
        fgoto(p, "Target"); r.do("a,wait:700"); res = p.text[24].strip()
        if "Yes" in fmw(p): r.do("b,wait:400"); res += " -> B = No"
        r.note("  fight Cast: aim menu", w2, "| Target ->", f"[{res[:50]}]")
    else:
        r.note("  fight Cast: after the spell's A ->", f"[{p.text[24].strip()[:60]}]", "pad", p.st["pad"])
    for i in range(4):
        rest(r)
        if p.st["fighting"] != "1" or at_menu(p): break
        if "Exit" in fmw(p) and fgoto(p, "Exit"): r.do("a,wait:400")
        else: r.do("b,wait:400")
    return True

def try_done(r):
    p = r.p
    if "Done" not in fmw(p): return r.note("  fight Done: no such word:", fmw(p))
    fgoto(p, "Done"); r.do("a,wait:400")
    w = fmw(p)
    r.note("  fight Done: its menu", w)
    for x in ("Delay", "Guard"):
        if x in w and "Move" not in w and fgoto(p, x):
            r.do("a,wait:400"); r.note(f"  fight Done - {x}: taken ->", f"[{p.text[24].strip()[:40]}]"); return
    if "Move" not in w: r.do("b,wait:400")

def seek_fight(r, steps_max):
    """Walks the map through, square by square (every side of every square tried once, then the
    nearest square with a side untried), until a fight starts. Overland maps: at random."""
    p = r.p; rnd = random.Random(7 + r.n); steps = 0; seen = set()
    D = {0: (0, -1), 2: (1, 0), 4: (0, 1), 6: (-1, 0)}
    maps = {}
    def here():
        x, y, d = (int(v) for v in p.st["pos"].split(","))
        return (x, y), d
    def press(k):
        nonlocal steps
        r.do(k + ",wait:130"); steps += 1
        if p.st["fighting"] != "1" and not r.exploring() and p.st["walking"] != "1": r.settle(30)
    def face(dd):
        for i in range(4):
            cur, d = here()
            if d == dd or p.st["fighting"] == "1" or not r.exploring(): return d == dd
            press("right" if (dd - d) % 8 == 2 else "left" if (dd - d) % 8 == 6 else "right")
        return here()[1] == dd
    while p.st["fighting"] != "1" and steps < steps_max:
        if p.st["desc"].startswith("Party") and "|0 char" in p.st["desc"]: break
        if not r.exploring():
            if not r.settle(30): break
            continue
        if r.words() == ["Move", "Encamp"] or p.st["walking"] != "1":
            if "Move" in r.words() and fgoto(p, "Move"): r.do("a,wait:400")
            for i in range(12):
                if p.st["fighting"] == "1": break
                press(rnd.choice(["up", "up", "left", "right", "down"]))
            pt.back_out(r)
            continue
        key = p.st["desc"].split("|")[-1]
        m = maps.setdefault(key, {"tried": {}, "open": {}})
        cur, d = here(); seen.add((key, cur))
        tried = m["tried"].setdefault(cur, set())
        untried = [x for x in (d, (d + 2) % 8, (d + 6) % 8, (d + 4) % 8) if x not in tried and x in D]
        if untried:
            dd = untried[0]
        else:
            # the nearest square with a side untried, through the ways known open
            prev = {cur: None}; queue = [cur]; goal = None
            while queue and goal is None:
                q = queue.pop(0)
                for x, to in m["open"].get(q, {}).items():
                    if to not in prev:
                        prev[to] = (q, x); queue.append(to)
                        if len(m["tried"].get(to, ())) < 4: goal = to; break
            if goal is None:
                for i in range(20):       # (all walked: a few steps at random - doors to other maps were stories)
                    if p.st["fighting"] == "1": break
                    press(rnd.choice(["up", "up", "left", "right"]))
                if len(seen) > 0 and steps > steps_max // 2: break
                continue
            while prev[goal][0] != cur: goal = prev[goal][0]
            dd = prev[goal][1]
        if not face(dd): continue
        press("up")
        tried.add(dd)
        if p.st["fighting"] == "1": break
        now = here()[0]
        if p.st["desc"].split("|")[-1] == key and now != cur:
            m["open"].setdefault(cur, {})[dd] = now
            m["open"].setdefault(now, {})[(dd + 4) % 8] = cur
            m["tried"].setdefault(now, set()).add((dd + 4) % 8)
    return steps, len(seen)

def hand_fight(r, steps_max=4000):
    p = r.p
    for i in range(3):                    # (out of the icon gallery, if the menus' walk left it up: B, then Yes)
        if r.exploring() or "Done" not in fmw(p): break
        r.do("b,wait:500")
        if "Yes" in fmw(p) and fgoto(p, "Yes"): r.do("a,wait:500")
        pt.back_out(r)
    steps, squares = seek_fight(r, steps_max)
    if p.st["fighting"] != "1":
        return r.note(f"  no fight met in {steps} steps ({squares} squares)")
    r.note(f"  A FIGHT after {steps} steps: [{p.text[24].strip()[:40]}]")
    plan = ["straight", "diagonal", "shoulder", "aim", "manual", "cast", "done"]
    turns = 0; cast_tries = 0
    while plan and turns < 14 and to_menu(r):
        turns += 1; w = fmw(p)
        if turns == 1: r.note("  fight: a fighter's menu", w); pt.shot(r, "hfight")
        a = plan[0]
        if a == "cast" and "Cast" not in w:
            cast_tries += 1
            if cast_tries > 6: plan.pop(0); r.note("  fight Cast: none of the fighters whose turn came had Cast")
            else: end_turn(r)
            continue
        plan.pop(0)
        if a == "straight": try_move(r, ["up", "right", "left", "down"], "Move, D-pad straight")
        elif a == "diagonal": try_move(r, ["up+right", "up+left", "down+right", "down+left"], "Move, two directions together")
        elif a == "shoulder": try_move(r, ["r1", "l1", "r2", "l2"], "Move, L1 / R1 / L2 / R2")
        elif a == "aim": try_aim(r, False)
        elif a == "manual": try_aim(r, True)
        elif a == "cast": try_cast(r); end_turn(r)
        elif a == "done": try_done(r)
    if plan and p.st["fighting"] == "1": r.note("  fight: not tried (turns ran out):", plan)
    elif plan: r.note("  fight: over before these were tried:", plan)
    n = 0
    while p.st["fighting"] == "1" and n < 900:
        w = fmw(p)
        if "Quick" in w and fgoto(p, "Quick"): r.do("a,wait:250")
        elif "Auto" in w and fgoto(p, "Auto"): r.do("a,wait:250")
        elif "Move" in w and "Guard" in w and fgoto(p, "Guard"): r.do("a,wait:250")
        elif "Move" in w and "Done" in w and fgoto(p, "Done"): r.do("a,wait:250")      # (Champions: Auto is under Done)
        elif "Exit" in w and "Move" not in w and "Quick" not in w and "Auto" not in w and fgoto(p, "Exit"): r.do("a,wait:250")   # (left in Aim, in the items)
        elif p.st["pad"] == "1": r.do(("b" if n % 2 else "a") + ",wait:250")      # (a move ends with A; an aim on nothing is left with B)
        else: r.do("a,wait:250")
        n += 1
    if p.st["fighting"] == "1": return r.note("  !! STILL FIGHTING after 900 presses - menu", fmw(p), "pad", p.st["pad"])
    if p.st["desc"].startswith("Party") and "|0 char" in p.st["desc"]:
        return r.note("  the party fell; the game is back at its party menu")
    for i in range(12):
        if "Take" in fmw(p) or r.exploring(): break
        r.do("a,wait:400")
    if "Take" in fmw(p):
        w = fmw(p); pt.shot(r, "htreasure")
        fgoto(p, "Take"); r.do("a,wait:500"); t1 = p.text[24].strip()
        for i in range(5):
            if "Take" in fmw(p) and "View" in fmw(p): break
            r.do("b,wait:400")
        back = "Take" in fmw(p)
        r.note("  treasure menu", w, "| Take ->", f"[{t1[:40]}]", "| B back:", "OK" if back else "!! " + p.text[24].strip()[:40])
    else:
        r.note("  after the fight: no treasure menu -", f"[{p.text[24].strip()[:40]}]")
    ok = r.settle(60)
    r.note("  back to exploring:", "OK" if ok and r.exploring() else "!! " + p.text[24].strip()[:50] + " | " + p.st["desc"])

# ---------------------------------------------------------------- the menus
SKIP = {"Exit", "Save", "Load", "Quit", "Begin", "Remove", "Area", "Search", "Look", "Move", "Yes", "No", "Next", "Prev", "Encamp_"}

class Crawl:
    def __init__(self, r, budget=140):
        self.r, self.p, self.left, self.flags = r, r.p, budget, 0
        self.stack = []                   # the menus above the one being walked
    def note(self, depth, *a):
        self.r.note("  " + "  " * depth + " ".join(str(x) for x in a))
    def back(self, words, tries=14):
        p = self.p
        for i in range(tries):
            if p.st["input"] == "0" and p.st["pad"] == "0" and (mw(p) == words or mw(p) in self.stack): break
            if p.st["journal"] != "0": self.r.do("b,wait:400"); continue
            if p.st["input"] != "0": self.r.do("y,wait:400"); continue
            if p.st["fighting"] == "1": pt.fight(self.r); continue
            if not litword(p) and p.text[24].strip() and i % 2 == 0:
                self.r.do("a,wait:500"); continue      # (a message that wants a press - "X is Blessed", a line a member)
            if "Yes" in mw(p) and "No" in mw(p):
                # (B's question - the icon gallery's "Is this icon ok?": No comes back to the same
                #  screen, so the second time Yes)
                yn = getattr(self, "yn", 0); self.yn = yn + 1
                if yn % 2 == 1 and pt.goto_word(p, "Yes"): self.r.do("a,wait:500"); continue
            self.r.do("b,wait:450")
        return mw(p) == words or mw(p) in self.stack
    def go(self, path, depth):
        r, p = self.r, self.p
        words = mw(p)
        started = False
        for w in words:
            if w in SKIP or self.left <= 0: continue
            if depth == 0 and not getattr(self, "any_top", False) and w not in ("Cast", "View", "Encamp", "Inv"): continue
            if depth > 0 and w == "View" and "Encamp" in path: continue      # (the same screens as View's own)
            if mw(p) != words:            # (a word's act went up a menu or two: in again)
                if mw(p) not in self.stack: self.left = 0; self.note(depth, "(the walk of this menu ended: now", mw(p), ")"); return
                i = self.stack.index(mw(p))
                for x in path[i:]:
                    if not pt.goto_word(p, x): break
                    r.do("a,wait:550")
                if mw(p) != words: self.note(depth, f"({'>'.join(path)}: not reached again - its other words not taken)"); return
            self.left -= 1
            if not pt.goto_word(p, w):
                # (words before the first that lights are the question's own: "Trade with Whom")
                if started: self.flags += 1; self.note(depth, f"!! {'>'.join(path + [w])}: the word can't be lit by L1 / R1")
                continue
            started = True
            before = view(p); r.do("a,wait:550"); after = view(p)
            what = []
            if p.st["input"] == "0" and p.st["pad"] == "0" and not litword(p) and p.text[24].strip():
                # (no boxed word: a message - "Must be Readied" - that goes by itself, or wants a press)
                msg = p.text[24].replace("?", " ").strip()[:36]
                for i in range(7):
                    if litword(p): break
                    r.do("wait:300")
                if not litword(p): r.do("a,wait:500")
                what.append(f"says [{msg}]")
            kids = mw(p); ll = litlines(p)
            if p.st["input"] != "0": what.append("asks for typing")
            if p.st["pad"] != "0": what.append("pad")
            if ll:
                line = re.sub(r"\?+", " ", p.text[ll[0]]).strip()[:22]
                what.append(f"list line lit [{line}]")
                if depth < 3:
                    r.do("r2,wait:250"); l2 = litlines(p)
                    what.append("R2 moves it" if l2 != ll else "R2: same line (one line?)")
                    r.do("l2,wait:250")
            if after == before and kids == words and not what:
                self.note(depth, f"{'>'.join(path + [w])}: A shows nothing new  (?)")
            else:
                self.note(depth, f"{'>'.join(path + [w])}: {kids if kids != words else '(same menu)'}", "|", ", ".join(what) if what else "-")
            if kids != words and kids and "Yes" not in kids and depth < 3 and p.st["input"] == "0" and p.st["pad"] == "0":
                self.stack.append(words); self.go(path + [w], depth + 1); self.stack.pop()
            if not self.back(words):
                self.flags += 1
                self.note(depth, f"!! {'>'.join(path + [w])}: B (6 presses) comes to no menu above it - now [{p.text[24].strip()[:50]}]")
                self.left = 0; return

def menus(r):
    p = r.p
    pt.calm(r)
    if not r.exploring(): return r.note("  !! menus: not at the exploring menu -", p.text[24].strip()[:50])
    r.note("  menus, from", mw(p), "- member", p.st["desc"])
    # (a member who can cast, so Cast and Memorize have something to show)
    for i in range(7):
        if "Cast" not in mw(p) or not pt.goto_word(p, "Cast"): break
        r.do("a,wait:500"); kids = mw(p)
        for j in range(4):
            if r.exploring(): break
            r.do("b,wait:400")
        if kids != mw(p): r.note(f"  menus: member {i + 1} can cast - Cast gives", kids); break
        r.do("r2,wait:250")
    c = Crawl(r)
    c.go([], 0)
    r.note(f"  menus: {140 - c.left} words taken, {c.flags} flagged")

def main():
    exe, games, n, out = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
    mode = sys.argv[5] if len(sys.argv) > 5 else "both"
    win = sys.argv[6] if len(sys.argv) > 6 else None
    pt.EXE, pt.OUT = exe, out
    os.makedirs(out, exist_ok=True)
    r = pt.Run(games, n, "h", win)
    try:
        if pt.start(r):
            if mode in ("menus", "both"): menus(r)
            if mode in ("fight", "both"): hand_fight(r)
    except Exception as e:
        import traceback
        r.note("  !! THE RUN STOPPED:", repr(e)[:200], traceback.format_exc().strip().splitlines()[-3][:120])
    finally:
        try: r.p.quit()
        except Exception: pass
    print(open(f"{out}/{n}.notes").read())

if __name__ == "__main__":
    main()
