#ifndef __PYTHON_MISSION_H__
#define __PYTHON_MISSION_H__
#include <string>
#include <vector>
class UnitContainer;
class PythonMissionBaseClass{
protected:
  virtual void Destructor();
public:
  std::vector<UnitContainer*> relevant_units;
  PythonMissionBaseClass ();
  virtual void Destroy(){Destructor();}
  virtual ~PythonMissionBaseClass();
  virtual void Execute ();
  virtual void callFunction (const std::string &);
  virtual std::string Pickle();
  virtual void UnPickle(const std::string &);
};

#endif
