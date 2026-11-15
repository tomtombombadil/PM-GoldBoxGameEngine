#!/usr/bin/env python3
"""Rough handling: random button presses on a game, a window resize now and then.
   monkey_test.py <binary (a sanitizer build)> <COPY of the games dir> <game n> <presses> [seed]
Looks for a crash (the process gone other than by the menu's own Quit / Exit Game), a sanitizer
report, or a port that stops answering. Needs real game files; scratch only. The helper scripts'
addresses are pointed nowhere, so a random Send Logs / Check for Updates reaches nobody."""
import sys, os, random, subprocess, time, threading, queue

BUTTONS = ["up"] * 6 + ["down"] * 4 + ["left"] * 4 + ["right"] * 4 + ["a"] * 9 + ["b"] * 4 + \
          ["x", "y", "l1", "r1", "l2", "r2", "start", "select"]
SIZES = ["640x480", "1280x720", "720x720", "480x320", "854x480", "1024x768"]

def main():
    exe, games, n, presses = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
    seed = int(sys.argv[5]) if len(sys.argv) > 5 else n
    rnd = random.Random(seed)
    conf = f"{games}/../monkeyconf-{n}"
    os.makedirs(conf, exist_ok=True)
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy",
               GOLDBOX_RELAY_URL="file:///nowhere", GOLDBOX_RELEASES_URL="file:///nowhere",
               ASAN_OPTIONS="detect_leaks=0:abort_on_error=0", UBSAN_OPTIONS="print_stacktrace=1")
    done, starts, sizes, problems, tail = 0, 0, 0, [], []
    while done < presses and len(problems) < 3:
        open(conf + "/settings.cfg", "w").write("font=1\nvolume=0\nreply_asked=1\n")
        p = subprocess.Popen([exe, "--games", games, "--conf", conf, "--game", str(n), "--drive", "--window", "640x480"],
                             stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env, text=True, bufsize=1)
        starts += 1
        alive = True
        q = queue.Queue()                 # (the port's lines, read on a thread: a wait with a limit)
        threading.Thread(target=lambda out=p.stdout: [q.put(x) for x in iter(out.readline, "")] + [q.put("")], daemon=True).start()
        while alive and done < presses:
            steps = []
            for i in range(12):
                steps.append(rnd.choice(BUTTONS)); steps.append("wait:%d" % rnd.choice((30, 30, 60, 150)))
            if rnd.random() < 0.04:
                steps.append("resize:" + rnd.choice(SIZES)); sizes += 1
            line = ",".join(steps)
            try:
                p.stdin.write(line + "\n"); p.stdin.flush()
            except Exception:
                alive = False; break
            t = time.time()
            while True:
                try: s = q.get(timeout=120)
                except queue.Empty:
                    problems.append(f"NO ANSWER in 120 s after {done} presses: {line}"); p.kill(); alive = False; break
                if not s: alive = False; break
                tail.append(s.rstrip()); tail[:] = tail[-60:]
                if "runtime error" in s or "AddressSanitizer" in s: problems.append(s.strip()[:300])
                if s.startswith("[end]"): break
            done += 12
        rc = p.wait() if not alive else (p.kill() or 0)
        if not alive and rc not in (0, None) and not any("NO ANSWER" in x for x in problems[-1:]):
            problems.append(f"THE PORT ENDED with code {rc} after {done} presses; its last lines: " + " / ".join(x for x in tail[-8:] if not x.startswith(("T|", "L|")))[:600])
    print(f"game {n} seed {seed}: {done} presses, {starts} starts (the menu's own Quit / Exit Game end one), {sizes} resizes:",
          "no crash, no sanitizer report" if not problems else "PROBLEMS:")
    for x in problems: print("   ", x)
    sys.exit(1 if problems else 0)

main()
