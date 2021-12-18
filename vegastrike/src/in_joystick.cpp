/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
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

/*
  Joystick support written by Alexander Rawass <alexannika@users.sourceforge.net>
*/
#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif
#include <list>
#include <deque>
#include <lin_time.h>
#include "vegastrike.h"
#include "vs_globals.h"

//#include "glob.h"
//#include "dbg.h"
#include "in_handler.h"
#include "in_joystick.h"
#include "config_xml.h"
#include "in_mouse.h"
#ifndef HAVE_SDL
#include "gldrv/gl_include.h"
#if (GLUT_API_VERSION >= 4 || GLUT_XLIB_IMPLEMENTATION >= 13)
#else
#define NO_SDL_JOYSTICK
#endif
#endif

#include "options.h"
#include "vs_log_modules.h"
#include "gnuhash.h"

extern vs_options game_options;

// Used for storing the max and min values of the tree Joystick Axes - Okona
static int maxx=1;
static int minx=-1;
static int maxy=1;
static int miny=-1;
static int maxz=1;
static int minz=-1;

typedef vsUMap<int, int> 	JoystickIDMap;
typedef std::deque<int> 	JoystickEventQueue;

static JoystickIDMap 		joystickIDMap;
static JoystickEventQueue 	joystickEventQueue;

JoyStick *joystick[MAX_JOYSTICKS]; // until I know where I place it
int num_joysticks=0;

unsigned int GetNumJoysticks() {
	return num_joysticks;
}

void RestoreJoystick() {
	joystickEventQueue.clear();
#if !defined(NO_SDL_JOYSTICK) && !defined(HAVE_SDL)
  //use glut
  if (glutDeviceGet(GLUT_HAS_JOYSTICK)||game_options.force_use_of_joystick) {
          glutJoystickFunc (myGlutJoystickCallback,JoystickPollingRate());
  }
#else
	//winsys_set_joystick_func(NULL);
  winsys_set_joystick_func(JoystickGameHandler);
#endif
}

void modifyDeadZone(JoyStick * j) {
    for(int a=0;a<j->nr_of_axes;a++){
        if(fabs(j->joy_axis[a])<=j->deadzone){
            j->joy_axis[a]=0.0;
        }else if (j->joy_axis[a]>0) {
            j->joy_axis[a]-=j->deadzone;
        }else {
            j->joy_axis[a]+=j->deadzone;
        }
        if (j->deadzone<.999) {
            j->joy_axis[a]/=(1-j->deadzone);
        }
    }
}
void modifyExponent(JoyStick * j) {
    if ((game_options.joystick_exponent != 1.0)&&(game_options.joystick_exponent > 0)) {
        for(int a=0;a<j->nr_of_axes;a++) 
            j->joy_axis[a]=((j->joy_axis[a]<0)?-pow(-j->joy_axis[a],game_options.joystick_exponent):pow(j->joy_axis[a],game_options.joystick_exponent));
    }
}
static bool JoyStickToggle=true;
void JoyStickToggleDisable() {
  JoyStickToggle=false;
}
void JoyStickToggleKey (const KBData& key, KBSTATE a) {
  if (a==PRESS) {
    JoyStickToggle=!JoyStickToggle;
  }
}
void myGlutJoystickCallback (unsigned int buttonmask, int x, int y, int z) {
    //printf ("joy %d x %d y %d z %d\n",buttonmask, x,y,z);
    unsigned int i;

    for (i=0;i<MAX_AXES;i++) {
    	joystick[0]->joy_axis[i]=0.0;
    }
    joystick[0]->joy_buttons=0;
    if (JoyStickToggle) {
      joystick[0]->joy_buttons=buttonmask;
      if (joystick[0]->nr_of_axes>0) {
              // Set the max and min of each axis - Okona
              if (x<minx) minx=x;
              if (x>maxx) maxx=x;
	      // Calculate an autocalibrated value based on the max min values - Okona
              joystick[0]->joy_axis[0]=((float)x-(((float)(maxx+minx))/2.0))/(((float)(maxx-minx))/2.0);
      }
      if (joystick[0]->nr_of_axes>1) {
              if (y<miny) miny=y;
              if (y>maxy) maxy=y;
	      joystick[0]->joy_axis[1]=((float)y-(((float)(maxy+miny))/2.0))/(((float)(maxy-miny))/2.0);
      }
      if (joystick[0]->nr_of_axes>2) {
              if (z<minz) minz=z;
              if (z>maxz) maxz=z;
	      joystick[0]->joy_axis[2]=((float)z-(((float)(maxz+minz))/2.0))/(((float)(maxz-minz))/2.0);
      }
      modifyDeadZone(joystick[0]);
      modifyExponent(joystick[0]);
    }
}

JoyStick::JoyStick () {
  for (int j=0;j<MAX_AXES;++j) {
    axis_axis[j]=-1;
    axis_inverse[j]=false;
    joy_axis[j]=axis_axis[j]=0;
  }
    joy_buttons=0;
}
int JoystickPollingRate () {
    return (game_options.polling_rate);
}
void InitJoystick(){
  int i;
  
#if defined(HAVE_SDL) && !defined(NO_SDL_JOYSTICK)
    // && defined(HAVE_SDL_MIXER)
    if (  SDL_InitSubSystem( SDL_INIT_JOYSTICK )) {
        JOY_LOG(logvs::ERROR, "Couldn't initialize SDL: %s", SDL_GetError());
        winsys_exit(1);
    }
    //SDL_EventState (SDL_JOYBUTTONDOWN,SDL_ENABLE);
    //SDL_EventState (SDL_JOYBUTTONUP,SDL_ENABLE);
#endif

  for (i=0;i<NUMJBUTTONS;i++) {
    for (int j=0;j<MAX_JOYSTICKS;j++) {
      UnbindJoyKey (j,i);
    }
  }
  for(int h=0;h<MAX_HATSWITCHES;h++){
    for(int v=0;v<MAX_VALUES;v++){
      UnbindHatswitchKey(h,v);
    }
  }
  for(int j=0;j<MAX_JOYSTICKS;j++){
	if (j < num_joysticks && joystick[j])
		delete joystick[j];
	joystick[j] = NULL;
    for(int h=0;h<MAX_DIGITAL_HATSWITCHES;h++){
      for(int v=0;v<MAX_DIGITAL_VALUES;v++){
	UnbindDigitalHatswitchKey(j,h,v);
      }
    }
  }
  UpdateJoystick(-1);
}

void UpdateJoystick(int index) {
	int i;
#ifndef NO_SDL_JOYSTICK
#ifdef HAVE_SDL
	num_joysticks = SDL_NumJoysticks() ;
	JOY_LOG(logvs::NOTICE, "%i joysticks were found.", num_joysticks);
	if (num_joysticks > 0)
		JOY_LOG(logvs::NOTICE, "The names of the joysticks are:");
#else
	//use glut
	if (glutDeviceGet(GLUT_HAS_JOYSTICK)||game_options.force_use_of_joystick) {
		JOY_LOG(logvs::NOTICE, "setting joystick functionality:: joystick online");
		glutJoystickFunc (myGlutJoystickCallback,JoystickPollingRate());
		num_joysticks=1;
	} else {
		num_joysticks = 0;
	}
	JOY_LOG(logvs::NOTICE, "Glut detects %d joystick", num_joysticks);
#endif
#endif

	for(i = 0; i < MAX_JOYSTICKS; ++i ) {
		if (joystick[i]) {
			if (num_joysticks > 0 && index >= num_joysticks - 1 && !joystick[i]->isAvailable()) {
				joystick[i]->Update(joystick[i]->player, index);
				JOY_LOG(logvs::INFO, "Joystick #%d.%d added for player %d", i, index, joystick[i]->player);
				index = -1;
			} else if (index == i && joystick[i]->isAvailable()) {
				// removed;
				joystick[i]->Update(joystick[i]->player, num_joysticks);
				for (JoystickIDMap::iterator it = joystickIDMap.begin(); it != joystickIDMap.end(); ) {
					if (it->second == i) {
#if defined(HAVE_TR1_UNORDERED_MAP) || defined(HAVE_UNORDERED_MAP)
						it = joystickIDMap.erase(it);
#else
						JoystickIDMap::iterator next = it; ++next;
						joystickIDMap.erase(it); it = next;
#endif
					}
					else ++it;
				}
				JOY_LOG(logvs::INFO, "Joystick #%d.%d removed for player %d", i, index, joystick[i]->player);
			}
		} else {
			joystick[i]=new JoyStick(i); // SDL_Init is done in winsys.cpp
		}
		if (i != MOUSE_JOYSTICK && joystick[i]->isAvailable()) {
#if !defined(HAVE_SDL) || !SDL_VERSION_ATLEAST(2,0,0)
			joystickIDMap.insert(std::make_pair(i, i));
#else
			joystickIDMap.insert(std::make_pair(SDL_JoystickInstanceID(joystick[i]->joy), i));
#endif
		}
	}
}

int GetJoystickByID(int id) {
	JoystickIDMap::iterator it = joystickIDMap.find(id);
	if (it != joystickIDMap.end())
		return it->second;
	return num_joysticks;
}

void DeInitJoystick() {
	joystickEventQueue.clear();
	num_joysticks = 0;
	for (int i = 0; i < MAX_JOYSTICKS; ++i) {
		if (joystick[i]) {
			delete joystick[i];
			joystick[i] = NULL;
		}
	}
}

JoyStick::JoyStick(int which): mouse(which==MOUSE_JOYSTICK) {
  for (int j=0;j<MAX_AXES;++j) {
    axis_axis[j]=-1;
    axis_inverse[j]=false;
    joy_axis[j]=0;
  }
  joy_buttons=0;

  player=which;//by default bind players to whichever joystick it is
  debug_digital_hatswitch=game_options.debug_digital_hatswitch;
  if (which!=MOUSE_JOYSTICK)
      deadzone=game_options.deadband; else
	  deadzone=game_options.mouse_deadband;;
  joy_available = 0;
  joy_x=joy_y=joy_z=0;
  if (which==MOUSE_JOYSTICK) {
    InitMouse(which);
  }
  this->Update(player, which);
}

JoyStick::~JoyStick() {
#if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL)
	if (joy != NULL) {
		//SDL_JoystickClose(joy);
	}
#endif
}

void JoyStick::Update(int player, int index) {
	this->player = player;
#if defined (NO_SDL_JOYSTICK)
  return;
#else
#ifdef HAVE_SDL
  num_joysticks=SDL_NumJoysticks() ;
  if (index >= num_joysticks || index == MOUSE_JOYSTICK) {
    if (index!=MOUSE_JOYSTICK)
      joy_available=false;
    return;
  }
  
  //    SDL_JoystickEventState(SDL_ENABLE);
  joy=SDL_JoystickOpen(index);  // joystick nr should be configurable
  if(joy==NULL){
      JOY_LOG(logvs::NOTICE, "warning: no joystick nr %d",index);
      joy_available = false;
      return;
  }
  joy_available=true;
  nr_of_axes=SDL_JoystickNumAxes(joy);
  nr_of_buttons=SDL_JoystickNumButtons(joy);
  nr_of_hats=SDL_JoystickNumHats(joy);
#else
    //WE HAVE GLUT
    if (index>0&&index!=MOUSE_JOYSTICK) {
        joy_available=false;
        return;
    }
    joy_available=true;
    nr_of_axes=3;//glutDeviceGet(GLUT_JOYSTICK_AXES);
    nr_of_buttons=15;//glutDeviceGet(GLUT_JOYSTICK_BUTTONS);
    nr_of_hats=0;
#endif // we have GLUT
#endif
#if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL)
# if !SDL_VERSION_ATLEAST(2,0,0)
    JOY_LOG(logvs::NOTICE, "    #%d.%d: %s", player, index, SDL_JoystickName(index));
# else
    JOY_LOG(logvs::NOTICE, "    #%d.%d: %s", player, index, SDL_JoystickNameForIndex(index));
# endif
#endif
     JOY_LOG(logvs::NOTICE, "      axes: %d buttons: %d hats: %d",
    		 nr_of_axes,nr_of_buttons,nr_of_hats);
}

void JoyStick::InitMouse (int which) {
  player=0;//default to first player
  joy_available=true;
  nr_of_axes=2;//x and y for mouse
  nr_of_buttons=15;
  nr_of_hats=0;
#if defined(HAVE_SDL)
  joy = NULL;
#endif
}

bool JoyStick::isAvailable(){
  return joy_available;
}

struct mouseData{
  int dx;
  int dy;
  float time;
  mouseData() {dx=dy=0;time=0;}
  mouseData(int ddx,int ddy, float ttime) {dx=ddx;dy=ddy;time=ttime;}
};

extern void GetMouseXY(int &mousex,int &mousey);

static std::list <mouseData> md;

void JoyStick::GetMouse (float &x, float &y, float &z, int &buttons) {
  int def_mouse_sens = 1;
  int _dx, _dy;
  float fdx,fdy;
  int _mx,_my;
  GetMouseXY (_mx,_my);
  GetMouseDelta (_dx,_dy);
  if (0&&(_dx||_dy))
    JOY_DBG(logvs::DBG, "dx:%d dy:%d",_dx,_dy);
  if (!game_options.warp_mouse) {
    fdx=(float)(_dx = _mx-g_game.x_resolution/2);
    def_mouse_sens=25;
    fdy=(float)(_dy = _my-g_game.y_resolution/2);
  }else {
    std::list<mouseData>::iterator i=md.begin();
    float ttime=getNewTime();
    float lasttime=ttime-game_options.mouse_blur;
    int avg=(_dx||_dy)?1:0;
    float valx=_dx;
    float valy=_dy;
    for(;i!=md.end();) {
      if ((*i).time>=lasttime) {
        bool found=false;
        int ldx=(*i).dx;
        int ldy=(*i).dy;
        if ((ldx>=0)*_dx*ldx==(_dx>=0)*_dx*ldx) {
          //make sure same sign or zero
          valx+=(*i).dx;
          found=true;
        }
        if ((ldy>=0)*_dy*ldy==(_dy>=0)*_dy*ldy) {
          //make sure same sign or zero
          valy+=(*i).dy;
          found=true;
        }
        if (found)
          avg++;
        ++i;
      } else {
        if ((i=md.erase(i))==md.end())
			break;
      } 
    }
    if (_dx||_dy)
      md.push_back(mouseData(_dx,_dy,ttime));
    if (avg) {
      _dx=float_to_int(valx/avg);
      _dy=float_to_int(valy/avg);
    }
    fdx=float(valx)/game_options.mouse_blur;
    fdy=float(valy)/game_options.mouse_blur;
    //JOY_LOG(logvs::DBG, " x:%.2f y:%.2f %d",fdx,fdy,avg);
  }
  joy_axis[0] = fdx/(g_game.x_resolution*def_mouse_sens/game_options.mouse_sensitivity);
  joy_axis[1] = fdy/(g_game.y_resolution*def_mouse_sens/game_options.mouse_sensitivity);

  if(!game_options.warp_mouse){
  	modifyDeadZone(this);
  }
  
  joy_axis[0]*=game_options.mouse_exponent;
  joy_axis[1]*=game_options.mouse_exponent;
  x = joy_axis[0];
  y = joy_axis[1];
  joy_axis[2] = z = 0;
  joy_buttons = buttons = getMouseButtonStatus();
}

void JoyStick::GetJoyStick(float &x,float &y, float &z, int &buttons)
{
    //int status;
    if(joy_available==false){
      for(int a=0;a<MAX_AXES;a++){
        joy_axis[a]=0;
      }
        x=y=z=0;
        joy_buttons=buttons=0;
        return;
    } else if (mouse) {
      GetMouse (x,y,z,buttons);
      return;
    }
    int a;
#ifndef NO_SDL_JOYSTICK
#if defined(HAVE_SDL)

    int numaxes = SDL_JoystickNumAxes (joy);

	vector <Sint16> axi(numaxes<MAX_AXES?MAX_AXES:numaxes);
    
    for(a=0;a<numaxes;a++){
      axi[a] = SDL_JoystickGetAxis(joy,a);
    }

    joy_buttons=0;
    nr_of_buttons=SDL_JoystickNumButtons(joy);

   for(int i=0;i<nr_of_buttons;i++){
     int  butt=SDL_JoystickGetButton(joy,i);
     if(butt==1){
       joy_buttons|=(1<<i);
      }
   }
   for(int h=0;h<nr_of_hats;h++){
       digital_hat[h]=SDL_JoystickGetHat(joy,h);
   }
   for(a=0;a<MAX_AXES;a++)
       joy_axis[a]=((float)axi[a]/32768.0);
   modifyDeadZone(this);
   modifyExponent(this);
#else //we have glut
    if (JoystickPollingRate()<=0) {
        glutForceJoystickFunc();
    }
#endif
    x=joy_axis[0];
    y=joy_axis[1];
    z=joy_axis[2];
    buttons=joy_buttons;

#endif // we have no joystick
    return;
}

int JoyStick::NumButtons(){
  return nr_of_buttons;
}

void JoystickQueuePush(int which) {
	joystickEventQueue.push_back(which);
}

void JoystickGameHandler(unsigned int which, float x, float y, float z, unsigned int buttons, unsigned int state) {
	(void)x; (void)y; (void)z; (void)buttons; (void)state;
	JoystickQueuePush(which);
}

void JoystickProcessQueue(int player) {
	bool done[MAX_JOYSTICKS];
	memset(done, 0, sizeof(done));
	if (md.size() > 0 && joystick[MOUSE_JOYSTICK]->isAvailable() && joystick[MOUSE_JOYSTICK]->player == player) {
		float x, y, z; int buttons;
		joystick[MOUSE_JOYSTICK]->GetJoyStick (x,y,z,buttons);
		JoystickQueuePush(MOUSE_JOYSTICK);
	}
	for (JoystickEventQueue::iterator it = joystickEventQueue.begin(); it != joystickEventQueue.end(); ) {
		unsigned int which = *it;
		if (!joystick[which]->isAvailable()) {
			it = joystickEventQueue.erase(it);
		} else if (joystick[which]->player == player) {
			if (!done[which]) {
				ProcessJoystick(which);
				JOY_DBG(logvs::DBG, "Joystick #%u x:%g y:%g z:%g buttons:%d", which, joystick[which]->joy_axis[0],
					    joystick[which]->joy_axis[1], joystick[which]->joy_axis[2], joystick[which]->joy_buttons);
			}
			if (done[which] || (joystick[which]->joy_buttons == 0)) { // The game loop needs to receive repeated buttons events
				it = joystickEventQueue.erase(it);
			} else {
				++it;
			}
			done[which] = true;
		} else if (joystick[*it]->player >= _Universe->numPlayers()) {
			it = joystickEventQueue.erase(it);
		} else {
			++it;
		}
	}
}
