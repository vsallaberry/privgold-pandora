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
  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/
#include "config.h"
#include "python/python_class.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#include <pwd.h>
#endif

#ifdef HAVE_FNMATCH_H
# include <fnmatch.h>
#endif

#include <expat.h>

#include <fstream>
#include <sstream>

#include "xml_support.h"

#include "vegastrike.h"
#include "vsfilesystem.h"
#include "lin_time.h"
#include "cmd/unit_generic.h"

#include "cmd/ai/order.h"
#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"
#include "savegame.h"
#include "msgcenter.h"
#include "cmd/briefing.h"
#include "pythonmission.h"
#ifdef HAVE_PYTHON
#include "Python.h"
#endif
#include "flightgroup.h"
#include "gldrv/winsys.h"
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/class.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif
#include "gfx/cockpit_generic.h"
//#include "vegastrike.h"
#include "log.h"
#include "unicode.h"
#include "options.h"

extern vs_options game_options;

#define DIR_LOG(lvl, ...) VS_LOG("game", lvl, __VA_ARGS__)

/* *********************************************************** */
//ADD_FROM_PYTHON_FUNCTION(pythonMission)

void Mission::DirectorUpdateGameTime() {
	double oldgametime=gametime;
	gametime+=SIMULATION_ATOM;//elapsed;
	if (getTimeCompression()>=.1) {
		if (gametime<=oldgametime)
			gametime=SIMULATION_ATOM;
	}
}

void Mission::DirectorLoop(){
   try {
      BriefingLoop();
      if (runtime.pymissions)
         runtime.pymissions->Execute();
   }catch (...) {
      if (PyErr_Occurred()) {
         PyErr_Print();
         PyErr_Clear();
         fflush(stderr);         
         fflush(stdout);
      }
      throw;
   }
}
void Mission::DirectorEnd(){
  if(director==NULL){
    return;
  }
  RunDirectorScript ("endgame");


}

void Mission::DirectorShipDestroyed(Unit *unit){
  Flightgroup *fg=unit->getFlightgroup();

  if(fg==NULL){
    DIR_LOG(logvs::INFO, "ship %s destroyed-no flightgroup", unit ? unit->getFullname().c_str() : "(null)");
    return;
  }
  if(fg->nr_ships_left<=0 && fg->nr_waves_left>0){
    DIR_LOG(logvs::WARN, "WARNING: unit %s: nr_ships_left<=0", unit ? unit->getFullname().c_str() : "(null)");
    return;
  }
  
  fg->nr_ships_left-=1;

  char buf[512];
  if ((fg->faction.length()+fg->type.length()+fg->name.length()+12+30)<sizeof(buf)) {
    sprintf(buf,"Ship destroyed: %s:%s:%s-%d",fg->faction.c_str(),fg->type.c_str(),fg->name.c_str(),unit->getFgSubnumber());
  } else {
    sprintf(buf,"Ship destroyed: (ERROR)-%d",unit->getFgSubnumber());
  }
  
  
  msgcenter->add("game","all",buf);

  if(fg->nr_ships_left==0){
    DIR_LOG(logvs::NOTICE, "no ships left in fg %s",fg->name.c_str());
    if(fg->nr_waves_left>0){
      //      printf("relaunching wave %d of fg %s\n",fg->waves-fg->nr_waves_left,fg->name.c_str());
      sprintf(buf,"Relaunching %s wave",fg->name.c_str());
      mission->msgcenter->add("game","all",buf);

      // launch new wave
      fg->nr_waves_left-=1;
      fg->nr_ships_left=fg->nr_ships;

      Order *order=NULL;
      order= unit->getAIState()?unit->getAIState()->findOrderList():NULL;
      fg->orderlist=NULL;
      if(order){
	DIR_LOG(logvs::NOTICE, "found an orderlist");
	fg->orderlist=order->getOrderList();
      }
      CreateFlightgroup cf;
      cf.fg = fg;
      cf.unittype= CreateFlightgroup::UNIT;
      cf.fg->pos=unit->Position();
      cf.waves = fg->nr_waves_left;
      cf.nr_ships = fg->nr_ships;
      
      //      cf.type = fg->type;
      call_unit_launch(&cf,UNITPTR,string(""));
    }
    else{
      mission->msgcenter->add("game","all","Flightgroup "+fg->name+" destroyed");
    }
  }
}

bool Mission::BriefingInProgress() {
  return (briefing!=NULL);
}
void Mission::BriefingStart() {
  if (briefing) {
    BriefingEnd();
  }
  briefing = new Briefing();
  if (runtime.pymissions)
	runtime.pymissions->callFunction("initbriefing");
  //RunDirectorScript ("initbriefing");
}
void Mission::BriefingUpdate() {
  if (briefing){
      briefing->Update();
  }
}

void Mission::BriefingLoop() {
  if (briefing) {
    if (runtime.pymissions)
      runtime.pymissions->callFunction("loopbriefing");
    //RunDirectorScript ("loopbriefing");
  }
}
class TextPlane * Mission::BriefingRender() {
  if (briefing) {
    vector <std::string> who;
    who.push_back ("briefing");
    string str1;
    gameMessage g1,g2;
	if (msgcenter->last(0,g1,who)) {
		str1= g1.message;
	}
    if (msgcenter->last(1,g2,who)) {
		str1 = str1 + string("\n")+g2.message;
    }
    briefing->tp.SetText (str1);
    briefing->Render();
    return &briefing->tp;
  }
  return NULL;
}

void Mission::BriefingEnd() {
  if (briefing) {
	if (runtime.pymissions)
		runtime.pymissions->callFunction("endbriefing");
	//RunDirectorScript ("endbriefing");      
    delete briefing;
    briefing = NULL;
  }
}

void Mission::DirectorBenchmark(){
   total_nr_frames++;

  //cout << "elapsed= " << elapsed << " fps= " << 1.0/elapsed << " average= " << ((double)total_nr_frames)/gametime << " in " << gametime << " seconds" << endl;

  if(benchmark>0.0 && benchmark<gametime){
    DIR_LOG(logvs::NOTICE, "Game was running for %lf secs,   av. framerate %lf",
            gametime, ((double)total_nr_frames)/gametime);
      
    winsys_exit(0);
  }
}

/* *********************************************
 * Render MessageCenter
 * ********************************************/
#if !defined(HAVE_STRCASECMP) && defined(HAVE_STRICMP)
# define strcasecmp(s1,s2) stricmp(s1,s2)
#endif
#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
# define strncasecmp(s1,s2,n) strnicmp(s1,s2,n)
#endif

#if 0
std::string tp_manual_line_truncation(TextPlane * tp, const std::string & msg, unsigned int & lines) {
    std::string color, line;
    unsigned int n = 0;
    unsigned int maxChars = (unsigned int) (1.9 / (charW*0.7));
    // Internal Truncation
    for (Utf8Iterator it = Utf8Iterator::begin(msg); it != it.end(); ) {
        unsigned int origpos = it.pos();
        if (*it == '#' && *(++it) != '#') {
            if (*(it += 6) != '#' && *(it+1) != '#') {
                color = msg.substr(origpos, 7);
            }
            line += color;
            if (it == it.end()) {
                line += (char) '\n';
                ++lines;
            }
            continue ;
        }
        it++;
        line = line + msg.substr(origpos, it.pos() - origpos);
        if (++n >= maxChars || it == it.end()) {
            line += (char) '\n';
            line += color;
            ++lines;
            n = 0;
        }
    }
    return line;
}
#endif

static void filter_msg(MessageCenter::FilterList & who, const std::string & filter, const std::string & me) {
    for (Utf8Iterator it = Utf8Iterator::begin(filter); it != it.end(); ) {
        Utf8Iterator tmp = std::find(it, it.end(), ';');
        std::string elt = filter.substr(it.pos(), tmp.pos() - it.pos());
        if (tmp != it.end()) {
            ++tmp;
        }
        if (elt == "me") {
            //Unit * un = _Universe->AccessCockpit()->GetParent();
            //if (un)
            //    elt = un->GetName() + '/' + un->GetFactionName();
            who.push_back(me);
        } else if (!elt.empty()){
            who.push_back (elt);
        }
        VS_LOG("msgcenter", logvs::VERBOSE, "msgcenter filter %s: PUSHED %s", filter.c_str(), who.back().c_str());
        it = tmp;
    }
}

class MsgFnMatchItPattern : public MessageStrEqualPred { // the pattern is in iterator, not data
    public:
        explicit MsgFnMatchItPattern(const std::string & str) : MessageStrEqualPred(str) {}
        virtual ~MsgFnMatchItPattern() {}
        virtual bool operator() (const std::string & other) const {
            //fprintf(stderr, "fnMatchItPattern other=%p str=%p\n", other.c_str(), _str.c_str());
#          ifdef HAVE_FNMATCH_H
            return (fnmatch(other.c_str(), _str.c_str(), FNM_CASEFOLD) == 0);
#          else
            return other == _str;
#          endif
        }
};

typedef MessageWhoPredicate<MsgFnMatchItPattern> MessageWhoFnMatchPred;
typedef MessageCenter::ReverseIterator<MessageWhoPredicate<MsgFnMatchItPattern> > MessageWhoFnMatchIterator;

class MessageWhoPredicateLogs : public MessageWhoFnMatchPred {
public:
	explicit MessageWhoPredicateLogs(const MessageCenter::FilterList & who    = MessageCenter::EmptyFilterList(),
            						 const MessageCenter::FilterList & whoNOT = MessageCenter::EmptyFilterList(),
									 const MessageCenter::FilterList & from   = MessageCenter::EmptyFilterList(),
									 const MessageCenter::FilterList & fromNOT= MessageCenter::EmptyFilterList())
									: MessageWhoPredicate(who, whoNOT, from, fromNOT) {}
	virtual ~MessageWhoPredicateLogs() {}
	bool operator()(gameMessage & gm) {
		if (gm.to.get() == "all" && strncasecmp("log/", gm.from.get().c_str(), 4) == 0) {
			return true;
		}
		return MessageWhoFnMatchPred::operator()(gm);
	}
};

void Mission::RenderMessages(TextPlane * tp) {
    if (msgcenter == NULL || mission == NULL) {
        return ;
    }
    static bool clear_msgcenter = XMLSupport::parse_bool(vs_config->getVariable("message_center", "clear", "true"));
    static bool clear_outofscreen = XMLSupport::parse_bool(vs_config->getVariable("message_center", "clear_outofscreen", "false"));
    static float msg_expiration = XMLSupport::parse_float(vs_config->getVariable("message_center", "expiration", "20"));
    static bool init_done = false;
    static MessageCenter::FilterList who, whoNOT, from, fromNOT, toKEEP, fromKEEP;
    static MessageWhoFnMatchPred who_pred(who, whoNOT, from, fromNOT);
    static MessageWhoPredicateLogs who_log_pred(who, whoNOT, from, fromNOT);
    static MessageWhoFnMatchPred keep_pred(toKEEP, MessageCenter::EmptyFilterList(), fromKEEP, MessageCenter::EmptyFilterList());
    static std::string me;
    static TextPlane * s_tp = NULL;
    static void * font = NULL;
    static float screen_width_ratio = XMLSupport::parse_float(
                                       vs_config->getVariable("message_center", "screen_width_ratio", "1.0"));
    static float screen_height_ratio= XMLSupport::parse_float(
                                       vs_config->getVariable("message_center", "screen_height_ratio", "1.0"));
    if ( ! init_done ) {
        std::ostringstream oss;
        oss << std::string("p") << _Universe->CurrentCockpit();
        me = oss.str();
        std::string to_filter_yes = vs_config->getVariable("message_center", "to_filter_allow", "");
        std::string to_filter_no = vs_config->getVariable("message_center", "to_filter_deny",
                                                          "news,bar,briefing");
        std::string from_filter_yes = vs_config->getVariable("message_center", "from_filter_allow", "");
        std::string from_filter_no = vs_config->getVariable("message_center", "from_filter_deny", "");
        std::string from_filter_keep = vs_config->getVariable("message_center", "from_filter_keep", "");
        std::string to_filter_keep = vs_config->getVariable("message_center", "to_filter_keep", "news");
        filter_msg(who, to_filter_yes, me);
        filter_msg(whoNOT, to_filter_no, me);
        filter_msg(from, from_filter_yes, me);
        filter_msg(fromNOT, from_filter_no, me);
        filter_msg(fromKEEP, from_filter_keep, me);
        filter_msg(toKEEP, to_filter_keep, me);
        init_done = true;
    }
    if (tp == NULL) {
        if (s_tp == NULL)
            s_tp = new TextPlane(GFXColor(1,1,1,1),GFXColor(0.0, 0.0, 0.0, 0.0));
        tp = s_tp;
    }
    void * save_font = tp->GetFont();
    unsigned int save_flags = tp->SetFlag(TextPlane::FLG_UNDERSCORE_AS_SPACE, false);
    if (font == NULL) {
        std::string fontName = vs_config->getVariable("message_center", "font",
                                                      vs_config->getVariable("graphics", "font", "helvetica12"));
        font = tp->SetFont(fontName);
        if (tp == s_tp) {
            save_font = font;
        }
        VS_DBG("msgcenter", logvs::DBG, "INIT fontname=%s font=%p fontHeight=%f",
               fontName.c_str(), tp->GetFont(), tp->GetFontHeight());
    } else if (save_font != font) {
        font = tp->SetFont(font);
    }
    
    gameMessage     gm;
    double          gametime = mission->getGametime();
    std::string     text;
    unsigned int    count;
    float           fontHeight = tp->GetFontHeight();
    float           posx = -0.96, posy = 0.99;
    
    count = 0;
    for (MessageCenter::last_iterator it = msgcenter->last_begin(); ! it.end(); ++it) {
        gm = *it;
        bool keep, display = keep = game_options.force_msgcenter_log ? who_log_pred(gm) : who_pred(gm);
        if (display) {
            if (gm.time >= 0) {
                if (gametime < gm.time) {
                    continue ; // delayed message, wait
                }
                if (msg_expiration > 0) {
                    gm.time = -(gametime + msg_expiration);
                    *it = gm; // set expiration date
                }
            } else if (gm.time < 0 && gametime >= -(gm.time)) {
                keep = display = false; // expired message
            }
        }
        bool out_of_screen = (count + 1) * fontHeight > (2 * posy) * screen_height_ratio;
        if (clear_msgcenter && ((out_of_screen && clear_outofscreen) || (!keep && ! keep_pred(gm)))) {
            VS_DBG("msgcenter", logvs::DBG+1, "erasing entry %s>%s: %s",
                   gm.from.get().c_str(), gm.to.get().c_str(), gm.message.get().c_str());
            it.erase();
            continue ;
        }
        if (!display || out_of_screen) { //if ignored or out_of_screen (don't let textplane trunk most recents)
            continue ; // for out_of_screen:continue instead of break to set the expiration date.
        }
        std::string msg = gm.message.get();
        msg = "#00FF00" + gm.from.get() + " > " + (me == gm.to.get() ? "me" : gm.to.get()) + " : " + "#FFFFFF" + msg;
        
        VS_DBG("msgcenter", logvs::DBG+3, "FULL MESSAGE : '%s' (time=%f)", msg.c_str(), gm.time);
        text = msg + (char) '\n' + text; // TextPlane will do the truncation
        ++count;
    }
    if (count) {
        static float background_alpha = XMLSupport::parse_float(vs_config->getVariable(
                                        "message_center", "text_background_alpha","0.0625"));
        tp->SetSize( (2*(-posx) * screen_width_ratio) + posx, (2*(-posy) * screen_height_ratio) + posy);
        tp->SetPos(posx, posy);
        GFXColor tpbg = tp->bgcol;
        GFXColor tpfg = tp->col;
        tp->col = GFXColor(1,1,1,1);
        bool automatte = (0.0 == tpbg.a);
        if(automatte) { tp->bgcol = GFXColor(0,0,0,background_alpha); }
        tp->Draw(text, 0, true, false, automatte);
        tp->bgcol=tpbg;
        tp->col=tpfg;
    }
    VS_DBG("msgcenter", logvs::DBG+2, "displayed=%u save_font=%p font=%p fontHeight=%f",
           count, save_font, tp->GetFont(), tp->GetFontHeight());
    if (save_font != font) {
        tp->SetFont(save_font);
    }
    tp->SetFlags(save_flags);
}
