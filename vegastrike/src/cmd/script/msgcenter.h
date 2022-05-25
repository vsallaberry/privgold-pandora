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

#ifndef _MSGCENTER_H_
#define _MSGCENTER_H_

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

#include "vegastrike.h"

#include "mission.h"
//#include "easydom.h"
//#include "vs_globals.h"
//#include "vegastrike.h"

#include "SharedPool.h"
#include "log.h"

/* the message content stored in the message center */
class gameMessage {
public:
    StringPool::Reference from, to, message;
    double time;
};

/* Predefined Predicates for last() and clear() */
class MessageStrEqualPred;
template<class StringPredicate> class MessageWhoPredicate;
typedef MessageWhoPredicate<MessageStrEqualPred> MessageWhoStrPredicate;
class MessageTruePredicate {
public:
	virtual ~MessageTruePredicate() {}
    virtual bool operator() (gameMessage & other) const { (void)other; return true; }
};

/* the MessageCenter Class */
class MessageCenter {
public:
    /* the list of names (from/to) to be filtered */
    typedef std::vector<std::string> FilterList;
    
    void        add(const std::string & from, const std::string & to, const std::string & message, double delay=0.0);
    
    template <class MsgPredicate>
    bool        last(unsigned int n, gameMessage & m, MsgPredicate predicate);
    
    bool last(unsigned int n, gameMessage & m,
              const FilterList & who     = EmptyFilterList(),
              const FilterList & whoNOT  = EmptyFilterList(),
              const FilterList & from    = EmptyFilterList(),
              const FilterList & fromNOT = EmptyFilterList());
    
    template <class MsgPredicate>
    void        clear(MsgPredicate predicate);
    
    void clear(const FilterList & who     = EmptyFilterList(),
               const FilterList & whoNOT  = EmptyFilterList(),
               const FilterList & from    = EmptyFilterList(),
               const FilterList & fromNOT = EmptyFilterList());
    
    std::vector<gameMessage> messages;

    static const FilterList & EmptyFilterList() { return _emptyFilterList; }
    
public:
    /* fast alternative to last() */
    template <class MsgPredicate>
    class ReverseIterator {
    public:
        ReverseIterator(const ReverseIterator<MsgPredicate> & other)
        : _predicate(other._predicate), _msgcenter(other._msgcenter), _index(other._index) {}
        explicit
        ReverseIterator(MessageCenter * msgcenter, MsgPredicate predicate)
        : _predicate(predicate), _msgcenter(msgcenter) {
            _index = (ssize_t)_msgcenter->messages.size() - 1;
            next();
        }
        ReverseIterator<MsgPredicate> & operator++() {
            --_index; next(); return *this;
        }
        gameMessage & operator *()                   {
            return _msgcenter->messages[_index];
        }
        ReverseIterator<MsgPredicate> erase() {
            ReverseIterator<MsgPredicate> copy = *this;
            _msgcenter->messages.erase(_msgcenter->messages.begin()+_index);
            return ++copy;
        }
        bool end() { return _index < 0; }
    protected:
        void next() {
            while (_index >= 0 && ! _predicate(_msgcenter->messages[_index])) {
                --_index;
            }
        }
    protected:
        MsgPredicate _predicate;
        MessageCenter * _msgcenter;
        ssize_t _index;
    };
    
    typedef ReverseIterator<MessageTruePredicate> last_iterator;
    typedef ReverseIterator<MessageWhoStrPredicate> last_whoiterator;
    
    template <class MsgPredicate>
    ReverseIterator<MsgPredicate> last_begin(MsgPredicate predicate) {
        return ReverseIterator<MsgPredicate>(this, predicate);
    }
    last_iterator last_begin() {
        return last_begin(_true_predicate);
    }
protected:
    static const MessageTruePredicate _true_predicate;
    static const FilterList _emptyFilterList;
};

template <class MsgPredicate>
void MessageCenter::clear (MsgPredicate predicate) {
    VS_DBG("msgcenter", logvs::DBG+1, "clear: checking messages (total %zu)", messages.size());
    for (ssize_t i = messages.size() - 1; i >= 0; --i) {
        if (predicate(messages[i])) {
            messages.erase(messages.begin()+i);
        }
    }
}

template <class MsgPredicate>
bool MessageCenter::last(unsigned int n, gameMessage & m, MsgPredicate predicate) {
    int j=0;
    int i=0;
    for (i = messages.size()-1; i >= 0; i--) {
        if (predicate(messages[i])) {
            if (j==(int)n)
                break;
            j++;
        }
    }
    if (i < 0)
        return false;
    
    m = messages[i];
    return true;
}

/* Predefined Predicates for last() and clear() */
//static MessageTruePredicate MessageCenter::_true_predicate = MessageTruePredicate();

class MessageStrEqualPred {
public:
    explicit MessageStrEqualPred(const std::string & str) : _str(str) {};
    virtual ~MessageStrEqualPred() {}
    virtual bool operator() (const std::string & other) const { return _str == other; }
protected:
    const std::string & _str;
};

template<class StringPredicate>
class MessageWhoPredicate {
public:
    explicit
    MessageWhoPredicate(const MessageCenter::FilterList & who    = MessageCenter::EmptyFilterList(),
                        const MessageCenter::FilterList & whoNOT = MessageCenter::EmptyFilterList(),
                        const MessageCenter::FilterList & from   = MessageCenter::EmptyFilterList(),
                        const MessageCenter::FilterList & fromNOT= MessageCenter::EmptyFilterList())
        : _who(who), _whoNOT(whoNOT), _from(from), _fromNOT(fromNOT)
    {}
    virtual ~MessageWhoPredicate() {}
    virtual bool operator() (gameMessage & gm) const {
        StringPredicate toPred(gm.to.get());
        StringPredicate fromPred(gm.from.get());
        if (std::find_if(_whoNOT.begin(), _whoNOT.end(), toPred) == _whoNOT.end()
        &&  (_who.empty() || std::find_if(_who.begin(), _who.end(), toPred) != _who.end())
        &&  std::find_if(_fromNOT.begin(), _fromNOT.end(), fromPred) == _fromNOT.end()
        &&  (_from.empty() || std::find_if(_from.begin(), _from.end(), fromPred) != _from.end())) {
            return true;
        }
        return false;
    }
protected:
    const MessageCenter::FilterList & _who;
    const MessageCenter::FilterList & _whoNOT;
    const MessageCenter::FilterList & _from;
    const MessageCenter::FilterList & _fromNOT;
};

#endif // _MSGCENTER_H_
