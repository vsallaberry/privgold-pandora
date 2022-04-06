
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef HAVE_PYTHON
#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include <compile.h>
#include <eval.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#endif
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#if defined (_MSC_VER) && _MSC_VER<=1200
#define Vector Vactor
#endif
#include <boost/python.hpp>
#include <boost/python/converter/from_python.hpp>
#if defined (_MSC_VER) && _MSC_VER<=1200
#undef Vector
#endif
#else
#include <boost/python/class_builder.hpp>
#include <boost/python/detail/extension_class.hpp>
#endif
#include "configxml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "init.h"
#include "python_compile.h"
#include "python_class.h"
#include "cmd/unit_generic.h"
#include "log.h"
#include "vs_log_modules.h"
#include "vsfilesystem.h"
#include "common/common.h"

class Unit;
//FROM_PYTHON_SMART_POINTER(Unit)
#ifdef OLD_PYTHON_TEST
     class hello {
		 std::string country;
      public:
         hello(const std::string& country) { this->country = country; }
         virtual std::string greet() const { return "Hello from " + country; }
         virtual ~hello(){VSFileSystem::vs_fprintf (stderr,"NO HELLO %d",this);} // Good practice 
     };

     struct hello_callback : hello
     {
         // hello constructor storing initial self_ parameter
         hello_callback(PyObject* self_, const std::string& x) // 2
             : hello(x), self(self_) {}

         // In case hello is returned by-value from a wrapped function
         hello_callback(PyObject* self_, const hello& x) // 3
             : hello(x), self(self_) {}

         // Override greet to call back into Python
         std::string greet() const // 4
             { return boost::python::callback<std::string>::call_method(self, "greet"); }
             virtual ~hello_callback () {VSFileSystem::vs_fprintf (stderr,"NO CALLBAC %d",this);}
         // Supplies the default implementation of greet
         static std::string default_greet(const hello& self_) //const // 5
             { return self_.hello::greet(); }
      private:
         PyObject* self; // 1
     };

class MyBaseClass {
private:
protected:
	int val;
public:
	MyBaseClass(int set) {val=set;}
	virtual void foo(int set){val=0;}
	virtual int get(){return -4364;}
	virtual ~MyBaseClass() {}
};

class MyDerivedClass : MyBaseClass {
private:
public:
	MyDerivedClass(int set) :MyBaseClass(set) {val=4364;}
	virtual void foo(int set) {val=set;}
	virtual int get(){return val;}
	virtual ~MyDerivedClass() {}
};

/* Simple Python configuration modifier. It modifies the attributes 
in the config_xml DOM; currently only works for variables in section
'data', and doesn't create new variables if necessary */

class PythonVarConfig {
public:
	struct myattr {
		std::string name;
		std::string value;
		myattr(std::string nam,std::string val) {name=nam;value=val;}
	};
	std::vector <myattr> myvar;
	std::string MyGetVariable(std::string name,int &loc) const {
		std::vector<myattr>::const_iterator iter=myvar.begin();
		std::string value="";
		if (iter) {
		while (iter!=myvar.end()) {
			if ((*iter).name==name) {
				value=(*iter).value;
				break;
			}
			++iter;
		}
		}
		return value;
	}
	void setVariable(const std::string& name,const std::string& value) {
		printf("variable %s set to %s.\n",name.c_str(),value.c_str());
		int loc;
		std::string newval=MyGetVariable(name,loc);
		if (newval.empty())
			myvar.push_back(myattr(name,value));
		else
			myvar[loc].value=value;
	}
	std::string getVariable(const std::string& name) const {
		int loc;
		std::string value=MyGetVariable(name,loc);
		if (value.empty())
			value="<UNDEFINED>";
		printf("variable %s is %s\n",name.c_str(),value.c_str());
		return value;
	}
};

class PythonIOString {
public:
	static std::strstream buffer;
	static void write(PythonIOString &self, string data) {
		buffer << data;
	}
};
std::strstream PythonIOString::buffer;

/* Basic mode of operation:
  Define a module_builder per module and class_builders for each 
  class to be exported to python

  <module_builder|class_builder>.def defines a function in the module or class
  <module_builder|class_builder>.add adds a variable to the module or class 

From the boost documentation (I haven't used this yet)

Direct Access to Data Members 
Boost.Python uses the special __xxxattr__<name>__ functionality described above to allow direct access to data members through the following special functions on class_builder<> and extension_class<>: 

def_getter(pointer-to-member, name) // read access to the member via attribute name 
def_setter(pointer-to-member, name) // write access to the member via attribute name 
def_readonly(pointer-to-member, name) // read-only access to the member via attribute name 
def_read_write(pointer-to-member, name) // read/write access to the member via attribute name 

  ( Pointer to member is &Class::member )

  */

#include "cmd/ai/fire.h"
class MyFA : public CommunicatingAI {
public:
	MyFA() :CommunicatingAI(0,0){}
	virtual void Execute() { PYTHON_LOG(logvs::NOTICE, "CommAI"); }
	virtual ~MyFA(){}
};

BOOST_PYTHON_MODULE_INIT(Vegastrike)
{
	/* Create a new module VS in python */
	boost::python::module_builder vs("VS");

/*	boost::python::class_builder<hello,hello_callback> BaseClass(vs, "hello");
	BaseClass.def(boost::python::constructor<std::string>());
	BaseClass.def(hello::greet,"greet",hello_callback::default_greet);
*/
/*    boost::python::class_builder<PythonVarConfig>
        Var(vs, "Var");
	//Define a constructor. To define a constructor with multiple arguments,
	//do <classbuilder_type>.def(boost::python::constructor<type1,type2,...>() 
	Var.def(boost::python::constructor<>());
	
	// Override __getattr__ and __setattr__ so that assignments to unbound variables in 
	//a Var will be redirected to the config assignment functions 
	Var.def(PythonVarConfig::getVariable,"__getattr__");
	Var.def(PythonVarConfig::setVariable,"__setattr__");

*/
	boost::python::class_builder<MyFA> FA(vs, "CommAI");
	FA.def(&MyFA::Execute,"Execute");
	FA.def(boost::python::constructor<>());
	boost::python::class_builder<PythonIOString >
		IO(vs, "IO");
	IO.def(boost::python::constructor<>());
	/* Implement a function that implements the same interface as the write file 
	I/O function in python. This is used to redirect output. A similar technique
	can be used to redirect input */
	IO.def(PythonIOString::write,"write");
}
//boost::python::detail::extension_instance::wrapped_objects

/*Orders::FireAt & from_python(PyObject *p,boost::python::type<Orders::FireAt &>) {
	return from_python(p,boost::python::type<Orders::FireAt &>());
}*/
#endif

static const std::string pretty_python_script(const std::string & pythonscript, size_t maxsize = 50) {
    std::string res = pythonscript;
    for (std::string::iterator it = res.begin(); it != res.end(); ++it) {
        if (*it == '\n') *it = ';';
        else if (*it == '\t') { *it++ = ' '; it = res.insert(it, ' '); }
    }
    return res.substr(0, (res.size() < maxsize ? res.size() : maxsize));
}

static int python_run(const std::string & str) {
	char * temppython = strdup(str.c_str());
	if (PYTHON_LOG(logvs::VERBOSE, "running '%s'...",temppython) <= 0) {
		PYTHON_LOG(logvs::NOTICE, "running '%s'...",pretty_python_script(str).c_str());
	}
	int ret = PyRun_SimpleString(temppython);
	Python::reseterrors();
	free (temppython);
	return ret;
}

#if defined(_WIN32)
# define VS_PYTHON_RELATIVE_MODULES_PATH VSFileSystem::resourcesdir
#endif
#if defined(VS_PYTHON_RELATIVE_MODULES_PATH)
# define VS_PYTHON_RELATIVE_IMPORT_DEFAULT "true"
#else
# define VS_PYTHON_RELATIVE_IMPORT_DEFAULT "false"
#endif

void Python::initpython() {
    std::string moduledir (vs_config->getVariable ("data","python_modules","modules"));
    bool override_python_path
          = XMLSupport::parse_bool (vs_config->getVariable ("python","override_python_path","true"));
    int python_optimized_bytecode
          = XMLSupport::parse_int (vs_config->getVariable ("python","optimized_bytecode","1"));
    bool python_relative_import
          = XMLSupport::parse_bool (vs_config->getVariable ("python","relative_import",VS_PYTHON_RELATIVE_IMPORT_DEFAULT));

    // First, set PYTHONHOME AND PYTHONPATH
    // Find the first the mod dir
    //std::string pydir = VSFileSystem::Rootdir[1]+ VSFS_PATHSEP +moduledir+ VSFS_PATHSEP "builtin";
    // Use builtin '<datadir>/modules/builtin'
    std::string pydir = moduledir + VSFS_PATHSEP "builtin";
    char origpath[16384] = { 0, };

    if (python_relative_import) {
		// The Python and mingw libraries I use have issues with paths containing unicode characters > 255
		// The dirty workaround applied here is to consider all elements of PYTHONPATH(sys.path) as
		// relatives to VSFileSystem::datadir, and patch the python import to make the chdir(datadir) while importing.
    	PYTHON_LOG(logvs::NOTICE, "using relative python import");
		VSCommon::vs_getcwd(origpath, sizeof(origpath)/sizeof(*origpath));
		VSCommon::vs_chdir((VSFileSystem::datadir + VSFS_PATHSEP + pydir).c_str());
		pydir = ".";
    } else {
    	pydir = VSFileSystem::datadir + VSFS_PATHSEP + pydir;
    }

    if (!VSFileSystem::DirectoryExists(pydir)) {
    	PYTHON_LOG(logvs::WARN, "WARNING: the Python Home Path does not exist, "
    			"this could be fatal (check the datadir)");
    	PYTHON_LOG(logvs::WARN, "  missing PYTHONHOME: %s", pydir.c_str());
    }

    PYTHON_LOG(logvs::NOTICE, "override env: %s -> PYTHONHOME:'%s', optimize:%d, relative_import:%s",
    		   override_python_path?"true":"false", pydir.c_str(), python_optimized_bytecode, python_relative_import?"true":"false");

    VSCommon::vs_setenv("PYTHONHOME", pydir.c_str(), override_python_path);
    VSCommon::vs_setenv("PYTHONPATH", pydir.c_str(), override_python_path);
    VSCommon::vs_setenv("PYTHONOPTIMIZE", python_optimized_bytecode > 0
    		                              ? XMLSupport::tostring(python_optimized_bytecode).c_str()
    		                              : "", override_python_path);

    // *********************************************************
    //   I n i t i a l i z e     P y t h o n    l i b r a r y
    // *********************************************************
    Py_Initialize();
    PYTHON_LOG(logvs::NOTICE, "python initialized (optimization = %d)", Py_OptimizeFlag);

    if (python_relative_import) {
    	python_run("import sys, sitecustomize; sys.meta_path.append("
    			      "sitecustomize.RelativeImporter(r'" + VSFileSystem::resourcesdir + "'))");
    	VSCommon::vs_chdir(origpath);
    }
}

void Python::initpaths(){
  // Looking for python lib-dynload (optional modules loaded dynamically)
  std::string pyLibsPath = VS_PATH_JOIN(VSFileSystem::resourcesdir.c_str(), VEGASTRIKE_PYTHON_DYNLIB_PATH);
  PYTHON_LOG(logvs::NOTICE, "PYTHON LIBS PATH: %s", pyLibsPath.c_str());

  std::string moduledir (vs_config->getVariable ("data","python_modules","modules"));
  std::string basesdir (vs_config->getVariable ("data","python_bases","bases"));
  bool python_relative_import = XMLSupport::parse_bool (vs_config->getVariable ("python","relative_import",
		                                                               VS_PYTHON_RELATIVE_IMPORT_DEFAULT));
  std::string pymodpaths;
  std::string modpaths;
  const std::string mods[] = {
	moduledir + VSFS_PATHSEP "builtin",
	moduledir + VSFS_PATHSEP "quests",
	moduledir + VSFS_PATHSEP "missions",
	moduledir + VSFS_PATHSEP "ai",
	moduledir,
	basesdir
  };

  modpaths += pyLibsPath;
  pyLibsPath = "r'" + pyLibsPath + "'";
  if (python_relative_import) {
	  pyLibsPath = "os.path.relpath(" + pyLibsPath + ", r'" + VSFileSystem::resourcesdir + "')";
  }
  pymodpaths += "decode(" + pyLibsPath + ")";
  // Find all the mods dir (ignore homedir)
  for( size_t i=1; i<VSFileSystem::Rootdir.size(); i++)
  {
	  for (size_t imod = 0; imod < sizeof(mods)/sizeof(*mods); ++imod) {
		  std::string dir = VSFileSystem::Rootdir[i]+ VSFS_PATHSEP + mods[imod];
		  if(pymodpaths.size()) pymodpaths += ",";
		  if(modpaths.size()) modpaths += ":";
		  modpaths += dir;
		  dir = "r'" + dir + "'";
		  if (python_relative_import) {
			  dir = "os.path.relpath(" + dir + ", r'" + VSFileSystem::resourcesdir + "')";
		  }
		  pymodpaths += "decode(" + dir + ")";
	  }
  }

  /*
  string::size_type backslash;
  while ((backslash=pymodpaths.find("\\"))!=std::string::npos) {
     pymodpaths[backslash]='/';
  }*/
  //VSCommon::vs_setenv("PYTHONPATH", modpaths.c_str(), 1); // not useful if done after Py_Initialize().

  python_run ("import sys, os.path\nold_syspath = sys.path\n"
		      "def decode(s):\n\ttry:\n\t\treturn s.decode('utf-8')\n\texcept:\n\t\treturn s\n"
		      "sys.path = ["+pymodpaths+"]\n");
}

void Python::reseterrors() {
  if (PyErr_Occurred()) {
    PyErr_Print();
    PyErr_Clear();
    fflush(stderr);
    fflush(stdout);
  }
#ifdef _DEBUG
  fflush(stderr);
#endif
}
/*
//PYTHON_INIT_GLOBALS(VS,UnitContainer);
PYTHON_INIT_GLOBALS(VS,Unit);
PYTHON_BEGIN_MODULE(VS)
PYTHON_BASE_BEGIN_CLASS(VS,UnitContainer,"StoredUnit")
Class.def(boost::python::constructor<Unit*>());
Class.def(&UnitContainer::SetUnit,"Set");
Class.def(&UnitContainer::GetUnit,"Get");
PYTHON_END_CLASS(VS,UnitContainer)
PYTHON_BASE_BEGIN_CLASS(VS,Unit,"Unit")
Class.def(boost::python::constructor<int>());
//Class.def(&UnitContainer::SetUnit,"Set");
//Class.def(&UnitContainer::GetUnit,"Get");
PYTHON_END_CLASS(VS,Unit)
PYTHON_END_MODULE(VS)
TO_PYTHON_SMART_POINTER(Unit) 
*/
#if BOOST_VERSION != 102800
static void* Vector_convertible(PyObject* p) {
	return PyTuple_Check(p)?p:0;
}

static void Vector_construct(PyObject* source, boost::python::converter::rvalue_from_python_stage1_data* data) {
	void* const storage = ((boost::python::converter::rvalue_from_python_storage<Vector>*)data)->storage.bytes;
	new (storage) Vector(0,0,0);
	// Fill in QVector values from source tuple here
	// details left to reader.
	Vector * vec = (Vector *) storage;
	PyArg_ParseTuple(source,"fff",&vec->i,&vec->j,&vec->k);
	data->convertible = storage;
}

static void QVector_construct(PyObject* source, boost::python::converter::rvalue_from_python_stage1_data* data) {
	void* const storage = ((boost::python::converter::rvalue_from_python_storage<QVector>*)data)->storage.bytes;
	new (storage) QVector(0,0,0);
	// Fill in QVector values from source tuple here
	// details left to reader.
	QVector * vec = (QVector *) storage;
	PyArg_ParseTuple(source,"ddd",&vec->i,&vec->j,&vec->k);
	data->convertible = storage;
}
#endif

void Python::init() {

  static bool isinit=false;
  if (isinit) {
    return;
  }
  isinit=true;
// initialize python library
  initpython();
  initpaths();
#if BOOST_VERSION != 102800
  boost::python::converter::registry::insert(Vector_convertible, QVector_construct, boost::python::type_id<QVector>());
	boost::python::converter::registry::insert(Vector_convertible, Vector_construct, boost::python::type_id<Vector>());
#endif
	InitBriefing ();
	InitVS ();
	PYTHON_LOG(logvs::NOTICE, "testing VS debug");
    std::string changepath("import sys\nimport debug\ndebug.debug('previous sys.path = '+str(old_syspath))\n"
                           "debug.debug('sys.path = '+str(sys.path))\n"
                           "import site\ndebug.debug('encoding:'+site.encoding)\n");

	char * temppython = strdup(changepath.c_str());
    if (PYTHON_LOG(logvs::VERBOSE, "running '%s'...",temppython) <= 0) {
        PYTHON_LOG(logvs::NOTICE, "running '%s'...", pretty_python_script(changepath).c_str());
    }
	PyRun_SimpleString(temppython);	
	Python::reseterrors();
	free (temppython);
	InitDirector ();
	InitBase ();
//  InitVegastrike();
}

void Python::test() {

	/* initialize vegastrike module so that 
	'import VS' works in python */

	/* There should be a python script automatically executed right after 
	   initVegastrike to rebind some of the objects into a better hierarchy,
	   and to install wrappers for class methods, etc.

This script should look something like <<EOF
import VS
import sys

#Set up output redirection
sys.stdout = VS.IO()
sys.stderr = sys.stdout

#Make Var look like a nested 'class'
VS.Config.Var = VS.Var()

EOF
  This should be executed with PyRun_SimpleString(char *command)
  See defs.py for some recipes that should go in there

  Other useful Python functions:

PyObject* Py_CompileString(char *str, char *filename, int start) 
   Return value: New reference. 
   Parse and compile the Python source code in str, returning the resulting code object. The start token is given by start; this can be used to constrain the code which can be compiled and should be Py_eval_input, Py_file_input, or Py_single_input. The filename specified by filename is used to construct the code object and may appear in tracebacks or SyntaxError exception messages. This returns NULL if the code cannot be parsed or compiled. 

  This would be the preferred mode of operation for AI scripts
*/

#if 0//defined(WIN32)
	FILE *fp = VSFileSystem::OpenFile("config.py","r");

	freopen("stderr","w",stderr);
	freopen("stdout","w",stdout);
	changehome(true);
	FILE *fp1 = VSFileSystem::OpenFile("config.py","r");
	returnfromhome();
	if (fp1==NULL) {
	  fp1=fp;
	}
	if(fp1!=NULL) {
		PyRun_SimpleFile(fp, "config.py");
		VSFileSystem::Close(fp1);
	}
#endif
#ifdef OLD_PYTHON_TEST
	//CompileRunPython ("simple_test.py");
    //		PyObject * arglist = CreateTuple (vector <PythonBasicType> ());
    //		PyObject * res = PyEval_CallObject(po, arglist);
    //		Py_DECREF(arglist);
		//		Py_XDECREF(res);


		PyRun_SimpleString(
	   "import VS\n"
	   "import sys\n"
	   "sys.stderr.write('asdf')\n"
//	   "VSConfig=VS.Var()\n"
//	   "VSConfig.test='hi'\n"
//	   "print VSConfig.test\n"
//	   "print VSConfig.undefinedvar\n"
//	   "VSConfig.__setattr__('undefined','An undefined variable')\n"
//	   "print VSConfig.__getattr__('undefined')\n"
	   "class MyAI(VS.CommAI):\n"
	   "   def Execute(self):\n"
	   "      sys.stdout.write('MyAI\\n')\n"
	   "hi2 = MyAI()\n"
	   "hi1 = VS.CommAI()\n"
	   "print hi1.Execute()\n"
	   "print hi2.Execute()\n"
	);
#endif
//	char buffer[128];
//	PythonIOString::buffer << endl << '\0';
//	vs_config->setVariable("data","test","NULL");
//	VSFileSystem::vs_fprintf(stdout, "%s", vs_config->getVariable("data","test", string()).c_str());
//	VSFileSystem::vs_fprintf(stdout, "output %s\n", PythonIOString::buffer.str());
	fflush(stderr);
	fflush(stdout);
}

#endif
