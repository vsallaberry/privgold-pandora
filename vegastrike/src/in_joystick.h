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
#include "config.h"
#include "in_kb_data.h"
#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#if defined(HAVE_SDL)
#include <SDL.h>
#endif

#include "vegastrike.h"
//#include "glob.h"
//#include "dbg.h"
#include "in_kb.h"
#define NUMJBUTTONS 32

class JoyStick;

//typedef void (*JoyHandler) (int);


extern void ProcessJoystick(int whichjoystick);
extern void InitJoystick();
extern void UpdateJoystick(int index);
extern void DeInitJoystick();

const int MAX_JOYSTICKS=16;
const int MOUSE_JOYSTICK = MAX_JOYSTICKS-1;
const int MAX_BUTTONS=48;
const int MAX_DIGITAL_HATSWITCHES=4;

enum { VS_HAT_CENTERED=0,VS_HAT_LEFT,VS_HAT_RIGHT,VS_HAT_DOWN,VS_HAT_UP,VS_HAT_RIGHTUP,VS_HAT_RIGHTDOWN,VS_HAT_LEFTUP,VS_HAT_LEFTDOWN,MAX_DIGITAL_VALUES };


extern JoyStick *joystick[MAX_JOYSTICKS];

class JoyStick {
  bool mouse;
  void InitMouse (int i);
  void GetMouse (float &x,float &y, float &z, int &buttons);
    public:
    // initializes the joystick
    JoyStick(int);
    void Update(int player, int index);
    virtual ~JoyStick();
    // engine calls GetJoyStick to get coordinates and buttons
    void GetJoyStick(float &x,float &y, float &z, int &buttons);
    bool isAvailable(void);
    bool is_around(float axe, float hswitch);
    int NumButtons();

#if defined(HAVE_SDL)
    SDL_Joystick *joy;
#else
    void *otherdata;//bad form to have an ifdef in a struct
#endif
    int nr_of_axes,nr_of_buttons,nr_of_hats;
    int hat_margin;
    int player;
#define MAX_AXES 32
    bool axis_inverse[MAX_AXES];
    int axis_axis[MAX_AXES];
    float joy_axis[MAX_AXES];
    JoyStick();
#if defined(IRIX)	// could be POSIX type uchar_t?
	uchar_t digital_hat[MAX_DIGITAL_HATSWITCHES];
#else
    unsigned char digital_hat[MAX_DIGITAL_HATSWITCHES];
#endif

    int joy_buttons;
    bool joy_available;
    float joy_xmin,joy_xmax,joy_ymin,joy_ymax, joy_zmin, joy_zmax;
    float joy_x,joy_y,joy_z;
    float  deadzone;
}
;


extern JoyStick *joystick[MAX_JOYSTICKS];
typedef void (*JoyHandler)(KBSTATE,float x, float y, int mod);

struct JoystickEvent {
	int which;
	unsigned int button, state, modifier;
	float x, y, z;
	JoystickEvent(int _which, unsigned int _button = 0, unsigned int _state = 0, unsigned int _modifier = 0,
			      float _x = 0., float _y = 0., float _z = 0.)
	: which(_which), button(_button), state(_state), modifier(_modifier), x(_x), y(_y), z(_z) {}
};

void BindJoyKey (int key, int joystick, unsigned int scope, KBHandler handler, const KBData&data);
void UnbindJoyKey (int joystick, int key, unsigned int scope);

void UnbindHatswitchKey (int hatswitch, int val_index, unsigned int scope);
void BindHatswitchKey (int hatswitch, int val_index, unsigned int scope, KBHandler handler, const KBData&data);

void BindDigitalHatswitchKey (int joystick,int hatswitch, int dir_index, unsigned int scope, KBHandler handler, const KBData&data);
void UnbindDigitalHatswitchKey (int joystick,int hatswitch, int dir_index, unsigned int scope);

unsigned int GetNumJoysticks();
int GetJoystickByID(int id);

void RestoreJoystick();
void JoystickGameHandler(unsigned int which, float x, float y, float z, unsigned int buttons, unsigned int state);
void JoystickProcessQueue(int player);
void JoystickQueuePush(unsigned int which);
void JoystickQueuePush(const JoystickEvent & joydata);

#endif // _JOYSTICK_H_
