#!/usr/bin/env python3
"""Playing further in, by buttons: a character made, the icon screen tried, then the maps walked
through for a long while - fights fought (Quick), every shop / temple / training hall met walked
through word by word, journal mentions counted, and every place the walk couldn't get out of noted.
   explore_test.py <binary> <COPY of the games dir> <game n> <out dir> [steps] [minutes]
Through play_test.py's driver and hand_test.py's menu walk (needs real game files; scratch only).
The notes end with one line of counts; "!!" marks what needs a look (a shot is taken there)."""
import sys, os, re, random, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt
import hand_test as ht

PLACE = ("Buy", "Sell", "Heal", "Train", "Appraise")

def icon_screen(r):
    p = r.p
    for w in ("Encamp", "Alter", "Icon"):
        if w not in ht.mw(p) or not pt.goto_word(p, w): return r.note(f"  icon screen: no {w} word here -", ht.mw(p)) or pt.back_out(r)
        r.do("a,wait:600")
    words = ht.mw(p); got = []
    for k in ("right", "down", "left", "up", "r2", "l2"):
        c = p.st.get("canvas"); r.do(k + ",wait:300")
        if p.st.get("canvas") != c: got.append(k)
    took = []
    for w in words:
        if w in ("Done", "Exit", "Edit"): continue
        if not pt.goto_word(p, w): continue
        c = p.st.get("canvas"); r.do("a,wait:400"); a = p.st.get("canvas") != c
        c = p.st.get("canvas"); r.do("right,wait:300"); b = p.st.get("canvas") != c
        took.append(f"{w}:{'A changes' if a else 'A same'}/{'D-pad changes' if b else 'D-pad same'}")
    edit = None
    if "Edit" in words and pt.goto_word(p, "Edit"):
        r.do("a,wait:500"); edit = ht.mw(p)
        for w in edit:
            if w == "Exit" or not pt.goto_word(p, w): continue
            c = p.st.get("canvas"); r.do("a,wait:400")
            took.append(f"Edit>{w}:{'changes' if p.st.get('canvas') != c else 'same'}")
        r.do("b,wait:400")
    pt.shot(r, "icon")
    q = []
    for i in range(6):                    # (B out of Edit's levels, then B asks "Is this icon ok?" - Yes)
        r.do("b,wait:500"); q = ht.mw(p)
        if "Yes" in q:
            if pt.goto_word(p, "Yes"): r.do("a,wait:600")
            break
    out = "Order" in ht.mw(p) or "Speed" in ht.mw(p)
    r.note("  icon screen", words, "| the picture changes on", got if got else "NO KEY", "|", "; ".join(took), "| Edit", edit,
           "| B asks", q, "- Yes leaves:", "OK" if out else "!! " + p.text[24].strip()[:40])
    pt.back_out(r); pt.calm(r)

def quick_fight(r):
    p = r.p; n = 0
    while p.st["fighting"] == "1" and n < 1200:
        w = ht.fmw(p)
        if "Quick" in w and ht.fgoto(p, "Quick"): r.do("a,wait:200")
        elif "Auto" in w and ht.fgoto(p, "Auto"): r.do("a,wait:200")
        elif "Move" in w and "Done" in w and ht.fgoto(p, "Done"): r.do("a,wait:200")
        elif "Exit" in w and "Move" not in w and "Quick" not in w and "Auto" not in w and ht.fgoto(p, "Exit"): r.do("a,wait:200")
        elif p.st["pad"] == "1": r.do(("b" if n % 2 else "a") + ",wait:200")
        else: r.do("a,wait:200")
        n += 1
    return p.st["fighting"] != "1"

def explore(r, steps_max, minutes):
    p = r.p; rnd = random.Random(11 + r.n); t_end = time.time() + minutes * 60
    D = (0, 2, 4, 6)
    S = {"steps": 0, "squares": set(), "maps": set(), "fights": 0, "journal": 0, "places": {}, "stuck": 0, "menus": set()}
    was_j = [False]; mend = [False]
    orig = r.do
    def do(steps):
        res = orig(steps)
        j = p.st.get("journal", "0") != "0"
        if j and not was_j[0]: S["journal"] += 1
        was_j[0] = j
        return res
    r.do = do
    def here():
        x, y, d = (int(v) for v in p.st["pos"].split(","))
        return (x, y), d
    def lost():
        return p.st["desc"].startswith("Party") and "|0 char" in p.st["desc"]
    def place():
        """A shop, temple or hall's menu is up: its words walked through once, then out"""
        w = ht.mw(p)
        if not (set(w) & set(PLACE)) or ht.litword(p) is None: return False
        key = " ".join(w)
        first = key not in S["places"]
        S["places"][key] = S["places"].get(key, 0) + 1
        if first:
            c = ht.Crawl(r, budget=24); c.any_top = True; c.stack = [w]
            r.note(f"  a place after {S['steps']} steps: {w}")
            c.go(["place"], 1)
            if c.flags: S["stuck"] += c.flags
        for i in range(6):
            if r.exploring() or p.st["fighting"] == "1": break
            if "Exit" in ht.mw(p) and pt.goto_word(p, "Exit"): r.do("a,wait:400")
            elif "Leave" in ht.mw(p) and pt.goto_word(p, "Leave"): r.do("a,wait:400")
            else: r.do("b,wait:400")
        return True
    def hall():
        """A training hall: the party menu's own kind of screen (a list). Train tried once, then its Exit line"""
        first = "hall" not in S["places"]
        S["places"]["hall"] = S["places"].get("hall", 0) + 1
        if first:
            lines = [re.sub(r"[?]+", " ", t).strip() for t in p.text[:23] if re.search("[A-Za-z]{4}", t)]
            res = "no Train line"
            if pt.goto_line(p, "Train"):
                r.do("a,wait:900"); res = f"[{p.text[24].strip()[:40]}] {p.st['desc'][:30]}"
                for i in range(6):
                    if p.find("Train") and p.st["desc"].startswith("Party") and p.st["input"] == "0": break
                    r.do("b,wait:500")
            r.note(f"  a training hall after {S['steps']} steps ({len(lines)} lines); Train + A ->", res)
        # (a hall is the game's whole party menu: left by BEGIN Adventuring - its "Exit to DOS" leaves the game)
        if pt.goto_line(p, "Begin"): r.do("a,wait:900")
        else: r.do("b,wait:500")
    def settle():
        for i in range(45):
            if lost(): return False
            if p.st["desc"].startswith("Party menu") and p.st["fighting"] != "1" and p.st["input"] == "0" and not ht.litword(p):
                hall(); continue
            if p.st["fighting"] == "1":
                S["fights"] += 1
                if not quick_fight(r): return False
                mend[0] = True
                continue
            if r.exploring(): return True
            if "icon ok" in r.bottom() and pt.goto_word(p, "Yes"): r.do("a,wait:500"); continue
            if place(): continue
            w = " ".join(ht.mw(p))
            if w and ht.litword(p): S["menus"].add(w)
            r.step_on()
        return r.exploring()
    def press(k):
        r.do(k + ",wait:120"); S["steps"] += 1
    def face(dd):
        for i in range(4):
            d = here()[1]
            if d == dd or not r.exploring(): return d == dd
            press("left" if (dd - d) % 8 == 6 else "right")
        return here()[1] == dd
    maps = {}; next_note = 1000; idle = 0
    while S["steps"] < steps_max and time.time() < t_end:
        if lost(): r.note("  the party fell - the walk ends"); break
        if p.st.get("playing") != "1":
            S["stuck"] += 1; r.note(f"  !! after {S['steps']} steps the game was left (the port's screen: {p.st.get('screen')}) - last presses:", " / ".join(x.split('|')[0].strip() + '>' + x.split('|')[-1].strip()[:24] for x in r.trail[-5:])); break
        if not r.exploring():
            if not settle():
                if lost(): continue
                S["stuck"] += 1; pt.shot(r, f"stuck{S['stuck']}")
                r.note(f"  !! after {S['steps']} steps the walk can't get on: [{p.text[24].strip()[:50]}] | {p.st['desc']} | input {p.st['input']} pad {p.st['pad']}")
                pt.back_out(r)
                if not r.exploring():
                    if S["stuck"] >= 3: break
                    for k in ("a", "b", "a", "b"): r.do(k + ",wait:500")
                    if not settle(): break
            continue
        if mend[0]:
            # (after a fight: Encamp - Fix, so the stock party lasts longer)
            mend[0] = False
            if "Encamp" in ht.mw(p) and pt.goto_word(p, "Encamp"):
                r.do("a,wait:600")
                if "Fix" in ht.mw(p) and pt.goto_word(p, "Fix"): r.do("a,wait:900"); S["fixes"] = S.get("fixes", 0) + 1
                for i in range(8):
                    if r.exploring() or p.st["fighting"] == "1": break
                    if not ht.litword(p): r.do("a,wait:400")
                    elif "Exit" in ht.mw(p) and pt.goto_word(p, "Exit"): r.do("a,wait:400")
                    else: r.do("b,wait:400")
            continue
        if S["steps"] >= next_note:
            next_note += 1000
            r.note(f"  ... {S['steps']} steps: {len(S['squares'])} squares on {len(S['maps'])} maps, {S['fights']} fights, {S['journal']} journal mentions, {len(S['places'])} places")
        if p.st["walking"] != "1" and r.words() != ["Move", "Encamp"]:
            r.do("wait:300")              # (the menu still up while something begins - an encounter's picture)
            idle = idle + 1 if r.exploring() and p.st["walking"] != "1" else 0
            if idle > 20:
                S["stuck"] += 1; pt.shot(r, f"stuck{S['stuck']}"); idle = 0
                r.note(f"  !! after {S['steps']} steps: the exploring menu is up but the D-pad doesn't walk (walking 0) at {p.st['pos']} | {p.st['desc']}")
                if S["stuck"] >= 3: break
                pt.back_out(r); r.do("a,wait:500")
            continue
        idle = 0
        if r.words() == ["Move", "Encamp"]:
            if "Move" in r.words() and pt.goto_word(p, "Move"): r.do("a,wait:400")
            for i in range(14):
                if p.st["fighting"] == "1" or p.st["walking"] != "1": break
                press(rnd.choice(["up", "up", "left", "right", "down"])); S["squares"].add((p.st["desc"], p.st["pos"].rsplit(",", 1)[0]))
            if p.st["fighting"] != "1" and p.st["walking"] == "1" and not r.exploring(): r.do("b,wait:400")
            S["maps"].add(p.st["desc"].split("|")[-1])
            continue
        key = p.st["desc"].split("|")[-1]; S["maps"].add(key)
        m = maps.setdefault(key, {"tried": {}, "open": {}})
        cur, d = here(); S["squares"].add((key, cur))
        tried = m["tried"].setdefault(cur, set())
        untried = [x for x in (d, (d + 2) % 8, (d + 6) % 8, (d + 4) % 8) if x not in tried]
        if untried:
            dd = untried[0]
        else:
            prev = {cur: None}; queue = [cur]; goal = None
            while queue and goal is None:
                q = queue.pop(0)
                for x, to in m["open"].get(q, {}).items():
                    if to not in prev:
                        prev[to] = (q, x); queue.append(to)
                        if len(m["tried"].get(to, ())) < 4: goal = to; break
            if goal is None:
                # (this map is walked: its sides tried again later - a door may lead on now)
                m["tried"].clear()
                for i in range(25):
                    if not r.exploring(): break
                    press(rnd.choice(["up", "up", "up", "left", "right"]))
                continue
            while prev[goal][0] != cur: goal = prev[goal][0]
            dd = prev[goal][1]
        if not face(dd): continue
        press("up"); tried.add(dd)
        if not r.exploring(): continue
        now = here()[0]
        if p.st["desc"].split("|")[-1] == key and now != cur:
            m["open"].setdefault(cur, {})[dd] = now
            m["open"].setdefault(now, {})[(dd + 4) % 8] = cur
            m["tried"].setdefault(now, set()).add((dd + 4) % 8)
    r.do = orig
    kinds = {k: sum(1 for w in S["places"] if k in w.split()) for k in PLACE}
    kinds["hall"] = 1 if "hall" in S["places"] else 0
    r.note(f"  WALK: {S['steps']} steps, {len(S['squares'])} squares on {len(S['maps'])} maps | {S['fights']} fights | {S['journal']} journal mentions opened |",
           f"places walked through: {len(S['places'])} {kinds} | other menus met: {len(S['menus'])} | could not get on: {S['stuck']}")

def main():
    exe, games, n, out = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
    steps = int(sys.argv[5]) if len(sys.argv) > 5 else 6000
    minutes = float(sys.argv[6]) if len(sys.argv) > 6 else 22
    pt.EXE, pt.OUT = exe, out
    os.makedirs(out, exist_ok=True)
    r = pt.Run(games, n, "x")
    p = r.p
    try:
        k = pt.to_party_menu(p, 70)
        r.name = pt.games_of(p)[n - 1]; r.note(f"== x{n} {r.name}")
        if k >= 0:
            ok = pt.create(r, "ZEDTEST")
            r.note("  Create New Character to its end by buttons:", "OK" if ok else "!! FAILED -", p.text[24].strip()[:40], "|", p.st["desc"])
            if not pt.pm(p):
                pt.back_out(r)
                for i in range(8):
                    if pt.pm(p): break
                    r.do("b,wait:400")
        if pt.start(r):
            icon_screen(r)
            explore(r, steps, minutes)
    except Exception as e:
        import traceback
        r.note("  !! THE RUN STOPPED:", repr(e)[:160], "|", " / ".join(x.strip() for x in traceback.format_exc().strip().splitlines()[-4:])[:300])
    finally:
        try: p.quit()
        except Exception: pass
    print(open(f"{out}/{n}.notes").read())

if __name__ == "__main__":
    main()
