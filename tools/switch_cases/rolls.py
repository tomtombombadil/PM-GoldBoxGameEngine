import os, sys, shutil, re
OUT=os.environ.get('OUT','/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")); sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import play_test as pt
pt.EXE=os.environ.get("EXE","build/goldbox"); GAMES=os.environ["GAMES"]; SAVES=os.environ.get("SAVES","")
def run(gi,cfg,tag,steps=400):
    D=OUT+f"/rolls_{gi}_{tag}"; shutil.rmtree(D,ignore_errors=True); shutil.copytree(GAMES,D+"/games"); pt.OUT=D; pt.CFG_EXTRA=cfg
    r=pt.Run(D+"/games",gi,"f"); p=r.p; pt.start(r)
    n=0
    import random; rnd=random.Random(7)
    while p.st["fighting"]!="1" and n<steps:
        r.do(rnd.choice(["up","up","up","left","right"])+",wait:260"); n+=1
        if not r.exploring() and p.st["fighting"]!="1": r.settle(25)
    seen=set(); hits=0; k=0
    while p.st["fighting"]=="1" and k<500:
        w=r.words()
        if "Quick" in w and pt.goto_word(p,"Quick"): r.do("a,wait:250")
        else: r.do("a,wait:250")
        for t in p.text[17:24]:
            t=t.strip(" ?")
            if t: seen.add(re.sub(r"\d+","N",t))
        k+=1
    p.quit(); shutil.rmtree(D)
    return n,k,sorted(seen)
if __name__=="__main__":
    gi=int(sys.argv[1])
    for tag,cfg in (("on",""),("off","comforts_off=32\n")):
        n,k,seen=run(gi,cfg,tag)
        roll=[s for s in seen if re.search(r"To Hit|Needed|Damage:|Save",s)]
        print(f"Show Rolls {tag}: fight after {n} steps, {k} presses, {len(seen)} different lines in the panel's foot; roll lines: {roll[:6]}")
        if tag=="on": print("   sample:", seen[:12])
