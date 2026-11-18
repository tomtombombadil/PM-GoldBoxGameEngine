#!/usr/bin/env python3
"""A switch's EFFECT in a game, through the port's buttons (Tom, 2026-11-18: a menu line that
flips isn't a tested switch). Needs real game files: run by hand on a COPY of a scanned games
folder (scratch only).

    tools/switch_test.py <binary> <games copy> <game n> <out dir> coins
    tools/switch_test.py <binary> <games copy> <game n> <out dir> wiring

wiring (any game): in a loaded game, every line of Game Play's pages is flipped with the buttons
and the ENGINE is asked what it has on (the [engine] line of --drive: rules::fix / adnd / enh -
the very gates the game's code asks). Each line must switch exactly its own engine switch off and
on again; the lines shown must be the fixes the engine lists for this game (the table in
cyd/src/engine/rules.cpp, read here on its own); Original Games must switch all off and give the
set back; one switch left off must still be off after the port is quit and started again.
This proves the menu reaches the running game - NOT what the switch then does (the engine's own
checks, cyd/tools/devkit/toggles.sh, and the effect cases here).

coins (Curse, its stock save): the Merchant cheat - Pool, Take 100 platinum, Buy the first item
(3000 gold), once with Coin Exchange on and once off (settings.cfg comforts_off 8192).
  On : the buyer's own coins go first, the counter pays the rest - buyer 0, counter 1200.
  Off: the originals, one purse or the other - buyer keeps 100, counter 1100.
"""
import os, re, shutil, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt

def platinum(lines):
    for t in lines:
        m = re.search(r"Platinum\s+(\d+)", t)
        if m: return int(m.group(1))
    return 0

def leave(r):
    for _ in range(4):
        if r.bottom().startswith("Buy View"): return
        r.do("b,wait:500")

def coins(src, gi, out, off):
    d = f"{out}/coins_{off}"
    shutil.rmtree(d, ignore_errors=True)
    shutil.copytree(src, d + "/games")
    pt.OUT = d
    pt.CFG_EXTRA = f"menus=2\ncomforts_off={off}\n"
    r = pt.Run(d + "/games", gi, "c"); p = r.p
    pt.start(r)
    r.do("start,wait:300,menu:Cheats,a,wait:400"); r.do("menu:Merchant,a,wait:1200")
    pt.goto_word(p, "Pool"); r.do("a,wait:800")
    pt.goto_word(p, "Take"); r.do("a,wait:800"); r.do("a,wait:800")
    r.do("type:100,wait:300"); r.do("y,wait:800")
    leave(r)
    pt.goto_word(p, "Buy"); r.do("a,wait:800"); r.do("a,wait:1200")
    leave(r)
    pt.goto_word(p, "View"); r.do("a,wait:800"); buyer = platinum(p.text); r.do("b,wait:500")
    pt.goto_word(p, "Take"); r.do("a,wait:800"); counter = platinum(p.text)
    p.quit()
    return buyer, counter

def engine_tables():
    """name -> bit, and the games' masks, from the engine's source (not from the port)."""
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "cyd", "src", "engine")
    src = open(os.path.join(root, "rules.cpp")).read()
    hdr = open(os.path.join(root, "rules.h")).read()
    order = re.findall(r"^\s*(kFix[A-Za-z0-9]+),", hdr[hdr.index("enum Fix"):hdr.index("kFixCount")], re.M)
    fixes = {}
    for m in re.finditer(r'\{(kFix[A-Za-z0-9]+), "([^"]+)", ([^,]+),', src):
        fixes[m.group(2)] = (order.index(m.group(1)), m.group(3).strip())
    bits = dict(re.findall(r"(kAdnd[A-Za-z]+)\s*=\s*(0x[0-9A-Fa-f]+|\d+),", hdr))
    rules = {m.group(2): int(bits[m.group(1)], 0) for m in re.finditer(r'\{(kAdnd[A-Za-z]+), "([^"]+)",', src)}
    return fixes, rules

GAME_MASK = {"Pool of Radiance": "kPool", "PoolRad": "kPool", "Darkness": "kDark", "Knights": "kKnights", "Queen": "kQueen", "Curse": "kCurse", "Secret": "kSecret", "Pools of Darkness": "kDark",
             "Champions": "kChamp", "Death Knights": "kKnights", "Dark Queen": "kQueen", "Gateway": "kGate", "Treasures": "kTreas"}

def wiring(src, gi, out):
    fixes, rules = engine_tables()
    d = f"{out}/wiring_{gi}"
    shutil.rmtree(d, ignore_errors=True)
    shutil.copytree(src, d + "/games")
    pt.OUT = d; pt.CFG_EXTRA = ""
    r = pt.Run(d + "/games", gi, "w"); p = r.p
    pt.start(r)
    bad = [0]; n = [0]
    def fail(msg): bad[0] += 1; print("  FAILED:", msg)
    def lines(): return [m[1:] for m in p.menu]
    def flip(label, key, bit):
        r.do("menu:" + label.replace(" ", "_") + ",wait:100"); a = dict(p.eng)
        r.do("a,wait:150"); b = dict(p.eng)
        r.do("a,wait:150"); c = dict(p.eng)
        n[0] += 1
        want = dict(a); want[key] = a[key] & ~bit
        if not (a[key] & bit): fail(f"{label}: the engine had it off before the press")
        elif b != want: fail(f"{label}: off gave fix={b['fix']:x} adnd={b['adnd']:x} enh={b['enh']:x}, wanted {key} without {bit:x} alone")
        elif c != a: fail(f"{label}: on again didn't give the engine's set back")
    r.do("start,wait:300"); r.do("menu:Game_Play,a,wait:300")
    # Enhancements the engine applies itself
    r.do("menu:Enhancements,a,wait:300")
    for label, bit in (("Coin Exchange", 1), ("Flasks of Oil", 2)): flip(label, "enh", bit)
    r.do("b,wait:300")
    # the AD&D 1E Rules
    r.do("menu:AD&D,a,wait:300")
    shown = [l.split(":")[0] for l in lines() if ":" in l]
    if sorted(shown) != sorted(rules): fail(f"rules shown {shown}, the engine's {sorted(rules)}")
    for label in shown:
        if label in rules: flip(label, "adnd", rules[label])
    r.do("b,wait:300")
    # the Game Fixes: Every Game + this game's page
    r.do("menu:Game_Fixes,a,wait:300")
    pages = [l.rstrip(".") for l in lines() if l.endswith("...")]
    seen = []
    mask = None
    for pg in pages:
        if pg != "Every Game":
            mask = next((v for k, v in GAME_MASK.items() if k.lower() in pg.lower() or pg.lower() in k.lower()), None)
            if mask is None: fail(f"page '{pg}': which game?")
        r.do("menu:" + pg.replace(" ", "_") + ",a,wait:300")
        for label in [l.split(":")[0] for l in lines() if ":" in l]:
            seen.append(label)
            if label not in fixes: fail(f"'{label}' isn't in the engine's table")
            else: flip(label, "fix", 1 << fixes[label][0])
        r.do("b,wait:300")
    if mask:
        want = sorted(k for k, (_, games) in fixes.items() if games == "kAll" or mask in [g.strip() for g in games.split("|")])
        if sorted(seen) != want: fail(f"fixes shown and the engine's list for this game differ: only shown {sorted(set(seen) - set(want))}, missing {sorted(set(want) - set(seen))}")
    r.do("b,wait:300")
    # Original Games: all off, then the set back
    a = dict(p.eng)
    r.do("menu:Original,a,wait:200"); b = dict(p.eng)
    r.do("a,wait:200"); c = dict(p.eng)
    if (b["fix"], b["adnd"], b["enh"]) != (0, 0, 0): fail(f"Original Games on: the engine still has fix={b['fix']:x} adnd={b['adnd']:x} enh={b['enh']:x}")
    if c != a: fail("Original Games off: the set didn't come back")
    # one switch left off: still off while playing, and after quitting and starting again
    r.do("menu:AD&D,a,wait:300"); first = [l.split(":")[0] for l in lines() if ":" in l][0]
    r.do("menu:" + first.replace(" ", "_") + ",a,wait:150")
    while p.st["menu"] == "1": r.do("b,wait:300")
    r.do("right,wait:400,left,wait:400")
    if p.eng["adnd"] & rules[first]: fail(f"{first} left off: on again once the menu closed")
    p.quit()
    r2 = pt.Run(d + "/games", gi, "w2", keep_conf=True) if "keep_conf" in pt.Run.__init__.__code__.co_varnames else None
    if r2:
        pt.start(r2)
        if r2.p.eng["adnd"] & rules[first]: fail(f"{first} left off: on again after the port was started anew")
        r2.p.quit()
    shutil.rmtree(d + "/games", ignore_errors=True)
    print(f"game {gi}: {n[0]} lines flipped in a loaded game ({len(seen)} fixes, {len(shown)} rules, 2 enhancements), Original Games, kept-off - "
          + ("OK" if not bad[0] else f"{bad[0]} FAILED"))
    return bad[0]

if __name__ == "__main__":
    pt.EXE, src, gi, out, what = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4], sys.argv[5]
    bad = 0
    if what == "coins":
        for off, want in ((0, (0, 1200)), (8192, (100, 1100))):
            got = coins(src, gi, out, off)
            ok = got == want
            bad += not ok
            print(f"Coin Exchange {'on ' if off == 0 else 'off'}: buyer {got[0]} platinum, counter {got[1]} - "
                  f"{'OK' if ok else 'FAILED, wanted %d / %d' % want}")
    if what == "wiring": bad += wiring(src, gi, out)
    sys.exit(1 if bad else 0)
