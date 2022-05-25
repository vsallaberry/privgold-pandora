#ifndef __SAVE_UTIL_H__
#define __SAVE_UTIL_H__

#include <string>

extern const char *mission_key;
float getSaveData (int whichcp, const std::string & key, unsigned int num);
std::string getSaveString (int whichcp, const std::string & key, unsigned int num);
unsigned int getSaveDataLength (int whichcp, const std::string & key);
unsigned int getSaveStringLength (int whichcp, const std::string & key);
unsigned int pushSaveData (int whichcp, const std::string & key, float val);
unsigned int eraseSaveData (int whichcp, const std::string & key, unsigned int index);
unsigned int pushSaveString (int whichcp, const std::string & key, const std::string & value);
void putSaveString (int whichcp, const std::string & key, unsigned int num, const std::string & val);
void putSaveData (int whichcp, const std::string & key, unsigned int num, float val);
unsigned int eraseSaveString (int whichcp, const std::string & key, unsigned int index);
std::vector <std::string> loadStringList (int playernum,const std::string & mykey);
void saveStringList (int playernum,const std::string & mykey,const std::vector<std::string> & names);
Unit * DockToSavedBases(int playernum, QVector &safevec);
#endif
