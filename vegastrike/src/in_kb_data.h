#ifndef _IN_KB_DATA_H_
#define _IN_KB_DATA_H_

#include <string>

class KBData{
public:
  enum TYPE { TYPE_NONE, TYPE_KB, TYPE_MS, TYPE_JS };
  KBData() 								  : _data(_empty_string) {}
  KBData(const std::string &s) 			  : _data(init_data(s)) {}
  KBData(const KBData & data) 			  : _data(init_data(data.data())) {}
  virtual ~KBData() 					  { destruct(); }
  KBData & operator=(const KBData & data) {
	  if (this == &data) return *this;
	  destruct(); _data = init_data(*data._data); return *this;
  }

  const std::string & data() const 	  	  { return *_data; };
  virtual unsigned int type() const 	  { return TYPE_KB; }
  virtual unsigned int id() const 	  	  { return 0; }

protected:
  const std::string * init_data(const std::string & src) {
	  return src.empty() ? _empty_string : new std::string(src);
  }
  virtual void destruct() { if (_data !=_empty_string) delete _data; }

protected:
  const std::string *   			_data;
  static const std::string * const	_empty_string; // impl. in in_kb.cpp
};

#endif // !_IN_KB_DATA_H_
