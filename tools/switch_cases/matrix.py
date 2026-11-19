import os, sys
OUT=os.environ.get('OUT','/tmp'); GAMES=os.environ.get('GAMES',''); SAVES=os.environ.get('SAVES',''); sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tour import tour
from multiprocessing import Pool
def one(a): return tour(a[0],a[1],a[2])
gi=int(sys.argv[1])
cases={"all on":"", "Icon Gallery off":"comforts_off=1\n","More Characters off":"comforts_off=4\n","Icon Portrait off":"comforts_off=8\n","Savegame Info off":"comforts_off=2048\n","Game Font: the game's own":"font=0\n","Camp Fix off":"comforts_off=64\n","Character Info off":"comforts_off=32768\n"}
with Pool(2) as ex:
    res=dict(zip(cases, ex.map(one, [(gi,v,str(i)) for i,v in enumerate(cases.values())])))
base=res["all on"]
for k,c in res.items():
    if k=="all on": continue
    diff=[x for x in base if c.get(x)!=base[x]]
    print(f"{k}: differs in {diff}")
    for x in diff:
        if "canvas" not in x: print(f"   {x} on : {base[x][-160:]!r}\n   {x} off: {c.get(x,'')[-160:]!r}")
