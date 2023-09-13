/*
 * Copyright (C) 2005 Rogue - Initial
 * Copyright (C) 2022-2023 Vincent Sallaberry - utf8,Readline-like and python improvements
 */
#include "command.h"
#include <sstream>
#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include "gldrv/winsys.h"
#include "main_loop.h"
#include "vs_random.h"
#include "python/python_class.h"
#include "cmd/base.h"
#include "universe_util.h"
#include "save_util.h"
#include "options.h"
#include "configxml.h"

#ifdef HAVE_SDL
#   include <SDL.h>
#endif

#include "log.h"
#include "vs_log_modules.h"

extern vs_options game_options;

// Introduction Comments {{{
// The {{{ and }}} symbols are VIM Fold Markers.
// They FOLD up the page so a user only needs to see a general outline of the entire huge file
// and be able to quickly get to exactly what part they need.
// It helps when a 300 line function is wrapped up to a one line comment
/*
	Index:
		1) Example Commands
		2) Argument Types
		3) PolyMorphic Behavior
		4) Multiple Arguments
		5) Multiple Command Processors made easy with Inheritence
*/
/* *******************************************************************
1) Example Commands:
class WalkControls {
	public:
		WalkControls() {
 // ****************************** 1
			Functor<WalkControls> *ctalk = new Functor<WalkControls>(this, &WalkControls::talk);
// Please notice the NEW, do _not_ delete this. It will be deleted for
// you when remCommand is run, or when the command processor destructor
// is called :)





			CommandInterpretor->addCommand(ctalk, "say", ARG_1STR); //1 c++ string argument,
			// CommandInterpretor is a global (defined in vs_globals.h or
			// vegastrike.h (don't remember which) and created in main.cpp
 // ******************************* 2
			Functor<WalkControls> *ctalk = new Functor<WalkControls>(this, &WalkControls::talk);
			CommandInterpretor->addCommand(ctalk, "order", ARG_1STRVEC);
// easy way to scroll through arguments to make logical desicions aboot them.
// use std::vector<std::string *>::iterator iter = d->begin();
// and (*(iter))->c_str() or (*(iter))->compare etc. iter++ to go up
// iter-- to go down, and if(iter >= d->end()) to check and see if it's at the end.
  // ******************************* 3
			Functor<WalkControls> *dWalkLeft = new Functor<WalkControls>(this, &WalkControls::WalkLeft);
			CommandInterpretor->addCommand(dWalkLeft, "left", ARG_1BOOL);
			//to use this, there'd need to be a mechanism to bind
			//a single charactor to a full command, then when that
			//charactor is passed alone to execute it should translate it to
			//whatever command it's bound to, then it can pass it to findCommand
			//and call findCommand("left", 0) instead of findCommand("a", 0);
  // ******************************** That's enuf.
	// Full list of supported argument types can be seen in commands.h (bottom)
	// for information about the actual supported callback methods
	// (all the functions below are be compatible, and then some)
	// see functors.h, commandI::fexecute, and the enum at the bottom of command.h
		}
		void talk(std::string &in);
		void order(std::vector<std::string *> *d);
		void WalkLeft(bool KeyIsDown);
		void WalkRight(bool KeyIsDown);
		void Jump(bool KeyIsDown);
		void setGravity(int &amount);
}
Then to get the commands to initiate, simply:
static WalkControls done; // and when the program starts, this variable will be made.
******************************************************************* */

/* *******************************************************************
2) Argument type details

	Usually:
	std::string &entire_command_typed
	std::vector<std::string *> *Commands_Typed_BrokenUp_At_Spaces
	bool is_the_Key_Pressed_Down_Or_Up(True for down, false for up)
	int first argument translated through atoi, or 0 if first word is not entered
	const char * first argument, if there is no first argument, it sends NULL
	const char *, const char * first two arguments, sends NULl if not defined

	are enough.

	If you use std::string &, the name of your command will be at
	string &in[0]; followed by a space

	if you use std::vector<std::string *>* you can use:
	std::vector<std::string *>::iterator iter = in->begin();
	the very first iterator will point to the name of the command entered
	not what the user inputed, findCommand will auto-finish commands entered
	then use in->size() to see how many arguments were passed, and do whatever

	1 Bool is for single keys bound to commands. An external layor to translate
	the number of a key when pressed from sdl to a command is needed to
	use this practically



	A note about const char * types. If the user doesn't sent input, it
	will pass NULL. So if you have two of them, and the user sends no
	input, it will send them both as NULL. If the user types one
	argument, the second one will be NULL, the first will be their argument.
	If your function needs an argument, and NULL is passed, you can safely
	throw an error message, either an std::exception or a const char *
	so you can do: if(argument1 == NULL && argument2 == NULL) throw "I need at least one argument!";
				else if(argument2 == NULL) do_something_with_argument1, or maybe throw an error
				else we_have_both_arg1_and_arg2
	const char *'s are the prefered method, std::exceptions get other error
	text appended to them, and is mostly for detecting bugs in the std library



	If you need to add a new argument type, or callback type, see functors.h
	(Callbacks don't have to be on objects, there just is no support for that
	in the functor class, and can be added as needed to make callbacks to existing
	just do the 4 steps to makeing a new argument type in the functor, ignore the object part, it should be fairly trivial.

	Sometimes this is useful, like with servers when passing a socket arond
	to functions to complete requests.


	To use a return value if support is added, the functor Call method returns
	a void *, which by default is a casted reference to the return_type object
	(see functors.h again) so it can be casted back with the string named "s"
	extracted, which could have data if someone made a function that returned
	an std::string and set it.


)
******************************************************************* */
/* *******************************************************************
(****** 3 ******)
PolyMorphic Behaviors:
If you have the command "left" on the object "flight-mode"
the flight-mode object may always be in memory, but imagine you want to land
on a planet, where the command "left" might need to toggle a different vector
 to get it to perform different physics equations.
You could create a new object: (psuedo)
	class walkOnPlanet  {
		Functor<WOP> *leftCommand;
		walkOnPlanet() {
			leftCommand = new Functor<WOP>(This, &walkOnPlanet::left);
			CommandInterpretor->addCommand(leftCommand, "left"); //adding the second left command will automagically override the first
	}
	~walkOnPlanet() {
		CommandInterpretor->remCommand(leftCommand); //by passing it by pointer we can be assured the right one will be removed, in case commands are added/removed out of order
	}
	void left(bool isDown) {
		perform different ops
	}

then create it, and it will ovverride the existing command with the access word "left"

******************************************************************* */

/*
(***** 4 *****)
 A quick comment on Multiple Arguments
	Imagine you have:
	void MyClass::myFunction(const char *arg1, const char *arg2)

	if you do:
	myFunction "a four word name" "some arguments"
	it will send "a four word name" as arg1,
	and "some arguments" as arg2 (With the quotes edited out.)

	Everything except std::string does this, std::string passess the entire input string
	If you need a quote to pass through the command processor to a function, use \" the same way you'd pass a quote to a variable in c++ ;)


*/
/* ********************
(****** 5 ******)
  Multiple Command Processors:
	Creating a custom command processor to do job X.
	Example abstract usage: Imbedded HTTP servers, objects which can be placed in game that can execute their own commands (then "taken-over" or "possessed", or given somehow to the player to be used)

	Example with psuedo:
	class HTTPserver : public commandI {
		class clients {
			friend class HTTPserver;
			friend class std::vector<clients>;
			clients(int &sock, char *addy) {
				socket = sock;
				ipaddress.append(addy);
			};
			clients(const clients &in) {
				socket = in.socket;
				ipaddress.append(in.ipaddress);
			}
			int socket;
			std::string ipaddress;
		};
		class Packet {
			public:
				int socket;
				char *data;
		};
		public:
		HTTPserver(char *port) : commandI() {
			setupserveronport(atoi(port));
			Functor<HTTPserver> *get = new Functor<HTTPserver>(this, &HTTPserver::GET);
			commandI::addCommand(get, "GET");

			Functor<HTTPserver> *post = new Functor<HTTPserver(this, &HTTPserver::POST);
			commandI::addCommand(post, "POST");


			fork() {
				while(SOMEBREAKVARIABLE)
				runserver();
			}
		}
		std::vector<clients> myclients;
		runServer() {
			bool incfound = socketlayor->listen4new();
			if(incfound) {
				while(clients newclient(socketlayor->getANewOne() ) != NULL)
				{
					myclients.push_back(newclient);
				}
			}
			Packet *incomingpacket = socketlayor->listen();
			if(incomingpacket != NULL) {
				std::string ConvertDataToString(incomingpacket->data);
				std::string buffer;
				for(unsigned int counter = 0; counter < CDTS.size(); counter++)
				{
					if(CDTS[counter] == '\n') {
						commandI::execute(buffer, true, socket);
						buffer.erase();
					} else {
						buffer += CDTS[counter];
					}
				} if(buffer.size() > 0) { //POSTS don't end post data with anything, so if we want to process it, we do it now. Headers all terminate with \r\n.
					commandI::execute(buffer, true, socket);
				}

				delete incomingpacket;
			}
		}
		void GET(std::string &page, int socket) {
			securityformat(page);
			std::string getPage = Utilities.loadFile(page);
			std::string buildHeader(page, getPage.size(), option1, option2, etc);
			socketlayor->send(buildheader, socket);
			socketlayor->send(getPage, socket);
		}
		void POST(std::string &page, int socket) {
			setMode(POST) ;// do whatever we want posts to do on the server
		}
	}
********************* */
// }}}

//Coms object {{{
coms::coms(TFunctor *t_in) {functor = t_in;};
coms::coms(coms *oldCom) {
	if(oldCom->Name.size() > 0)
		Name.append(oldCom->Name);
	functor = oldCom->functor;
};
coms::coms(const coms &in) {
	if(in.Name.size() > 0)
		Name.append(in.Name);
	functor = in.functor;
}
coms::~coms() {
	CMD_DBG(logvs::DBG, "Destroying coms object");
};
// }}}
class HoldCommands;
HoldCommands *rcCMD = 0x0;
bool rcCMDEXISTS = false; //initialize to false
class HoldCommands { //  Hold the commands here{{{
/*
// Large comment about why and what {{{
what:
It creates "procs" objects which hold a pointer to a memory address(of the command processor) and the list of commands
that processor has. It holds this list of "procs" in a global variable, which has a single bool to notify command
interpretors when it needs to be created (So if there are 0 command interpretors in memory while the program is running,
HoldCommands won't take up any space as a running object)

It contains 3 utility functions which take the "this" pointer
addCMD: adds a command and takes a this pointer, if the command processor is not found in HoldCommands vector, it
		creates a new one, adds the command to the new one. Otherwise it adds it to the proper one.
popProc: Pop a processor off the list (Not the usual pop_back type, it actually calls std::vector::erase(iterator))
getProc: returns the procs object owned by the command interpretor calling it

why:
	To support multiple command processors, while still allowing commands to be added to a _Global_ command processor or two
	before they have allocated their internal variables and run their constructors. (If you add something to a vector
	on a global object before the object has initialized, that thing will get lost.)
	class B;
	extern B globalB;
	class A {
		public:
		A() { cout << "A created\n"; globalB.list.push_back(1); };
	}
	class B {
		public:
		B() { cout << "B created\n"; };
		vector<int> list;
	}
	A anA(); //A is defined BEFORE B. Very important
	B globalB();
	int main() {
		cout << globalB.list.size(); // gives me zero. Should be one.
	}
	(Normally, this wouldn't happen, IF we were working with just one file.
	But because we are working with multiple files that can be compiled in pretty much any order
	we have limited control of what order our globals are declared in. So we take advantage of the fact we can
	still run functions on a global object before it's initialized, and initialize what we need by hand at the
	exact time it's needed, which would be at any commandI::addCommand call, at any time.)
// }}} I love folds so I can see only what I need ;) (ViM)
*/
		friend class commandI;
		bool finishmeoff;
		class procs {
			public:
			procs(commandI *processor, coms *initcmd) {
				proc = processor;
				rc.push_back(initcmd);
			}
			procs(const procs &in) {
				procs *blah = const_cast<procs *>(&in);
				proc = blah->proc;
				for(std::vector<coms>::iterator iter = blah->rc.begin(); iter < blah->rc.end(); iter++) rc.push_back((*(iter)));
			}
			~procs() {
				while(rc.size() > 0) rc.pop_back();
			}
			commandI *proc;
			std::vector<coms> rc;
		};
		HoldCommands() {

			if(rcCMD != 0x0) {
				CMD_LOG(logvs::WARN, "Error, there shouldn't be 2 holdCommands objects!");
			}
			rcCMD = this;
			finishmeoff = false;
		}
		std::vector<procs> cmds; //for multiple command processors.
		void addCMD(coms &commandin, commandI *proc2use) {
			bool found = false;
			for(std::vector<procs>::iterator iter = cmds.begin(); iter < cmds.end(); iter++) {
				if((*(iter)).proc == proc2use) {
					found = true;
					(*(iter)).rc.insert((*(iter)).rc.begin(), commandin);
					iter = cmds.end();
				}
			}
			if(!found) {
				procs newproc(&(*proc2use), &commandin);
				cmds.push_back(newproc);
			}
		}
		void popProc(commandI *proc2use) {
			for(std::vector<procs>::iterator iter = cmds.begin(); iter < cmds.end(); iter++ )
			{
				if(proc2use == (*(iter)).proc) {
					cmds.erase(iter);
				};
			};
			if(cmds.size() == 0) finishmeoff = true;
		}
		procs *getProc(commandI *in) {
			for(std::vector<procs>::iterator iter = cmds.begin(); iter < cmds.end(); iter++) {
				if(in == (*(iter)).proc) return (&(*(iter)));
			}
			return NULL;
		}
};
//mmoc initclientobject;

// Formerly RegisterPythonWithCommandInterp f***ingsonofat***w***lioness;

// We use a pointer so we can initialize it in addCommand, which can, and does
// run before the command interpretor constructor, and before all local variables
// on the command interpretor itself might be initialized.

// }}}

// {{{ command interpretor constructor
commandI::commandI() {
	CMD_LOG(logvs::NOTICE, "Command Interpretor Created");
	// {{{ add some base commands

	TFunctor *dprompt = make_functor(this, &commandI::prompt);
    //fill with dummy function.
	dprompt->attribs.hidden = true;
	addCommand(dprompt, "prompt");

	TFunctor *newFunct =make_functor(this, &commandI::dummy);
	newFunct->attribs.hidden = true;
	addCommand(newFunct, "dummy");

	TFunctor *dcommands = make_functor(this, &commandI::pcommands);
	addCommand(dcommands, "commands");

	addCommand(make_functor(this, &commandI::cmd_fflush), "fflush");
	addCommand(make_functor(this, &commandI::cmd_clear), "clear");

	TFunctor *dhelp = make_functor(this, &commandI::help);
	addCommand(dhelp, "help");
	// }}}
	// set some local object variables {{{
	menumode = false;
	immortal = false;
	console = false;
	new RegisterPythonWithCommandInterpreter(this); // mem leak - not cleaned up at end of program.
	// }}}
};
// }}}
// {{{ command interpretor destructor
commandI::~commandI() {
        this->enable(false);
        {
			HoldCommands::procs *findme = rcCMD->getProc(this);
			if(findme->rc.size() > 0) {
		        coms *iter = &findme->rc.back();
		        while(findme->rc.size() > 0) {
						iter = &findme->rc.back();
						delete iter->functor;
	        	        findme->rc.pop_back();
		       	};
			}
		}
		{
			menu *iter;
			while(menus.size() > 0) {
				iter = menus.back();
				delete iter;
				menus.pop_back();
			};
		}
		if(rcCMDEXISTS) {
			rcCMD->popProc(this);
			if(rcCMD->finishmeoff) {
				rcCMDEXISTS = false;
				delete rcCMD;
			}
		}
};
// }}}

// {{{ overriden RText::get_prompt()
std::string commandI::get_prompt(const std::string & beforeCursor, const std::string & afterCursor) {
	return std::string("#FF1100") + (menumode ? menu_in->Name + ":" : ">")
			+ " " + "#FF1100" + beforeCursor + "#000000"+"|" + "#FF1100" + afterCursor;
}
// }}}

// {{{ Menu object destructor
menu::~menu() {
	for(mItem *iter;
		items.size() > 0;) {
			iter = items.back();
			delete iter;
			items.pop_back();
	};
};
// }}}

// {{{ UNFINISHED HELP COMMAND
void commandI::help(const char * helponthis) {
    std::string buf;
    buf.append("Sorry, there is no help system yet\n\r ");
	buf.append("But most commands are self supporting, just type them to see what they do.\n\r");
	if (!helponthis || !*helponthis) {
		buf.append("+ Type  Esc to exit interpreter, 'commands' to have commands list,\n\r");
		buf.append("+ Left/Right/Home/End/ctrl+a,e/Alt+Left|Right to navigate through the prompt,\n\r");
		buf.append("+ Up/Down to navigate through history,\n\r");
		buf.append("+ Ctrl+u/ctrl+k to delete before/after cursor.\n\r");
		buf.append("+ Shift+PgUp|PgDown to resize console, (Ctrl?+)PgUp|PgDown, to scroll.\n\r");
		buf.append("+ ':' is an alias for python (:print 'hello', :1+(3.1*2)), '!' an alias to repeat last command.\n\r");
	}
    conoutf(buf);
};
// }}}
// {{{ send prompt ONLY when 0 charactors are sent with a newline
void commandI::prompt() {
	std::string l;
	l.append("Wooooooooooo\n");
	conoutf(l);
	CMD_DBG(logvs::DBG, "Prompt called :)");
};
// }}}
// {{{ dummy function
void commandI::dummy(std::vector<std::string *> *d) {
    // {{{
	std::string outs("Error: unknown command '" + *(d->front()) + "'. Try: 'commands' or 'help'\n\r");
	conoutf(outs);
	// }}}
}
// }}}
// {{{ flush all FILE *
void commandI::cmd_fflush() {
	if (fflush(NULL) != 0)
		conoutf("! fflush error");
	CMD_DBG(logvs::DBG, "fflush :)");
};
// }}}
// {{{ clear the console
void commandI::cmd_clear() {
	conlines.clear();
	scrollpos = 0;
	CMD_DBG(logvs::DBG, "clear :)");
};
// }}}
//list all the commands {{{
#include <iomanip>
void commandI::pcommands() {
	int x = 0;
	std::ostringstream cmd;
	cmd << "\n\rCommands available:\n\r";
	std::vector<coms>::iterator iter;

	HoldCommands::procs *commands = rcCMD->getProc(this);
	for(iter = commands->rc.begin(); iter < commands->rc.end(); iter++) {
		if(!(*(iter)).functor->attribs.hidden && !(*(iter)).functor->attribs.webbcmd) {
			if((*(iter)).functor->attribs.immcmd == true) {
				if(immortal) {
					if(x != 5)  cmd << std::setiosflags(std::ios::left) <<std::setw(19);
					cmd << (*(iter)).Name.c_str() ;

					x++;
				} //we don't want to add the command if we arn't immortal
			} else {
				if(x != 5) cmd << std::setiosflags(std::ios::left) <<std::setw(10);

				cmd <<(*(iter)).Name.c_str();
				x++;
			}
			if(x == 5)  {

				cmd << "\n\r";
				x = 0;
			}
		}
	}
	if(x != 5) {
			cmd << "\n\r";
	}
	std::string cmd2;
	cmd2.append(cmd.str());
	conoutf(cmd2);
}
// }}}
// {{{ addCommand - Add a command to the interpreter
void commandI::addCommand(TFunctor *com, const char *name){
	CMD_LOG(logvs::INFO, "Adding command: %s", name);
	coms *newOne = new coms(com);
	// See the very bottom of this file for comments about possible optimization
	newOne->Name.append(name);
	//push the new command back the vector.
	if(!rcCMDEXISTS && rcCMD == 0x0) {
		if(rcCMD != 0x0) {
			CMD_LOG(logvs::VERBOSE, "Apparently rcCMD is not 0x0..");
		}
		rcCMD = new HoldCommands();
		rcCMDEXISTS = true;
	};
	rcCMD->addCMD(*newOne, this);
//        rcCMD->rc.push_back(newOne);
};
// }}}
// {{{ Remove a command remCommand(char *name)
void commandI::remCommand(char *name){
	HoldCommands::procs *findme = rcCMD->getProc(this);
	if(findme->rc.size() < 1) return;
	for(std::vector<coms>::iterator iter = findme->rc.begin(); iter < findme->rc.end();iter++) {
		if((*(iter)).Name.compare(name) == 0) {
			CMD_LOG(logvs::INFO, "Removing: %s", name);
			delete (*(iter)).functor;
			findme->rc.erase(iter);
			return;
		}
	}
	CMD_LOG(logvs::WARN, "Error, command %s not removed, "
			             "try using the TFunctor *com version instead. Also, this is case sensitive ;)", name);
}
void  commandI::remCommand(TFunctor *com) {
	HoldCommands::procs *findme = rcCMD->getProc(this);
    if(findme->rc.size() < 1) return;
    for(std::vector<coms>::iterator iter = findme->rc.begin(); iter < findme->rc.end();iter++) {
        if((*(iter)).functor == com) {
        	CMD_LOG(logvs::INFO, "Removing: %s", (*(iter)).Name.c_str());
            delete (*(iter)).functor;
            findme->rc.erase(iter);
            return;
        }
    }
    CMD_LOG(logvs::WARN, "Error, couldn't find the command that owns the memory area: %zx", (size_t) com);
}
// }}}
// {{{ Find a command in the command interpretor
coms *commandI::findCommand(const char *comm, int &sock_in) {
	HoldCommands::procs *findme = rcCMD->getProc(this);
	if(findme->rc.size() < 1) throw "Error, commands vector empty, this shouldn't happen!\n";

	std::ostringstream in_s;

	if(!comm) ;
	else in_s << comm;//this is actually a hack
	//comm shouldn't ever be null if it gets this far.
	//but for some fucking reason it is sometimes..
	std::string name;
	name.append(in_s.str());
	size_t x;

// remove \n and \r's (4 possible network input) {{{
	for(x = name.find(' '); x != std::string::npos; x = name.find(' ', x+1)) name.erase(name.begin()+x);
	for(x = name.find('\n'); x != std::string::npos; x = name.find('\n', x+1)) name.erase(name.begin()+x);
	for(x = name.find('\r'); x != std::string::npos; x = name.find('\r', x+1)) name.erase(name.begin()+x);
// }}}
//if the input is less than one return prompt function{{{
	if(name.size() < 1) {
		std::vector<coms>::iterator iter = findme->rc.begin();
	    bool breaker = true;
	    while(breaker == true) {
	        if(iter >= findme->rc.end()) {iter--; breaker = false;continue;}
    	    else if((*(iter)).Name.compare("prompt") == 0) { return &(*(iter));}
			else iter++;
		}
		return&(*(iter)); //assign testCom to the iterator
	}
// }}}
//transform name (the word in) to lowercase {{{
	bool golower = true;
	if(golower)
		std::transform(name.begin(), name.end(), name.begin(),static_cast < int(*)(int) > (tolower));
// }}}
// Start testing command names against the command entered {{{
	coms *fuzzymatch = NULL;
	std::vector<coms>::iterator iter;
	for(iter = findme->rc.begin();iter < findme->rc.end(); iter++) {
	//set the test variable to the iterator of something in the command vector
		coms &testCom = ((*(iter)));
		//clear the temporary buffer used for holding the name of this command
		std::string temp;
		//define a string to possibly print something to the user
		std::string printer;
		//if the length of the commands name is larger than what was entered {{{
		if(testCom.Name.length() >= name.length() ) {
			//append the size of the command entered of the test commands name
			//to the temporary test string
			temp.append(testCom.Name, 0, name.size());
			//transform the partial name to lowercase
			bool golower = true;
			if(golower)
				std::transform(temp.begin(), temp.end(), temp.begin(),static_cast < int(*)(int) > (tolower));
			//compare them
			if(temp.compare(name) == 0 && name.size() > 0) {
				//they match {{{
				//If it is an immortal command
				bool returnit = true;
				if(testCom.functor->attribs.immcmd == true) {
					//if we are immortal all's good, go on
					if(immortal);
					else {
						//if we arn't immortal move on to the next command
						//this allows commands to have immortal/mortal versions
						//that call different functions.
						returnit = false;
//						iter = findme->rc.begin();
//						iter++;
//						testCom = (*(iter));

					}
				}
				//if it's an immortal command and we are an immortal simply don't return it.
				if(returnit) {
					if(name.size() == testCom.Name.size())
					return &testCom;
					if(fuzzymatch == NULL)
					fuzzymatch = &testCom;
				}
				// }}}
			}
//} }}}
// else {{{}
		//the command entered is larger than the commands length
		//if it's at most 1 larger try shaving off the last 1
		//try fuzzy match
		} else if(testCom.Name.length() < name.length() && testCom.Name.length() >= name.length()-1 ) {
			temp.append(testCom.Name);
			std::string commandentered2;
			commandentered2.append(name, 0, testCom.Name.size());
			//transform them to lowercase
			std::transform(temp.begin(), temp.end(), temp.begin(),static_cast <int(*)(int) > (tolower));
			std::transform(commandentered2.begin(), commandentered2.end(), commandentered2.begin(),static_cast <int(*)(int) > (tolower));
			if(temp.compare(commandentered2) == 0) {
                //they match {{{
                //If it is an immortal command
                bool returnit = true;
                if(testCom.functor->attribs.immcmd == true) {
                    //if we are immortal all's good, go on
                    if(immortal);
                    else {
                        //if we arn't immortal move on to the next command
                        returnit = false;

                    }
                }
                //if it's an immortal command and we are an immortal simply don't return it.
                if(returnit)
					if(fuzzymatch == NULL)
                   fuzzymatch = &testCom;
                // }}}

			}

		}
		// }}}
	}
	if(fuzzymatch != NULL) return fuzzymatch;
// }}}
	iter = findme->rc.begin();
	for(; iter < findme->rc.end(); iter++ ) {
		if((*(iter)).Name.find("dummy") == 0) {
			return &(*(iter));
		}
	}
	//shouldn't get here.
	return NULL;

};
/// }}}
//strips up command, extracts the first word and runs
//findCommand on it,
//then tries to execute the member function.
//If one is not found, it will call commandI::dummy() .
// {{{ Main execute entrace, all input comes in here, this sends it to the menusystem, then in the return at the very last line executes the fexecute function which actually parses and finds commands, if the menusystem allows. This way the menusystem can manipulate user input, ie insert command names into the input to make it go to any function.
bool commandI::execute(std::string *incommand, bool isDown, int sock_in)
{
	int socket = sock_in;
	//use the menusystem ONLY if the sock_in is the same as socket{{{
	{
		if(menumode && sock_in == socket) {
			std::string l;
			std::string y;
			size_t x = incommand->find(" ");
			if(x < std::string::npos) {
				l.append(incommand->substr(0, x));
			} else {
				l.append(incommand->c_str());
			}
			std::string t;
			t.append((*(incommand)));
			if(x < std::string::npos)
				y.append(incommand->substr(x, incommand->size()-1));
			else
				y.append(incommand->c_str());
			if(l.compare("\r\n") == 0) ;
			else {
				size_t lv = l.find("\r");
				while(lv < std::string::npos) {
					l.replace(lv, 1, "");
					lv = l.find("\r");
				}
	            lv = l.find("\n");
	            while(lv < std::string::npos) {
	                l.replace(lv, 1, "");
	                lv = l.find("\n");
	            }
	            lv = y.find("\r");
	            while(lv < std::string::npos) {
	                y.replace(lv, 1, "");
	                lv = y.find("\r");
	            }
	            lv = y.find("\n");
    	        while(lv < std::string::npos) {
	                y.replace(lv, 1, "");
	                lv = y.find("\n");
	            }

			}
			char *name_out = NULL;
			if(l.size() > 0) name_out = (char *)l.c_str();
			if(callMenu(name_out, (char *)y.c_str(), t) ) return false;
			*incommand=std::string();
			incommand->append(t); //t may have changed if we got this far
		}

	}
	// }}}
	return fexecute(incommand, isDown, sock_in);
};
// }}}
//broken up into two execute functions
//the one below is the real execute, the one above uses the menusystem
//it's broken up so the menusystem can call fexecute themself at the right
//time
//Main Execute Function {{{
bool commandI::fexecute(std::string *incommand, bool isDown, int sock_in) {
	size_t ls, y;
	bool breaker = false;

	//************ try to replace erase leading space, CR, LF if there is one
	//eg, someone types: " do_something" instead of "do_something"
	while(breaker == false) {
		ls = incommand->find_first_of(" \r\n");
		if(ls != 0) {
			breaker = true;
		} else {
			incommand->erase(ls, 1);
		}
	}

// Print back what the user typed.. {{{
// .. Sometimes people believe they typed python print "hello world\n"
// (and saw what they typed when they typed it)
// but may have actually typed oython print "hello world\n"
// and don't want to admit it, so they blame the system.
// So the system must sometimes politely tell the user what they typed
		{
			bool printit = false;
			if(menumode) {
//				if(menu_in->selected) {
//					if(menu_in->iselected->inputbit || menu_in->iselected->inputbit2) printit = true;
//				}
			} else if(console) printit = true;
			if(printit) {
				std::string webout(">");
				webout.append(incommand->c_str());
				//webout.append("\n\r");
                std::string::size_type len = webout.find_last_not_of("\n\r");
                if (len != std::string::npos)
                   ++len; 
				CMD_LOG(logvs::NOTICE, "%s", webout.substr(0, len).c_str());
                conoutf(webout);
			}
		}
// }}}

	/*//replace \r\n with a space {{{
	for(y = incommand->find("\r\n"); y != std::string::npos; y = incommand->find("\r\n", y+1)) {
        incommand->replace(y, 2, " ");
    }
	// }}}
	// remove multiple spaces {{{
	for(y = incommand->find("  "); y != std::string::npos; y = incommand->find("  ", y+1)) {
		incommand->replace(y, 1, "");
	}
	// }}}*/

    // {{{ ! to the last command typed
	{
        size_t x = incommand->find("!");
        if(x == 0) {
            incommand->replace(0, 1, lastcommand);
        }
        // }}}
        // {{{ : to python
        x = incommand->find(":");
        if(x == 0) {
            incommand->replace(0, 1, "python ");
        }
    }
    // }}}

	breaker = false; //reset our exit bool

	//done with formatting
	//now make what our std::vector<std::string> {{{
	std::vector<std::string> strvec; //to replace newincommand
								// to reduce data replication by one;
    coms * pTheCommand = NULL;
    {
    	size_t last = 0, next = 0;
    	bool quote = false;
    	bool escape = false;
    	/*
    	//next=incommand->find(" ");
    	for(next = incommand->find("\"\"", 0); (next=incommand->find("\"\"",last),(last!=std::string::npos)); last=(next!=std::string::npos)?next+1:std::string::npos) {
    		if(next < std::string::npos)
    			incommand->replace(next, 2, "\" \""); //replace "" with " "
    	}*/
    	std::string starter("");
    	strvec.push_back(starter);
    	for(std::string::const_iterator scroller = incommand->begin(); scroller < incommand->end(); scroller++)
    	{
    		if (strvec.size() == 2 && pTheCommand == NULL) {
    			try {
    				pTheCommand = findCommand(strvec[0].c_str(), sock_in);
    			} catch(...) {}
    		}
    		bool escape_possible = (pTheCommand == NULL || pTheCommand->functor == NULL || pTheCommand->functor->attribs.escape_chars);
    		if(escape_possible && *scroller == '\\') {
    			escape = true;
    			continue;
    		}
    		if(escape) {
    			if(*scroller == '\"') strvec[strvec.size()-1] += *scroller;
    			continue;
    		}
    		if(escape_possible && *scroller=='\"') {
    			if(quote) {
    				quote = false;
    				if (scroller != incommand->end() && *(scroller+1) == '\"') // replaces the find "\"\"" above
    					incommand->insert(scroller + 1 - incommand->begin(), 1, ' ');
    			} else {
    				quote = true;
    			}
    			continue;
    		}
    		if((strvec.size() == 1 || escape_possible)
    		&& !quote && incommand->find_first_of(" \r\n") == scroller - incommand->begin()) {
    			while (incommand->find_first_of(" \r\n", (size_t)(scroller-incommand->begin()+1)) == scroller-incommand->begin()+1)
    				++scroller;
    			strvec.push_back(starter);
    		    continue;
    		}
    		strvec[strvec.size()-1] += *scroller;
    	}
    }
    // }}}

    {
		// if the last argument is a space, erase it. {{{
        std::vector<std::string>::iterator iter = strvec.end();
        iter--;
        if((*(iter)).compare(" ") == 0) {
            strvec.erase(iter);
        }
		// }}}
    }

    if (CMD_LOG_START(logvs::INFO, "command arguments: ") > 0) {
    	for (size_t i = 0; i < strvec.size(); ++i) {
    		logvs::log_printf("<%s> ", strvec[i].c_str());
    	}
    	CMD_LOG_END(logvs::INFO, "");
    }

	try {
		if (pTheCommand == NULL)
			pTheCommand = findCommand(strvec[0].c_str(), sock_in);
		coms & theCommand = *pTheCommand;

//Now, we try to replace what was typed with the name returned by findCommand {{{
//to autocomplete words (EX: translate gos into gossip so the gossip
//command only has to find it's access name and not all possible
//methods of accessing it.)
		if(theCommand.Name.compare("dummy") != 0) {
			size_t x = incommand->find_first_of(strvec[0]);
			if(x != std::string::npos) {
				strvec[0].erase();strvec[0].append( theCommand.Name);
			}

// }}}
			lastcommand.erase();lastcommand.append(*incommand); //set the
		// last command entered - use ! to trigger
		}
		{
			// save the command history into savegame (indeed commands can alter game/saves/...)
			std::string message("interpreter command\\\\" + *incommand + "\\\\");
			UniverseUtil::IOmessage(0, "interpreter", "news", message);
			if (!game_options.news_from_cargolist) {
				for (int i = 0; i < _Universe->numPlayers(); ++i)
					pushSaveString(i, "dynamic_news", std::string("#")+message);
			}
		}
		//Try to execute now {{{
		try {
			//maybe if/else if would be more efficient, if this ever
			//gets really large.
			theCommand.functor->Call(strvec, sock_in, &isDown);
		//try to catch any errors that occured while executing
		} catch(const char *in) {
			std::string l;
			l.append(in);
            CMD_LOG(logvs::NOTICE, "error : %s", in);
			conoutf(l); //print the error to the console
		}catch (std::exception e) {
			std::string l;
			l.append("Command processor: Exception occured: ");
			l.append(e.what());
			CMD_LOG(logvs::NOTICE, "%s", l.c_str());
			l.append("\n\r");
			conoutf(l);
		} catch (...) {
			std::string y;
			y.append("Command processor: exception occurered: Unknown, most likely cause: Wrong Arg_type arguement sent with addCommand.");
			CMD_LOG(logvs::NOTICE, "%s", y.c_str());
			y.append("\n\r");
			conoutf(y);
		}

    // }}}
	} catch(const char *in) { //catch findCommand error
		CMD_LOG(logvs::WARN, "%s", in);
	} catch (std::exception & e) {
		CMD_LOG(logvs::WARN, "Command processor: Exception occured: %s", e.what());
	}
	return true;
}

// }}}

std::string commandI::display(std::string &in) {
	//If the menusystem has a value to display, eg:
	//	Editing User
	//	1) Change Username - Current Name: XXX
	// and XXX is replaced with a value here
	// basically, call: string.replace(xxx,3, display("uname") )
	// then display does:
	// if(in.compare(uname) == 0) return current_mob_editing.Name;
	// The value to pass to display is set when creating a menuitem
	std::string f;
	f.append("FAKE");
	return f;
};

// {{{ menusystem
/* ***************************************
An example of how the menusystem is used:
(the very first menu when a player logs onto the ANT-Engine http://daggerfall.dynu.com:5555/player1/index.html OR telnet://daggerfall.dynu.com:5555 )

    {
    menu *m = new menu("newuser", "Welcome to the <GREEN>ANT<NORM> engine", "\r\n");
    m->autoselect = true; //automatically select a menuitem, MUST BE SET
    m->noescape = true; //no escaping this menu except by forcing it
    addMenu(m); //add the menu to the command processor
    mItem *mi = new mItem; //make a new menuitem
    mi->Name.append(" "); //argument to access menu  //must have a name
    mi->action.append("UNAME "); //adds this to the function 2 call as the argument
    mi->action.append(seccode); //add the security code.
    mi->display.append(" "); // menu's display name
    mi->func2call.append("loginfunc"); //function 2 call
    mi->inputbit = true; // set single-line input mode
    mi->selectstring.append("Enter a username"); //string to display when this menuitem is selected
    addMenuItem(mi);// add the menuitem to the command processor, by default
					// added to the last menu added, can be overredden by passing
					// a menu * pointer as the second argument, eg:
					// addMenuItem(mi, m);
    m->aselect = mi; //this is the menu item to automatically select
    }

*************************************** */
//add a menu {{{
bool commandI::addMenu(menu *menu_in) {
	menus.push_back(menu_in);
	lastmenuadded = menu_in;
	return true;
};
// }}}
// {{{ display menu function
std::string commandI::displaymenu() {
    if(menumode) {
        std::ostringstream ps;
		ps << "#de9a4a" << menu_in->Display << "#000000" << "\n";
        for(std::vector<mItem *>::iterator iter = menu_in->items.begin();
            iter < menu_in->items.end(); iter++) {
            ps << "#00DA00" << (*(iter))->Name << "#000000" << " " << (*(iter))->display;
			if((*(iter))->predisplay.size() > 0)
					 ps << " " << display((*(iter))->predisplay);
			ps << "\n";
        }
        std::string buf;
        buf.append(ps.str());
		if(menu_in->autoselect == true) {
			if(menu_in->selected == true) {
				buf.append(menu_in->iselected->selectstring);
			buf.append(": ");
			}
		} else {
			if(!menu_in->noescape) {

		        buf.append("Use: ");
		        if(menu_in->escape.compare("\r\n") == 0)
		            buf.append("enter");
		        else
		            buf.append(menu_in->escape);
		        buf.append(" to quit: \n" );
			} else {
				buf.append("Enter your selection: \n");
			}
		}
		return buf;
//		conoutf(buf);
	}
	std::string buf;
	buf.append("Error, not in menumode!");
	return buf;
};
// }}}
//menuitem to be appended to the last menu appended, or an existing menu if {{{
//the menu2use is specified
bool commandI::addMenuItem(mItem *mi, menu *menuin) {
	menu *menu2use;
	if(menuin == NULL)
		menu2use = lastmenuadded;
	else
		menu2use = menu_in;
	//if the command isn't found it will return dummy or prompt.
	for(std::vector<menu *>::iterator iter = menus.begin(); iter < menus.end(); iter++) {
		if(menu2use == (*(iter))) {
			menu2use->items.push_back(mi); //doh! :)
			return true;
		}
	}
return false;
};
// }}}
//call a menu with arguements {{{
bool commandI::callMenu(const char *name_in, const char *args_in, std::string &d) {
	//if there is a menu operation return true;
	std::string name;
	if(name_in != NULL)
	name.append(name_in);
//	bool freturnfalse = false; //force return false
	//{{{ if the name_in is the menu_in's escape charactor
		//change the menu_in to the last menu on menustack if there is
		//one, and pop the stack. If there is no menustack, set menumode
		//off.
		if(menumode) {
			if(!menu_in->selected) {
				if(!menu_in->noescape) {
					if(name.compare(menu_in->escape) == 0) {
						if(menustack.size() > 0 ) {
							std::vector<menu *>::iterator iter = menustack.end();
							iter--;
							menu_in = (*(iter));
							menustack.pop_back();
		//					return true;
						} else {
							menu_in = NULL;
							menumode = false;
							return true;
						}
					}
				}
			}
		}
	// }}}
	if(menumode) {
		if(menu_in->selected) {
			// Input mode 1  {{{
			if(menu_in->iselected->inputbit == true && menu_in->iselected->inputbit2 == false) {
				menu_in->selected = false;
				std::string arg;
				arg.append(menu_in->iselected->action);
				std::string funcn;
				funcn.append(menu_in->iselected->func2call);
				std::string dreplace;
				dreplace.append(d);
				d.erase();
				d.append(funcn);
				d.append(" ");
				d.append(arg);
				d.append(" ");
				d.append(dreplace);
			//setMenus {{{
				if(funcn.compare("setMenu") == 0) {
					std::string l;
					l.append(setMenu((char *)arg.c_str()));
					conoutf(l);
					return true;
				}
			// }}}
				size_t ylast = 0, xasd = 0;
				//login function {{{
				if(funcn.compare("loginfunc") == 0) {
					std::vector<std::string *> d_out;
					d.append(" ");
					for(size_t x = d.find("\r\n"); x < std::string::npos; x = d.find("\r\n", x+3)) {
						d.replace(x, 1, " \r\n");
					}
					for(size_t iter= 0; iter < d.size();iter++) {
						if(d[iter]==32) {
							std::string *xs = new std::string();
							xs->append(d.substr(ylast, xasd-ylast));
							ylast = xasd;
							d_out.push_back(xs);
						}
						xasd++;
					}
//					loginfunc(&d_out); //login function
					std::vector<std::string *>::iterator itera = d_out.begin();
					while(d_out.size() > 0 ) {
						std::string *s = (*(itera));
						delete s;
						d_out.erase(itera);
						itera=d_out.begin();
					}
					return true;
				}
			// }}}
			//autoreprint {{{
				if(menu_in->iselected->autoreprint == true) {
						fexecute(&d, true, 0);
						std::string x;
						x.append(displaymenu());
						conoutf(x);
						return true;
				}
		// }}}
				return false;
			}
			// }}}
			//input mode 2 {{{
            if(menu_in->iselected->inputbit == false && menu_in->iselected->inputbit2 == true) {
				//wait until we find an escape seqence alone {{{
				if( name.compare(menu_in->escape) == 0 ) {
					menu_in->selected = false;
					std::string arg;
					arg.append(menu_in->iselected->action);
					std::string funcn;
					funcn.append(menu_in->iselected->func2call);
					d.erase();
					d.append(funcn);
					d.append(" ");
					d.append(arg);
					d.append(" ");
					{
						size_t l = 0;
						bool y = false;
						for(size_t x = menu_in->iselected->menubuf.find("\r\n"); x < std::string::npos; x = menu_in->iselected->menubuf.find("\r\n", x+1)) {
							menu_in->iselected->menubuf.replace(x, 2, "<BR>");
							l = x;
							y = true;
						}
						if(y)
						menu_in->iselected->menubuf.replace(l, 4, ""); //replace the last <BR>
					}
					d.append(menu_in->iselected->menubuf);
					d.append(" ");
					menu_in->iselected->menubuf.erase();

					if(funcn.compare("setMenu") == 0) {
						std::string buf;
						buf.append(setMenu((char *)arg.c_str()));
						conoutf(buf);
						return true;
	                }
					if(funcn.compare("loginfunc") == 0) {
						std::vector<std::string *> d_out;
						d.append(" ");
						for(size_t x = d.find("\r\n"); x < std::string::npos; x = d.find("\r\n", x+1)) {
							d.replace(x, 2, "<BR>");
						}
						size_t ylast = 0, xasd = 0;
						for(size_t iter= 0; iter < d.size();iter++) {
							if(d[iter]==32) {
								std::string *xs = new std::string();
								xs->append(d.substr(ylast, xasd-ylast));
								ylast = xasd;
								d_out.push_back(xs);
							}
						xasd++;
						}
//						loginfunc(&d_out); //login function
						std::vector<std::string *>::iterator itera = d_out.begin();
						while(d_out.size() > 0 ) {
						 	std::string *s = (*(itera));
							delete s;
							d_out.erase(itera);
							itera=d_out.begin();
						}
						return true;
					}
                    if(menu_in->iselected->autoreprint == true) {
						fexecute(&d, true, 0);
                        std::string x;
                        x.append(displaymenu());
                        conoutf(x);
						return true;
                    }

					return false;
				// }}}
				// or we append the input to the buffer  {{{
				} else {
					menu_in->iselected->menubuf.append(d);
				}
				// }}}

                return true;
            }
			// }}}
		}

		// if we don't have anything selected, select one.. {{{
		if(!menu_in->selected) {
			for(std::vector<mItem*>::iterator iter = menu_in->items.begin();
			iter < menu_in->items.end(); iter++)  {
				if((*(iter))->Name.compare(name) == 0) {
					menu_in->selected = true;
					menu_in->iselected = (*(iter));
//					if(menu_in->iselected->predisplay.size() > 0) {
//						display(menu_in->iselected->predisplay);
//					}
					if(menu_in->iselected->inputbit2) {
						std::string buf;
						buf.append(menu_in->iselected->selectstring);
						buf.append("\n\r");
						buf.append("Use: ");
						if(menu_in->escape.compare("\r\n") == 0)
							buf.append("enter");
						else
							buf.append(menu_in->escape);
						buf.append(" to confirm: " );

						conoutf(buf);
					} else if(menu_in->iselected->inputbit) {
 						std::string buf;
						buf.append(menu_in->iselected->selectstring);
						buf.append(": ");
						conoutf(buf);
					}
				}
			}
			if(menu_in->selected) {
				if(!menu_in->iselected->inputbit && !menu_in->iselected->inputbit2)
				{
					menu_in->selected = false;
					std::string arg;
					arg.append(menu_in->iselected->action);
                    std::string funcn;
					funcn.append(menu_in->iselected->func2call);
					std::string dreplace;
					dreplace.append(d);
					d=std::string();
					d.append(funcn);
					d.append(" ");
					d.append(arg);
					d.append(" ");
					d.append(dreplace);
					if(funcn.compare("setMenu") == 0) {
						std::string l;
						l.append(setMenu((char *)arg.c_str()));
						conoutf(l);
						return true;
					}
					return false;
				}
				return true;
			} else {
				if(menu_in->defaultInput) {
					menu_in->selected = true;
					menu_in->iselected = menu_in->idefaultInput;
					execute(&d, true, 0);
					return true;
				}
			}
		}
	// }}}
	}
	if(menumode && !menu_in->selected) {
		//we're in a menu but don't have anything selected {{{
		std::string y;
		y.append(displaymenu());
		conoutf(y);
		return true;
	}
	// }}}
	return false;
};
// }}}

// set a menu {{{
std::string commandI::setMenu(const char *name_in) {
	std::string name;
	name.append(name_in);
	if(name[0] == 32) name.replace(0, 1, "");
	for(std::vector<menu *>::iterator iter = menus.begin();
		iter < menus.end(); iter++ ){
		if((*(iter))->Name.compare(name) == 0) {
			if(!menumode) {
				menumode = true;
			} else {
				menustack.push_back(menu_in);
			}
			menu_in = (*(iter));
			menu_in->selected = false;
			if(menu_in->autoselect == true) {
				menu_in->selected = true;
				menu_in->iselected = menu_in->aselect;
			}
			iter = menus.end();
		}
	}
	return displaymenu();
};
// }}}
void commandI::breakmenu() {
	while(menustack.size() > 0 ) {
		menustack.pop_back();
	}
	menu_in = NULL;
	menumode = false;
};
// }}}

// declared STATIC in command.h
commandI *commandI::current_gui_interpretor = NULL;

// ---------------------------------------------------------------------------
# include <fcntl.h>

#if defined(_WIN32)
# define flockfile(f) //_lock_file(f) // deadlock if used
# define funlockfile(f) //_unlock_file(f) // deadlock if used
# define pipe(fds) _pipe(fds, PIPE_BUF, _O_BINARY)
#else
# //define STREAMWRITER_IO_UNBLOCK
# include <sys/select.h>
#endif

#ifndef PIPE_BUF
# define PIPE_BUF 512
#endif

const StreamWriter::threadid_type StreamWriter::thread_invalid = (threadid_type)-1;

static FILE * open_file(int fd, const char * mode, int buffering, int bufsz) {
	FILE * fp;
	int fc;
	if ((fp = fdopen(fd, mode)) == NULL
		|| setvbuf(fp, NULL, buffering, bufsz) != 0
# if defined(STREAMWRITER_IO_UNBLOCK)
		|| (fc = fcntl(fd, F_GETFL)) == -1
		|| fcntl(fd, F_SETFL, fc | O_NONBLOCK) == -1
# endif
	) {
		CMD_LOG(logvs::WARN, "StreamWriter: error fdopen/setvbuf/fcntl");
	}
	return fp;
}

StreamWriter::StreamWriter(unsigned int _flags)
: flags(_flags), _running(false), tid(thread_invalid) {
	int ret = -1, pipeout[2], pipein[2];
	if (pipe(pipeout) == 0 && (ret = pipe(pipein)) != 0) {
		close(pipeout[0]);
		close(pipeout[1]);
	}
	if (ret == 0) {
		const int buffering = _IOLBF; //_IONBF,_IOLBF,_IOFBF;
		const int bufsz = BUFSIZ; //PIPE_BUF;
		fout_wr = open_file(pipeout[1], "w", buffering, bufsz);
		fout_rd = open_file(pipeout[0], "r", buffering, bufsz);
		fin_wr  = open_file(pipein[1], "w", buffering, bufsz);
		fin_rd  = open_file(pipein[0], "r", buffering, bufsz);
	} else {
		CMD_LOG(logvs::WARN, "%s(): cannot create pipe: %s", __func__, strerror(errno));
		fin_rd = fin_wr = fout_rd = fout_wr = NULL;
	}
}

StreamWriter::StreamWriter(FILE * fin, FILE * fout, unsigned int _flags)
	: flags(_flags), _running(false), tid(thread_invalid),
	  fout_wr(fout), fout_rd(fout), fin_wr(fin), fin_rd(fin) {}

StreamWriter::~StreamWriter() {
	stop();
	if (fout_rd)
		fclose(fout_rd);
	if (fout_wr)
		fclose(fout_wr);
	if (fin_rd)
		fclose(fin_rd);
	if (fin_wr)
		fclose(fin_wr);
}

StreamWriter::threadret_type StreamWriter::thread_body(void * data) {
	StreamWriter * swriter = (StreamWriter *) data;
	FILE * foutrd = swriter->outrd_get();
	FILE * log_file = logvs::log_getfile();

	swriter->_running = true;
	if (foutrd == NULL)
		return (threadret_type)NULL;

	int fd_in = fileno(foutrd);

	/*FILE * finwr = swriter->inwr_get();
	int fd_out = finwr ? fileno(finwr) : -1;*/

#if defined(_WIN32)
	HANDLE hfoutrd = (HANDLE) _get_osfhandle(fd_in);
#else
	fd_set rd_fds, err_fds;
	int fd_max = fd_in + 1;
	//fd_set wr_fds; int fd_max = 1 + (fd_in > fd_out ? fd_in : fd_out);*/
#endif
	char buf[1024];
	size_t off = 0;
	do {
		CMD_DBG(logvs::DBG, "%s(%p): looping fd=%d isCmd:%d", __func__, swriter, fd_in, dynamic_cast<CmdStreamWriter*>(swriter)!=NULL);
#if !defined(_WIN32)
		FD_ZERO(&rd_fds);
		FD_ZERO(&err_fds);
		FD_SET(fd_in, &rd_fds);
		//FD_ZERO(&wr_fds); if (fd_out != -1) FD_SET(fd_out, &wr_fds);
		int ret = select(fd_max, &rd_fds, NULL, &err_fds, NULL);
		CMD_DBG(logvs::DBG, "%s(%p): select ret %d %s", __func__, swriter, ret, ret<0?strerror(errno):"");
		if (ret < 0) {
			if (errno == EINTR) continue ;
			break ;
		} else if (ret == 0) {
			break ;
		}
		if (FD_ISSET(fd_in, &err_fds)) {
			CMD_DBG(logvs::DBG, "%s(%p): fd %d error", __func__, swriter, fd_in);
			break ;
		}
		if (FD_ISSET(fd_in, &rd_fds))
#else
		int ret = WaitForSingleObject(hfoutrd, INFINITE);
		CMD_DBG(logvs::DBG, "%s(%p): WaitForSingleObject ret %d", __func__, swriter, ret);
		if (ret != 0)
			break ;
#endif
		{
			//flockfile(swriter->outwr_get());
			while (1) {
				ssize_t n = read(fd_in, buf + off, sizeof(buf)-1-off);
				CMD_DBG(logvs::DBG, "%s(%p): read at %zu: %zd %s", __func__, swriter, off, n, n<0?strerror(errno):"");
				if (n < 0) {
					if (errno == EINTR)
						continue ;
					swriter->_running = false;
				}
				if (n <= 0)
					break ;
				buf[off+n] = 0;
				if (*buf != 0) {
					std::string s(buf);
					std::string::size_type eol = (swriter->flags & SWF_WAITEOL) == 0 ? 0 : s.find_last_of("\r\n");
					if (eol == std::string::npos && off + n + 1 < sizeof(buf)-1) {
						off += n; // no eol detected but space remaining in buffer, -> wait.
					} else {
						swriter->out(s);
						off = 0;
					}
				}
				if (n < sizeof(buf)-1-off) {
					break ;
				} else {
#if defined(_WIN32)
					if (WaitForSingleObject(hfoutrd, 0) != 0) { // poll
#else
					struct timeval polltv = { 0, 0 };
					if (select(fd_in+1, &rd_fds, NULL, NULL, &polltv) <= 0) { // poll
#endif
						break ;
					}
				}
	 		}
			//funlockfile(swriter->outwr_get());
		}

	} while (swriter->_running);
	if (off > 0) {
		swriter->out(std::string(buf));
	}
	CMD_DBG(logvs::DBG, "%s(%p): end thread", __func__, swriter);
	swriter->_running = false;
	return (threadret_type)NULL;
}

bool StreamWriter::run() {
	int ret;
	if (_running)
		return true;
	if ((flags & SWF_THREAD) == 0) {
		thread_body(this);
		return true;
	}
#if defined(_WIN32)
	DWORD wtid;
	tid = CreateThread(NULL, 0, StreamWriter::thread_body, this, 0, &wtid);
	ret = (tid == INVALID_HANDLE_VALUE) ? -1 : 0;
#else
	ret = pthread_create(&tid, NULL, StreamWriter::thread_body, this);
#endif
	if (ret < 0) {
		CMD_LOG(logvs::WARN, "cannot create StreamWriter thread: %s", strerror(errno));
		return false;
	}
	while (!_running)
		usleep(100);
	return true;
}

void StreamWriter::out(const std::string & s) {
	CMD_LOG(logvs::NOTICE, "StreamWriter::out> %s", s.c_str());
}

void StreamWriter::stop() {
	CMD_DBG(logvs::DBG, "StreamWriter::stop() this=%p", this);
	if (fout_wr) fflush(fout_wr);
	if (fout_rd) fflush(fout_rd);
	if (fin_wr) fflush(fin_wr);
	if (fin_rd) fflush(fin_rd);
	if (_running) {
		usleep(100);
		_running = false;
		if (fout_wr)
			write(fileno(fout_wr), "\0", 1);
	}
	if ((flags & SWF_THREAD) == 0 || tid == (threadid_type)(thread_invalid))
		return ;
#if defined(_WIN32)
	WaitForSingleObjectEx(tid, INFINITE, FALSE);
	CloseHandle(tid);
#else
	pthread_join(tid, NULL);
#endif
	tid = (threadid_type)(thread_invalid);
}

size_t StreamWriter::out_write(const std::string & s) {
	if (fout_wr) {
		fflush(fout_wr);
		ssize_t n = write(fileno(fout_wr), s.c_str(), s.length());
		if (n >= 0)
			return n;
	}
	return 0;
}

size_t StreamWriter::in_write(const std::string & s) {
	if (fin_wr) {
		fflush(fin_wr);
		ssize_t n = write(fileno(fin_wr), s.c_str(), s.length());
		if (n >= 0)
			return n;
	}
	return 0;
}

void CmdStreamWriter::out(const std::string & s) {
	CMD_DBG(logvs::DBG, "CmdStreamWriter::out> %s", s.c_str());
	cmdI.conoutf(s);
}

// {{{ Python object

RegisterPythonWithCommandInterpreter::RegisterPythonWithCommandInterpreter(commandI *addTo)
	: cmdI(addTo),
	  writer(CmdStreamWriter(*cmdI)) {
	TFunctor *l = make_functor(this, &RegisterPythonWithCommandInterpreter::runPy);
	l->attribs.escape_chars = false;
	addTo->addCommand(l, "python");
}

// (vsa) experimental feature to run python console cmds in threads
//#define CMD_PYTHON_THREADS 

#ifdef CMD_PYTHON_THREADS
static PyThreadState *gmstate = NULL, * gtstate = NULL;
static void py_threads_deinit() {
	if (gtstate) {
	        PyEval_AcquireThread(gtstate);
	        gtstate = NULL;
	        Py_Finalize();
	}
}
static void py_threads_init()
{
    if (gtstate)
        return;
    //Py_Initialize();
    PyEval_InitThreads(); // Create (and acquire) the interpreter lock
    gtstate = PyThreadState_Get();
}
typedef struct { RegisterPythonWithCommandInterpreter * _this; CmdStreamWriter * writer; std::string args; } pythread_data_t;
static void * py_thread_body(void *data);
#endif

static int close_py_file(FILE * fp) {
	// do not close the file;
	CMD_DBG(logvs::DBG, "ignoring python close request for %p", fp);
	return 0;
}

//run a python string
void RegisterPythonWithCommandInterpreter::runPy(std::string &argsin) {
#ifdef CMD_PYTHON_THREADS
	int ret;
	pythread_data_t * pydata = new pythread_data_t;// (pythread_data_t*) malloc(sizeof(*pydata));
	if (pydata == NULL)
		return ;
	pydata->_this = this;
	pydata->writer = &this->writer;
	pydata->args = argsin;
	py_threads_init();
	Py_BEGIN_ALLOW_THREADS
# if defined(_WIN32)
	DWORD wtid;
	HANDLE tid = CreateThread(NULL, 0, py_thread_body, pydata, 0, &wtid);
	ret = (tid == INVALID_HANDLE_VALUE) ? -1 : 0;
# else
	pthread_t tid;
	ret = pthread_create(&tid, NULL, py_thread_body, pydata);
# endif
	if (ret != 0) {
		delete pydata;
	}
	usleep(50000);
	Py_END_ALLOW_THREADS
}
static void * py_thread_body(void * data) {
	pythread_data_t * pydata = (pythread_data_t*)data;
	std::string & argsin = pydata->args;
	commandI * cmdI = pydata->_this->interpretor();
	CmdStreamWriter & writer(*pydata->writer);
#endif
	static char stdin_str[] 	= { 's','t','d','i','n',0 };
	static char stdout_str[] 	= { 's','t','d','o','u','t',0 };
	static char stderr_str[] 	= { 's','t','d','e','r','r',0 };
	static char r_str[] 		= { 'r', 0 };
	static char w_str[] 		= { 'w', 0 };
	static const char * main_str= "__main__";
	std::string pyRunString;

	pyRunString.append(argsin); //append the arguments in to the string to run
	size_t x = pyRunString.find("python "); //strip out the name of the command
	//and the first space
	if(x == 0)
		pyRunString.replace(x, 7, ""); //replace here
//this method was copied from somewhere else in the vegastrike source
	//now replace <BR> with \r\n
	{
		size_t x = pyRunString.find("<BR>");
		while(x != std::string::npos) {
			pyRunString.replace(x, 4, "\r\n");
			x = pyRunString.find("<BR>");
		}
	}
	CMD_DBG(logvs::DBG, "pyRunString: %s", pyRunString.c_str());

	char *temppython = strdup(pyRunString.c_str()); //copy to a char *

#ifdef CMD_PYTHON_THREADS
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();

	PyObject * mainmod = PyImport_AddModule(main_str);
	PyObject * globals = PyModule_GetDict(mainmod);
#else
    PyObject * mainmod = PyImport_AddModule(main_str);
    PyObject * globals = PyModule_GetDict(mainmod);
#endif

	Py_INCREF(mainmod);
	Py_INCREF(globals);

	PyObject * pyStdin = PySys_GetObject(stdin_str);
	PyObject * pyStdout = PySys_GetObject(stdout_str);
	PyObject * pyStderr = PySys_GetObject(stderr_str);

	FILE * stdinFile = writer.inrd_get();
	FILE * stdoutFile = writer.outwr_get();

	PyObject * pyNewStdin = PyFile_FromFile(stdinFile, stdin_str, r_str, close_py_file);
	PyObject * pyNewStdout = PyFile_FromFile(stdoutFile, stdout_str, w_str, close_py_file);
	PyObject * pyNewStderr = PyFile_FromFile(stdoutFile, stdout_str, w_str, close_py_file);
	PyFile_IncUseCount((PyFileObject *)pyNewStdin);
	PyFile_IncUseCount((PyFileObject *)pyNewStdout);
	PyFile_IncUseCount((PyFileObject *)pyNewStderr);
	PyFile_SetBufSize(pyNewStdin, 1);//PIPE_BUF); //0:IO_NBF, 1:IO_LBF, >1:IO_FBF
	PyFile_SetBufSize(pyNewStdout, 1);//PIPE_BUF);
	PyFile_SetBufSize(pyNewStderr, 1);//PIPE_BUF);

	PySys_SetObject(stdin_str, pyNewStdin);
	PySys_SetObject(stdout_str, pyNewStdout);
	PySys_SetObject(stderr_str, pyNewStderr);

	/* TODO STDIN with python not supported for themoment */
	if (writer.inrd_get()) close(fileno(writer.inrd_get()));

	writer.run();

	PyObject * pyRes = PyRun_String(temppython, cmdI->getmenumode() ? Py_file_input : Py_single_input, globals, globals);
	fflush(NULL);
	if (pyRes == NULL) {
		PyObject * pyExStr = PyObject_Str(PyErr_Occurred());
		if (pyExStr) {
			PyObject * pyErrType, * pyErrMsg, * pyErrTrace;
			PyErr_Fetch(&pyErrType, &pyErrMsg, &pyErrTrace);
			PyObject * pyErrTypeStr = PyObject_Repr(pyErrType);
			PyObject * pyErrMsgStr = PyObject_Str(pyErrMsg);
			//PyObject * pyErrTraceStr = PyObject_Repr(pyErrTrace);

			cmdI->conoutf(std::string("Exception #ff0000")
										+ PyString_AsString(pyErrTypeStr)
					                    + "#000000: " + PyString_AsString(pyErrMsgStr)
										+ "\r\n");
			writer.out_write("#de9a4a");
			PyTraceBack_Print(pyErrTrace, pyNewStdout);

			Py_DECREF(pyExStr);
			Py_DECREF(pyErrTypeStr);
			Py_DECREF(pyErrMsgStr);
			//Py_DECREF(pyErrTraceStr);
		}
		PyErr_Print();
		PyErr_Clear();
	} else {
		if (! PyObject_TypeCheck(pyRes, Py_TYPE(Py_None))) {
			PyObject * pyResStr = PyObject_Str(pyRes);
			cmdI->conoutf(std::string(PyString_AsString(pyResStr))+"\r\n");
			Py_DECREF(pyResStr);
		}
		Py_DECREF(pyRes);
	}
	writer.stop();
	PySys_SetObject(stdin_str, pyStdin);
	PySys_SetObject(stdout_str, pyStdout);
	PySys_SetObject(stderr_str, pyStderr);

	PyFile_DecUseCount((PyFileObject *)pyNewStdin);
	PyFile_DecUseCount((PyFileObject *)pyNewStdout);
	PyFile_DecUseCount((PyFileObject *)pyNewStderr);
	/*Py_XDECREF(pyNewStdin);
	Py_XDECREF(pyNewStdout);
	Py_XDECREF(pyNewStderr);*/

#if !defined(CMD_PYTHON_THREADS)
	Py_XDECREF(globals);
	Py_XDECREF(mainmod);
#else
	Py_XDECREF(globals);
	Py_XDECREF(mainmod);
	PyGILState_Release(gstate);
	delete pydata;
	free(temppython);
	return NULL;
#endif

	free (temppython); //free the copy char *
}

// }}};

/*---------------------------------------------------------------------------*/
/*!
  New input wrapper for new Command Processor SDL version
  \author  Rogue
  \date    Created:  2005-8-16
*/

/** return NULL if CommandInterpretor is not created/not enabled */
static commandI * commandI_isEnabled_orRestoreGameLoop() {
    commandI * interpretor = commandI::getCurrent();
    if (!interpretor) {
		if (BaseInterface::CurrentBase) {
			BaseInterface::CurrentBase->InitCallbacks();
		} else {
			restore_main_loop();
		}
		return NULL;
	} else if (!interpretor->enabled()) {
        CMD_LOG(logvs::WARN, "Warning: Interpretor was disabled without retoring the mainloop!!");
        interpretor->enable(false);
        return NULL;
    }
    return interpretor;
}

//if(!keypress(event.key.keysym.sym, event.key.state==SDL_PRESSED, event.key.keysym.unicode))
void commandI::keypress(int code, int modifiers, bool released, int x, int y) {
	commandI * interpretor = commandI_isEnabled_orRestoreGameLoop();
    if (!interpretor) {		
		return ;
	}
    if(code==WSK_ESCAPE || (code == 'd' && (modifiers == WSK_MOD_LCTRL || modifiers == WSK_MOD_RCTRL)
                && interpretor->getcurcommand().empty())) {
        interpretor->enable(false);
        return;
    };
    if(code==WSK_RETURN && !released) {
        std::string commandBuf = interpretor->getcurcommand();
        commandBuf.append("\r\n");
        interpretor->execute(&commandBuf, released, 0); //execute console on enter
        //don't return so the return get's processed by
        //interpretor->ConsoleKeyboardI, so it can clear the
        //command buffer
    }
    interpretor->ConsoleKeyboardI(code, modifiers, released);
    return;	

/* Proposed (Would need a couple commands inserted into the command processor
	// one to read a keymap file and one to re-map a single key
	// (and the keymap file would have to be read at startup)
	// struct keym { int code; char * name; char * action; }; or so
	std::vector<KeyMapObject>::iterator iter = keyMapVector.begin();
        while(iter < keyMapVector.end()) {
            keym *tester = &(*(iter));
            if(tester->code == code){
            // lookup in keymap and execute
            if(tester->action)
                    execCommand(tester->action, isdown);
                return true;
            }
            iter++;
        }
    }
*/
}

void commandI::mouse_event(int xint, int yint, int button, int state) {
	commandI * interpretor = commandI_isEnabled_orRestoreGameLoop();
    if (!interpretor) {		
		return ;
	}
    if (state != WS_MOUSE_UP)
        return ;
    int key = WSK_PAGEUP;
    switch(button) {
        case WS_WHEEL_DOWN:
            key = WSK_PAGEDOWN;
        case WS_WHEEL_UP: 
            commandI::keypress(key, WSK_MOD_NONE, true, xint, yint);
            break ;
    }
}

void commandI::enable(bool bEnable) {
	if (bEnable) {
		if (!enabled()) {
			CMD_LOG(logvs::NOTICE, "enabling command interpreter...");
			this->console = true;
            if (commandI::current_gui_interpretor != NULL) {
                CMD_LOG(logvs::WARN, "Warning: Interpretor enable request while another is already using the GUI!!");
            } else {
                commandI::current_gui_interpretor = this;
    			winsys_set_keyboard_func((winsys_keyboard_func_t)&commandI::keypress);
    			winsys_set_kb_mode(WS_UNICODE_FULL, WS_KB_REPEAT_ENABLED_DEFAULT, game_options.kb_repeat_interval,
		        		           (unsigned int *)kbmode_backup+0, kbmode_backup+1, kbmode_backup+2);
            }
		}
	} else {
        if (enabled()) {
		    CMD_LOG(logvs::NOTICE, "disabling command interpreter...");
		    this->console = false;
        }
        if (commandI::current_gui_interpretor == this) {
            commandI::current_gui_interpretor = NULL;
            winsys_set_kb_mode((unsigned int)kbmode_backup[0], kbmode_backup[1], kbmode_backup[2], NULL, NULL, NULL);
	    	if (BaseInterface::CurrentBase) {
		    	BaseInterface::CurrentBase->InitCallbacks();
		    } else {
			    restore_main_loop();
		    }
        }
	}
}

/* ***************************************************************
 Possible Optimizations:

	Optimizations discussed here arn't the tiny little save 2 or 3 cpu ops by reforming a for loop.
	These optimizations may make an impact on very very slow machines, or
	when ram is limited for copying objects, or when certain copies or types
	arn't ever needed.

	Possible optimization for findCommand (small optimization, less game-time copying overhead (after boot, while playing))
		copy a coms object when adding a command to the real command vector, (as it is now in addCommand)

		return a reference to the coms object from findCommand, to avoid
		copying every time a key is pressed or a command is entered.
		(change      coms findCommand   to    coms *findCommand)




	 Possible optimization for the main execute function ( medium optimization, less unneeded allocated variables in the execute function when not needed)
		Move findCommand higher up, before the string vector and 1str array are
		built, and build those depending on the argument type, to avoid
		excessive string copying when it's not needed, such as when not in
		console mode but in game mode, when 1 bool is enough to tell
		the function being called wether the key is pressed or not.
	- This might make it a little more difficult to read the execute function


*************************************************************** */

// footer, leave at bottom
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
