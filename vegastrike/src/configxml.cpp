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

#include <expat.h>
#include "xml_support.h"

//#include "vegastrike.h"
#include <assert.h>


#include "configxml.h"
#include "easydom.h"
#include "log.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

#define CONFIG_LOG(_lvl, ...) VS_LOG("config", _lvl, __VA_ARGS__)

/* *********************************************************** */

VegaConfig::VegaConfig(const char *configfile){

  //configNodeFactory *domf = new configNodeFactory();
  configNodeFactory domf;
  configNode *top=domf.LoadXML(configfile);

  if(top==NULL){
    CONFIG_LOG(logvs::ERROR, "Panic exit - no configuration");
    exit(0);
  }
  if (!top->isValid()) {
    CONFIG_LOG(logvs::ERROR, "Panic exit - invalid xml");
    exit(0);
  }
  //top->walk(0);
  
  variables=NULL;
  colors=NULL;

  if (!checkConfig(top)) {
    CONFIG_LOG(logvs::WARN, "WARNING, configuration could be incomplete");
  }
}

VegaConfig::~VegaConfig()
{
	if( variables!=NULL)
		delete variables;
	if( colors!=NULL)
		delete colors;
	if( bindings!=NULL)
		delete bindings;
}

/* *********************************************************** */

bool VegaConfig::checkVersion(float version) {
	return myversion >= version;
}

bool VegaConfig::checkConfig(configNode *node){
  if(node->Name()!="vegaconfig"){
    CONFIG_LOG(logvs::ERROR, "this is no Vegastrike config file");
    return false;
  }
  std::string versionstr = node->attr_value("version");
  if (versionstr.empty()) {
	  myversion = XMLCONFIG_MAKEVERSION(1,2,0);
  } else {
	  myversion = 0;
	  float factor = 1.0;
	  for (std::string::iterator it = versionstr.begin(); it != versionstr.end(); ) {
		  std::string::iterator tmp = std::find(it, versionstr.end(), '.');
		  myversion += XMLSupport::parse_int(std::string(it, tmp)) / factor;
		  factor *= 1000.0;
		  if (tmp != versionstr.end()) {
			  ++tmp;
		  }
		  it = tmp;
	  }
  }
  CONFIG_LOG(logvs::NOTICE, "config version = %f", myversion);

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);

    if(cnode->Name()=="variables"){
      doVariables(cnode);
    }
    else if(cnode->Name()=="colors"){
      doColors(cnode);
    }
    else if(cnode->Name()=="bindings"){
      bindings=cnode; // delay the bindings until keyboard/joystick is initialized
      //doBindings(cnode);
    }
    else{
      CONFIG_LOG(logvs::WARN, "Unknown tag: %s", cnode->Name().c_str());
    }
  }
  return true;
}

/* *********************************************************** */

void VegaConfig::doVariables(configNode *node){
  if(variables!=NULL){
    CONFIG_LOG(logvs::WARN, "only one variable section allowed");
    return;
  }
  variables=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    checkSection(cnode,SECTION_VAR);
  }
}

/* *********************************************************** */

void VegaConfig::doSection(string prefix, configNode *node, enum section_t section_type){
  string section=node->attr_value("name");
  if(section.empty()){
    CONFIG_LOG(logvs::WARN, "no name given for section");
  }
  
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if(section_type==SECTION_COLOR){
      checkColor(prefix,cnode);
    }
    else if(section_type==SECTION_VAR){
      if(cnode->Name()=="var"){
	doVar(prefix,cnode);
      }
      else if(cnode->Name()=="section"){
	doSection(prefix+cnode->attr_value("name")+"/",cnode,section_type);
      }
      else{
	    CONFIG_LOG(logvs::WARN, "neither a variable nor a section");
      }
    }
  }
}

/* *********************************************************** */

void VegaConfig::checkSection(configNode *node, enum section_t section_type){
    if(node->Name()!="section"){
      CONFIG_LOG(logvs::WARN, "not a section");
      node->printNode(logvs::log_getfile(),0,1);

      return;
  }

    doSection(node->attr_value("name")+"/",node,section_type);
}

/* *********************************************************** */

void VegaConfig::doVar(string prefix, configNode *node){
  string name=node->attr_value("name");
  string value=node->attr_value("value");
  string hashname=prefix+name;
  map_variables[hashname]=value;

  //  cout << "checking var " << name << " value " << value << endl;
  if(name.empty()){
    CONFIG_LOG(logvs::WARN, "no name given for variable '%s=%s'", name.c_str(), value.c_str());
  }
}

/* *********************************************************** */

void VegaConfig::checkVar(configNode *node){
    if(node->Name()!="var"){
      CONFIG_LOG(logvs::WARN, "not a variable");
    return;
  }

    doVar("",node);
}

/* *********************************************************** */

bool VegaConfig::checkColor(string prefix, configNode *node){
  if(node->Name()!="color"){
    CONFIG_LOG(logvs::WARN, "no color definition");
    return false;
  }

  if(node->attr_value("name").empty()){
    CONFIG_LOG(logvs::WARN, "no color name given");
    return false;
  }

  string name=node->attr_value("name");
  string hashname=prefix+name;

  vColor *color;

  if(node->attr_value("ref").empty()){
    string r=node->attr_value("r");
    string g=node->attr_value("g");
    string b=node->attr_value("b");
    string a=node->attr_value("a");
    if(r.empty() || g.empty() || b.empty() || a.empty()){
      CONFIG_LOG(logvs::WARN, "neither name nor r,g,b given for color %s", node->Name().c_str());
      return false;
    }
    float rf=atof(r.c_str());
    float gf=atof(g.c_str());
    float bf=atof(b.c_str());
    float af=atof(a.c_str());

    vColor &vc = map_colors[hashname];
    vc.name.erase();
    vc.r = rf;
    vc.g = gf;
    vc.b = bf;
    vc.a = af;

    color=new vColor;

    color->r=rf;
    color->g=gf;
    color->b=bf;
    color->a=af;
  }
  else{
    float refcol[4];

    string ref_section=node->attr_value("section");
    string ref_name=node->attr_value("ref");
    if(ref_section.empty()){
      CONFIG_LOG(logvs::WARN, "you have to give a referenced section when referencing colors");
      ref_section="default";
    }

    //    cout << "refsec: " << ref_section << " ref " << node->attr_value("ref") << endl;
    getColor(ref_section,ref_name,refcol);

    vColor &vc = map_colors[hashname];
    vc.name = ref_section+"/"+ref_name;
    vc.r = refcol[0];
    vc.g = refcol[1];
    vc.b = refcol[2];
    vc.a = refcol[3];

    color=new vColor;

    color->r=refcol[0];
    color->g=refcol[1];
    color->b=refcol[2];
    color->a=refcol[3];
  }

  color->name=node->attr_value("name");

  node->color=color;
  //  colors.push_back(color);

  return true;
}

/* *********************************************************** */

void VegaConfig::doColors(configNode *node){
  if(colors!=NULL){
    CONFIG_LOG(logvs::WARN, "only one variable section allowed");
    return;
  }
  colors=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    checkSection(cnode,SECTION_COLOR);
  }
}

/* *********************************************************** */

string VegaConfig::getVariable(string section,string subsection,string name,string defaultvalue){
  string hashname = section + "/" + subsection + "/" + name;
  std::map<string,string>::iterator it;
  if ((it=map_variables.find(hashname)) != map_variables.end())
      return (*it).second; else
      return defaultvalue;

  /*configNode *secnode=findSection(section,variables);
  if(secnode!=NULL){
    configNode *subnode=findSection(subsection,secnode);
    if(subnode!=NULL){
      configNode *entrynode=findEntry(name,subnode);
      if(entrynode!=NULL){
	return entrynode->attr_value("value");
      }
    }
  }

  return defaultvalue;*/
}

/* *********************************************************** */

string VegaConfig::getVariable(string section,string name,string defaultval){
  string hashname = section + "/" + name;
  std::map<string,string>::iterator it;
  if ((it=map_variables.find(hashname)) != map_variables.end())
      return (*it).second; else
      return defaultval;

  /*vector<easyDomNode *>::const_iterator siter;
  
  for(siter= variables->subnodes.begin() ; siter!=variables->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return getVariable(cnode,name,defaultval);
    }
  }

  cout << "WARNING: no section named " << section << endl;

  return defaultval;*/
}

/* *********************************************************** */

string VegaConfig::getVariable(configNode *section,string name,string defaultval){
    vector<easyDomNode *>::const_iterator siter;
  
  for(siter= section->subnodes.begin() ; siter!=section->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if((cnode)->attr_value("name")==name){
      return (cnode)->attr_value("value");
    }
  }
  static bool foundshouldwarn=false;
  static bool shouldwarn=true;
  if (!foundshouldwarn) {
    if (name!="debug_config") {
      shouldwarn=XMLSupport::parse_bool(getVariable("general","debug_config","true"));
      foundshouldwarn=true;
    }
  }
  if (shouldwarn) {
    CONFIG_LOG(logvs::WARN, "WARNING: no var named %s in section %s using default: %s", 
               name.c_str(), section->attr_value("name").c_str(), defaultval.c_str());
  }
  return defaultval; 
}

/* *********************************************************** */

void VegaConfig::gethColor(string section, string name, float color[4],int hexcolor){
  color[3]=((float)(hexcolor & 0xff))/256.0;
  color[2]=((float)((hexcolor & 0xff00)>>8))/256.0;
  color[1]=((float)((hexcolor & 0xff0000)>>16))/256.0;
  color[0]=((float)((hexcolor & 0xff000000)>>24))/256.0;
  
  getColor(section,name,color,true);
}

/* *********************************************************** */

void VegaConfig::getColor(string section, string name, float color[4],bool have_color){
  string hashname = section + "/" + name;
  std::map<string,vColor>::iterator it;
  if ((it=map_colors.find(hashname)) != map_colors.end()) {
      color[0] = (*it).second.r;
      color[1] = (*it).second.g;
      color[2] = (*it).second.b;
      color[3] = (*it).second.a;
  } else {
      if (!have_color) {
          color[0]=color[1]=color[2]=color[3]=1.0f;
      }
  }

  /*vector<easyDomNode *>::const_iterator siter;
  
  if( colors == NULL )
  {
      cout << "WARNING: no colors defined in vegatrike config file" << endl;
      return;
  }

  for(siter= colors->subnodes.begin() ; siter!=colors->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //          cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      getColor(cnode,name,color,have_color);
      return;
    }
  }

  cout << "WARNING: no section named " << section << endl;

  return;*/
  
}

/* *********************************************************** */

void VegaConfig::getColor(configNode *node,string name,float color[4],bool have_color){
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    //            cout << "scanning color " << (cnode)->attr_value("name") << endl;
    if((cnode)->attr_value("name")==name){
      color[0]=(cnode)->color->r;
      color[1]=(cnode)->color->g;
      color[2]=(cnode)->color->b;
      color[3]=(cnode)->color->a;
      return;
    }
  }

  if(have_color==false){
    color[0]=1.0;
    color[1]=1.0;
    color[2]=1.0;
    color[3]=1.0;

    CONFIG_LOG(logvs::WARN, "WARNING: color %s not defined, using default (white)", name.c_str());
  }
  else{
    CONFIG_LOG(logvs::WARN, "WARNING: color %s not defined, using default (hexcolor)", name.c_str());
  }

}

/* *********************************************************** */

configNode *VegaConfig::findEntry(string name,configNode *startnode){
  return findSection(name,startnode);
}

/* *********************************************************** */

configNode *VegaConfig::findSection(string section,configNode *startnode){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= startnode->subnodes.begin() ; siter!=startnode->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return cnode;
    }
  }

  CONFIG_LOG(logvs::WARN, "WARNING: no section/variable/color named %s", section.c_str());

  return NULL;
 
  
}

/* *********************************************************** */

void VegaConfig::setVariable(configNode *entry,string value){
      entry->set_attribute("value",value);
}

/* *********************************************************** */

bool VegaConfig::setVariable(string section,string name,string value){
  configNode *sectionnode=findSection(section,variables);
  if(sectionnode!=NULL){
    configNode *varnode=findEntry(name,sectionnode);

    if(varnode!=NULL){
      // now set the thing
      setVariable(varnode,value);
    }
  }
  string hashname = section + "/" + name;
  map_variables[hashname]=value;
  return true;
}


bool VegaConfig::setVariable(string section,string subsection,string name,string value){
  configNode *sectionnode=findSection(section,variables);
  if(sectionnode!=NULL){
    configNode *subnode=findSection(name,sectionnode);
    
	if(subnode!=NULL) {
		configNode *varnode=findEntry(name,subnode);
		if(varnode!=NULL){
			// now set the thing
			setVariable(varnode,value);
		}
	}
  }
  string hashname = section + "/" + subsection + "/" + name;
  map_variables[hashname]=value;
  return true;
}

