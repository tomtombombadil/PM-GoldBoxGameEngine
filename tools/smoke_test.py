#!/usr/bin/env python3
"""Runs the port without a screen (SDL's dummy drivers) and plays back
button presses: the game list with no games, the menu, a folder that isn't
a game, a game folder whose files are missing. No game files are needed
(or allowed in the repo); playing a real game is checked by hand with the
player's own files.

    python3 tools/smoke_test.py build/goldbox
"""
import os
import pathlib
import subprocess
import sys
import tempfile


def run(exe, games, conf, script):
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
    r = subprocess.run([exe, "--games", str(games), "--conf", str(conf), "--script", script],
                       env=env, capture_output=True, text=True, timeout=60)
    print(r.stdout, r.stderr, sep="")
    if r.returncode != 0:
        sys.exit(f"FAILED: exit code {r.returncode}")
    if "runtime error" in r.stderr or "AddressSanitizer" in r.stderr:
        sys.exit("FAILED: the sanitizers found something")
    return r.stdout


def main():
    exe = str(pathlib.Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory() as t:
        t = pathlib.Path(t)
        conf = t / "conf"
        # 1. No games folder at all; the menu: every item changed once
        shot = t / "a.bmp"
        out = run(exe, t / "nothing", conf,
                  f"wait:100,a,start,down,right,left,down,right,down,right,down,right,down,a,down,a,a,up,up,up,up,up,a,b,b,"
                  f"select,shot:{shot},quit")
        assert "0 game folder(s) found" in out, "expected no games"
        assert shot.stat().st_size > 320 * 200 * 3, "no screenshot"
        assert (conf / "settings.cfg").is_file(), "settings not saved"
        # 2. Folders: lower-case names, files a folder down, an unknown one,
        # the engine's own folder
        g = t / "games"
        for d, f in (("curse of the azure bonds", "geo1.dax"), ("Pool of Radiance/game", "GEO1.DAX"),
                     ("Mystery", "A.DAX"), ("_CYD", "X.DAX"), ("Empty", "readme.txt")):
            (g / d).mkdir(parents=True)
            (g / d / f).write_bytes(b"not a game file")
        out = run(exe, g, conf, "wait:100,a,wait:200,a,down,a,wait:200,a,down,a,a,up,quit")
        assert "3 game folder(s) found" in out, "expected 3 folders"
        assert "Pool of Radiance (1 files in games/Pool of Radiance/game)" in out
        assert "curse of the azure bonds: Curse of the Azure Bonds" in out
        assert "Mystery: Unknown" in out and "not playable" in out
        assert "playing" not in out, "a game without its files must not start"
    print("smoke test passed")


if __name__ == "__main__":
    main()
