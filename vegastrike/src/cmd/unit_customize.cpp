#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
//#include "unit.h"
//#include "unit_template.h"
#include "unit_factory.h"
#include "images.h"
#include "universe.h"
#include "vegastrike.h"
#include <stdio.h>
#include <stdlib.h>
#include "gfx/cockpit.h"
#include "savegame.h"
#include "config_xml.h"
#include "xml_serializer.h"
#include "audiolib.h"
#include "vs_globals.h"
#include "unit_const_cache.h"
#if !defined(HAVE_STRCASECMP) && defined(HAVE_STRICMP)
# define strcasecmp(s1,s2) stricmp(s1,s2)
#endif
#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
# define strncasecmp(s1,s2,n) strnicmp(s1,s2,n)
#endif
extern int GetModeFromName (const char *);

extern Unit * CreateGameTurret (const std::string & tur,int faction);

template <class UnitType>
bool GameUnit<UnitType>::UpgradeSubUnits (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)  {
  bool bl = this->UpgradeSubUnitsWithFactory( up, subunitoffset, touchme, downgrade, numave, percentage,&CreateGameTurret);
  if (!up->SubUnits.empty())
    this->DisableTurretAI();
  return bl;
}
extern char * GetUnitDir (const char *);
template <class UnitType>
double GameUnit<UnitType>::Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts) {
  return Unit::Upgrade(file,mountoffset,subunitoffset,force,loop_through_mounts);
}
