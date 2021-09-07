#ifndef __UNIT_CONST_CACHE_H
#define __UNIT_CONST_CACHE_H
#include "config.h"
#include <string>
#include <gnuhash.h>
#ifndef WIN32
class ConstHasher;
#endif
class StringIntKey {
  friend class ConstHasher;
  std::string key;
  int fac;
public:
  StringIntKey (std::string k, int f) {
    key=k;
    fac=f;
  }
  bool operator == (const StringIntKey &b) const {
    return fac==b.fac&&key==b.key;
  }
  bool operator < (const StringIntKey &b) const {
    if (fac!=b.fac)
      return fac<b.fac;
    return key < b.key;
  }
  operator size_t () const{return Hashtable<std::string,int,(1<<30)>::hash(key)^fac;}
};

#if !defined(_WIN32) && __GNUC__!=2
class ConstHasher {
public:
  template <class T> size_t operator () (const T&key)const{
    return vsHash<T>()(key);
  }
  size_t operator () (const StringIntKey &key)const{
    return vsHash<std::string>()(key.key)^vsHash<size_t>()((size_t)key.fac);
  }
};
#endif

template <class Typ,class Key> class ClassCache {
#if !defined(_WIN32) && __GNUC__!=2
  static vsUMap<Key, Typ *, ConstHasher> unit_cache;
#else
  static vsUMap<Key, Typ *> unit_cache;
#endif
 public:
  static const Typ *getCachedConst (Key k) {
    return getCachedMutable(k);
  }
  static Typ *getCachedMutable (const Key &k) {
#if !defined(_WIN32) && __GNUC__!=2
    typename vsUMap<Key,Typ *,ConstHasher>::iterator i=unit_cache.find(k);
#else
	typename vsUMap<Key,Typ *>::iterator i=unit_cache.find(k);
#endif
    if (i!=unit_cache.end())
      return (*i).second;
    return NULL;
  }
  static Typ *setCachedMutable (const Key &k, Typ *un) {
    unit_cache.insert (std::pair<Key,Typ *> (k,un));
    return un;
  }
  static const Typ *setCachedConst (const Key &k, Typ *un) {
    return setCachedMutable (k,un);
  }
  static void purgeCache(void (*Kill) (Typ * un)) {
    typename vsUMap<Key,Typ *,ConstHasher>::iterator i=unit_cache.begin();
    for (;i!=unit_cache.end();++i) {
      (*Kill) ((*i).second);
    }
    unit_cache.clear();
  }
};

#if (defined(__GNUC__)&& ((__GNUC__ == 3 && __GNUC_MINOR__ >= 4)|| __GNUC__>3))
template <class Typ,class Key> vsUMap<Key,Typ*,ConstHasher> ClassCache<Typ,Key>::unit_cache;
#endif

typedef ClassCache<Unit,StringIntKey> UnitConstCache;
typedef ClassCache<Mesh,std::string> WeaponMeshCache;

#endif

