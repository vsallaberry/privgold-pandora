/// Headers for python modules
///
#ifndef _UNIVERSE_UTILGENERIC_H__
#define _UNIVERSE_UTILGENERIC_H__
#include "cmd/collection.h"
#include "gfx/vec.h"
#include "networking/const.h"
#include "cmd/unit_util.h"

#include <string>
#include <vector>


class Unit;
class StarSystem;
class Cargo;

namespace UniverseUtil
{

	class PythonUnitIter: public un_iter
	{
		public:
			PythonUnitIter ():UnitIterator(){}
			//	PythonUnitIterator (UnitListNode * start):UnitIterator(start){}
			PythonUnitIter (const UnitCollection::UnitIterator &t):UnitIterator(t){}
			 ~PythonUnitIter() {;}
			Unit* current();
			inline void advance() { UnitIterator::advance();}
			void advanceSignificant();
			void advanceInsignificant();
			void advancePlanet();
			void advanceJumppoint();
			void advanceN(int);
			void advanceNSignificant(int n);
			void advanceNInsignificant(int n);
			void advanceNPlanet(int n);
			void advanceNJumppoint(int n);
			inline void preinsert(Unit *unit) { UnitIterator::preinsert(unit);}
			inline bool isDone() { return(UnitIterator::isDone());}
			inline bool notDone() {return(UnitIterator::notDone());}
			inline void remove() { UnitIterator::remove();}
			inline class Unit* next() { advance();return(current());}
			
	};

	std::string LookupUnitStat(const std::string &unitname, const std::string &faction, const std::string &statname);

	Unit * GetUnitFromSerial( ObjSerial serial);
	///this gets a unit with 1 of each cargo type in it
	Unit *GetMasterPartList ();
	///this function sets the "current" system to be "name"  where name may be something like "Sol/Sol" or "Crucible/Cephid_17"  this function may take some time if the system has not been loaded before

	void pushSystem (const std::string & name);
	bool systemInMemory(const std::string & name);
	///this function restores the active system.... there must be an equal number of pushSystems ans popSystems or else Vega Strike may behave unpredictably
	void popSystem ();
	///This function gets the current system's official name
	std::string getSystemFile();
	///this function gets the current system's nickname (not useful)
	std::string getSystemName();
	///this function gets an iterator into the units in the current system... do NOT keep an iterator across a frame--it may get deleted!
	PythonUnitIter getUnitList();
	///This function gets a unit given a number (how many iterations to go down in the iterator)
	Unit *getUnit(int index);
	///This function gets a unit given a name
	Unit *getUnitByName(const std::string & name);
	///This function gets a unit given an unreferenceable pointer to it - much faster if finder is provided
	Unit *getUnitByPtr(void* ptr, Unit * finder=0, bool allowslowness=true);
	Unit *getScratchUnit();
	void setScratchUnit(Unit *);

	void precacheUnit(const std::string & name, const std::string & faction);
	QVector getScratchVector();
	void setScratchVector(QVector);
	int getNumUnits();
	void cacheAnimation (const std::string & anim);
	///this function launches a wormhole or ajump point.
	Unit *launchJumppoint(const std::string & name_string,
		const std::string & faction_string,
		const std::string & type_string,
		const std::string & unittype_string,
		const std::string & ai_string,
		int nr_of_ships,
		int nr_of_waves,
		QVector pos,
		const std::string & squadlogo,
		const std::string & destinations);
	std::string vsConfig(const std::string & category,const std::string & option,const std::string & def);
	///this function launches a normal fighter  the name is the flightgroup name, the type is the ship type, the faction is who it belongs to, the unittype is usually "unit" unless you want to make asteroids or nebulae or jump points or planets.  the aistring is either a python filename or "default"  the nr of ships is the number of ships to be launched with this group, the number of waves is num reinforcements... the position is a tuple (x,y,z) where they appear and the squadlogo is a squadron image...you can leave this the empty string '' for the default squadron logo.
	Unit* launch (const std::string & name_string,const std::string & type_string,const std::string & faction_string,const std::string & unittype, const std::string & ai_string,int nr_of_ships,int nr_of_waves, QVector pos, const std::string & sqadlogo);
	///this gets a random cargo type (useful for cargo missions) from either any category if category is '' or else from a specific category  'Contraband'  comes to mind!
	Cargo getRandCargo(int quantity, const std::string & category);
	///this gets a string which has in it a space delimited list of neighmoring systems
	std::string GetAdjacentSystem (const std::string & str, int which);
	///this gets a specific property of this system as found in universe/milky_way.xml
	std::string GetGalaxyProperty (const std::string & sys, const std::string & prop);
	///this gets a specific property of this system as found in universe/milky_way.xml and returns a default value if not found
	std::string GetGalaxyPropertyDefault (const std::string & sys, const std::string & prop, const std::string & def);
	///this gets the number of systems adjacent to the sysname
	std::string GetGalaxyFaction(const std::string & sys);
	void SetGalaxyFaction(const std::string & sys, const std::string & fac);
	int GetNumAdjacentSystems (const std::string & sysname);
	///this gets the current time in seconds
	float GetGameTime ();
	///this sets the time compresison value to zero
	void SetTimeCompression ();
	///this adds a playlist to the music and may be triggered with an int
	int musicAddList(const std::string & str);
	///sets the software volume, with smooth transitions (latency_override==-1 uses default transition time)
	void musicSetSoftVolume(float vol, float latency_override);
	///sets the hardware volume, does not support transitions of any kind.
	void musicSetHardVolume(float vol);
	///this plays a specific song, at a specific layer
	void musicLayerPlaySong(const std::string & str,int layer);
	///this plays msuci from a given list, at a specific layer (where the int is what was returned by musicAddList)
	void musicLayerPlayList(int which,int layer);
	///this plays msuci from a given list, at a specific layer (where the int is what was returned by musicAddList)
	void musicLayerLoopList(int numloops,int layer);
	///this skips the current music track, at a specific layer (and goes to the next in the currently playing list)
	void musicLayerSkip(int layer);
	///this stops the music currently playing at a specific layer - with a nice fadeout
	void musicLayerStop(int layer);
	///sets the software volume, with smooth transitions (latency_override==-1 uses default transition time)
	void musicLayerSetSoftVolume(float vol, float latency_override, int layer);
	///sets the hardware volume, does not support transitions of any kind.
	void musicLayerSetHardVolume(float vol, int layer);
	///this mutes sound - or unmutes it
	void musicMute(bool stopSound);
	///this plays a specific song, through the crossfader construct
	inline void musicPlaySong(const std::string & str) { musicLayerPlaySong(str,-1); }
	///this plays msuci from a given list (where the int is what was returned by musicAddList)
	inline void musicPlayList(int which) { musicLayerPlayList(which,-1); }
	///this plays msuci from a given list (where the int is what was returned by musicAddList)
	inline void musicLoopList(int numloops) { musicLayerLoopList(numloops,-1); }
	///this skips the current music track (and goes to the next in the currently playing list)
	inline void musicSkip() { musicLayerSkip(-1); }
	///this stops the music currently playing - with a nice fadeout
	inline void musicStop() { musicLayerStop(-1); }
	///this gets the difficutly of the game... ranges between 0 and 1... many missions depend on it never going past .99 unless it's always at one.
	float GetDifficulty ();
	///this sets the difficulty
	void SetDifficulty (float diff);
	///this plays a sound at a location...if the sound has dual channels it will play in the center
	void playSound(const std::string & soundName, QVector loc, Vector speed);
  ///this plays a sound at full volume in the cockpit
  void playSoundCockpit(const std::string & soundName);
	///this plays an image (explosion or warp animation) at a location
	void playAnimation(const std::string & aniName, QVector loc, float size);
	void playAnimationGrow(const std::string & aniName, QVector loc, float size,float growpercent);
	///tells the respective flightgroups in this system to start shooting at each other
	void TargetEachOther (const std::string & fgname, const std::string & faction, const std::string & enfgname, const std::string & enfaction);
	///tells the respective flightgroups in this system to stop killing each other urgently...they may still attack--just not warping and stuff
	void StopTargettingEachOther(const std::string & fgname, const std::string & faction, const std::string & enfgname, const std::string & enfaction);

	///this ends the mission with either success or failure
	void terminateMission(bool term);
	///this gets the player belonging to this mission
	Unit *getPlayer();
	///this gets a player number (if in splitscreen mode)
	Unit *getPlayerX(int which);
	unsigned int getCurrentPlayer();
	///this gets the number of active players
 	int getNumPlayers ();
	// Clears all objectives (used for server-side when it's easy to get inconsistent.
    void setTargetLabel(const std::string & label);
    std::string getTargetLabel();


	float getRelationModifierInt(int which_cp, int faction);
	float getRelationModifier(int which_cp, const std::string & faction);
	float getFGRelationModifier(int which_cp, const std::string & fg);
	void adjustRelationModifierInt(int which_cp, int faction, float delta);
	void adjustRelationModifier(int which_cp, const std::string & faction, float delta);
	void adjustFGRelationModifier(int which_cp, const std::string & fg, float delta);
	
	void AdjustRelation(const std::string & myfaction,const std::string & theirfaction, float factor, float rank);
	float GetRelation(const std::string & myfaction,const std::string & theirfaction);

	void clearObjectives();
	// Erases an objective.
	void eraseObjective(int which);
	///this adds an objective for the cockpit to view ("go here and do this)
	int addObjective(const std::string & objective);
	///this sets the objective's completeness (the int was returned by add objective)
	void setObjective(int which, const std::string & newobjective);
	///this sets the completeness of a particular objective... chanigng the color onscreen
	void setCompleteness(int which, float completeNess);
	///this gets that completeness
	float getCompleteness(int which);
	///this sets the owner of a completeness
	void setOwnerII(int which,Unit *owner);
	///this gets an owner of a completeness (NULL means all players can see this objective)
	Unit* getOwner(int which);
	//gets the owner of this mission
	int getMissionOwner();
	//sets the owner of this mission to be a particular cockpit
	void setMissionOwner(int);
	///returns number missions running to tweak difficulty
	int numActiveMissions();
	///this sends an IO message... I'm not sure if delay currently works, but from, to and message do :-) ... if you want to send to the bar do "bar" as the to string... if you want to make news for the news room specify "news"
	void IOmessage(int delay,const std::string & from,const std::string & to,const std::string & message);
	///this gets a unit with 1 of each cargo type in it
	Unit *GetMasterPartList ();
	///this gets a unit with a faction's contraband list... may be null (check with isNull)
	Unit *GetContrabandList (const std::string & faction);
	///this sets whether or not a player may autopilot.  Normally they are both 0 and the autopiloting is allowed based on if enemies are near... if you pass in 1 then autopilot will be allowed no matter who is near... if you set -1 then autopilot is never allowed.  global affects all players... player just affects the player who accepted the mission.
	void SetAutoStatus (int global_auto, int player_auto);
	void LoadMission (const std::string & missionname);
	void LoadMissionScript (const std::string & scriptcontents);
	void LoadNamedMissionScript (const std::string & missiontitle, const std::string & scriptcontents);
	QVector SafeEntrancePoint (QVector,float radial_size=-1);
	QVector SafeStarSystemEntrancePoint (StarSystem *sts, QVector,float radial_size=-1);
	float getPlanetRadiusPercent ();

	void cacheAnimation (const std::string & anim);
	///this function launches a wormhole or ajump point.
	Unit *launchJumppoint(const std::string & name_string,
		const std::string & faction_string,
		const std::string & type_string,
		const std::string & unittype_string,
		const std::string & ai_string,
		int nr_of_ships,
		int nr_of_waves,
		QVector pos,
		const std::string & squadlogo,
		const std::string & destinations);
	///this function launches a normal fighter  the name is the flightgroup name, the type is the ship type, the faction is who it belongs to, the unittype is usually "unit" unless you want to make asteroids or nebulae or jump points or planets.  the aistring is either a python filename or "default"  the nr of ships is the number of ships to be launched with this group, the number of waves is num reinforcements... the position is a tuple (x,y,z) where they appear and the squadlogo is a squadron image...you can leave this the empty string '' for the default squadron logo.
	Unit* launch (const std::string & name_string,const std::string & type_string,const std::string & faction_string,const std::string & unittype, const std::string & ai_string,int nr_of_ships,int nr_of_waves, QVector pos, const std::string & sqadlogo);
	///this gets a random cargo type (useful for cargo missions) from either any category if category is '' or else from a specific category  'Contraband'  comes to mind!
	Cargo getRandCargo(int quantity, const std::string & category);
	///this gets the current time in seconds
	float GetGameTime ();
	///this sets the time compresison value to zero
	void SetTimeCompression ();
	///this adds a playlist to the music and may be triggered with an int
	int musicAddList(const std::string & str);
	///this plays a specific song
	void musicPlaySong(const std::string & str);
	///this plays msuci from a given list (where the int is what was returned by musicAddList)
	void musicPlayList(int which);
	///this plays msuci from a given list (where the int is what was returned by musicAddList)
	void musicLoopList(int numloops);
	///this plays a sound at a location...if the sound has dual channels it will play in the center
	void playSound(const std::string & soundName, QVector loc, Vector speed);
	///this plays an image (explosion or warp animation) at a location
	void playAnimation(const std::string & aniName, QVector loc, float size);
	void playAnimationGrow(const std::string & aniName, QVector loc, float size,float growpercent);
	///this gets the player belonging to this mission
	Unit *getPlayer();
	///this gets a player number (if in splitscreen mode)
	Unit *getPlayerX(int which);
	void StopAllSounds(void);
	unsigned int getCurrentPlayer();
	///this gets the number of active players
	int getNumPlayers ();
	int maxMissions ();
	bool networked ();
	bool isserver ();
	// Forwards this request onto the server if this is a client, or send back to client.
	void sendCustom (int cp, const std::string & cmd, const std::string & args, const std::string & id);
	// Executes a python script
	void receivedCustom (int cp, bool trusted, const std::string & cmd, const std::string & args, const std::string & id);
	
	std::string getVariable(const std::string & section,const std::string & name,const std::string & def);
	std::string getSubVariable(const std::string & section,const std::string & subsection,const std::string & name,const std::string & def);
	double timeofday ();
	double queryTimeofday ();
	double sqrt (double);
	double log (double);
	double exp (double);
	double cos (double);
	double sin (double);
	double acos (double);
	double asin (double);
	double atan (double);
	double tan (double);
	void micro_sleep(int n);
	void addParticle (QVector loc, Vector velocity, Vector color, float size);

	void    ComputeGalaxySerials( std::vector<std::string> & stak);
	void    ComputeSystemSerials( std::string & systempath);

	std::string getSaveDir();
    std::string getGuiLabelWithoutEscapes(const std::string & label);
    std::string getEscapedGuiLabel(const std::string & str);
	std::string getSaveInfo(const std::string &filename, bool formatForTextbox);
	std::string getCurrentSaveGame();
	std::string setCurrentSaveGame(const std::string &newsave);
	std::string getNewGameSaveName();
	bool loadGame(const std::string &savename);
	bool saveGame(const std::string &savename);

	// Splash stuff
	void showSplashScreen(const std::string &filename);
	void showSplashMessage(const std::string &text);
	void showSplashProgress(float progress);
	void hideSplashScreen();
	bool isSplashScreenShowing();

	// Defined in gamemenu.cpp for vegastrike, universe_util_server.cpp for vegaserver.
	void startMenuInterface(bool firsttime, const std::string & alert=std::string()); // If game fails, bring it back to a common starting point.

    //keyboard repeat mngt
    void enableKeyRepeat();
    bool restoreKeyRepeat();

    //logging
    int Log(const std::string & module, unsigned int level, const std::string & message);
    unsigned int LogLevel(const std::string & module, bool store = true);
    std::string LogFile(const std::string & module);
    int LogPrint(const std::string & message, const std::string & module);

}


#undef activeSys
#endif
