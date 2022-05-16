#include "in_joystick.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "in_kb_data.h"
#include <assert.h>	/// needed for assert() calls.
#include "vs_log_modules.h"

void DefaultJoyHandler (const KBData&, KBSTATE newState) {
  //  VSFileSystem::Fprintf (stderr,"STATE: %d", st);
}
struct JSHandlerCall {
  KBHandler function;
  KBData data;
  JSHandlerCall() {
    function=DefaultJoyHandler;
  }
  JSHandlerCall(KBHandler function, const KBData& data) {
    this->function=function;
    this->data=data;
  }
};
enum JSSwitches{
  JOYSTICK_SWITCH,
  HATSWITCH,
  DIGHATSWITCH,
  NUMSWITCHES
};
#define MAXOR(A,B) (((A)<(B))?(B):(A))
JSHandlerCall JoystickBindings [INSC_NB][NUMSWITCHES][MAXOR(MAX_HATSWITCHES,MAX_JOYSTICKS)][MAXOR(NUMJBUTTONS,MAXOR(MAX_VALUES,MAX_DIGITAL_HATSWITCHES*MAX_DIGITAL_VALUES))];
KBSTATE JoystickState [NUMSWITCHES][MAXOR(MAX_HATSWITCHES,MAX_JOYSTICKS)][MAXOR(MAX_VALUES,MAXOR(NUMJBUTTONS,MAX_DIGITAL_HATSWITCHES*MAX_DIGITAL_VALUES))];

static void GenUnbindJoyKey (JSSwitches whichswitch, int joystick, int key, unsigned int scope) {
	if (!(key<MAXOR(NUMJBUTTONS,MAXOR(MAX_VALUES,MAX_DIGITAL_HATSWITCHES*MAX_DIGITAL_VALUES))&&joystick<MAXOR(MAX_JOYSTICKS,MAX_HATSWITCHES)
	      && whichswitch < NUMSWITCHES && key >= 0)) {
		JOY_LOG(logvs::WARN, "unbind joystick switch(%d)/joy(%d)/key(%d)/scope(%u) out of bound", whichswitch, joystick, key, scope);
		return ;
	}
	for (size_t ijoy = joystick>=0 ? joystick : 0; ijoy < (joystick>=0 ? joystick+1 : MAX_JOYSTICKS); ++ijoy) {
		for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
			JoystickBindings[iscp][whichswitch][ijoy][key]=JSHandlerCall();
		}
		JoystickState[whichswitch][ijoy][key]=UP;
	}
}

static void GenBindJoyKey (JSSwitches whichswitch,int joystick, int key, unsigned int scope, KBHandler handler, const KBData &data) {
  if (!(key<MAXOR(NUMJBUTTONS,MAXOR(MAX_VALUES,MAX_DIGITAL_HATSWITCHES*MAX_DIGITAL_VALUES))&&joystick<MAXOR(MAX_JOYSTICKS,MAX_HATSWITCHES)
        && whichswitch < NUMSWITCHES && key >= 0)) {
    JOY_LOG(logvs::WARN, "bind joystick switch(%d)/joy(%d)/key(%d)/scope(%u) out of bound", whichswitch, joystick, key, scope);
    return ;
  }
  for (size_t ijoy = joystick>=0 ? joystick : 0; ijoy < (joystick>=0 ? joystick+1 : MAX_JOYSTICKS); ++ijoy) {
	  for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
		  JoystickBindings[iscp][whichswitch][ijoy][key]=JSHandlerCall(handler,data);
	  }
  }
  handler (KBData(),RESET);
}


void UnbindJoyKey (int joystick, int key, unsigned int scope) {
  GenUnbindJoyKey(JOYSTICK_SWITCH,joystick,key,scope);
}

void BindJoyKey (int joystick, int key, unsigned int scope, KBHandler handler, const KBData &data) {
  GenBindJoyKey(JOYSTICK_SWITCH,joystick,key,scope,handler,data);
}

void UnbindHatswitchKey (int joystick, int key, unsigned int scope) {
  GenUnbindJoyKey(HATSWITCH,joystick,key,scope);
}

void BindHatswitchKey (int joystick, int key, unsigned int scope, KBHandler handler, const KBData &data) {
  GenBindJoyKey(HATSWITCH,joystick,key,scope,handler,data);
}

void UnbindDigitalHatswitchKey (int joystick, int key, int dir, unsigned int scope) {
  GenUnbindJoyKey(DIGHATSWITCH,joystick,key*MAX_DIGITAL_VALUES+dir,scope);
}

void BindDigitalHatswitchKey (int joystick, int key, int dir, unsigned int scope, KBHandler handler, const KBData &data) {
  GenBindJoyKey(DIGHATSWITCH,joystick,key*MAX_DIGITAL_VALUES+dir,scope,handler,data);
}


void ProcessJoystick (int which) {
  float x,y,z;
  int buttons;
  unsigned int scope = inGetCurrentScope();
#if !defined(HAVE_SDL)
  SDL_JoystickUpdate();// this is called by SDL event loop (even by glut) / winsys.cpp
#endif
  for (int i=which;i<which+1&&i<MAX_JOYSTICKS;i++) {
    buttons=0;
    if(joystick[i]->isAvailable()){
#if !defined(HAVE_SDL)
      joystick[i]->GetJoyStick (x,y,z,buttons); // This is done by winsys SDL event handler (winsys.cpp)
#else
      x = joystick[i]->joy_axis[0]; y = joystick[i]->joy_axis[1]; z = joystick[i]->joy_axis[2];
      buttons = joystick[i]->joy_buttons;
#endif

      for(int h=0;h<joystick[i]->nr_of_hats;h++){
#ifdef HAVE_SDL
	Uint8 
#else
	  unsigned char
#endif
	  hsw=joystick[i]->digital_hat[h];
      if(joystick[i]->debug_digital_hatswitch){
	char buf[100];
	sprintf(buf,"hsw: %d",hsw);
	std::cout << buf << std::endl;
	  }
	for(int dir_index=0;dir_index<MAX_DIGITAL_VALUES;dir_index++){
	  bool press=false;
#ifdef HAVE_SDL
#ifndef NO_SDL_JOYSTICK
	  // CENTERED is an exact position.
	  if(dir_index==VS_HAT_CENTERED && (hsw == SDL_HAT_CENTERED)){
	    if(joystick[i]->debug_digital_hatswitch){
	      std::cout << "center" << std::endl;
	    }
	    press=true;
	  }
	  if(dir_index==VS_HAT_LEFT && (hsw & SDL_HAT_LEFT)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_RIGHT && (hsw & SDL_HAT_RIGHT)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_DOWN && (hsw & SDL_HAT_DOWN)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_UP && (hsw & SDL_HAT_UP)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_RIGHTUP && (hsw & SDL_HAT_RIGHTUP)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_RIGHTDOWN && (hsw & SDL_HAT_RIGHTDOWN)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_LEFTUP && (hsw & SDL_HAT_LEFTUP)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_LEFTDOWN && (hsw & SDL_HAT_LEFTDOWN)){
	    press=true;
	  }
#endif
#endif
          KBSTATE * state
            =&JoystickState[DIGHATSWITCH][i][h*MAX_DIGITAL_VALUES+dir_index];
          JSHandlerCall* handler
            =&JoystickBindings[scope][DIGHATSWITCH][i][h*MAX_DIGITAL_VALUES+dir_index];
	  if(press==true){
	    if(*state==UP){
	      (*handler->function)
                (handler->data,PRESS);
	      *state=DOWN;
	    }
	  }
	  else{
	    if(*state==DOWN){
	      (*handler->function)
                (handler->data,RELEASE);
	    }
	    *state=UP;
	  }
	  (*handler->function) (handler->data,*state);
	}
      } // digital_hatswitch
      
      for (int j=0;j<NUMJBUTTONS;j++) {
        KBSTATE * state = &JoystickState[JOYSTICK_SWITCH][i][j];
        JSHandlerCall*handler=&JoystickBindings [scope][JOYSTICK_SWITCH][i][j];
	if ((buttons&(1<<j))) {
	  if (*state==UP) {
	    (*handler->function)
              (handler->data,PRESS);
	    *state=DOWN;
          }
	}else {
	  if (*state==DOWN) {
	    (*handler->function)(handler->data,RELEASE);
	  }
	  *state=UP;
	}
	(*handler->function) (handler->data,*state);
      }
    } // is available
    JOY_DBG(logvs::DBG, "Joystick #%u x:%g y:%g z:%g buttons:%d", i, joystick[i]->joy_axis[0],
      	    joystick[i]->joy_axis[1], joystick[i]->joy_axis[2], joystick[i]->joy_buttons);
  } // for nr joysticks  

  for(int h=0;h<MAX_HATSWITCHES;h++){
    float margin=fabs(vs_config->hatswitch_margin[h]);
      if(margin<1.0){
	// we have hatswitch nr. h
	int hs_axis=vs_config->hatswitch_axis[h];
	int hs_joy=vs_config->hatswitch_joystick[h];

	if(joystick[hs_joy]->isAvailable() && hs_axis < joystick[hs_joy]->nr_of_axes){
	  float axevalue=joystick[hs_joy]->joy_axis[hs_axis];
	  
	  for(int v=0;v<MAX_VALUES;v++){
	    float hs_val=vs_config->hatswitch[h][v];
	    if(fabs(hs_val)<=1.0){
	      // this is set
	      JSHandlerCall *handler=&JoystickBindings[scope][HATSWITCH][h][v];
              KBSTATE * state = &JoystickState[HATSWITCH][h][v];
	      if(hs_val-margin<=axevalue && axevalue<=hs_val+margin){
		// hatswitch pressed
		
		if(*state==UP){
		  (*handler->function)(handler->data,PRESS);
		  *state=DOWN;
		}
	      }
	      else{
		// not pressed
		if(*state==DOWN){
		  (*handler->function)(handler->data,RELEASE);
		}
		*state=UP;
	      }
	      (*handler->function)(handler->data,*state);
	    }
	  } // for all values
	  JOY_DBG(logvs::DBG, "Joystick hatswitch #%u hat:%d haxis:%d axeval:%g", hs_joy, h, hs_axis, axevalue);
	} // is available
      }
  }
}

