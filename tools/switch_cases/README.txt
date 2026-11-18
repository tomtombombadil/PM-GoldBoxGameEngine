A switch's EFFECT in a game, through the port's buttons - the cases behind tools/switch_test.py's
table (2026-11-18). Each plays the same steps with a switch on and off and prints what the game
showed. They need real game files and are run by hand:

  GAMES=<a scanned copy of the nine GOG folders>  OUT=<scratch dir>  EXE=build/goldbox
  SAVES=<dir of prepared saves>   (maxhp, km: made with the CYD playsim from the stock Curse save -
        C: SETXP=300000 SAVENOW=A;  K: MEMSET="4:" SAVENOW=A;  rd's argument: a games copy whose
        save A has member 0 dead - RECSET="0:195=6,0:1a4=0" SAVENOW=A)

  matrix.py <game n>      Icon Gallery, More Characters, Icon Portrait, Savegame Info, Game Font, Camp Fix
                          (tour.py's captures, all-on against each switch off)
  rolls.py <game n>       Show Rolls - the roll lines in a fight
  maxhp.py <keys> tag=cfg ...   a Trainer level-up's hit points (Max Hit Points; Multi-Class Hit Points)
  km.py <cfg>             Keep Memorizing - chosen spells after the camp is left
  rd.py <cfg> <games>     Raising the Dead - Constitution after the Healer's Raise Dead
  race.py <n> <race> <rolls> tag=cfg ...   Race Adjustments - average rolled abilities
<game n> counts from 1. Scratch-grown scripts: read before reuse.
