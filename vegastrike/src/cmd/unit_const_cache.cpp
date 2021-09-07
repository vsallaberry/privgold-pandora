#include <string>
#include <vector>
#include <gnuhash.h>


#include "unit_generic.h"
#include "unit_const_cache.h"
#if (!defined(__GNUC__)) || !(defined(__GNUC__)&& ((__GNUC__ == 3 && __GNUC_MINOR__ >= 4)|| __GNUC__>3))
#if !defined(_WIN32) && __GNUC__!=2
vsUMap<StringIntKey,Unit*,ConstHasher> UnitConstCache::unit_cache;
vsUMap<std::string , Mesh*,ConstHasher> WeaponMeshCache::unit_cache;
#else
vsUMap<StringIntKey,Unit*> UnitConstCache::unit_cache;
vsUMap<std::string , Mesh*> WeaponMeshCache::unit_cache;
#endif
#endif

#if 0
typedef vsUMap<keyval, Unit *> UnitCacheType;
static UnitCacheType unit_cache;
const Unit * getCachedConstUnit (std::string name, int faction) {
  keyval kv (name,faction);
  UnitCacheType::const_iterator i = unit_cache.find (kv);
  if (i!=unit_cache.end())
    return (*i).second;
  return NULL;
}
const Unit * setCachedConstUnit (std::string name, int faction, Unit * un) {
  unit_cache.insert (pair<keyval,Unit *> (keyval(name,faction),un));
  return un;
}
void purgeCache( ) {
    UnitCacheType::iterator i=unit_cache.begin();
    for (;i!=unit_cache.end();++i) {
      (*i).second->Kill();
    }
    unit_cache.clear();
}
#endif
