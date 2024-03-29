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
  easyDom - easy DOM for expat - written by Alexander Rawass <alexannika@users.sourceforge.net>
*/


#include <expat.h>
//#include "xml_support.h"
#include "easydom.h"

#include <assert.h>	/// needed for assert() calls.

//using XMLSupport::EnumMap;
//using XMLSupport::Attribute;
//using XMLSupport::AttributeList;


using std::cout;
using std::cerr;
using std::endl;

easyDomNode::easyDomNode() : valid(false) {
}

easyDomNode::~easyDomNode() {
    for (std::vector<easyDomNode*>::const_iterator it = subnodes.begin(); it != subnodes.end(); ++it) {
        delete *it;
    }
}

void easyDomNode::set(easyDomNode *_parent,const std::string & _name,const XML_Char **atts ){
  parent=_parent;
  //  attributes=_attributes;

  if(atts!=NULL){
    for(;*atts!=NULL;atts+=2) {
    //    for(AttributeList::const_iterator iter = _attributes->begin(); iter!=_attributes->end(); iter++) {
    //    cout <<  _name << "::" << (*iter).name << endl;
    //    printf("iter=%x *iter=%x\n",iter,*iter);
    //cout << " " << (*iter).name << "=\"" << (*iter).value << "\"" << endl;
#if 0
      att_name.push_back((*iter).name);
      att_value.push_back((*iter).value);
#endif
      //      attribute_map[(*iter).name]=(*iter).value;
      attribute_map[atts[0]]=atts[1];
    }
  }

  name=_name;
}

void easyDomNode::addChild(easyDomNode *child){
  subnodes.push_back(child);
}

std::string easyDomNode::attr_value(const std::string & search_name){
  return attribute_map[search_name];
}

void easyDomNode::printNode(std::ostringstream& out,int recurse_level,int level){
	vsUMap<std::string,std::string>::const_iterator iter;
 //vector<string>::const_iterator iter2;

 out << "<" << name ;

 for(iter = attribute_map.begin(); iter!=attribute_map.end(); iter++){
       out << " " << (*iter).first << "=\"" << (*iter).second << "\"" ;
  }
  out << ">" << endl;

  std::vector<easyDomNode *>::const_iterator siter;
  
  if(recurse_level>0){
    for(siter= subnodes.begin() ; siter!=subnodes.end() ; siter++){
      (*siter)->printNode(out,recurse_level-1,level+1);
    }
  }

  if(!(recurse_level==0 && level==0)){
    out << "</" << name << ">" << endl;
  }
}

void easyDomNode::printNode(std::ostream& out,int recurse_level,int level){
    std::ostringstream oss;
    printNode(oss, recurse_level, level);
    out << oss.str();
}

void easyDomNode::printNode(FILE * out,int recurse_level,int level){
    std::ostringstream oss;
    printNode(oss, recurse_level, level);
    fputs(oss.str().c_str(), out);
}

const char * textAttr="Text_Attr";

