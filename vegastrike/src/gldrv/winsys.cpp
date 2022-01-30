/*
 * Tux Racer
 * Copyright (C) 1999-2001 Jasmin F. Patry
 *
 * Vegastrike
 * Copyright (C) 2002-2022 The Vegastrike developers
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
#include "config.h"
#include "gl_globals.h"
#include "winsys.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "unicode.h"
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>

// #include <sys/signal.h>
#include "log.h"
#include "vs_log_modules.h"
#include "in_joystick.h"
#include "options.h"

extern vs_options game_options;

/*---------------------------------------------------------------------------*/
/* Windowing System Abstraction Layer */
/* Abstracts creation of windows, handling of events, etc. */
/*---------------------------------------------------------------------------*/

#define WINSYS_ERR(...)             WINSYS_LOG(logvs::ERROR, __VA_ARGS__)

// for printf functions, display only displayable characters
#define WS_CHAR(c)  ((char)(isprint(c) ? c : '?'))

#define WINSYS_JOY_REPEAT_MS 0 // ignore joystick event if oldticks+WINSYS_JOY_REPEAT_MS>=ticks

/*---------------------------------------------------------------------------*/
/*!
  Common init for all renderers
  \author  vsa
  \date    Created:  2021-09
*/
static int winsys_common_init() {
    return 0;
}

extern int shiftdown(int);
extern int shiftup(int);

/*---------------------------------------------------------------------------*/
/*!
  shift up/down wrapper
  \author  vsa
  \date    Created:  2021-10
*/
static int winsys_kbshift(unsigned int ch, bool shifton, bool sendlower) {
    if (ch > 127)
        return ch;
    if (shifton) {
        return sendlower ? shiftdown(ch) : toupper(ch);
    } else {
        return tolower(ch);
    }
}

/*---------------------------------------------------------------------------*/

#if defined( SDL_WINDOWING ) && defined (HAVE_SDL)

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* SDL WINDOWING */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#ifdef SDL_MAJOR_VERSION
# define WINSYS_SDL_MAJOR SDL_MAJOR_VERSION
#else
# define WINSYS_SDL_MAJOR 0
#endif

static winsys_display_func_t display_func = NULL;
static winsys_idle_func_t idle_func = NULL;
static winsys_reshape_func_t reshape_func = NULL;
static winsys_keyboard_func_t keyboard_func = NULL;
static winsys_mouse_func_t mouse_func = NULL;
static winsys_motion_func_t motion_func = NULL;
static winsys_motion_func_t passive_motion_func = NULL;
static winsys_atexit_func_t atexit_func = NULL;
static winsys_joystick_func_t joystick_func = NULL;

static bool redisplay = false;

/*---------------------------------------------------------------------------*/
/*!
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay()
{
    redisplay = true;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
    display_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
    idle_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    reshape_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    mouse_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    motion_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    passive_motion_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the joystick callback
  \author  vsa
  \date    Created:  2021-11
*/
void winsys_set_joystick_func( winsys_joystick_func_t func )
{
    joystick_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
  Display some sdl GL attributes
  \author  vsa
  \date    Created:  2021-09
*/
// Helper macros to display GL attrs
#define WS_STR(x) #x
#define WS_SDL_PRINT_GL_ATTR(attr)                             	 		\
    {                                                           		\
        int value;                                              		\
        if (SDL_GL_GetAttribute(attr, &value) == 0) {           		\
            WINSYS_LOG(logvs::NOTICE, "  %s = %d", WS_STR(attr), value);\
        }                                                       		\
    }
void winsys_sdl_print_gl_attributes() {
    WS_SDL_PRINT_GL_ATTR(SDL_GL_RED_SIZE);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_GREEN_SIZE);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_BLUE_SIZE);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_ALPHA_SIZE);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_BUFFER_SIZE);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_DEPTH_SIZE);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_STENCIL_SIZE);
    //SDL_GL_ACCUM_RED_SIZE, SDL_GL_ACCUM_GREEN_SIZE, SDL_GL_ACCUM_BLUE_SIZE,SDL_GL_ACCUM_ALPHA_SIZE,
    WS_SDL_PRINT_GL_ATTR(SDL_GL_STEREO);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_MULTISAMPLEBUFFERS);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_MULTISAMPLESAMPLES);
    WS_SDL_PRINT_GL_ATTR(SDL_GL_ACCELERATED_VISUAL);
#if SDL_VERSION_ATLEAST(2,0,0)
    WS_SDL_PRINT_GL_ATTR(SDL_GL_RETAINED_BACKING);
#endif
    WS_SDL_PRINT_GL_ATTR(SDL_GL_DOUBLEBUFFER);
}

/*---------------------------------------------------------------------------*/

#if SDL_VERSION_ATLEAST(2,0,0)
/*---------------------------------------------------------------------------*/
/*     S D L   2                                                             */
/*---------------------------------------------------------------------------*/

# include <queue>

static SDL_Window *     sdl_window = NULL;
static SDL_GLContext    sdl_glcontext;
static unsigned int     kb_unicode_mode = WS_UNICODE_DISABLED;
static int              kb_repeat_delay = WS_KB_REPEAT_DISABLED;
static int              kb_repeat_interval = 0;
static unsigned int     kb_repeat_next = 0;
static unsigned int     kb_last_unicode = 0;
static bool             kb_reset_unicode_table = true;

/*---------------------------------------------------------------------------*/
/*!
  Copies the OpenGL back buffer to the front buffer
  \author  vsa
  \date    Created:  2021-09
  \date    Modified: 2021-09
*/
void winsys_swap_buffers()
{
    //SDL_GL_SwapBuffers();
    SDL_GL_SwapWindow(sdl_window);
    ++glswap_count;
}

/*---------------------------------------------------------------------------*/
/*!
  Moves the mouse pointer to (x,y)
  \author vsa
  \date    Created:  2021-09
  \date    Modified: 2021-09
*/
void winsys_warp_pointer( int x, int y )
{
    //SDL_WarpMouseGlobal(x, y );
    SDL_WarpMouseInWindow(sdl_window, x, y );
    WINSYS_DBG(logvs::DBG, "SDL2: WARP to %dx%d", x, y);
}

static int sdl_pixel_format_from_bpp(int bpp) {
	static std::string rgbfmt = vs_config->getVariable("graphics","rgb_pixel_format",
#if defined(__APPLE__)
			"8888");
#else
			"888");
#endif
	//int zs = XMLSupport::parse_int( vs_config->getVariable("graphics","z_pixel_format","24") );
	switch (bpp) {
        case 8:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 3);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 3);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 2);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
            return SDL_PIXELFORMAT_INDEX8;
        case 15:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
            return SDL_PIXELFORMAT_RGB555;
        case 16:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
            return SDL_PIXELFORMAT_RGB565;
        case 24:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            return SDL_PIXELFORMAT_RGB24;
        case 32:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            if (rgbfmt == "8888") {
            	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            	return SDL_PIXELFORMAT_ARGB32; //ARGB8888;
            }
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
            return SDL_PIXELFORMAT_RGB888;
        default:
            return SDL_PIXELFORMAT_RGB888;
    }
}

/*---------------------------------------------------------------------------*/
/*!
 Sets up the SDL2 OpenGL rendering context
 \author  vsa
 \date    Created:  2021-09
 \date    Modified: 2021-09
 */
static void setup_sdl_video_mode()
{
    int bpp = gl_options.color_depth;
    int width, height;
    SDL_DisplayMode display_mode;

    /*int rs,gs,bs,zs; rs=gs=bs=(bpp==16)?5:8;
    string rgbfmt = vs_config->getVariable("graphics","rgb_pixel_format",((bpp==16)?"555":"888"));
    zs = XMLSupport::parse_int( vs_config->getVariable("graphics","z_pixel_format","24") );
    if ((rgbfmt.length()==3)&&isdigit(rgbfmt[0])&&isdigit(rgbfmt[1])&&isdigit(rgbfmt[2])) {
        rs = rgbfmt[0]-'0';
        gs = rgbfmt[1]-'0';
        bs = rgbfmt[2]-'0';
    };*/

    /* setup the requested display mode */
    width = g_game.x_resolution;
    height =g_game.y_resolution;

    SDL_zero(display_mode);
    display_mode.w = width;         // width, in screen coordinates
    display_mode.h = height;        // height, in screen coordinates
    display_mode.refresh_rate = 0;  // refresh rate (or zero for unspecified)
    display_mode.format = sdl_pixel_format_from_bpp(bpp);

    for (int setup_try = 0; setup_try < 2; ++setup_try) {

        // (re)TRY IT
        if ( SDL_SetWindowDisplayMode(sdl_window, &display_mode) == 0) {
            //OK
            break ;
        } else {
            SDL_DisplayMode closest;

            // FAILED, fallback once and exit on failure.
            WINSYS_ERR("(SDL%d:%s) Couldn't initialize video (%dx%d @%d %s): %s",
                       WINSYS_SDL_MAJOR, SDL_GetCurrentVideoDriver(),
                       display_mode.w, display_mode.h, display_mode.refresh_rate,
                       SDL_GetPixelFormatName(display_mode.format),
                       SDL_GetError());

            if (setup_try == 1) {
                WINSYS_ERR("FAILED to initialize video");
                exit(1);
            }

            if (SDL_GetClosestDisplayMode(SDL_GetWindowDisplayIndex(sdl_window), &display_mode, &closest) == NULL) {
                WINSYS_ERR("(SDL%d:%s) no more available display mode",
                           WINSYS_SDL_MAJOR, SDL_GetCurrentVideoDriver());
                WINSYS_ERR("FAILED to initialize video");
                exit(1);
            }

            SDL_memcpy(&display_mode, &closest, sizeof(display_mode));
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
        }
    }

    if (SDL_GetWindowDisplayMode(sdl_window, &display_mode) < 0) {
        WINSYS_LOG(logvs::NOTICE, "warning: cannot retrieve SDL display mode: %s", SDL_GetError());
    } else {

        WINSYS_LOG(logvs::NOTICE, "(SDL%d:%s) Setting Screen to %dx%d @%d %s (glCtx=%p)",
                   WINSYS_SDL_MAJOR, SDL_GetCurrentVideoDriver(),
                   display_mode.w, display_mode.h, display_mode.refresh_rate,
                   SDL_GetPixelFormatName(display_mode.format),
                   SDL_GL_GetCurrentContext());
    }
}

static const char * winsys_sdl2_swapinterval_desc(int v) {
    if (v == 0) return "immediate";
    if (v == 1) return "vsync";
    if (v == -1) return "adaptative-vsync";
    return "unknown";
}

/*---------------------------------------------------------------------------*/
/*!
 Initializes the OpenGL rendering context, and creates a window (or
 sets up fullscreen mode if selected)
 \author  vsa
 \date    Created:  2021-09
 \date    Modified: 2021-09
 */
void winsys_init( int *argc, char **argv, const char *window_title,
                  const char *icon_title )
{
    SDL_DisplayMode display_mode;

    winsys_common_init();

    WINSYS_LOG(logvs::NOTICE, "(SDL%d) Initializing...", WINSYS_SDL_MAJOR);

    // SDL_INIT_AUDIO|
    Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
    static int maximized = XMLSupport::parse_bool (vs_config->getVariable ("graphics","maximized","false"));
    static bool get_stencil = XMLSupport::parse_bool (vs_config->getVariable ("graphics","glut_stencil","true"));
    static bool vsync = XMLSupport::parse_bool (vs_config->getVariable ("graphics/sdl2","vsync","true"));
    g_game.x_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","x_resolution","1024"));
    g_game.y_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","y_resolution","768"));
    gl_options.fullscreen = XMLSupport::parse_bool (vs_config->getVariable ("graphics","fullscreen","false"));
    gl_options.color_depth = XMLSupport::parse_int (vs_config->getVariable ("graphics","colordepth","32"));

    /*
     * Initialize SDL
     */
    if ( SDL_Init( sdl_flags ) < 0 ) {
        WINSYS_ERR("Couldn't initialize SDL: %s", SDL_GetError() );
        exit(1);
    }
    SDL_StopTextInput();

    // signal( SIGSEGV, SIG_DFL );
    SDL_Surface *icon = NULL;
    if (icon_title) {
        icon = SDL_LoadBMP(icon_title);
        if (icon) {
            SDL_SetColorKey(icon,1/*SDL_SRCCOLORKEY*/,((Uint32*)(icon->pixels))[0]);//FIXME
        } else {
            WINSYS_LOG(logvs::NOTICE, "warning: cannot load icon '%s': %s", icon_title, SDL_GetError());
        }
    }

    /* basic GL settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO, 0);//0
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);*/
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (get_stencil) {
        /* Not sure if this is sufficient to activate stencil buffer  */
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    } else {
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    }
    sdl_pixel_format_from_bpp(gl_options.color_depth);

    /*
     * Init video
     */
    int width = g_game.x_resolution;
    int height = g_game.y_resolution;

    sdl_flags = SDL_WINDOW_OPENGL;

    if (gl_options.fullscreen) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN
#if !defined(__APPLE__) && !defined(_WIN32)
                  | SDL_WINDOW_BORDERLESS
#endif
        ;
    } else {
        sdl_flags |= SDL_WINDOW_RESIZABLE;
    }

    //SDL_Renderer *renderer;
    //if (SDL_CreateWindowAndRenderer(width, height, sdl_flags, &sdl_window, &renderer) < 0) {
    if ((sdl_window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       width, height, sdl_flags)) == NULL) {
        WINSYS_ERR("(SDL%d) SDL_CreateWindow() error: %s",
                   WINSYS_SDL_MAJOR, SDL_GetError());
        exit(1);
    }
    WINSYS_LOG(logvs::NOTICE, "(SDL%d) window created, GLcontext=%p",
               WINSYS_SDL_MAJOR, SDL_GL_GetCurrentContext());

    /* Enumerate supported display mode */
    int widx    = SDL_GetWindowDisplayIndex(sdl_window);
    int count   = SDL_GetNumDisplayModes(widx);
    WINSYS_LOG(logvs::NOTICE, "(SDL%d) Availaible modes: %d", WINSYS_SDL_MAJOR, count);
    for (int i=0; i < count; ++i) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(widx, i, &mode) != 0)
            continue ;
        WINSYS_LOG(logvs::NOTICE, "          %dx%d@%d %s",
                   mode.w, mode.h, mode.refresh_rate,
                   SDL_GetPixelFormatName(mode.format));
    }

    if (icon) {
        SDL_SetWindowIcon(sdl_window, icon);
    }

    /* show window */
    SDL_ShowWindow(sdl_window);

    /* setup resultion and pixel format */
    setup_sdl_video_mode();

    /* Create OpenGL context */
    sdl_glcontext = SDL_GL_CreateContext(sdl_window);

    if (!sdl_glcontext) {
        WINSYS_ERR("(SDL%d) SDL_GL_CreateContext() error: %s",
                   WINSYS_SDL_MAJOR, SDL_GetError());
        exit(1);
    } else {
        WINSYS_LOG(logvs::NOTICE, "(SDL%d) created SDL GL context %p",
                   WINSYS_SDL_MAJOR, SDL_GL_GetCurrentContext());
    }

    //VSYNC
    WINSYS_DBG(logvs::DBG, "(SDL%d) Initial SwapInterval is %s",
               WINSYS_SDL_MAJOR, winsys_sdl2_swapinterval_desc(SDL_GL_GetSwapInterval()));
    if (!vsync) {
        SDL_GL_SetSwapInterval(0);
    } else {
        int ret;
        if (SDL_GL_SetSwapInterval(-1) == 0) {
            // auto vsync ok
            WINSYS_DBG(logvs::DBG, "(SDL%d) SwapInterval set to %s (%d)", WINSYS_SDL_MAJOR,
                       winsys_sdl2_swapinterval_desc(-1), -1);
        } else {
            if (SDL_GL_SetSwapInterval(1) != 0) {
                WINSYS_LOG(logvs::NOTICE, "(SDL%d) cannot set SwapInterval to %s (%d)",
                           WINSYS_SDL_MAJOR, winsys_sdl2_swapinterval_desc(1), 1);
            };
        }
    }
    WINSYS_LOG(logvs::NOTICE, "(SDL%d) SwapInterval set to %s",
               WINSYS_SDL_MAJOR, winsys_sdl2_swapinterval_desc(SDL_GL_GetSwapInterval()));

    if (gl_options.fullscreen) {
       // nothing
    } else if (maximized) {
        WINSYS_LOG(logvs::NOTICE, "(SDL%d) Enter Fullscreen", WINSYS_SDL_MAJOR);
        SDL_MaximizeWindow(sdl_window);
        SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    /* finalize init */
    glutInit(argc,argv);

    if (SDL_GetWindowDisplayMode(sdl_window, &display_mode) < 0) {
        WINSYS_LOG(logvs::NOTICE, "(SDL%d) warning: cannot retrieve SDL display mode: %s",
                   WINSYS_SDL_MAJOR, SDL_GetError());
    } else {
        WINSYS_LOG(logvs::NOTICE, "(SDL%d) Screen INITIALIZED (%dx%d @%d %s /GLctx=%p",
                   WINSYS_SDL_MAJOR,
                   display_mode.w, display_mode.h, display_mode.refresh_rate,
                   SDL_GetPixelFormatName(display_mode.format),
                   SDL_GL_GetCurrentContext());
    }
    winsys_sdl_print_gl_attributes();

    SDL_RaiseWindow(sdl_window);
}

/*---------------------------------------------------------------------------*/

static bool sdl_check_kb_repeat(unsigned int timestamp, int repeat, unsigned int key) {
    (void) key;
    if (kb_repeat_delay != WS_KB_REPEAT_DISABLED) {
        if (repeat) {
            if (!SDL_TICKS_PASSED(timestamp, kb_repeat_next)) {
                // the repeated key has not passed the required delay
                WINSYS_DBG(logvs::DBG+1, "repeat ignored (%x)", key);
                return false;
            }
            // the next delay is set to interval
            kb_repeat_next = timestamp + kb_repeat_interval;
        } else {
            // reset the repeat delay
            kb_repeat_next = timestamp + kb_repeat_delay;
        }
    } else if (repeat) {
        // ignore the repeated key
        WINSYS_DBG(logvs::DBG+1, "repeat disabled (%x)", key);
        return false;
    }
    return true;
}

/*---------------------------------------------------------------------------*/
/*!
  Enables/disables key repeat messages from being generated
  \return
  \author  vsa
  \date    Created:  2021-09
  \date    Modified: 2021-09
*/
void winsys_set_kb_mode(unsigned unicode, int delay_ms, int interval_ms,
                        unsigned int * unicode_bak, int * delay_ms_bak, int * interval_ms_bak)
{
    if (unicode_bak)
        *unicode_bak = kb_unicode_mode;
    if (delay_ms_bak)
        *delay_ms_bak = kb_repeat_delay;
    if (interval_ms_bak)
        *interval_ms_bak = kb_repeat_interval;

    if (unicode && unicode != kb_unicode_mode) {
        kb_reset_unicode_table = true;
        SDL_StartTextInput();
    } else if (!unicode && unicode != kb_unicode_mode) {
        SDL_StopTextInput();
        kb_reset_unicode_table = true;
    } else if (delay_ms != kb_repeat_delay) {
        kb_reset_unicode_table = true;
    }
    kb_unicode_mode = unicode;

    if (delay_ms != WS_KB_REPEAT_DISABLED) {
        if (kb_repeat_delay == WS_KB_REPEAT_DISABLED) {
            //SDL_StartTextInput();
        }
        if (delay_ms == WS_KB_REPEAT_ENABLED_DEFAULT) {
            delay_ms = WS_KB_REPEAT_DELAY;
            interval_ms = WS_KB_REPEAT_INTERVAL;
        }
        kb_repeat_delay = delay_ms;
        kb_repeat_interval = interval_ms;

        WINSYS_LOG(logvs::NOTICE, "(SDL%d) Keyboard Repeat %s (delay:%dms interval:%dms unicode:%d)",
                   WINSYS_SDL_MAJOR, "ON", kb_repeat_delay, kb_repeat_interval, unicode);

    } else {
        if (kb_repeat_delay != WS_KB_REPEAT_DISABLED) {
            //SDL_StopTextInput();
        }
        kb_repeat_delay = WS_KB_REPEAT_DISABLED;

        WINSYS_LOG(logvs::NOTICE, "(SDL%d) Keyboard Repeat %s (unicode:%d)", WINSYS_SDL_MAJOR, "OFF", unicode);
    }
}

/*---------------------------------------------------------------------------*/
/*!
  Processes and dispatches events.  This function never returns. (new sdl2 from former sdl1)
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
  \date    Modified: 2005-8-16 - Rogue
  \date    Modified: 2005-12-24 - ace123
  \date    Modified: 2021-09 - vsa
 */

void winsys_process_events()
{
    SDL_Event event;
    SDL_Keymod mod;
    unsigned int key;
    int x, y;
    bool released;
    bool fullwindow_hidden = false;
    
    static bool sdl_lockaudio = XMLSupport::parse_bool(vs_config->getVariable("graphics/sdl2","lockaudio_workaround","true"));
    static bool kb_sendlower = XMLSupport::parse_bool(vs_config->getVariable("keyboard","lower_keys","true"));
    static bool kb_alt_tab = XMLSupport::parse_bool(vs_config->getVariable("keyboard","alt_tab_override","true"));
    
    // **************
    // **** SDL2 ****
    // **************
    static unsigned int             keysym_to_unicode[WSK_LAST];
    static std::queue<unsigned int> unicode_keysym_queue;
    unsigned int					last_joystick_ms[MAX_JOYSTICKS];
    memset(last_joystick_ms, 0, sizeof(last_joystick_ms));

    if (sdl_lockaudio) {
    	SDL_InitSubSystem(SDL_INIT_AUDIO);
    }

    WINSYS_LOG(logvs::NOTICE, "(SDL%d) Entering main loop...", WINSYS_SDL_MAJOR);
    while (true) {
    unsigned int event_count = 0;
    if (sdl_lockaudio) {
        SDL_LockAudio();
        SDL_UnlockAudio();
    }

    while ( SDL_PollEvent( &event ) ) {
        ++event_count;
        released = false;

        if (kb_reset_unicode_table) {
            memset(keysym_to_unicode, 0, sizeof(keysym_to_unicode));
            while (!unicode_keysym_queue.empty()) unicode_keysym_queue.pop();
            kb_reset_unicode_table = false;
        }

        switch ( event.type ) {

        case SDL_KEYUP:
            released = true;
            // does same thing as KEYDOWN, but with different state.
        case SDL_KEYDOWN: {
            mod = (SDL_Keymod) event.key.keysym.mod;
            // ALT TAB handling in fullscreen mode
            if (kb_alt_tab
            &&  !released && event.key.keysym.sym == SDLK_TAB && gl_options.fullscreen
            &&  (mod & (KMOD_LALT|KMOD_RALT|KMOD_ALT|KMOD_GUI|KMOD_LGUI|KMOD_RGUI)) != 0) {
                //SDL_HideWindow(sdl_window);
                SDL_MinimizeWindow(sdl_window);
                fullwindow_hidden = true;
                break ;
            }
            if ( keyboard_func == NULL) {
                break ;
            }
            SDL_GetMouseState( &x, &y );
            bool shifton = mod&(KMOD_SHIFT|KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS);

            // TextInput does not manage keyUP: let TEXTINPUT handle keyDOWN and keyUP(released==true) here.
            if (!(event.key.keysym.sym & SDLK_SCANCODE_MASK) && SDL_IsTextInputActive()) {
                if (released) {
                    // UNICODE KEYUP
                    if ((key = keysym_to_unicode[event.key.keysym.scancode]) != 0) {
                        keysym_to_unicode[event.key.keysym.scancode] = 0;
                        kb_last_unicode = 0;
                        WINSYS_LOG(logvs::VERBOSE,
                                   "SDL%d: Processing UNICODE '%lc' (%x,scan:%x,code:%x,release:%d,mod:%x)",
                                   WINSYS_SDL_MAJOR, (wchar_t)WSK_CODE_TO_UTF32(key), WSK_CODE_TO_UTF32(key),
                                   event.key.keysym.scancode, key, released, mod);
                        // Send the event
                        (*keyboard_func)( key, mod, released, x, y );
                        continue ;
                    }
                } else if (iswprint(event.key.keysym.sym)
                && ((kb_unicode_mode & WS_UNICODE_FULL) != 0 || !HasKeyBinding(event.key.keysym.sym, mod))) {
                    // UNICODE KEYDOWN
                    WINSYS_DBG(logvs::DBG+1,
                               "SDL%d: key '%c' (%x,scan:%x,release:%d,mod:%x) to be processed by TEXTINPUT(q:%zu)",
                               WINSYS_SDL_MAJOR, WS_CHAR(event.key.keysym.sym), event.key.keysym.sym,
                               event.key.keysym.scancode, released, mod, unicode_keysym_queue.size());
                    // event ignored, process next one which should be TEXTINPUT
                    //if (unicode_keysym_queue.empty() || unicode_keysym_queue.front() != event.key.keysym.sym)
                    unicode_keysym_queue.push(event.key.keysym.scancode);
                    --event_count;
                    continue ;
                }
            }
            // Ignore Repeats
            kb_last_unicode = 0;
            if (!sdl_check_kb_repeat(event.key.timestamp, event.key.repeat, event.key.keysym.sym)) {
                // event ignored, process next one
                --event_count;
                continue ;
            } // SCANCODES: special keys
            else if ((event.key.keysym.sym & SDLK_SCANCODE_MASK)) {
                key = event.key.keysym.scancode + WSK_KEY_OFFSET;
            } // sanity check for keysym.sym (no oveeride with scancodes or unicodes
            else if (event.key.keysym.sym >= WSK_KEY_OFFSET) { // Should not happen
                WINSYS_LOG(logvs::NOTICE, "SDL%d: warning: keysym %x [%c] scan:%x (mod %x) "
                           "is neither a keysym nor a scancode", WINSYS_SDL_MAJOR,
                           event.key.keysym.sym, WS_CHAR(event.key.keysym.sym),
                           event.key.keysym.scancode, mod);
                // event ignored, process next one
                --event_count;
                continue ;
            } // Regular Key
            else {
                key = winsys_kbshift(event.key.keysym.sym, shifton, kb_sendlower);
            }

            if (WINSYS_LOG_START(logvs::VERBOSE, "SDL%d: processing key '", WINSYS_SDL_MAJOR) > 0) {
                if (key < 128 && isprint(event.key.keysym.sym)) logvs::vs_printf("%c", key);
                else logvs::vs_printf("%s", SDL_GetKeyName(event.key.keysym.sym));
                WINSYS_LOG_END(logvs::VERBOSE, "' (%x) (release:%d mod:%x)",
                               event.key.keysym.sym, released, mod);
            }

            // Send the event
            (*keyboard_func)(key, mod, released, x, y);
            break;
        }

        case SDL_TEXTEDITING:
            WINSYS_DBG(logvs::DBG, "SDL%d EDIT %s", WINSYS_SDL_MAJOR, event.edit.text);
        case SDL_TEXTINPUT:
            if (unicode_keysym_queue.empty()) {
                break ;
            }
            if (keyboard_func == NULL) {
                break ;
            }
            // Get the unicode value
            for (Utf8Iterator it = Utf8Iterator(event.text.text); it != it.end(); ++it) {
                wchar_t u32 = *it;

                key = WSK_UTF32_TO_CODE(u32);

                if (!unicode_keysym_queue.empty()) {
                    keysym_to_unicode[unicode_keysym_queue.front()] = key;
                    unicode_keysym_queue.pop();
                } else if (event.type != SDL_TEXTEDITING){
                    WINSYS_DBG(logvs::DBG, "WARNING: received TEXTINPUT with keysym queue empty");
                }

                if (u32 >= 128 && !WSK_CODE_IS_UTF32(key)) { //does not fit in int type, ignore
                    WINSYS_DBG(logvs::DBG, "SDL%d: warning: unicode %x does not fit", WINSYS_SDL_MAJOR, key);
                    // event ignored, process next one
                    --event_count;
                    continue ;
                }

                if (!sdl_check_kb_repeat(event.text.timestamp, key == kb_last_unicode, key)) {
                    // event ignored, process next one
                    --event_count;
                    continue ;
                }
                kb_last_unicode = key;

                mod = SDL_GetModState();
                SDL_GetMouseState( &x, &y );

                if (WINSYS_LOG_START(logvs::VERBOSE, "SDL%d: Processing UNICODE '", WINSYS_SDL_MAJOR) > 0) {
                    logvs::vs_printf("%lc", u32);
                    WINSYS_LOG_END(logvs::VERBOSE, "' (%x, code:%x, release:%d, mod:%x)", u32, key, 0, mod);
                }
                // send only the DOWN event, KEYUP will handle the UP event
                (*keyboard_func)(key, mod, false, x, y);
            }
            break ;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if ( mouse_func ) {
                (*mouse_func)(event.button.button,
                              event.button.state,
                              event.button.x,
                              event.button.y);
            }
            break;

        case SDL_MOUSEWHEEL:
            SDL_GetMouseState( &x, &y );
            WINSYS_DBG(logvs::DBG, "SDL%d WHEEL x=%d y=%d dir=%d",
                         WINSYS_SDL_MAJOR, event.wheel.x, event.wheel.y, event.wheel.direction);
            if ( mouse_func ) {
                static int wheel_direction = INT_MIN+1;
                if (wheel_direction == INT_MIN+1) {
                    std::string wheel_dir_str = vs_config->getVariable("mouse","wheel_direction","system");
                    if (wheel_dir_str == "natural") {
                        wheel_direction = SDL_MOUSEWHEEL_FLIPPED;
                    } else if (wheel_dir_str == "normal") {
                        wheel_direction = SDL_MOUSEWHEEL_NORMAL;
                    } else {
                        wheel_direction = INT_MIN;
                    }
                }
                if (wheel_direction != INT_MIN && wheel_direction != event.wheel.direction) {
                    event.wheel.y = -event.wheel.y;
                    event.wheel.x = -event.wheel.x;
                }
                int button = event.wheel.y > 0 ? WS_WHEEL_UP : WS_WHEEL_DOWN;
                (*mouse_func)(button, SDL_PRESSED, x, y );
                (*mouse_func)(button, SDL_RELEASED, x, y );
            }
            break ;

        case SDL_MOUSEMOTION:
            if ( event.motion.state ) {
                /* buttons are down */
                if ( motion_func ) {
                (*motion_func)( event.motion.x,
                        event.motion.y );
                }
            } else {
                /* no buttons are down */
                if ( passive_motion_func ) {
                (*passive_motion_func)( event.motion.x,
                            event.motion.y );
                }
            }
            break;

        case SDL_QUIT:
            VSExit(0); //winsys_exit(0);
            break ;

        case SDL_WINDOWEVENT:
            if (!gl_options.fullscreen && event.window.event == SDL_WINDOWEVENT_LEAVE) {
                WINSYS_DBG(logvs::DBG, "WINDOW_LEAVE");
                SDL_SetWindowFullscreen(sdl_window, SDL_FALSE);
                winsys_show_cursor(1);
                break ;
            }
            if (event.window.event == SDL_WINDOWEVENT_ENTER) {
                WINSYS_DBG(logvs::DBG, "WINDOW_ENTER");
                if (fullwindow_hidden) {
                    fullwindow_hidden = false;
                    SDL_MaximizeWindow(sdl_window);
                    SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN);//not needed if only minimized
                }
                winsys_show_cursor(0);
                break ;
            }
            if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                VSExit(0); //winsys_exit(0);
                break ;
            }
            if (event.window.event != SDL_WINDOWEVENT_RESIZED) {
                break ;
            }
            x = event.window.data1;
            y = event.window.data2;
            WINSYS_LOG(logvs::NOTICE, "SDL%d WINDOW RESIZE %dx%d", WINSYS_SDL_MAJOR, x, y);
            g_game.x_resolution = x;
            g_game.y_resolution = y;
            setup_sdl_video_mode();
            if ( reshape_func ) {
                    (*reshape_func)( x, y );
            }
            break;

        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
        	if (SDL_NumJoysticks() != GetNumJoysticks()) {
        		WINSYS_LOG(logvs::INFO, "SDL%d: Joystick %s %d", WINSYS_SDL_MAJOR,
        			event.jdevice.type == SDL_JOYDEVICEREMOVED ? "REMOVED, id:" : "ADDED, index:",
        			event.jdevice.which);
        		UpdateJoystick(event.jdevice.type == SDL_JOYDEVICEREMOVED
        				       ? GetJoystickByID(event.jdevice.which) : GetNumJoysticks());
        	}
        	break ;

        case SDL_JOYAXISMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBALLMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        	if (joystick_func) {
        		int index = GetJoystickByID(event.jaxis.which);
        		if (index >= GetNumJoysticks() || (last_joystick_ms[index] + WINSYS_JOY_REPEAT_MS >= event.jaxis.timestamp)
        		|| joystick[index] == NULL || !joystick[index]->isAvailable())
        			break ;
        		last_joystick_ms[index] = event.jaxis.timestamp;
        		float x, y, z;
        		int buttons;
        		joystick[index]->GetJoyStick(x, y, z, buttons);
        		WINSYS_DBG(logvs::DBG, "SDL%d: joystick #%d x:%g y:%g z:%g buttons:%x",
        				   WINSYS_SDL_MAJOR, index, x, y, z, buttons);
        		joystick_func(index, joystick[index]->joy_axis[0], joystick[index]->joy_axis[1],
        					  joystick[index]->joy_axis[2], joystick[index]->joy_buttons, 0);
        	}
        	break ;

        } // end switch

        if (sdl_lockaudio) {
            SDL_LockAudio();
            SDL_UnlockAudio();
        }
    } // end while SDL_PollEvent()

    if ( redisplay && display_func ) {
        redisplay = false;
        (*display_func)();
    } else if ( idle_func ) {
        (*idle_func)();
    }

    /* Delay for 1 ms.  This allows the other threads to do some
       work (otherwise the audio thread gets starved). */
    if (sdl_lockaudio) {
        SDL_Delay(1);
    }

    } // end while(true)

    /* Never exits */
#define CODE_NOT_REACHED 0
    assert(CODE_NOT_REACHED);
    //    code_not_reached();
}

/*---------------------------------------------------------------------------*/
/*!
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
  static bool shutdown=false;
  if (!shutdown) {
    shutdown=true;
    SDL_Quit();
  }
}

/* --------------------------------------------------------------------------*/

#else
/* --------------------------------------------------------------------------*/
/*     S D L   1                                                             */
/* --------------------------------------------------------------------------*/
#include "gnuhash.h"

static unsigned int kb_unicode_mode = WS_UNICODE_DISABLED;
static SDL_Surface *screen = NULL;
static SDL_Cursor * oldcursor = NULL;

/*---------------------------------------------------------------------------*/
/*!
 Copies the OpenGL back buffer to the front buffer
 \author  jfpatry
 \date    Created:  2000-10-19
 \date    Modified: 2000-10-19
 */
void winsys_swap_buffers()
{
    SDL_GL_SwapBuffers();
    ++glswap_count;
}


/*---------------------------------------------------------------------------*/
/*!
 Moves the mouse pointer to (x,y)
 \author  jfpatry
 \date    Created:  2000-10-19
 \date    Modified: 2000-10-19
 */
void winsys_warp_pointer( int x, int y )
{
    SDL_WarpMouse( x, y );
}

/*---------------------------------------------------------------------------*/
/*!
  Sets up the SDL OpenGL rendering context
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
static void setup_sdl_video_mode()
{
    Uint32 video_flags = SDL_OPENGL;
    int bpp = 0;
    int width, height;

    if ( gl_options.fullscreen ) {
	    video_flags |= SDL_FULLSCREEN
#if !defined(__APPLE__) && !defined(_WIN32)
                       | SDL_NOFRAME
#endif
        ;
    } else {
#ifndef _WIN32
	video_flags |= SDL_RESIZABLE;
#endif
    }

    bpp = gl_options.color_depth;

    int rs,gs,bs,zs; rs=gs=bs=(bpp==16)?5:8;
    string rgbfmt = vs_config->getVariable("graphics","rgb_pixel_format",((bpp==16)?"555":"888"));
    zs = XMLSupport::parse_int( vs_config->getVariable("graphics","z_pixel_format","24") );
    if ((rgbfmt.length()==3)&&isdigit(rgbfmt[0])&&isdigit(rgbfmt[1])&&isdigit(rgbfmt[2])) {
	rs = rgbfmt[0]-'0';
	gs = rgbfmt[1]-'0';
	bs = rgbfmt[2]-'0';
    };
    int otherbpp;
    int otherattributes;
    if (bpp==16) {
      otherattributes=8;
      otherbpp=32;
      SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rs );
      SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, gs );
      SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bs );
      SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, zs );
      SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    }else {
      otherattributes=5;
      otherbpp=16;
      SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rs );
      SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, gs );
      SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bs );
      SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, zs );
      SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    };
#if SDL_VERSION_ATLEAST(1,2,10)
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1);
#endif
    width = g_game.x_resolution;
    height =g_game.y_resolution  ;

    if ( ( screen = SDL_SetVideoMode( width, height, bpp, video_flags ) ) ==
	 NULL )
    {
        WINSYS_ERR("Couldn't initialize video: %s", SDL_GetError() );
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
        for (int counter=0;screen==NULL&&counter<2;++counter) {
          for (int bpd=4;bpd>1;--bpd) {
            SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, bpd*8 );
            if ( ( screen = SDL_SetVideoMode( width, height, bpp, video_flags|SDL_ANYFORMAT ) ) ==
                 NULL )
            {
                WINSYS_ERR("Couldn't initialize video bpp %d depth %d: %s",
                           bpp,bpd*8,SDL_GetError() );
            }else {
              break;
            }
          }
          if (screen==NULL) {
            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, otherattributes );
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, otherattributes );
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, otherattributes );
            gl_options.color_depth=bpp=otherbpp;
          }
        }
        if (screen==NULL) {
          WINSYS_ERR("FAILED to initialize video");
          exit(1);
        }
    }
    WINSYS_LOG(logvs::NOTICE,
            "(SDL%d) Setting Screen to w %d h %d and pitch of %d and %d bpp %d bytes per pix mode",
            WINSYS_SDL_MAJOR,
            screen->w,screen->h,screen->pitch, screen->format->BitsPerPixel, screen->format->BytesPerPixel);

    if (WINSYS_LOG(logvs::NOTICE, "(SDL%d) pixel-format: alpha:%d",
                   WINSYS_SDL_MAJOR, screen->format->alpha) > 0) {
        logvs::vs_printf(
                   "               mask R:%x G:%x B:%x A:%x \n"
                   "               loss R:%d G:%d B:%d A:%d \n"
                   "               shift R:%d G:%d B:%d A:%d \n",
               screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask,
               screen->format->Rloss, screen->format->Gloss, screen->format->Bloss, screen->format->Aloss,
               screen->format->Rshift, screen->format->Gshift, screen->format->Bshift, screen->format->Ashift);
    }
}


/*---------------------------------------------------------------------------*/
/*!
  Initializes the OpenGL rendering context, and creates a window (or
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/

void winsys_init( int *argc, char **argv, const char *window_title,
		  const char *icon_title )
{
    winsys_common_init();

    WINSYS_LOG(logvs::NOTICE, "(SDL%d) Initializing...", WINSYS_SDL_MAJOR);

	// SDL_INIT_AUDIO|
	Uint32 sdl_flags = SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;
    static bool get_stencil=XMLSupport::parse_bool (vs_config->getVariable ("graphics","glut_stencil","true"));
    g_game.x_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","x_resolution","1024"));
    g_game.y_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","y_resolution","768"));
    gl_options.fullscreen = XMLSupport::parse_bool (vs_config->getVariable ("graphics","fullscreen","false"));
    gl_options.color_depth = XMLSupport::parse_int (vs_config->getVariable ("graphics","colordepth","32"));
    /*
     * Initialize SDL
     */
    if ( SDL_Init( sdl_flags ) < 0 ) {
	    WINSYS_ERR("Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    SDL_EnableUNICODE(1); // with 1, supposedly fixes int'l keyboards.

    // signal( SIGSEGV, SIG_DFL );
	SDL_Surface *icon = NULL;
    if (icon_title) {
        icon = SDL_LoadBMP(icon_title);
        if (icon) {
            SDL_SetColorKey(icon,SDL_SRCCOLORKEY,((Uint32*)(icon->pixels))[0]);
        } else {
            WINSYS_LOG(logvs::NOTICE, "warning: cannot load icon '%s': %s", icon_title, SDL_GetError());
        }
    }

    /*
     * Init video
     */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    if (get_stencil) {
        /* Not sure if this is sufficient to activate stencil buffer  */
        SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    }

    SDL_WM_SetCaption( window_title, window_title );
    if (icon) {
        SDL_WM_SetIcon(icon,0);
    }

    setup_sdl_video_mode();

    glutInit(argc,argv);

    if (gl_options.fullscreen) {
       // nothing
    } else if (XMLSupport::parse_bool(vs_config->getVariable("graphics","maximized","false"))) {
       #if 0 // stays fullscreen after that, so disable it
        WINSYS_LOG(logvs::NOTICE, "(SDL%d) Enter Fullscreen", WINSYS_SDL_MAJOR);
        SDL_WM_ToggleFullScreen(screen);
       #endif
    }

    winsys_sdl_print_gl_attributes();
}

/*---------------------------------------------------------------------------*/
/*!
  Enables/disables key repeat messages from being generated
  \return
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2021-09
*/
void winsys_set_kb_mode(unsigned int unicode, int delay_ms, int interval_ms,
                        unsigned int * unicode_bak, int * delay_ms_bak, int * interval_ms_bak)
{
    if (unicode_bak)
        *unicode_bak = kb_unicode_mode;
    if (delay_ms_bak || interval_ms_bak) {
        int sdl_delay, sdl_interval;
        SDL_GetKeyRepeat(&sdl_delay, &sdl_interval);
        if (delay_ms_bak)
            *delay_ms_bak = sdl_delay;
        if (interval_ms_bak)
            *interval_ms_bak = sdl_interval;
    }

    if (unicode != kb_unicode_mode) {
        //SDL_EnableUNICODE(unicode);
    }
    kb_unicode_mode = unicode;

    if (delay_ms != WS_KB_REPEAT_DISABLED) {
        if (delay_ms == WS_KB_REPEAT_ENABLED_DEFAULT) {
            delay_ms = WS_KB_REPEAT_DELAY;
            interval_ms = WS_KB_REPEAT_INTERVAL;
        }

        SDL_EnableKeyRepeat(delay_ms, interval_ms);

        WINSYS_LOG(logvs::NOTICE, "(SDL%d) Keyboard Repeat %s (delay:%dms interval:%dms unicode:%d)",
                   WINSYS_SDL_MAJOR, "ON", delay_ms, interval_ms, unicode);

    } else {
        SDL_EnableKeyRepeat( 0, 0 );

        WINSYS_LOG(logvs::NOTICE, "(SDL%d) Keyboard Repeat %s (unicode:%d)",
                   WINSYS_SDL_MAJOR, "OFF", unicode);
    }
}

/*---------------------------------------------------------------------------*/
/*!
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
  \date    Modified: 2005-8-16 - Rogue
  \date    Modified: 2005-12-24 - ace123
*/

void winsys_process_events()
{
    SDL_Event event;
    unsigned int key;
    SDLMod mod;
    int x, y;
    bool released;

    static bool sdl_lockaudio = XMLSupport::parse_bool(vs_config->getVariable("graphics/sdl1","lockaudio_workaround","true"));
    static bool kb_sendlower = XMLSupport::parse_bool(vs_config->getVariable("keyboard","lower_keys","true"));

    static vsUMap<unsigned int, unsigned int> keysym_to_unicode;
    //static vsUMap<SDLKey, SDLKey> keysym_to_shifted;
    static vsUMap<unsigned int, unsigned int> keysym_to_shifted;

    vsUMap<unsigned int, unsigned int>::const_iterator ituni;
    //vsUMap<SDLKey, SDLKey>::const_iterator itshift;
    vsUMap<unsigned int, unsigned int>::const_iterator itshift;

    unsigned int last_joystick_ms[MAX_JOYSTICKS];
    memset(last_joystick_ms, 0, sizeof(last_joystick_ms));

    // **************
    // **** SDL1 ****
    // **************
    if (sdl_lockaudio) {
    	SDL_InitSubSystem(SDL_INIT_AUDIO);
    }
    // On some Windows and linux systems, the mouse motion events
    // are badly handled when the cursor is hidden (winsys_show_cursor(0).
    // This is a hack to hide it without telling SDL or WM.
    winsys_show_cursor(1);
    unsigned char cdata = 0, cmask = 0;
    oldcursor = SDL_GetCursor();
    SDL_Cursor * cursor = SDL_CreateCursor(&cdata, &cmask, 1, 1, 0, 0);
    SDL_SetCursor(cursor);
    winsys_warp_pointer(g_game.x_resolution/2, g_game.y_resolution/2);

    WINSYS_LOG(logvs::NOTICE, "(SDL%d) Entering main loop...", WINSYS_SDL_MAJOR);
    
    while (true) {
    if (sdl_lockaudio) {
        SDL_LockAudio();
        SDL_UnlockAudio();
    }
    while ( SDL_PollEvent( &event ) ) {
        released = false;
        switch ( event.type ) {
        case SDL_KEYUP:
            released = true;
            // does same thing as KEYDOWN, but with different state.
        case SDL_KEYDOWN:
        mod = event.key.keysym.mod;
        if ( keyboard_func ) {
            WINSYS_DBG(logvs::DBG, "SDL%d: KEY event %s (sym:%x,uni:%x,scan:%x) (mod:%x release:%d)",
                       WINSYS_SDL_MAJOR, SDL_GetKeyName(event.key.keysym.sym),
                       event.key.keysym.sym, event.key.keysym.unicode,
                       event.key.keysym.scancode, mod, released);

            SDL_GetMouseState( &x, &y );
            bool shifton = mod&(KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS);
            bool isprintable = iswprint(event.key.keysym.sym);
            bool maybe_unicode = kb_unicode_mode && !(event.key.keysym.sym&~0xFF);

            // On some computers (eg apple), alt+ascii gives unicode specials, then we ignore that
            // if we find a key binding without unicode
            if ((kb_unicode_mode & WS_UNICODE_FULL) == 0) {
                maybe_unicode &= !HasKeyBinding(event.key.keysym.sym, mod);
            }

            // Translate untranslated release events
            if (maybe_unicode && released
            && (ituni = keysym_to_unicode.find(event.key.keysym.sym)) != keysym_to_unicode.end()) {
                event.key.keysym.unicode = ituni->second;
            }
            // Translate Shift events when unicode disabled
            // It is useful both for azerty nums 1,2... on release, and for MacOS alt+ascii which
            // gives unicode specials forbidding the bindings such as alt+x, alt+d, alt+1,2...
            // For alt+1,this is a hack which works once 1 alone has been entered. bad but better than nothing.
            else if (!kb_unicode_mode) {
                if (shifton && (isascii(event.key.keysym.sym))
                && (itshift = keysym_to_shifted.find(event.key.keysym.sym)) != keysym_to_shifted.end()) {
                    event.key.keysym.sym = (SDLKey) itshift->second;
                } else {
                    if (iswprint(event.key.keysym.unicode) && (unsigned int)event.key.keysym.unicode < 128u) {
                        if (shifton) {
                            if (!kb_sendlower || !isupper(event.key.keysym.unicode))
                            keysym_to_shifted.insert(std::make_pair((unsigned int)event.key.keysym.sym,
                                                                    event.key.keysym.unicode));
                            event.key.keysym.sym = (SDLKey) winsys_kbshift(event.key.keysym.unicode, shifton, kb_sendlower);
                        } else {
                            event.key.keysym.sym = (SDLKey) winsys_kbshift(event.key.keysym.unicode, shifton, kb_sendlower);
                        }
                    } else if (isprintable && ((unsigned int)event.key.keysym.sym) < 128u) {
                        event.key.keysym.sym = (SDLKey) winsys_kbshift(event.key.keysym.sym, shifton, kb_sendlower);
                    }
                }
            }

            bool is_unicode = maybe_unicode && event.key.keysym.unicode;
            // Remember translation for translating release events
            if (is_unicode && !released) {
                keysym_to_unicode[event.key.keysym.sym] = event.key.keysym.unicode;
            }

            // Ugly hack: prevent shiftup/shiftdown screwups on intl keyboard
            // Note: Thank god we'll have OIS for 0.5.x
            if (   shifton && is_unicode
                && shiftup(shiftdown(event.key.keysym.unicode)) != event.key.keysym.unicode)
            {
                event.key.keysym.mod = mod = SDLMod(mod & ~(KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS));
                shifton = false;
            }
            // Choose unicode or symbolic, depending on whether there is an unicode code
            // (unicode codes must be postprocessed to make sure application of the shiftup
            // modifier does not destroy it)
            key = is_unicode ? winsys_kbshift(event.key.keysym.unicode, shifton, kb_sendlower)
                             : event.key.keysym.sym;

            if (WINSYS_LOG_START(logvs::VERBOSE, "SDL%d: processing key '", WINSYS_SDL_MAJOR) > 0) {
                if (is_unicode || (isprintable && isascii(key))) {
                    logvs::vs_printf("%lc", is_unicode ? event.key.keysym.unicode : event.key.keysym.sym);
                } else {
                    logvs::vs_printf("%s", SDL_GetKeyName(event.key.keysym.sym));
                }
                WINSYS_LOG_END(logvs::VERBOSE, "' (sym:%x,uni(%d):%x,scan:%x,code:%x) (mod:%x release:%d)",
                        event.key.keysym.sym, is_unicode, event.key.keysym.unicode, event.key.keysym.scancode,
                        is_unicode?WSK_UTF32_TO_CODE(key):key, mod, released);
            }
            if (is_unicode)
                key = WSK_UTF32_TO_CODE(key);

            // Send the event
            (*keyboard_func)( key,
                      mod,
                      released,
                      x, y );
        }
        break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        if ( mouse_func ) {
            (*mouse_func)( event.button.button,
                   event.button.state,
                   event.button.x,
                   event.button.y );
        }
        break;

        case SDL_MOUSEMOTION:
        if ( event.motion.state ) {
            /* buttons are down */
            if ( motion_func ) {
            (*motion_func)( event.motion.x,
                    event.motion.y );
            }
        } else {
            /* no buttons are down */
            if ( passive_motion_func ) {
            (*passive_motion_func)( event.motion.x,
                        event.motion.y );
            }
        }
        break;

        case SDL_JOYAXISMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBALLMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        	if (joystick_func) {
        		unsigned int ticks = SDL_GetTicks();
        		int index = GetJoystickByID(event.jaxis.which);
        		if (index >= GetNumJoysticks() || last_joystick_ms[index] + WINSYS_JOY_REPEAT_MS >= ticks
        				|| joystick[index] == NULL || !joystick[index]->isAvailable())
        			break ;
        		last_joystick_ms[index] = ticks;
        		float x, y, z;
        		int buttons;
        		joystick[index]->GetJoyStick(x, y, z, buttons);
        		WINSYS_DBG(logvs::DBG, "SDL%d: joystick #%d x:%g y:%g z:%g buttons:%x",
        				    WINSYS_SDL_MAJOR, index, x, y, z, buttons);
        		joystick_func(index, joystick[index]->joy_axis[0], joystick[index]->joy_axis[1],
        				      joystick[index]->joy_axis[2], joystick[index]->joy_buttons, 0);
        	}
        	break ;

        case SDL_QUIT:
            VSExit(0); //winsys_exit(0);
            break ;

        case SDL_VIDEORESIZE:

#if (!(defined(_WIN32)&&defined(SDL_WINDOWING)) && !(defined(__APPLE__)&&defined(SDL_WINDOWING)))
        x = event.resize.w;
        y = event.resize.h;
        g_game.x_resolution = x;
        g_game.y_resolution = y;
        setup_sdl_video_mode();
        if ( reshape_func ) {
                (*reshape_func)( x, y );
        }
#else
        // nothing
#endif
        break;
        }

        if (sdl_lockaudio) {
            SDL_LockAudio();
            SDL_UnlockAudio();
        }
    }

    if ( redisplay && display_func ) {
        redisplay = false;
        (*display_func)();
    } else if ( idle_func ) {
        (*idle_func)();
    }

    /* Delay for 1 ms.  This allows the other threads to do some
       work (otherwise the audio thread gets starved). */
    SDL_Delay(1);

    }

    /* Never exits */
#define CODE_NOT_REACHED 0
    assert(CODE_NOT_REACHED);
    //    code_not_reached();
}

/*---------------------------------------------------------------------------*/
/*!
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
  static bool shutdown=false;
  if (!shutdown) {
    shutdown=true;
    SDL_SetCursor(oldcursor);
    SDL_Quit();
  }
}

#endif /* ! SDL1/SDL2 */

/*---------------------------------------------------------------------------*/
/*!
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool visible )
{
	static bool vis=true;
	if (visible!=vis) {
		SDL_ShowCursor( visible );
		vis = visible;
	}
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
  static bool called = false;

  if ( called != false)
     WINSYS_LOG(logvs::NOTICE, "winsys_atexit called twice");

  called = true;

  //atexit_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
    WINSYS_LOG(logvs::NOTICE, "(SDL%d) exiting...", WINSYS_SDL_MAJOR);
    
    winsys_show_cursor(1);
    winsys_shutdown();

    if ( atexit_func ) {
	  (*atexit_func)();
    }

    exit( code );
}

#else

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* GLUT version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static winsys_keyboard_func_t keyboard_func = NULL;
static winsys_mouse_func_t mouse_func = NULL;
static winsys_atexit_func_t atexit_func = NULL;
static winsys_joystick_func_t joystick_func = NULL;

static bool redisplay = false;
static int kb_repeat_delay = WS_KB_REPEAT_DISABLED;
static unsigned int kb_unicode_mode = WS_UNICODE_DISABLED;
static bool kb_sendlower = false;

#if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL)
static winsys_display_func_t display_func = NULL;
static winsys_idle_func_t idle_func = NULL;
static void winsys_glut_display_func();
static void winsys_glut_idle_func();
static bool joystick_pending = false;
static void ProcessSDLJoystick();
static void InitSDLJoystick();
#endif

/*---------------------------------------------------------------------------*/
/*!
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay()
{
    redisplay = true;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
#if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL)
    display_func = func;
#else
    glutDisplayFunc(func);
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
#if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL)
    idle_func = func;
#else
    glutIdleFunc( func );
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    glutReshapeFunc( func );
}

char AdjustKeyCtrl(char ch) {
  if (ch=='\0') {
    ch='2';
  }else if (ch>='0'&&ch<='9'){

  }else if (ch>=27&&ch<=31){
    ch=ch+'0'-24;
  }else if (ch==127) {
    ch='8';
  }else if (ch<=26) {
    ch+='a'-1;
  }
  return ch;
}

/* Keyboard callbacks */
static char glut_utf8_buffer[6+1] = {0,};
static unsigned int glut_utf8_buffer_size = 0;

static unsigned int glut_handle_utf8(unsigned int ch, const char * log_label) {
    unsigned int wch = ch;
    if (wch >= 128) { // glut sends several keyboard_cb to build an utf8 string
        if (!kb_unicode_mode) {
            return 0;
        }
        glut_utf8_buffer[glut_utf8_buffer_size++] = wch;
        if (((wch & 0xC0) == 0x80 && glut_utf8_buffer_size > 0) // last fragment
        || glut_utf8_buffer_size >= sizeof(glut_utf8_buffer) - 1) {
            glut_utf8_buffer[glut_utf8_buffer_size] = 0;
            wch = utf8_to_utf32(glut_utf8_buffer);
            wch = WSK_UTF32_TO_CODE(wch);
            glut_utf8_buffer_size = 0;
        } else if ((wch & 0xC0) == 0xC0){
           #ifndef VS_DEBUG_LOG
            (void)log_label;
           #endif
            WINSYS_DBG(logvs::DBG, "(GLUT) %s%u(%x) '%c' (utf8 fragment#%d)",
                       log_label,ch,ch,WS_CHAR(ch),glut_utf8_buffer_size);
            return 0;
        } else {
            glut_utf8_buffer_size = 0; // not utf8
        }
    } else {
        glut_utf8_buffer_size = 0; // ascii
    }
    return wch;
}

static void glut_keyboard_cb( unsigned char ch, int x, int y )
{
    static bool kb_alt_tab = false // does not work
        && XMLSupport::parse_bool(vs_config->getVariable("keyboard","alt_tab_override","true"));
    
    if ( keyboard_func ) {
        unsigned int wch = ch & 0xff;
        int gm = glutGetModifiers();
        if (gm) {
            if (kb_alt_tab && (gm & GLUT_ACTIVE_ALT) == GLUT_ACTIVE_ALT
            && wch == 9 && gl_options.fullscreen) {
                glutLeaveGameMode();
                glutIconifyWindow();
                return ;
            }
            if (gm&GLUT_ACTIVE_CTRL) {
                wch=AdjustKeyCtrl(wch);
            }
            WINSYS_DBG(logvs::DBG, "(GLUT) Down Modifier %d for char %d(%x) '%c' (code:%x)",
                         gm,(int)ch,(int)ch,WS_CHAR(ch),wch);
        }
        if ((wch = glut_handle_utf8(wch, "Down ")) == 0) {
            return ;
        }
        wch = winsys_kbshift(wch, (gm&(GLUT_ACTIVE_SHIFT))!=0, kb_sendlower);
        
        WINSYS_LOG(logvs::VERBOSE, "(GLUT) Down %d(%x) '%lc' (%x,code:%x)",
                   (int)ch,ch,(wchar_t)WSK_CODE_TO_UTF32(wch),WSK_CODE_TO_UTF32(wch),wch);

        (*keyboard_func)(wch, gm, false, x, y);
    }
}

static void glut_special_cb( int key, int x, int y )
{
    if ( keyboard_func ) {
#ifdef __APPLE__ //Apple glut does not have wheel support, emulate with ctrl+up/down
        int gm = glutGetModifiers();
        if (gm) {
          WINSYS_DBG(logvs::DBG, "(GLUT) Down Modifier %d for special %d %c",gm,(int)key,WS_CHAR(key));
        }
        if (gm == GLUT_ACTIVE_CTRL && mouse_func) {
            if (key == GLUT_KEY_UP) {
                (*mouse_func)(WS_WHEEL_UP, WS_MOUSE_DOWN, x, y);
                (*mouse_func)(WS_WHEEL_UP, WS_MOUSE_UP, x, y);
            } else if (key == GLUT_KEY_DOWN) {
                (*mouse_func)(WS_WHEEL_DOWN, WS_MOUSE_DOWN, x, y);
                (*mouse_func)(WS_WHEEL_DOWN, WS_MOUSE_UP, x, y);
            }
        }
#endif
        WINSYS_LOG(logvs::VERBOSE, "(GLUT) Down special %d '%c'",(int)key,WS_CHAR(key));
        (*keyboard_func)( key+128, glutGetModifiers(), false, x, y );
    }
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y )
{
    if ( keyboard_func ) {
        unsigned int wch = ch & 0xff;
        int gm = glutGetModifiers();
        if (gm) {
            WINSYS_DBG(logvs::DBG, "(GLUT) Up Modifier %d for char %d %c",gm,(int)ch,WS_CHAR(ch));
            if (gm&GLUT_ACTIVE_CTRL) {
                wch=AdjustKeyCtrl(wch);
            }
        }
        if ((wch = glut_handle_utf8(wch, "Up ")) == 0) {
            return ;
        }
        wch = winsys_kbshift(wch, (gm&(GLUT_ACTIVE_SHIFT))!=0, kb_sendlower);
        
        WINSYS_LOG(logvs::VERBOSE, "(GLUT) Up %d(%x) '%lc' (%x,code:%x)",
                   (int)ch,ch,(wchar_t)WSK_CODE_TO_UTF32(wch),WSK_CODE_TO_UTF32(wch),wch);

        (*keyboard_func)(wch, gm, true, x, y);
    }
}

static void glut_special_up_cb( int key, int x, int y )
{
    if ( keyboard_func ) {
	(*keyboard_func)( key+128, glutGetModifiers(), true, x, y );
    }
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    mouse_func = func;
    glutMouseFunc( func );
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    glutMotionFunc( func );
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    glutPassiveMotionFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the joystick callback
  \author  vsa
  \date    Created:  2021-11
*/
void myGlutJoystickCallback (unsigned int buttonmask, int x, int y, int z);
static void winsys_glut_joystick_func(unsigned int button_mask, int x, int y, int z) {
	if (joystick_func && GetNumJoysticks() > 0 && joystick[0] && joystick[0]->isAvailable()) {
		myGlutJoystickCallback(button_mask,x, y, z);
		WINSYS_DBG(logvs::DBG, "GLUT: joystick #%d x:%d y:%d z:%d buttons:%x",
		           0, x, y, z, button_mask);
		joystick_func(0, joystick[0]->joy_axis[0], joystick[0]->joy_axis[1], joystick[0]->joy_axis[2], joystick[0]->joy_buttons, 0);
	}
}
void winsys_set_joystick_func( winsys_joystick_func_t func ) {
#if !defined(NO_SDL_JOYSTICK) && !defined(HAVE_SDL)
	if ((glutDeviceGet(GLUT_HAS_JOYSTICK)||game_options.force_use_of_joystick)
	&& func != joystick_func) {
		WINSYS_LOG(logvs::VERBOSE, "GLUT: set joystick func %p (get:%d)", func, glutDeviceGet(GLUT_HAS_JOYSTICK));
		joystick_func = func;
		if (func == NULL) {
			glutJoystickFunc(NULL, game_options.polling_rate);
		} else {
			glutJoystickFunc(winsys_glut_joystick_func, game_options.polling_rate);
		}
	}
#else
	joystick_func = func;
#endif
}

/*---------------------------------------------------------------------------*/
/*!
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
    glutSwapBuffers();
    ++glswap_count;
}


/*---------------------------------------------------------------------------*/
/*!
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
    glutWarpPointer( x, y );
}

/*---------------------------------------------------------------------------*/
/*!
  Display some glut GL attributes
  \author  vsa
  \date    Created:  2021-09
*/
// Helper macros to display GL attrs
#define WS_GLUT_PRINT_GL_ATTR(attr)                             \
    {                                                           \
        int value = glutGet(attr);                              \
        WINSYS_LOG(logvs::NOTICE, "  %s = %d", #attr, value);   \
    }
void winsys_glut_print_gl_attributes() {
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_BUFFER_SIZE);
#ifdef GLUT_WINDOW_STENCIL_SIZE
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_STENCIL_SIZE);
#endif
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_DEPTH_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_RED_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_GREEN_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_BLUE_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_ALPHA_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_ACCUM_RED_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_ACCUM_GREEN_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_ACCUM_BLUE_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_ACCUM_ALPHA_SIZE);
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_DOUBLEBUFFER);
#ifdef GLUT_WINDOW_RGBA
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_RGBA);
#endif
#ifdef GLUT_WINDOW_COLORMAP_SIZE
    WS_GLUT_PRINT_GL_ATTR(GLUT_WINDOW_COLORMAP_SIZE);
#endif
}

/*---------------------------------------------------------------------------*/
/*!
  Initializes the OpenGL rendering context, and creates a window (or
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, const char *window_title,
		  const char *icon_title )
{
    int width, height;
    int glutWindow;
    int maximized;
    int status;

    winsys_common_init();

    WINSYS_LOG(logvs::NOTICE, "(GLUT) Initializing...");
    g_game.x_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","x_resolution","1024"));
    g_game.y_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","y_resolution","768"));
    gl_options.fullscreen = XMLSupport::parse_bool (vs_config->getVariable ("graphics","fullscreen","false"));
    gl_options.color_depth = XMLSupport::parse_int (vs_config->getVariable ("graphics","colordepth","32"));
    maximized = XMLSupport::parse_bool (vs_config->getVariable ("graphics","maximized","false"));
    glutInit( argc, argv );
    static bool get_stencil=XMLSupport::parse_bool (vs_config->getVariable ("graphics","glut_stencil","true"));
    kb_sendlower = XMLSupport::parse_bool(vs_config->getVariable("keyboard","lower_keys","true"));
    if (get_stencil) {
#ifdef __APPLE__
      if (!(
#endif
            glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL )
#ifdef __APPLE__
            ,1
#endif

#ifdef __APPLE__
            )) {
        glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
      }
#endif
      ;

    }else {
      glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
    }

    char str [1024];
    sprintf (str, "%dx%d:%d@60",g_game.x_resolution,g_game.y_resolution,gl_options.color_depth);
    glutGameModeString(str);
    WINSYS_LOG(logvs::NOTICE, "(GLUT) Game Mode Params %dx%d at depth %d @ %d Hz",
    		   glutGameModeGet( GLUT_GAME_MODE_WIDTH ),glutGameModeGet( GLUT_GAME_MODE_HEIGHT ),
			   glutGameModeGet( GLUT_GAME_MODE_PIXEL_DEPTH ),glutGameModeGet( GLUT_GAME_MODE_REFRESH_RATE ));

    /* Create a window */
    if ( gl_options.fullscreen &&(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)!=-1)) {
    	glutInitWindowPosition( 0, 0 );
    	if ((status = glutEnterGameMode()) != 0) {
            WINSYS_LOG(logvs::NOTICE, "(GLUT) Entered Game Mode, Params %dx%d at depth %d @ %d Hz",
                       glutGameModeGet( GLUT_GAME_MODE_WIDTH ), glutGameModeGet( GLUT_GAME_MODE_HEIGHT ),
				       glutGameModeGet( GLUT_GAME_MODE_PIXEL_DEPTH ), glutGameModeGet( GLUT_GAME_MODE_REFRESH_RATE ));
        } else {
            WINSYS_LOG(logvs::WARN, "(GLUT) Cannot enter in Game Mode, trying to create a window...");
        }
    } else {
        status = 1;
    }
    if (!status) {
        /* Set the initial window size */
        glutInitWindowSize( g_game.x_resolution,g_game.y_resolution );

        glutWindow = glutCreateWindow(window_title);

        if ( glutWindow == 0 ) {
            WINSYS_ERR("(GLUT) Couldn't create a window.");
            exit(1);
        }
        if (maximized || gl_options.fullscreen) {
            WINSYS_LOG(logvs::NOTICE, "(GLUT) enter fullscreen");
            glutFullScreen();
        }
    }
#if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL)
    glutDisplayFunc(winsys_glut_display_func);
    glutIdleFunc(winsys_glut_idle_func);
    InitSDLJoystick();
#endif
    winsys_glut_print_gl_attributes();
}


/*---------------------------------------------------------------------------*/
/*!
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
  static bool shutdown=false;
  if (!shutdown) {
    shutdown=true;
    if ( gl_options.fullscreen ) {
	glutLeaveGameMode();
    }
  }
}

/*---------------------------------------------------------------------------*/
/*!
  Enables/disables key repeat messages from being generated
  \return
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2021-09
*/
void winsys_set_kb_mode(unsigned int unicode, int delay_ms, int interval_ms,
                        unsigned int * unicode_bak, int * delay_ms_bak, int * interval_ms_bak)
{
    if (delay_ms_bak)
        *delay_ms_bak = kb_repeat_delay;
    if (unicode_bak)
        *unicode_bak = kb_unicode_mode;
    (void)interval_ms;
    (void)interval_ms_bak;
    kb_repeat_delay = delay_ms;
    kb_unicode_mode = unicode;

    glutIgnoreKeyRepeat(delay_ms == WS_KB_REPEAT_DISABLED);

    if (delay_ms != WS_KB_REPEAT_DISABLED) {
        WINSYS_LOG(logvs::NOTICE, "(GLUT) Keyboard Repeat %s "
                   "(delay:%dms interval:%dms-ignored unicode:%d)",
                   "ON", kb_repeat_delay, interval_ms, unicode);
    } else {
        WINSYS_LOG(logvs::NOTICE, "(GLUT) Keyboard Repeat %s (unicode:%d)", "OFF", unicode);
    }
}

/*---------------------------------------------------------------------------*/
/*!
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool visible )
{
	static bool vis=true;
	if (visible!=vis) {

    if ( visible ) {
	glutSetCursor( GLUT_CURSOR_LEFT_ARROW );
    } else {
	glutSetCursor( GLUT_CURSOR_NONE );
    }
	vis = visible;
	}
}

void entry_cb(int state) {
    static bool redim_done = !XMLSupport::parse_bool(vs_config->getVariable ("graphics","maximized","false"));
    static int oldstate = INT_MIN;
    if (state == oldstate) {
        return ;
    }
    oldstate = state;
    WINSYS_DBG(logvs::DBG, "GLUT entry %d", state);
    if (state == GLUT_LEFT) {
        if (!redim_done) {
            redim_done = true;
            glutPositionWindow(0,0);
            glutReshapeWindow(g_game.x_resolution, g_game.y_resolution);
        }
        winsys_show_cursor(1);
    } else if (state == GLUT_ENTERED) {
        winsys_show_cursor(0);
    }
}

/*---------------------------------------------------------------------------*/
/*!
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
    /* Set up keyboard callbacks */
    glutKeyboardFunc( glut_keyboard_cb );
    glutKeyboardUpFunc( glut_keyboard_up_cb );
    glutSpecialFunc( glut_special_cb );
    glutSpecialUpFunc( glut_special_up_cb );

    if (!gl_options.fullscreen){
        glutEntryFunc(entry_cb);
    }

    WINSYS_LOG(logvs::NOTICE, "(GLUT) Entering main loop...");
    glutMainLoop();
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool called = false;

    if (called)
      WINSYS_LOG(logvs::NOTICE, "winsys_atexit called twice");

    called = true;

    //atexit_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
    WINSYS_LOG(logvs::NOTICE, "(GLUT) exiting...");

    winsys_show_cursor(1);
    winsys_shutdown();

    if ( atexit_func ) {
      (*atexit_func)();
    }

    exit(code);
}

# if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL)
#  include "SDL.h"

#ifdef SDL_MAJOR_VERSION
# define WINSYS_SDL_MAJOR SDL_MAJOR_VERSION
#else
# define WINSYS_SDL_MAJOR 0
#endif

void winsys_glut_idle_func() {
	ProcessSDLJoystick();
	if (idle_func) {
		idle_func();
	}
}

void winsys_glut_display_func() {
	ProcessSDLJoystick();
	if (display_func) {
		display_func();
	}
}

static unsigned int last_joystick_ms[MAX_JOYSTICKS];

static void ProcessSDLJoystick() {
	SDL_Event event;
	SDL_JoystickUpdate();
#if SDL_VERSION_ATLEAST(2,0,0)
	while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) > 0) {
#else
	while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS) > 0) {
#endif
		switch(event.type) {
#if SDL_VERSION_ATLEAST(2,0,0)
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			if (SDL_NumJoysticks() != GetNumJoysticks()) {
				WINSYS_LOG(logvs::INFO, "(GLUT) SDL%d: Joystick %s %d", WINSYS_SDL_MAJOR,
						event.jdevice.type == SDL_JOYDEVICEREMOVED ? "REMOVED, id:" : "ADDED, index:",
						event.jdevice.which);
				UpdateJoystick(event.jdevice.type == SDL_JOYDEVICEREMOVED
						       ? GetJoystickByID(event.jdevice.which) : GetNumJoysticks());
			}
			break ;
#endif
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (joystick_func) {
#if SDL_VERSION_ATLEAST(2,0,0)
				unsigned int ticks = event.jaxis.timestamp;
#else
				unsigned int ticks = SDL_GetTicks();
#endif
				int index = GetJoystickByID(event.jaxis.which);
				if (index >= GetNumJoysticks() || last_joystick_ms[index] + WINSYS_JOY_REPEAT_MS >= ticks
						|| joystick[index] == NULL || !joystick[index]->isAvailable())
					break ;
				last_joystick_ms[index] = ticks;
				float x, y, z;
				int buttons;
				joystick[index]->GetJoyStick(x, y, z, buttons);
				WINSYS_DBG(logvs::DBG, "(GLUT) SDL joystick #%d x:%g y:%g z:%g buttons:%x",
						   index, x, y, z, buttons);
				joystick_func(index, joystick[index]->joy_axis[0], joystick[index]->joy_axis[1],
						      joystick[index]->joy_axis[2], joystick[index]->joy_buttons, 0);
			}
			break ;
		}
	}
}

#if SDL_VERSION_ATLEAST(2,0,0)
static int winsys_glut_sdl_event_filter(void * data, SDL_Event * event) { (void) data;
#else
static int winsys_glut_sdl_event_filter(const SDL_Event * event) {
#endif
	switch(event->type) {
#if SDL_VERSION_ATLEAST(2,0,0)
	case SDL_JOYDEVICEADDED:
	case SDL_JOYDEVICEREMOVED:
#endif
	case SDL_JOYAXISMOTION:
	case SDL_JOYHATMOTION:
	case SDL_JOYBALLMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		return 1;
	default:
		return 0;
	}
}

static void InitSDLJoystick() {
	int ret;
	memset(last_joystick_ms, 0, sizeof(last_joystick_ms));
#if SDL_VERSION_ATLEAST(2,0,0)
	ret = SDL_Init(SDL_INIT_JOYSTICK);
#else
	if ((ret = SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_VIDEO ))) { // INIT_VIDEO NEEDED with SDL1 in order to have events
		WINSYS_LOG(logvs::WARN, "(GLUT) Couldn't initialize SDL%d Joystick Events: %s",
			       WINSYS_SDL_MAJOR, SDL_GetError());
		ret = SDL_Init( SDL_INIT_JOYSTICK );
	}
#endif
	if (ret) {
		WINSYS_LOG(logvs::ERROR, "(GLUT) Couldn't initialize SDL%d Joystick: %s",
				   WINSYS_SDL_MAJOR, SDL_GetError());
		winsys_exit(1);
	}
	SDL_SetEventFilter(winsys_glut_sdl_event_filter
#if SDL_VERSION_ATLEAST(2,0,0)
			,NULL
#endif
			);
}

# endif /* ! #if !defined(NO_SDL_JOYSTICK) && defined(HAVE_SDL) */

#endif /* defined( SDL_WINDOWING ) */

/* EOF */
