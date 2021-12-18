/*
 * Vega Strike MultiMap
 * Copyright (C) 2021 Vincent Sallaberry
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
#ifndef VS_MULTIMAP_H
#define VS_MULTIMAP_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <map>
#include <stdlib.h>

#ifdef HAVE_CODECVT
# define MULTIMAP_HAVE_CXX11_ERASE
#endif

#ifdef VEGASTRIKE_VERSION
# include "vs_log_modules.h"
#else
# define MMAP_LOG(...)
# define MMAP_DBG(...)
#endif

namespace ChainedMultimapUtil {
#ifndef MULTIMAP_HAVE_CXX11_ERASE
// std::multimap::erase() is void and does not return an iterator until CXX11
template <class Map>
typename Map::iterator multimap_cxx11_erase(Map & map, typename Map::iterator & it) {
	typename Map::iterator next = it;
	++next;

#if defined(MULTIMAP_ERASE_LOOSE_ITS) // Should not happen: CXX STDs even 98 says erase does not invalidate other iterators
	bool stop = (next == map.end());
	typename Map::key_type saveindex = (!stop ? next->first : it->first);
	typename Map::mapped_type savevalue = (!stop ? next->second : it->second);
#endif
	(void) ((std::multimap<typename Map::key_type,typename Map::mapped_type,
			               typename Map::key_compare,typename Map::allocator_type> *)
			(&map))->erase(it);

#if defined(MULTIMAP_ERASE_LOOSE_ITS)
	if (stop)
		return map.end();
	next = map.lower_bound(saveindex);
	while (next != map.end() && savevalue != next->second)
		++next;
#endif
	return next;
}
#endif
// MultiMap built-in Pretty-Printers
std::string multimap_debug_key(const std::string & key);
std::string multimap_debug_key(const void * ptr);
std::string multimap_debug_key(const char * s);
std::string multimap_debug_key(long l);
std::string multimap_debug_key(unsigned long ul);
std::string multimap_debug_key(int i);
std::string multimap_debug_key(unsigned int ui);
std::string multimap_debug_key(double d);
std::string multimap_debug_key(float f);
template <typename Key>
static std::string multimap_debug_key(Key key) {
	return "undefined";
}
} // ! namespace ChainedMultimapUtil

// ChainedContainer
template<typename T>
class ChainedContainer {
public:
	typedef T value_type;
	value_type elt;
	ChainedContainer<T> * next;
	ChainedContainer<T> * prev;
	ChainedContainer(const value_type & _elt, ChainedContainer<T> * _next = NULL, ChainedContainer<T> * _prev = NULL)
		: elt(_elt), next(_next), prev(_prev) {}
};

// ChainedMultimap
//   std::multimap based class maintaining a double-linked
//   chained list of 'insertion-time' ordered elements.
//   Requirements for classes Key and T:
//     + must have bool operator==(const <T/Key> & other) const; (or global operator==)
//     + std::less<Key> must be implemented or the Compare class
//       must have bool operator()(const Key & k1, const Key & k2) const; // return true if k1 < k2
template<class Key, class T, class Compare = std::less<Key>,
		 class Allocator = std::allocator<std::pair<const Key, ChainedContainer<T> *> > >
class ChainedMultimap : public std::multimap<Key, ChainedContainer<T> *, Compare, Allocator> {
private:
	typedef std::multimap<Key,ChainedContainer<T> *,Compare,Allocator> super;
public:
	using typename super::iterator;
	using typename super::const_iterator;
	using typename super::value_type;
	using typename super::mapped_type;
	using typename super::key_type;
	using typename super::size_type;

public:
	ChainedMultimap(const Compare & comp = Compare())
		: super::multimap(comp), _first(NULL), _last(NULL), _ref_count(0) {}

	virtual ~ChainedMultimap() { this->clear(); };

	iterator insert(const std::pair<const Key,T> & value, bool front = false) {
		MMAP_DBG(logvs::DBG+2, "insert '%s' first=%s last=%s",
				 ChainedMultimapUtil::multimap_debug_key(value.first).c_str(),
		 		 _first ? ChainedMultimapUtil::multimap_debug_key(_first->elt).c_str() : "(none)",
				 _last  ? ChainedMultimapUtil::multimap_debug_key(_last->elt).c_str()  : "(none)");

		ChainedContainer<T> * elt = new ChainedContainer<T>(value.second, front ? _first : NULL, front ? NULL : _last);
		if (elt == NULL) return this->end();

		if (front) {
			if (_first == NULL) {
				_last = elt;
			} else {
				_first->prev = elt;
			}
			_first = elt;
		} else {
			if (_last == NULL) {
				_first = elt;
			} else {
				_last->next = elt;
			}
			_last = elt;
		}
		return super::insert(std::make_pair(value.first, elt));
	}

   #ifdef MULTIMAP_HAVE_CXX11_ERASE
	iterator erase(const_iterator it) {
   #else
	iterator erase(iterator it) {
   #endif
		MMAP_DBG(logvs::DBG+2, "erase '%s':%s first=%s last=%s",
			     it != this->end() ? ChainedMultimapUtil::multimap_debug_key(it->first).c_str() : "(none)",
			     it != this->end() ? ChainedMultimapUtil::multimap_debug_key(it->second->elt).c_str() : "(none)",
			     _first ? ChainedMultimapUtil::multimap_debug_key(_first->elt).c_str() : "(none)",
			     _last  ? ChainedMultimapUtil::multimap_debug_key(_last->elt).c_str()  : "(none)");

		if (it == this->end()) return this->end();

		mapped_type deleting = it->second;
		if (deleting) {
			if (deleting->next == NULL) {
				_last = deleting->prev;
			} else {
				deleting->next->prev = deleting->prev;
			}
			if (deleting->prev == NULL) {
				_first = deleting->next;
			} else {
				deleting->prev->next = deleting->next;
			}
		}
	   #ifdef MULTIMAP_HAVE_CXX11_ERASE // CXX11 only
		iterator next = super::erase(it); // DO NOT USE it after this
	   #else
		iterator next = ChainedMultimapUtil::multimap_cxx11_erase(*this, it);
	   #endif
		if (deleting) delete deleting;
		return next;
	}

public:
	class chained_iterator {
	public:
		typedef ChainedMultimap<Key,T,Compare,Allocator> container_type;
		bool end() const { return _list == NULL; }
		T & operator *() const { return _list->elt; }
		chained_iterator(const chained_iterator & i) : _map(i._map), _list(i._list) { init(); }
		virtual ~chained_iterator() { --(_map->_ref_count); }

		bool operator==(const chained_iterator & i) const { return _map == i._map && _list == i._list; }
		bool operator!=(const chained_iterator & i) const { return !(*this == i); }
		chained_iterator & operator ++() { _list=_list->next; return *this; }
		chained_iterator & operator --() { _list=_list->prev; return *this; }
		chained_iterator operator ++(int) { chained_iterator ret = *this; ++(*this); return ret; }
		chained_iterator operator --(int) { chained_iterator ret = *this; --(*this); return ret; }
		chained_iterator & operator+=(ssize_t n) {
			chained_iterator & ret = *this; if (n < 0) ret -= -n; else while (_list && n--) ++ret; return ret;
		}
		chained_iterator & operator-=(ssize_t n) {
			chained_iterator & ret = *this; if (n < 0) ret += -n; else while (_list && n--) --ret; return ret;
		}
		chained_iterator operator+(ssize_t n) const { chained_iterator ret = *this; ret += n; return ret; }
		chained_iterator operator-(ssize_t n) const { chained_iterator ret = *this; ret -= n; return ret; }

	protected:
		friend class ChainedMultimap;
		container_type *	_map;
		mapped_type 		_list;
		void init() 		{ ++_map->_ref_count; }
		chained_iterator(container_type * map, mapped_type value) : _map(map), _list(value) { init(); }
	};

	chained_iterator first() { return chained_iterator(this, _first); }
	chained_iterator last()  { return chained_iterator(this, _last); }
	chained_iterator none()  { return chained_iterator(this, NULL); }
	size_t busy() const 	 { return _ref_count; }

protected:
	mapped_type _first, _last;
	size_t _ref_count;

public:
	/********************************************************************************
	 * Overriding some std::multimap methods in order to use new insert() and erase()
	 ********************************************************************************/
	iterator insert(const value_type & value) {
		return this->insert(std::make_pair(value.first, value.second->elt));
	}
	template< class InputIt >
	void insert( InputIt first, InputIt last ) {
		while (first != last) insert(*first++);
	}
	size_type erase(const key_type & key) {
		unsigned int n = 0;
		for (std::pair<iterator,iterator> its = this->equal_range(key); its.first != its.second; ++n) {
			its.first = this->erase(its.first);
		}
		return n;
	}
   #ifdef MULTIMAP_HAVE_CXX11_ERASE
	void erase(const_iterator first, const_iterator end) {
   #else
	void erase(iterator first, iterator end) {
   #endif
		while (first != end) first = this->erase(first);
	}
   #ifdef MULTIMAP_HAVE_CXX11_ERASE
	iterator insert(const_iterator hint, const value_type & value) {
   #else
	iterator insert(iterator hint, const value_type & value) {
   #endif
		(void)hint;
		return this->insert(value);
	}
	void clear() {
		for (iterator it = this->begin(); it != this->end(); ++it) {
			if (it->second) { delete(it->second); it->second = NULL; }
		}
		_first = _last = NULL;
		super::clear();
	}
};

#ifdef VS_UNIT_TESTS
namespace ChainedMultimapTests {
	unsigned int test_multimap();
}
#endif

#endif // !#define VS_MULTIMAP_H
