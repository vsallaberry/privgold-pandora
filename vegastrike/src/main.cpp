/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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
//#include <fenv.h>
#include "config.h"
#include "version.h"
#include <Python.h>

#if defined(HAVE_SDL)
# include <SDL.h>
# if ! defined(SDL_WINDOWING)
#  if defined(__APPLE__)
#   undef main
#  elif defined(_WIN32)
#   undef main
#   undef WinMain
#  endif
# endif
#endif
#include "cmd/role_bitmask.h"
#if defined(WITH_MACOSX_BUNDLE)
#import <sys/param.h>
#endif
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#endif
#include "gfxlib.h"
#include "in_kb.h"
#include "lin_time.h"
#include "main_loop.h"
#include "config_xml.h"
#include "cmd/script/mission.h"
#include "audiolib.h"
#include "config_xml.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "gfx/animation.h"
#include "cmd/unit.h"
#include "gfx/cockpit.h"
#include "python/init.h"
#include "savegame.h"
#include "force_feedback.h"
#include "gfx/hud.h"
#include "gldrv/winsys.h"
#include "universe_util.h"
#include "networking/netclient.h"
#include "universe.h"
#include "save_util.h"
#include "gfx/masks.h"
#include "cmd/music.h"
#include "ship_commands.h"
#include "gamemenu.h"
#include "log.h"
#include "vs_log_modules.h"

#include <time.h>
#include <stdio.h>

#ifndef _WIN32
#include <sys/signal.h>
#endif

#if defined (_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#if defined(CG_SUPPORT)
#include "cg_global.h"
#endif

#include "options.h"

extern std::string global_username;
extern std::string global_password;

vs_options game_options;

int nadanixnuthin() {
  float a=0;
  int test=0;
  Delta(a, a);
  flipbit(test,test);
  checkbit(test,test);
  dosetbit(test,test);
  unsetbit(test,test);
  return 0;
}
/*
 * Globals
 */
Universe * _Universe;
// GameUniverse _Universe;
TextPlane *bs_tp=NULL;
char SERVER=0;

// false if command line option --net is given to start without network
static bool ignore_network = true;

enum {
    VPB_NONE        = 0,
    VPB_VERSION     = 1 << 0,
    VPB_ARCH        = 1 << 1,
    VPB_CONFIGURE   = 1 << 2,
    VPB_BUILD       = 1 << 3,
    VPB_LIBS        = 1 << 4,
	VPB_SCM         = 1 << 5,
    VPB_ALL         = 0xffffffff
};
static void vs_print_buildinfo(std::ostream & out, int flags);

void enableNetwork(bool usenetwork) {
	ignore_network = !usenetwork;
}

/*
 * Function definitions
 */

void setup_game_data ( ){ //pass in config file l8r??
  g_game.audio_frequency_mode=4;//22050/16
  g_game.sound_enabled =1;
  g_game.use_textures =1;
  g_game.use_ship_textures =0;
  g_game.use_planet_textures =0;
  g_game.use_sprites =1;
  g_game.use_animations =1;
  g_game.use_videos =1;
  g_game.use_logos =1;
  g_game.music_enabled=1;
  g_game.sound_volume=1;
  g_game.music_volume=1;
  g_game.warning_level=20;
  g_game.capture_mouse=GFXFALSE;
  g_game.y_resolution = 768;
  g_game.x_resolution = 1024;
  g_game.fov=78;
  g_game.MouseSensitivityX=2;
  g_game.MouseSensitivityY=4;

}

VegaConfig * createVegaConfig( char * file)
{
	return new GameVegaConfig( file);
}

extern bool soundServerPipes();
std::string ParseCommandLine(int argc, char ** CmdLine);
int ParseConfigOverrides(int argc, char ** argv, VSFileSystem::ConfigOverrides_type & overrides);
    
void VSExit( int code)
{
    fflush(NULL);
    GAME_LOG(logvs::NOTICE, "About to quit...");
    if (BaseInterface::CurrentBase) {
    	BaseInterface::CurrentBase->Terminate();
    }
    GAME_LOG(logvs::NOTICE, "Loop average : %g", avg_loop);
    GAME_LOG(logvs::NOTICE, "FPS average : %g", avg_fps);
    GAME_LOG(logvs::NOTICE, "Number of Mission instances : %u", active_missions.size());
    unsigned int game_loglvl = logvs::vs_log_level("game");
    if (game_loglvl >= logvs::INFO) {
    	for (size_t i = 0; i < active_missions.size(); ++i) {
    		GAME_LOG(logvs::INFO, "  mission: %s", active_missions[i]->mission_name.c_str());
    	}
    }
    GAME_LOG(logvs::NOTICE, "Message Center messages : %zu",
             mission && mission->msgcenter ? mission->msgcenter->messages.size() : 0);
    if (game_loglvl >= logvs::INFO) {
        for (MessageCenter::last_iterator it = mission->msgcenter->last_begin(); ! it.end(); ++it) {
        	GAME_LOG(logvs::INFO, "  message: (%s>%s) %s",
        			 (*it).from.get().c_str(), (*it).to.get().c_str(), (*it).message.get().c_str());
        }
    }
    Music::CleanupMuzak();
	winsys_exit(code);
}

void cleanup(void)
{
  VSFileSystem::vs_fprintf( stdout, "\n\nLoop average : %g\n\n", avg_loop);
  VSFileSystem::vs_fprintf( stderr, "\n\nLoop average : %g\n\n", avg_loop);
  STATIC_VARS_DESTROYED=true;
  printf ("Thank you for playing!\n");

  // In network mode, we may not do the save since it is useless
  if( _Universe != NULL && Network==NULL)
	  _Universe->WriteSaveGame(true);
#ifdef _WIN32
#if defined (_MSC_VER) && defined(_DEBUG)
  if (!cleanexit) {
    _RPT0(_CRT_ERROR, "WARNING: Vega Strike exit not clean\n");
  }
  return;
#endif
#else
  while (!cleanexit)
    int i=1;
#endif
  if( Network!=NULL)
  {
		cout<<"Number of players"<<_Universe->numPlayers()<<endl;
		for( int i=0; i<_Universe->numPlayers(); i++)
			if( Network[i].isInGame())
				Network[i].logout(false);
		delete [] Network;
  }

#if defined(CG_SUPPORT)
    if (cloak_cg->vertexProgram) cgDestroyProgram(cloak_cg->vertexProgram);
    if (cloak_cg->shaderContext)  cgDestroyContext(cloak_cg->shaderContext);
#endif

  Music::CleanupMuzak();
  winsys_shutdown();
  //    write_config_file();
  AUDDestroy();
  //destroyObjects();
  //Unit::ProcessDeleteQueue();
  //delete _Universe;
    delete [] CONFIGFILE;


}

//Mission *mission;
LeakVector<Mission *> active_missions;

char mission_name[1024];

void bootstrap_main_loop();
void bootstrap_first_loop();
#if defined(WITH_MACOSX_BUNDLE)
//WTF! this causes windowed creation to fail... please justify yourself ;-)  #undef main
#endif
void nothinghappens (unsigned int, unsigned int, bool,int,int) {

}

//int allexcept=FE_DIVBYZERO|FE_INVALID;//|FE_OVERFLOW|FE_UNDERFLOW;
extern void InitUnitTables();
bool isVista=false;

int main( int argc, char *argv[] )
{
	VSFileSystem::ChangeToProgramDirectory(argv[0]);
	CONFIGFILE=0;
	mission_name[0]='\0';
        {
          char pwd[8192]="";
          VSFileSystem::vs_getcwd(pwd,8191);
          pwd[8191]='\0';
          printf (" In path %s\n",pwd);
        }
#ifdef _WIN32
	    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
	isVista=(osvi.dwMajorVersion==6);
printf ("Windows version %d %d\n",osvi.dwMajorVersion,osvi.dwMinorVersion);
#endif
    /* Print copyright notice */
	vs_print_buildinfo(std::cout, VPB_VERSION | VPB_SCM);
	std::cout << "See http://www.gnu.org/copyleft/gpl.html for license details." << std::endl << std::endl;
    /* Seed the random number generator */

    if(benchmark<0.0){
      srand( time(NULL) );
    }
    else{
      // in benchmark mode, always use the same seed
      srand(171070);
    }
    //this sets up the vegastrike config variable
    setup_game_data();
    // loads the configuration file .vegastrike/vegastrike.config from home dir if such exists
    {
      string subdir=ParseCommandLine(argc,argv);

      vs_print_buildinfo(std::cerr, VPB_ALL & (~(VPB_VERSION | VPB_SCM | VPB_CONFIGURE)));
      std::cerr << std::endl;
        
      cerr<<"GOT SUBDIR ARG = "<<subdir<<endl;
      if (CONFIGFILE==0) {
        CONFIGFILE=new char[42];
        sprintf(CONFIGFILE,"vegastrike.config");
      }

      // Config overrides via command line
      VSFileSystem::ConfigOverrides_type overrides;
      if (ParseConfigOverrides(argc,argv,overrides) != 0) {
          exit(1);
      }
      // Specify the config file and the possible mod subdir to play
      // After this, vs_config is available.
      VSFileSystem::InitPaths( CONFIGFILE, subdir, &overrides);
    }
    
    // setup log file
    std::string logfile = UniverseUtil::LogFile("");
    if (logfile == "") {
        logvs::vs_log_setfile(NULL);
    } else if (logfile == "stdout") {
        logvs::vs_log_setfile(stdout);
    } else{
        logvs::vs_log_setfile(stderr); // currently only stdout,stderr and none supported
    }
    std::string loglocation = vs_config->getVariable("log", "location", "false");
    logvs::vs_log_setflag(logvs::F_LOCATION_MASK, false);
    if (loglocation == "header") {
        logvs::vs_log_setflag(logvs::F_LOCATION_HEADER, true);
    } else if (loglocation == "footer") {
        logvs::vs_log_setflag(logvs::F_LOCATION_FOOTER, true);
    }
    logvs::vs_log_setflag(logvs::F_LOCATION_ALLWAYS,
                XMLSupport::parse_bool(vs_config->getVariable("log", "location_allways", "false")));
    logvs::vs_log_setflag(logvs::F_MSGCENTER,
                           XMLSupport::parse_bool(vs_config->getVariable("log", "msgcenter", "false")));
    
	// This should be put into some common function...
	// Keep in mind that initialization must also go into networking/netserver.cpp
    game_options.init();

    //can use the vegastrike config variable to read in the default mission
    if (XMLSupport::parse_bool(vs_config->getVariable ("network","force_client_connect","false"))) {
      ignore_network=false;
    }
    g_game.music_enabled = XMLSupport::parse_bool (vs_config->getVariable ("audio","Music","true"));
    if (mission_name[0]=='\0')
    {
      std::string defmis=vs_config->getVariable ("general","default_mission","test/test1.mission");
      strcpy(mission_name,defmis.c_str());
      GAME_LOG(logvs::NOTICE, "MISSION_NAME is empty, using : %s", mission_name);
    }
    //might overwrite the default mission with the command line
    InitUnitTables();
#ifdef HAVE_PYTHON
    std::cerr << std::endl;
    Python::init();

    Python::test();
    std::cerr << std::endl;
#endif
    std::vector<std::vector <char > > temp = ROLES::getAllRolePriorities();
    // signal( SIGSEGV, SIG_DFL );
#if 0
    InitTime();
    UpdateTime();
#endif

    AUDInit();
    AUDListenerGain (XMLSupport::parse_float(vs_config->getVariable ("audio","sound_gain",".5")));
    Music::InitMuzak();
    /* Set up a function to clean up when program exits */
    winsys_atexit( cleanup );
    /*
#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)

    init_audio_data();
    init_audio();
    init_joystick();

#endif
    */
      //Register commands
      // COmmand Interpretor Seems to break VC8, so I'm leaving disabled for now - Patrick, Dec 24
	if (XMLSupport::parse_bool(vs_config->getVariable("general","command_interpretor","false"))) {
		CommandInterpretor = new commandI;
		InitShipCommands();
	}

    _Universe= new GameUniverse(argc,argv,vs_config->getVariable ("general","galaxy","milky_way.xml").c_str());
	//_Universe.Init(argc, argv, vs_config->getVariable ("general","galaxy","milky_way.xml").c_str());
    //    winsys_set_keyboard_func(nothinghappens);
    _Universe->Loop(bootstrap_first_loop);

    //Unregister commands - and cleanup memory
    UninitShipCommands();

    return 0;
}
  static Animation * SplashScreen = NULL;
static bool BootstrapMyStarSystemLoading=true;
void SetStarSystemLoading (bool value) {
  BootstrapMyStarSystemLoading=value;
}
bool GetStarSystemLoading () {
  return BootstrapMyStarSystemLoading;
}
void SetSplashScreen(Animation * ss)
{
	SplashScreen=ss;
}
Animation* GetSplashScreen()
{
	return SplashScreen;
}
void bootstrap_draw (const std::string &message, Animation * newSplashScreen) {

    static Animation *ani=NULL;
    static bool reentryWatchdog = false;

    if (!BootstrapMyStarSystemLoading || reentryWatchdog)
        return;

    Music::MuzakCycle(); // Allow for loading music...

    if(SplashScreen==NULL && newSplashScreen==NULL) {
        // if there's no splashscreen, we don't draw on it
        // this happens, when the splash screens texture is loaded
        return;
    }

    reentryWatchdog = true;

    if (newSplashScreen!=NULL)
        ani = newSplashScreen;

    UpdateTime();

    Matrix tmp;
    Identity (tmp);
    BootstrapMyStarSystemLoading=false;
    static Texture dummy ("white.bmp",0,MIPMAP,TEXTURE2D,TEXTURE_2D, 1 );
    BootstrapMyStarSystemLoading=true;
    dummy.MakeActive();
    GFXDisable(LIGHTING);
    GFXDisable(DEPTHTEST);
    GFXBlendMode (ONE,ZERO);
    GFXEnable (TEXTURE0);
    GFXDisable (TEXTURE1);
    GFXColor4f (1,1,1,1);
    GFXClear (GFXTRUE);
    GFXLoadIdentity (PROJECTION);
    GFXLoadIdentity(VIEW);
    GFXLoadMatrixModel (tmp);
    GFXBeginScene();

    bs_tp->SetPos (-.99,-.97); // Leave a little bit of room for the bottoms of characters.
    bs_tp->SetCharSize (.4,.8);
    ScaleMatrix (tmp,Vector (6500,6500,0));
    GFXLoadMatrixModel (tmp);
    GFXHudMode (GFXTRUE);
    if (ani) {
        if (GetElapsedTime()<10) ani->UpdateAllFrame();
        ani->DrawNow(tmp);
    }

    static std::string defaultbootmessage=vs_config->getVariable("graphics","default_boot_message","");
    static std::string initialbootmessage=vs_config->getVariable("graphics","initial_boot_message","Loading...");
    bs_tp->Draw (defaultbootmessage.length()>0?defaultbootmessage:message.length()>0?message:initialbootmessage);

    GFXHudMode (GFXFALSE);
    GFXEndScene();

    reentryWatchdog = false;
}
extern Unit **fighters;


bool SetPlayerLoc (QVector &sys, bool set) {
  static QVector mysys;
  static bool isset=false;
  if (set) {
    isset = true;
    mysys = sys;
    return true;
  }else {
    if (isset)
      sys =mysys;
    return isset;
  }

}
bool SetPlayerSystem (std::string &sys, bool set) {
  static std::string mysys;
  static bool isset=false;
  if (set) {
    isset = true;
    mysys = sys;
    return true;
  }else {
    if (isset)
      sys =mysys;
    return isset;
  }
}
vector<string> parse_space_string(std::string s) {
	vector<string> ret;
	string::size_type index=0;
	while ((index =s.find(" "))!=string::npos) {
		ret.push_back (s.substr(0,index));
		s = s.substr (index+1);
	}
	ret.push_back(s);
	return ret;
}
void bootstrap_first_loop() {
  static int i=0;
  static std::string ss=vs_config->getVariable ("graphics","splash_screen","vega_splash.ani");
  static std::string sas=vs_config->getVariable ("graphics","splash_audio","");
  static bool isgamemenu=XMLSupport::parse_bool(vs_config->getVariable("graphics","main_menu","false"));
  if (i==0) {
	vector<string> s = parse_space_string(ss);
    vector<string> sa = parse_space_string(sas);
    int snum=time(NULL)%s.size();
    SplashScreen = new Animation (s[snum].c_str(),0);
    if (sa.size()&&sa[0].length()) muzak->GotoSong(sa[snum%sa.size()]);
    bs_tp=new TextPlane();
  }
  bootstrap_draw ("Vegastrike Loading...",SplashScreen);

  if (i++>4) {
    if (_Universe) {
      if (isgamemenu) {
        UniverseUtil::startMenuInterface(true);
      } else {
        _Universe->Loop(bootstrap_main_loop);
      }
    }
  }
}
void SetStartupView(Cockpit * cp) {
      static std::string startupview = vs_config->getVariable("graphics","startup_cockpit_view","front");
      cp->SetView(startupview=="view_target"?CP_TARGET:(startupview=="back"?CP_BACK:(startupview=="chase"?CP_CHASE:CP_FRONT)));
}
void bootstrap_main_loop () {
  static bool LoadMission=true;
  static bool loadLastSave = XMLSupport::parse_bool(vs_config->getVariable("general","load_last_savegame","false"));
  InitTime();

  if (LoadMission) {
    LoadMission=false;
    active_missions.push_back(mission=new Mission(mission_name));

    mission->initMission();

    UniverseUtil::showSplashScreen(""); // Twice for double or triple-buffering
    UniverseUtil::showSplashScreen("");

    QVector pos;
    string planetname;

    mission->GetOrigin(pos,planetname);
    bool setplayerloc=false;
    string mysystem = mission->getVariable("system","sol.system");

	int numplayers;
	/*
	string nbplayers = vs_config->getVariable("network","nbplayers","1");
	// Test if nb players if present in network section
	if( !ignore_network && nbplayers != "")
	{
		numplayers = atoi( nbplayers.c_str());
		cout<<numplayers<<" Players in Networking Mode"<<endl;
	}
	else
	{
	*/
		numplayers = XMLSupport::parse_int (mission->getVariable ("num_players","1"));
	//}
    vector <std::string>playername;
    vector <std::string>playerpasswd;
	string pname, ppasswd;
    for (int p=0;p<numplayers;p++) {
	  pname = vs_config->getVariable("player"+((p>0)?tostring(p+1):string("")),"callsign","");
	  ppasswd = vs_config->getVariable("player"+((p>0)?tostring(p+1):string("")),"password","");
          if (p==0&&global_username.length())
            pname=global_username;
          if (p==0&&global_password.length())
            ppasswd=global_password;

	  if ( !ignore_network )
	  {
		  // In network mode, test if all player sections are present
		  if( pname=="")
		  {
			  GAME_LOG(logvs::ERROR, "Missing or incomlpete section for player %d", p);
			  cleanexit=true;
			  winsys_exit(1);
		  }
	  }
      playername.push_back( pname);
	  playerpasswd.push_back(ppasswd);
    }

    float credits=XMLSupport::parse_float (mission->getVariable("credits","0"));
    g_game.difficulty=XMLSupport::parse_float (mission->getVariable("difficulty","1"));
	string savegamefile = mission->getVariable ("savegame","");
    vector <SavedUnits> savedun;
    vector <string> playersaveunit;
	vector <StarSystem *> ss;
	vector <string> starsysname;
	vector <QVector> playerNloc;

	/************************* NETWORK INIT ***************************/
	  vector<vector<std::string> > savefiles;
	if ( !ignore_network )
	{
		cout<<"Number of local players = "<<numplayers<<endl;
		// Initiate the network if in networking play mode for each local player
		if( !ignore_network )
		{
            setNewTime(0.);
			// Get the number of local players
			Network = new NetClient[numplayers];

		// Here we say we want to only handle activity in 1 starsystem not more
			run_only_player_starsystem=true;
		}
		else
		{
			Network = NULL;
			cout<<"Non-networking mode"<<endl;
		// Here we say we want to only handle activity in 1 starsystem not more
			run_only_player_starsystem=true;
		}
	}

    _Universe->SetupCockpits(playername);

	/************************* NETWORK INIT ***************************/
    vector<std::string>::iterator it, jt;
    unsigned int k=0;
    for (k=0, it=playername.begin(), jt=playerpasswd.begin();k<(unsigned int)_Universe->numPlayers();k++, it++, jt++) {
      Cockpit *cp = _Universe->AccessCockpit(k);
      SetStartupView(cp);
      bool setplayerXloc=false;
      std::string psu;
      if (k==0) {
		QVector myVec;
		if (SetPlayerLoc (myVec,false)) {
		  _Universe->AccessCockpit(0)->savegame->SetPlayerLocation(myVec);
		}
		std::string st;
		if (SetPlayerSystem (st,false)) {
		  _Universe->AccessCockpit(0)->savegame->SetStarSystem(st);
		}
      }
	  vector <SavedUnits> saved;
		/************* NETWORK PART ***************/
	  if( Network!=NULL)
	  {
		string err;
		string srvipadr;
		unsigned short port;
		// Are we using the directly account server to identify us ?
		Network[k].SetConfigServerAddress(srvipadr, port); // Sets from the config vars.

        if (!Network[k].connectLoad(pname, ppasswd, err)) {
			cout<<"error while connecting: "<<err<<endl;
			VSExit(1);
		}
		savefiles.push_back( *Network[k].loginSavedGame(0) );
		_Universe->AccessCockpit(k)->savegame->ParseSaveGame ("",mysystem,mysystem,pos,setplayerXloc,credits,_Universe->AccessCockpit()->unitfilename,k, savefiles[k][0], false);
		_Universe->AccessCockpit(k)->TimeOfLastCollision=getNewTime();
		/*
		cout<<"UNIT XML :"<<endl<<savefiles[k][0]<<endl<<endl;
		cout<<"UNIT FILE NAME = "<<_Universe->AccessCockpit(k)->unitfilename[0]<<endl;
		*/
	  } else {
		if (loadLastSave)
		  _Universe->AccessCockpit(k)->savegame->ParseSaveGame (savegamefile,mysystem,mysystem,pos,setplayerXloc,credits,_Universe->AccessCockpit()->unitfilename,k);
		else
		  _Universe->AccessCockpit(k)->savegame->SetOutputFileName (savegamefile);
      }
          CopySavedShips(playername[k],k,_Universe->AccessCockpit()->unitfilename,true);
	  playersaveunit.push_back(_Universe->AccessCockpit(k)->GetUnitFileName());
	  _Universe->AccessCockpit(k)->credits=credits;
		  ss.push_back (_Universe->Init (mysystem,Vector(0,0,0),planetname));
	  if (setplayerXloc) {
	   	  playerNloc.push_back(pos);
	  }else {
		  playerNloc.push_back(QVector(FLT_MAX,FLT_MAX,FLT_MAX));
	  }
	  setplayerloc=setplayerXloc;//FIX ME will only set first player where he was

      for (unsigned int j=0;j<saved.size();j++) {
		savedun.push_back(saved[j]);
      }

    }
    SetStarSystemLoading (true);
    InitializeInput();

    vs_config->bindKeys();//gotta do this before we do ai

    createObjects(playersaveunit,ss,playerNloc, savefiles);

    if (setplayerloc&&fighters) {
      if (fighters[0]) {
	//fighters[0]->SetPosition (Vector (0,0,0));
      }
    }

    while (!savedun.empty()) {
      AddUnitToSystem (&savedun.back());
      savedun.pop_back();
    }


    forcefeedback=new ForceFeedback();

    UpdateTime();
    FactionUtil::LoadContrabandLists();
	{
		string str=vs_config->getVariable ("general","intro1","Welcome to Vega Strike! Use #8080FFTab#000000 to afterburn (#8080FF+,-#000000 cruise control), #8080FFarrows#000000 to steer.");
		if (!str.empty()) {
			UniverseUtil::IOmessage (0,"game","all",str);
			str=vs_config->getVariable ("general","intro2","The #8080FFt#000000 key targets objects; #8080FFspace#000000 fires at them & #8080FFa#000000 activates the SPEC drive. To");
			if (!str.empty()) {
				UniverseUtil::IOmessage (4,"game","all",str);
				str=vs_config->getVariable ("general","intro3","go to another star system, buy a jump drive for about 10000 credits, fly to a");
				if (!str.empty()) {
					UniverseUtil::IOmessage (8,"game","all",str);
					str = vs_config->getVariable ("general","intro4","wireframe jump-point and press #8080FFj#000000 to warp to a near star. Target a base or planet;");
					if (!str.empty()) {
						UniverseUtil::IOmessage (12,"game","all",str);
						str=vs_config->getVariable ("general","intro5","When you get close a green box will appear. Inside the box, #8080FFd#000000 will land.");
						if (!str.empty())
							UniverseUtil::IOmessage (16,"game","all",str);
					}
				}
			}
		}
	}
	// Never dock on load in networking if it was said so in the save file NETFIXME--this may change
	if (Network==NULL && mission->getVariable("savegame","").length()!=0&&XMLSupport::parse_bool(vs_config->getVariable("AI","dockOnLoad","true"))) {
		for (int i=0;i<_Universe->numPlayers();i++) {
			QVector vec;
			DockToSavedBases(i, vec);
		}
	}
    GAME_LOG(logvs::NOTICE, "Loading completed, now network init");
	// Send a network msg saying we are ready and also send position info
	if( Network!=NULL) {
		int l;
		/*
		for(l=0; l<_Universe->numPlayers(); l++)
		{
			Network[l].downloadZoneInfo();
		}
		*/
		// Downloading zone info before setting inGame (CMD_ADDCLIENT) causes a race condition.
		// CMD_ADDEDYOU (response to CMD_ADDCLIENT) now sends zone info.
		for(l=0; l<_Universe->numPlayers(); l++)
		{
			Network[l].inGame();
		}
	}
	if (loadLastSave) {
		// Don't write if we didn't load...
		for (unsigned int i=0;i<_Universe->numPlayers();++i) {
			WriteSaveGame(_Universe->AccessCockpit(i),false);
		}
	}
	cur_check = getNewTime();
        for (unsigned int i=0;i<_Universe->numPlayers();++i) {
          _Universe->AccessCockpit(i)->savegame->LoadSavedMissions();
        }
    GAME_LOG(logvs::NOTICE, "Running Main Loop");
             _Universe->Loop(main_loop);
    ///return to idle func which now should call main_loop mohahahah
    if (XMLSupport::parse_bool(vs_config->getVariable("splash","auto_hide","true")))
        UniverseUtil::hideSplashScreen();
    //winsys_warp_pointer(g_game.x_resolution / 2 ,g_game.y_resolution / 2);
    //winsys_post_redisplay();
  }
  ///Draw Texture

}

int ParseConfigOverrides(int argc, char ** argv, VSFileSystem::ConfigOverrides_type & overrides) {
    for (int i = 1; i < argc; ++i) {
        if (!strncmp(argv[i],"-C",2)) {
            // ParseCommandLine ensures good format
            const char * equal = strchr(argv[i]+2, '=');
            const char * slash = argv[i]+2 - 1;
            for ( const char * tmpslash; (tmpslash = strchr(slash+1, '/')) != NULL
                 && tmpslash < equal; slash = tmpslash) {
            }
            std::string section(argv[i]+2, 0, slash - (argv[i]+2));
            std::string key(slash + 1, 0, equal - (slash + 1));
            std::string value(equal + 1);
            std::cout << "config override: " << section << " / " << key << " = " << value << std::endl;
            overrides.push_back(std::make_pair(std::make_pair(section,key), value));
        }
    }
    return 0;
}

const char helpmessage[] =
"Command line options for vegastrike\n"
"\n"
" -D -d     Specify data directory\n"
" -N -n     Number of players\n"
" -M -m     Specify a mod to play\n"
" -P -p     Specify player location\n"
" -J -j     Start in a specific system\n"
" -A -a     Normal resolution (800x600)\n"
" -H -h     High resolution (1024x768)\n"
" -V -v     Super high resolution (1280x1024)\n"
" -C        Config override -C<section(s)/name>=<value>\n"
" --net     Networking Enabled (Experimental)\n"
" --version Version information\n"
"\n";
std::string ParseCommandLine(int argc, char ** lpCmdLine) {
  std::string st;
  std::string retstr;
  std::string datatmp;
  QVector PlayerLocation;
  for (int i=1;i<argc;i++) {
    if(lpCmdLine[i][0]=='-') {
		std::cerr << "ARG #" << i <<" = " << lpCmdLine[i] << std::endl;
		if (i + 1 == argc)
			std::cerr << std::endl;
      switch(lpCmdLine[i][1]){
    case 'd':
    case 'D':
      // Specifying data directory
        if(lpCmdLine[i][2] == 0) {
            cout << "Option -D requires an argument" << endl;
            exit(1);
        }
 		datatmp = &lpCmdLine[i][2];
		if( VSFileSystem::DirectoryExists( datatmp))
			VSFileSystem::datadir = datatmp;
		else
		{
			cout<<"Specified data directory not found... exiting"<<endl;
			exit(1);
		}
        cout << "Using data dir specified on command line : " << datatmp << endl;
	    break ;
	  case 'r':
      case 'R':
	break;
      case 'N':
      case 'n':
		  if (!(lpCmdLine[i][2]=='1'&&lpCmdLine[i][3]=='\0')) {
	  		CONFIGFILE=new char[40+strlen(lpCmdLine[i])+1];
			sprintf(CONFIGFILE,"vegastrike.config.%splayer",lpCmdLine[i]+2);
		  }
	break;
      case 'M':
      case 'm':
		retstr=string(lpCmdLine[i]+2);
	break;
      case 'f':
      case 'F':
	break;
      case 'U':
      case 'u':
        global_username=lpCmdLine[i]+2;
        break;
      case 'P':
      case 'p':
        global_password=lpCmdLine[i]+2;
        break;
      case 'L':
      case 'l':

	if (3==sscanf (lpCmdLine[i]+2,"%lf,%lf,%lf",&PlayerLocation.i,&PlayerLocation.j,&PlayerLocation.k)) {
          SetPlayerLoc (PlayerLocation,true);
        }
	break;
      case 'J':
      case 'j'://low rez
	st=string ((lpCmdLine[i])+2);
	SetPlayerSystem (st ,true);
	break;
      case 'A'://average rez
      case 'a':
	g_game.y_resolution = 600;
	g_game.x_resolution = 800;
	break;
      case 'H':
      case 'h'://high rez
	g_game.y_resolution = 768;
	g_game.x_resolution = 1024;
	break;
      case 'V':
      case 'v':
	g_game.y_resolution = 1024;
	g_game.x_resolution = 1280;
	break;
      case 'G':
      case 'g':
	//viddrv = "GLDRV.DLL";
	break;
    case 'C': {
        const char * equal = strchr(lpCmdLine[i]+2, '='), * slash;
        // it is a config override, it will be handled later once vs_config is loaded
        if (equal == NULL || (slash = strchr(lpCmdLine[i]+2, '/'))==NULL || slash > equal) {
            cout << "Option -C requires an argument of format <section(s)/key>=<value>" << endl;
            exit(1);
        }
        } break ;
      case '-':
        // long options
        if(strcmp(lpCmdLine[i],"--benchmark")==0){
          //benchmark=30.0;
          benchmark=atof(lpCmdLine[i+1]);
          i++;
        }
        else if(strcmp(lpCmdLine[i], "--net")==0) {
          // don't ignore the network section of the config file
          ignore_network=false;
        }
        else if(strcmp(lpCmdLine[i], "--help")==0) {
          cout << helpmessage;
          exit(0);
        }
        else if(strcmp(lpCmdLine[i], "--version")==0) {
          vs_print_buildinfo(std::cout, VPB_ALL & (~(VPB_VERSION | VPB_SCM)));
          exit(0);
        }
      }
    }
    else{
      // no "-" before it - it's the mission name
      strncpy (mission_name,lpCmdLine[i],1023);
	  mission_name[1023]='\0';
    }
  }
  return retstr;
}

#ifdef HAVE_FFMPEG
# include "ffmpeg_init.h"
#endif
#if defined HAVE_OGG
# include <vorbis/vorbisfile.h>
#endif
#if defined HAVE_PNG
# include <png.h>
#endif
#if defined HAVE_JPEG
# include <jpeglib.h>
#endif
#if defined(HAVE_AL)
# include "aldrv/al_globals.h"
#endif
static void vs_print_buildinfo(std::ostream & out, int flags) {
    (void)flags;
  #if defined(HAVE_SDL)
  # if defined(SDL_MAJOR_VERSION) && defined(SDL_MINOR_VERSION) && defined(SDL_PATCHLEVEL)
    char sdlstr[32];
    snprintf(sdlstr, sizeof(sdlstr), "%d.%d.%d",
             SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
  # else
    std::string sdlstr = "?";
  # endif
  #endif
    
    if ((flags & VPB_VERSION)) {
        out << "Vegastrike v" << VERSION
        << std::endl;
    }
    if ((flags & VPB_SCM)) {
        out << "revision " << SCM_VERSION << " from " << SCM_REMOTE
        << std::endl;
    }
    if ((flags & VPB_ARCH)) {
        out << POSH_GetArchString() << std::endl;
    }
    if (!(flags & (~(VPB_VERSION|VPB_ARCH|VPB_SCM))))
        return ;
    out << "configured with:";
    if ((flags & VPB_CONFIGURE)) {
        out << std::endl << "  " << CONFIGURE_CMD
            << std::endl;
    }
    if ((flags & VPB_BUILD)) {
        out << std::endl << "  COMPILER: " << COMPILER
            << std::endl << "  COMPILER_FLAGS: " << COMPILER_FLAGS
            << std::endl;
    }
    if ((flags & VPB_LIBS)) {
        out
      #if defined(HAVE_SDL)
        << std::endl << "  " << "SDL: " << sdlstr << " (h)"
      #endif
      #if defined(GLUT_API_VERSION)
        << std::endl << "  " << "GLUT api: " << GLUT_API_VERSION
      #endif
      #if defined(HAVE_FFMPEG)
        << std::endl << "  " << "FFMPEG:"
      # if defined(FFMPEG_VERSION)
        << " " << FFMPEG_VERSION << " (h)"
      # endif
      # if defined(LIBAVCODEC_IDENT)
        << " " << LIBAVCODEC_IDENT << " (h)"
      # endif
      # if defined(LIBAVFORMAT_IDENT)
        << " " << LIBAVFORMAT_IDENT << " (h)"
      # endif
      # if defined(LIBAVUTIL_IDENT)
        << " " << LIBAVUTIL_IDENT << " (h)"
      # endif
      # if defined(LIBSWSCALE_IDENT)
        << " " << LIBSWSCALE_IDENT << " (h)"
      # endif
      #endif
      #if defined(HAVE_PYTHON)
        << std::endl << "  " << "PYTHON:"
      # if defined(PY_VERSION)
        << " " << PY_VERSION << " (h)"
      # endif
      #endif
      #if defined(BOOST_LIB_VERSION)
        << std::endl << "  " << "BOOST: " << BOOST_LIB_VERSION
      #elif defined(BOOST_VERSION)
        << std::endl << "  " << "BOOST: " << BOOST_VERSION << " (h)"
      #endif
      #ifdef HAVE_AL
        << std::endl << "  " << "OpenAL: " << AL_VERSION << " (h)"
      #endif
      #if defined(HAVE_OGG)
        << std::endl << "  " << "VORBIS: " << vorbis_version_string()
      #endif
      #ifdef PNG_LIBPNG_VER_STRING
        << std::endl << "  " << "PNG: " << PNG_LIBPNG_VER_STRING << " (h)"
      #endif
      #ifdef JPEG_LIB_VERSION
        << std::endl << "  " << "JPEG: " << JPEG_LIB_VERSION << " (h)"
      #endif
      #ifdef HAVE_EXPAT
        << std::endl << "  " << "EXPAT: " << XML_ExpatVersion()
      #endif
      #if 0 && defined(zlib_version)
        << std::endl << "  " << "ZLIB: " << zlib_version
      #elif defined(ZLIB_VERSION)
        << std::endl << "  " << "ZLIB: " << ZLIB_VERSION << " (h)"
      #endif
        << std::endl;
    }
}
