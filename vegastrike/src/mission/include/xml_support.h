#ifndef _XML_SUPPORT_H_

#define _XML_SUPPORT_H_
#include <stdio.h>
#include <string>
#ifndef WIN32
#include <strstream>
#endif
#include "hashtable.h"
#include <vector>
#include <expat.h>
#include <iostream>		// needed for cout calls in config_xml.cpp (and other places too i'm sure)


#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

std::string strtoupper(const std::string &foo);

namespace XMLSupport {

  struct Attribute {
    std::string name;
    std::string value;
    Attribute(const std::string & name, const std::string & value) : name(name), value(value)
    {};
  };

  class AttributeList : public std::vector<Attribute> {
  public:
    AttributeList(const XML_Char **atts);
  };
  double parse_float(const std::string &str);
  int parse_int(const std::string &str);
  bool parse_bool (const std::string &str);

  class EnumMap {

    //    static inline double parse_float (const std::string &str) {return ::parse_float (str)};
    Hashtable<std::string,const int, char[1001]> forward;
    Hashtable<std::string,const string, char[1001]> reverse;
  public:

    struct Pair {
      std::string name;
      int val;
	  Pair (const std::string & c, int v) {
		name = c;
		val = v;
	  }
    };
 
    EnumMap(const Pair *data, unsigned int num);


    int lookup(const std::string &str) const;
    const std::string &lookup(int val) const;
  };

  /*
    std::string tostring(int num);
    std::string tostring(float num);
  */
//#ifdef WIN32
  std::string inline tostring (int num) {
	char tmp[256];
	sprintf (tmp,"%d",num);
	return std::string(tmp);
  }
  std::string inline tostring (float num) {
	char tmp[256];
	sprintf (tmp,"%f",num);
	return std::string(tmp);
  }
/*#else
  template<class T> inline std::string tostring(T num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
	
  }
#endif*/
}
#endif
