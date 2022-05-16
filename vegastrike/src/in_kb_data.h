#ifndef _IN_KB_DATA_H_
#define _IN_KB_DATA_H_
#include <string>
class KBData{
public:
  KBData() : _data(_empty_string) {}
  KBData(const std::string &s) : _data(s.empty() ? _empty_string : new std::string(s)) {}
  KBData(const KBData & data) : _data(data.data().empty() ? _empty_string : new std::string(data.data())) {}
  //KBData & operator=(const KBData & data) { this->return *this; }
  ~KBData() { if (_data !=_empty_string) delete _data; }
  const std::string & data() const { return *_data; };
protected:
  const std::string *   			_data;
  static const std::string * const	_empty_string; // impl. in in_kb.cpp
};
#endif
