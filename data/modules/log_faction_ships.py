import faction_ships
import debug
stattable={}
def lg (num):
    import VS
    return VS.log(1+num)/VS.log(2)
#for i in faction_ships.stattableexp:
for i in faction_ships.stattable:
    #tuple = faction_ships.stattableexp[i]
    tuple = faction_ships.stattable[i]
    stattable[i]=(tuple[0],tuple[1],lg(tuple[2]),lg(tuple[3]),lg(tuple[4]))
    stattable[i+'.blank']=(tuple[0],tuple[1]*.5,lg(tuple[2])*.5,lg(tuple[3])*.5,lg(tuple[4])*.5)
debug.debug(repr(stattable))
