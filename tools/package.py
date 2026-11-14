#!/usr/bin/env python3
"""Makes the PortMaster package dist/goldbox.zip from port/ and the built
binaries (dist/goldbox.<arch>):

    Gold Box Game Engine.sh
    goldbox/  port.json, README.md, gameinfo.xml, goldbox.aarch64, ...,
              licenses/, games/README.txt

Checks, as it goes, that nothing from the games is in it.
"""
import json
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
    binaries = [DIST / f"{NAME}.{a}" for a in port["attr"]["arch"]]
    missing = [b.name for b in binaries if not b.is_file()]
    if missing:
        print("not built:", ", ".join(missing))
        return 1
    files = []  # (path in the zip, source, executable)
    files.append((SCRIPT, PORT / SCRIPT, True))
    for p in sorted((PORT / NAME).rglob("*")):
        if p.is_file():
            files.append((f"{NAME}/{p.relative_to(PORT / NAME).as_posix()}", p, False))
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
    if b"\r\n" in (PORT / SCRIPT).read_bytes():
        print("the launch script has Windows line ends: it would not run on a handheld")
        return 1
    out = DIST / f"{NAME}.zip"
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        for name, src, exe in files:
            info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = ((0o755 if exe else 0o644) | 0o100000) << 16
            z.writestr(info, src.read_bytes())
    print(f"{out.relative_to(ROOT)}: version {version}, {len(files)} files, {out.stat().st_size // 1024} KB")
    return 0


if __name__ == "__main__":
    sys.exit(main())
