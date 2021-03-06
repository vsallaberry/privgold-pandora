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

#include <queue>
#include <list> 
//#include "glob.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "in_handler.h"
#include "gldrv/winsys.h"
#include "in_kb_data.h"
#include "log.h"
//#include "cmd_unit.h"
/*
extern queue<InputListener*> activationreqqueue;
extern list<InputListener*> listeners;

extern InputListener* activelistener;
*/

static void DefaultKBHandler(const KBData&,KBSTATE newState) {
	// do nothing
	return;
}
struct HandlerCall{
  KBHandler function;
  KBData data;
  HandlerCall() {
    function=DefaultKBHandler;
  }
};
static HandlerCall keyBindings [LAST_MODIFIER][WSK_LAST];
static unsigned int playerBindings [LAST_MODIFIER][WSK_LAST];
KBSTATE keyState [LAST_MODIFIER][WSK_LAST];

static void kbGetInput(int key, int modifiers, bool release, int x, int y){
  ///FIXME If key is out of array index range, do nothing. This is a quick hack, the underlying cause of invalid parameters ever being given should probably be fixed instead
  //if (key < 0 || key >= WSK_LAST) // check done before in glut_keyboard_cb
  //  return;

  int i=_Universe->CurrentCockpit();
  _Universe->SetActiveCockpit(playerBindings[modifiers][key]);


  if ((keyState[modifiers][key]==RESET||keyState[modifiers][key]==UP)&&!release)
    keyBindings[modifiers][key].function(keyBindings[modifiers][key].data,PRESS);
  if ((keyState[modifiers][key]==DOWN||keyState[modifiers][key]==RESET)&&release)
    keyBindings[modifiers][key].function(keyBindings[modifiers][key].data,RELEASE);
  keyState[modifiers][key] = release?UP:DOWN;
  _Universe->SetActiveCockpit(i);
}

static bool kbHasBinding(int key, int modifiers)
{
	static HandlerCall defaultHandler;
	return (keyBindings[modifiers][key].function != defaultHandler.function);
}

// modifiers: enum KB_MODIFIER_ENUM
bool HasKeyBinding(unsigned int key, unsigned int modifiers)
{
    static HandlerCall defaultHandler;
    if (key >= WSK_LAST)
        return false;
    unsigned int internal_mod = (modifiers & (WSK_MOD_LALT | WSK_MOD_RALT) ? KB_MOD_ALT : 0)
                              | (modifiers & (WSK_MOD_LSHIFT | WSK_MOD_RSHIFT) ? KB_MOD_SHIFT : 0)
                              | (modifiers & (WSK_MOD_LCTRL | WSK_MOD_RCTRL) ? KB_MOD_CTRL : 0);
    return (keyBindings[internal_mod][key].function != defaultHandler.function);
}

// The SDL KB unicode driver can identify the right shifted keys
// according to user keyboard layout. A trick is needed on SDL1,
// but it goes well with SDL2.
//#define QWERTY_US_SHIFT_TRANSLATE
#undef QWERTY_US_SHIFT_TRANSLATE

#ifdef QWERTY_US_SHIFT_TRANSLATE // does not work on azerty or non-us qwerty
static const char _lomap[] = "0123456789-=\';/.,`\\";
static const char _himap[] = ")!@#$%^&*(_+\":?><~|";
#endif

int shiftup (int ch) {
    if (ch == (ch&0xFF)) {
#ifdef QWERTY_US_SHIFT_TRANSLATE
    const char *c = strchr(_lomap,ch);
    if (c) 
	return _himap[c-_lomap]; else
#endif
	return toupper(ch);
  } else return ch;
}

int shiftdown (int ch) {
    if (ch == (ch&0xFF)) {
#ifdef QWERTY_US_SHIFT_TRANSLATE
    const char *c = strchr(_himap,ch);
    if (c) 
	return _lomap[c-_himap]; else
#endif
	return tolower(ch);
  } else return ch;
}

static unsigned int _activeModifiers=0;

void setActiveModifiers(unsigned int mask)
{
	_activeModifiers = mask;
}
#ifdef SDL_WINDOWING
# if !SDL_VERSION_ATLEAST(2,0,0)
void setActiveModifiersSDL(SDLMod mask)
# else
void setActiveModifiersSDL(SDL_Keymod mask)
# endif
{
    setActiveModifiers(
        ((mask&(KMOD_LSHIFT|KMOD_RSHIFT))?KB_MOD_SHIFT:0) |
        ((mask&(KMOD_LCTRL|KMOD_RCTRL))?KB_MOD_CTRL:0) |
        ((mask&(KMOD_LALT|KMOD_RALT))?KB_MOD_ALT:0) );
}
#endif
unsigned int getActiveModifiers()
{
    return _activeModifiers;
}
unsigned int pullActiveModifiers()
{
#ifdef SDL_WINDOWING
    setActiveModifiersSDL(SDL_GetModState());
#endif
    return getActiveModifiers();
}
unsigned int getModifier(const char* mod_name){
  if (mod_name[0]=='\0')
    return 0;
  unsigned int rv = 0;
  if (strstr(mod_name,"shift")||strstr(mod_name,"uppercase")||strstr(mod_name,"caps"))
    rv |= KB_MOD_SHIFT;
  if (strstr(mod_name,"ctrl")||strstr(mod_name,"cntrl")||strstr(mod_name,"control"))
    rv |= KB_MOD_CTRL;
  if (strstr(mod_name,"alt")||strstr(mod_name,"alternate"))
    rv |= KB_MOD_ALT;
  return rv;
}
int getModifier(bool alton, bool cntrlon, bool shifton) {
	return cntrlon?KB_MOD_CTRL:(alton?KB_MOD_ALT:(shifton?KB_MOD_SHIFT:0));
}
 void glut_keyboard_cb( unsigned int  ch,unsigned int mod, bool release, int x, int y ) 
{
  bool shifton=false;
  int alton=false;
  int ctrlon=false;
  unsigned int shiftup_ch, shiftdown_ch;
    
  unsigned int modmask = KB_MOD_MASK;

  VS_DBG("keyboard", logvs::DBG, "'%c' (%d,0x%x) (mod:%d release:%d)",
           ch < 127 ? ch : '?', ch, ch, mod, release);
  
  shiftup_ch = shiftup(ch);
  shiftdown_ch = shiftdown(ch);
    
  if ((WSK_MOD_LSHIFT==(mod&WSK_MOD_LSHIFT))||(WSK_MOD_RSHIFT==(mod&WSK_MOD_RSHIFT))) {
    // This is ugly, but we have to support legacy config files...
	// ...maybe add config option to disable this soooo ugly thing...
	if (!kbHasBinding(ch,KB_MOD_SHIFT)) {
		ch = shiftup_ch;
		modmask &= ~KB_MOD_SHIFT;
	}
	shifton=true;
  }
  if ((WSK_MOD_LALT==(mod&WSK_MOD_LALT))||(WSK_MOD_RALT==(mod&WSK_MOD_RALT))) {
    alton=true;
  }
  if ((WSK_MOD_LCTRL==(mod&WSK_MOD_LCTRL))||(WSK_MOD_RCTRL==(mod&WSK_MOD_RCTRL))) {
    ctrlon=true;
  }

  // Polling state
  setActiveModifiers( 
	    (shifton?KB_MOD_SHIFT:0)
	   |(alton  ?KB_MOD_ALT  :0)
	   |(ctrlon ?KB_MOD_CTRL :0)  );  

  // No Key State for unicode characters yet
  if (shiftup_ch >= WSK_LAST || shiftdown_ch >= WSK_LAST) {
      return ;
  }
  int curmod=getModifier(alton,ctrlon,shifton) & modmask;
  kbGetInput( ch, curmod,release, x, y );
  if (release) {
    for (int i=0;i<LAST_MODIFIER;++i) {
      if (i!=curmod){
        if(keyState[i][shiftdown_ch]==DOWN)
          kbGetInput (shiftdown_ch,i,release,x,y);
        if(keyState[i][shiftup_ch]==DOWN)
          kbGetInput (shiftup_ch,i,release,x,y);
      }else{
        if (shifton) {
          if (shiftdown_ch!=ch&&keyState[i][shiftdown_ch]==DOWN) {
            kbGetInput (shiftdown_ch,i,release,x,y);
          }
        }else {
          if (shiftup_ch!=ch&&keyState[i][shiftup_ch]==DOWN) {
            kbGetInput (shiftup_ch,i,release,x,y);
          }
        }
      }
    }
  }
}
/*
static void glut_special_cb( int key, int x, int y ) 
{
  //  VSFileSystem::Fprintf (stderr,"keyboard s %d",key);
    kbGetInput( 128+key, 1, 0, x, y );
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y ) 
{
  //  VSFileSystem::Fprintf (stderr,"keyboard up %d",ch);
    kbGetInput( ch, 0, 1, x, y );
}

static void glut_special_up_cb( int key, int x, int y ) 
{
  //  VSFileSystem::Fprintf (stderr,"keyboard s up %d",key);
    kbGetInput( 128+key, 1, 1, x, y );
}
*/
void RestoreKB() {
  for (int i=0;i<LAST_MODIFIER;++i) {
    for(int a=0; a<KEYMAP_SIZE; a++) {
      if (keyState[i][a]==DOWN) {
        keyBindings[i][a].function(keyBindings[i][a].data,RELEASE);      
        keyState[i][a] = UP;
      }
    }
  }
  winsys_set_keyboard_func( glut_keyboard_cb );
}

void InitKB()
{
  for (int i=0;i<LAST_MODIFIER;++i) {
    for(int a=0; a<KEYMAP_SIZE; a++) {
      keyState[i][a] = UP;
      UnbindKey(a,i);
    }
  }
  RestoreKB();
}


void ProcessKB(unsigned int player)
{
  /*  if(!activationreqqueue.empty()) {
    InputListener *newactive = NULL;
    list<InputListener*>::const_iterator li_it = listeners.begin();
    float min = FLT_MAX;
    
    while(li_it!=listeners.end()) { //pick the one with lowest z
      float curr_z = (*li_it)->parent->Position().k;
      if(curr_z<min ) {
	min = curr_z;
	newactive = *li_it;
      }
    }
    if(newactive!=NULL)
      activelistener = newactive;
    //empty & analyze to see which one deserves to be activated
    }*/
  for(int mod=0; mod<LAST_MODIFIER; mod++) {
    for(int a=0; a<KEYMAP_SIZE; a++) {
      if (playerBindings[mod][a]==player)
        keyBindings[mod][a].function(keyBindings[mod][a].data,keyState[mod][a]);
    }
  }
}	

void BindKey(unsigned int key,unsigned int mod, unsigned int player, KBHandler handler, const KBData&data) {
    if (key >= WSK_LAST || mod >= LAST_MODIFIER) {
        // TODO: unicode with hashmap
        VS_LOG("keyboard", logvs::WARN, "WARNING: Bindkey with key(%d)/mod(%d) out of bounds", key, mod);
        return ;
    }
    keyBindings[mod][key].function = handler;
	keyBindings[mod][key].data = data;
	playerBindings[mod][key]=player;
	handler(std::string(),RESET); // key is not used in handler
}

void UnbindKey(unsigned int key,unsigned int mod) {
  if (key >= WSK_LAST || mod >= LAST_MODIFIER) {
      // TODO: unicode with hashmap
      VS_LOG("keyboard", logvs::WARN, "WARNING: Unbindkey with key(%d)/mod(%d) out of bounds", key, mod);
      return ;
  }
  keyBindings[mod][key] = HandlerCall();
}
