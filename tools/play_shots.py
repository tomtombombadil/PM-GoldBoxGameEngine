#!/usr/bin/env python3
"""Plays a --script of button presses through the port without a screen and
puts a screenshot after every step on one contact sheet (Claude's side:
needs Pillow and the player's game files in scratch space).

    python3 tools/play_shots.py <binary> <games dir> <out.png> [--game N] step step ...

A step is a --script step (a, down, hold:up, wait:500, type:BOB ...); a
trailing '!' (a!) means "no screenshot after this one". The sheet shows
real game screens: it stays in scratch, never in the repo.
"""
import os
import pathlib
import subprocess
import sys
import tempfile

from PIL import Image, ImageDraw


def main():
    exe, games, out = sys.argv[1:4]
    args = sys.argv[4:]
    game = "1"
    if args and args[0] == "--game":
        game, args = args[1], args[2:]
    wait = os.environ.get("STEP_WAIT", "500")
    with tempfile.TemporaryDirectory() as t:
        steps, shots = [], []
        for i, s in enumerate(args):
            quiet = s.endswith("!")
            s = s.rstrip("!")
            steps.append(s)
            if not quiet:
                steps += [f"wait:{wait}", f"shot:{t}/{i:03d}.bmp"]
                shots.append((f"{t}/{i:03d}.bmp", s))
        steps.append("quit")
        env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
        conf = os.environ.get("CONF", t + "/conf")
        r = subprocess.run([exe, "--games", games, "--conf", conf, "--game", game, "--script", ",".join(steps)],
                           env=env, capture_output=True, text=True, timeout=900)
        log = [l for l in (r.stdout + r.stderr).splitlines() if not l.startswith("[script] ")]
        print("\n".join(log[-int(os.environ.get("LOG", "12")):]))
        print("exit code", r.returncode)
        cols = int(os.environ.get("COLS", "4"))
        w, h = 480, 300
        rows = (len(shots) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * w, rows * (h + 14)), (40, 40, 40))
        d = ImageDraw.Draw(sheet)
        for i, (f, label) in enumerate(shots):
            if not pathlib.Path(f).exists():
                continue
            x, y = (i % cols) * w, (i // cols) * (h + 14)
            sheet.paste(Image.open(f).resize((w, h), Image.NEAREST), (x, y + 14))
            d.text((x + 3, y + 1), f"{i + 1}: {label}", fill=(255, 255, 0))
        sheet.save(out)


if __name__ == "__main__":
    main()
