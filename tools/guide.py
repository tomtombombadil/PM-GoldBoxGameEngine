#!/usr/bin/env python3
"""The cluebook a play test walks by (Tom, 2026-11-16: "You already KNOW where the encounters / fights /
inns / shops / trainers are. Use that information. Don't wander randomly").

The book is the CYD sweep's `events` run with EVMAP=1 on the player's game files (cyd/tools/playsim/
sweep.cpp; built by cyd/tools/devkit/build.sh): for every script block its map (each square's four
walls) and what each square's script shows - text, a menu's words, a fight, a shop's screen. It is made
from game files: it stays in scratch, never in a repo.

   load(path) -> {(area, block): Block}      Block.w, .h, .walls[y][x] = [N, E, S, W], .ev[(x, y, dir)] = [(kind, text)]
                                             (x, y, dir: the square and facing a step forward was taken FROM)
   Block.find(kind)                          those (x, y, dir) of a kind: shop temple training inn fight
   Block.path((x, y, dir), goal, shut)       the buttons there: left / right turn, up a step
"""
import re, heapq

KIND = {
    "shop":     lambda k, t: k == "SCREEN" and "Buy" in t,
    "temple":   lambda k, t: k == "SCREEN" and "Heal" in t,
    "training": lambda k, t: k == "HALL" or (k in ("SCREEN", "MENU") and "Train" in t) or (k == "TEXT" and "TRAIN" in t.upper()),
    "inn":      lambda k, t: k == "TEXT" and re.search(r"\bINN\b|ROOMS? FOR|STAY THE NIGHT", t.upper()) is not None,
    "fight":    lambda k, t: k == "FIGHT",
}
STEP = {0: (0, -1), 2: (1, 0), 4: (0, 1), 6: (-1, 0)}

class Block:
    def __init__(self): self.w = self.h = 0; self.walls = []; self.ev = {}
    def find(self, kind):
        f = KIND[kind]
        return [sq for sq, evs in self.ev.items() if any(f(k, t) for k, t in evs)]
    def says(self, sq):
        return " | ".join(f"{k} {t[:50]}" for k, t in self.ev.get(sq, [])[:4])
    def path(self, at, goal, shut=()):
        """Dijkstra over (x, y, dir): a turn 1, a step through an open side 1, through a wall type that
        may be a door 4; sides in `shut` ((x, y, dir) found closed) not at all. None if there is no way"""
        if not self.walls: return None
        x0, y0, d0 = at
        best = {(x0, y0, d0): 0}; q = [(0, x0, y0, d0, [])]
        while q:
            c, x, y, d, keys = heapq.heappop(q)
            if (x, y) == tuple(goal[:2]) and (len(goal) < 3 or d == goal[2]): return keys
            if best.get((x, y, d), 1e9) < c: continue
            for nd, k in (((d + 6) % 8, "left"), ((d + 2) % 8, "right")):
                if best.get((x, y, nd), 1e9) > c + 1:
                    best[(x, y, nd)] = c + 1; heapq.heappush(q, (c + 1, x, y, nd, keys + [k]))
            dx, dy = STEP[d]; nx, ny = x + dx, y + dy
            if 0 <= nx < self.w and 0 <= ny < self.h and (x, y, d) not in shut:
                wt = self.walls[y][x][d // 2]; nc = c + (1 if wt == 0 else 4)
                if best.get((nx, ny, d), 1e9) > nc:
                    best[(nx, ny, d)] = nc; heapq.heappush(q, (nc, nx, ny, d, keys + ["up"]))
        return None

def load(path):
    out = {}; b = None
    for line in open(path, errors="replace"):
        m = re.match(r"== ECL(\d+) block (\d+)", line)
        if m: b = out[(int(m.group(1)), int(m.group(2)))] = Block(); continue
        if b is None: continue
        m = re.match(r"  MAP (\d+) (\d+)", line)
        if m: b.w, b.h, b.walls = int(m.group(1)), int(m.group(2)), []; continue
        if line.startswith("  M "):
            b.walls.append([[int(q[i:i + 2], 16) for i in (0, 2, 4, 6)] for q in line.split()[1:]]); continue
        m = re.match(r"  @ (\d+),(\d+),(\d+) (\w+) \[(.*)\]\s*$", line)
        if m: b.ev.setdefault((int(m.group(1)), int(m.group(2)), int(m.group(3))), []).append((m.group(4), m.group(5)))
    return out

if __name__ == "__main__":
    import sys
    bk = load(sys.argv[1])
    for key in sorted(bk):
        b = bk[key]
        print(f"ECL{key[0]} block {key[1]}: map {b.w} x {b.h}, {len(b.ev)} squares with something |",
              " ".join(f"{k} {len(b.find(k))}" for k in KIND))
