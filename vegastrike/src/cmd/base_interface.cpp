/*
 * Vega Strike
 * Copyright (C) 2001-2021 VegaStrike developers
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
#include "config.h"
#include <Python.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "gldrv/winsys.h"
#include "vsfilesystem.h"
#include "lin_time.h"
#include "audiolib.h"
#include "gfx/camera.h"
#include "gfx/cockpit_generic.h"
#include "python/init.h"
#include "python/python_compile.h"
#include "planet_generic.h"
#include <algorithm>
#include "base_util.h"
#include "config_xml.h"
#include "save_util.h"
#include "unit_util.h"
#include "networking/netclient.h"
#include "gfx/cockpit.h"
#include "gfx/ani_texture.h"
#include "music.h"
#include "lin_time.h"
#include "load_mission.h"
#include "universe_util.h"
#include "gui/guidefs.h"
#ifdef RENDER_FROM_TEXTURE
#include "gfx/stream_texture.h"
#endif
#include "main_loop.h"
#include "in_mouse.h"
#include "in_kb.h"
#include "in_joystick.h"
#include "options.h"
#include "command.h"
#include "log.h"
#include "vs_log_modules.h"

extern vs_options game_options;

static unsigned int& getMouseButtonMask()
{
	static unsigned int mask = 0;
	return mask;
}

static void biModifyMouseSensitivity(int &x, int &y, bool invert){
  int xrez=g_game.x_resolution;
  int yrez=g_game.y_resolution;
  static int whentodouble=XMLSupport::parse_int(vs_config->getVariable("joystick","double_mouse_position","1280"));
  static float factor=XMLSupport::parse_float(vs_config->getVariable("joystick","double_mouse_factor","2"));
  if (xrez>=whentodouble) {
    x-=g_game.x_resolution/2;
    y-=g_game.y_resolution/2;
    if (invert) {
      x=int(x/factor);
      y=int(y/factor);
    }else {
      x=int(x*factor);
      y=int(y*factor);
    }
    x+=g_game.x_resolution/2;
    y+=g_game.y_resolution/2;
    if (x>g_game.x_resolution)
      x=g_game.x_resolution;
    if (y>g_game.y_resolution)
      y=g_game.y_resolution;
    if (x<0) x=0;
    if (y<0) y=0;
  }
}
static bool createdbase=false;
static int createdmusic=-1;
void ModifyMouseSensitivity(int &x, int &y) {
  biModifyMouseSensitivity(x,y,false);
}
#ifdef BASE_MAKER
 #include <stdio.h>
 #ifdef _WIN32
  #include <windows.h>
 #endif
static char makingstate=0;
#endif
extern const char *mission_key; //defined in main.cpp
bool BaseInterface::Room::BaseTalk::hastalked=false;
using namespace VSFileSystem;
#define NEW_GUI

#ifdef NEW_GUI
#include "basecomputer.h"
#include "../gui/eventmanager.h"
#endif
std::vector<unsigned int>base_keyboard_queue;
static void CalculateRealXAndY (int xbeforecalc, int ybeforecalc, float *x, float *y) {
	(*x)=(((float)(xbeforecalc*2))/g_game.x_resolution)-1;
	(*y)=-(((float)(ybeforecalc*2))/g_game.y_resolution)+1;
}
#define mymin(a,b) (((a)<(b))?(a):(b))
static void SetupViewport() {
        static int base_max_width=XMLSupport::parse_int(vs_config->getVariable("graphics","base_max_width","0"));
        static int base_max_height=XMLSupport::parse_int(vs_config->getVariable("graphics","base_max_height","0"));
        if (base_max_width&&base_max_height) {
          int xrez = mymin(g_game.x_resolution,base_max_width);
          int yrez = mymin(g_game.y_resolution,base_max_height);
          int offsetx = (g_game.x_resolution-xrez)/2;
          int offsety = (g_game.y_resolution-yrez)/2;
          glViewport(offsetx,offsety,xrez,yrez);
        }
}
#undef mymin

// MultiMap BaseObj & Link Pretty-Printers
namespace ChainedMultimapUtil {
	template <> std::string multimap_debug_key(BaseInterface::Room::BaseObj * obj) {
		if (obj == NULL) return "(null)";
		std::ostringstream oss; oss << "(" << obj->index << "," << (const void *) obj << ")";
		return oss.str();
	}
	template <> std::string multimap_debug_key(BaseInterface::Room::Link * link) {
		if (link == NULL) return "(null)";
		std::ostringstream oss; oss << "(" << link->index << "," << (const void *) link << ")";
		return oss.str();
	}
} // ! namespace ChainedMultimapUtil

BaseInterface::Room::~Room () {
	BASE_LOG(logvs::INFO, "ByeBye Room %s - %zu objs, %zu links",
			 this->deftext.c_str(), objsmap.size(), linksmap.size());
	for (linksmap_type::iterator it = linksmap.begin(); it != linksmap.end(); ++it) {
		if (it->second) {
		   #ifdef BASELINK_CHAINED_MULTIMAP
			if (it->second->elt) { // destructor of ChainedMultimap will delete it->second
				delete (it->second->elt);
				it->second->elt = NULL;
			}
		   #else
			delete (it->second);
			it->second = NULL;
		   #endif
		}
	}
	for (objsmap_type::iterator it = objsmap.begin(); it != objsmap.end(); ++it) {
		if (it->second) {
	       #ifdef BASEOBJ_CHAINED_MULTIMAP
			if (it->second->elt) {
				delete (it->second->elt); // destructor of ChainedMultimap will delete it->second
				it->second->elt = NULL;
			}
		   #else
			delete (it->second);
			it->second = NULL;
		   #endif
		}
	}
}

bool BaseInterface::Room::ObjsKeyCompare::operator()(const std::string & s1, const std::string & s2) const {
	return s1 < s2;
}

BaseInterface::Room::Room ()
: objsmap(objsmap_type(/*ObjsKeyCompare()*/)),
  linksmap(linksmap_type(/*ObjsKeyCompare()*/))
#if defined(BASEOBJ_ALPHAORDER_MULTIMAP)
  , _busyobj(false)
#endif
#if defined(BASELINK_CHAINED_MULTIMAP)
, hotlink_it(linksmap.none())
#elif defined(BASELINK_ALPHAORDER_MULTIMAP)
  , _busylink(false), hotlink_it(linksmap.end())
#endif
{
	Enter();
}

void BaseInterface::Room::Enter() {
#if defined(BASELINK_CHAINED_MULTIMAP)
	hotlink_it = linksmap.none();
#elif defined(BASELINK_ALPHAORDER_MULTIMAP)
	hotlink_it = linksmap.end();
#else
	hotlink_idx = (size_t)-1;
#endif
}

#if defined(BASEOBJ_CHAINED_MULTIMAP)
# define BASEOBJ_VECSZ (0LU)
# define BASEOBJ_BUSY(map) ((map).busy())
#elif defined(BASEOBJ_ALPHAORDER_MULTIMAP)
# define BASEOBJ_VECSZ (0LU)
# define BASEOBJ_BUSY(map) (_busyobj)
#else
# define BASEOBJ_VECSZ (objs.size())
# define BASEOBJ_BUSY(map) (true)
#endif

BaseInterface::Room::objsmap_type::iterator
BaseInterface::Room::AddObj(BaseObj* obj, bool front) {
	BASE_DBG(logvs::DBG+1, "AddObj(%s): %s new obj (%p) mapSz:%zu vecSz:%zu",
		     obj ? obj->index.c_str() : "(null)", front? "inserting" : "pushing", obj, objsmap.size(), BASEOBJ_VECSZ);

	objsmap_type::iterator it = objsmap.insert(std::make_pair(obj->index, obj)
#if defined(BASEOBJ_CHAINED_MULTIMAP)
			       , front);
#else
	               );
# if !defined(BASEOBJ_ALPHAORDER_MULTIMAP)
	if (front) {
		objs.insert(objs.begin(), obj);
	} else {
		objs.push_back(obj);
	}
# endif
#endif
	BASE_DBG(logvs::DBG+3, "AddObj(%s) %s", obj ? obj->index.c_str() : "(null)", it != objsmap.end() ? "ok" : "failed");
	return it;
}

BaseInterface::Room::objsmap_type::iterator
BaseInterface::Room::EraseObj(const std::string & index, bool on_ptr, BaseObj * obj, bool only_notvalid, bool force) {
	objsmap_range_type its = this->GetObjRange(index);
	for ( ; its.first != its.second; ) {
		BaseObj * zombie = GetObj(its.first);
		if ((on_ptr && obj != zombie)
		||  (only_notvalid && CheckObj(zombie))) { ++its.first; continue ; }

		if (!force && BASEOBJ_BUSY(objsmap)) {
			if (CheckObj(zombie)) {
				BASE_DBG(logvs::DBG+1, "EraseObj(%s): INVALIDATING '%s' (%p) mapSz:%zu vecSz:%zu",
						index.c_str(), zombie ? zombie->index.c_str() : "(null)", zombie, objsmap.size(), BASEOBJ_VECSZ);
				zombie->valid = false;
			} else if (!zombie) {
				BASE_LOG(logvs::WARN, "EraseObj(%s): Trying to INVALIDATE NULL Object!", index.c_str());
			}
			++its.first;
			continue;
		}
		BASE_DBG(logvs::DBG+1, "EraseObj(%s): DELETING obj '%s' (%p) mapSz:%zu vecSz:%zu",
				 index.c_str(), zombie ? zombie->index.c_str() : "(null)", zombie, objsmap.size(), BASEOBJ_VECSZ);

	   #if defined(BASEOBJ_CHAINED_MULTIMAP) || defined(MULTIMAP_HAVE_CXX11_ERASE) // only ChainedMultimap or CXX11 std::multimap
		its.first = objsmap.erase(its.first);
	   #else
		its.first = ChainedMultimapUtil::multimap_cxx11_erase(objsmap, its.first);
	   #endif
		if (zombie)
			delete zombie;
	}
	return its.first;
}

#if defined(BASELINK_CHAINED_MULTIMAP)
# define BASELINK_VECSZ (0LU)
# define BASELINK_BUSY(map) ((map).busy() > 1) // Room::hotlink_it holds one reference then: busy if > 1
# define BASELINK_FOREACH(_room, _link) BASELINK_FOREACH0(_room, _link, _it, *(_it), ++(_it))
# define BASELINK_FOREACH2(_room, _link) BASELINK_FOREACH0(_room, _link, _it, *((_it)++), (void)0)
# define BASELINK_FOREACH0(_room, _link, _it, _assign, _postincr) \
	for (Room::Link * _link = (Room::Link*)1; _link; _link = 0) \
		for (Room::linksmap_type::chained_iterator _it = (_room)->linksmap.first(); \
		     ! _it.end() && ((_link=(_assign))||1); _postincr)
#elif defined(BASELINK_ALPHAORDER_MULTIMAP)
# define BASELINK_VECSZ (0LU)
# define BASELINK_BUSY(map) (_busylink)
# define BASELINK_FOREACH(_room, _link) BASELINK_FOREACH0(_room, _link, _it, (_it)->second, ++(_it))
# define BASELINK_FOREACH2(_room, _link) BASELINK_FOREACH0(_room, _link, _it, (_it++)->second, (void)0)
# define BASELINK_FOREACH0(_room, _link, _it, _assign, _postincr) \
	for (Room::Link * _link = (Room::Link*)1; _link && ((_room->_busylink = true)||1); _link = 0, _room->_busylink=false) \
		for (linksmap_type::iterator _it = (_room)->linksmap.begin(); \
		     _it != (_room)->linksmap.end() && ((_link=(_assign))||1); _postincr)
#else
# define BASELINK_VECSZ (links.size())
# define BASELINK_BUSY(map) (true)
# define BASELINK_FOREACH(_room, _link) BASELINK_FOREACH0(_room, _link, _it, *(_it), ++(_it))
# define BASELINK_FOREACH2(_room, _link) BASELINK_FOREACH0(_room, _link, _it, *((_it)++), (void)0)
# define BASELINK_FOREACH0(_room, _link, _it, _assign, _postincr) \
	for (Room::Link * _link = (Room::Link*)1; _link; _link = 0) \
		for (BaseInterface::Room::links_type::iterator _it = (_room)->links.begin(); \
		    _it != (_room)->links.end() && ((_link=(_assign))||1); _postincr)
#endif

BaseInterface::Room::linksmap_type::iterator
BaseInterface::Room::AddLink(Link * lnk, bool front) {
	BASE_DBG(logvs::DBG+1, "AddLink(%s): %s new link (%p) mapSz:%zu vecSz:%zu",
			lnk ? lnk->index.c_str() : "(null)", front? "inserting" : "pushing", lnk, linksmap.size(), BASELINK_VECSZ);

	linksmap_type::iterator it = linksmap.insert(std::make_pair(lnk->index, lnk)
#ifdef BASELINK_CHAINED_MULTIMAP
						           , front);
#else
	                               );
# if !defined(BASELINK_ALPHAORDER_MULTIMAP)
	if (front) {
		links.insert(links.begin(), lnk);
	} else {
		links.push_back(lnk);
	}
# endif
#endif
	BASE_DBG(logvs::DBG+2, "AddLink(%s) %s", lnk ? lnk->index.c_str() : "(null)", it != linksmap.end() ? "ok" : "failed");
	return it;
}

BaseInterface::Room::linksmap_type::iterator
BaseInterface::Room::EraseLink(const std::string & index, bool on_ptr, Link * lnk, bool only_notvalid, bool force) {
	Link * curlink = GetCurrentLink(0, false);
	linksmap_range_type its = this->GetLinkRange(index);
	for ( ; its.first != its.second; ) {
		Link * zombie = GetLink(its.first);
		if ((on_ptr && lnk != zombie)
		||  (only_notvalid && CheckLink(zombie))) { ++its.first; continue ; }

		if ((!force && BASELINK_BUSY(linksmap)) || zombie == curlink) {
			if (CheckLink(zombie)) {
				BASE_DBG(logvs::DBG+1, "EraseLink(%s): INVALIDATING '%s' (%p) mapSz:%zu vecSz:%zu",
						index.c_str(), zombie ? zombie->index.c_str() : "(null)", zombie, linksmap.size(), BASELINK_VECSZ);
				zombie->valid = false;
			} else if (!zombie) {
				BASE_LOG(logvs::WARN, "EraseLink(%s): Trying to INVALIDATE NULL Link!", index.c_str());
			}
			++its.first;
			continue;
		}
		BASE_DBG(logvs::DBG+1, "EraseLink(%s): DELETING link '%s' (%p) mapSz:%zu vecSz:%zu",
				index.c_str(), zombie ? zombie->index.c_str() : "(null)", zombie, linksmap.size(), BASELINK_VECSZ);

		// PERFORM MULTIMAP ERASE
	   #if defined(BASELINK_CHAINED_MULTIMAP) || defined(MULTIMAP_HAVE_CXX11_ERASE) // only ChainedMultimap or CXX11 std::multimap
		its.first = linksmap.erase(its.first);
	   #else
		its.first = ChainedMultimapUtil::multimap_cxx11_erase(linksmap, its.first);
	   #endif

		if (zombie)
			delete zombie;
	}
   #if defined(BASELINK_ALPHAORDER_MULTIMAP) // erase has invalidated hotlink_it, whereever it was
	hotlink_it = (curlink == NULL ? linksmap.end() : linksmap.find(curlink->index));
   #endif
	return its.first;
}

BaseInterface::Room::Link * BaseInterface::Room::GetCurrentLink(int offset, bool only_valid, bool set) {
	if (linksmap.size() == 0) return NULL;
	size_t count = 0;
#if defined(BASELINK_CHAINED_MULTIMAP)
	linksmap_type::chained_iterator hotlink_it = this->hotlink_it;
	if (hotlink_it.end()) {
		if (offset == 0) return NULL;
		hotlink_it = offset > 0 ? linksmap.last() : linksmap.first();
	}
	if (offset < 0) {
		do {
			if ((--hotlink_it).end()) hotlink_it = linksmap.last();
		} while (++count < linksmap.size() && ((only_valid && !CheckLink(*hotlink_it)) || ++offset < 0));
	} else if (offset > 0) {
		do {
			if ((++hotlink_it).end()) hotlink_it = linksmap.first();
		} while (++count < linksmap.size() && ((only_valid && !CheckLink(*hotlink_it)) || --offset > 0));
	}
	if (set) this->hotlink_it = hotlink_it;
	return hotlink_it.end() || (only_valid && !CheckLink(*hotlink_it)) ? NULL : *hotlink_it;
#elif defined(BASELINK_ALPHAORDER_MULTIMAP)
	linksmap_type::iterator hotlink_it = this->hotlink_it;
	if (hotlink_it == linksmap.end()) {
		if (offset == 0) return NULL;
		hotlink_it = offset < 0 ? linksmap.begin() : linksmap.end() - 1;
	}
	if (offset < 0) {
		do {
			if (hotlink_it == linksmap.begin()) hotlink_it = linksmap.end(); --hotlink_it;
		} while (++count < linksmap.size() && ((only_valid && !CheckLink(*hotlink_it)) || ++offset < 0));
	} else if (offset > 0) {
		do {
			if ((++hotlink_it) == linksmap.end()) hotlink_it = linksmap.begin();
		} while (++count < linksmap.size() && ((only_valid && !CheckLink(*hotlink_it)) || --offset > 0));
	}
	if (set) this->hotlink_it = hotlink_it;
	return hotlink_it == linksmap.end() || (only_valid && !CheckLink(*hotlink_it)) ? NULL : *hotlink_it;
#else
	size_t hotlink_idx = this->hotlink_idx;
	if (hotlink_idx >= links.size()) {
		if (offset == 0) return NULL;
		hotlink_idx = offset > 0 : linksmap.size() - 1 : 0;
	}
	if (offset < 0) {
		do {
			if (hotlink_idx == 0) hotlink_idx = links.size() - 1; else (--hotlink_idx);
		} while (++count < links.size() && ((only_valid && !CheckLink(links[hotlink_idx])) || ++offset < 0));
	} else if (offset > 0) {
		do {
			if ((++hotlink_idx) == links.size()) hotlink_idx = 0;
		} while (++count < links.size() && ((only_valid && !CheckLink(links[hotlink_idx])) || --offset > 0));
	}
	if (set) this->hotlink_idx = hotlink_idx;
	return hotlink_idx >= links.size() || (only_valid && !CheckLink(links[hotlink_idx])) ? NULL : links[hotlink_idx];
#endif
}

void BaseInterface::Room::BaseObj::Draw (BaseInterface *base) {
//		Do nothing...
}

static FILTER BlurBases() {
  static bool blur_bases = XMLSupport::parse_bool(vs_config->getVariable("graphics","blur_bases","true"));
  return blur_bases?BILINEAR:NEAREST;
}
BaseInterface::Room::BaseVSSprite::BaseVSSprite (const std::string &spritefile, const std::string &ind) 
  : BaseObj(ind),spr(spritefile.c_str(),BlurBases(),GFXTRUE) {}

BaseInterface::Room::BaseVSMovie::BaseVSMovie(const std::string &moviefile, const std::string &ind)
  : BaseVSSprite(ind, VSSprite(AnimatedTexture::CreateVideoTexture(moviefile), 0, 0, 2, 2)) {}

void BaseInterface::Room::BaseVSSprite::SetSprite (const std::string &spritefile)
{
	// Destroy SPR
	spr.~VSSprite();
	// Re-create it (in case you don't know the following syntax, 
	//	which is a weird but standard syntax, 
	//	it initializes spr instead of allocating memory for it)
	// PS: I hope it doesn't break many compilers ;) 
	//	(if it does, spr will have to become a pointer)
	new(&spr)VSSprite(spritefile.c_str(),BlurBases(),GFXTRUE);
}

void BaseInterface::Room::BaseVSMovie::SetMovie(const std::string &moviefile)
{
    // Get sprite position and size so that we can preserve them
    float x,y,w,h,rot;
    spr.GetPosition(x,y);
    spr.GetSize(w,h);
    spr.GetRotation(rot);
    
    // See notes above
    spr.~VSSprite();
    new(&spr)VSSprite( AnimatedTexture::CreateVideoTexture(moviefile), x,y,w,h );
    spr.SetRotation(rot);
}

float BaseInterface::Room::BaseVSMovie::GetTime() const
{
    return spr.getTexture()->curTime();
}

void BaseInterface::Room::BaseVSMovie::SetTime(float t)
{
    spr.getTexture()->setTime(t);
}

void BaseInterface::Room::BaseVSSprite::Draw (BaseInterface *base) {
  static float AlphaTestingCutoff = XMLSupport::parse_float(vs_config->getVariable("graphics","base_alpha_test_cutoff","0"));
  GFXAlphaTest (GREATER,AlphaTestingCutoff);
  GFXBlendMode(SRCALPHA,INVSRCALPHA);
  GFXEnable(TEXTURE0);
  spr.Draw();
  GFXAlphaTest (ALWAYS,0);
}

void BaseInterface::Room::BaseShip::Draw (BaseInterface *base) {
	Unit *un=base->caller.GetUnit();
	if (un) {
		GFXHudMode (GFXFALSE);
                float tmp = g_game.fov;
                static float standard_fov=XMLSupport::parse_float(vs_config->getVariable("graphics","base_fov","90"));
                g_game.fov=standard_fov;
                float tmp1=_Universe->AccessCamera()->GetFov();
                _Universe->AccessCamera()->SetFov(standard_fov);
		Vector p,q,r;
		_Universe->AccessCamera()->GetOrientation (p,q,r);
		float co=_Universe->AccessCamera()->getCockpitOffset();
		_Universe->AccessCamera()->setCockpitOffset(0);
		_Universe->AccessCamera()->UpdateGFX();
		QVector pos =  _Universe->AccessCamera ()->GetPosition();
		Matrix cam (p.i,p.j,p.k,q.i,q.j,q.k,r.i,r.j,r.k,pos);
		Matrix final;
		Matrix newmat = mat;
		newmat.p.k*=un->rSize();
		newmat.p+=QVector(0,0,g_game.znear);
		newmat.p.i*=newmat.p.k;
		newmat.p.j*=newmat.p.k;
		MultMatrix (final,cam,newmat);
                SetupViewport();
		GFXClear(GFXFALSE);//clear the zbuf

		GFXEnable (DEPTHTEST);
		GFXEnable (DEPTHWRITE);
		GFXEnable(LIGHTING);
		int light=0;
		GFXCreateLight(light,GFXLight(true,GFXColor(1,1,1,1),GFXColor(1,1,1,1),GFXColor(1,1,1,1),GFXColor(.1,.1,.1,1),GFXColor(1,0,0),GFXColor(1,1,1,0),24),true);

		(un)->DrawNow(final,FLT_MAX);
		GFXDeleteLight(light);
		GFXDisable (DEPTHTEST);
		GFXDisable (DEPTHWRITE);
		GFXDisable(LIGHTING);
                GFXDisable(TEXTURE1);
                GFXEnable(TEXTURE0);
		_Universe->AccessCamera()->setCockpitOffset(co);
		_Universe->AccessCamera()->UpdateGFX();
                SetupViewport();                
//		_Universe->AccessCockpit()->SetView (CP_PAN);
		GFXHudMode (GFXTRUE);
                g_game.fov=tmp;
                _Universe->AccessCamera()->SetFov(tmp1);
	}
}

void BaseInterface::Room::Draw (BaseInterface *base) {
	std::string index;
	bool erase_needed = false;
	size_t i = 0;
   #if defined(BASEOBJ_CHAINED_MULTIMAP)
	for (objsmap_type::chained_iterator it = objsmap.first(); !it.end(); ++i) {
		BaseObj * obj = *it++;
   #elif defined BASEOBJ_ALPHAORDER_MULTIMAP
	for (_busyobj=true, objsmap_type::iterator it = objsmap.begin(); it != objsmap.end() || (_busyobj=/*ASSIGN*/false); ) {
		BaseObj * (it++)->second;
   #else
	for ( ; i < objs.size() ; ) {
		BaseObj * obj = objs[i++];
   #endif
		bool obj_valid = CheckObj(obj);
		if (obj_valid) {
			BASE_DBG(logvs::DBG+3, "Drawing BaseObj #%zu '%s' (%p) (mapsz:%zu,vsz:%zu)!",
				     i, obj->index.c_str(), obj, objsmap.size(), BASEOBJ_VECSZ);

			GFXBlendMode(SRCALPHA,INVSRCALPHA);
			obj->Draw(base);

		}
		if (!obj_valid || !CheckObj(obj)) {
			BASE_DBG(logvs::DBG+2, "detected INVALIDATED BaseObj #%zu '%s' (%p) (mapsz:%zu,vsz:%zu)!",
					 i, obj == NULL ? "(null)" : obj->index.c_str(), obj, objsmap.size(), BASEOBJ_VECSZ);
			// this will be faster to delete while traversing the whole tree than erasing one by one
			// and if there is only one obj, index is kept.
			if (!erase_needed) {
				erase_needed = true;
				if (obj) index = obj->index;
			} else if (!index.empty()) {
				index.clear();
			}
		   #if !defined(BASEOBJ_CHAINED_MULTIMAP) && !defined(BASEOBJ_ALPHAORDER_MULTIMAP)
			objs.erase(objs.begin()+(--i));
		   #endif
		}
	}

	if (erase_needed) {
		this->EraseObj(index, false, NULL, true, true); // delete invalid objects.
	}

	GFXBlendMode(SRCALPHA,INVSRCALPHA);
	// draw location markers
	//<!-- config options in the "graphics" section -->
	//<var name="base_enable_locationmarkers" value="true"/>
	//<var name="base_locationmarker_sprite" value="base_locationmarker.spr"/>
	//<var name="base_draw_locationtext" value="true"/>
	//<var name="base_locationmarker_textoffset_x" value="0.025"/>
	//<var name="base_locationmarker_textoffset_y" value="0.025"/>
	//<var name="base_locationmarker_drawalways" value="false"/>
	//<var name="base_locationmarker_distance" value="0.5"/>
	//<var name="base_locationmarker_textcolor_r" value="1.0"/>
	//<var name="base_locationmarker_textcolor_g" value="1.0"/>
	//<var name="base_locationmarker_textcolor_b" value="1.0"/>
	//<var name="base_drawlocationborders" value="false"/>
	static bool enable_markers = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_enable_locationmarkers","false"));
	static bool draw_text      = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_draw_locationtext","false"));
	static bool draw_always    = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_locationmarker_drawalways","false"));
	static float y_lower       = -0.9; // shows the offset on the lower edge of the screen (for the textline there) -> Should be defined globally somewhere
	static float base_text_background_alpha=XMLSupport::parse_float(vs_config->getVariable("graphics","base_text_background_alpha","0.0625"));
    static std::string marker_font = vs_config->getVariable("graphics","base_locationmarker_font", "");

    if (enable_markers) {
		float x, y, text_wid, text_hei;
		//get offset from config;
		static float text_offset_x = XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_textoffset_x","0"));
		static float text_offset_y = XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_textoffset_y","0"));
		static float text_color_r  = XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_textcolor_r","1"));
		static float text_color_g  = XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_textcolor_g","1"));
		static float text_color_b  = XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_textcolor_b","1"));
		BASELINK_FOREACH(this, link) { //loop through all links and draw a marker for each
        	if (this->CheckLink(link)) {
				if ((link->alpha < 1) || (draw_always)) {
					if (draw_always) { link->alpha = 1; }  // set all alphas to visible
					x = (link->x + (link->wid / 2));   // get the center of the location
					y = (link->y + (link->hei / 2));   // get the center of the location

					/* draw marker */
					static string spritefile_marker = vs_config->getVariable("graphics","base_locationmarker_sprite","");
					if (spritefile_marker.length()) {
						static VSSprite *spr_marker = new VSSprite(spritefile_marker.c_str());
						float wid,hei;
						spr_marker->GetSize(wid,hei);
						// check if the sprite is near a screenedge and correct its position if necessary
						if ((x + (wid / 2)) >=  1     ) {x = ( 1      - (wid / 2));}
						if ((y + (hei / 2)) >=  1     ) {y = ( 1      - (hei / 2));}
						if ((x - (wid / 2)) <= -1     ) {x = (-1      + (wid / 2));}
						if ((y - (hei / 2)) <= y_lower) {y = (y_lower + (hei / 2));}
						spr_marker->SetPosition(x, y);
						GFXDisable(TEXTURE1);
						GFXEnable(TEXTURE0);
						GFXColor4f(1,1,1,link->alpha);
						spr_marker->Draw();
					} // if spritefile

					if (draw_text) {
						GFXDisable(TEXTURE0);
						TextPlane text_marker;
                        if (!marker_font.empty())
                            text_marker.SetFont(marker_font);
						text_marker.SetText(link->text);
						text_marker.GetCharSize(text_wid, text_hei);     // get average charactersize
						float text_pos_x = x + text_offset_x;            // align right ...
						float text_pos_y = y + text_offset_y + text_hei; // ...and on top
						text_wid = text_wid * link->text.length() * 0.25;     // calc ~width of text (=multiply the average characterwidth with the number of characters)
						if ((text_pos_x + text_offset_x + text_wid) >= 1) {       // check right screenborder
							text_pos_x = (x - fabs(text_offset_x) - text_wid);     // align left
						}
						if ((text_pos_y + text_offset_y) >= 1) {                  // check upper screenborder
							text_pos_y = (y - fabs(text_offset_y));                // align on bottom
						}
						if ((text_pos_y + text_offset_y - text_hei) <= y_lower) { // check lower screenborder
							text_pos_y = (y + fabs(text_offset_y) + text_hei);     // align on top
						}
						text_marker.col = GFXColor(text_color_r, text_color_g, text_color_b, link->alpha);
						text_marker.SetPos(text_pos_x, text_pos_y);
						if(link->pythonfile != "#" && text_marker.GetText().find("XXX")!=0){
							GFXColor tmpbg=text_marker.bgcol;
							bool automatte=(0==tmpbg.a);
							if(automatte){text_marker.bgcol=GFXColor(0,0,0,base_text_background_alpha);}
							text_marker.Draw(text_marker.GetText(),0,true,false,automatte);
							text_marker.bgcol=tmpbg;
						}
						GFXEnable(TEXTURE0);
					} // if draw_text
				}
			} // if link
		}  // for i
	} // enable_markers

	static bool draw_borders  = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_drawlocationborders","false"));
	static bool debug_markers = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_enable_debugmarkers","false"));
	if (draw_borders || debug_markers) {
		float x, y, text_wid, text_hei;
		//get offset from config;
		static float text_offset_x = XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_textoffset_x","0"));
		static float text_offset_y = XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_textoffset_y","0"));
		BASELINK_FOREACH(this, link) {
			if (CheckLink(link)) {
				// Debug marker
				if (debug_markers) {
					//compute label position
					x = (link->x + (link->wid / 2));   // get the center of the location
					y = (link->y + (link->hei / 2));   // get the center of the location
					TextPlane text_marker;
                    if (!marker_font.empty())
                        text_marker.SetFont(marker_font);
					text_marker.SetText(link->index);
					text_marker.GetCharSize(text_wid, text_hei);     // get average charactersize
					float text_pos_x = x + text_offset_x;            // align right ...
					float text_pos_y = y + text_offset_y + text_hei; // ...and on top
					text_wid = text_wid * link->text.length() * 0.25;     // calc ~width of text (=multiply the average characterwidth with the number of characters)
					if ((text_pos_x + text_offset_x + text_wid) >= 1)         // check right screenborder
						text_pos_x = (x - fabs(text_offset_x) - text_wid);     // align left
					if ((text_pos_y + text_offset_y) >= 1)                    // check upper screenborder
						text_pos_y = (y - fabs(text_offset_y));                // align on bottom
					if ((text_pos_y + text_offset_y - text_hei) <= y_lower)   // check lower screenborder
						text_pos_y = (y + fabs(text_offset_y) + text_hei);     // align on top
					if (enable_markers)
						text_pos_y += text_hei;
					
					text_marker.col = GFXColor(1, 1, 1, 1);
					text_marker.SetPos(text_pos_x, text_pos_y);
					
					GFXDisable(TEXTURE0);
					GFXColor tmpbg=text_marker.bgcol;
					bool automatte=(0==tmpbg.a);
					if(automatte){text_marker.bgcol=GFXColor(0,0,0,base_text_background_alpha);}
					text_marker.Draw(text_marker.GetText(),0,true,false,automatte);
					text_marker.bgcol=tmpbg;
					GFXEnable(TEXTURE0);
				}
				// link border
				GFXColor4f(1,1,1,1);
				Vector c1(link->x,link->y,0);
				Vector c3(link->wid+c1.i,link->hei+c1.j,0);
				Vector c2(c1.i,c3.j,0);
				Vector c4(c3.i,c1.j,0);
				GFXDisable(TEXTURE0);
				GFXBegin(GFXLINESTRIP);
				GFXVertexf(c1);
				GFXVertexf(c2);
				GFXVertexf(c3);
				GFXVertexf(c4);
				GFXVertexf(c1);
				GFXEnd();
				GFXEnable(TEXTURE0);
			} // if link
		} // for i
	} // if draw_borders
}
static std::vector<BaseInterface::Room::BaseTalk *> active_talks;

BaseInterface::Room::BaseTalk::BaseTalk (const std::string & msg,const std::string & ind, bool only_one) :BaseObj(ind), curchar (0), curtime (0), message(msg) {
	if (only_one) {
		active_talks.clear();
	}
	active_talks.push_back(this);
}


void BaseInterface::Room::BaseText::Draw (BaseInterface *base) {
  int tmpx=g_game.x_resolution;
  int tmpy=g_game.y_resolution;
  static int base_max_width=XMLSupport::parse_int(vs_config->getVariable("graphics","base_max_width","0"));
  static int base_max_height=XMLSupport::parse_int(vs_config->getVariable("graphics","base_max_height","0"));
  if (base_max_width&&base_max_height) {
    if (base_max_width<tmpx)
      g_game.x_resolution=base_max_width;
    if (base_max_height<tmpy)
      g_game.y_resolution=base_max_height;
  }
  
  static float base_text_background_alpha=XMLSupport::parse_float(vs_config->getVariable("graphics","base_text_background_alpha","0.0625"));
  GFXColor tmpbg=text.bgcol;
  bool automatte=(0==tmpbg.a);
  if(automatte){
	text.bgcol=GFXColor(0,0,0,base_text_background_alpha);
  }
  if (!automatte && text.GetText().empty()) {
    float posx,posy,wid,hei;
    text.GetPos(posy,posx);
    text.GetSize(wid,hei);

    GFXColorf(text.bgcol);
    GFXBegin(GFXQUAD);
    GFXVertex3f(posx,hei,0.0f);
    GFXVertex3f(wid,hei,0.0f);
    GFXVertex3f(wid,posy,0.0f);
    GFXVertex3f(posx,posy,0.0f);
    GFXEnd();
  } else {
    text.Draw(text.GetText(),0,true,false,automatte);
  }
  text.bgcol=tmpbg;
  g_game.x_resolution=tmpx;
  g_game.y_resolution=tmpy;
}

void RunPython(const char *filnam) {
#ifdef DEBUG_RUN_PYTHON
	printf("Run python:\n%s\n", filnam);
#endif
	if (filnam[0]) {
		if (filnam[0]=='#' && filnam[1]!='\0') {
            if (BASE_DBG(logvs::DBG+1, "Running python string '%s'", filnam) <= 0) {
                BASE_DBG(logvs::DBG, "Running python string '%s'...", Python::prettyPythonScript(filnam).c_str());
            }
			::Python::reseterrors();
			PyRun_SimpleString(const_cast<char*>(filnam));
			::Python::reseterrors();
		}else {
			/*FILE *fp=VSFileSystem::vs_open(filnam,"r");
			if (fp) {
				int length=strlen(filnam);
				char *newfile=new char[length+1];
				strncpy(newfile,filnam,length);
				newfile[length]='\0';
				::Python::reseterrors();
				PyRun_SimpleFile(fp,newfile);
				::Python::reseterrors();
				fclose(fp);
				processDelayedMissions();
			} else {
				fprintf(stderr,"Warning:python link file '%s' not found\n",filnam);
			}*/
            BASE_DBG(logvs::DBG, "Running python script %s...", filnam);
            CompileRunPython(filnam);
		}
	}
}

void BaseInterface::Room::BasePython::Draw (BaseInterface *base) {
	timeleft+=GetElapsedTime()/getTimeCompression();
	if (timeleft>=maxtime) {
        const char * script = this->pythonfile.c_str();
		timeleft=0;
        if (BASE_LOG(logvs::NOTICE, "Room display: Running python script%s...",
                     *script == '#' ? " (string)" : script) > 0) {
            if (*script == '#')
                BASE_LOG(logvs::VERBOSE, "script: '%s'", script);
        }
		RunPython(script);
		return; //do not do ANYTHING with 'this' after the previous statement...
	}
}

void BaseInterface::Room::BasePython::Relink(const std::string &python)
{
	pythonfile = python;
}

void BaseInterface::Room::BaseTalk::Draw (BaseInterface *base) {
/*	GFXColor4f(1,1,1,1);
	GFXBegin(GFXLINESTRIP);
		GFXVertex3f(caller->x,caller->y,0);
		GFXVertex3f(caller->x+caller->wid,caller->y,0);
		GFXVertex3f(caller->x+caller->wid,caller->y+caller->hei,0);
		GFXVertex3f(caller->x,caller->y+caller->hei,0);
		GFXVertex3f(caller->x,caller->y,0);
	GFXEnd();*/

	// FIXME: should be called from draw()
	if (hastalked) return;
	curtime+=GetElapsedTime()/getTimeCompression();
	static float delay=XMLSupport::parse_float(vs_config->getVariable("graphics","text_delay",".05"));

	std::vector<BaseTalk *>::iterator ind2=std::find(active_talks.begin(), active_talks.end(), this);
	if (ind2==active_talks.end()
	||  (curchar>=message.size()&&curtime>((delay*message.size())+2))) {
		curtime=0;
		this->valid = false; // will be deleted

		if (ind2 != active_talks.end()) {
			active_talks.erase(ind2);
		}
		base->othtext.SetText("");
		return ;
	}
	if (curchar<message.size()) {
		static float inbetween=XMLSupport::parse_float(vs_config->getVariable("graphics","text_speed",".025"));
		if (curtime>inbetween) {
			base->othtext.SetText(message.substr(0,++curchar));
			curtime=0;
		}
	}
	hastalked=true;
}

BaseInterface::Room::Link * BaseInterface::Room::LinkAtPosition(float x, float y) {
	BASELINK_FOREACH(this, link) {
		if (this->CheckLink(link)
				&& x >= link->x
				&& x <= (link->x + link->wid)
				&& y >= link->y
				&& y <= (link->y + link->hei)) {
	   	   #if defined(BASELINK_CHAINED_MULTIMAP) || defined(BASELINK_ALPHAORDER_MULTIMAP)
			hotlink_it = _it;
		   #else
			hotlink_idx = _it - objs.begin()
	   	   #endif
			return link;
		}
	}
   #if defined(BASELINK_CHAINED_MULTIMAP)
	if (!(hotlink_it.end())) hotlink_it = linksmap.none();
   #elif defined(BASELINK_ALPHAORDER_MULTIMAP)
	hotlink_it = linksmap.end();
   #else
	hotlink_idx = (size_t)-1;
   #endif
	return NULL;
}

BaseInterface::Room::Link * BaseInterface::Room::MouseOver (BaseInterface *base,float x, float y) {
	Room::Link * hotlink = this->GetCurrentLink(0, false);
	Link * link = this->LinkAtPosition(x, y);
	BASELINK_FOREACH(this, _dummy) { // this will not loop but will forbid python to delete objects (only invalidate)
		if (hotlink && hotlink != link)
			hotlink->MouseLeave(base,x,y,getMouseButtonMask());
		if (link && link != hotlink)
			link->MouseEnter(base,x,y,getMouseButtonMask());
		if (link)
			link->MouseMove(base,x,y,getMouseButtonMask());
		break ;
	}
	if (link && !(this->CheckLink(link))) {
   	   #if !defined(BASELINK_CHAINED_MULTIMAP) && !defined(BASELINK_ALPHAORDER_MULTIMAP)
		for (size_t i = 0; i < objs.size(); /*no_incr*/ ) {
			if (this->CheckLink(objs[i])) ++i; else objs.erase(objs.begin()+i);
		}
   	   #endif
		this->EraseLink("", false, NULL, true, true);
		link = this->LinkAtPosition(x, y);
	}
	return link;
}

namespace BaseKeys {
	enum BASE_LINK_CMD_ENUM { BASE_LINK_NONE = -2, BASE_PREV_LINK, BASE_ENTER_LINK, BASE_NEXT_LINK };
	static void handleLink(BASE_LINK_CMD_ENUM what, const KBData&, KBSTATE newState) {
		if (newState != PRESS)
			return ;
		BaseInterface * base = BaseInterface::CurrentBase;
		if (base == NULL || base->curroom < 0 || base->curroom > base->rooms.size())
			return ;
		int curroom = base->curroom;
		BaseInterface::Room * room = base->rooms[curroom];
		int button;
		float x, y;
		if (what == BASE_ENTER_LINK) 	button = WS_LEFT_BUTTON;
		else if (what == BASE_PREV_LINK)button = WS_WHEEL_UP;
		else 							button = WS_WHEEL_DOWN;
		BaseInterface::Room::Link * link = room->GetCurrentLink(0, false);
		if (link != NULL) {
			x = link->x + link->wid / 2.;
			y = link->y + link->hei / 2.;
		} else {
			int mx, my;
			GetMouseXY(mx, my);
			CalculateRealXAndY(mx,my,&x,&y);
		}
		if (base && base == BaseInterface::CurrentBase && curroom == base->curroom)
			room->Click(base, x, y, button, WS_MOUSE_DOWN);
		if (base && base == BaseInterface::CurrentBase && curroom == base->curroom)
			room->Click(base, x, y, button, WS_MOUSE_UP);
	}
	void NextLink(const KBData & data, KBSTATE newState) {
		handleLink(BASE_NEXT_LINK, data, newState);
	}
 	void PrevLink(const KBData & data, KBSTATE newState) {
 		handleLink(BASE_PREV_LINK, data, newState);
 	}
 	void EnterLink(const KBData & data, KBSTATE newState) {
 		handleLink(BASE_ENTER_LINK, data, newState);
 	}
 	void Computer(const KBData & data, KBSTATE newState) {
 		if (newState != PRESS || BaseInterface::CurrentBase == NULL)
 			return ;
 		BaseInterface::Room::Comp comp("BaseKeys::Computer", "");
 		for (unsigned int i = 0; i < BaseComputer::LOADSAVE/* DISPLAY_MODE_COUNT*/; ++i) {
 			comp.modes.push_back((enum BaseComputer::DisplayMode)i);
 		}
 		comp.Click(BaseInterface::CurrentBase, -2.0, -2.0, WS_LEFT_BUTTON, WS_MOUSE_DOWN);
 		comp.Click(BaseInterface::CurrentBase, -2.0, -2.0, WS_LEFT_BUTTON, WS_MOUSE_UP);
 	}
}

// This is a hack to emulate link navigation with joystick.
// We could use the KeyBindings mechanism provided by in_sdl/in_joystick
// (assuming we can differenciate game bindings and base bindings).
void BaseInterface::Joystick(unsigned int which, float x, float y, float z, unsigned int buttons, unsigned int state) {
	static const bool joy_link_nav_emul = XMLSupport::parse_bool(vs_config->getVariable("joystick", "base_link_nav", "true"));
	static bool init_done = false;
	static enum BaseKeys::BASE_LINK_CMD_ENUM prev_cmd[MAX_JOYSTICKS];
	BaseInterface * base = CurrentBase;
	if (!joy_link_nav_emul || base == NULL || !(base->python_kbhandler.empty()) || which >= GetNumJoysticks())
		return ;
	if (!init_done) {
		for (size_t i = 0; i < sizeof(prev_cmd)/sizeof(*prev_cmd); ++i) prev_cmd[i] = BaseKeys::BASE_LINK_NONE;
		init_done = true;
	}
	enum BaseKeys::BASE_LINK_CMD_ENUM what = BaseKeys::BASE_LINK_NONE;
	if (buttons != 0) {
		what = BaseKeys::BASE_ENTER_LINK;
	} else if (fabs(x) >= 0.8 && x*x > y*y) {
		what = x>0 ? BaseKeys::BASE_NEXT_LINK : BaseKeys::BASE_PREV_LINK;
	} else if (fabs(y) >= 0.8 && y*y > x*x) {
		what = y>0 ? BaseKeys::BASE_NEXT_LINK : BaseKeys::BASE_PREV_LINK;
	}
	if (what != prev_cmd[which]) {
		prev_cmd[which] = what;
		if (what != BaseKeys::BASE_LINK_NONE) {
			BASE_LOG(logvs::VERBOSE, "joystick: #%d x:%g y:%g z:%g buttons:%x", which, x, y, z, buttons);
			BaseKeys::handleLink(what, KBData(), PRESS);
		}
	}
}

BaseInterface *BaseInterface::CurrentBase=NULL;
static BaseInterface *lastBaseDoNotDereference=NULL;

void * BaseInterface::CurrentFont = NULL;

extern bool QuitAllow;

bool RefreshGUI(void) {
	bool retval=false;
	if (_Universe->AccessCockpit()) {
		if (BaseInterface::CurrentBase) {
			if (_Universe->AccessCockpit()->GetParent()==BaseInterface::CurrentBase->caller.GetUnit()){
				if (BaseInterface::CurrentBase->CallComp) {
#ifdef NEW_GUI
					globalWindowManager().draw();
					return true;
#else
					return RefreshInterface ();
#endif
				} else if (QuitAllow) {
					static VSSprite QuitSprite("quit.sprite",BILINEAR,GFXTRUE);
					static VSSprite QuitCompatSprite("quit.spr",BILINEAR,GFXTRUE);

					GFXColor(0,0,0,0);
					glViewport(0,0,g_game.x_resolution, g_game.y_resolution);
					StartGUIFrame(GFXTRUE);
					//GFXEnable(TEXTURE0);
					if (QuitSprite.LoadSuccess()) {
						QuitSprite.Draw();
					} else {
						QuitCompatSprite.Draw();
					}
					EndGUIFrame(GFXFALSE);
				} else {
					BaseInterface::CurrentBase->Draw();
				}
				retval=true;
			}
		}
	}
	return retval;
}

void base_main_loop() {
	UpdateTime();
    Music::MuzakCycle();
	if( Network!=NULL)
	{
		for( int jj=0; jj<_Universe->numPlayers(); jj++)
		{
			Network[jj].checkMsg( NULL);
		}
	}
	GFXBeginScene();
	if (createdbase) {

          //		static int i=0;
          //		if (i++%4==3) {
                  createdbase=false;
                  //		}
		AUDStopAllSounds(createdmusic);
	}        
	if (!RefreshGUI()) {
		restore_main_loop();
	}else {
		GFXEndScene();
		micro_sleep(1000);
	}
	BaseComputer::dirty = false;
}

void BaseInterface::Room::Key(BaseInterface* base, unsigned int ch, unsigned int mod, bool release, float x, float y) {
	static const bool kb_link_nav_emul = XMLSupport::parse_bool(vs_config->getVariable("keyboard", "base_link_nav", "true"));
	BASE_DBG(logvs::DBG, "Room::Key() %x mod:%x release:%d", ch, mod, release);
	kbGetInput(ch, mod, release, x, y);
}

void BaseInterface::Room::Click (BaseInterface* base,float x, float y, int button, int state) {
	BASE_DBG(logvs::DBG+1, "Room::Click() button:%d state:%d", button, state);
	if (button==WS_LEFT_BUTTON) {
		Link * link = LinkAtPosition(x,y);
		if (CheckLink(link)) {
			BASELINK_FOREACH(this, _dummy) { // Prevents python to delete objects (only invalidate)
				link->Click(base,x,y,button,state);
				// this and base could be deleted at this point.
				break ;
			}
		}
	} else {
#ifdef BASE_MAKER
		if (state==WS_MOUSE_UP) {
			char input [201];
			char *str;
			if (button==WS_RIGHT_BUTTON)
				str="Please create a file named stdin.txt and type\nin the sprite file that you wish to use.";
			else if (button==WS_MIDDLE_BUTTON)
				str="Please create a file named stdin.txt and type\nin the type of room followed by arguments for the room followed by text in quotations:\n1 ROOM# \"TEXT\"\n2 \"TEXT\"\n3 vector<MODES>.size vector<MODES> \"TEXT\"";
			else
				return;
#ifdef _WIN32
			int ret=MessageBox(NULL,str,"Input",MB_OKCANCEL);
#else
			printf("\n%s\n",str);
			int ret=1;
#endif
			int index;
			int rmtyp;
			if (ret==1) {
				if (button==WS_RIGHT_BUTTON) {
#ifdef _WIN32
					FILE *fp=VSFileSystem::vs_open("stdin.txt","rt");
#else
					FILE *fp=stdin;
#endif
					VSFileSystem::vs_fscanf(fp,"%200s",input);
#ifdef _WIN32
					VSFileSystem::vs_close(fp);
#endif
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==0) {
					int i;
#ifdef _WIN32
					FILE *fp=VSFileSystem::vs_open("stdin.txt","rt");
#else
					FILE *fp=stdin;
#endif
	 				VSFileSystem::vs_fscanf(fp,"%d",&rmtyp);
					switch(rmtyp) {
					case 1:
						links.push_back(new Goto("linkind","link"));
						VSFileSystem::vs_fscanf(fp,"%d",&((Goto*)links.back())->index);
						break;
					case 2:
						links.push_back(new Launch("launchind","launch"));
						break;
					case 3:
						links.push_back(new Comp("compind","comp"));
						VSFileSystem::vs_fscanf(fp,"%d",&index);
						for (i=0;i<index&&(!VSFileSystem::vs_feof(fp));i++) {
							VSFileSystem::vs_fscanf(fp,"%d",&ret);
							((Comp*)links.back())->modes.push_back((BaseComputer::DisplayMode)ret);
						}
						break;
					default:
#ifdef _WIN32
						VSFileSystem::vs_close(fp);
						MessageBox(NULL,"warning: invalid basemaker option","Error",MB_OK);
#endif
						printf("warning: invalid basemaker option: %d",rmtyp);
						return;
					}
					VSFileSystem::vs_fscanf(fp,"%200s",input);
					input[200]=input[199]='\0';
					links.back()->text=string(input);
#ifdef _WIN32
					VSFileSystem::vs_close(fp);
#endif
				}
				if (button==WS_RIGHT_BUTTON) {
					input[200]=input[199]='\0';
					objs.push_back(new BaseVSSprite(input,"tex"));
					((BaseVSSprite*)objs.back())->texfile=string(input);
					((BaseVSSprite*)objs.back())->spr.SetPosition(x,y);
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==0) {
					links.back()->x=x;
					links.back()->y=y;
					links.back()->wid=0;
					links.back()->hei=0;
					makingstate=1;
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==1) {
					links.back()->wid=x-links.back()->x;
					if (links.back()->wid<0)
						links.back()->wid=-links.back()->wid;
					links.back()->hei=y-links.back()->y;
					if (links.back()->hei<0)
						links.back()->hei=-links.back()->hei;
					makingstate=0;
				}
			}
		}
#else // ! #ifdef BASE_MAKER
		if (state==WS_MOUSE_UP && linksmap.size()) {
			int offset = button == WS_WHEEL_UP ? -1 : +1;
			Link *curlink = this->GetCurrentLink(offset);
			if (CheckLink(curlink)) {
				int x=int((((curlink->x+(curlink->wid/2))+1)/2)*g_game.x_resolution);
				int y=-int((((curlink->y+(curlink->hei/2))-1)/2)*g_game.y_resolution);
				biModifyMouseSensitivity(x,y,true);
				winsys_warp_pointer(x,y);
				PassiveMouseOverWin(x,y);
			}
		}
#endif
	}
}

void BaseInterface::MouseOver (int xbeforecalc, int ybeforecalc) {
	float x, y;
	CalculateRealXAndY(xbeforecalc,ybeforecalc,&x,&y);
	Room::Link *link=rooms[curroom]->MouseOver(this,x,y);

	static float overcolor[4]={1,.666666667,0,1};
	static bool donecolor1=(vs_config->getColor("default","base_mouse_over",overcolor,true),true);
	static float inactivecolor[4]={0,1,0,1};
	static bool donecolor2=(vs_config->getColor("default","base_mouse_passive",inactivecolor,true),true);
	if (link) {
          curtext.SetText(link->text);
	} else {
          curtext.SetText(rooms[curroom]->deftext);
	}
	if (link && link->pythonfile!="#") {
          curtext.col=GFXColor(overcolor[0],overcolor[1],overcolor[2],overcolor[3]);
          drawlinkcursor=true;
	} else {
          curtext.col=GFXColor(inactivecolor[0],inactivecolor[1],inactivecolor[2],inactivecolor[3]);
          drawlinkcursor=false;
	}
	static const bool draw_borders      = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_drawlocationborders","false"));
	static const bool debug_markers     = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_enable_debugmarkers","false"));
	static const bool enable_markers    = XMLSupport::parse_bool(vs_config->getVariable("graphics","base_enable_locationmarkers","false"));
    static const bool draw_always     	= XMLSupport::parse_bool(vs_config->getVariable("graphics","base_locationmarker_drawalways","false"));
    static const bool attenuate_markers = !draw_always && (draw_borders || debug_markers || enable_markers);
    if (attenuate_markers) {
    	static const float defined_distance = fabs(XMLSupport::parse_float(vs_config->getVariable("graphics","base_locationmarker_distance","0.5")));
    	float cx, cy, wid, hei;
    	float dist_cur2link;
    	for (Room::linksmap_range_type its = rooms[curroom]->GetLinkRange(""); its.first != its.second; ++its.first) {
    		Room::Link * link = rooms[curroom]->GetLink(its.first);
    		if (!rooms[curroom]->CheckLink(link)) continue ;
    		cx = (link->x + (link->wid / 2));   //get the center of the location
    		cy = (link->y + (link->hei / 2));   //get the center of the location
    		dist_cur2link = sqrt(pow((cx-x),2) + pow((cy-y),2));
    		if ( dist_cur2link < defined_distance ) {
    			link->alpha = (1 - (dist_cur2link / defined_distance));
    		}
    		else {
    			link->alpha = 1;
    		} //if
    	} // for i
    } // if !draw_always
}

void BaseInterface::Click (int xint, int yint, int button, int state) {
	float x,y;
	CalculateRealXAndY(xint,yint,&x,&y);
	rooms[curroom]->Click(this,x,y,button,state);
	//this could be deleted at this point
}

void BaseInterface::ClickWin (int button, int state, int x, int y) {
    ModifyMouseSensitivity(x,y);
	if (state == WS_MOUSE_DOWN)
		getMouseButtonMask() |=  (1<<(button-1)); else if (state == WS_MOUSE_UP)
		getMouseButtonMask() &= ~(1<<(button-1));
	if (CurrentBase) {
		if (CurrentBase->CallComp) {
#ifdef NEW_GUI
			EventManager ::
#else
			UpgradingInfo::
#endif
			               ProcessMouseClick(button,state,x,y);
		} else {
			CurrentBase->Click(x,y,button,state);
		}
	}else {
		NavigationSystem::mouseClick(button,state,x,y);	  
	}
}


void BaseInterface::PassiveMouseOverWin (int x, int y) {
        ModifyMouseSensitivity(x,y);
	SetSoftwareMousePosition(x,y);
	if (CurrentBase) {
		if (CurrentBase->CallComp) {
#ifdef NEW_GUI
			EventManager ::
#else
			UpgradingInfo::
#endif
			               ProcessMousePassive(x,y);
	 	} else {
			CurrentBase->MouseOver(x,y);
		}
	}else {
		NavigationSystem::mouseMotion(x,y);
	}
}

void BaseInterface::ActiveMouseOverWin (int x, int y) {
        ModifyMouseSensitivity(x,y);
	SetSoftwareMousePosition(x,y);
	if (CurrentBase) {
		if (CurrentBase->CallComp) {
#ifdef NEW_GUI
			EventManager ::
#else
			UpgradingInfo::
#endif
			               ProcessMouseActive(x,y);
		} else {
			CurrentBase->MouseOver(x,y);
		}
	}else {
		NavigationSystem::mouseDrag(x,y);
	}
}

int shiftup(int);
int shiftdown(int);

void BaseInterface::Key(unsigned int ch, unsigned int mod, bool release, int x, int y)
{
	if (!python_kbhandler.empty()) {
		const std::string *evtype;
		if (release) {
			static const std::string release_evtype("keyup");
			evtype = &release_evtype;
		} else {
			static const std::string press_evtype("keydown");
			evtype = &press_evtype;
		}
		BaseUtil::SetKeyEventData(*evtype,ch);
		RunPython(python_kbhandler.c_str());
	}
	else {
		float fx, fy;
		unsigned int shiftdown_ch = shiftdown(ch);
		CalculateRealXAndY(x, y, &fx, &fy);
		if ((mod & KB_MOD_SHIFT) != 0 && kbHasBinding(shiftdown_ch, mod)) {
			ch = shiftdown_ch;
		} else {
			mod &= ~KB_MOD_SHIFT;
		}
		rooms[curroom]->Key(this, ch, mod, release, fx, fy);
	}
}

void BaseInterface::GotoLink (int linknum) {
	othtext.SetText("");
	if (rooms.size()>linknum&&linknum>=0) {
		BASE_LOG(logvs::INFO, "Setting current room: %d", linknum);
		rooms[linknum]->Enter();
		curtext.SetText(rooms[linknum]->deftext);
		curroom=linknum;
		drawlinkcursor=false;
	} else {
#ifndef BASE_MAKER
		BASE_LOG(logvs::ERROR, "WARNING: base room #%d tried to go to an invalid index: #%d",curroom,linknum);
		assert(0);
#else
		while(rooms.size()<=linknum) {
			rooms.push_back(new Room());
			char roomnum [50];
			sprintf(roomnum,"Room #%d",linknum);
			rooms.back()->deftext=roomnum;
		}
		GotoLink(linknum);
#endif
	}
}

BaseInterface::~BaseInterface () {
#ifdef BASE_MAKER
	FILE *fp=VSFileSystem::vs_open("bases/NEW_BASE"BASE_EXTENSION,"wt");
	if (fp) {
		EndXML(fp);
		VSFileSystem::vs_close(fp);
	}
#endif
	size_t links = 0, objs = 0;
	CurrentBase=NULL;
	restore_main_loop();
	for (size_t i=0;i<rooms.size();i++) {
		objs += rooms[i]->objs_size();
		links +=  rooms[i]->links_size();
		delete rooms[i];
	}
	BASE_LOG(logvs::NOTICE, "ByeBye Base - %zu rooms, %zu objs, %zu links", rooms.size(), objs, links);
}
void base_main_loop();
static void base_keyboard_cb( unsigned int  ch,unsigned int mod, bool release, int x, int y ) {
	// Set modifiers
	unsigned int amods = 0;
	amods |= (mod&(WSK_MOD_LSHIFT|WSK_MOD_RSHIFT)) ? KB_MOD_SHIFT : 0;
	amods |= (mod&(WSK_MOD_LCTRL |WSK_MOD_RCTRL )) ? KB_MOD_CTRL  : 0;
	amods |= (mod&(WSK_MOD_LALT  |WSK_MOD_RALT  )) ? KB_MOD_ALT   : 0;
	setActiveModifiers(amods);
	unsigned int shiftedch = ((amods & KB_MOD_SHIFT) != 0) ?shiftup(ch):ch;
	if (BaseInterface::CurrentBase && !BaseInterface::CurrentBase->CallComp) {
		// Flush buffer
		if (base_keyboard_queue.size())
			BaseInterface::ProcessKeyboardBuffer();
		// Send directly to base interface handlers
		BaseInterface::CurrentBase->Key(shiftedch,amods,release,x,y);
	} else {
		// Queue keystroke
		if (!release)
			base_keyboard_queue.push_back (shiftedch);
	}
}
void BaseInterface::InitCallbacks () {
	BASE_DBG(logvs::DBG, "%s()", __func__);
	RestoreKB();
	base_keyboard_queue.clear();
	winsys_set_keyboard_func(base_keyboard_cb);	
	winsys_set_mouse_func(ClickWin);
	winsys_set_joystick_func(Joystick);
	winsys_set_motion_func(ActiveMouseOverWin);
	winsys_set_passive_motion_func(PassiveMouseOverWin);
	CurrentBase=this;
//	UpgradeCompInterface(caller,baseun);
	CallComp=false;
	//if (CommandInterpretor) CommandInterpretor->enable(false);
	if (!(game_options.simulate_while_at_base||_Universe->numPlayers()>1)) {
		GFXLoop(base_main_loop);
	}
}

BaseInterface::Room::Talk::Talk (const std::string & ind,const std::string & pythonfile)
		: BaseInterface::Room::Link(ind,pythonfile) {
	curroom = -1;
#ifndef BASE_MAKER
	gameMessage last;
	int i=0;
	vector <std::string> who;
	string newmsg;
	string newsound;
	who.push_back ("bar");
	while (( mission->msgcenter->last(i++,last,who))) {
		newmsg=last.message;
		newsound="";
		string::size_type first=newmsg.find_first_of("[");
		{
			string::size_type last=newmsg.find_first_of("]");
			if (first!=string::npos&&(first+1)<newmsg.size()) {
				newsound=newmsg.substr(first+1,last-first-1);
				newmsg=newmsg.substr(0,first);
			}
		}
		this->say.push_back(newmsg);
		this->soundfiles.push_back(newsound);
	}
#endif
}
double compute_light_dot (Unit * base,Unit *un) {
  StarSystem * ss =base->getStarSystem ();
  double ret=-1;
  Unit * st;
  Unit * base_owner=NULL;
  if (ss) {
    _Universe->pushActiveStarSystem (ss);
    un_iter ui = ss->getUnitList().createIterator();
    for (;(st = *ui);++ui) {
      if (st->isPlanet()) {
	if (((Planet *)st)->hasLights()) {
	  QVector v1 = (un->Position()-base->Position()).Normalize();
	  QVector v2 = (st->Position()-base->Position()).Normalize();
	  double dot = v1.Dot(v2);
	  if (dot>ret) {
	    BASE_LOG(logvs::NOTICE, "light dot %lf", dot);
	    ret=dot;
	  }
	} else {
	  un_iter ui = ((Planet *)st)->satellites.createIterator();
	  Unit * ownz=NULL;
	  for (;(ownz=*ui);++ui) {
	    if (ownz==base) {
	      base_owner = st;
	    }
	  }
	}
      }
    }
    _Universe->popActiveStarSystem();
  }else return 1;
  if (base_owner==NULL||base->isUnit()==PLANETPTR) {
    return ret;
  }else {
    return compute_light_dot(base_owner,un);
  }
}

const char * compute_time_of_day (Unit * base,Unit *un) {
  if (!base || !un)
    return "day";
  float rez= compute_light_dot (base,un);
  if (rez>.2) 
    return "day";
  if (rez <-.1)
    return "night";
  return "sunset";
}

extern void ExecuteDirector();

BaseInterface::BaseInterface (const char *basefile, Unit *base, Unit*un)
		: curtext(getConfigColor("Base_Text_Color_Foreground",GFXColor(0,1,0,1)),getConfigColor("Base_Text_Color_Background",GFXColor(0,0,0,1))) , othtext(getConfigColor("Fixer_Text_Color_Foreground",GFXColor(1,1,.5,1)),getConfigColor("FixerTextColor_Background",GFXColor(0,0,0,1))) {
    
    static const std::string speechfont = vs_config->getVariable("graphics", "base_speech_font", "");
    static const std::string linkfont = vs_config->getVariable("graphics", "base_link_font", "");
	CurrentBase=this;
	CallComp=false;
        createdbase=true;
        createdmusic=-1;
	caller=un;
        curroom=0;
	this->baseun=base;
	float x,y;
    if (!linkfont.empty()) {
        curtext.SetFont(linkfont);
    }
    if (!speechfont.empty()) {
        othtext.SetFont(speechfont);
    }
	curtext.GetCharSize(x,y);
	curtext.SetCharSize(x*2,y*2);
	//	curtext.SetSize(2-(x*4 ),-2);
	curtext.SetSize(1-.01,-2);
	othtext.GetCharSize(x,y);
	othtext.SetCharSize(x*2,y*2);
	//	othtext.SetSize(2-(x*4),-.75);
	othtext.SetSize(1-.01,-.75);

        std::string fac = base ? FactionUtil::GetFaction(base->faction) : "neutral";
        if (base && fac=="neutral")
          fac  = UniverseUtil::GetGalaxyFaction(UnitUtil::getUnitSystemFile(base));
        //AUDStopAllSounds();
	Load(basefile, compute_time_of_day(base,un),fac.c_str());
        createdmusic=AUDHighestSoundPlaying();
	if (base && un) {
		vector <string> vec;
		vec.push_back(base->name);
		int cpt=UnitUtil::isPlayerStarship(un);
		if (cpt>=0) 
			saveStringList(cpt,mission_key,vec);
	}
	if (!rooms.size()) {
		BASE_LOG(logvs::ERROR, "ERROR: there are no rooms in basefile \"%s%s%s\" ...",
                 basefile,compute_time_of_day(base,un),BASE_EXTENSION);
		rooms.push_back(new Room ());
		rooms.back()->deftext="ERROR: No rooms specified...";
#ifndef BASE_MAKER
		rooms.back()->AddObj(new Room::BaseShip (-1,0,0,0,0,-1,0,1,0,QVector(0,0,2),"default room"));
		BaseUtil::Launch(0,"default room",-1,-1,1,2,"ERROR: No rooms specified... - Launch");
		BaseUtil::Comp(0,"default room",0,-1,1,2,"ERROR: No rooms specified... - Computer",
				"Cargo Upgrade Info ShipDealer News Missions");
#endif
	}
	GotoLink(0);
        {
          for (unsigned int i=0;i<16;++i) {
            ExecuteDirector();
          }
        }
}


// Need this for NEW_GUI.  Can't ifdef it out because it needs to link.
void InitCallbacks(void) {
	if(BaseInterface::CurrentBase) {
		BaseInterface::CurrentBase->InitCallbacks();
	}
}
void TerminateCurrentBase(void) {
	if (BaseInterface::CurrentBase) {
		BaseInterface::CurrentBase->Terminate();
		BaseInterface::CurrentBase=NULL;
	}
}
void CurrentBaseUnitSet(Unit * un) {
	if (BaseInterface::CurrentBase) {
		BaseInterface::CurrentBase->caller.SetUnit(un);
	}
}
// end NEW_GUI.

void BaseInterface::Room::Comp::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Link::Click(base,x,y,button,state);
		Unit *un=base->caller.GetUnit();
		Unit *baseun=base->baseun.GetUnit();
		if (un&&baseun) {
			base->CallComp=true;
#ifdef NEW_GUI
            BaseComputer* bc = new BaseComputer(un, baseun, modes);
            bc->init();
            bc->run();
#else
			UpgradeCompInterface(un,baseun,modes);
#endif // NEW_GUI
		}
	}
}
void BaseInterface::Terminate() {
  Unit *un=caller.GetUnit();
  int cpt=UnitUtil::isPlayerStarship(un);
  if (un&&cpt>=0) {
    vector <string> vec;
    vec.push_back(string());
    saveStringList(cpt,mission_key,vec);
  }
  BaseInterface::CurrentBase=NULL;
  restore_main_loop();
  delete this;
}
extern void SwitchUnits(Unit* ol, Unit* nw);
extern void SwitchUnits2(Unit* nw);
extern void abletodock(int dock);
extern vector <int> switchunit;
extern vector <int> turretcontrol;
#include "ai/communication.h"

void BaseInterface::Room::Launch::Click (BaseInterface *base,float x, float y, int button, int state) {
	static int numtimes = 0;
	if (state==WS_MOUSE_UP) {
	  Link::Click(base,x,y,button,state);
	  static bool auto_undock_var = XMLSupport::parse_bool(vs_config->getVariable("physics","AutomaticUnDock","true"));
	  bool auto_undock = auto_undock_var;
	  Unit * bas = base->baseun.GetUnit();
	  Unit * playa = base->caller.GetUnit();
	  if (Network!=NULL && auto_undock && playa && bas) {
		  cerr<<"Sending an undock notification"<<endl;
		  int playernum = _Universe->whichPlayerStarship( playa );
		  if( playernum>=0) {
			  Network[playernum].undockRequest( bas->GetSerial());
			  auto_undock = false;
		  }
	  }
          if (playa&&bas) {
              if (((playa->name=="eject") || (playa->name == "ejecting") || (playa->name == "pilot") || (playa->name == "Pilot") || (playa->name == "Eject")) && (bas->faction==playa->faction)) {
                  playa->name = "return_to_cockpit";
              }
          }
	  if ((playa && bas)&&(auto_undock || (playa->name=="return_to_cockpit"))) {
              if (playa->UnDock (bas)) {
                  static bool wipe_oldmissions = XMLSupport::parse_bool(
                                    vs_config->getVariable("general", "wipe_oldmissions", "true"));
                  if (wipe_oldmissions)
                      Mission::wipeDeletedMissions();
              }
              CommunicationMessage c(bas,playa,NULL,0);
              c.SetCurrentState (c.fsm->GetUnDockNode(),NULL,0);
              if (playa->getAIState())
                  playa->getAIState()->Communicate (c);
              abletodock(5);
              
              if (playa->name=="return_to_cockpit") {
                  if (playa->faction == playa->faction) 
                      playa->owner = bas;
              }
              
              /*          if (playa->name=="return_to_cockpit")
                          {
			  // triggers changing to parent unit.
                          while (turretcontrol.size()<=_Universe->CurrentCockpit())
                          turretcontrol.push_back(0);
                          turretcontrol[_Universe->CurrentCockpit()]=1;
                          }
              */	  
              
	  }
	  base->Terminate();
        }
}
inline float aynrand (float min, float max) {
    return ((float)(rand ())/RAND_MAX)*(max-min)+min;
}

inline QVector randyVector (float min, float max) {
    return QVector (aynrand(min,max),
                    aynrand(min,max),
                    aynrand(min,max));
}
 void BaseInterface::Room::Eject::Click (BaseInterface *base,float x, float y, int button, int state) {
   static int numtimes = 0;
   if (state==WS_MOUSE_UP) {
     Link::Click(base,x,y,button,state);
     static bool auto_undock = XMLSupport::parse_bool(vs_config->getVariable("physics","AutomaticUnDock","true"));
     Unit * bas = base->baseun.GetUnit();
     Unit * playa = base->caller.GetUnit();
     if (playa && bas) {
       
 
       if (playa->name=="return_to_cockpit") {
         playa->name = "ejecting";
         Vector tmpvel=bas->Velocity * -1;
         if (tmpvel.MagnitudeSquared()<.00001) {
           tmpvel=randyVector(-(bas->rSize()),bas->rSize()).Cast();
           if (tmpvel.MagnitudeSquared()<.00001) {
             tmpvel=Vector(1,1,1);
           }
         }
         tmpvel.Normalize();
         playa->SetPosAndCumPos (bas->Position()+tmpvel*1.5*bas->rSize()+randyVector(-.5*bas->rSize(), .5*bas->rSize()));
         playa->SetAngularVelocity(bas->AngularVelocity);
         playa->SetOwner(bas);
         static float velmul=XMLSupport::parse_float(vs_config->getVariable("physics","eject_cargo_speed","1"));
         playa->SetVelocity(bas->Velocity*velmul+randyVector(-.25,.25).Cast());
         //            SwitchUnit(bas,playa);
       }
       
       playa->UnDock (bas);
       CommunicationMessage c(bas,playa,NULL,0);
       c.SetCurrentState (c.fsm->GetUnDockNode(),NULL,0);
       if (playa->getAIState())
         playa->getAIState()->Communicate (c);
       abletodock(5);
       playa->EjectCargo((unsigned int)-1);
       
       if ((playa->name == "return_to_cockpit") || (playa->name == "ejecting") || (playa->name == "eject") ||(playa->name == "Eject") ||(playa->name == "Pilot") || (playa->name == "pilot"))
       {
         playa->Kill();
       }
       
       
     }
     base->Terminate();
   }
 }

void BaseInterface::Room::Goto::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Link::Click(base,x,y,button,state);
		base->GotoLink(lnkindex);
	}
}

void BaseInterface::Room::Talk::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (base == NULL)
		return;
	if (state==WS_MOUSE_UP) {
		Link::Click(base,x,y,button,state);
		if (!objindex.empty()) {
			if (curroom < 0 || curroom >= base->rooms.size())
				return ;
			valid = false;
			BaseUtil::EraseObj(curroom, objindex);
			objindex="";
			base->othtext.SetText("");
		} else if (say.size()) {
			curroom=base->curroom;
			if (curroom < 0 || curroom >= base->rooms.size())
				return ;
//			index=base->rooms[curroom]->objs.size();
			int sayindex=rand()%say.size();
			objindex = index + "::room::talk::currentmsg";
			BaseTalk * newobj = new Room::BaseTalk(say[sayindex],objindex,true);
			base->rooms[curroom]->AddObj(newobj);
//			newobj->sayindex=(sayindex);
//			newobj->curtime=0;
			if (soundfiles[sayindex].size()>0) {
				int sound = AUDCreateSoundWAV (soundfiles[sayindex],false);
				if (sound==-1) {
					BASE_LOG(logvs::WARN, "Can't find the sound file %s",soundfiles[sayindex].c_str());
				} else {
//					AUDAdjustSound (sound,_Universe->AccessCamera ()->GetPosition(),Vector(0,0,0));
					AUDStartPlaying (sound);
					AUDDeleteSound(sound);//won't actually toast it until it stops
				}
			}
		} else {
			BASE_LOG(logvs::ERROR, "There are no things to say...");
			assert(0);
		}
	}
}

void BaseInterface::Room::Link::Click (BaseInterface *base,float x, float y, int button, int state) 
{
	unsigned int buttonmask = getMouseButtonMask();
	BASE_DBG(logvs::DBG+1, "Link::Click(%s, %p) button:%d state:%d", index.c_str(), this, button, state);
	if (state==WS_MOUSE_UP) {
		if (eventMask & UpEvent) {
			static std::string evtype("up");
			BaseUtil::SetMouseEventData(evtype,x,y,buttonmask);
			RunPython(this->pythonfile.c_str());
		}
	}
	if (state==WS_MOUSE_UP) {
		// For now, the same. Eventually, we'll want click & double-click
		if (eventMask & ClickEvent) {
			static std::string evtype("click");
			BaseUtil::SetMouseEventData(evtype,x,y,buttonmask);
			RunPython(this->pythonfile.c_str());
		}
	}
	if (state==WS_MOUSE_DOWN) {
		if (eventMask & DownEvent) {
			static std::string evtype("down");
			BaseUtil::SetMouseEventData(evtype,x,y,buttonmask);
			RunPython(this->pythonfile.c_str());
		}
	}
}

void BaseInterface::Room::Link::MouseMove (::BaseInterface* base,float x, float y, int buttonmask)
{
	// Compiling Python code each mouse movement == Bad idea!!!
	// If this support is needed we will need to use Python-C++ inheritance.
	// Like the Execute() method of AI and Mission classes.
	
	// Even better idea: Rewrite the entire BaseInterface python interface.
	
	if (eventMask & MoveEvent) {
		static std::string evtype("move");
		BASE_DBG(logvs::DBG+1, "Link::MouseMove(%s, %p)", index.c_str(), this);
		BaseUtil::SetMouseEventData(evtype,x,y,buttonmask);
		RunPython(this->pythonfile.c_str());
	}
	
}

void BaseInterface::Room::Link::MouseEnter (::BaseInterface* base,float x, float y, int buttonmask)
{
	if (eventMask & EnterEvent) {
		static std::string evtype("enter");
		BASE_DBG(logvs::DBG+1, "Link::MouseEnter(%s, %p)", index.c_str(), this);
		BaseUtil::SetMouseEventData(evtype,x,y,buttonmask);
		RunPython(this->pythonfile.c_str());
	}
}

void BaseInterface::Room::Link::MouseLeave (::BaseInterface* base,float x, float y, int buttonmask)
{
	if (eventMask & LeaveEvent) {
		static std::string evtype("leave");
		BASE_DBG(logvs::DBG+1, "Link::MouseLeave(%s, %p)", index.c_str(), this);
		BaseUtil::SetMouseEventData(evtype,x,y,buttonmask);
		RunPython(this->pythonfile.c_str());
	}
	clickbtn = -1;
}

void BaseInterface::Room::Link::Relink(const std::string &pfile)
{
	pythonfile = pfile;
}

struct BaseColor {
  unsigned char r,g,b,a;
};
static void AnimationDraw() {  
#ifdef RENDER_FROM_TEXTURE
  static StreamTexture T(512,256,NEAREST,NULL);
  BaseColor (* data)[512] = reinterpret_cast<BaseColor(*)[512]>(T.Map());
  bool counter=false;
  srand(time(NULL));
  for (int i=0;i<256;++i) {
    for (int j=0;j<512;++j) {
      data[i][j].r=rand()&0xff;
      data[i][j].g=rand()&0xff;
      data[i][j].b=rand()&0xff;
      data[i][j].a=rand()&0xff;
    }
  }
  T.UnMap();
  T.MakeActive();
  GFXTextureEnv(0,GFXREPLACETEXTURE);
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXDisable(CULLFACE);
  GFXBegin(GFXQUAD);
  GFXTexCoord2f(0,0);
  GFXVertex3f(-1.0,-1.0,0.0);
  GFXTexCoord2f(1,0);
  GFXVertex3f(1.0,-1.0,0.0);
  GFXTexCoord2f(1,1);
  GFXVertex3f(1.0,1.0,0.0);
  GFXTexCoord2f(0,1);
  GFXVertex3f(-1.0,1.0,0.0);
  GFXEnd();
#endif
}
void BaseInterface::Draw () {
	GFXColor(0,0,0,0);
        SetupViewport();
	StartGUIFrame(GFXTRUE);
    if (GetElapsedTime()<1) {
          AnimatedTexture::UpdateAllFrame();
    }
	Room::BaseTalk::hastalked=false;
	rooms[curroom]->Draw(this);
        AnimationDraw();
    
	float x,y;
        glViewport (0, 0, g_game.x_resolution,g_game.y_resolution);
    static float base_text_background_alpha=XMLSupport::parse_float(vs_config->getVariable("graphics","base_text_background_alpha","0.0625"));

    if (game_options.show_msgcenter_base && mission) {
        mission->gametime += GetElapsedTime(); //DirectorLoop not run in base
        Mission::RenderMessages(NULL);
    }
    if(CommandInterpretor && CommandInterpretor->console){
        GFXColorf(GFXColor(1,1,1,1));
        CommandInterpretor->renderconsole();
    }

	curtext.GetCharSize(x,y);
	curtext.SetPos(-.99,-1+(y*1.5));
//	if (!drawlinkcursor)
//		GFXColor4f(0,1,0,1);
//	else
//		GFXColor4f(1,.333333,0,1);
        if (curtext.GetText().find("XXX")!=0) {
			GFXColor tmpbg=curtext.bgcol;
			bool automatte=(0==tmpbg.a);
			if(automatte){curtext.bgcol=GFXColor(0,0,0,base_text_background_alpha);}
            curtext.Draw(curtext.GetText(),0,true,false,automatte);
	        curtext.bgcol=tmpbg;
        }
        othtext.SetPos(-.99,1);
//	GFXColor4f(0,.5,1,1);
		if (othtext.GetText().length()!=0){
			GFXColor tmpbg=othtext.bgcol;
			bool automatte=(0==tmpbg.a);
			if(automatte){othtext.bgcol=GFXColor(0,0,0,base_text_background_alpha);}
			othtext.Draw(othtext.GetText(),0,true,false,automatte);
			othtext.bgcol=tmpbg;
		}
        SetupViewport();
	EndGUIFrame (drawlinkcursor);
        glViewport (0, 0, g_game.x_resolution,g_game.y_resolution);
	Unit *un=caller.GetUnit();
	Unit *base=baseun.GetUnit();
	if (un&&(!base)) {
		BASE_LOG(logvs::ERROR, "Error: Base NULL");
		mission->msgcenter->add("game","all","[Computer] Docking unit destroyed. Emergency launch initiated.");
		for (int i=0;i<un->image->dockedunits.size();i++) {
			if (un->image->dockedunits[i]->uc.GetUnit()==base) {
				un->FreeDockingPort (i);
			}
		}
		Terminate();
	}
}

void BaseInterface::ProcessKeyboardBuffer()
{
	if (CurrentBase) {
		if (!CurrentBase->CallComp) {
			for (std::vector<unsigned int>::iterator it=base_keyboard_queue.begin(); it!=base_keyboard_queue.end(); ++it) {
				CurrentBase->Key(*it,0,false,0,0);
				CurrentBase->Key(*it,0,true,0,0);
			}
			base_keyboard_queue.clear();
		}
	}
}
