import vsrandom
import faction_ships
import launch_recycle
import launch
import VS
import unit
import sys
import adventure
import news
import universe
import fg_util
import dynamic_battle
import debug
import generate_dyn_universe

class random_encounters:
    class playerdata:
        def GeneratePhaseAndAmplitude(self):
            self.prob_phase=20*vsrandom.random()
            self.prob_amplitude = .5+.5*vsrandom.random()
            self.prob_period = 20*vsrandom.random()+1
        def UpdatePhaseAndAmplitude(self):
            self.prob_phase+=1;
            return self.prob_amplitude*(.6+.4*VS.cos ((self.prob_phase*3.1415926536*2)/self.prob_period))
        def __init__(self,sig_distance,det_distance):
            debug.debug("init playerdata")
            try:
                faction_ships.Precache()
            except:
                import sys
                debug.debug(str(sys.exc_info()[0])+str(sys.exc_info()[1]))
            self.quests=[]
            self.curquest=0
            self.last_ship=0
            self.curmode=0
            self.lastmode=0
            self.lastsys=""
            self.sig_container=VS.Unit()
            self.significant_distance=sig_distance
            self.detection_distance=det_distance
            self.GeneratePhaseAndAmplitude()
            debug.debug("done playerdat")
    def __init__(self, sigdis, detectiondis, gendis,  minnships, gennships, unitprob, enemyprob, capprob, capdist):
        unitprob=1
        debug.debug("init random enc")
        
        self.capship_gen_distance=capdist
        #    player_num=player
        self.enprob = enemyprob
        self.fighterprob = unitprob

        self.det_distance = detectiondis
        self.sig_distance = sigdis
        self.players=[]
        self.generation_distance=gendis
        self.min_num_ships=minnships
        self.gen_num_ships=gennships
        self.capship_prob=capprob
        self.cur_player=0
        self.sig_distance_table = {"enigma_sector/heavens_gate":(2000,4000,.4)}
        debug.debug("end random enc")
    def AddPlayer (self):
#    print "begin add player"
        self.players+=[random_encounters.playerdata(self.sig_distance,self.det_distance)]
#    print "add player"
    def NewSystemHousekeeping(self,oldsystem,newsystem):
        fg_util.launchBases(newsystem)
        news.newNews()
        newquest = adventure.newAdventure (self.cur_player,oldsystem,newsystem)
        if (newquest):
            self.cur.quests+=[newquest]
        else:
            self.RestoreDroneMission()
        self.CalculateSignificantDistance()
    def RestoreDroneMission(self):
        qdf=adventure.persistentAdventure (self.cur_player)
        if (qdf):
            self.cur.quests+=[qdf]
    def CalculateSignificantDistance(self):
        sysfile = VS.getSystemFile()
        self.cur.GeneratePhaseAndAmplitude()
        if sysfile in self.sig_distance_table:
            self.cur.significant_distance = self.sig_distance_table[sysfile][0]
            self.cur.detection_distance = self.sig_distance_table[sysfile][1]
            self.cur.prob_amplitude = self.sig_distance_table[sysfile][2]
            return
        minsig =  unit.minimumSigDistApart()
        if (self.sig_distance>minsig*0.15):
            self.cur.significant_distance=minsig*0.15
        else:
            self.cur.significant_distance=self.sig_distance
        if (self.det_distance>minsig*0.2):
            self.cur.detection_distance=minsig*0.2
        else:
            self.cur.detection_distance=self.det_distance

        debug.debug("resetting sigdist=%f detdist=%f" % (self.cur.significant_distance,self.cur.detection_distance))

    def SetEnemyProb (self,enp):
        self.enprob = enp


    def AsteroidNear (self,uni, how):
        i = VS.getUnitList()
        dd = self.cur.detection_distance
        while i.notDone():
            un = i.current()            
            if (uni.getSignificantDistance(un)<how):
                if (unit.isAsteroid (un)):
                    debug.debug("asty near")
                    return 1
            i.advance()
        return 0
    def TrueEnProb(self,enprob):
        ret=1
        nam = VS.numActiveMissions()
        while (nam>0):
            ret*=(1-enprob)
            nam-=1
        debug.debug(1-ret)
        return 1-ret;
    def fixupFactionRelations(self):
        for fac in ["mining", "refinery","naval","commerce"]:
            reldel=VS.GetRelation("privateer",fac)
            VS.AdjustRelation("privateer",fac,-reldel,1);
            print "adjusting by "+str(reldel)
            reldel=VS.GetRelation(fac,"privateer")
            VS.AdjustRelation(fac,"privateer",-reldel,1);
            print "adjusting by "+str(reldel)
            
    def launch_near(self,un, forceLaunch=False):
        if (VS.GetGameTime()<10 and not forceLaunch):
            debug.debug("hola!")
            return
        self.fixupFactionRelations();
        cursys=VS.getSystemFile()
#    numsigs=universe.GetNumSignificantsForSystem(cursys)
        for factionnum in range(faction_ships.getMaxFactions()-1):
            faction=faction_ships.intToFaction(factionnum)
            fglist=fg_util.FGsInSystem(faction,cursys)
            if not len(fglist):
                print 'no flight group for faction: '+faction+' in system '+cursys+'.'
                continue
            num=len(fglist)
            print 'Probability numbers: ',num, fg_util.MaxNumFlightgroupsInSystem(cursys)#,numsigs
            avg=float(num)/float(fg_util.MaxNumFlightgroupsInSystem(cursys))#/float(numsigs)
            fortress_level=0
            if cursys in faction_ships.fortress_systems:
                foretress_level=faction_ships.fortress_systems[cursys]
            avg*=(not (VS.GetRelation(VS.GetGalaxyFaction(cursys),faction)<0 and cursys in faction_ships.fortress_systems))*fortress_level+(1-fortress_level)
            print 'Chance for %s ship: %g'%(faction, avg)
            rndnum=vsrandom.random()
            print 'Random number: %g; will generate ship: %d'%(rndnum,rndnum<avg)
            if rndnum<avg:
                #now we know that we will generate some ships!
                flightgroup=fglist[vsrandom.randrange(len(fglist))]
                typenumbers=fg_util.GetShipsInFG(flightgroup,faction)
                print 'FG Name: "%s", ShipTypes: %s'%(flightgroup,str(typenumbers))
                launch_recycle.launch_types_around(flightgroup,faction,typenumbers,'default',self.generation_distance*vsrandom.random()*0.9,un,self.generation_distance*vsrandom.random()*2,'')
    def atLeastNInsignificantUnitsNear (self,uni, n):
        num_ships=0
        leadah = uni.getFlightgroupLeader ()
        i = VS.getUnitList()
        dd = self.cur.detection_distance
        while i.notDone():
            un = i.current()
            if (uni.getSignificantDistance(un)<dd*1.6):
                if ((not un.isSignificant()) and (not un.isSun())):
                    unleadah = un.getFlightgroupLeader ()
                    if (leadah!=unleadah):
                        num_ships+=1
            i.advance()
        return num_ships>=n

    def SetModeZero(self):
        self.cur.last_ship=0
        self.cur.curmode=0
        self.cur.sig_container.setNull()
        for q in self.cur.quests:
            q.NoSignificantsNear()

    def SetModeOne (self,significant):
        self.cur.last_ship=0
        self.cur.curmode=1
        self.cur.sig_container=significant
        if VS.getPlayer().isNull():
            return
        cursys = VS.getSystemFile()
        oldsys = self.cur.lastsys==cursys
        if (not oldsys):
            self.NewSystemHousekeeping(self.cur.lastsys,cursys)
            self.cur.lastsys=cursys
        for q in self.cur.quests:
            q.SignificantsNear(self.cur.sig_container)
#    import dynamic_battle
#    dynamic_battle.UpdateCombatTurn()

    def decideMode(self):
        myunit=VS.getPlayerX(self.cur_player)
        if (not myunit):
            self.SetModeZero()
            return myunit
        significant_unit = self.cur.sig_container
#        un=VS.getUnit(0);
#        i=0
#        while (un):
#            debug.debug(un.getName())
#            i+=1
#            un=  VS.getUnit(i)

        if (not significant_unit):
            un=VS.getUnit(self.cur.last_ship)
            if (self.DifferentSystemP()):
                un.setNull()
            if (not un):
                self.SetModeZero()
            else:
                sd = self.cur.significant_distance
                if ((un.getSignificantDistance(myunit)<sd) and (un.isSignificant())):
                    self.SetModeOne (un)
                    return un
                self.cur.last_ship+=1
            return VS.Unit()
        else:
            #significant_unit is something.... lets see what it is
            cursys = VS.getSystemFile()
            if (self.DifferentSystemP()):
                debug.debug("different")
                self.SetModeZero()
                significant_unit.setNull ()
            else:
                dd = self.cur.detection_distance
                if (myunit.getSignificantDistance (significant_unit)>dd):
                    self.SetModeZero ()
                    return VS.Unit()
                else:
                    return significant_unit
            return significant_unit
    def DifferentSystemP(self):
        if VS.getPlayer().isNull():
            return 0
        cursys=VS.getSystemFile()
        if (cursys==self.cur.lastsys):
            return 0
        self.NewSystemHousekeeping(self.cur.lastsys,cursys)
        self.cur.lastsys=cursys
        return 1
    def Execute(self):
        generate_dyn_universe.KeepUniverseGenerated()
        dynamic_battle.UpdateCombatTurn()
        if (self.cur_player>=len(self.players)):
            self.AddPlayer()
        self.cur=self.players[self.cur_player]
        if (self.cur.curquest<len(self.cur.quests)):
            if (self.cur.quests[self.cur.curquest].Execute()):
                self.cur.curquest+=1
            else:
                del self.cur.quests[self.cur.curquest]
        else:
            self.cur.curquest=0
        un = self.decideMode ()
	if VS.getPlayerX(self.cur_player) and (self.cur.curmode!=self.cur.lastmode):
            #lastmode=curmode#processed this event don't process again if in critical zone
            self.cur.lastmode=self.cur.curmode
            print "curmodechange %d" % (self.cur.curmode)#?
            if un:
#      if ((vsrandom.random()<(self.fighterprob*self.cur.UpdatePhaseAndAmplitude())) and un):
                if (not self.atLeastNInsignificantUnitsNear (un,self.min_num_ships)):
                    #determine whether to launch more ships next to significant thing based on ships in that range
                    debug.debug("launch near")
                    self.launch_near (VS.getPlayerX(self.cur_player))
        self.cur_player+=1
        if (self.cur_player>=VS.getNumPlayers()):
            self.cur_player=0
	
	# This could cause some issues, don'cha think?
        #VS.setMissionOwner(self.cur_player)

debug.debug("done loading rand enc")
