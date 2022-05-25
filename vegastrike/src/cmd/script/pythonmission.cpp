#include <Python.h>
#include <math.h>
#include "pythonmission.h"
#include "python/python_compile.h"
#include <string>
#include "pythonmission.h"
#include "python/init.h"
#include <boost/version.hpp>
#if BOOST_VERSION == 102800
#include <boost/python/reference.hpp>
#include <boost/python/callback.hpp>
#endif
#include "vsfilesystem.h"
#include "cmd/container.h"
#include "log.h"

#define PYMISSION_LOG(lvl, ...) VS_LOG("pyMission", lvl, __VA_ARGS__)

PythonMissionBaseClass::PythonMissionBaseClass () {
}
void PythonMissionBaseClass::Destructor () {
	delete this;
}
PythonMissionBaseClass::~PythonMissionBaseClass(){
	for (unsigned int i=0;i<relevant_units.size();++i) {
		relevant_units[i]->SetUnit(NULL);
		delete relevant_units[i];
	}
	relevant_units.clear();
	PYMISSION_LOG(logvs::INFO, "BaseClass Destruct called. If called from C++ this is death %ld (0x%x)",
                  (unsigned long)(size_t)this,(unsigned int)(size_t)this);
}

void PythonMissionBaseClass::Execute () {
}
void PythonMissionBaseClass::callFunction (const std::string &) {
}

std::string PythonMissionBaseClass::Pickle() {
	return std::string();
}

void PythonMissionBaseClass::UnPickle(const std::string & s)  {
}


