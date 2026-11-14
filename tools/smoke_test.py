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
import shutil
import subprocess
import sys
import tempfile


def run(exe, games, conf, script, window=None):
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
    r = subprocess.run([exe, "--games", str(games), "--conf", str(conf), "--script", script]
                       + (["--window", window] if window else []),
                       env=env, capture_output=True, text=True, timeout=60)
    print(r.stdout, r.stderr, sep="")
    if r.returncode != 0:
        sys.exit(f"FAILED: exit code {r.returncode}")
    if "runtime error" in r.stderr or "AddressSanitizer" in r.stderr:
        sys.exit("FAILED: the sanitizers found something")
    return r.stdout


ROOT = pathlib.Path(__file__).resolve().parent.parent
# Settings' lines (each changed and changed back by the first test)
SETTINGS = ("Display", "Game_Sound", "Music", "Volume", "Game_Font", "D-Pad_Walks", "Confirm_Button", "Game_Fixes",
            "Show_Rolls", "Readable", "Camp_Fix", "Icon_Gallery", "Icon_Portrait", "More_Char", "Button_Stops",
            "Quick_Actions", "Cheats", "Max_Hit")
MENU_UPDATE = "start,menu:Check_for_Updates,"   # the Start menu's line


def update_test(exe):
    """The whole way a tester gets a build, against a pretend release (files
    and file:// addresses, a pretend PortMaster): the installer installs
    it; the port finds a newer one, downloads it and restarts; the launch
    script puts it in place and starts it."""
    import hashlib
    import platform
    import zipfile
    arch = platform.machine()
    with tempfile.TemporaryDirectory() as t:
        t = pathlib.Path(t)
        ports = t / "roms" / "ports"
        ports.mkdir(parents=True)
        pm = t / "share" / "PortMaster"
        pm.mkdir(parents=True)
        (pm / "control.txt").write_text(
            f'directory="{str(t / "roms").lstrip("/")}"\nCFW_NAME=test\nDEVICE_ARCH={arch}\nESUDO=""\nGPTOKEYB=true\n'
            "get_controls() { :; }\npm_platform_helper() { :; }\npm_finish() { :; }\npm_message() { :; }\n")

        def release(version, binary):
            name = f"goldbox_port-v{version}.zip"
            with zipfile.ZipFile(t / name, "w") as z:
                z.write(ROOT / "port" / "Gold Box Game Engine.sh", "Gold Box Game Engine.sh")
                z.writestr("goldbox/VERSION", version + "\n")
                z.writestr(f"goldbox/goldbox.{arch}", binary)
                for f in (ROOT / "port" / "goldbox" / "tools").glob("*.sh"):
                    z.write(f, f"goldbox/tools/{f.name}")
            (t / (name + ".sha256")).write_text(hashlib.sha256((t / name).read_bytes()).hexdigest() + "  " + name + "\n")
            return name

        def publish(*names):          # (as GitHub lists releases: newest first, a field a line)
            rels = [{"tag_name": "v" + n.split("-v")[1][:-4], "prerelease": True,
                     "assets": [{"browser_download_url": f"file://{t}/{n}"},
                                {"browser_download_url": f"file://{t}/{n}.sha256"}]} for n in names]
            import json
            (t / "releases.json").write_text(json.dumps(rels, indent=2))

        env = dict(os.environ, XDG_DATA_HOME=str(t / "share"), HOME=str(t), GOLDBOX_PAUSE="0",
                   GOLDBOX_RELEASES_URL=f"file://{t}/releases.json", SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
        # 1. The installer, alone in the ports folder, installs the newest release
        first = release("0.0.1", pathlib.Path(exe).read_bytes())
        publish(first)
        shutil.copy(ROOT / "port" / "GoldBox.Installer.sh", ports)
        r = subprocess.run(["/bin/bash", str(ports / "GoldBox.Installer.sh")], env=env, capture_output=True, text=True, timeout=120)
        print(r.stdout, r.stderr, sep="")
        assert r.returncode == 0, "the installer failed"
        game = ports / "goldbox"
        assert (ports / "Gold Box Game Engine.sh").is_file() and (game / "VERSION").read_text().strip() == "0.0.1"
        assert os.access(game / f"goldbox.{arch}", os.X_OK) and (game / "games").is_dir()
        assert sorted(f.name for f in ports.iterdir()) == ["Gold Box Game Engine.sh", "GoldBox.Installer.sh", "goldbox"], \
            "the installer left something else in the ports folder"
        # 2. A newer release appears (its "port" a script that says so): the
        # port finds it, downloads it, restarts; the launch script applies it
        (game / "conf").mkdir(exist_ok=True)
        (game / "conf" / "settings.cfg").write_text("volume=30\n")
        second = release("99.0.0", "#!/bin/bash\necho THE NEW VERSION RUNS\n")
        publish(second, first)
        env["GOLDBOX_ARGS"] = f"--script wait:100,{MENU_UPDATE}a,wait:100,a,wait:100,a,wait:3000,quit"
        r = subprocess.run(["/bin/bash", str(ports / "Gold Box Game Engine.sh")], env=env, capture_output=True, text=True, timeout=180)
        print(r.stdout, r.stderr, sep="")
        assert "Update Found" in r.stdout and "99.0.0" in r.stdout, "the check didn't find the release"
        assert "THE NEW VERSION RUNS" in r.stdout, "the update wasn't applied and started"
        assert (game / "VERSION").read_text().strip() == "99.0.0" and not (game / ".update").exists()
        assert "volume=30" in (game / "conf" / "settings.cfg").read_text(), "settings were lost"
        assert "now 99.0.0" in (game / "update.log").read_text()
        assert (game / "log.prev.txt").is_file(), "the run before's log wasn't kept"
        # 3. Nothing newer: says so
        publish(first)
        (game / f"goldbox.{arch}").write_bytes(pathlib.Path(exe).read_bytes())
        env["GOLDBOX_ARGS"] = f"--script wait:100,{MENU_UPDATE}a,wait:100,quit"
        r = subprocess.run(["/bin/bash", str(ports / "Gold Box Game Engine.sh")], env=env, capture_output=True, text=True, timeout=120)
        assert "This is the newest version" in r.stdout, r.stdout


def logs_test(exe):
    """Send Logs to a pretend log relay (as Google's: the POST answered with
    a redirect to the result): the reply address asked for the first time,
    typed or declined, remembered; the files and the device's details sent."""
    import http.server
    import threading
    import urllib.parse
    posts = []

    class Relay(http.server.BaseHTTPRequestHandler):
        def do_POST(self):
            body = self.rfile.read(int(self.headers.get("Content-Length", 0))).decode()
            form = dict(urllib.parse.parse_qsl(body, keep_blank_values=True))
            posts.append(form)
            self.send_response(302)
            self.send_header("Location", f"/done?n={sum(1 for k in form if k.startswith('file:'))}&t={int(bool(form.get('reply')))}")
            self.send_header("Content-Length", "0")
            self.end_headers()

        def do_GET(self):
            q = dict(urllib.parse.parse_qsl(urllib.parse.urlparse(self.path).query))
            out = f"OK {q.get('n')} files{' thanks' if q.get('t') == '1' else ''}".encode()
            self.send_response(200)
            self.send_header("Content-Length", str(len(out)))
            self.end_headers()
            self.wfile.write(out)

        def log_message(self, *a):
            pass

    srv = http.server.HTTPServer(("127.0.0.1", 0), Relay)
    threading.Thread(target=srv.serve_forever, daemon=True).start()

    def go(t, steps):
        env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy",
                   GOLDBOX_RELAY_URL=f"http://127.0.0.1:{srv.server_port}/exec")
        r = subprocess.run([exe, "--games", str(t / "games"), "--conf", str(t / "conf"), "--tools", str(t / "tools"),
                            "--script", steps], env=env, capture_output=True, text=True, timeout=120)
        print(r.stdout, r.stderr, sep="")
        assert r.returncode == 0
        return r.stdout

    def port(t):
        t = pathlib.Path(t)
        shutil.copytree(ROOT / "port" / "goldbox" / "tools", t / "tools")
        (t / "log.txt").write_text("a line of this run's log\n")
        (t / "log.prev.txt").write_text("the run before\n")
        (t / "device.txt").write_text("version=9.9.9\ndevice=TESTBOX (Test Maker Box)\nfirmware=TestOS 1.2\nportmaster=2026.01.01\nscreen=640x480\n")
        (t / "games" / "_CYD" / "Some Game").mkdir(parents=True)
        (t / "games" / "_CYD" / "Some Game" / "BOOK.TXT").write_text("a journal's text: never sent\n")
        (t / "games" / "_CYD" / "QUICK.TXT").write_text("engine text\n")
        return t

    # 1. The first time: the address asked for, typed (a bad one refused), sent with the logs; the second time not asked
    with tempfile.TemporaryDirectory() as t:
        t = port(t)
        out = go(t, "wait:100,start,menu:Send_Logs,a,wait:100,type:tester,y,wait:100,type:@example.com,y,wait:900,a,"
                    "start,menu:Send_Logs,a,wait:900,quit")
        assert len(posts) == 2, posts
        f = posts[0]
        assert f["app"] == "goldbox" and f["reply"] == "tester@example.com" and f["device"] == "TESTBOX (Test Maker Box)"
        assert f["firmware"] == "TestOS 1.2" and f["portmaster"] == "2026.01.01" and f["version"]
        assert f["file:log.txt"] == "a line of this run's log\n" and "file:log.prev.txt" in f and "file:device.txt" in f
        assert "file:engine_QUICK.TXT" in f and not any("BOOK" in k for k in f), sorted(f)
        assert posts[1]["reply"] == "tester@example.com"
        cfg = (t / "conf" / "settings.cfg").read_text()
        assert "reply_email=tester@example.com" in cfg and "reply_asked=1" in cfg, cfg
        assert "A note is on its way to tester@example.com" in out
        assert not (t / ".sendlogs").exists(), "the sender's scratch folder left behind"
    # 2. No Thanks (X): sent without an address, and not asked again; Settings' Reply Address sets one later
    posts.clear()
    with tempfile.TemporaryDirectory() as t:
        t = port(t)
        out = go(t, "wait:100,start,menu:Send_Logs,a,wait:100,x,wait:900,a,start,menu:Send_Logs,a,wait:900,a,"
                    "start,menu:Settings,a,menu:Reply_Address,a,wait:100,type:me@example.org,y,wait:100,quit")
        assert len(posts) == 2 and posts[0]["reply"] == "" and posts[1]["reply"] == "", posts
        assert "reply_email=me@example.org" in (t / "conf" / "settings.cfg").read_text()
        assert "The logs were sent to the developer" in out
    srv.shutdown()


def main():
    exe = str(pathlib.Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory() as t:
        t = pathlib.Path(t)
        conf = t / "conf"
        # 1. No games folder at all; the menu: every item changed once
        shot = t / "a.bmp"
        out = run(exe, t / "nothing", conf,
                  f"wait:100,a,start,menu:Settings,a,"
                  + "".join(f"menu:{m},right,left,a," for m in SETTINGS)
                  + f"menu:As_Original,a,menu:All_On,a,l2,r2,menu:Back,a,"
                  f"menu:Controls,a,b,b,"
                  f"select,shot:{shot},quit")
        assert "0 game folder(s) found" in out, "expected no games"
        assert shot.stat().st_size > 320 * 200 * 3, "no screenshot"
        assert (conf / "settings.cfg").is_file(), "settings not saved"
        cfg = (conf / "settings.cfg").read_text()
        assert "comforts_off=0" in cfg and "max_hp=0" in cfg and "swap_ab=1" in cfg, cfg
        (conf / "settings.cfg").unlink()
        # 2. Folders: lower-case names, files a folder down, an unknown one,
        # the engine's own folder
        g = t / "games"
        for d, f in (("curse of the azure bonds", "geo1.dax"), ("Pool of Radiance/game", "GEO1.DAX"),
                     ("Mystery", "A.DAX"), ("_CYD", "X.DAX"), ("Empty", "readme.txt")):
            (g / d).mkdir(parents=True)
            (g / d / f).write_bytes(b"not a game file")
        # (the first run finds the two playable ones: the scan's list, until Continue)
        out = run(exe, g, conf, f"wait:100,shot:{shot},a,wait:100,a,wait:200,a,down,a,wait:200,a,down,a,a,up,quit")
        assert out.count("[scan] Found ") == 2 and "[scan] Done." in out, "the first run's scan"
        assert (g / "_CYD" / "curse of the azure bonds" / "PMSCAN.TXT").is_file(), "the scan's mark"
        assert (g / "_CYD" / "curse of the azure bonds" / "ART.BIN").is_file(), "the scan's picture file"
        assert "3 game folder(s) found" in out, "expected 3 folders"
        assert "Pool of Radiance (1 files in games/Pool of Radiance/game)" in out
        assert "curse of the azure bonds: Curse of the Azure Bonds" in out
        assert "Mystery: Unknown" in out and "not playable" in out
        assert "playing" not in out, "a game without its files must not start"
        # (found already: no scan the second time; Rescan Game Files: all anew)
        out = run(exe, g, conf, "wait:100,quit")
        assert "[scan]" not in out, "a second scan of games already found"
        # (another Display shape: the list's pictures are made again when the menu closes - and only they)
        out = run(exe, g, conf, "wait:100,start,menu:Settings,a,menu:Display,right,b,b,wait:100,a,"
                                "start,menu:Settings,a,menu:Display,left,b,b,wait:100,a,quit")
        assert out.count("[scan] Preparing the ") == 4 and "[scan] Found" not in out, "pictures after a Display change"
        # (a panel whose real size arrives after the start - Tom's RGB10 MAX3, v0.6.2: the pictures are made
        # for the shape it settles at, once; the next start finds them ready)
        late = "wait:100,resize:1280x720,wait:700,right,wait:100,quit"
        out = run(exe, g, conf, late, "640x480")
        assert out.count("[scan] Preparing the ") == 2, "pictures for the panel's late size"
        out = run(exe, g, conf, late, "640x480")
        assert "[scan]" not in out, "pictures made again at the next start"
        out = run(exe, g, conf, "wait:100,start,menu:Rescan,a,wait:100,a,right,quit")
        assert out.count("[scan] Found ") == 2, "Rescan Game Files"
    update_test(exe)
    logs_test(exe)
    print("smoke test passed")


if __name__ == "__main__":
    main()
