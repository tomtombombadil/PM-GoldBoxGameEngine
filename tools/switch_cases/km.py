import os, sys, shutil, re
OUT=os.environ.get('OUT','/tmp'); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import play_test as pt
pt.EXE=os.environ.get("EXE","build/goldbox"); GAMES=os.environ["GAMES"]; SAVES=os.environ.get("SAVES","")
def txt(p): return "\n".join(t.rstrip() for t in p.text if t.strip(" ?"))
cfg=sys.argv[1].replace("\\n","\n") if len(sys.argv)>1 else ""
D=OUT+"/km_run"; shutil.rmtree(D,ignore_errors=True); shutil.copytree(SAVES+"/K",D+"/games"); pt.OUT=D; pt.CFG_EXTRA=cfg
r=pt.Run(D+"/games",1,"t"); p=r.p; pt.start(r)
def step(keys,tag): r.do(keys); print("--",tag,"|",r.bottom()); print(txt(p)[-330:] if "-v" in sys.argv else "")
pt.goto_word(p,"Encamp"); step("a,wait:900","camp")
pt.goto_word(p,"Magic"); step("a,wait:900","magic")
r.do("r2,wait:200,r2,wait:200,r2,wait:200,r2,wait:200")
pt.goto_word(p,"Memorize"); step("a,wait:900","memorize")
r.do("a,wait:700"); r.do("a,wait:700"); r.do("b,wait:700")
print("asked:", r.bottom()); pt.goto_word(p,"Yes"); r.do("a,wait:700")
for k in range(4):
    if r.exploring(): break
    r.do("b,wait:700")
print("left the camp:", r.exploring(), r.bottom())
pt.goto_word(p,"Encamp"); r.do("a,wait:900"); pt.goto_word(p,"Magic"); r.do("a,wait:900")
pt.goto_word(p,"Memorize"); r.do("a,wait:900")
print("RESULT", r.bottom(), "\n", txt(p)[-420:])
p.quit()
