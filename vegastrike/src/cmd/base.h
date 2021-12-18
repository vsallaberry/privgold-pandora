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
#ifndef __BASE_H__
#define __BASE_H__
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <vector>
#include <string>
#include <map>
#include "basecomputer.h"
#include "gfx/hud.h"
#include "gfx/sprite.h"
#include <stdio.h>
#include "multimap.h"

//#define BASE_MAKER
//#define BASE_XML //in case you want to write out XML instead...

#define BASE_EXTENSION ".py"

class BaseInterface {
	bool drawlinkcursor;
	TextPlane curtext;
public:
	class Room {
	public:
		class Link {
		public:
			enum {
				ClickEvent = (1<<0),
				DownEvent  = (1<<1),
				UpEvent    = (1<<2),
				MoveEvent  = (1<<3),
				EnterEvent = (1<<4),
				LeaveEvent = (1<<5)
			};

			bool valid;
			std::string pythonfile;
			float x,y,wid,hei,alpha;
			std::string text;
			const std::string index;
			unsigned int eventMask;
			int clickbtn;

			virtual void Click (::BaseInterface* base, float x, float y, int button, int state);
			virtual void MouseMove (::BaseInterface* base, float x, float y, int buttonmask);
			virtual void MouseEnter (::BaseInterface* base, float x, float y, int buttonmask);
			virtual void MouseLeave (::BaseInterface* base, float x, float y, int buttonmask);

			void setEventMask(unsigned int mask) 
				{ eventMask = mask; }

			explicit Link (const std::string &ind,const std::string &pfile) : valid(true), pythonfile(pfile),x(0.f),y(0.f),wid(0.f),hei(0.f),alpha(1.0f),index(ind),eventMask(ClickEvent),clickbtn(-1) {}
			virtual ~Link(){} 
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			virtual void Relink(const std::string &pfile);
		};
		class Goto : public Link {
		public:
			int lnkindex;
			virtual void Click (::BaseInterface* base,float x, float y, int button, int state);
			virtual ~Goto () {}
			explicit Goto (const std::string & ind, const std::string & pythonfile) : Link(ind,pythonfile), lnkindex(0) {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Comp : public Link {
		public:
			vector <BaseComputer::DisplayMode> modes;
			virtual void Click (::BaseInterface* base,float x, float y, int button, int state);
			virtual ~Comp () {}
			explicit Comp (const std::string & ind, const std::string & pythonfile) : Link(ind,pythonfile) {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Launch : public Link {
		public:
			virtual void Click (::BaseInterface* base,float x, float y, int button, int state);
			virtual ~Launch () {}
			explicit Launch (const std::string & ind, const std::string & pythonfile) : Link(ind,pythonfile) {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Eject : public Link {
		public:
			virtual void Click (::BaseInterface* base,float x, float y, int button, int state);
			virtual ~Eject () {}
			explicit Eject (const std::string & ind, const std::string & pythonfile) : Link(ind,pythonfile) {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Talk : public Link {
		public:
			//At the moment, the BaseInterface::Room::Talk class is unused... but I may find a use for it later...
			std::vector <std::string> say;
			std::vector <std::string> soundfiles;
			std::string objindex;
			int curroom;
			virtual void Click (::BaseInterface* base,float x, float y, int button, int state);
			explicit Talk (const std::string & ind, const std::string & pythonfile);
			virtual ~Talk () {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Python : public Link {
		public:
			//std::string file;
			//virtual void Click (::BaseInterface* base,float x, float y, int button, int state); // Commented since base class already does what's necessary
			Python(const std::string & ind,const std::string & pythonfile) : Link(ind,pythonfile) {}
			virtual ~Python () {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class BaseObj {
		public:
			const std::string index;
			bool valid;
			virtual void Draw (::BaseInterface *base);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BaseObj () {}
			explicit BaseObj (const std::string & ind) : index(ind), valid(true) {}
		};
		class BasePython : public BaseObj{
		public:
			std::string pythonfile;
			float timeleft;
			float maxtime;
			virtual void Draw (::BaseInterface *base);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BasePython () {}
			BasePython (const std::string & ind, const std::string & python, float time) : BaseObj(ind), pythonfile(python), timeleft(0), maxtime(time) {}
			virtual void Relink(const std::string & python);
		};
		class BaseText : public BaseObj{
		public:
			TextPlane text;
			virtual void Draw (::BaseInterface *base);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BaseText () {}
			BaseText (const std::string & texts, float posx, float posy, float wid, float hei, float charsizemult, GFXColor backcol, GFXColor forecol, const std::string & ind) : BaseObj(ind), text(forecol, backcol) {
                text.SetFont(BaseInterface::CurrentFont);
				text.SetPos(posx,posy);
				text.SetSize(wid,hei);
				float cx=0, cy=0;
				text.GetCharSize(cx, cy);
				cx*=charsizemult;
				cy*=charsizemult;
				text.SetCharSize(cx, cy);
				text.SetText(texts);
			}
			void SetText(const std::string & newtext) { text.SetText(newtext); }
            void SetFont(const std::string & fontname) {
                if (fontname.empty()) text.SetFont(BaseInterface::CurrentFont); else text.SetFont(fontname);
            }
            void * GetFont() { return text.GetFont(); }
			void SetPos(float posx, float posy) { text.SetPos(posx,posy); }
			void SetSize(float wid, float hei) { text.SetSize(wid,hei); }
		};
		class BaseShip : public BaseObj {
		public:
			virtual void Draw (::BaseInterface *base);
			Matrix mat;
			virtual ~BaseShip () {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			explicit BaseShip (const std::string & ind) : BaseObj (ind) {}
			BaseShip (float r0, float r1, float r2, float r3, float r4, float r5, float r6, float r7, float r8, QVector pos, const std::string & ind)
				:BaseObj (ind),mat (r0,r1,r2,r3,r4,r5,r6,r7,r8,QVector(pos.i/2,pos.j/2,pos.k))  {}
		};
		class BaseVSSprite : public BaseObj {
		public:
			virtual void Draw (::BaseInterface *base);
			VSSprite spr;
#ifdef BASE_MAKER
			std::string texfile;
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BaseVSSprite () {}
			BaseVSSprite (const std::string &spritefile, const std::string & ind);
			void SetSprite (const std::string &spritefile);
			void SetPos (float posx, float posy) { spr.SetPosition(posx,posy); }
			void SetSize (float wid, float hei) { spr.SetSize(wid,hei); }
			void SetTime (float t) { spr.SetTime(t); }
            
        protected:
            BaseVSSprite(const std::string &ind, const VSSprite &sprite) : BaseObj(ind), spr(sprite) {}
		};
        
        class BaseVSMovie : public BaseVSSprite {
        public:
            virtual ~BaseVSMovie() {}
            BaseVSMovie (const std::string &moviefile, const std::string & ind);
            void SetMovie(const std::string &moviefile);
            
            float GetTime() const;
            void SetTime(float t);
        };
        
		class BaseTalk : public BaseObj {
		public:
			static bool hastalked;
			virtual void Draw (::BaseInterface *base);
//			Talk * caller;
//			int sayindex;
			int curchar;
			float curtime;
			virtual ~BaseTalk () {}
			std::string message;
//			BaseTalk (Talk *caller) : caller (caller),  sayindex (0),curchar(0) {}
			BaseTalk (const std::string & msg,const std::string & ind, bool only_one_talk);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp) {}
#endif
		};
		std::string soundfile;
		std::string deftext;
		/* the multimap was introduced in order to speed up lookups on base interface objects
		 * from python, which is very often. The BaseObj vector was kept (or replaced by a
		 * ChainedMultiMap which simulates the vector), because, in the current
		 * design, the order of Objs insertions defines the order of display layers.
		 * Ultimately, the pythons could name their indexes according to the drawing priority they
		 * need, so that the engine would rely only on index alphabetic order, and the vector would
		 * not be needed anymore. Moreover, the pythons would not need to erase/reinsert objects if
		 * their display priority did not change. */
		//#define BASEOBJ_ALPHAORDER_MULTIMAP
		//#define BASELINK_ALPHAORDER_MULTIMAP
		#define BASEOBJ_CHAINED_MULTIMAP
		#define BASELINK_CHAINED_MULTIMAP
		struct ObjsKeyCompare {
			bool operator()(const std::string & s1, const std::string & s2) const;
		};
	   #ifdef BASEOBJ_CHAINED_MULTIMAP
	   # define BASEOBJ_IT_OBJ(IT) ((IT)->second->elt)
		typedef ChainedMultimap <const std::string, BaseObj* /*, ObjsKeyCompare */ > objsmap_type;
	   #else
	   # define BASEOBJ_IT_OBJ(IT) ((IT)->second)
		typedef std::multimap <const std::string, BaseObj* /*, ObjsKeyCompare */ > objsmap_type;
	   # if !defined(BASEOBJ_ALPHAORDER_MULTIMAP)
		typedef std::vector<BaseObj*> objs_type;
	   # endif
	   #endif
	   #ifdef BASELINK_CHAINED_MULTIMAP
	   # define BASELINK_IT_LNK(IT) ((IT)->second->elt)
		typedef ChainedMultimap <const std::string, Link* /*, ObjsKeyCompare */ > linksmap_type;
	   #else
	   # define BASELINK_IT_LNK(IT) ((IT)->second)
		typedef std::multimap <const std::string, Link* /*, ObjsKeyCompare */ > linksmap_type;
	   # if !defined(BASELINK_ALPHAORDER_MULTIMAP)
		typedef std::vector <Link*> links_type;
	   # endif
	   #endif
		typedef std::pair<objsmap_type::iterator,objsmap_type::iterator> objsmap_range_type;
		typedef std::pair<linksmap_type::iterator,linksmap_type::iterator> linksmap_range_type;
	   #ifdef BASE_MAKER
		void EndXML(FILE *fp);
	   #endif

		void Draw (::BaseInterface *base);
		void Click (::BaseInterface* base, float x, float y, int button, int state);
		void Key(::BaseInterface* base, unsigned int ch, unsigned int mod, bool release, float x, float y);
		Link * MouseOver (::BaseInterface *base,float x, float y);
		objsmap_type::iterator AddObj(BaseObj* obj, bool front = false);
		linksmap_type::iterator AddLink(Link* obj, bool front = false);
		inline bool CheckObj(BaseObj * obj) { return obj && obj->valid; }
		inline bool CheckLink(Link * lnk) { return lnk && lnk->valid; }
		void Enter();

		inline objsmap_type::iterator EraseObj(const std::string & index) {
			return this->EraseObj(index, false, NULL, false, false);
		}
		inline objsmap_type::iterator EraseObj(const std::string & index, BaseObj * obj) {
			return this->EraseObj(index, true, obj, false, false);
		}
		inline objsmap_range_type GetObjRange(const std::string & index) {
			return index.empty() ? std::make_pair(objsmap.begin(),objsmap.end()) : objsmap.equal_range(index);
		}
		inline BaseObj * GetObj(objsmap_type::iterator & it) const {
			return BASEOBJ_IT_OBJ(it);
		}
		template <class BaseObjT>
		inline BaseObjT * GetObj(objsmap_type::iterator & it) const {
			return dynamic_cast<BaseObjT*>(GetObj(it));
		}
		inline linksmap_type::iterator EraseLink(const std::string & index) {
			return this->EraseLink(index, false, NULL, false, false);
		}
		inline linksmap_type::iterator EraseLink(const std::string & index, Link * obj) {
			return this->EraseLink(index, true, obj, false, false);
		}
		inline linksmap_range_type GetLinkRange(const std::string & index) {
			return index.empty() ? std::make_pair(linksmap.begin(),linksmap.end()) : linksmap.equal_range(index);
		}
		inline Link * GetLink(linksmap_type::iterator & it) const {
			return BASELINK_IT_LNK(it);
		}
		template <class LinkT>
		inline LinkT * GetLink(linksmap_type::iterator & it) const {
			return dynamic_cast<LinkT *>(GetLink(it));
		}
		Link * GetCurrentLink(int offset = 0, bool only_valid = true, bool set = false);
		Room ();
		~Room ();

	protected:
		objsmap_type objsmap;
		linksmap_type linksmap;

		Link * LinkAtPosition(float x, float y);
		objsmap_type::iterator EraseObj(const std::string & index, bool on_ptr, BaseObj * obj, bool only_notvalid, bool force);
		linksmap_type::iterator EraseLink(const std::string & index, bool on_ptr, Link * lnk, bool only_notvalid, bool force);

	   #if !defined(BASEOBJ_CHAINED_MULTIMAP) && !defined(BASEOBJ_ALPHAORDER_MULTIMAP)
		objs_type objs;
	   #endif
	   #if !defined(BASELINK_CHAINED_MULTIMAP) && !defined(BASELINK_ALPHAORDER_MULTIMAP)
		links_type links;
	   #endif
	   #if defined(BASEOBJ_ALPHAORDER_MULTIMAP)
		bool _busyobj;
	   #endif
	   #if defined(BASELINK_ALPHAORDER_MULTIMAP)
		bool _busylink;
	   #endif

	   #if defined(BASELINK_CHAINED_MULTIMAP)
		linksmap_type::chained_iterator hotlink_it;
	   #elif defined(BASELINK_ALPHAORDER_MULTIMAP)
		linksmap_type::iterator hotlink_it;
	   #else
		size_t hotlink_idx;
	   #endif
	   #undef BASEOBJ_IT_OBJ
	   #undef BASELINK_IT_LNK
	};
	friend class Room;
	friend class Room::BaseTalk;
	int curroom;
	std::vector <Room*> rooms;
	TextPlane othtext;
	static BaseInterface *CurrentBase;
    static void * CurrentFont;
	bool CallComp;
	UnitContainer caller;
	UnitContainer baseun;

	std::string python_kbhandler;

#ifdef BASE_MAKER
	void EndXML(FILE *fp);
#endif
	void Terminate ();
	void GotoLink(int linknum);
	void InitCallbacks ();
	void CallCommonLinks (const std::string & name, const std::string & value);
//	static void BaseInterface::beginElement(void *userData, const XML_Char *names, const XML_Char **atts);
//	void BaseInterface::beginElement(const string &name, const AttributeList attributes);
//	static void BaseInterface::endElement(void *userData, const XML_Char *name);
	void Load(const char * filename, const char * time_of_day, const char * faction);
	static void ClickWin (int x, int y, int button, int state);
	void Click (int x, int y, int button, int state);
	void Key(unsigned int ch, unsigned int mod, bool release, int x, int y);
	static void Joystick(unsigned int which, float x, float y, float z, unsigned int buttons, unsigned int state);
	static void PassiveMouseOverWin (int x, int y);
	static void ActiveMouseOverWin (int x, int y);
	static void ProcessKeyboardBuffer();
	void MouseOver (int x, int y);
	BaseInterface (const char *basefile, Unit *base, Unit *un);
	~BaseInterface ();
	void Draw ();
};

#endif
