#ifndef __UNIT_INTERFACE_H__
#define __UNIT_INTERFACE_H__
#include "gui/text_area.h"
#include "gui/button.h"
#include "main_loop.h"
#include "cmd/script/mission.h"
#include "images.h"

bool RefreshInterface ();

extern void LoadMission (const char *, bool loadfirst);
extern void SwitchUnits (Unit * ol, Unit * nw);
extern Cargo * GetMasterPartList(const char *input_buffer);
extern Unit&GetUnitMasterPartList();
class UpgradingInfo;
void BuyShip(Unit*base, Unit*un, std::string name, bool my_fleet=false, UnitContainer * buyer=NULL, UpgradingInfo *baseinfo=NULL);
class UpgradingInfo {
  void DoDone();
  friend void BuyShip(Unit*,Unit*,std::string,bool,UnitContainer*,UpgradingInfo*);
public:
  bool readnews;
  bool drawovermouse;
  Mission * briefingMission;//do not dereference! instead scan through activve_missions
  TextArea *CargoList, *CargoInfo;
  Button *OK, *COMMIT;
  UnitContainer base;
  UnitContainer buyer;
  //below are state variables while the user is selecting mounts
  const Unit * NewPart;
  const Unit * templ;
  const Unit * downgradelimiter;
  Cargo part;
  int selectedmount;
  int selectedturret;
  //end it
  struct LastSelected{int type; float x; float y; int button; int state;bool last;LastSelected() {last=false;}} lastselected;
  void ProcessMouse(int type, int x, int y, int button, int state);

  vector <CargoColor> TempCargo;//used to store cargo list
  vector <CargoColor> * CurrentList;
  enum SubMode {NORMAL,MOUNT_MODE,SUBUNIT_MODE, CONFIRM_MODE, STOP_MODE}submode;
  enum BaseMode {BUYMODE,SELLMODE,MISSIONMODE,BRIEFINGMODE,NEWSMODE,SHIPDEALERMODE,UPGRADEMODE,ADDMODE,DOWNGRADEMODE, SAVEMODE, MAXMODE} mode;
  bool multiplicitive;
  Button **Modes;
  vector <BaseMode> availmodes;
  string title;
  vector <string> curcategory;
  vector <CargoColor>&FilterCargo(Unit *un, const string filterthis, bool inv, bool removezero);
  vector <CargoColor>&GetCargoFor(Unit *un);
  vector <CargoColor>&GetCargoList ();
  vector <CargoColor>&MakeActiveMissionCargo();
  vector <CargoColor>&MakeMissionsFromSavegame(Unit *un);
  void StopBriefing();
  void SetupCargoList();
  bool beginswith (const vector <std::string> &cat, const std::string &s);
  void SetMode (enum BaseMode mod, enum SubMode smod);
  UpgradingInfo(Unit * un, Unit * base, vector<BaseMode> modes);
  ~UpgradingInfo();
  void Render();
  static void ProcessMouseClick(int button, int state, int x, int y);
  static void ProcessMouseActive(int x, int y);
  static void ProcessMousePassive(int x, int y);
  void SelectLastSelected();
  bool SelectItem (const char * str, int button, int state);
  void CommitItem (const char * str, int button, int state);
  //this function is called after the mount is selected and stored in selected mount
  void CompleteTransactionAfterMountSelect();
  //this function is called after the turret is selected and stored in selected turret
  void CompleteTransactionAfterTurretSelect();
  void CompleteTransactionConfirm();
};

void UpgradeCompInterface(Unit *un,Unit * base, std::vector <UpgradingInfo::BaseMode> modes);

#endif
