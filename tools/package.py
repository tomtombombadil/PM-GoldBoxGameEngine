#!/usr/bin/env python3
"""Makes the port's package from port/ and the built binaries
(dist/goldbox.<arch>): dist/goldbox_port-v<version>.zip with its .sha256
(a release's assets - the installer and the port's own updater look for
that name), dist/GoldBox.Installer.sh beside them, and the same zip as
dist/goldbox.zip (PortMaster's name for it):

    Gold Box Game Engine.sh
    goldbox/  port.json, README.md, gameinfo.xml, VERSION, goldbox.aarch64,
              ..., tools/, licenses/, games/README.txt

Checks, as it goes, that nothing from the games is in it.
"""
import hashlib
import json
import os
import shutil
import pathlib
import sys
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
DIST = ROOT / "dist"
PORT = ROOT / "port"
NAME = "goldbox"
SCRIPT = "Gold Box Game Engine.sh"
# Game files never ship (bring your own game)
BLOCKED = {".dax", ".tlb", ".glb", ".pdf", ".exe", ".ovr", ".cha", ".sav", ".ecl", ".fon", ".drv"}


def main() -> int:
    version = (ROOT / "VERSION").read_text().strip()
    port = json.loads((PORT / NAME / "port.json").read_text())
    # (PACKAGE_ARCHS=x86_64: a package of fewer builds, for the checks)
    archs = os.environ.get("PACKAGE_ARCHS", "").split() or port["attr"]["arch"]
    binaries = [DIST / f"{NAME}.{a}" for a in archs]
    missing = [b.name for b in binaries if not b.is_file()]
    if missing:
        print("not built:", ", ".join(missing))
        return 1
    files = []  # (path in the zip, source, executable)
    files.append((SCRIPT, PORT / SCRIPT, True))
    for p in sorted((PORT / NAME).rglob("*")):
        if p.is_file():
            files.append((f"{NAME}/{p.relative_to(PORT / NAME).as_posix()}", p, p.suffix == ".sh"))
    for b in binaries:
        files.append((f"{NAME}/{b.name}", b, True))
    files.append((f"{NAME}/licenses/LICENSE.goldbox.txt", ROOT / "LICENSE", False))
    files.append((f"{NAME}/licenses/LICENSE.engine.txt", ROOT / "cyd" / "LICENSE", False))
    files.append((f"{NAME}/licenses/THIRD_PARTY_NOTICES.md", ROOT / "THIRD_PARTY_NOTICES.md", False))
    files.append((f"{NAME}/VERSION", ROOT / "VERSION", False))
    for name, src, _ in files:
        if pathlib.PurePosixPath(name).suffix.lower() in BLOCKED:
            print("a game file must never be packaged:", src)
            return 1
    for name, src, _ in files + [("", PORT / "GoldBox.Installer.sh", True)]:
        if src.suffix == ".sh" and b"\r" in src.read_bytes():
            print("Windows line ends - it would not run on a handheld:", src)
            return 1
    out = DIST / f"{NAME}_port-v{version}.zip"
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        for name, src, exe in files:
            info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = ((0o755 if exe else 0o644) | 0o100000) << 16
            z.writestr(info, src.read_bytes())
    digest = hashlib.sha256(out.read_bytes()).hexdigest()
    (DIST / (out.name + ".sha256")).write_text(f"{digest}  {out.name}\n")
    shutil.copyfile(out, DIST / f"{NAME}.zip")
    shutil.copyfile(PORT / "GoldBox.Installer.sh", DIST / "GoldBox.Installer.sh")
    print(f"{out.relative_to(ROOT)}: version {version}, {len(files)} files, {out.stat().st_size // 1024} KB, sha256 {digest[:12]}...")
    return 0


if __name__ == "__main__":
    sys.exit(main())
