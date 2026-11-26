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
        # (the list's own column: Treasures' / Darkness's Add list shows the lit character's sheet
        # beside it, its name at the top - found first, never lit, and the test paged for ever)
        at = next(((y, t.upper().find(name.upper())) for y, t in enumerate(p.text[:23])
                   if 0 <= t.upper().find(name.upper()) < 18), None)
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
                    r.do("a,wait:400"); poor = "enough" in p.text[24].lower()
                    for k in range(4):                     # (a message - "Not enough Money." - passes or wants a press)
                        if "Buy" in ht.mw(p) and ht.litword(p): break
                        poor = poor or "enough" in p.text[24].lower()
                        r.do("a,wait:400")
                    if poor: continue                      # (too dear: the next of wants)
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

# ---- The walk with the cluebook (tools/guide.py): straight to the shop, the temple, the hall, a fight --------
def where(p):
    m = re.match(r"Area (\d+), script (\d+)", p.st["desc"])
    try: x, y, d = (int(v) for v in p.st["pos"].split(","))
    except Exception: return None, None
    return ((int(m.group(1)), int(m.group(2))) if m else None), (x, y, d)

ASKS = {"temple": "HEAL", "shop": "SHOW YOU|WARES|SELECTION|FOR SALE", "training": "TRAIN"}
PLACE_WORDS = {"shop": "Buy", "temple": "Heal", "training": "Train", "inn": "Rest"}

def in_place(p, kind):
    w = PLACE_WORDS.get(kind)
    return bool(w) and (w in ht.mw(p) or (kind == "training" and p.find("Train") is not None and pt.pm(p)))

def go(r, book, kind, goal, S, max_keys=200):
    """To the book's square and facing (x, y, dir), by its walls - a side found closed is left out and
    the way planned anew - then the step forward the book took there, and its questions answered Yes.
    "in" at the place's own menu, "fight" after one, "nothing" when the step showed none, else why not"""
    p = r.p; key0, at = where(p); shut = set(); fought = False; stepped = False; up_from = None
    b = book.get(key0)
    if b is None: return "no map in the book"
    for n in range(max_keys):
        if p.st["fighting"] == "1":
            S["fights"] = S.get("fights", 0) + 1; fought = True
            if not ex.quick_fight(r): return "the fight was not got through"
            for i in range(30):
                if r.exploring(): break
                r.step_on()
            ex.healer(r, S)
            if kind == "fight": return "fight"
            continue
        if in_place(p, kind): return "in"
        if not r.exploring():
            if p.st.get("playing") != "1" or (pt.pm(p) and ex.members(p) == 0): return "the party fell"
            w = ht.mw(p)
            said = " ".join(p.text[12:24]).upper()
            # (the place met on the way counts too; in a town a question on the way to a shop, a temple or a
            # hall is taken as its door's unless it speaks of a fight)
            if "Yes" in w and (stepped or re.search(ASKS.get(kind, "$^"), said) or (kind in ASKS and not re.search("FIGHT|ATTACK|STEAL|FLEE", said))) \
               and pt.goto_word(p, "Yes"): r.do("a,wait:500")
            elif stepped and kind == "fight" and "Fight" in w and pt.goto_word(p, "Fight"): r.do("a,wait:500")
            elif stepped and kind == "fight" and "Attack" in w and pt.goto_word(p, "Attack"): r.do("a,wait:500")
            else: r.step_on()
            continue
        if stepped: return "fight" if fought and kind == "fight" else "nothing"
        key, at = where(p)
        if key != key0: return f"left the map (now {key})"
        if up_from and at[:2] == up_from[:2]: shut.add(up_from)      # (the step was turned back - a question answered No, a closed door)
        up_from = None
        if at == tuple(goal):
            r.do("up,wait:500"); stepped = True
            continue
        keys = b.path(at, goal, shut)
        if not keys: return "no way there"
        if keys[0] == "up": up_from = at
        r.do(keys[0] + ",wait:250")
    return "too far"

def visit(r, book, kind, S, tries=4):
    """The nearest squares of a kind tried in turn; at the place its own screen used. A line for the notes"""
    p = r.p; key, at = where(p); b = book.get(key)
    if b is None: return f"{kind}: this map ({key}) is not in the book"
    sq = [(len(b.path(at, g) or []) or 999, g) for g in b.find(kind)]
    sq = [g for n, g in sorted(sq) if n < 999]
    if not sq: return f"{kind}: none on this map by the book"
    for g in sq[:tries]:
        how = go(r, book, kind, g, S)
        if how == "in":
            did = ""
            if kind == "shop":
                if pt.goto_word(p, "Buy"):
                    r.do("a,wait:600"); did = f"bought {shop_buy(r, ('Dagger', 'Dart', 'Club', 'Quarter Staff', 'Sling', 'Hand Axe', 'Arrow', '10 Arrows', 'Flask of Oil'))}"
                    if "None" in did: did = "the list opened: " + " / ".join(" ".join(t.split()) for t in rows(p)[2:8] if t.strip())[:90]
            elif kind == "temple":
                if pt.goto_word(p, "Heal"): r.do("a,wait:500"); did = "Heal's list: " + " / ".join(t.strip() for t in rows(p)[2:12] if t.strip())[:90]
            elif kind == "training":
                if pt.goto_line(p, "Train"):
                    r.do("a,wait:700"); did = f"Train says [{r.bottom()[:40]}]"
                    if "Select" in ht.mw(p) and pt.goto_word(p, "Select"):       # ("Pick Character")
                        r.do("a,wait:700"); did += f" then [{r.bottom()[:40]}]"
            for k in range(24):
                if r.exploring() or p.st["fighting"] == "1": break
                w = ht.mw(p)
                if pt.pm(p) and (ex.chosen(p) or not ht.litword(p)) and pt.goto_line(p, "Begin"): r.do("a,wait:900")   # (a hall is left by Begin)
                elif "Exit" in w and pt.goto_word(p, "Exit"): r.do("a,wait:400")
                elif "Leave" in w and pt.goto_word(p, "Leave"): r.do("a,wait:400")
                elif "Press" in r.bottom() or not w: r.do("a,wait:400")
                else: r.do("b,wait:400")
            return f"{kind}: WALKED IN at {g} ({b.says(g)[:60]}) - {did} - back to exploring: {'OK' if r.exploring() else '!! ' + r.bottom()[:40]}"
        if how == "fight": return f"{kind}: fought at {g} and came through - {'exploring' if r.exploring() else '!! ' + r.bottom()[:40]}"
        if how in ("the party fell", "the fight was not got through") or how.startswith("left"): return f"{kind}: {how} (going to {g})"
        S.setdefault("miss", []).append(f"{kind} {g}: {how}")
    return f"{kind}: not got into - tried {sq[:tries]}: {S.get('miss', [])[-tries:]}"

def guided(r, book_path):
    import guide
    book = guide.load(book_path); S = {}
    p = r.p; key, at = where(p)
    r.note(f"  the cluebook walk - at {at} on map {key}; the book has it: {key in book}")
    for kind in ("shop", "temple", "training", "fight", "fight"):
        for k in range(12):
            if r.exploring() or p.st.get("playing") != "1": break
            if p.st["fighting"] == "1": ex.quick_fight(r)
            else: r.step_on()
        if p.st.get("playing") != "1" or not r.exploring(): r.note("    (the walk ends:", r.bottom()[:40], "|", p.st["desc"], ")"); break
        r.note("   ", visit(r, book, kind, S), "| now at", where(p)[1])
    return S

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
        book = os.environ.get("BOOK")
        if ok and book: guided(r, book)
        elif ok: ex.explore(r, steps, minutes)
    except Exception as e:
        import traceback
        r.note("  !! THE RUN STOPPED:", repr(e)[:160], "|", " / ".join(x.strip() for x in traceback.format_exc().strip().splitlines()[-4:])[:300])
    finally:
        try: p.quit()
        except Exception: pass
    print(open(f"{out}/{n}.notes").read())

if __name__ == "__main__":
    main()
