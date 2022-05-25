/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  xml Configuration written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#ifndef _VEGASIMPLECONFIG_H_
#define _VEGASIMPLECONFIG_H_

#include <expat.h>
#include <string>
#include "xml_support.h"
#include "easydom.h"
#include <map>

#define XMLCONFIG_MAKEVERSION(major,minor,rev)  ((major)/1.0 + (minor)/1000.0 + (rev)/1000000.0)

using XMLSupport::AttributeList;

class vColor {
 public:
  std::string name;
  float r,g,b,a;
};


class configNode : public easyDomNode {
 public:
  vColor *color;
};

enum section_t { SECTION_COLOR,SECTION_VAR };

class configNodeFactory : public easyDomFactory<configNode> {
};

class VegaConfig {
 public:
  bool checkVersion(float version);

#define MAX_AXIS 32
  int axis_axis[MAX_AXIS];
  int axis_joy[MAX_AXIS];
  bool axis_inverse[MAX_AXIS];
#define MAX_HATSWITCHES 16
#define MAX_VALUES 12
  float hatswitch[MAX_HATSWITCHES][MAX_VALUES];
  float hatswitch_margin[MAX_HATSWITCHES];
  int hatswitch_axis[MAX_HATSWITCHES];
  int hatswitch_joystick[MAX_HATSWITCHES];

  VegaConfig(const char *configfile);
  virtual ~VegaConfig();

  void getColor(configNode *node, const std::string & name, float color[4],bool have_color=false);
  void getColor(const std::string & section, const std::string & name, float color[4],bool have_color=false);
  void gethColor(const std::string & section, const std::string & name, float color[4],int hexcolor);
  void getColor(const std::string & name, float color[4]) { getColor("default",name,color); };

  std::string getVariable(const std::string & section, const std::string & name, const std::string & defaultvalue);
  std::string getVariable(const std::string & section, const std::string & subsection,
		             const std::string & name, const std::string & defaultvalue);

  configNode *findSection(const std::string & section, configNode *startnode);
  configNode *findEntry(const std::string & name, configNode *startnode);

  void setVariable(configNode *entry, const std::string & value);
  bool setVariable(const std::string & section, const std::string & name, const std::string & value);
  bool setVariable(const std::string & section, const std::string & subsection,
		           const std::string & name, const std::string & value);

  easyDomNode *Variables() { return variables; };
  virtual void bindKeys() {}

  const easyDomNode * Bindings() const { return bindings; }
    
 protected:
  float myversion;

  std::string getVariable(configNode *section, const std::string & name, const std::string & defaultval);

  configNode *variables;
  configNode *bindings;
  configNode *colors;

  std::map<std::string,std::string> map_variables;
  std::map<std::string,vColor> map_colors;

  int hs_value_index;

  //  vector<vColor *> colors;

  bool checkConfig(configNode *node);
  void doVariables(configNode *node);
  void checkSection(configNode *node,enum section_t section_type);
  void checkVar(configNode *node);
  void doSection(const std::string & prefix, configNode *node,enum section_t section_type);
  void doVar(const std::string & prefix, configNode *node);
  void doColors(configNode *node);
  bool checkColor(const std::string & prefix, configNode *node);

  virtual void doBindings(configNode *node) {}
  virtual void checkBind(configNode *node) {}
  virtual void doAxis(configNode *node) {}
  virtual void checkHatswitch(int nr,configNode *node) {}
};

#endif // _VEGACONFIG_H_
