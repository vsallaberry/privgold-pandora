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
#include <Python.h>
#include "python/python_class.h"
#include <string>
#include <stdlib.h>
#include "audiolib.h"
#include "base.h"
#include "base_util.h"
#include "universe_util.h"
#include "basecomputer.h"
#include "main_loop.h"

#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python.hpp>
typedef boost::python::dict BoostPythonDictionary ;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary ;
#endif

#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include "in_kb.h"
#include "unicode.h"
#include "log.h"
#include "vs_log_modules.h"

namespace BaseUtil {
	inline BaseInterface::Room *CheckRoom (int room) {
		if (!BaseInterface::CurrentBase) return 0;
		if (room<0||room>=BaseInterface::CurrentBase->rooms.size()) return 0;
		return BaseInterface::CurrentBase->rooms[room];
	}
	int Room (const std::string & text) {
		if (!BaseInterface::CurrentBase) return -1;
		BaseInterface::CurrentBase->rooms.push_back(new BaseInterface::Room());
		BaseInterface::CurrentBase->rooms.back()->deftext=text;
		return BaseInterface::CurrentBase->rooms.size()-1;
	}
	static BaseInterface::Room::BaseVSSprite * CreateTexture(int room, const std::string & index, const std::string & file, float x, float y) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return NULL;
		BaseInterface::Room::BaseVSSprite* newobj = new BaseInterface::Room::BaseVSSprite(file.c_str(),index);
		newroom->AddObj(newobj);
#ifdef BASE_MAKER
		newobj->texfile=file;
#endif
		float tx=0, ty=0;
		static bool addspritepos = XMLSupport::parse_bool(vs_config->getVariable("graphics","offset_sprites_by_pos","true"));
		if (addspritepos)
			newobj->spr.GetPosition(tx,ty);
                
		newobj->spr.SetPosition(x+tx,y+ty);
		return newobj;
	}
	void Texture(int room, const std::string & index, const std::string & file, float x, float y) {
		CreateTexture(room, index, file, x, y);
	}
    void Video(int room, const std::string & index, const std::string & vfile, const std::string & afile, float x, float y) {
        BaseInterface::Room *newroom=CheckRoom(room);
        if (!newroom) return;
        
        BaseInterface::Room::BaseVSSprite * newobj = BaseUtil::CreateTexture(room, index, vfile, x, y);
        
        int sndstream = AUDCreateMusic(afile);
        newobj->spr.SetTimeSource(sndstream);
    }
    void VideoStream(int room, const std::string & index, const std::string & streamfile, float x, float y, float w, float h) {
        BaseInterface::Room *newroom=CheckRoom(room);
        if (!newroom) return;
        
        static bool addspritepos = XMLSupport::parse_bool(vs_config->getVariable("graphics","offset_sprites_by_pos","true"));
        float tx=0, ty=0;
        
        BaseInterface::Room::BaseVSSprite *newobj = new BaseInterface::Room::BaseVSMovie(streamfile, index);
        if (addspritepos)
            newobj->spr.GetPosition(tx,ty);
        newobj->spr.SetPosition(x+tx,y+ty);
        newobj->spr.SetSize(w,h);
        
#ifdef BASE_MAKER
        newobj->texfile=file;
#endif
        
        newroom->AddObj(newobj);
    }
	void SetTexture(int room, const std::string & index, const std::string & file)
	{
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::objsmap_range_type its = newroom->GetObjRange(index);
				its.first != its.second; ++its.first) {
			BaseInterface::Room::BaseVSSprite* sprite = newroom->GetObj<BaseInterface::Room::BaseVSSprite>(its.first);
			if (newroom->CheckObj(sprite)) {
				sprite->SetSprite(file);
			} else if (!sprite) {
				BASE_LOG(logvs::WARN, "SetTexture: the object '%s' is not a Sprite !", index.c_str());
			}
		}
	}
	void SetTextureSize(int room, const std::string & index, float w, float h)
	{
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::objsmap_range_type its = newroom->GetObjRange(index);
				its.first != its.second; ++its.first) {
			BaseInterface::Room::BaseVSSprite* sprite = newroom->GetObj<BaseInterface::Room::BaseVSSprite>(its.first);
			if (newroom->CheckObj(sprite)) {
				sprite->SetSize(w,h);
			} else if (!sprite) {
				BASE_LOG(logvs::WARN, "SetTextureSize: the object '%s' is not a Sprite !", index.c_str());
			}
		}
	}
	void SetTexturePos(int room, const std::string & index, float x, float y)
	{
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::objsmap_range_type its = newroom->GetObjRange(index);
			 its.first != its.second; ++its.first) {
			BaseInterface::Room::BaseVSSprite* sprite = newroom->GetObj<BaseInterface::Room::BaseVSSprite>(its.first);
			if (newroom->CheckObj(sprite)) {
				sprite->SetPos(x,y);
			} else if (!sprite) {
				BASE_LOG(logvs::WARN, "SetTexturePos: the object '%s' is not a Sprite !", index.c_str());
			}
		}
	}
    void PlayVideo(int room, const std::string & index)
    {
    	BaseInterface::Room *newroom=CheckRoom(room);
    	if (!newroom) return;
    	for (BaseInterface::Room::objsmap_range_type its = newroom->GetObjRange(index);
    		 its.first != its.second; ++its.first) {
    		BaseInterface::Room::BaseVSSprite* sprite = newroom->GetObj<BaseInterface::Room::BaseVSSprite>(its.first);
    		if (newroom->CheckObj(sprite)) {
    			int snd = sprite->spr.GetTimeSource();
    			if (snd)
    				AUDStartPlaying(snd);
    		} else if (!sprite) {
    			BASE_LOG(logvs::WARN, "PlayVideo: the object '%s' is not a Sprite !", index.c_str());
    		}
    	}
    }
	void Ship (int room, const std::string & index,QVector pos,Vector Q, Vector R) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		Vector P = R.Cross(Q);
		P.Normalize();
		newroom->AddObj(new BaseInterface::Room::BaseShip(P.i,P.j,P.k,Q.i,Q.j,Q.k,R.i,R.j,R.k,pos,index));
//		return BaseInterface::CurrentBase->rooms[BaseInterface::CurrentBase->curroom]->links.size()-1;
	}
	void RunScript (int room, const std::string & ind, const std::string & pythonfile, float time) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->AddObj(new BaseInterface::Room::BasePython(ind, pythonfile, time));
	}
	void TextBox (int room, const std::string & ind, const std::string & text, float x, float y, Vector widheimult, Vector backcol, float backalp, Vector forecol) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->AddObj(new BaseInterface::Room::BaseText(text, x, y, widheimult.i, widheimult.j, widheimult.k, GFXColor(backcol, backalp), GFXColor(forecol), ind));
	}
	void SetTextBoxText(int room, const std::string & index, const std::string & text) {
        BASE_DBG(logvs::DBG+1, "SetTextBoxText '%s'", text.c_str());
        BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::objsmap_range_type its = newroom->GetObjRange(index);
			 its.first != its.second; ++its.first) {
			BaseInterface::Room::BaseText* basetext = newroom->GetObj<BaseInterface::Room::BaseText>(its.first);
			if (newroom->CheckObj(basetext)) {
				basetext->SetText(text);
			} else if (!basetext) {
				BASE_LOG(logvs::WARN, "SetTextBoxText: the object '%s' is not a TextBox !", index.c_str());
			}
		}
	}
    void SetTextBoxFont(int room, const std::string & index, const std::string & fontname) {
        BASE_DBG(logvs::DBG+1, "SetTextBoxFont '%s'", fontname.c_str());
        BaseInterface::Room *newroom=CheckRoom(room);
        if (!newroom) {
            if (BaseInterface::CurrentBase && room == -1 && index.empty())
                BaseInterface::CurrentFont = fontname.empty() ? NULL : getFontFromName(fontname);
            return;
        }
        for (BaseInterface::Room::objsmap_range_type its = newroom->GetObjRange(index);
        	 its.first != its.second; ++its.first) {
        	BaseInterface::Room::BaseText* basetext = newroom->GetObj<BaseInterface::Room::BaseText>(its.first);
        	if (newroom->CheckObj(basetext)) {
        		basetext->SetFont(fontname);
        	} else if (!basetext) {
        		BASE_LOG(logvs::WARN, "SetTextBoxFont: the object '%s' is not a TextBox !", index.c_str());
        	}
        }
    }
    std::string GetTextBoxFont(int room, const std::string & index) {
    	BASE_DBG(logvs::DBG+1, "GetTextBoxFont %d '%s'", room, index.c_str());
    	BaseInterface::Room *newroom=CheckRoom(room);
    	if (!newroom) {
    		return getFontName(BaseInterface::CurrentFont);
    	}
        for (BaseInterface::Room::objsmap_range_type its = newroom->GetObjRange(index);
        		its.first != its.second; ++its.first) {
        	BaseInterface::Room::BaseText* basetext = newroom->GetObj<BaseInterface::Room::BaseText>(its.first);
        	if (newroom->CheckObj(basetext)) {
        		return getFontName(basetext->GetFont());
        	} else if (!basetext) {
        		BASE_LOG(logvs::WARN, "GetTextBoxFont: the object '%s' is not a TextBox !", index.c_str());
        	}
    	}
        return "";
    }
	void SetLinkArea(int room, const std::string & index, float x, float y, float wid, float hei)
	{
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::linksmap_range_type its = newroom->GetLinkRange(index);
		     its.first != its.second; ++its.first) {
			BaseInterface::Room::Link* link = newroom->GetLink(its.first);
		    if (newroom->CheckLink(link)) {
				link->x   = x;
				link->y   = y;
				link->wid = wid;
				link->hei = hei;
			}
		}
	}
	void SetLinkText(int room, const std::string & index, const std::string & text)
	{
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::linksmap_range_type its = newroom->GetLinkRange(index);
			 its.first != its.second; ++its.first) {
			BaseInterface::Room::Link* link = newroom->GetLink(its.first);
			if (newroom->CheckLink(link)) {
				link->text= text;
			}
		}
	}
	void SetLinkPython(int room, const std::string & index, const std::string & python)
	{
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::linksmap_range_type its = newroom->GetLinkRange(index);
			 its.first != its.second; ++its.first) {
			BaseInterface::Room::Link* link = newroom->GetLink(its.first);
			if (newroom->CheckLink(link)) {
				link->Relink(python);
			}
		}
	}
	void SetLinkRoom(int room, const std::string & index, int to)
	{
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::linksmap_range_type its = newroom->GetLinkRange(index);
			 its.first != its.second; ++its.first) {
			BaseInterface::Room::Goto * lgoto = newroom->GetLink<BaseInterface::Room::Goto>(its.first);
			if (newroom->CheckLink(lgoto)) {
				lgoto->lnkindex = to;
			}
		}
	}
	void SetLinkEventMask(int room, const std::string & index, const std::string & maskdef)
	{ 
		int i;

		// c=click, u=up, d=down, e=enter, l=leave, m=move
		unsigned int mask = 0;
		for (i=0; i<maskdef.length(); ++i) {
			switch(maskdef[i]) {
			case 'c': case 'C': mask |= BaseInterface::Room::Link::ClickEvent; break;
			case 'u': case 'U': mask |= BaseInterface::Room::Link::UpEvent; break;
			case 'd': case 'D': mask |= BaseInterface::Room::Link::DownEvent; break;
			case 'e': case 'E': mask |= BaseInterface::Room::Link::EnterEvent; break;
			case 'l': case 'L': mask |= BaseInterface::Room::Link::LeaveEvent; break;
			case 'm': case 'M':
				BASE_LOG(logvs::NOTICE, "SetLinkEventMask: WARNING: Ignoring request for movement event mask.");
				//mask |= BaseInterface::Room::Link::MoveEvent;
				break;
			}
		} 

		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (BaseInterface::Room::linksmap_range_type its = newroom->GetLinkRange(index);
			 its.first != its.second; ++its.first) {
			BaseInterface::Room::Link* link = newroom->GetLink(its.first);
			if (newroom->CheckLink(link)) {
				link->setEventMask(mask);
			}
		}
	}
	static BaseInterface::Room::linksmap_type::iterator BaseLink (BaseInterface::Room *room, BaseInterface::Room::Link * lnk,
						  float x, float y, float wid, float hei, const std::string & text, bool front = false) {
		lnk->x=x;
		lnk->y=y;
		lnk->wid=wid;
		lnk->hei=hei;
		lnk->text=text;
		return room->AddLink(lnk, front);
	}
	void Link (int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, int to) {
		LinkPython (room, index, "",x, y,wid, hei, text, to);
	}
	void LinkPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text, int to) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		BaseInterface::Room::Goto * lgoto = new BaseInterface::Room::Goto (index,pythonfile);
		BaseLink(newroom,lgoto,x,y,wid,hei,text);
		lgoto->lnkindex=to;
	}
	void Launch (int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text) {
		LaunchPython (room, index,"", x, y, wid, hei, text);
	}
	void LaunchPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		BaseLink(newroom, new BaseInterface::Room::Launch (index,pythonfile), x,y,wid,hei,text);
	}
	void EjectPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		BaseLink(newroom, new BaseInterface::Room::Eject (index,pythonfile), x,y,wid,hei,text);
	}
	void Comp(int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, const std::string & modes) {
	  CompPython(room, index,"", x, y, wid, hei, text,modes) ;
	}
	void CompPython(int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text, const std::string & modes) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		BaseInterface::Room::Comp *newcomp=new BaseInterface::Room::Comp (index,pythonfile);
		BaseLink(newroom, newcomp, x,y,wid,hei,text);
		static const EnumMap::Pair modelist [] = {
			EnumMap::Pair ("Cargo", BaseComputer::CARGO), 
			EnumMap::Pair ("Upgrade", BaseComputer::UPGRADE), 
			EnumMap::Pair ("ShipDealer", BaseComputer::SHIP_DEALER), 
			EnumMap::Pair ("Missions", BaseComputer::MISSIONS),
			EnumMap::Pair ("News", BaseComputer::NEWS), 
			EnumMap::Pair ("Info", BaseComputer::INFO),
			EnumMap::Pair ("LoadSave", BaseComputer::LOADSAVE), 
			EnumMap::Pair ("Network", BaseComputer::NETWORK),
			EnumMap::Pair ("UNKNOWN", BaseComputer::LOADSAVE),
		};
		static const EnumMap modemap (modelist,sizeof(modelist)/sizeof(*modelist));
		const char *newmode=modes.c_str();
		int newlen=modes.size();
		char *curmode=new char [newlen+1];
		for (int i=0;i<newlen;) {
			int j;
			for (j=0;newmode[i]!=' '&&newmode[i]!='\0';i++,j++) {
				curmode[j]=newmode[i];
			}
			while(newmode[i]==' ')
				i++;
			if (j==0)
				continue;
			//in otherwords, if j is 0 then the 0th index will become null
			//EnumMap crashes if the string is empty.
			curmode[j]='\0';
			int modearg = modemap.lookup(curmode);
			if (modearg<BaseComputer::DISPLAY_MODE_COUNT) {
				newcomp->modes.push_back((BaseComputer::DisplayMode)(modearg));
			} else {
				BASE_LOG(logvs::WARN, "WARNING: Unknown computer mode %s found in python script...",curmode);
			}
		}
		delete [] curmode;
	}
	void Python(int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, const std::string & pythonfile,bool front) {
		//instead of "Talk"/"Say" tags
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		BaseLink(newroom, new BaseInterface::Room::Python (index,pythonfile), x,y,wid,hei,text,front);
	}

	void GlobalKeyPython(const std::string & pythonfile)
	{
		if (BaseInterface::CurrentBase)
			BaseInterface::CurrentBase->python_kbhandler = pythonfile;
	}

	void MessageToRoom(int room, const std::string & text) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->AddObj(new BaseInterface::Room::BaseTalk(text,"currentmsg",true));
	}
	void EnqueueMessageToRoom(int room, const std::string & text) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->AddObj(new BaseInterface::Room::BaseTalk(text,"currentmsg",false));
	}
	void Message(const std::string & text) {
		if (!BaseInterface::CurrentBase) return;
		MessageToRoom(BaseInterface::CurrentBase->curroom,text);
	}
	void EnqueueMessage(const std::string & text) {
		if (!BaseInterface::CurrentBase) return;
		EnqueueMessageToRoom(BaseInterface::CurrentBase->curroom,text);
	}
	void EraseLink (int room, const std::string & index) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		BASE_DBG(logvs::DBG+2, "api: Erasing links '%s'", index.c_str());
		newroom->EraseLink(index);
	}
	void EraseObj (int room, const std::string & index) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		BASE_DBG(logvs::DBG+2, "api: Erasing objs '%s'", index.c_str());
		newroom->EraseObj(index);
	}
	int GetCurRoom () {
		if (!BaseInterface::CurrentBase) return -1;
		return BaseInterface::CurrentBase->curroom;
	}
	void SetCurRoom (int room) {
		BaseInterface::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		if (!BaseInterface::CurrentBase) return;
		BaseInterface::CurrentBase->GotoLink(room);
	}
	int GetNumRoom () {
		if (!BaseInterface::CurrentBase) return -1;
		return BaseInterface::CurrentBase->rooms.size();
	}
	bool BuyShip(const std::string & name, bool my_fleet, bool force_base_inventory) {
		Unit * base = BaseInterface::CurrentBase->baseun.GetUnit();
		Unit * un=BaseInterface::CurrentBase->caller.GetUnit();
		return ::buyShip(base,un,name,my_fleet,force_base_inventory,NULL);
	}
	bool SellShip(const std::string & name) {
		Unit * base = BaseInterface::CurrentBase->baseun.GetUnit();
		Unit * un=BaseInterface::CurrentBase->caller.GetUnit();
		return ::sellShip(base,un,name,NULL);
	}

	Dictionary& _GetEventData()
	{
		static BoostPythonDictionary data;
		return data;
	}

	void SetEventData(BoostPythonDictionary data)
	{
		_GetEventData() = data;
	}

	void SetMouseEventData(const std::string & type, float x, float y, int buttonMask)
	{
		BoostPythonDictionary &data = _GetEventData();

		// Event type
		data["type"] = type;

		// Mouse data
		data["mousex"] = x;
		data["mousey"] = y;
		data["mousebuttons"] = buttonMask;

		SetKeyStatusEventData();
	}

	void SetKeyStatusEventData(unsigned int modmask)
	{
		BoostPythonDictionary &data = _GetEventData();

		// Keyboard modifiers (for kb+mouse)
		if (modmask==~0)
			modmask = pullActiveModifiers();
		data["modifiers"] = modmask;
		data["alt"]   = ((modmask & KB_MOD_ALT)!=0);
		data["shift"] = ((modmask & KB_MOD_SHIFT)!=0);
		data["ctrl"]  = ((modmask & KB_MOD_CTRL)!=0);
	}

	void SetKeyEventData(const std::string & type, unsigned int keycode, unsigned int modmask)
	{
		BoostPythonDictionary &data = _GetEventData();

		// Event type
		data["type"] = type;

		// Keycode
		data["key"] = keycode;
		if ((keycode>0x20) && (keycode < 128))
			data["char"] = std::string(1,keycode);
        else if (WSK_CODE_IS_UTF32(keycode)) {
            char utf8[MB_LEN_MAX+1];
            unsigned int utf32 = WSK_CODE_TO_UTF32(keycode);
            data["key"] = utf32;
            utf32_to_utf8(utf8, utf32);
            data["char"] = std::string(utf8);
        } else {
			data["char"] = std::string();
        }
		SetKeyStatusEventData(modmask);
	}

	const Dictionary& GetEventData()
	{
		return _GetEventData();
	}

	float GetTextHeight(const std::string & text, Vector widheimult)
	{
		static bool force_highquality = true;
		static bool use_bit = force_highquality||XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
		static float font_point = XMLSupport::parse_float (vs_config->getVariable ("graphics","font_point","16"));
		return use_bit ? getFontHeight(BaseInterface::CurrentFont) : (font_point * 2 / g_game.y_resolution);
	}

	float GetTextWidth(const std::string & text, Vector widheimult)
	{
		// Unsupported for now
		return 0;
	}

	void LoadBaseInterface(const std::string & name)
	{
		LoadBaseInterfaceAtDock(name,UniverseUtil::getPlayer(),UniverseUtil::getPlayer());
	}

	void LoadBaseInterfaceAtDock(const std::string & name, Unit* dockat, Unit *dockee)
	{
		if (BaseInterface::CurrentBase) 
			BaseInterface::CurrentBase->Terminate();
		BaseInterface *base = new BaseInterface(name.c_str(),dockat,dockee);
		base->InitCallbacks();
	}

	void refreshBaseComputerUI(const Cargo *carg) {
		if (carg) {
			// BaseComputer::draw() used dirty to determine what to recalculate.
			BaseComputer::dirty=1; // everything.
			//BaseComputer::dirtyCargo=*carg;
		} else {
			BaseComputer::dirty=2; // only title.
		}
	}

	void ExitGame()
	{
		CockpitKeys::QuitNow(KBData(),PRESS);
	}

}
