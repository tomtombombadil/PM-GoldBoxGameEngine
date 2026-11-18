import os, sys, shutil, re
OUT=os.environ.get('OUT','/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import play_test as pt
pt.EXE=os.environ.get("EXE","build/goldbox"); GAMES=os.environ["GAMES"]; SAVES=os.environ.get("SAVES","")
def hps(p):
    out=[]
    for t in p.text:
        m=re.match(r"\?(\S[^?]*?)\s+(-?\d+)\s+(\d+)\?",t)
        if m: out.append((m.group(1).strip(),int(m.group(3))))
    return out
def run(cfg,tag):
    D=OUT+f"/maxhp_{tag}"; shutil.rmtree(D,ignore_errors=True); shutil.copytree(SAVES+"/C",D+"/games"); pt.OUT=D; pt.CFG_EXTRA=cfg
    r=pt.Run(D+"/games",1,"t"); p=r.p; pt.start(r)
    r.do("start,wait:300,menu:Cheats,a,wait:400"); r.do("menu:Trainer,a,wait:1200")
    before=hps(p); log=[]
    r.do(SEL)
    for m in range(6):
        pt.goto_line(p,"Train"); r.do("a,wait:900")
        for k in range(12):
            if p.has("BEGIN Adv") and "Choose" in r.bottom(): break
            log.append(r.bottom())
            if "Yes" in r.bottom(): pt.goto_word(p,"Yes")
            r.do("a,wait:700")
        r.do("down,wait:200") if False else None
        break
    after=hps(p)
    p.quit(); shutil.rmtree(D)
    return before,after,log
import os, sys
SEL=sys.argv[1]
for tag,cfg in [(a.split("=")[0], a.split("=",1)[1].replace("\\n","\n")) for a in sys.argv[2:]]:
    b,a,log=run(cfg,tag); print(tag, [(n, a[i][1]-h) for i,(n,h) in enumerate(b)], [l for l in log if l.strip()][:3])
