import os, sys, shutil, re
OUT=os.environ.get('OUT','/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import play_test as pt
pt.EXE=os.environ.get("EXE","build/goldbox"); GAMES=os.environ["GAMES"]; SAVES=os.environ.get("SAVES","")
def txt(p): return "\n".join(t.rstrip() for t in p.text if t.strip(" ?"))
gi=int(sys.argv[1]); race=sys.argv[2]; N=int(sys.argv[3])
def run(cfg,tag):
    D=OUT+f"/race_{tag}"; shutil.rmtree(D,ignore_errors=True); shutil.copytree(GAMES,D+"/games"); pt.OUT=D; pt.CFG_EXTRA=cfg
    r=pt.Run(D+"/games",gi,"t"); p=r.p; pt.to_party_menu(p,70)
    pt.goto_line(p,"Create New") or pt.goto_line(p,"Create Character"); r.do("a,wait:700")
    for k in range(12):
        f=p.find(race)
        if f and p.lit_at(f[0],f[1],len(race)): break
        r.do("down,wait:200")
    picked=bool(f and p.lit_at(f[0],f[1],len(race)))
    rolls=[]
    for i in range(400):
        b=r.bottom()
        if "reroll" in b.lower() or "keep" in b.lower():
            st={}
            for t in p.text:
                for m in re.finditer(r"(STR|INT|WIS|DEX|CON|CHA)\s+(\d+)",t): st[m.group(1)]=int(m.group(2))
            rolls.append(st)
            if len(rolls)>=N: break
            w="Yes" if "reroll" in b.lower() else "Reroll"
            if not pt.goto_word(p,w): print("cannot reroll:",b); break
            r.do("a,wait:300")
        else: r.do("a,wait:500")
    if "-v" in sys.argv: print(txt(p))
    p.quit(); shutil.rmtree(D)
    return picked,rolls
for a in sys.argv[4:]:
    if a=="-v": continue
    tag,cfg=a.split("=",1); picked,rolls=run(cfg.replace("\\n","\n"),tag)
    print(tag,"picked",race,picked,"rolls",len(rolls),{s: round(sum(x[s] for x in rolls if s in x)/len(rolls),2) for s in ("STR","INT","WIS","DEX","CON","CHA")} if rolls else "")
