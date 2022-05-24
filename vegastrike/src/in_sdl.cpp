#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if defined(HAVE_SDL)
# include <SDL.h>
#endif
#include "in_joystick.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "in_kb_data.h"
#include <assert.h>	/// needed for assert() calls.
#include "vs_log_modules.h"

#if !defined(HAVE_SDL) && !defined(SDL_HAT_LEFT)
# define SDL_HAT_LEFT 		(1 << 0)
# define SDL_HAT_RIGHT 		(1 << 1)
# define SDL_HAT_UP 		(1 << 2)
# define SDL_HAT_DOWN 		(1 << 3)
# define SDL_HAT_LEFTUP		(SDL_HAT_LEFT	| SDL_HAT_UP)
# define SDL_HAT_RIGHTUP	(SDL_HAT_RIGHT	| SDL_HAT_UP)
# define SDL_HAT_LEFTDOWN	(SDL_HAT_LEFT	| SDL_HAT_DOWN)
# define SDL_HAT_RIGHTDOWN	(SDL_HAT_RIGHT	| SDL_HAT_DOWN)
#endif

  /*KBData & operator=(const KBData & data) {
	  if (this == &data) return *this;
	  destruct(); _data = init_data(*data._data); return *this;
  }*/

class JSData : public KBData {
public:
  JSData(const KBData & data, unsigned int id) : KBData(data), _id(id) {}
  unsigned int type() const 	{ return TYPE_JS; }
  unsigned int id() const 		{ return _id; }
protected:
  unsigned int _id;
};

class MSData : public KBData {
public:
  MSData(const KBData & data)	: KBData(data) {}
  unsigned int type() const 	{ return TYPE_MS; }
};

void DefaultJoyHandler (const KBData&, KBSTATE newState) {
  //  VSFileSystem::Fprintf (stderr,"STATE: %d", st);
}

struct JSHandlerCall {
  KBHandler function;
  KBData * data;
  JSHandlerCall() : function(DefaultJoyHandler), data(default_data) {
	  JOY_DBG(logvs::DBG+2, "JSH CONTRUCT() %p", this);
  }
  JSHandlerCall(KBHandler _function, KBData * _data)
  : function(_function?_function:DefaultJoyHandler), data(_data?_data:default_data) {
	  JOY_DBG(logvs::DBG+2, "JSH CONSTRUCT(...) %p", this);
  }
  virtual ~JSHandlerCall() { JOY_DBG(logvs::DBG+2, "JSH DESTRUCT %p",this); if (data != default_data) delete data; }
  JSHandlerCall & operator=(JSHandlerCall src) { // copy-and-swap assignment
	  JOY_DBG(logvs::DBG+2, "JSH OP= [%p=%p]\n", this,&src);
	  std::swap(src.function, function);
	  std::swap(src.data, data); // when src is deleted it will clean old data of *this
	  return *this;
  }
  static KBData * default_data;
};
KBData * JSHandlerCall::default_data = new KBData();

enum JSSwitches{
  JOYSTICK_SWITCH,
  HATSWITCH,
  DIGHATSWITCH,
  NUMSWITCHES
};

#define MAXOR(A,B) (((A)<(B))?(B):(A))

static const size_t s_joy_nb 		= MAXOR(MAX_HATSWITCHES,MAX_JOYSTICKS);
static const size_t s_joy_key_nb	= MAXOR(NUMJBUTTONS,MAXOR(MAX_VALUES,MAX_DIGITAL_HATSWITCHES*MAX_DIGITAL_VALUES));

JSHandlerCall 	JoystickBindings 			[INSC_NB] [NUMSWITCHES] [s_joy_nb] [s_joy_key_nb];
KBSTATE 		JoystickState 						  [NUMSWITCHES] [s_joy_nb] [s_joy_key_nb];
char 			JoystickAxisDigitalHatSync 	[INSC_NB] [MAX_JOYSTICKS];

static void GenUnbindJoyKey (JSSwitches whichswitch, int joystick, int key, unsigned int scope) {
	if (!(key<(int)s_joy_key_nb && joystick < (int)s_joy_nb && whichswitch < NUMSWITCHES)) {
		JOY_LOG(logvs::WARN, "unbind joystick switch(%d)/joy(%d)/key(%d)/scope(%u) out of bound", whichswitch, joystick, key, scope);
		return ;
	}
	for (int ijoy = (joystick >= 0) ? joystick : 0; ijoy < ((joystick >= 0) ? joystick+1 : s_joy_nb); ++ijoy) {
		if (ijoy != joystick && ijoy == MOUSE_JOYSTICK) continue ;
		for (int ikey = (key < 0) ? 0 : key; ikey < ((key < 0) ? s_joy_key_nb : key+1); ++ikey) {
			for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
				JOY_DBG(logvs::DBG+1, "JSH UNBIND copy joy%d sw%d key%d scope%d\n",ijoy,whichswitch,ikey,iscp);
				JoystickBindings[iscp][whichswitch][ijoy][ikey]=JSHandlerCall();
			}
			JoystickState[whichswitch][ijoy][ikey]=UP;
		}
	}
}

static void GenBindJoyKey (JSSwitches whichswitch,int joystick, int key, unsigned int scope, KBHandler handler, const KBData &data) {
	if (!(key<(int)s_joy_key_nb && joystick < (int)s_joy_nb && whichswitch < NUMSWITCHES)) {
		JOY_LOG(logvs::WARN, "bind joystick switch(%d)/joy(%d)/key(%d)/scope(%u) out of bound", whichswitch, joystick, key, scope);
		return ;
	}
	for (int ijoy = (joystick >= 0) ? joystick : 0; ijoy < ((joystick >= 0) ? joystick+1 : s_joy_nb); ++ijoy) {
		if (ijoy != joystick && ijoy == MOUSE_JOYSTICK) continue ;
		for (int ikey = (key < 0) ? 0 : key; ikey < ((key < 0) ? s_joy_key_nb : key+1); ++ikey) {
			for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
				JOY_DBG(logvs::DBG+1, "JSH BIND copy joy%d sw%d key%d scope%d\n",ijoy,whichswitch,ikey,iscp);
				JoystickBindings[iscp][whichswitch][ijoy][ikey]
					=JSHandlerCall(handler,ijoy == MOUSE_JOYSTICK ? (KBData*)new MSData(data) : (KBData*)new JSData(data,ijoy));
			}
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
	if (dir >= MAX_DIGITAL_VALUES) {
		for (int ijoy = (joystick >= 0) ? joystick : 0; ijoy < ((joystick >= 0) ? joystick+1 : MAX_JOYSTICKS); ++ijoy) {
			if (ijoy != joystick && ijoy == MOUSE_JOYSTICK) continue ;
			for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
				JoystickAxisDigitalHatSync[iscp][ijoy] = -1;
			}
		}
		return ;
	}
	GenUnbindJoyKey(DIGHATSWITCH,joystick,key*MAX_DIGITAL_VALUES+dir,scope);
}

void BindDigitalHatswitchKey (int joystick, int key, int dir, unsigned int scope, KBHandler handler, const KBData &data) {
  if (dir >= MAX_DIGITAL_VALUES) {
	  for (int ijoy = (joystick >= 0) ? joystick : 0; ijoy < ((joystick >= 0) ? joystick+1 : MAX_JOYSTICKS); ++ijoy) {
		  if (ijoy != joystick && ijoy == MOUSE_JOYSTICK) continue ;
		  for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
			  JoystickAxisDigitalHatSync[iscp][ijoy] = key;
		  }
	  }
	  return ;
  }
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

      // Optional synchronization of Joy axes and digital hat, done if:
      // 1. enabled, 2a. Hat is centered or 2b. joystick has not this hat
      char sync_hat_nr = JoystickAxisDigitalHatSync[scope][i];

      if (sync_hat_nr >= 0 && sync_hat_nr < MAX_DIGITAL_HATSWITCHES
      &&  (joystick[i]->digital_hat[sync_hat_nr] == VS_HAT_CENTERED || sync_hat_nr >= joystick[i]->nr_of_hats)) {
    	  const float x_margin = 0.8, y_margin = 0.8;
    	  joystick[i]->digital_hat[sync_hat_nr] = VS_HAT_CENTERED;
    	  if (fabs(x) >= x_margin) { // && x*x > y*y) {
    		  joystick[i]->digital_hat[sync_hat_nr] |= ((joystick[i]->axis_inverse[0]?-1:1) * x > 0) ? SDL_HAT_RIGHT : SDL_HAT_LEFT;
    	  }
    	  if (fabs(y) >= y_margin) { // && y*y > x*x) {
    		  joystick[i]->digital_hat[sync_hat_nr] |= ((joystick[i]->axis_inverse[1]?-1:1) * y >0) ? SDL_HAT_DOWN : SDL_HAT_UP;
    	  }
      } else sync_hat_nr = -1;

      for(int h = 0; h < MAXOR(joystick[i]->nr_of_hats, sync_hat_nr+1); ++h) {
#ifdef HAVE_SDL
	Uint8 
#else
	  unsigned char
#endif
	  hsw=joystick[i]->digital_hat[h];
      JOY_DBG(logvs::DBG+2, "joy %d dighat %d mask=%x", i, h, hsw);
	for(int dir_index=0;dir_index<MAX_DIGITAL_VALUES;dir_index++){
	  bool press=false;

	  // CENTERED is an exact position.
	  if(dir_index==VS_HAT_CENTERED && (hsw == SDL_HAT_CENTERED)){
	    JOY_DBG(logvs::DBG+2, "joy %d hat %d centered", i, h);
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

	  JOY_DBG(logvs::DBG+1, "joy%d HAT%d dir=%d mask=%x press=%d", i, h, dir_index, hsw, press);
          KBSTATE * state
            =&JoystickState[DIGHATSWITCH][i][h*MAX_DIGITAL_VALUES+dir_index];
          JSHandlerCall* handler
            =&JoystickBindings[scope][DIGHATSWITCH][i][h*MAX_DIGITAL_VALUES+dir_index];
	  if(press==true){
	    if(*state==UP){
	    	JOY_DBG(logvs::DBG, "joy%d HAT%d dir=%d CALL HDLR PRESS default:%d", i, h, dir_index, handler->function == DefaultJoyHandler);
	      (*handler->function)
                (*handler->data,PRESS);
	      *state=DOWN;
	    }
	  }
	  else{
	    if(*state==DOWN){
	    	JOY_DBG(logvs::DBG, "joy%d HAT%d dir=%d CALL HDLR RELEASE default:%d", i, h, dir_index, handler->function == DefaultJoyHandler);
	      (*handler->function)
                (*handler->data,RELEASE);
	    }
	    *state=UP;
	  }
	  (*handler->function) (*handler->data,*state);
	}
      } // digital_hatswitch
      
      for (int j=0;j<NUMJBUTTONS;j++) {
        KBSTATE * state = &JoystickState[JOYSTICK_SWITCH][i][j];
        JSHandlerCall*handler=&JoystickBindings [scope][JOYSTICK_SWITCH][i][j];
	if ((buttons&(1<<j))) {
	  if (*state==UP) {
	    (*handler->function)
              (*handler->data,PRESS);
	    *state=DOWN;
          }
	}else {
	  if (*state==DOWN) {
	    (*handler->function)(*handler->data,RELEASE);
	  }
	  *state=UP;
	}
	(*handler->function) (*handler->data,*state);
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
		  (*handler->function)(*handler->data,PRESS);
		  *state=DOWN;
		}
	      }
	      else{
		// not pressed
		if(*state==DOWN){
		  (*handler->function)(*handler->data,RELEASE);
		}
		*state=UP;
	      }
	      (*handler->function)(*handler->data,*state);
	    }
	  } // for all values
	  JOY_DBG(logvs::DBG, "Joystick hatswitch #%u hat:%d haxis:%d axeval:%g", hs_joy, h, hs_axis, axevalue);
	} // is available
      }
  }
}

