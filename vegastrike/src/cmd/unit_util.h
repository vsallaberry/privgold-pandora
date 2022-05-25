#ifndef __UNIT_UTIL_H__
#define __UNIT_UTIL_H__
#include <string>
#include "unit_generic.h"
#include "images.h"

namespace UnitUtil {
	void setMissionRelevant(Unit* my_unit);
	std::string getFactionName (const Unit *my_unit);
	int getFactionIndex (const Unit *my_unit);
	void setFactionIndex (Unit *my_unit,int factionname);
	void RecomputeUnitUpgrades(Unit *my_unit);
	void setFactionName (Unit *my_unit,const std::string & factionname);
	float getFactionRelation (const Unit *my_unit, const Unit *their_unit);
	float getRelationToFaction (const Unit *my_unit, int other_faction);
	float getRelationFromFaction (const Unit *their_unit, int my_faction);
	std::string getName(const Unit *my_unit);
	void setName(Unit *my_unit,const std::string & name);
	void SetHull(Unit *my_unit,float hull);
    std::string getFlightgroupName(const Unit *my_unit);
	const std::string& getFlightgroupNameCR(const Unit *my_unit);
	Unit *getFlightgroupLeader (Unit *my_unit);
	void orbit (Unit * my_unit, Unit * orbitee, float speed, QVector R, QVector S, QVector center); 
	bool setFlightgroupLeader (Unit *my_unit, Unit *un);
	std::string getFgDirective(const Unit *my_unit);
	bool setFgDirective(Unit *my_unit,const std::string & inp);
	int getPhysicsPriority(Unit * un);
	int getFgSubnumber(const Unit *my_unit);
	int removeCargo(Unit *my_unit,const std::string & s, int quantity, bool erasezero);
        bool repair (Unit * my_unit);
        int removeWeapon(Unit *my_unit, const std::string & weapon,int mountoffset,bool loop_through_mounts); // -1 tells no weapon removed
	float upgrade(Unit *my_unit, const std::string & file,int mountoffset,int subunitoffset, bool force,bool loop_through_mounts);
	int addCargo (Unit *my_unit,Cargo carg);
	int forceAddCargo (Unit *my_unit,Cargo carg);
	bool incrementCargo(Unit *my_unit,float percentagechange,int quantity);
	bool decrementCargo(Unit *my_unit,float percentagechange);
	float getDistance(const Unit *my_unit,const Unit *un);
	float getSignificantDistance (const Unit *un, const Unit *sig);
	int hasCargo (const Unit * my_unit, const std::string & mycarg);
	Cargo GetCargoIndex (const Unit * my_unit, int index);
	Cargo GetCargo (const Unit *my_unit, const std::string & cargname);
	std::string getUnitSystemFile (const Unit * my_unit);
	float getCredits(const Unit *my_unit);
	void addCredits(const Unit *my_unit,float credits);
	bool isSignificant(const Unit *my_unit);
        bool isCloseEnoughToDock(const Unit *my_unit, const Unit *un);
	bool isCapitalShip(const Unit *my_unit);
	bool isDockableUnit(const Unit *my_unit);
	bool isAsteroid(const Unit *my_unit);
	bool isSun(const Unit *my_unit);
	void switchFg(Unit *my_unit,const std::string & arg);
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood);
	bool commAnimation(Unit *my_unit,const std::string & anim);
	bool JumpTo (Unit * unit, const std::string & system);
	int isPlayerStarship (const Unit * un);
	void setECM (Unit * un, int NewECM);  //short fix
	int getECM (const Unit * un); //short fix
	void setSpeed (Unit * un, float speed);
	Unit *owner (const Unit *un);
	float maxSpeed (const Unit *un);
	float maxAfterburnerSpeed (const Unit *un);
	void performDockingOperations (Unit * un, Unit * unitToDockWith,int actuallyDockP);
        float PercentOperational(Unit * un, const std::string&,const std::string & category,bool countHullAndArmorAsFull);
}

#endif
