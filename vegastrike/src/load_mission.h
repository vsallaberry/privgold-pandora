#ifndef VS_LOAD_MISSION_H
#define VS_LOAD_MISSION_H

void LoadMission (const char *, const std::string &scriptname,bool loadfirst);
void delayLoadMission (const std::string & missionfile);
void delayLoadMission (const std::string & missionfile, const std::string & script);
void processDelayedMissions();
void UnpickleMission(const std::string & pickled);
std::string PickleAllMissions ();
std::string UnpickleAllMissions (FILE *);
std::string UnpickleAllMissions (char * &buf);
std::string PickledDataSansMissionName (const std::string & file);

#endif // ! #ifndef VS_LOAD_MISSION_H
