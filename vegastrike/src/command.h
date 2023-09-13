/*
 * Copyright (C) 2005 Rogue - Initial
 * Copyright (C) 2022-2023 Vincent Sallaberry - utf8,Readline-like and python improvements
 */
#ifndef COMMANDINTERP
#define COMMANDINTERP
#include <iostream>
#include <string>
#include <vector>
#include "functors.h"
#include "rendertext.h" //menusystem
#include "in.h"
//
//#include "areas.h" //must be at the bottom

#if !defined(_WIN32)
# include <pthread.h>
#else
# include <windows.h>
#endif

//this class is ONLY used by commandI.
//it encapsulates a function pointer, with a name and an ARG_TYPE.
class coms {
        public:
        coms(TFunctor *t_in);
        coms(coms *oldCom);
	coms(const coms &in);
	~coms();

	std::string Name;
	TFunctor *functor;
};
//hmm so how do we do menus. Should they be done right in here?
//two classes, menu class, menuitem class, and a vector of menu's.
//and a vector of menuitems on the menu class. 
class mItem { 
	//no destructor is made for this item, there do not need to be ANY 
	//pointers here.
	public:
	mItem() { inputbit = false; inputbit2 = false; autoreprint = false; };
	std::string Name; //menuitem name
	std::string action; //menuitem action -- arguments for the com
	std::string display; //display txt
			//object all menuitems *should* have but may not.
	std::string func2call; //function 2 call 
//	coms *COM; //don't ever DELETE this pointer!
		//and don't ever use a NEW on it either!
		//it should be an existing coms object in the command
		//vector!!! 
	bool inputbit; // if this menuitem expects input from the user
	bool inputbit2;
	bool autoreprint; //autoreprint the menu after it's called
	std::string menubuf; //holds a buffer to be executed later
			//if inputbit2 is true.
	std::string selectstring; //string to be displayed when selected
	std::string predisplay; //call virtual function Display(std::string &)
				//using this string, IF it's larger than 0.
};
class menu {
	protected:
	void init() {
		selected = false; noescape = false; autoselect = false; defaultInput = false;
		iselected = NULL; idefaultInput = NULL; aselect = NULL;
	}
	public:
	menu() { this->init(); };
	menu(const char *n_in, const char *d_in, const char *e_in) {
		this->init();
		Name.append(n_in);
		Display.append(d_in);
		escape.append(e_in);
	}
	~menu();
	std::string Name; //menuname
	std::string Display; //display string.
	std::vector<mItem *> items; //items on the menu.
	bool selected; //is an item selected?
	mItem *iselected; //selected item in this menu
	std::string escape; //escape string \n\r for enter
	bool noescape; // escaping this menu not allowed
	bool autoselect; //autoselect menuitem below
	mItem *aselect; //item to autoselect
	bool defaultInput; //enable defaultinput mItem below
	mItem *idefaultInput; //if a menu item isn't selected while in a menu
	//and the input isn't a menuitem name, sent it to this menuitem
	//example usage: password menu, don't need to press 1 but you still can
};

class commandI : public RText {
	protected:
		//new command interpretor
//                static std::vector<coms *> commands; //our vector of POINTERS
                        //to encapsulated COM objects. (see top of this file)
		coms *findCommand(const char *comm, int &sock_in);
//		mud *World; //to print to the charactor - server object
		menu *lastmenuadded;
		std::vector<menu *> menus;
		menu *menu_in; //menu currently in
		std::vector<menu *> menustack; //stack menus
				//for nested menus.
		bool menumode;
//		object *player; -- player object
		std::string lastcommand;
		bool console;
		int kbmode_backup[3];
		bool immortal;
        static commandI * current_gui_interpretor;
	public:
		commandI();		
//                commandI(mud *mud_in);
//		commandI(mud *mud_in, object *player_in);
		virtual ~commandI();
		
		std::string get_prompt(const std::string & beforeCursor, const std::string & afterCursor);

        static commandI * getCurrent() { return current_gui_interpretor; }
		static void keypress(int code, int modifiers, bool isDown, int x, int y);
    	static void mouse_event(int xint, int yint, int button, int state);

		bool getmenumode() { return menumode; };
                //creates a coms object, adds it to the command vector
                //if args is not supplied it assumes your function
                //has no arguements (not even a void)
		void help(const char *); //help command, should read a help file
                void addCommand(TFunctor *com, const char *name); //add a downcasted Functor
		void remCommand(char *name);
		void remCommand(TFunctor *com); //use this now if possible		
                void dummy(std::vector<std::string *> *d);// {return;}; //first thing to be added to the vecto
		void prompt();
		void cmd_fflush();
		void cmd_clear();
		void pcommands();//lists all the commands to the socket
                        //commands.at(0), returned by findcommand if nothing els                       //is found.

                bool execute(std::string *incommand, bool isDown, int sock_in = 0);

		bool fexecute(std::string *incommand, bool isDown, int sock_in = 0);
                //so far ONLY void XXX:XXX(bool *) is called even if isDown
                //is false. Everything else is only called when isDown is true.
                //Cube string stuff
                //splits up input by line using strsep() (renamed for microsoft windows compatibility)
                bool execCommand(std::string *string, bool isDown);
		bool addMenu(menu *menu2add);
		bool addMenuItem(mItem *mitem2add, menu * = NULL);
		//the args added at addMenu are appended when the
		//menuitem is called
		bool callMenu(const char *name, const char *args, std::string &s2manip);
		std::string setMenu(const char *name); //force a menu set.
		std::string displaymenu(); //utility to display the current menu
		void breakmenu(); //utility to force a break from all menus
		virtual std::string display(std::string &s); // build parts
		//of the menu, eg, if(s.compare("SHIPNAME") == 0 )return editingship->Name;
		
		bool enabled() { return console; }
		void enable(bool bEnable = true);
};

// ---------------------------------------------------------------------------
class StreamWriter {
public:
#if defined(_WIN32)
	typedef DWORD 		threadret_type;
	typedef HANDLE 		threadid_type;
#else
	typedef void * 		threadret_type;
	typedef pthread_t 	threadid_type;
#   define WINAPI
#endif
	enum SWFlags {
		SWF_NONE	= 0,
		SWF_THREAD	= 1 << 0,
		SWF_WAITEOL	= 1 << 1,
		SWF_DEFAULT	= SWF_THREAD | SWF_WAITEOL
	};
	StreamWriter(unsigned int flags = SWF_DEFAULT);
	StreamWriter(FILE * fin, FILE * fout, unsigned int flags = SWF_DEFAULT);
	virtual ~StreamWriter(); // all child classes must call stop() in their own destructor!!
	FILE * 	outwr_get() const	{ return fout_wr; }
	FILE * 	outrd_get() const	{ return fout_rd; }
	FILE * 	inwr_get() const	{ return fin_wr; }
	FILE * 	inrd_get() const	{ return fin_rd; }
	bool 	running() const		{ return _running; }
	bool 	run();
	void 	stop();
	size_t	out_write(const std::string & s);
	size_t	in_write(const std::string & s);
protected:
	virtual void					out(const std::string & s);
	static threadret_type WINAPI	thread_body(void * data);

	static const threadid_type thread_invalid;
	unsigned int	flags;
	bool 			_running;
	threadid_type 	tid;
	FILE * 			fout_wr, * fout_rd;
	FILE * 			fin_wr,  * fin_rd;
};

// ---------------------------------------------------------------------------
class CmdStreamWriter : public StreamWriter {
public:
	CmdStreamWriter(commandI & _cmdI, unsigned int flags = SWF_DEFAULT)
		: StreamWriter(flags), cmdI(_cmdI) {}
	CmdStreamWriter(commandI & _cmdI, FILE * fin, FILE * fout, unsigned int flags = SWF_DEFAULT)
		: StreamWriter(fin, fout, flags), cmdI(_cmdI) {}
	~CmdStreamWriter() { stop(); } // all child classes must call stop() in their own destructor!!
protected:
	void out(const std::string & s);
	commandI & cmdI;
};
// ---------------------------------------------------------------------------

class RegisterPythonWithCommandInterpreter {
public:
	RegisterPythonWithCommandInterpreter(commandI *cI);
	void runPy(std::string &argsin);
    commandI* interpretor() { return cmdI; }
protected:
	commandI * cmdI;
	CmdStreamWriter writer;
};

enum { ARG_1INT, ARG_NONE, ARG_1STR, ARG_2STR, ARG_1CSTR, ARG_2CSTR, ARG_1CSTRARRAY, ARG_1BOOL, ARG_1STRVEC, ARG_1STRVECSPEC, ARG_1STRSPEC };
// ---------------------------------------------------------------------------

namespace ConsoleKeys {

    extern void BringConsole(const KBData&, KBSTATE a);
 
}
//#include "areas.h"
#endif
