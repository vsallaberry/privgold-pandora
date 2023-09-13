#include "cmd/unit_generic.h"
#include "config_xml.h"
#include "xml_support.h"
//#include "vs_globals.h"
#include "universe_util.h"
#include "gldrv/winsys.h"
#include "command.h"
#include "vs_log_modules.h"

//fmin conflicted with c++11 double std::fmin(double,double)
static inline float vs_fminf(float a, float b) { return (a<b)?a:b; };
//fmax conflicted with c++11 double std::fmax(double,double)
static inline float vs_fmaxf(float a, float b) { return (a>b)?a:b; };

class ShipCommands {
		TFunctor *csetkps;
		TFunctor *cleft;
		TFunctor *cright;
		TFunctor *cup;
		TFunctor *cdown;
		TFunctor *cslide;
		TFunctor *cfire;
		TFunctor *croll;
		TFunctor *cpymenu;
		bool broll;
		bool bleft;
		bool bright;
		bool bup;
		bool bdown;
	public:
		ShipCommands() {
			//create some functors, register them with the command interp {{{
			cslide = cright = cdown = cup = cleft = croll = cfire = NULL;
			cpymenu = make_functor(this, &ShipCommands::pymenu);
			CommandInterpretor->addCommand(cpymenu, "pymenu");
			csetkps = make_functor(this, &ShipCommands::setkps);
			CommandInterpretor->addCommand(csetkps, "setspeed");
			// }}}
			// set some local bools false {{{
			broll = false;
			bleft = false;
			bright = false;
			bup = false;
			bdown = false;
			// }}}
			// a test menu {{{
			{
				menu *m = new menu("python test", "This is a test of the menusystem", "\r\n");
				CommandInterpretor->addMenu(m);
				{
					mItem *mi = new mItem;
					mi->autoreprint = true;
					mi->Name.append("1"); //argument to access menu
//					mi->action.append("python"); //adds this to the function  2 call as the argument
					mi->display.append("Python One Line input"); // menu's display name
					mi->func2call.append("python");
					mi->inputbit = true; // set single-line input mode
					mi->selectstring.append("Type a single line of Python"); // call function "Display" with this string
//					mi->predisplay.append("Python");

					CommandInterpretor->addMenuItem(mi);
				}
				{
					mItem *mi = new mItem;
					mi->autoreprint = true; //auto-re-print the
					//menu after this menuitem is finished
					mi->Name.append("2"); //argument to access menu
//					mi->action.append("python"); //adds this to the function  2 call as the argument
					mi->display.append("(Python Multi-Line input)"); // menu's display name
					mi->func2call.append("python"); //call this function when this menuitem is called and input is all recieved, user input is appened with a space, along with the action string if there is one. (string generated: "func2call action userinput")
					mi->inputbit2 = true; // set single-line input mode
					mi->selectstring.append("Type multiple lines of python input. Use <ENTER> on a line ALONE to finish"); //Call function "Display" with this string
//					mi->predisplay.append(""); // this would be called if we wanted to look up a value of something on another object, using this string to do the lookup
					CommandInterpretor->addMenuItem(mi);
				}
			}
			// }}}
		}
		~ShipCommands() {
			CommandInterpretor->remCommand(cpymenu);
			CommandInterpretor->remCommand(csetkps);
		}
		void pymenu();
		void left(bool *isKeyDown);
		void right(bool *isKeyDown);
		void up(bool *isKeyDown);
		void down(bool *isKeyDown);
		void roll(bool *isKeyDown);
		void setkps(const char *in);
};
// these _would_ work if the physics routines polled the ship_commands object
// for these bools..
void ShipCommands::pymenu() {
        std::string response(CommandInterpretor->setMenu("python test"));
        CommandInterpretor->conoutf(response);
}
void ShipCommands::left(bool *isKeyDown) {
	bleft = isKeyDown;
}
void ShipCommands::right(bool *isKeyDown) {
	bright = isKeyDown;
}
void ShipCommands::up(bool *isKeyDown) {
	bup = isKeyDown;
}
void ShipCommands::down(bool *isKeyDown) {
	bdown = isKeyDown;
}
void ShipCommands::roll(bool *isKeyDown) {
	broll = isKeyDown;
}


static ShipCommands *ship_commands=NULL;

void ShipCommands::setkps(const char *in)
{
	if(in == NULL) throw "What speed?";
	float kps = XMLSupport::parse_float(std::string(in));
	Unit *player = UniverseUtil::getPlayer();
	if (player) {
		static float game_speed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
		static bool display_in_meters = XMLSupport::parse_bool (vs_config->getVariable("physics","display_in_meters","true"));
		static bool lie = XMLSupport::parse_bool (vs_config->getVariable("physics","game_speed_lying","true"));
		if (lie)
			kps *= game_speed; else
		kps /= display_in_meters?1.0f:3.6f;

		player->GetComputerData().set_speed = vs_fminf(player->GetComputerData().max_speed(),kps);
	}
}

void InitShipCommands()
{
	if (ship_commands) delete ship_commands;
    if (CommandInterpretor) {
    	ship_commands = new ShipCommands;
    } else {
        CMD_LOG(logvs::ERROR, "cannot create ShipCommands: CommandInterpretor is not created");
        ship_commands = NULL;
    }
}

void UninitShipCommands()
{
	if (ship_commands) delete ship_commands;
	ship_commands = NULL;
}
