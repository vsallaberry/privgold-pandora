
## Privateer Gold Pandora / Vegastrike for Privateer Gemini Gold
------------------

* [Introduction](#introduction)
* [Installation](#installation)
* [Source Control Management](#source-control-management)
* [About Licenses](#about-licenses)
* [Updates Overview](#updates-overview)
* [Updates Detail](#updates-detail)
* [About](#about)
* [Credits](#credits)

## Introduction

This is based on Privateer Gold v1.03 (2009) but 
this is not an official release of vegastrike or Privateer Gold.  
  
The first Goal was to have a 64bit MacOS build: this is when the pandora
box has been opened, implying a lot more updates.
  
Tested on macOS El Capitan, Mojave and Catalina, linux debian 9, and on virtual machines with Windows 7&10.
Vegastrike now supports 3 graphical engines for openGL: GLUT, SDL1, SDL2.
SDL1 is discouraged, SDL2 is recommended, use GLUT if trouble with SDL2.
Unicode handling (keyboard and graphics) has been improved.
  
A big credit and thanks to [Vegastrike Team](https://www.vega-strike.org), 
[Privateer Gemini Gold Team](https://priv.solsector.net),
[Origin Systems Inc](https://en.m.wikipedia.org/wiki/Origin_Systems), 
and [macports](https://www.macports.org) for useful hints and patches, 
[MSYS2](https://sourceforge.net/projects/msys2) for open-source libraries ports on windows.  
  
See [source-control](#source-control-management), 
[licenses](#about-licenses),
and [credits](#credits) for more details.


## Installation

Binaries are available [here](https://github.com/vsallaberry/privgold-pandora/releases)  
 
Tested on:  
* el capitan
* mojave
* catalina
  
* Experimental non-tested build for macOS BigSur or later with arm64 (apple M1).  
* Experimental build for linux-x86_64 (tested on a macbook with freshly installed debian9)
* Experimental build for Windows-x86_64 (tested with windows 10 & 7 in Virtual Machines) 
  
* MacOS:
  + supported architectures are i386, x86_64, arm64.
  + open the APP with alt (option) key pressed in order to run SETUP.


## Source Control Management

* updates are done based on following version:  
    $ svn info  
    URL: https://svn.code.sf.net/p/privateer/code  
    Relative URL: ^/  
    Repository Root: https://svn.code.sf.net/p/privateer/code  
    Repository UUID: f9092690-2135-0410-9f1c-825117feb74d  
    Revision: 258  
    Node Kind: directory  
    Schedule: normal  
    Last Changed Author: johncordell  
    Last Changed Rev: 258  
    Last Changed Date: 2010-01-08 10:05:48 +0100 (Ven, 08 jan 2010)  
  
It contains the vegastrike engine: privgold/dependencies/vs_gold_1_03.tar.bz2  
Privateer Gold Gemini Gold v1.03 (last official version, 2009, dec) 
is SVN revision 257, the 258 contains one single fix in Privateer data.


## About Licenses

* All art provided here comes from Privateer Gemini Gold 1.03, and all credits 
  about that must come to Privateer Gemini Gold Team and Origin Systems Inc.

* Privateer Gemini Gold licence : non commercial use
  <https://svn.code.sf.net/p/privateer/code>
  <https://priv.solsector.net>
  
* Vegastrike license (as of 2009) : GPL v2
  <https://svn.code.sf.net/p/privateer/code/privgold> : dependencies/vs_gold_1_03.tar.bz2)

* Extract from Privateer Gemini Gold Manual (1.03)
  
______  
__Important__    
We offer our thanks to the authors of the original Privateer for their 
inspirational game that drove us to recreate it so that it works on our 
computers once again. We know their hard work and effort has not gone 
to waste with the obsolescence of the old game, but instead provided 
the seeds for and has fed and watered and allowed this remake to grow.  

We would like to remind users that while the source code to the game 
engine (vegastrike) is GPL'd, the engine data used in Privateer Gemini Gold 
is for personal use only, not to be involved with anything promotional 
or commercial.  

We would like to credit the authors of Privateer and Origin Systems Inc. 
for their ideas and images that allow Privateer Gemini Gold to be what 
it is today...
Some descriptions and images in this manual were extracted from the 
original Privateer Manual created by Origin Systems Inc. © 1993.
______  
  
* Except some MacOS system libs, libs used by vegastrike engine are 
open sources with various licences (mit, gpl, lgpl, ...)
'deps' folder contains scripts and patches used to build these dependencies
for macOS or linux. See [credits](#credits) for more details.

* unlike macOS and linux releases for which I rebuilt myself needed libraries,
the Windows release is shipped with open sources libraries provided
by [MSYS2](https://sourceforge.net/projects/msys2), which is LGPL itself, and 
the libraries it uses are open sources with various licences (mit,bsd,gpl,lgpl,...)
There are currently three exceptions: I rebuilt myself ffmpeg, libpng 
(need old versions), and SDL 1 (msys2 has gcc build, needed a clang build).

## Updates Overview

Updates on Vegastrike Engine and Privateer Gemini Gold scripts.

* Focus on building/running on MacOS with recent compilers
* very few (or none) updates needed to build with gcc 6.5
* more updates needed in engine source code to build
  with gcc >=7 or clang, with c++98 or c++11
  need boost 1.50 to use cxx11
* Unicode (UTF8)
* SDL2
* more choices for fonts (hud,base,oldcomputer)
* optional display of FPS/loop rate
* optional display of key bindings
* optional override of a config variable via command-line
* uniformization of logging messages
* message center alternative to handle fixers messages during campaign battles
* bug fixes since 1.03:
  + Oxford Matterson library message was not delivered to the right room
  + in addition to the oxford fix, now sounds are also played in the right room
  + QUINE was not displaying all on-going missions
  + probably fixed: a random crash in gl light engine when jumping
* not fixed since 1.03: 
  + addition of random parts to ship, the only way to solve it
    for now is to edit the '~/.privgold100/serialized_xml/<save>/<ship>.begin.csv,
    and remove the part {...}.
  + still some memory leaks, but much less now. The game can run for hours.
  + accept/reject option in campaign dialog remains when leaving the room
    without answering
* the cockpit is no more shown during game loading before to show the base
* Navigation Screen margins computation improvement on 16/9 resolutions
* keyboard repeat is enabled for gui text input sequences
* BaseInterface Links&Objects lookup optimization and improvement of memory leaks 
* BaseInterface: Links navigation with keyboard or joystick
* joystick: now joystick handling is event-based, and plug&play with SDL2.
* 1.2.1: experimental build for macOS BigSur and Apple M1 (arm64)
* 1.2.2: experimental build for linux-x86_64 (tested with a freshly installed
  debian9 on a x86_64 mac)
* 1.2.3: 
  + EXPERIMENTAL build for windows-x86_64 (tested only with windows 10 & 7 on 
    Virtual Machines). In this Windows release the HOME dir (where user data including 
    savegames are stored) is now in Windows UserProfile Folder instead of the data 
    directory (eg: 'c:\users\NAME\Local Settings\privgold120'). 
  + SDL1 mouse workaround on some windows and linux systems (mouse was not moving)
  + fix: FontHeight management in cockpit when Cockpit font is not base font
  + fix: (1.03 bug) don't crash if the savegame cannot be saved for some reasons 
    (eg:System does not allow name)

## Updates Detail

Here is a non exhaustive list of updates:  
Note: engine refer to Vegastrike and data refer to PrivateerGold scripts.

SDL2, Joystick, UTF8, MemoryLeaks, Fonts, CampaignMessages, Log, and more...

* ------------------------------------------------------------------------
* bug fixes since 1.03 :
  + Oxford Matterson library message was not delivered to the right room, but
    the the launch pad.

  + in addition to the oxford fix, now sounds are also played in the right room

  + python missions database was cleared on success of any mission,
    which lead to an empty mission list in QUINE even if there were others
    (old basecomputer could see them). To reproduce the bug on old
    version, accept 2 missions, finsih one and land. you will see on
    Quine an empty mission list. Cause: all missions had same ID=0.

  + probably fixed: random crash when jumping : i did reproduce it
    with gdb (crash in gl_light_state>unpicklights(), then applied a patch provided
    by a 2012 svn commit (see rev 297), and did not see the issue again after hours,
    of jumps and starsystems garbage collection.
    In addition i changed something in universe->StartDraw() to prevent a system
    to be garbage collected before the python scripts have run.

  + overwrite savegame in QUINE, now confirmation is handled well even if the
    existing file is typed by user. And 'New_Game' name is forbidden.

  + QUINE: SaveGames with underscores are correctly displayed.

  + time was running too fast when several missions were on-going
    (increment of SIMULATION_ATOM per mission)

* ------------------------------------------------------------------------
* NOT fixed since 1.03:
  + addition of random parts to ship, the only way to solve it
    for now is to edit the '~/.privgold100/serialized_xml/<save>/<ship>.begin.csv,
    and remove the part {...}.
  + still some memory leaks, but much less. The game can run for hours.
    Part of the Memory usage is normal as the game caches 15 star systems, but some
    other are not. I started easyDomNode cleaning, and finished-(Mission*) cleaning,
    but of course it is not enough/finished.
  + in some campaign dialogs with accept/reject option, the accept/reject is not
    cleared if we leave the room without answering

* ------------------------------------------------------------------------
* engine: override a XML config variable via command line
  (eg: ./vegastrike -Cgraphics/fullscreen=no)

* engine/python: LOGGING
  + starting to use a common log module,
    progressively replacing printfs, cout, ...

* the cockpit is no more shown during game loading before to show the base

* the Navigation Screen margins computation for the System and Sector Map
  has been improved for 16/9 resolutions (1680x1050,1280x800,...).
  No Change for 4/3 resolutions

* message center alternative to handle fixers messages during campaign battles
  (when a campaign fixer has some thing to tell you, it will only
  displayed when docked at a base, now, it is display during flight.

* engine: HashTable and Unit Container
  + don't use tr1 unordered maps
  + use gnu hash_map or cxx11 unordered_map
  + use New STL collection or old collection.
    Currently Old one is used.

* engine: SDL2 support
  SDL2 provides better unicode support, better window management support (resize, ...),
  and joystick plug and play support.
  - engine/sdl2: alt+tab will minimize game in fullscreen
  - SDL: possibility to disable the SDL LockAudio workaround

* Joystick/Mouse: reduce polling, joystick when docked
  + Only SDL is polling the joystick (via SDL_PollEvent).
    Now the mouse and joystick interface register to a joystick
    winsys event instead of polling joystick state.
  + With SDL2, the joystick can be plugged/unplugged while the
    the game is running.

* BaseInterface Links&Objects lookup optimization and improvement of memory leaks
  + Links&Objects lookup optimization
    Now Links and Object are stored in a multimap which speeds up the
    lookups and deletion. Because the order of insertion matters when
    drawing, the multimap contains chained object in order of insertion
    without adding complexity to multimap lookup/insertion/deletion.
    This redesign allowed to fix a huge memory leak (can double the RAM
    usage in few minutes).
  + Links Navigation: we can now navigate between links with keyboard
    (arrow keys, space, enter) or joystick

* engine: improved sdl1 and glut support:
  + sdl1/keyboard: alt+x or any alt+ascii were not working on azerty mac,
    and maybe qwerty also, because of alt+key on mac gives unicode.
  + glut/keyboard: handle utf8 input
  + glut/mousewheel: emulation of mouse wheel with ctrl+up/down on glut
    (no wheel support on glut for mac)

* engine/data: unicode(utf8) support on keyboard and on display
  + engine/data: update all all gui elements taking keyboard input
  + engine/data: update some GUI elements that could display unicode
  + engine/data: management '##' escape sequence used to display a '#'
  + engine: update of Font class
  + engine: glut/sdlx updates to handle unicode 32bits input
  + Still TODO: the gfx/gui PaintText class is not fully utf8.
    The Trucation of text needs some more work. Not done because, on
    Privateer, the PaintText class is only used by the 'OLD Base Computer'

* engine/setup:
  + build both gtk and dialog versions
  + gtk/dialog: if available, open Manual.pdf instead of readme
  + choose fonts, graphic engine, unicode configuration, setup program.

* bundle:
  + regroup game and setup in the same bundle
  + run Setup by pressing 'alt' when running the game
  + add 3 differents graphical engines (separate binaries): GLUT, SDL1, SDL2.

* main_loop:
  + optional display of FPS and loop rate (alt+p)
  + optional display of key bindings during flight (alt+h)
  + optional display of logs during flight via message center (alt+m)

* data: UTF8 Savegames AND home folder
  + home folder is now .privgold120. When running the app bundle,
    the .privgold100 is automatically duplicated to .privgold120
  + This version is backward-campatible with 1.0x (.privGold100).
    But the contrary is not fully true about savegames. Indeed,
    because of UTF8 support, the savegames saved from new version
    are not directly compatible with old one, but the tool
    unicode-conv (Contents/Resources/bin/unicode-conv) can convert a new
    savegame to the 1.0x old format so that they run on old version:
    'unicode-conv --convert-back ~/.privgold120/save/Game old_Game'
  + When loading an old savegame, it is converted on-the-fly to utf8.

* tools
  + build objconv tools and others with cmake
  + add unitary tests for Utf8Iterator and ChainedMultimap
  + add a tool converting a file or privateer savegame to/from UTF8
  + Generation of the C++ Python API files (empty functions) in order
    to be used in an IDE Completion System
  + Detection of Space/Tabs usage in python files

## About

Privateer Gold Pandora (Vincent Sallaberry, 2021-2022) is a fork of
Privateer Gemini Gold 1.03, 2009. It includes builds for recent
MacOS systems, updates/new features and experimental builds for
x86_64 linux and Windows.  

https://github.com/vsallaberry/privgold-pandora

## Credits

* [GitHub](https://github.com)
* [Vegastrike Team](https://www.vega-strike.org)
* [Privateer Gemini Gold Team](https://priv.solsector.net)
* [Origin Systems Inc](https://en.m.wikipedia.org/wiki/Origin_Systems)
* [macports](https://www.macports.org)
* [Xcode-SDK](https://developer.apple.com/xcode)
* [MSYS2](https://sourceforge.net/projects/msys2)
* [GCC](https://gcc.gnu.org)
* [CLANG](https://clang.llvm.org)
* [GnuTools](https://www.gnu.org/software)
* [SDL](http://www.libsdl.org)
* [FFMPEG](https://ffmpeg.org)
* [OpenAL](http://www.openal.org)
* [LibOGG](http://www.vorbis.com)
* [LibVorbis](https://xiph.org/vorbis)
* [OpenGL](http://www.opengl.org)
* [FreeGLUT](http://freeglut.sourceforge.net)
* [GTK](https://www.gtk.org)
* [Python](https://www.python.org)
* [Boost](http://www.boost.org)
* [Expat](http://www.libexpat.org)
* [CMake](https://cmake.org)
* [PNG](http://www.libpng.org/pub/png/libpng.html)
* [JPEG](http://www.ijg.org)
* [JPEG-Turbo](https://www.libjpeg-turbo.org)
* [BZIP2](https://sourceware.org/bzip2)
* [ZLIB](http://www.zlib.net)
* [NCurses](https://invisible-island.net/ncurses)
* [LibDialog](https://invisible-island.net/dialog)
* [Xorg](https://www.x.org)
* [Mesa](https://www.mesa3d.org)
* [VirtualBox](https://www.virtualbox.org)
* and more...

