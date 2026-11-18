import os, sys, shutil, hashlib
OUT=os.environ.get('OUT','/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import play_test as pt
pt.EXE=os.environ.get("EXE","build/goldbox"); GAMES=os.environ["GAMES"]; SAVES=os.environ.get("SAVES","")
def txt(p): return "\n".join(t.rstrip() for t in p.text if t.strip(" ?"))
def tour(gi, cfg, tag, verbose=False):
    D=OUT+f"/tour_{gi}_{tag}"; shutil.rmtree(D,ignore_errors=True); shutil.copytree(GAMES,D+"/games"); pt.OUT=D; pt.CFG_EXTRA=cfg
    r=pt.Run(D+"/games",gi,"t"); p=r.p; cap={}
    pt.to_party_menu(p,70)
    # Add Character lists (More Characters)
    if pt.goto_line(p,"Add Character"):
        r.do("a,wait:700"); cap["add1"]=txt(p)
        r.do("a,wait:900"); cap["add2"]=txt(p)
        for k in range(4):
            if p.has("Load Saved"): break
            r.do("b,wait:500")
    pt.start(r)
    cap["explore_canvas"]=p.st["canvas"]; cap["explore_text"]=txt(p)
    pt.goto_word(p,"View"); r.do("a,wait:800"); cap["view_canvas"]=p.st["canvas"]; cap["view_text"]=txt(p); r.do("b,wait:500")
    pt.goto_word(p,"Encamp"); r.do("a,wait:900"); cap["camp_menu"]=r.bottom()
    pt.goto_word(p,"Save"); r.do("a,wait:900"); cap["save_text"]=txt(p); cap["save_canvas"]=p.st["canvas"]; r.do("b,wait:600")
    if pt.goto_word(p,"Alter"):
        r.do("a,wait:800"); cap["alter_menu"]=r.bottom()
        if pt.goto_word(p,"Icon"): r.do("a,wait:1200"); cap["icon_text"]=txt(p)[-300:]; cap["icon_bottom"]=r.bottom()
    p.quit(); shutil.rmtree(D)
    return cap
if __name__=="__main__":
    gi=int(sys.argv[1]); base=tour(gi,"","on")
    for k,v in base.items(): print("##",k); print(v)
