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
#include <sstream>
#include "multimap.h"

// MultiMap built-in Pretty-Printers
namespace ChainedMultimapUtil {
	std::string multimap_debug_key(const std::string & key) {
		return key;
	}
	std::string multimap_debug_key(const char * s) {
		if (s == NULL) return "(null)";
		return std::string(s);
	}
	std::string multimap_debug_key(const void * ptr) {
		std::ostringstream oss; oss << ptr; return oss.str();
	}
	std::string multimap_debug_key(long l) {
		std::ostringstream oss; oss << l; return oss.str();
	}
	std::string multimap_debug_key(unsigned long ul) {
		std::ostringstream oss; oss << ul; return oss.str();
	}
	std::string multimap_debug_key(int i) {
		std::ostringstream oss; oss << i; return oss.str();
	}
	std::string multimap_debug_key(unsigned int ui) {
		std::ostringstream oss; oss << ui; return oss.str();
	}
	std::string multimap_debug_key(double d) {
		std::ostringstream oss; oss << d; return oss.str();
	}
	std::string multimap_debug_key(float f) {
		std::ostringstream oss; oss << f; return oss.str();
	}
} // ! namespace ChainedMultimapUtil

/* *********************************************************************
 *
 * ChainedMultimap Unit TESTS
 *
 * *********************************************************************/
#ifdef VS_UNIT_TESTS

#include <stdlib.h>
#include <time.h>
#include <list>
#include <algorithm>
#include <sstream>
#include "multimap.h"

namespace ChainedMultimapTests {

using namespace ChainedMultimapUtil;

template <class T, class Map>
class MapTest {
public:
	typedef typename Map::iterator 						  iterator;
	typedef typename Map::key_type 						  key_type;
	typedef typename Map::mapped_type 					  mapped_type;
	typedef T 											  element_type;
	typedef std::list<std::pair<const key_type,element_type> > reflist_type;
	typedef std::pair<iterator,iterator>				  range_type;

	MapTest(const std::string & _name = "") : n_tests(0), n_errors(0), name(_name) {}
	virtual ~MapTest() {
		Check();
		fprintf(stderr, "%s(%s): %zu test(s), %zu error(s).\n", __func__,
				name.empty() ? multimap_debug_key((const void *)this).c_str() : name.c_str(),
				n_tests, n_errors);
	}

	size_t 				n_tests;
	size_t 				n_errors;
	const std::string 	name;
	Map 				map;
	reflist_type 		reflist;

	bool Check();

	iterator insert(const std::pair<const key_type, element_type> & value, bool front = false) {
		if(front)
			reflist.insert(reflist.begin(), value);
		else
			reflist.push_back(value);

		iterator ret = map.insert(value, front);

		if (!Check(ret != map.end() && ret->first == value.first && ret->second->elt == value.second)) {
			fprintf(stderr, "insert_test: bad iterator returned by insert\n");
		}

		Check();
		return ret;
	}

	iterator erase(iterator it) {
		typename reflist_type::iterator listit
			= (it == map.end() ? reflist.end() : std::find(reflist.begin(), reflist.end(),
					std::pair<const key_type,element_type>(it->first, it->second->elt)));

		std::pair<const key_type,element_type> refval = listit != reflist.end() ? *listit
				: std::pair<const key_type,element_type>(it->first, it->second->elt);
		iterator next = it; ++next;
		bool mapend = (next == map.end());
		std::pair<const key_type,element_type> nextval = mapend ? refval
			: std::pair<const key_type,element_type>(next->first, next->second->elt);

		if (Check(listit != reflist.end())) {
			reflist.erase(listit);
		} else {
			fprintf(stderr, "erase: ERROR: element '%s' not found in REFLIST\n", multimap_debug_key(it->first).c_str());
		}

		iterator ret = map.erase(it);

		if (!Check((mapend && ret == map.end())
				|| (!mapend && ret != map.end()
					&& ret->first == nextval.first && ret->second->elt == nextval.second))) {
			fprintf(stderr, "erase: ERROR: bad iterator returned by erase('%s')\n", multimap_debug_key(it->first).c_str());
		}
		Check();
		return ret;
	}

	void erase(const key_type & key) {
		map.erase(key);
		for (typename reflist_type::iterator listit = reflist.begin(); listit != reflist.end(); ) {
			if (listit->first == key) {
				listit = reflist.erase(listit);
			} else {
				++listit;
			}
		}
		Check();
	}

	void clear() {
		map.clear();
		reflist.clear();
		Check();
	}

	bool Check(bool cond) {
		++n_tests; if (!cond) ++n_errors; return cond;
	}

	bool CheckChained(typename Map::chained_iterator & it, typename reflist_type::iterator & refit);
};

template <class T, class Map>
bool MapTest<T, Map>::CheckChained(typename Map::chained_iterator & it, typename reflist_type::iterator & refit) {
	unsigned int saved_errors = n_errors;
	if (!Check(it != map.none())) {
		fprintf(stderr, "Check: ERROR: chained_it comparison withe none() failed\n");
	}
	if (!Check(*it == refit->second)) {
		fprintf(stderr, "Check: ERROR: chained_iterator('%s') has wrong order, expected '%s'\n",
				multimap_debug_key(*it).c_str(), multimap_debug_key(refit->second).c_str());
	}
	if (!Check(map.busy() == 1)) {
		fprintf(stderr, "Check: ERROR: map busy value(%zu) should be ONE\n", map.busy());
	}
	typename Map::chained_iterator it2 = it;
	if (!Check(map.busy() == 2)) {
		fprintf(stderr, "Check: ERROR: map busy value(%zu) should be TWO\n", map.busy());
	}
	if (it2 - 1 == map.none()) {
		if (!Check(--it2 == map.none()))
			fprintf(stderr, "Check: ERROR --it2 != map.none()\n");
	} else {
		if (!Check((--it2)++ == it - 1 && it2 == it))
			fprintf(stderr, "Check: ERROR (--it2)++ != it - 1 // it\n");
	}
	it2 = it;
	if (it2 + 1 == map.none()) {
		if (!Check(++it2 == map.none()))
			fprintf(stderr, "Check: ERROR ++it2 != map.none()\n");
	} else {
		if (!Check((++it2)-- == it + 1 && it2 == it))
			fprintf(stderr, "Check: ERROR (++it2)-- != it + 1 // it\n");
	}
	it2 = it;
	if (!Check(it2 + map.size() == map.none() && it2 - map.size() == map.none()
	&&  (it2 += 0) == it && (it2 -= 0) == it && (it2 += 2) == it + 2 && (it2 == map.none() || (it2 -= 4) == it - 2))) {
		fprintf(stderr, "Check: ERROR: chained_IT comparison/increment FAILED\n");
	}
	return n_errors == saved_errors;
}

template <class T, class Map>
bool MapTest<T, Map>::Check() {
	unsigned int saved_errors = n_errors;

	// Check Size
	if (!Check(reflist.size() == map.size())) {
		fprintf(stderr, "Check: ERROR: map(%zu) and ref list(%zu) have different size\n", map.size(), reflist.size());
	}
	if (map.size() == 0 && !Check(map.first() == map.last() && map.first() == map.none())) {
		fprintf(stderr, "Check: ERROR: map first()/last()/none() when size 0\n");
	}
	if (map.size() != 0 && !Check(map.first() != map.none() && map.last() != map.none())) {
		fprintf(stderr, "Check: ERROR: map first()/last()/none() when size != 0\n");
	}

	// Check elements in map are in ref list
	for (range_type its = std::make_pair(map.begin(), map.end()); its.first != its.second; ++its.first) {

		typename reflist_type::iterator listit = std::find(reflist.begin(), reflist.end(),
			std::pair<const key_type,element_type>(its.first->first, its.first->second->elt));

		if (!Check(listit != reflist.end())) {
			fprintf(stderr, "Check: ERROR: map element '%s' not found in reflist\n",
					multimap_debug_key(its.first->first).c_str());
		}
	}

	// Check the chained_iterator has the right order
	typename reflist_type::iterator refit = reflist.begin();
	size_t n = 0;
	if (!Check(map.busy() == 0)) {
		fprintf(stderr, "Check: ERROR, map is busy\n");
	}
	for (typename Map::chained_iterator it = map.first(); !it.end() && refit != reflist.end(); ++it, ++n, ++refit) {
		CheckChained(it, refit);
	}
	if (!Check(n == reflist.size() && refit == reflist.end())) {
		fprintf(stderr, "Check: ERROR: chained_iterator (%zu) did not iterate all (%zu) \n", n, reflist.size());
	}

	// chained_iterator reverse order
	if (!Check(map.busy() == 0)) {
		fprintf(stderr, "Check: ERROR, map is busy\n");
	}
	{
		typename Map::chained_iterator it = map.last();
		refit = reflist.end();
		for (n = 0; !it.end() && refit != reflist.begin(); --it, ++n) {
			CheckChained(it, --refit);
		}
		if (!Check(n == reflist.size() && it == map.none() && it.end() && refit == reflist.begin())) {
			fprintf(stderr, "Check: ERROR: chained_iterator(reverse,%zu) did not iterate all (%zu)\n", n, reflist.size());
		}
	}
	return n_errors == saved_errors;
}

/* **************
 * *** TESTS ****
 * **************/
typedef MapTest<int, ChainedMultimap<const std::string, int> > maptest_type;
typedef MapTest<int, ChainedMultimap<long, int> > maptest2_type;

class MapTest3_Key {
public:
	int a;
	MapTest3_Key(int _a) : a(_a) {}
	bool operator==(const MapTest3_Key & other) const { return a == other.a; }
};
class MapTest3_Val {
public:
	int a;
	MapTest3_Val(int _a) : a(_a) {}
	MapTest3_Val operator ++(int) { MapTest3_Val ret = *this; ++a; return ret; }
	bool operator==(const MapTest3_Val & other) const { return a == other.a; }
};
struct MapTest3_Comp {
	bool operator() (const MapTest3_Key & key1, const MapTest3_Key & key2) const { return key1.a < key2.a; }
};
typedef MapTest<MapTest3_Val, ChainedMultimap<MapTest3_Key, MapTest3_Val, MapTest3_Comp> > maptest3_type;

unsigned int test_multimap() {
	srand(time(NULL));
	maptest_type maptest;
	maptest_type::element_type val = 1;
	maptest.insert(std::make_pair("B_TEST001", val++));
	maptest.insert(std::make_pair("B_TEST001", val++));
	maptest.insert(std::make_pair("C_TEST001", val++));
	maptest.insert(std::make_pair("C_TEST001", val++));
	maptest.insert(std::make_pair("A_TEST001", val++));
	maptest.insert(std::make_pair("D_TEST001", val++));
	maptest.insert(std::make_pair("D_TEST001", val++));
	maptest.insert(std::make_pair("D_TEST001", val++));
	maptest.insert(std::make_pair("Z_TEST001", val++), true);

	for (maptest_type::range_type its = maptest.map.equal_range("B_TEST001"); its.first != its.second; ) {
		maptest.erase(its.first);
		break ;
	}
	maptest.erase("C_TEST001");
	maptest.clear();

	maptest_type maptestbig;
	const size_t test_size = 1000;
	size_t underrange_count = 0, i;
	maptest_type::element_type range = test_size / 8;
	val = 1;
	for (size_t i = 0; i < test_size; ++i) {
		int keyval = rand() % (test_size / 2);
		std::ostringstream oss; oss << keyval;
		if (val < range) ++underrange_count;
		maptestbig.insert(std::make_pair(oss.str(), val++));
	}
	i=0;
	for (maptest_type::range_type its = std::make_pair(maptestbig.map.begin(),maptestbig.map.end());
			its.first != its.second; ++i) {
		if (its.first->second->elt < range)
			its.first = maptestbig.erase(its.first);
		else
			++its.first;
	}
	if (!maptestbig.Check(maptestbig.map.size() == test_size - underrange_count))
		fprintf(stderr, "Check: ERROR: %zu element should have been removed\n", underrange_count);
	i=0;
	for (maptest_type::range_type its = std::make_pair(maptestbig.map.begin(),maptestbig.map.end());
			its.first != its.second; ++i) {
		if (rand()%(100)<10)
			its.first = maptestbig.erase(its.first);
		else
			++its.first;
	}
	for (maptest_type::range_type its = std::make_pair(maptestbig.map.begin(),maptestbig.map.end());
		 its.first != its.second; ) {
		its.first = maptestbig.erase(its.first);
	}
	if (!maptestbig.Check(maptestbig.map.size() == 0))
		fprintf(stderr, "Check: ERROR: all %zu element should have been removed\n", test_size);
	maptestbig.clear();

	maptest2_type maptest2;
	maptest2_type::element_type val2 = 1;
	maptest2.insert(std::make_pair(0, val2++));
	maptest2.insert(std::make_pair(0, val2++));
	maptest2.insert(std::make_pair(1, val2++));
	maptest2.insert(std::make_pair(-1, val2++));
	maptest2.insert(std::make_pair(4, val2++), true);
	maptest2.insert(std::make_pair(1, val2++));
	maptest2.insert(std::make_pair(1, val2++));
	maptest2.insert(std::make_pair(1, val2++));
	maptest2.erase(maptest2.map.find(4));
	maptest2.erase(maptest2.map.find(0));
	maptest2.erase(4);
	maptest2.clear();

	maptest3_type maptest3;
	maptest3_type::element_type val3 = MapTest3_Val(5);
	maptest3.insert(std::make_pair(MapTest3_Key(3), val3++));
	maptest3.insert(std::make_pair(MapTest3_Key(1), val3++));
	maptest3.insert(std::make_pair(MapTest3_Key(3), val3++));
	maptest3.clear();

	return maptest.n_errors + maptest2.n_errors + maptest3.n_errors;
}

} // ! namespace ChainedMultimapTests

#endif
