import VS
import vsrandom
import debug

BATTLELIST=0
PEACELIST=1
PANICLIST=2
VICTORYLIST=3
LOSSLIST=4
_situation_name = { BATTLELIST:  'battle',
                    PEACELIST:   'peace',
                    PANICLIST:   'panic',
                    VICTORYLIST: 'victory',
                    LOSSLIST:    'loss' }
def situation_name(situation):
    global _situation_name
    try:
        return _situation_name[situation]
    except:
        return 'unknown'

HOSTILE_AUTODIST=1600
HOSTILE_NEWLAUNCH_DISTANCE=6000
peacelist={"aera":VS.musicAddList('aera.m3u'),
            "confed":VS.musicAddList('terran.m3u'),
            "iso":VS.musicAddList('iso.m3u'),
            "AWACS":VS.musicAddList('AWACS_peace.m3u'),
            None:PEACELIST
            }
battlelist={"aera":VS.musicAddList('aerabattle.m3u'),
            "confed":VS.musicAddList('terranbattle.m3u'),
            "iso":VS.musicAddList('isobattle.m3u'),
            "AWACS":VS.musicAddList('AWACS.m3u'),
            None:BATTLELIST
            }
paniclist={None:PANICLIST,
            "AWACS":VS.musicAddList('AWACS.m3u')}
asteroidmisic=VS.musicAddList('asteroids.m3u')

def LookupTable(list,faction):
    if faction in list:
        if (list[faction]!=-1):
            return list[faction]
        else:
            return list[None]
    else:
        return list[None]
situation=PEACELIST

__enabled = True

def enable():
    global __enabled
    if not __enabled:
        debug.debug("dj_lib: enabling")
    __enabled = True

def disable():
    global __enabled
    if __enabled:
        debug.debug("dj_lib: disabling")
    __enabled = False

def mpl (list,newsituation,forcechange):
    global situation
    debug.debug("Situation is "+situation_name(situation)+' ('+str( situation)+") force change "+str(forcechange) + " bool "+ str(forcechange or newsituation!=situation), debug.INFO)
    if (forcechange or newsituation!=situation):
        debug.debug("Situation is RESET to "+situation_name(newsituation)+' ('+str(newsituation)+')', debug.NOTICE)
        situation=newsituation
        VS.musicPlayList(list)

def PlayMusik(forcechange=1,hostile_dist=0):
    global __enabled
    un = VS.getPlayer()
    if not un or not __enabled:
        #mpl (PEACELIST,PEACELIST,forcechange)
        #print "Ppeace"
        pass
    elif un.DockedOrDocking() not in [1,2]:
        perfect=1
        iter = VS.getUnitList()
        target = iter.current()
        unlist=[]
        asteroid=0
        while (iter.notDone()):
            if (target):
                ftmp = 2*target.getRelation(un)
                nam=target.getName().lower()
                if un.getSignificantDistance(target)<=2*target.rSize() and ('afield'==nam[:6] or 'asteroid'==nam[:8]):
                    asteroid=1
                hdis = HOSTILE_AUTODIST
                if (hostile_dist!=0):
                    hdis = hostile_dist
                if (target.GetTarget()==un or (ftmp<0 and un.getDistance(target)<hdis)):
                    unlist.append(target.getFactionName())
                    perfect=0
            iter.advance()
            target=iter.current()
        if (perfect):
            if asteroid and asteroidmisic!=-1 and vsrandom.random()<.7:
                mpl(asteroidmisic,PEACELIST,forcechange)
                return
            sys=VS.getSystemFile()
            fact=VS.GetGalaxyFaction(sys)
            if vsrandom.random()<.5:
                fact=None
            mpl(LookupTable(peacelist,fact),PEACELIST,forcechange)
            debug.debug("music: peaCce", debug.NOTICE)
        else:
            ftmp = (un.FShieldData()+2*un.GetHullPercent()+un.RShieldData()-2.8)*2
            fact=None
            if len(unlist) and vsrandom.random()<.5:
                fact=unlist[vsrandom.randrange(0,len(unlist))]
            debug.debug("music for " + str(fact), debug.NOTICE)
            if (ftmp<-.5):
                mpl(LookupTable(paniclist,fact),BATTLELIST,forcechange)
                debug.debug("music: paAnic", debug.NOTICE)
            else:
                mpl(LookupTable(battlelist,fact),BATTLELIST,forcechange)
                debug.debug("music: bSattle", debug.NOTICE)

