#ifndef _STARSYSGEN_H_
#define _STARSYSGEN_H_
#include <vector>
#include <string>

/// All the properties from the galaxy in a system.
struct SystemInfo {
  std::string sector;
  std::string name;
  std::string filename;
  float sunradius;
  float compactness;
  int numstars;
  bool nebulae;
  bool asteroids;
  int numun1;
  int numun2;
  std::string faction;
  std::string names;
  std::string stars;
  std::string planetlist;
  std::string smallun;
  std::string nebulaelist;
  std::string asteroidslist;
  std::string ringlist;
  std::string backgrounds;
  std::vector <std::string> jumps;
  int seed;
  bool force;
};

///appends .system
std::string getStarSystemFileName (const std::string &input);
///finds the name after all / characters and capitalizes the first letter
std::string getStarSystemName (const std::string &in);
///finds the name before the first /  this is the sector name
std::string getStarSystemSector (const std::string &in);
std::string getUniversePath ();
void readnames (std::vector <std::string> &entity, const char *filename);
void generateStarSystem (SystemInfo &si);
#endif
