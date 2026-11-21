#!/bin/bash
# racecap_prep.sh <playsim> <games folder (scanned, GOG)> <out> <game n> <folder name> [begin tokens]
# The saves racecap.py plays: the game's own saved game A with member 1 made an elf fighter (Strength 18: its
# limit 7) with experience for everything - slot B at level 6 (near the limit), C at 7 (at it), D at 8 (past it).
# <playsim>: the CYD repo's (cyd/tools/devkit/build.sh splay "-DCYD_TEST_HOOKS" tools/playsim/sim.cpp).
# Begin tokens, where the save doesn't open at the exploring menu: Secret "....../E/N.", Darkness "/N....".
# Gateway (GOG) has no saved game of its own: the CYD's tools/devkit/make_saves.sh gateway A first.
# Scratch only - the result is made from game files. Then: GAMES=<out>/g<n> racecap.py B C D
sim=$1; src=$2; out=$3; n=$4; f=$5; tok=${6:-}
R=$out/g$n; rm -rf "$R"; mkdir -p "$R/_CYD"; cp -r "$src/$f" "$R/"; cp -r "$src/_CYD/$f" "$R/_CYD/" 2>/dev/null
gf="$R/$f"; [ "$n" = 1 ] && gf="$R/$f/POOLRAD"
for s in B:6 C:7 D:8; do sl=${s%%:*}; lv=${s##*:}
  (cd "$R" && env GAME=$n SAVE=A SAVESLOT=$sl RECSET="0:74=2,0:75=2,0:10=12,0:11=12,0:109=0,0:10a=0,0:10b=$lv,0:10c=0,0:10d=0,0:10e=0,0:10f=0,0:110=0,0:127=0,0:128=87,0:129=93,0:12a=3" timeout 200 "$sim" "$gf" "$tok" 2>&1 | grep -a "saved at" | tail -1)
done
