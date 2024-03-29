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

#include "vegastrike.h"
//#include "glob.h"
//#include "dbg.h"
#include "in_handler.h"
#include "in_mouse.h"
#include <deque>
#include "vs_globals.h"
#include "config_xml.h"
#include "in_joystick.h"
#include "gldrv/winsys.h"
using std::deque;
#define NUM_BUTTONS 15

/** Gets the button number of the function used to draw the mouse*/
int getMouseDrawFunc() {
  return NUM_BUTTONS;
}
KBSTATE MouseState [NUM_BUTTONS+1]= {RELEASE};
static MouseHandler mouseBindings [INSC_NB][NUM_BUTTONS+1];

int mousex=0;
int mousey=0;
void GetMouseXY(int &mx, int &my) {
  mx = mousex;
  my= mousey;
}
int getMouseButtonStatus() {
  int ret=0;
  for (int i=0;i<NUM_BUTTONS;i++) {
    ret |= (MouseState[i]==PRESS||MouseState[i]==DOWN)?(1<<i):0;
  }
  return ret;
}

static deque<MouseEvent> eventQueue;

void mouseClickQueue(int button, int state, int x, int y) {
  int mod = 0;
  //glutGetModifiers();
  eventQueue.push_back(MouseEvent(MouseEvent::CLICK, button, state, mod, x, y));
}
int delx=0;
int dely=0;
void AddDelta (int dx, int dy) {
  delx+=dx;
  dely+=dy;
}
int warpallowage=2;
void DealWithWarp (int x, int y) {
  static bool warp_pointer = XMLSupport::parse_bool(vs_config->getVariable ("joystick","warp_mouse","false"));
  static int mouse_warp_zone = XMLSupport::parse_int(vs_config->getVariable ("joystick","warp_mouse_zone","100"));
 if (warp_pointer) {
   if (joystick[MOUSE_JOYSTICK]->player<_Universe->numPlayers()) {
    if (x<mouse_warp_zone||y<mouse_warp_zone||x>g_game.x_resolution-mouse_warp_zone||y>g_game.y_resolution-mouse_warp_zone) {
      //VSFileSystem::Fprintf (stderr,"warped from %d %d to %d %d",mousex,mousey, g_game.x_resolution/2,g_game.y_resolution/2);
      
      int delx = -x+g_game.x_resolution/2;
      int dely = -y+g_game.y_resolution/2;
      mousex+=delx;
      mousey+=dely;
      deque<MouseEvent>::iterator i;
      for (i=eventQueue.begin();i!=eventQueue.end();i++) {
	i->x+=delx;
	i->y+=dely;
      }
      if (warpallowage-->=0) {
	winsys_warp_pointer(g_game.x_resolution/2,g_game.y_resolution/2);
      }
    }
   }
  }

}

void mouseDragQueue(int x, int y) {
  eventQueue.push_back(MouseEvent(MouseEvent::DRAG, -1, -1, -1, x, y));
  DealWithWarp(x,y);
}

void mouseMotionQueue(int x, int y) {
  
  eventQueue.push_back(MouseEvent(MouseEvent::MOTION, -1, -1, -1, x, y));
  DealWithWarp(x,y);



}
/*
void mouseClick( int button, int state, int x, int y ) {
  int mod = 0;//glutGetModifiers();
  unsigned int scope = inGetCurrentScope();
  if(button>=NUM_BUTTONS) return;

  mousex = x;
  mousey = y;
  mouseBindings[scope][button](state==WS_MOUSE_DOWN?PRESS:RELEASE,x,y,0,0,mod);
  MouseState[button]=(state==WS_MOUSE_DOWN)?DOWN:UP;
}
*/
int lookupMouseButton(int b) {
	static int adj=0;
	if (b+adj<WS_LEFT_BUTTON) {
		adj=WS_LEFT_BUTTON-b;
	}
	b+=adj;
  switch (b) {
  case WS_LEFT_BUTTON:
    return 0;
  case WS_RIGHT_BUTTON:
    return 2;
  case WS_MIDDLE_BUTTON:
    return 1;
  case WS_WHEEL_UP:
    return 3;
  case WS_WHEEL_DOWN:
    return 4;
  default:
    return ((b-WS_LEFT_BUTTON)>=NUM_BUTTONS)?NUM_BUTTONS-1:b-WS_LEFT_BUTTON;
  }
  return 0;
}
void mouseClick0( int button, int state, int mod, int x, int y ) {
  unsigned int scope = inGetCurrentScope();
  button = lookupMouseButton(button);
  VS_DBG("mouse", logvs::DBG, "mouseClick button=%d state=%d mod=%d, x=%d y=%d", button, state, mod, x, y);
  if(button>=NUM_BUTTONS) return;
  AddDelta(x-mousex,y-mousey);
  mousex = x;
  mousey = y;
  mouseBindings[scope][button](state==WS_MOUSE_DOWN?PRESS:RELEASE,x,y,0,0,mod);
  MouseState[button]=(state==WS_MOUSE_DOWN)?DOWN:UP;
}
void SetDelta (int dx, int dy) {
  delx=dx;
  dely=dy;
}
void GetMouseDelta (int &dx, int & dy) {
  dx = delx;
  dy = dely;
  delx=dely=0;
}

void  mouseDrag( int x, int y ) {
  unsigned int scope = inGetCurrentScope();
  //  int mod =glutGetModifiers();
  for (int i=0;i<NUM_BUTTONS+1;i++) {
    mouseBindings[scope][i](MouseState[i],x,y,x-mousex,y-mousey,0);
  }
  AddDelta(x-mousex,y-mousey);
  mousex = x;
  mousey = y;
  
}	

void mouseMotion(int x, int y) {
  unsigned int scope = inGetCurrentScope();
  //  int mod =glutGetModifiers();
  for (int i=0;i<NUM_BUTTONS+1;i++) {
    mouseBindings[scope][i](MouseState[i],x,y,x-mousex,y-mousey,0);
  }
  AddDelta(x-mousex,y-mousey);
 mousex = x;
 mousey = y;

}




/**
GLUT_ACTIVE_SHIFT 
    Set if the Shift modifier or Caps Lock is active. 
GLUT_ACTIVE_CTRL 
    Set if the Ctrl modifier is active. 
GLUT_ACTIVE_ALT 
    Set if the Alt modifier is active. 
*/
static void DefaultMouseHandler (KBSTATE, int x, int y, int delx, int dely,int mod) {
  return;
}

void UnbindMouse (int button, unsigned int scope) {
	for (int ibutt = (button < 0) ? 0 : button; ibutt < ((button < 0) ? NUM_BUTTONS : button+1); ++ibutt) {
		for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
			mouseBindings[iscp][ibutt]=DefaultMouseHandler;
		}
		MouseState[ibutt] = UP;
	}
}

void BindMouse (int button, unsigned int scope, MouseHandler handler) {
	for (int ibutt = (button < 0) ? 0 : button; ibutt < ((button < 0) ? NUM_BUTTONS : button+1); ++ibutt) {
		for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
			mouseBindings[iscp][ibutt]=handler;
		}
	}
	handler (RESET,mousex,mousey,0,0,0);
}

void RestoreMouse() {
  eventQueue.clear();
  winsys_set_mouse_func (mouseClickQueue);
  winsys_set_motion_func (mouseDragQueue);
  winsys_set_passive_motion_func(mouseMotionQueue);
}

void InitMouse(){
	for (int a=0;a<NUM_BUTTONS+1;a++) {
		for (unsigned int iscp = 0; iscp < INSC_ALL; ++iscp) {
			UnbindMouse (a, iscp);
		}
	}
	RestoreMouse();
}
				
void ProcessMouse () {
  warpallowage=2;
  if (eventQueue.size()) {
	  bool buttons[NUM_BUTTONS];
	  bool available = joystick[MOUSE_JOYSTICK]->isAvailable();
	  memset(buttons, 0, sizeof(buttons));
	  do {
		  MouseEvent e = eventQueue.front();
		  eventQueue.pop_front();
		  switch(e.type) {
		  case MouseEvent::CLICK:
			  mouseClick0(e.button, e.state, e.mod, e.x, e.y);
			  if (available) {
				  int button = lookupMouseButton(e.button);
				  VS_DBG("mouse", logvs::DBG+1, "Queue CLICK wsbut:%d button:%d state:%d cache:%d",
						  e.button, button, e.state, button<NUM_BUTTONS?buttons[button]:-1);
				  if (button < NUM_BUTTONS) {
					  // When we have button down and up in the same queue, we
					  // must send the down event, before pushing the event, to not have it discarded.
					  if (e.state == WS_MOUSE_DOWN) {
						  buttons[button] = true;
					  } else if (buttons[button]) { // WS_MOUSE_UP
						  joystick[MOUSE_JOYSTICK]->joy_buttons |= (1 << button);
						  ProcessJoystick(MOUSE_JOYSTICK);
					  }
				  }
			  }
			  break;
		  case MouseEvent::DRAG:
			  mouseDrag(e.x, e.y);
			  break;
		  case MouseEvent::MOTION:
			  mouseMotion(e.x, e.y);
			  break;
		  }
	  } while(eventQueue.size());

	  if (available) {
		  float x, y, z; int buttons;
		  joystick[MOUSE_JOYSTICK]->GetJoyStick(x,y,z,buttons);
		  JoystickQueuePush(MOUSE_JOYSTICK); //JoystickEvent(MOUSE_JOYSTICK));
	  }
  }
  /*
  for (int a=0;a<NUM_BUTTONS+1;a++) {
    mouseBindings[a](MouseState[a],mousex,mousey,0,0,0);
    }*/
}

