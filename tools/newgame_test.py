#!/usr/bin/env python3
"""A NEW game as a player starts one, by buttons: six characters made (Human Fighters where the game
offers them), Modify to the highest figures, Begin Adventuring, armour and weapons bought and readied,
then the maps walked with the party kept alive by the Cheats (Healer after fights, free training).
   newgame_test.py <binary> <COPY of the games dir> <game n> <out dir> [steps] [minutes]
Through play_test.py's driver (needs real game files; scratch only). "!!" marks what needs a look."""
import sys, os, re, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt
import hand_test as ht
import explore_test as ex

def list_pick(p, r, *wants):
    """A list is up (its menu line only Exit): the line holding one of wants lit, if there is one"""
    for w in wants:
        for i in range(1, 23):
            t = re.sub(r"[?]+", " ", p.text[i]).strip()
            if t == w: return pt.goto_line(p, w)
    return False

def create(r, name):
    """Create New Character by buttons - Human, Fighter where a list offers them, else its first line"""
    p = r.p
    if not (pt.goto_line(p, "Create New") or pt.goto_line(p, "Create Character")): return False
    r.do("a,wait:600")
    for i in range(70):
        if pt.pm(p) and i > 3: return True
        b = r.bottom(); w = r.words()
        if p.st["input"] != "0": r.do(f"type:{name},wait:200,y,wait:600")
        elif any("PICK RACE" in t for t in p.text): r.do("r1,wait:200"); r.do("a,wait:700")
        elif "reroll" in b.lower() and pt.goto_word(p, "No"): r.do("a,wait:600")
        elif "Keep" in w and pt.goto_word(p, "Keep"): r.do("a,wait:600")
        elif "Done" in w and pt.goto_word(p, "Done"): r.do("a,wait:600")
        elif "Yes" in w and pt.goto_word(p, "Yes"): r.do("a,wait:600")
        else:
            list_pick(p, r, "Human", "Fighter")
            r.do("a,wait:600")
    return pt.pm(p)

def add(r, name):
    """Add Character to Party: the made character found on the list's pages (Next) and taken. "name" when
    it was that one, "other" when the list had no such line and its first was taken, None when nobody came"""
    p = r.p; before = ex.members(p); how = None
    if name[:5] in "".join(p.text[:12]): return "name"
    if not pt.goto_line(p, "Add Character"): return None
    r.do("a,wait:600")
    for i in range(40):
        if ex.members(p) > before: break
        at = p.find(name)
        if at and at[0] < 23:
            if p.lit_at(*at, len(name)): how = "name"; r.do("a,wait:600")
            else: r.do("r2,wait:120")
        elif "Next" in ht.mw(p) and i < 30 and pt.goto_word(p, "Next"): r.do("a,wait:400")
        else: how = how or "other"; r.do("a,wait:600")
    ok = ex.members(p) > before
    for i in range(6):
        if pt.pm(p) and not ht.litword(p): break
        r.do("b,wait:500")
    return how if ok else None

def to_exploring(r, tries=400):
    """From Begin Adventuring through the opening - story, pictures, a first fight - to the exploring menu"""
    p = r.p
    for i in range(tries):
        if p.st["fighting"] == "1":
            if not ex.quick_fight(r): return False
            continue
        if r.exploring(): return True
        if p.st.get("playing") != "1": return False
        r.step_on()
    return r.exploring()

def modify_all(r):
    """Modify Character for every member: each figure to its highest (Right until it stands), Keep"""
    p = r.p; n = ex.members(p); done = 0
    for i in range(n):
        if not pt.goto_line(p, "Modify"): break
        r.do("a,wait:500")
        if "Keep" not in ht.mw(p): r.do("a,wait:500")       # ("Pick Character")
        if "Keep" in ht.mw(p):
            add = "Add" in ht.mw(p)
            for item in range(12):
                if not add and ht.litword(p): break
                same = 0
                for k in range(110):
                    a = "".join(p.text[:23])
                    if add and not pt.goto_word(p, "Add"): break
                    r.do("a,wait:120" if add else "right,wait:50")
                    same = same + 1 if "".join(p.text[:23]) == a else 0
                    if same >= (2 if add else 1): break
                r.do("down,wait:120")
            if pt.goto_word(p, "Keep"): r.do("a,wait:500"); done += 1
        for k in range(8):
            if ex.chosen(p) or pt.pm(p) and not ht.litword(p): break
            r.do("b,wait:400")
        r.do("r1,wait:200")
    return done, n

# ---- Armour and weapons (the Merchant cheat's shop - the game's own shop screens) ---------------------------
OUTFIT = (("Chain Mail", "Scale Mail", "Ring Mail", "Banded Mail", "Leather Armor"), ("Shield",),
          ("Long Sword", "Broad Sword", "Battle Axe", "Mace"))

def rows(p):
    return [re.sub(r"[?]+", " ", t).rstrip() for t in p.text[:23]]

def lit_row(p):
    for i in range(1, 23):
        if re.search("H{3}", p.lit[i]): return i
    return -1

def to_row(r, i, key="r2", tries=30):
    p = r.p
    for k in range(tries):
        if lit_row(p) == i: return True
        r.do(key + ",wait:80")
    return lit_row(p) == i

def shop_buy(r, wants):
    """The shop's list is up: the first of wants found on its pages bought. Its name, or None"""
    p = r.p
    for page in range(14):
        rw = rows(p)
        for w in wants:
            for i, t in enumerate(rw):
                m = re.match(r"^\s*(.+?)\s+(\d+)\s*$", t)
                if m and m.group(1).strip() == w and to_row(r, i) and pt.goto_word(p, "Buy"):
                    r.do("a,wait:400")
                    for k in range(4):                     # (a message - "Not enough Money" - passes or wants a press)
                        if "Buy" in ht.mw(p) and ht.litword(p): break
                        r.do("a,wait:400")
                    return w
        if "Next" not in ht.mw(p) or not pt.goto_word(p, "Next"): return None
        before = "".join(rw); r.do("a,wait:400")
        if "".join(rows(p)) == before: return None
    return None

def ready_all(r):
    """View - Items for the chosen member: every item not readied, readied. How many say Yes after"""
    p = r.p
    if not pt.goto_word(p, "View"): return -1
    r.do("a,wait:500")
    if not pt.goto_word(p, "Items"): pt.goto_word(p, "Exit"); r.do("a,wait:400"); return -1
    r.do("a,wait:500")
    for k in range(8):
        todo = [i for i, t in enumerate(rows(p)) if re.match(r"^\s*No\s+\S", t)]
        if not todo or "Ready" not in ht.mw(p): break
        if to_row(r, todo[0]) and pt.goto_word(p, "Ready"): r.do("a,wait:400")
        for j in range(3):
            if "Ready" in ht.mw(p) and ht.litword(p): break
            r.do("a,wait:400")
        if [i for i, t in enumerate(rows(p)) if re.match(r"^\s*No\s+\S", t)] == todo: break    # (it won't be readied)
    yes = sum(1 for t in rows(p) if re.match(r"^\s*Yes\s+\S", t))
    for k in range(6):
        if "Buy" in ht.mw(p) or "Pool" in ht.mw(p): break
        if "Exit" in ht.mw(p) and pt.goto_word(p, "Exit"): r.do("a,wait:400")
        else: r.do("b,wait:400")
    return yes

def outfit(r):
    """Everyone armoured and armed at the Merchant's: the coins pooled, then Buy and Ready a member"""
    p = r.p
    if not ex.cheat(r, "Merchant"): return "!! the Merchant did not open"
    if "Buy" not in ht.mw(p): pt.back_out(r); return f"!! no Buy word - {ht.mw(p)}"
    if pt.goto_word(p, "Pool"): r.do("a,wait:500")
    n = ex.members(p) or len(ex.party_hp(p)) or 6; got = []; ready = []
    for i in range(n):
        if not pt.goto_word(p, "Buy"): break
        r.do("a,wait:600"); mine = []
        for wants in OUTFIT:
            # (each buy from the list's first page: back there with Prev)
            for k in range(14):
                if "Prev" not in ht.mw(p) or not pt.goto_word(p, "Prev"): break
                r.do("a,wait:300")
            mine.append(shop_buy(r, wants))
        got.append(mine)
        for k in range(5):
            if "Pool" in ht.mw(p) or "View" in ht.mw(p): break
            if "Exit" in ht.mw(p) and pt.goto_word(p, "Exit"): r.do("a,wait:400")
            else: r.do("b,wait:400")
        ready.append(ready_all(r))
        r.do("r2,wait:200")
    party = " / ".join(" ".join(t.split()) for t in rows(p)[3:11] if re.search("[A-Z]{3}", t))
    if "Share" in ht.mw(p) and pt.goto_word(p, "Share"): r.do("a,wait:500")
    pt.back_out(r)
    return f"bought {got} | readied {ready} | the party now: {party}"

def main():
    exe, games, n, out = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
    steps = int(sys.argv[5]) if len(sys.argv) > 5 else 6000
    minutes = float(sys.argv[6]) if len(sys.argv) > 6 else 30
    pt.EXE, pt.OUT = exe, out
    pt.CFG_EXTRA = "max_hp=1\n"
    ex.STRONG = True
    os.makedirs(out, exist_ok=True)
    r = pt.Run(games, n, "n"); p = r.p
    try:
        k = pt.to_party_menu(p, 70)
        r.name = pt.games_of(p)[n - 1]; r.note(f"== n{n} {r.name} - a new game")
        made = 0; hows = []
        for i in range(6):
            nm = "ZED" + chr(65 + i)
            if not create(r, nm): r.note(f"  !! character {i + 1} not made - [{r.bottom()[:40]}] {p.st['desc']}"); break
            if ex.members(p) <= made:
                h = add(r, nm); hows.append(h)
                if not h: r.note(f"  !! character {i + 1} made but Add Character to Party brought nobody - {p.st['desc']}"); break
            made = ex.members(p)
        r.note(f"  six characters made by buttons: {made} in the party", f"(Add Character to Party: {hows})" if hows else "(each in the party as made)")
        r.note("  Modify Character, every figure to its highest:", modify_all(r))
        lines = [re.sub(r"[?]+", " ", t).rstrip() for t in p.text[2:11] if re.search("[A-Z]{3}", t)]
        r.note("  the party:", " / ".join(" ".join(x.split()) for x in lines))
        ok = pt.goto_line(p, "Begin"); r.do("a,wait:1500"); ok = to_exploring(r)
        r.note("  Begin Adventuring - a new game - to the exploring menu:", "OK" if ok else "!! " + r.bottom()[:50], "| walking", p.st["walking"], "|", p.st["desc"])
        if ok: r.note("  the Merchant cheat - armour, a shield and a weapon bought and readied for everyone:", outfit(r))
        if ok and len(sys.argv) > 7 and sys.argv[7] == "stop": return
        if ok: ex.explore(r, steps, minutes)
    except Exception as e:
        import traceback
        r.note("  !! THE RUN STOPPED:", repr(e)[:160], "|", " / ".join(x.strip() for x in traceback.format_exc().strip().splitlines()[-4:])[:300])
    finally:
        try: p.quit()
        except Exception: pass
    print(open(f"{out}/{n}.notes").read())

if __name__ == "__main__":
    main()
