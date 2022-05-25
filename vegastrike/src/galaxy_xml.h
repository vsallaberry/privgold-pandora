#ifndef GALAXY_XML_H_
#define GALAXY_XML_H_

#include <string>
#include <map>
#include <vector>
#include "gfx/vec.h"
#include "vsfilesystem.h"

void ComputeSerials( std::vector<std::string> & stak);

namespace GalaxyXML {
typedef std::map<std::string,std::string> StringMap;
class SubHeirarchy;
class SGalaxy {
protected:
  friend class Galaxy;
  class SubHeirarchy * subheirarchy;
  StringMap data;
  SGalaxy & operator = (const SGalaxy & a);

 public:
  SGalaxy () {subheirarchy=NULL;}
  SGalaxy(const char *configfile);
  SGalaxy(const SGalaxy & g);
  void writeGalaxy(VSFileSystem::VSFile &f) const;
  void writeSector(VSFileSystem::VSFile &f, int tabs, const std::string &sectorType, SGalaxy * planet_types) const;

  void processGalaxy(const std::string &sys);
  void processSystem(const std::string &sys,const QVector &suggested_coordinates);

  ~SGalaxy();
  const std::string& getVariable(const std::vector<std::string> &section, const std::string &name, const std::string &default_value) const;
  const std::string& getRandSystem(const std::string &section, const std::string &default_value) const;
  const std::string& getVariable(const std::string &section, const std::string &name, const std::string &defaultvalue) const;
  const std::string& getVariable(const std::string &section, const std::string &subsection, const std::string &name, const std::string &defaultvalue) const;
  bool setVariable(const std::string &section,const std::string &name, const std::string &value);
  bool setVariable(const std::string &section,const std::string &subsection, const std::string &name, const std::string &value);
  void addSection(const std::vector<std::string> &section);
  void setVariable(const std::vector<std::string> &section, const std::string &name, const std::string &value);
  SubHeirarchy & getHeirarchy();
  const std::string& operator [](const std::string &s) const
  {
	  static std::string empty_string;
	  StringMap::const_iterator it = data.find(s);
	  if (it != data.end())
		  return it->second; else
		  return empty_string;
  }
};
class Galaxy: public SGalaxy {
  SGalaxy * getInitialPlanetTypes();
  SGalaxy *planet_types; 
  SGalaxy & operator = (const SGalaxy & a);
  StringMap initial2name;
  StringMap texture2name;  
  void setupPlanetTypeMaps();
 public:
  
  const std::string& getPlanetNameFromInitial(const std::string &abbrev) const
  {
	  static std::string empty_string;
	  StringMap::const_iterator it = initial2name.find(abbrev);
	  if (it != initial2name.end())
		  return it->second; else
		  return empty_string;
  }
  const std::string& getPlanetNameFromTexture(const std::string &tex) const
  {
	  static std::string empty_string;
	  StringMap::const_iterator it = texture2name.find(tex);
	  if (it != texture2name.end())
		  return it->second; else
		  return empty_string;
  }
  const std::string& getPlanetVariable(const std::string &name, const std::string &defaultvalue) const;
  const std::string& getPlanetVariable(const std::string &planet, const std::string &name, const std::string &defaultvalue) const;
  void writeGalaxy(VSFileSystem::VSFile &f) const;
  SGalaxy * getPlanetTypes();
  bool setPlanetVariable(const std::string &name, const std::string &value);
  void addPlanetSection(const std::vector<std::string> &section);
  bool setPlanetVariable(const std::string &planet, const std::string &name, const std::string &value);
  Galaxy () {subheirarchy=NULL;planet_types=NULL;}
  Galaxy(const char *configfile);
  Galaxy(const SGalaxy & g);

};

class SubHeirarchy : public vsUMap<std::string,class SGalaxy> {};

}

std::string getStarSystemFileName (const std::string &input);
std::string getStarSystemName (const std::string &in);
std::string getStarSystemSector (const std::string &in);
#endif
