import os, sys, shutil, re
OUT=os.environ.get('OUT','/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import play_test as pt
pt.EXE=os.environ.get("EXE","build/goldbox"); GAMES=os.environ["GAMES"]; SAVES=os.environ.get("SAVES","")
def txt(p): return "\n".join(t.rstrip() for t in p.text if t.strip(" ?"))
cfg=sys.argv[1].replace("\\n","\n")
D=OUT+"/rd_run"; shutil.rmtree(D,ignore_errors=True); shutil.copytree(sys.argv[2],D+"/games"); pt.OUT=D; pt.CFG_EXTRA=cfg
r=pt.Run(D+"/games",1,"t"); p=r.p; pt.start(r)
def con(tag):
    pt.goto_word(p,"View"); r.do("a,wait:800")
    print(tag, [t.strip(" ?")[:24] for t in p.text if "CON" in t or "Status" in t or "HP " in t]); r.do("b,wait:500")
print("exploring:", r.bottom(), {k: hex(v) for k,v in p.eng.items()}); print(open(D+"/conf-1/settings.cfg").read().replace("\n"," ; "))
r.do("start,wait:300,menu:Cheats,a,wait:400"); r.do("menu:Healer,a,wait:1200"); print("healer:", r.bottom())
con("before:")
pt.goto_word(p,"Heal"); r.do("a,wait:900"); print("heal:", r.bottom()); 
if "-v" in sys.argv: print(txt(p))
f=p.find("Raise Dead")
for k in range(12):
    f=p.find("Raise Dead")
    if f and p.lit_at(f[0],f[1],8): break
    r.do("down,wait:200")
print("on Raise Dead:", bool(f and p.lit_at(f[0],f[1],8)))
r.do("a,wait:900"); print(">", r.bottom()); print(txt(p)[-300:])
for k in range(0):
    if "Yes" in r.bottom(): pt.goto_word(p,"Yes")
    if r.bottom().startswith("Heal View"): break
    r.do("a,wait:800"); print(">", r.bottom(), "|", [t.strip(" ?") for t in p.text[16:24] if t.strip(" ?")][-2:])
for k in range(4):
    if r.bottom().startswith("Heal View"): break
    r.do("b,wait:600")
con("after: ")
p.quit()
