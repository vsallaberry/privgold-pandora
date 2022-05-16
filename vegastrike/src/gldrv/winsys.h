/* 
 * Tux Racer 
 * Copyright (C) 1999-2001 Jasmin F. Patry
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
#ifndef WINSYS_H
#define WINSYS_H 1
#include "config.h"
#ifndef UCHAR_MAX
#define UCHAR_MAX 255
#endif
#define HAVE_GLUT
#ifndef HAVE_SDL
#undef SDL_WINDOWING
#endif

#if defined( SDL_WINDOWING ) && defined (HAVE_SDL)
#   include <SDL.h>
#elif defined( HAVE_GLUT )
#if defined(__APPLE__) || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#else
#   error "Neither SDL nor GLUT are present."
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Keysyms */
#define WSK_UTF32_TO_CODE(ch32) ((((unsigned int)(ch32)) >= 128) \
                                 ? (WSK_LAST + (unsigned int)(ch32) - 128) : (ch32))
#define WSK_CODE_IS_UTF32(code) (((unsigned int)(code)) >= WSK_LAST)
#define WSK_CODE_TO_UTF32(code) (WSK_CODE_IS_UTF32(code) ? ((unsigned int)(code) - WSK_LAST + 128): (code))

#if defined( SDL_WINDOWING ) && defined (HAVE_SDL) 
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* SDL version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#if SDL_VERSION_ATLEAST(2,0,0)
//-------------------
// SDL2
//-------------------
// the in_kb.cpp/BindKey and XmlConfig::checkBind considers any keystr from config
// with length greater that 1 as a special key (up/down/...) then not unicode.
// Then it is safe here to assume key sym won't be > 127, and reserve 128 syms plus
// SDL_NUM_SCANCODES scancodes for the bindings. Then it is still possible to
// handle unicode characters (without binding) as they will have a value > WSK_LAST.
#define WSK_KEY_OFFSET (128)
#define WSK_KEY(x) ((x) + WSK_KEY_OFFSET)
typedef enum {
    WSK_NOT_AVAIL = SDLK_UNKNOWN,

    /* Numeric keypad */
    WSK_KP0 = WSK_KEY(SDL_SCANCODE_KP_0),
    WSK_KP1 = WSK_KEY(SDL_SCANCODE_KP_1),
    WSK_KP2 = WSK_KEY(SDL_SCANCODE_KP_2),
    WSK_KP3 = WSK_KEY(SDL_SCANCODE_KP_3),
    WSK_KP4 = WSK_KEY(SDL_SCANCODE_KP_4),
    WSK_KP5 = WSK_KEY(SDL_SCANCODE_KP_5),
    WSK_KP6 = WSK_KEY(SDL_SCANCODE_KP_6),
    WSK_KP7 = WSK_KEY(SDL_SCANCODE_KP_7),
    WSK_KP8 = WSK_KEY(SDL_SCANCODE_KP_8),
    WSK_KP9 = WSK_KEY(SDL_SCANCODE_KP_9),

    WSK_KP_PERIOD = WSK_KEY(SDL_SCANCODE_KP_PERIOD),
    WSK_KP_DIVIDE = WSK_KEY(SDL_SCANCODE_KP_DIVIDE),
    WSK_KP_MULTIPLY = WSK_KEY(SDL_SCANCODE_KP_MULTIPLY),
    WSK_KP_MINUS = WSK_KEY(SDL_SCANCODE_KP_MINUS),
    WSK_KP_PLUS = WSK_KEY(SDL_SCANCODE_KP_PLUS),
    WSK_KP_ENTER = WSK_KEY(SDL_SCANCODE_KP_ENTER),
    WSK_KP_EQUALS = WSK_KEY(SDL_SCANCODE_KP_EQUALS),
	
	WSK_RETURN = 13,
	WSK_TAB = '\t',
	WSK_ESCAPE = 27,
#if 1
	WSK_BACKSPACE = SDLK_BACKSPACE,
    WSK_DELETE = SDLK_DELETE,
#else
	WSK_BACKSPACE =
# ifdef __APPLE__
    127,
# else
    8,
# endif
	WSK_DELETE = 
# ifdef __APPLE__
    8,
# else
    127,
# endif
#endif
    /* Arrows + Home/End pad */
    WSK_UP = WSK_KEY(SDL_SCANCODE_UP),
    WSK_DOWN = WSK_KEY(SDL_SCANCODE_DOWN),
    WSK_RIGHT = WSK_KEY(SDL_SCANCODE_RIGHT),
    WSK_LEFT = WSK_KEY(SDL_SCANCODE_LEFT),
    WSK_INSERT = WSK_KEY(SDL_SCANCODE_INSERT),
    WSK_HOME = WSK_KEY(SDL_SCANCODE_HOME),
    WSK_END = WSK_KEY(SDL_SCANCODE_END),
    WSK_PAGEUP = WSK_KEY(SDL_SCANCODE_PAGEUP),
    WSK_PAGEDOWN = WSK_KEY(SDL_SCANCODE_PAGEDOWN),

    /* Function keys */
    WSK_F1 = WSK_KEY(SDL_SCANCODE_F1),
    WSK_F2 = WSK_KEY(SDL_SCANCODE_F2),
    WSK_F3 = WSK_KEY(SDL_SCANCODE_F3),
    WSK_F4 = WSK_KEY(SDL_SCANCODE_F4),
    WSK_F5 = WSK_KEY(SDL_SCANCODE_F5),
    WSK_F6 = WSK_KEY(SDL_SCANCODE_F6),
    WSK_F7 = WSK_KEY(SDL_SCANCODE_F7),
    WSK_F8 = WSK_KEY(SDL_SCANCODE_F8),
    WSK_F9 = WSK_KEY(SDL_SCANCODE_F9),
    WSK_F10 = WSK_KEY(SDL_SCANCODE_F10),
    WSK_F11 = WSK_KEY(SDL_SCANCODE_F11),
    WSK_F12 = WSK_KEY(SDL_SCANCODE_F12),
    WSK_F13 = WSK_KEY(SDL_SCANCODE_F13),
    WSK_F14 = WSK_KEY(SDL_SCANCODE_F14),
    WSK_F15 = WSK_KEY(SDL_SCANCODE_F15),

    /* Key state modifier keys */
    WSK_NUMLOCK = WSK_NOT_AVAIL, //FIXME
    WSK_CAPSLOCK = WSK_KEY(SDL_SCANCODE_CAPSLOCK),
    WSK_SCROLLOCK = WSK_KEY(SDL_SCANCODE_SCROLLLOCK),

    WSK_RSHIFT = WSK_KEY(SDL_SCANCODE_RSHIFT),
    WSK_LSHIFT = WSK_KEY(SDL_SCANCODE_LSHIFT),
    WSK_RCTRL = WSK_KEY(SDL_SCANCODE_RCTRL),
    WSK_LCTRL = WSK_KEY(SDL_SCANCODE_LCTRL),
    WSK_RALT = WSK_KEY(SDL_SCANCODE_RALT),
    WSK_LALT = WSK_KEY(SDL_SCANCODE_LALT),
    WSK_RMETA = WSK_KEY(SDL_SCANCODE_RGUI),
    WSK_LMETA = WSK_KEY(SDL_SCANCODE_LGUI),
    WSK_BREAK = WSK_NOT_AVAIL, //FIXME
    
    WSK_PAUSE = WSK_KEY(SDL_SCANCODE_PAUSE),

    WSK_LAST=SDL_NUM_SCANCODES + WSK_KEY_OFFSET,
} winsys_keysym_t;
#else
//-------------------
// SDL1
//-------------------
typedef enum {
    WSK_NOT_AVAIL = SDLK_UNKNOWN,

    /* Numeric keypad */
    WSK_KP0 = SDLK_KP0,
    WSK_KP1 = SDLK_KP1,
    WSK_KP2 = SDLK_KP2,
    WSK_KP3 = SDLK_KP3,
    WSK_KP4 = SDLK_KP4,
    WSK_KP5 = SDLK_KP5,
    WSK_KP6 = SDLK_KP6,
    WSK_KP7 = SDLK_KP7,
    WSK_KP8 = SDLK_KP8,
    WSK_KP9 = SDLK_KP9,

    WSK_KP_PERIOD = SDLK_KP_PERIOD,
    WSK_KP_DIVIDE = SDLK_KP_DIVIDE,
    WSK_KP_MULTIPLY = SDLK_KP_MULTIPLY,
    WSK_KP_MINUS = SDLK_KP_MINUS,
    WSK_KP_PLUS = SDLK_KP_PLUS,
    WSK_KP_ENTER = SDLK_KP_ENTER,
    WSK_KP_EQUALS = SDLK_KP_EQUALS,
    
    WSK_RETURN = 13,
    WSK_TAB = '\t',
    WSK_ESCAPE = 27,
#if 1
	WSK_BACKSPACE = SDLK_BACKSPACE,
    WSK_DELETE = SDLK_DELETE,
#else
    WSK_BACKSPACE =
#ifdef __APPLE__
    127,
#else
    8,
#endif
    WSK_DELETE =
#ifdef __APPLE__
    8,
#else
    127,
#endif
#endif
    /* Arrows + Home/End pad */
    WSK_UP = SDLK_UP,
    WSK_DOWN = SDLK_DOWN,
    WSK_RIGHT = SDLK_RIGHT,
    WSK_LEFT = SDLK_LEFT,
    WSK_INSERT = SDLK_INSERT,
    WSK_HOME = SDLK_HOME,
    WSK_END = SDLK_END,
    WSK_PAGEUP = SDLK_PAGEUP,
    WSK_PAGEDOWN = SDLK_PAGEDOWN,

    /* Function keys */
    WSK_F1 = SDLK_F1,
    WSK_F2 = SDLK_F2,
    WSK_F3 = SDLK_F3,
    WSK_F4 = SDLK_F4,
    WSK_F5 = SDLK_F5,
    WSK_F6 = SDLK_F6,
    WSK_F7 = SDLK_F7,
    WSK_F8 = SDLK_F8,
    WSK_F9 = SDLK_F9,
    WSK_F10 = SDLK_F10,
    WSK_F11 = SDLK_F11,
    WSK_F12 = SDLK_F12,
    WSK_F13 = SDLK_F13,
    WSK_F14 = SDLK_F14,
    WSK_F15 = SDLK_F15,

    /* Key state modifier keys */
    WSK_NUMLOCK = SDLK_NUMLOCK,

    WSK_CAPSLOCK = SDLK_CAPSLOCK,
    WSK_SCROLLOCK = SDLK_SCROLLOCK,
    WSK_RSHIFT = SDLK_RSHIFT,
    WSK_LSHIFT = SDLK_LSHIFT,
    WSK_RCTRL = SDLK_RCTRL,
    WSK_LCTRL = SDLK_LCTRL,
    WSK_RALT = SDLK_RALT,
    WSK_LALT = SDLK_LALT,
    WSK_RMETA = SDLK_RMETA,
    WSK_LMETA = SDLK_LMETA,
    WSK_BREAK = SDLK_BREAK,
    
    WSK_PAUSE = SDLK_PAUSE,

    WSK_LAST=SDLK_LAST
} winsys_keysym_t;
#endif //SDL1/2

typedef enum {
    WSK_MOD_NONE=KMOD_NONE,
	WSK_MOD_LSHIFT=KMOD_LSHIFT,
	WSK_MOD_RSHIFT=KMOD_RSHIFT,
	WSK_MOD_LCTRL=KMOD_LCTRL ,
	WSK_MOD_RCTRL=KMOD_RCTRL ,
	WSK_MOD_LALT=KMOD_LALT  ,
	WSK_MOD_RALT=KMOD_RALT  ,
#if !SDL_VERSION_ATLEAST(2,0,0)
    WSK_MOD_LMETA=KMOD_LMETA ,
	WSK_MOD_RMETA=KMOD_RMETA ,
#else
    WSK_MOD_LMETA = KMOD_LGUI,
    WSK_MOD_RMETA = KMOD_RGUI,
#endif
	WSK_MOD_NUM=KMOD_NUM   ,
	WSK_MOD_CAPS=KMOD_CAPS  ,
	WSK_MOD_MODE=KMOD_MODE  
} winsys_modifiers;

// mouse wheel events are only available with SDL 1.2.5 or later
#ifndef SDL_BUTTON_WHEELUP
# define SDL_BUTTON_WHEELUP 254
#endif
#ifndef SDL_BUTTON_WHEELDOWN
# define SDL_BUTTON_WHEELDOWN 255
#endif
typedef enum {
    WS_LEFT_BUTTON = SDL_BUTTON_LEFT,
    WS_MIDDLE_BUTTON = SDL_BUTTON_MIDDLE,
    WS_RIGHT_BUTTON = SDL_BUTTON_RIGHT,
	WS_WHEEL_UP = SDL_BUTTON_WHEELUP,
	WS_WHEEL_DOWN = SDL_BUTTON_WHEELDOWN
} winsys_mouse_button_t;

typedef enum {
    WS_MOUSE_DOWN = SDL_PRESSED,
    WS_MOUSE_UP = SDL_RELEASED
} winsys_button_state_t;

#else
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* GLUT version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* GLUT doesn't define as many keysyms as SDL; we map those to 
   WSK_NOT_AVAIL (0) */

typedef enum {
    WSK_NOT_AVAIL = 0,

    /* Numeric keypad */
    WSK_KP0 = 0,
    WSK_KP1 = 0,
    WSK_KP2 = 0,
    WSK_KP3 = 0,
    WSK_KP4 = 0,
    WSK_KP5 = 0,
    WSK_KP6 = 0,
    WSK_KP7 = 0,
    WSK_KP8 = 0,
    WSK_KP9 = 0,
    WSK_KP_PERIOD = 0,
    WSK_KP_DIVIDE = 0,
    WSK_KP_MULTIPLY = 0,
    WSK_KP_MINUS = 0,
    WSK_KP_PLUS = 0,
    WSK_KP_ENTER = 0,
    WSK_KP_EQUALS = 0,

	WSK_RETURN = 13,
	WSK_TAB = '\t',
	WSK_ESCAPE = 27,
	WSK_BACKSPACE =
#ifdef __APPLE__
    127,
#else
    8,
#endif
	WSK_DELETE = 
#ifdef __APPLE__
    8,
#else
    127,
#endif

    /* Arrows + Home/End pad */
    WSK_UP = GLUT_KEY_UP+128,
    WSK_DOWN = GLUT_KEY_DOWN+128,
    WSK_RIGHT = GLUT_KEY_RIGHT+128,
    WSK_LEFT = GLUT_KEY_LEFT+128,
    WSK_INSERT = GLUT_KEY_INSERT+128,
    WSK_HOME = GLUT_KEY_HOME+128,
    WSK_END = GLUT_KEY_END+128,
    WSK_PAGEUP = GLUT_KEY_PAGE_UP+128,
    WSK_PAGEDOWN = GLUT_KEY_PAGE_DOWN+128,

    /* Function keys */
    WSK_F1 = GLUT_KEY_F1+128,
    WSK_F2 = GLUT_KEY_F2+128,
    WSK_F3 = GLUT_KEY_F3+128,
    WSK_F4 = GLUT_KEY_F4+128,
    WSK_F5 = GLUT_KEY_F5+128,
    WSK_F6 = GLUT_KEY_F6+128,
    WSK_F7 = GLUT_KEY_F7+128,
    WSK_F8 = GLUT_KEY_F8+128,
    WSK_F9 = GLUT_KEY_F9+128,
    WSK_F10 = GLUT_KEY_F10+128,
    WSK_F11 = GLUT_KEY_F11+128,
    WSK_F12 = GLUT_KEY_F12+128,
    WSK_F13 = 0,
    WSK_F14 = 0,
    WSK_F15 = 0,

    /* Key state modifier keys */
    WSK_NUMLOCK = 0,
    WSK_CAPSLOCK = 0,
    WSK_SCROLLOCK = 0,
    WSK_RSHIFT = 0,
    WSK_LSHIFT = 0,
    WSK_RCTRL = 0,
    WSK_LCTRL = 0,
    WSK_RALT = 0,
    WSK_LALT = 0,
    WSK_RMETA = 0,
    WSK_LMETA = 0,
    WSK_BREAK = 0,
 	WSK_PAUSE = 0,
   WSK_LAST = UCHAR_MAX /* GLUT doesn't define a max key, but this is more
			    than enough as of version 3.7 */
} winsys_keysym_t;
typedef enum {
        WSK_MOD_NONE=0,
	WSK_MOD_LSHIFT=GLUT_ACTIVE_SHIFT,
	WSK_MOD_RSHIFT=GLUT_ACTIVE_SHIFT,
	WSK_MOD_LCTRL=GLUT_ACTIVE_CTRL ,
	WSK_MOD_RCTRL=GLUT_ACTIVE_CTRL ,
	WSK_MOD_LALT=GLUT_ACTIVE_ALT  ,
	WSK_MOD_RALT=GLUT_ACTIVE_ALT  ,
	WSK_MOD_LMETA=0 ,
	WSK_MOD_RMETA=0,
	WSK_MOD_NUM=0,
	WSK_MOD_CAPS=0,
	WSK_MOD_MODE=0
} winsys_modifiers;

typedef enum {
    WS_LEFT_BUTTON = GLUT_LEFT_BUTTON,
    WS_MIDDLE_BUTTON = GLUT_MIDDLE_BUTTON,
    WS_RIGHT_BUTTON = GLUT_RIGHT_BUTTON,
	WS_WHEEL_UP,
	WS_WHEEL_DOWN
} winsys_mouse_button_t;

typedef enum {
    WS_MOUSE_DOWN = GLUT_DOWN,
    WS_MOUSE_UP = GLUT_UP
} winsys_button_state_t;

#endif /* defined( SDL_WINDOWING ) */


typedef void (*winsys_display_func_t)();
typedef void (*winsys_idle_func_t)();
typedef void (*winsys_reshape_func_t)( int w, int h );
typedef void (*winsys_keyboard_func_t)( unsigned int key, unsigned int mod,
					bool release, int x, int y );
typedef void (*winsys_mouse_func_t)( int button, int state, int x, int y );
typedef void (*winsys_motion_func_t)( int x, int y );

typedef void (*winsys_atexit_func_t)( void );

typedef void (*winsys_joystick_func_t)(unsigned int n, float x, float y, float z, unsigned int button, unsigned int state);

void winsys_post_redisplay();
void winsys_set_display_func( winsys_display_func_t func );
void winsys_set_idle_func( winsys_idle_func_t func );
void winsys_set_reshape_func( winsys_reshape_func_t func );
void winsys_set_keyboard_func( winsys_keyboard_func_t func );
void winsys_set_mouse_func( winsys_mouse_func_t func );
void winsys_set_motion_func( winsys_motion_func_t func );
void winsys_set_passive_motion_func( winsys_motion_func_t func );
void winsys_set_joystick_func( winsys_joystick_func_t func );

void winsys_swap_buffers();
#define WS_KB_REPEAT_DISABLED           0   /* repeated keys are ignored */
#define WS_KB_REPEAT_ENABLED_DEFAULT    -1  /* default delay/interval values for repeated keys */
#define WS_KB_REPEAT_DELAY              500 /* ms delay before a non-repeated key can be repeated */
#define WS_KB_REPEAT_INTERVAL           50  /* ms delay before a repeated key can repeat again */
enum {
    WS_UNICODE_DISABLED     = 0,
    WS_UNICODE_ENABLED      = 1 << 0, // enabled but bindings have priority over unicode
    WS_UNICODE_FULL         = 1 << 1  // full unicode
};
void winsys_set_kb_mode(unsigned int unicode, int delay_ms, int interval_ms,
                        unsigned int * unicode_bak, int * delay_ms_bak, int * interval_ms_bak);
void winsys_warp_pointer( int x, int y );
void winsys_show_cursor( bool visible );

void winsys_init( int *argc, char **argv, const char *window_title,
		  const char *icon_title );
void winsys_shutdown();

void winsys_process_events(); /* Never returns */

void winsys_atexit( winsys_atexit_func_t func );

void winsys_exit( int code );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WINSYS_H */

/* Emacs Customizations
;;; Local Variables: ***
;;; c-basic-offset:0 ***
;;; End: ***
*/

/* EOF */
