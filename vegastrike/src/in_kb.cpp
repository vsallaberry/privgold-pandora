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
#include "cmd/base.h"

const std::string * const	KBData::_empty_string = new std::string();

static void DefaultKBHandler(const KBData&,KBSTATE newState) {
	// do nothing
	return;
}

struct HandlerCall{
  KBHandler function;
  KBData data;
  HandlerCall() : function(DefaultKBHandler) {}
};

static HandlerCall	keyBindings [INSC_NB][LAST_MODIFIER][KEYMAP_SIZE];
static unsigned int	playerBindings [LAST_MODIFIER][KEYMAP_SIZE];
KBSTATE				keyState [LAST_MODIFIER][KEYMAP_SIZE];

typedef vsUMap<std::pair<unsigned char,unsigned int>,unsigned int> PlayerBindingsMap;
typedef vsUMap<std::pair<unsigned char,unsigned int>,KBSTATE> KeyStateMap;
typedef vsUMap<std::pair<unsigned char,std::pair<unsigned char,unsigned int> >,HandlerCall> KeyBindingsMap;

#ifndef stdext
# define stdext std
#endif
namespace stdext {
template<> GNUHASH_CLASS hash<std::pair<unsigned char,unsigned int> /*PlayerBindingsMap::key_type*/> {
	hash<size_t> a;
public:
	size_t operator () (const /*PlayerBindingsMap::key_type*/ std::pair<unsigned char,unsigned int> &key) const{
		return a((size_t)(key.first) | (size_t)(((unsigned short)(key.second))<<4));
	}
};
template<> GNUHASH_CLASS hash</*KeyBindingsMap::key_type*/ std::pair<unsigned char,std::pair<unsigned char,unsigned int> > > {
	hash<size_t> a;
public:
	size_t operator () (const /*KeyBindingsMap::key_type*/ std::pair<unsigned char,std::pair<unsigned char,unsigned int> > &key) const{
		return (size_t)(a((size_t)(key.first) | (size_t)(((unsigned short)(key.second.first))<<4))
				        ^ a(((size_t)key.second.second)));
	}
};
}

static PlayerBindingsMap 	s_playerBindingsMap;
static KeyStateMap			s_keyStateMap;
static KeyBindingsMap 		s_keyBindingsMap;

static HandlerCall  s_defaultHandler;
static KBSTATE		s_defaultState;
static unsigned int	s_defaultPlayer;

unsigned int inGetCurrentScope() {
	return BaseInterface::CurrentBase == NULL ? INSC_COCKPIT : INSC_BASE;
}

static inline PlayerBindingsMap::key_type makePlayerBindingsKey(unsigned int key, unsigned int modifiers) {
	return std::make_pair((unsigned char)modifiers, key);
}
static inline KeyBindingsMap::key_type makeKeyBindingsKey(unsigned int key, unsigned int modifiers, unsigned int scope) {
	return std::make_pair((unsigned char)scope, std::make_pair((unsigned char)modifiers, key));
}
static inline KeyStateMap::key_type makeKeyStateKey(unsigned int key, unsigned int modifiers) {
	return std::make_pair((unsigned char)modifiers, key);
}

static inline KBSTATE & keyStateRef(unsigned int key, unsigned int modifiers) {
	if (key >= KEYMAP_SIZE) {
		KeyStateMap::iterator it = s_keyStateMap.find(makeKeyStateKey(key, modifiers));
		if (it != s_keyStateMap.end())
			return it->second;
		return s_defaultState;
	}
	return keyState[modifiers][key];
}
static inline HandlerCall & keyBindingsRef(unsigned int key, unsigned int modifiers, unsigned int scope) {
	if (key >= KEYMAP_SIZE) {
		KeyBindingsMap::iterator it = s_keyBindingsMap.find(makeKeyBindingsKey(key, modifiers, scope));
		if (it != s_keyBindingsMap.end())
			return it->second;
		return s_defaultHandler;
	}
	return keyBindings[scope][modifiers][key];
}
static inline unsigned int & playerBindingsRef(unsigned int key, unsigned int modifiers) {
	if (key >= KEYMAP_SIZE) {
		PlayerBindingsMap::iterator it = s_playerBindingsMap.find(makePlayerBindingsKey(key, modifiers));
		if (it != s_playerBindingsMap.end())
			return it->second;
		return s_defaultPlayer;
	}
	return playerBindings[modifiers][key];
}

void kbGetInput(int key, int modifiers, bool release, int x, int y){
	unsigned int scope = inGetCurrentScope();

	KBSTATE & state = keyStateRef(key, modifiers);
	HandlerCall & handler = keyBindingsRef(key, modifiers, scope);
	unsigned int & player = playerBindingsRef(key, modifiers);

	VS_DBG("keyboard", logvs::DBG, "kbGetInput '%c' (%d,0x%x) (mod:%d release:%d scope:%d state=%d)",
			key < 128 ? key : '?', key, key, modifiers, release, scope, state);

	int i=_Universe->CurrentCockpit();
	_Universe->SetActiveCockpit(player);//playerBindings[modifiers][key]);

	/*if ((keyState[modifiers][key]==RESET||keyState[modifiers][key]==UP)&&!release)
		keyBindings[scope][modifiers][key].function(keyBindings[scope][modifiers][key].data,PRESS);
	if ((keyState[modifiers][key]==DOWN||keyState[modifiers][key]==RESET)&&release)
		keyBindings[scope][modifiers][key].function(keyBindings[scope][modifiers][key].data,RELEASE);
	keyState[modifiers][key] = release?UP:DOWN; */

	if ((state==RESET||state==UP)&&!release)
		handler.function(handler.data,PRESS);
	if ((state==DOWN||state==RESET)&&release)
		handler.function(handler.data,RELEASE);
	state = release?UP:DOWN;

	_Universe->SetActiveCockpit(i);
}

// modifiers: enum KB_MODIFIER_ENUM
bool kbHasBinding(unsigned int key, unsigned int modifiers)
{
	unsigned int scope = inGetCurrentScope();
	VS_DBG("keyboard", logvs::DBG+1, "kbHasBindings(%x,mod:%x,scop:%x)?", key, modifiers, scope);
	bool has_bindings = (keyBindingsRef(key, modifiers, scope).function != s_defaultHandler.function);
	VS_DBG("keyboard", logvs::DBG+1, "kbHasBindings -> %d", has_bindings);
	return has_bindings;
}

// modifiers: enum WSK_*
bool kbHasBindingWSK(unsigned int key, unsigned int modifiers)
{
    unsigned int internal_mod = (modifiers & (WSK_MOD_LALT | WSK_MOD_RALT) ? KB_MOD_ALT : 0)
                              | (modifiers & (WSK_MOD_LSHIFT | WSK_MOD_RSHIFT) ? KB_MOD_SHIFT : 0)
                              | (modifiers & (WSK_MOD_LCTRL | WSK_MOD_RCTRL) ? KB_MOD_CTRL : 0);
    return kbHasBinding(key,internal_mod);//(keyBindings[internal_mod][key].function != defaultHandler.function);
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
	return (cntrlon?KB_MOD_CTRL:0) | (alton?KB_MOD_ALT:0) | (shifton?KB_MOD_SHIFT:0);
}

void glut_keyboard_cb( unsigned int  ch,unsigned int mod, bool release, int x, int y )
{
  bool shifton = 0 != (mod & (WSK_MOD_LSHIFT|WSK_MOD_RSHIFT));
  bool alton = 0 != (mod & (WSK_MOD_LALT|WSK_MOD_RALT));
  bool ctrlon = 0 != (mod & (WSK_MOD_LCTRL|WSK_MOD_RCTRL));

  unsigned int shiftup_ch, shiftdown_ch;
    
  unsigned int modmask = KB_MOD_MASK;

  VS_DBG("keyboard", logvs::DBG, "'%c' (%d,0x%x) (mod:%d release:%d)",
           ch < 127 ? ch : '?', ch, ch, mod, release);
  
  shiftup_ch = shiftup(ch);
  shiftdown_ch = shiftdown(ch);
    
  if (shifton) {
    // This is ugly, but we have to support legacy config files...
	// ...maybe add config option to disable this soooo ugly thing...
	if (!kbHasBinding(ch,getModifier(alton,ctrlon,shifton))) {
		ch = shiftup_ch;
		modmask &= ~KB_MOD_SHIFT;
	}
  }

  // Polling state
  setActiveModifiers( 
	    (shifton?KB_MOD_SHIFT:0)
	   |(alton  ?KB_MOD_ALT  :0)
	   |(ctrlon ?KB_MOD_CTRL :0)  );  

  int curmod=getModifier(alton,ctrlon,shifton) & modmask;

  kbGetInput( ch, curmod,release, x, y );
  if (release) {
    for (int i=0;i<LAST_MODIFIER;++i) {
      if (i!=curmod){
        if(keyStateRef(shiftdown_ch, i) == DOWN) //keyState[i][shiftdown_ch]==DOWN)
          kbGetInput (shiftdown_ch, i,release,x,y);
        if(keyStateRef(shiftup_ch, i) == DOWN) //keyState[i][shiftup_ch]==DOWN)
          kbGetInput (shiftup_ch,i,release,x,y);
      }else{
        if (shifton) {
          if (shiftdown_ch!=ch && keyStateRef(shiftdown_ch, i) == DOWN) { //keyState[i][shiftdown_ch]==DOWN) {
            kbGetInput (shiftdown_ch,i,release,x,y);
          }
        }else {
          if (shiftup_ch!=ch && keyStateRef(shiftup_ch, i) == DOWN) { //keyState[i][shiftup_ch]==DOWN) {
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
  unsigned int scope = inGetCurrentScope();
  for (unsigned s = 0; s < INSC_ALL; ++s) {
    for (int i=0;i<LAST_MODIFIER;++i) {
      for(int a=0; a<KEYMAP_SIZE; a++) {
        if (keyState[i][a]==DOWN) {
          if (s == scope) {
            keyBindings[s][i][a].function(keyBindings[s][i][a].data,RELEASE);
          }
          keyState[i][a] = UP;
        }
      }
    }
  }
  for (KeyBindingsMap::iterator it = s_keyBindingsMap.begin(); it != s_keyBindingsMap.end(); ++it) {
	  unsigned int s = it->first.first;
	  KBSTATE & stateRef = keyStateRef(it->first.second.second, it->first.second.first);
	  if (stateRef==DOWN) {
		  if (s == scope) {
	      	it->second.function(it->second.data,RELEASE);
	      }
	      stateRef = UP;
	  }
  }
  winsys_set_keyboard_func( glut_keyboard_cb );
}

void InitKB()
{
  VS_LOG("keyboard", logvs::INFO, "%s(): keyBindings:%zub(KBHandler:%zu,KBData:%zu)",
		  __func__, sizeof(keyBindings), sizeof(KBHandler), sizeof(KBData));
  for (unsigned s = 0; s < INSC_ALL; ++s) {
	  for (int i=0;i<LAST_MODIFIER;++i) {
		for(int a=0; a<KEYMAP_SIZE; a++) {
		  keyState[i][a] = UP;
		  UnbindKey(a, i, s);
		}
	  }
  }
  s_keyBindingsMap.clear();
  s_keyStateMap.clear();
  s_playerBindingsMap.clear();
  RestoreKB();
}


void ProcessKB(unsigned int player)
{
	unsigned int scope = inGetCurrentScope();
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
			if (playerBindingsRef(a, mod) == player) { //playerBindings[mod][a]==player)
				//keyBindings[scope][mod][a].function(keyBindings[scope][mod][a].data,keyState[mod][a]);
				HandlerCall & handler = keyBindingsRef(a, mod, scope);
				handler.function(handler.data,keyStateRef(a,mod));
			}
		}
	}

	for (KeyBindingsMap::iterator it = s_keyBindingsMap.begin(); it != s_keyBindingsMap.end(); ++it) {
		unsigned int s = it->first.first;
		unsigned int key = it->first.second.second;
		unsigned int mod = it->first.second.first;
		KBSTATE & stateRef = keyStateRef(key, mod);
		if (s == scope && playerBindingsRef(key, mod) == player) { //playerBindings[mod][a]==player)
			HandlerCall & handler = keyBindingsRef(key, mod, scope);
			handler.function(handler.data,keyStateRef(key,mod));
		}
	}
}	

void BindKey(unsigned int key,unsigned int mod, unsigned int player, unsigned int scope, KBHandler handler, const KBData&data) {
    if (mod >= LAST_MODIFIER) {
        VS_LOG("keyboard", logvs::WARN, "WARNING: Bindkey with key(%u)/mod(%u) out of bounds", key, mod);
        return ;
    }
    if (key >= KEYMAP_SIZE) {
    	s_keyStateMap[makeKeyStateKey(key, mod)] = UP;
    	s_playerBindingsMap[makePlayerBindingsKey(key, mod)] = player;
    }
    for (unsigned int iscp = scope >= INSC_ALL ? 0 : scope; iscp < (scope >= INSC_ALL ? INSC_ALL : scope+1); ++iscp) {
    	if (key >= KEYMAP_SIZE) {
    	    s_keyBindingsMap[makeKeyBindingsKey(key, mod, iscp)] = HandlerCall();
    	}
    	/*keyBindings[i][mod][key].function = handler;
		keyBindings[i][mod][key].data = data;
		playerBindings[mod][key]=player;*/
    	HandlerCall & handlerRef = keyBindingsRef(key, mod, iscp);
    	unsigned int & playerRef = playerBindingsRef(key, mod);

    	handlerRef.function = handler;
		handlerRef.data = data;
		playerRef = player;
		handler(std::string(),RESET); // key is not used in handler
		VS_DBG("keyboard", logvs::DBG, "BindKey(%c/%x,mod:%x,ply:%u,scp:%u)",key,key,mod,player,iscp);
    }
}

void UnbindKey(unsigned int key, unsigned int mod, unsigned int scope) {
  if (mod >= LAST_MODIFIER) {
      VS_LOG("keyboard", logvs::WARN, "WARNING: Unbindkey with key(%u)/mod(%u) out of bounds", key, mod);
      return ;
  }
  for (unsigned int iscp = (scope >= INSC_ALL) ? 0 : scope; iscp < ((scope >= INSC_ALL) ? INSC_ALL : scope+1); ++iscp) {
	  //keyBindings[i][scope][key] = HandlerCall();
	  keyBindingsRef(key, mod, iscp) = HandlerCall();
  }
}
