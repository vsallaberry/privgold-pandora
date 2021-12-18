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
  MessageCenter written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know
// what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"
#include "networking/netserver.h"
#include "vegastrike.h"

#include "cmd/unit_generic.h"
#include "mission.h"
#include "easydom.h"

#include "msgcenter.h"
#include <algorithm>
//#include "vs_globals.h"
//#include "vegastrike.h"

const MessageTruePredicate MessageCenter::_true_predicate = MessageTruePredicate();
const MessageCenter::FilterList MessageCenter::_emptyFilterList = MessageCenter::FilterList();

void MessageCenter::add(std::string from, std::string to, std::string message, double delay) {
    gameMessage msg;
    
    msg.from = from;
    msg.to = to;
    msg.message = message;
    
    msg.time = mission->getGametime() + delay;
    
    if (SERVER) {
        VSServer->sendMessage(from, to, message, (float)delay);
    }

#ifdef VS_DEBUG_LOG
    static unsigned int msgcenter_lvl = logvs::vs_log_level("msgcenter");
    if (msgcenter_lvl >= logvs::DBG+1) {
		unsigned int flags = logvs::vs_log_setflag(logvs::F_MSGCENTER, false);
		logvs::vs_log("msgcenter", logvs::DBG+1, logvs::F_NO_LVL_CHECK, __FILE__, __func__, __LINE__,
					  "pushing message (%s>%s) %s",
					  msg.from.get().c_str(), msg.to.get().c_str(), msg.message.get().c_str());
		logvs::vs_log_setflags(flags);
    }
#endif

    messages.push_back(msg);
}

bool MessageCenter::last(unsigned int n, gameMessage & m,
                         const FilterList & who,
                         const FilterList & whoNOT,
                         const FilterList & from,
                         const FilterList & fromNOT) {
    if (who.empty() && whoNOT.empty() && from.empty() && fromNOT.empty()) {
        int size  = messages.size();
        int index = size-1-n;
        
        if (index>=0) {
            m=messages[index];
            return true;
        } else {
            return false;
        }
    }
    return last(n, m, MessageWhoPredicate<MessageStrEqualPred>(who, whoNOT, from, fromNOT));
}

void MessageCenter::clear(const FilterList & who,
                          const FilterList & whoNOT,
                          const FilterList & from,
                          const FilterList & fromNOT) {
    if (who.empty() && whoNOT.empty() && from.empty() && fromNOT.empty()) {
        messages.clear();
    } else {
        clear(MessageWhoPredicate<MessageStrEqualPred>(who, whoNOT, from, fromNOT));
    }
}

