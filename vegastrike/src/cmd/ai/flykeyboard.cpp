#include "in_joystick.h"
#include "flykeyboard.h"
#include "cmd/unit_generic.h"
#include "navigation.h"
#include "config_xml.h"
#include "xml_support.h"
#include "vs_globals.h"
#include "gfx/cockpit.h" 
#include "networking/netclient.h"
#include "lin_time.h"

using std::string;
using std::vector;

struct StarShipControlKeyboard {
  bool switchmode;
  bool setunvel;
  bool setnulvel;
  bool matchspeed;
  bool jumpkey;
  signed char axial;
  signed char vertical;
  signed char horizontal;
  int sheltonpress;
  int sheltonrelease;
  int uppress;//negative if not pressed last
  int uprelease;
  int downpress;
  int downrelease;
  int leftpress;
  int leftrelease;
  int rightpress;
  int rightrelease;
  int ABpress;
  int ABrelease;
  int accelpress;
  int accelrelease;
  int decelpress;
  int decelrelease;
  int rollrightpress;
  int rollrightrelease;
  int rollleftpress;
  int rollleftrelease;
  int joyinertialxypress;
  int joyinertialxyrelease;
  int joyinertialxzpress;
  int joyinertialxzrelease;
  int joyrollpress;
  int joyrollrelease;
  int joybankpress;
  int joybankrelease;
  int inertialflightpress;
  int inertialflightrelease;
  bool stoppress;
  bool startpress;
  bool dirty;//it wasn't updated...
  bool autopilot;
  bool switch_combat_mode;
  bool ASAP;
  bool terminateauto;
  bool realauto;
  bool startcomm;
  bool commchanged;
  bool switchwebcam;
  bool switchsecured;
  bool freq_increase;
  bool freq_decrease;
  bool switchinertialflight;
  bool switchjoyinertialxy;
  bool switchjoyinertialxz;
  bool switchjoyroll;
  bool switchjoybank;
  void UnDirty() {
      sheltonpress=sheltonrelease=uppress=uprelease=downpress=downrelease=
          leftpress=leftrelease=rightpress=rightrelease=ABpress=ABrelease=
          accelpress=accelrelease=decelpress=decelrelease=rollrightpress=rollrightrelease=
          rollleftpress=rollleftrelease=joyinertialxypress=joyinertialxyrelease=
          joyinertialxzpress=joyinertialxzrelease=joyrollpress=joyrollrelease=
          joybankpress=joybankrelease=inertialflightpress=inertialflightrelease=0;
      jumpkey=startpress=stoppress=autopilot=dirty=switch_combat_mode=terminateauto=
          setunvel=switchmode=setnulvel=realauto=matchspeed=ASAP=switchinertialflight=
          commchanged=startcomm=switchwebcam=switchsecured=freq_increase=freq_decrease=
          switchjoyinertialxy=switchjoyinertialxz=switchjoyroll=switchjoybank=false;
      axial=vertical=horizontal=0;
  }
  StarShipControlKeyboard() {UnDirty();}
};
static vector <StarShipControlKeyboard> starshipcontrolkeys;
static StarShipControlKeyboard &g() {
  while (starshipcontrolkeys.size()<=(unsigned int)_Universe->CurrentCockpit()) {
    starshipcontrolkeys.push_back(StarShipControlKeyboard());
  }
  return starshipcontrolkeys [_Universe->CurrentCockpit()];
}

extern void JoyStickToggleDisable();
FlyByKeyboard::FlyByKeyboard (unsigned int whichplayer): FlyByWire (),axis_key(0,0,0) {
  this->last_jumped=0;
  this->whichplayer=whichplayer;
  while(starshipcontrolkeys.size()<=whichplayer) {
    starshipcontrolkeys.push_back (StarShipControlKeyboard());
  }
  autopilot=NULL;
  inauto=false;

  //Initial Joystick Mode
  //  NOTE: Perhaps it should be handled by FlyByJoystick, but it was cumbersome to do that
  //    since it handled mainly keystrokes - Any ideas?
  static string initialJoyMode = vs_config->getVariable ("joystick","initial_mode","normal" );
  joy_mode = 0;
  if (initialJoyMode=="inertialxy")
      joy_mode = joyModeInertialXY; else if (initialJoyMode=="inertialxz")
      joy_mode = joyModeInertialXZ; else if (initialJoyMode=="roll")
      joy_mode = joyModeRoll; else if (initialJoyMode=="bank")
      joy_mode = joyModeBank;

}
float FlyByKeyboard::clamp_axis (float v) {
  static int axis_scale =XMLSupport::parse_int (vs_config->getVariable ("physics","slide_start","3"));
  int as  = parent->GetComputerData().slide_start;
  if (as==0) {
    as = axis_scale;
  }
  if (as) {
	v/=as;
	if (v>1){
	 return 1;
	}
	if (v<-1) {
		return -1;
	}
  }
  return v;
}
float FlyByKeyboard::reduce_axis (float v) {
  static int axis_scale =XMLSupport::parse_int (vs_config->getVariable ("physics","slide_end","2"));
  int as  = parent->GetComputerData().slide_end;
  if (as==0) {
    as = axis_scale;
  }
  if (as&&fabs(v)>as) {
    if (v>0) {
      v-=as;
    }else {
      v+=as;
    }
  } else {
    v=0;
  }
  return v;
}
void FlyByKeyboard::Destroy() {
  if (autopilot)
    autopilot->Destroy();
  Order::Destroy();
}
FlyByKeyboard::~FlyByKeyboard() {
}
void FlyByKeyboard::KeyboardUp (float v) {
  if (v==0) {
    axis_key.i=reduce_axis(axis_key.i);
  }else {    
    if ((v>0)==(axis_key.i>=0)) {
      axis_key.i+=v;
    }else {
      axis_key.i=v;
    }
  }
  Up(clamp_axis (axis_key.i));
}
void FlyByKeyboard::KeyboardRight (float v) {
  if (v==0) {
    axis_key.j=reduce_axis(axis_key.j);
  }else {
    if ((v>0)==(axis_key.j>=0)) {
      axis_key.j+=v;
    }else {
      axis_key.j=v;
    }
  }
  Right(clamp_axis (axis_key.j));

}
void FlyByKeyboard::KeyboardRollRight (float v) {
  if (v==0) {
    axis_key.k=reduce_axis (axis_key.k);
  } else {
    if ((v>0)==(axis_key.k>=0)) {
      axis_key.k+=v;
    }else {
      axis_key.k=v;
    }
  }
  RollRight(clamp_axis (axis_key.k));
}
#define FBWABS(m) (m>=0?m:-m)
void FlyByKeyboard::Execute () {
  FlyByKeyboard::Execute (true);
}

void FlyByKeyboard::Execute (bool resetangvelocity) {
#define SSCK starshipcontrolkeys[whichplayer]
  if(Network!=NULL && !SSCK.startcomm && SSCK.commchanged && whichplayer==0) {
	printf( "Stopping a NETCOMM\n");
	Network[whichplayer].stopCommunication();
	SSCK.commchanged=false;
  }
  if(Network!=NULL && SSCK.startcomm && SSCK.commchanged && whichplayer==0) {
	printf( "Starting a NETCOMM\n");
	Network[whichplayer].startCommunication();
	SSCK.commchanged=false;
  }
  if( Network!=NULL && SSCK.freq_increase)
  {
  	SSCK.freq_increase = false;
	Network[whichplayer].increaseFrequency();
  }
  if( Network!=NULL && SSCK.freq_decrease)
  {
  	SSCK.freq_decrease = false;
	Network[whichplayer].decreaseFrequency();
  }
  if( Network!=NULL && SSCK.switchwebcam)
  {
    SSCK.switchwebcam=false;
	Network[whichplayer].switchWebcam();
  }
  if( Network!=NULL && SSCK.switchsecured)
  {
    SSCK.switchsecured=false;
	Network[whichplayer].switchSecured();
  }
  if (SSCK.setunvel) {
    SSCK.setunvel=false;
    Unit * t=parent->Target();
    int neu=FactionUtil::GetNeutralFaction();
    int upg=FactionUtil::GetUpgradeFaction();   
    static bool allowanyreference=XMLSupport::parse_bool(vs_config->getVariable("AI","AllowAnySpeedReference","false"));
    static bool onlyupgraderef=XMLSupport::parse_bool(vs_config->getVariable("AI","OnlyUpgradeSpeedReference","false"));
    if (t)
      if ((t->getRelation(parent)>=0&&!onlyupgraderef)||t->faction==neu||t->faction==upg||allowanyreference)
        parent->VelocityReference (parent->Target());
  }
  if (SSCK.setnulvel) {
    SSCK.setnulvel=false;
    parent->VelocityReference( NULL);
  }
  if (SSCK.switch_combat_mode) {
    SSCK.switch_combat_mode=false;
    parent->SwitchCombatFlightMode();
  }
  if (SSCK.ASAP) {
    SSCK.ASAP=false;
	if(FlyByKeyboard::inauto){
		this->eraseType(FACING|MOVEMENT);
		FlyByKeyboard::inauto=false;
		parent->autopilotactive=FlyByKeyboard::inauto;
		if(parent->graphicOptions.InWarp){
		  parent->graphicOptions.InWarp=0;
		  parent->graphicOptions.WarpRamping=1;
		}
	} else {
		Orders::AutoLongHaul* temp = new Orders::AutoLongHaul();
		temp->SetParent(parent);
		Order::EnqueueOrderFirst(temp);
		FlyByKeyboard::inauto=true;
		parent->autopilotactive=FlyByKeyboard::inauto;
    }
  }
  bool enteredautopilot=false;
  if (SSCK.realauto) {
    Cockpit * cp = _Universe->isPlayerStarship(parent);
    if (cp) {
      cp->Autopilot (parent->Target());
    }
    enteredautopilot=true;
    SSCK.realauto=false;
  }
  if (autopilot) {
    autopilot->Execute();
  }
  if (resetangvelocity)
    desired_ang_velocity=Vector(0,0,0);


  //  printf("flybykey::exec\n");
  static bool initial_inertial_mode = XMLSupport::parse_bool( vs_config->getVariable("flight","inertial::initial","false") );
  if (SSCK.dirty) {
    //go with what's last there: no frames since last physics frame
    if (SSCK.uppress<=0&&SSCK.downpress<=0)
      KeyboardUp(0);
    else {
      if (SSCK.uppress>0)
	KeyboardUp(1);
      if (SSCK.downpress>0)
	KeyboardUp(-1);
    }
    if (SSCK.leftpress<=0&&SSCK.rightpress<=0)
      KeyboardUp(0);
    else {
      if (SSCK.rightpress>0)
	KeyboardRight(1);
      if (SSCK.leftpress>0)
	KeyboardRight(-1);
    }
    if (SSCK.rollrightpress<=0&&SSCK.rollleftpress<=0)
      KeyboardRollRight(0);
    else {
      if (SSCK.rollrightpress>0)
	KeyboardRollRight(1);
      if (SSCK.rollleftpress>0)
	KeyboardRollRight(-1);
    }

    //    VSFileSystem::vs_fprintf(stderr,"AB: press %d rel %d\n",SSCK.ABpress,SSCK.ABrelease);
    if (SSCK.ABpress>=1)
      Afterburn(1);
    else
      Afterburn (0);

    if (SSCK.joyinertialxypress>0)
        joy_mode |= joyModeInertialXY;
    if (SSCK.joyinertialxzpress>0)
        joy_mode |= joyModeInertialXZ;
    if (SSCK.joyrollpress>0)
        joy_mode |= joyModeRoll;
    if (SSCK.joybankpress>0)
        joy_mode |= joyModeBank;
    if (SSCK.inertialflightpress>0)
        InertialFlight(!initial_inertial_mode);

    if (SSCK.accelpress>0)
      Accel(1);
    if (SSCK.decelpress>0)
      Accel(-1);
  }else {
    if (SSCK.uppress==0&&SSCK.downpress==0)
      KeyboardUp(0);
    else {

      if (SSCK.uppress!=0&&SSCK.downpress==0)
	KeyboardUp(((float)FBWABS(SSCK.uppress))/(FBWABS(SSCK.uppress)+SSCK.uprelease));
      else {
	if (SSCK.downpress!=0&&SSCK.uppress==0)
	  KeyboardUp(-((float)FBWABS(SSCK.downpress))/(FBWABS(SSCK.downpress)+SSCK.downrelease));
        else {
	  KeyboardUp(((float)FBWABS(SSCK.uppress)-(float)FBWABS(SSCK.downpress))/(FBWABS(SSCK.downpress)+SSCK.downrelease+FBWABS(SSCK.uppress)+SSCK.uprelease));
	}
      }
    }
    if (SSCK.rightpress==0&&SSCK.leftpress==0)
      KeyboardRight(0);
    else {
      if (SSCK.rightpress!=0&&SSCK.leftpress==0)
	KeyboardRight(((float)FBWABS(SSCK.rightpress))/(FBWABS(SSCK.rightpress)+SSCK.rightrelease));
      else {
	if (SSCK.leftpress!=0&&SSCK.rightpress==0)
	  KeyboardRight(-((float)FBWABS(SSCK.leftpress))/(FBWABS(SSCK.leftpress)+SSCK.leftrelease));
        else {
	  KeyboardRight(((float)FBWABS(SSCK.rightpress)-(float)FBWABS(SSCK.leftpress))/(FBWABS(SSCK.leftpress)+SSCK.leftrelease+FBWABS(SSCK.rightpress)+SSCK.rightrelease));
	}
      }
    }
    if (SSCK.rollrightpress==0&&SSCK.rollleftpress==0)
      KeyboardRollRight(0);
    else {
      if (SSCK.rollrightpress!=0&&SSCK.rollleftpress==0)
	KeyboardRollRight(((float)FBWABS(SSCK.rollrightpress))/(FBWABS(SSCK.rollrightpress)+SSCK.rollrightrelease));
      else {
	if (SSCK.rollleftpress!=0&&SSCK.rollrightpress==0)
	  KeyboardRollRight(-((float)FBWABS(SSCK.rollleftpress))/(FBWABS(SSCK.rollleftpress)+SSCK.rollleftrelease));
        else {
	  KeyboardRollRight(((float)FBWABS(SSCK.rollrightpress)-(float)FBWABS(SSCK.rollleftpress))/(FBWABS(SSCK.rollleftpress)+SSCK.rollleftrelease+FBWABS(SSCK.rollrightpress)+SSCK.rollrightrelease));
	}
      }
    }
    if (SSCK.accelpress!=0) {
      Accel (((float)FBWABS(SSCK.accelpress))/(FBWABS(SSCK.accelpress)+SSCK.accelrelease));
    }
    if (SSCK.decelpress!=0) {
      Accel (-((float)FBWABS(SSCK.decelpress))/(FBWABS(SSCK.decelpress)+SSCK.decelrelease));
    }
    if (SSCK.ABpress||SSCK.ABrelease) {
      //VSFileSystem::vs_fprintf(stderr,"AB: press %d rel %d\n",SSCK.ABpress,SSCK.ABrelease);
      Afterburn ((SSCK.ABpress>=1)?1:0);
    }
    if (SSCK.joyinertialxypress||SSCK.joyinertialxyrelease) {
        if (SSCK.joyinertialxypress>0)
            joy_mode |= joyModeInertialXY; else
            joy_mode &= ~joyModeInertialXY;
    }
    if (SSCK.joyinertialxzpress||SSCK.joyinertialxzrelease) {
        if (SSCK.joyinertialxzpress>0)
            joy_mode |= joyModeInertialXZ; else
            joy_mode &= ~joyModeInertialXZ;
    }
    if (SSCK.joyrollpress||SSCK.joyrollrelease) {
        if (SSCK.joyrollpress>0)
            joy_mode |= joyModeRoll; else
            joy_mode &= ~joyModeRoll;
    }
    if (SSCK.joybankpress||SSCK.joybankrelease) {
        if (SSCK.joybankpress>0)
            joy_mode |= joyModeBank; else
            joy_mode &= ~joyModeBank;
    }
    if (SSCK.inertialflightpress||SSCK.inertialflightrelease) {
        if (SSCK.inertialflightpress>0)
            InertialFlight(!initial_inertial_mode); else
            InertialFlight(initial_inertial_mode);
    }
  }
  if (SSCK.stoppress) {
    Stop(0);
  }
  if (SSCK.startpress||enteredautopilot) {
    Stop(1);
  }
  if (SSCK.sheltonpress>0) {
    SheltonSlide(true);
  } else {
    SheltonSlide(false);
  }
  if (SSCK.switchinertialflight) {
    InertialFlight(!InertialFlight());
    SSCK.switchinertialflight = false;
  }
  if (SSCK.switchjoyinertialxy) {
    joy_mode ^= joyModeInertialXY;
    SSCK.switchjoyinertialxy = false;
  }
  if (SSCK.switchjoyinertialxz) {
    joy_mode ^= joyModeInertialXZ;
    SSCK.switchjoyinertialxz = false;
  }
  if (SSCK.switchjoyroll) {
    joy_mode ^= joyModeRoll;
    SSCK.switchjoyroll = false;
  }
  if (SSCK.switchjoybank) {
    joy_mode ^= joyModeBank;
    SSCK.switchjoybank = false;
  }
  if (SSCK.switchmode) {
    FlyByWire::SwitchFlightMode();
  }
  if (SSCK.vertical) {
    FlyByWire::DirectThrustUp (SSCK.vertical);
  }
  if (SSCK.horizontal)
    FlyByWire::DirectThrustRight(SSCK.horizontal);
  if (SSCK.axial)
    FlyByWire::DirectThrustFront(SSCK.axial);
  if (SSCK.autopilot&&!autopilot) {
    autopilot = new Orders::FaceTarget (false,1);
    autopilot->SetParent (parent);
    SSCK.autopilot=false;
  }
  if (SSCK.autopilot||SSCK.terminateauto) {
    if (autopilot) {
      autopilot->Destroy();
      autopilot=NULL;
    }
    SSCK.autopilot=false;
    SSCK.terminateauto=false;
  }
  if (SSCK.matchspeed) {
    SSCK.matchspeed=false;
    Unit * targ = parent->Target();
    if (targ)
      MatchSpeed (targ->GetVelocity());
  }
  static unsigned int counter=0;
  counter++;
  if (SSCK.jumpkey) {
    static float jump_key_delay=XMLSupport::parse_float(vs_config->getVariable("general","jump_key_delay",".125"));
    if (counter-last_jumped>(int)(jump_key_delay/SIMULATION_ATOM)||last_jumped==0) {
      last_jumped=counter;
      parent->ActivateJumpDrive();
      if (parent->GetJumpStatus().drive>=0) {
        static soundContainer foobar;
        if (foobar.sound==-2) {
          static string str=vs_config->getVariable("cockpitaudio","jump_engaged","jump");
          foobar.loadsound(str);
        }
        foobar.playsound();
      }
    }
    SSCK.jumpkey=false;
  }else {
    parent->DeactivateJumpDrive();
  }
  SSCK.dirty=true;
#undef SSCK

  if(queryType(FACING|MOVEMENT)){
	  Order::Execute();
          if (queryType(FACING|MOVEMENT)==NULL) 
            FlyByKeyboard::inauto=false;
          done=false;
  } else {
	  FlyByWire::Execute();
  }

}

// Changing the frequency doesn't kill a communication anymore until the player stopped its current one
// and starts a new one in that other frequency

void FlyByKeyboard::DownFreq (const KBData&,KBSTATE k) {
if(Network!=NULL)
{
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:
	g().freq_decrease = true;
  break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}
}

void FlyByKeyboard::UpFreq (const KBData&,KBSTATE k) {
if(Network!=NULL)
{
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:
	g().freq_increase = true;
  break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}
}

void FlyByKeyboard::ChangeCommStatus (const KBData&,KBSTATE k) {
if(Network!=NULL)
{
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:
  case UP:
  break;
  case PRESS:
	printf( "Pressed NETCOMM key !!!\n");
	if(g().startcomm==true)
		g().startcomm=false;
	else
		g().startcomm=true;
	g().commchanged=true;
  break;
  case RELEASE:
  case RESET:
    break;
  }
}
}

void FlyByKeyboard::SwitchWebcam(const KBData&,KBSTATE k)
{
if(Network!=NULL)
{
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:
  case UP:
  break;
  case PRESS:
	printf( "Pressed SWITCHWEBCAM key !!!\n");
	g().switchwebcam=true;
  break;
  case RELEASE:
  case RESET:
    break;
  }
}
}

void FlyByKeyboard::SwitchSecured(const KBData&,KBSTATE k)
{
if(Network!=NULL)
{
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:
  case UP:
  break;
  case PRESS:
	printf( "Pressed SWITCHSECURED key !!!\n");
	g().switchsecured=true;
  break;
  case RELEASE:
  case RESET:
    break;
  }
}
}

void FlyByKeyboard::SetVelocityRefKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:
    break;
  case DOWN:g().setunvel=true;
    break;
  default:break;
  }
}
void FlyByKeyboard::SetNullVelocityRefKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:
    break;
  case DOWN:g().setnulvel=true;
    break;
  default:break;
  }
}


void FlyByKeyboard::SheltonKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:
    g().sheltonrelease=FBWABS(g().sheltonrelease)+1;
    break;
  case DOWN:g().sheltonpress=FBWABS(g().sheltonpress)+1;
    break;
  default:break;
  }
}

void FlyByKeyboard::InertialToggleKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case PRESS: g().switchinertialflight=true; break;
  default:break;
  }
}

void FlyByKeyboard::InertialPulsorKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:  g().inertialflightrelease=FBWABS(g().inertialflightrelease)+1; break;
  case DOWN:g().inertialflightpress=FBWABS(g().inertialflightpress)+1; break;
  default:break;
  }
}

void FlyByKeyboard::JoyInertialXYToggleKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case PRESS: g().switchjoyinertialxy=true; break;
  default:break;
  }
}

void FlyByKeyboard::JoyInertialXZToggleKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case PRESS: g().switchjoyinertialxz=true; break;
  default:break;
  }
}

void FlyByKeyboard::JoyRollToggleKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case PRESS: g().switchjoyroll=true; break;
  default:break;
  }
}

void FlyByKeyboard::JoyBankToggleKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case PRESS: g().switchjoybank=true; break;
  default:break;
  }
}

void FlyByKeyboard::JoyInertialXYPulsorKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:  g().joyinertialxyrelease=FBWABS(g().joyinertialxyrelease)+1; break;
  case DOWN:g().joyinertialxypress=FBWABS(g().joyinertialxypress)+1; break;
  default:break;
  }
}

void FlyByKeyboard::JoyInertialXZPulsorKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:  g().joyinertialxzrelease=FBWABS(g().joyinertialxzrelease)+1; break;
  case DOWN:g().joyinertialxzpress=FBWABS(g().joyinertialxzpress)+1; break;
  default:break;
  }
}

void FlyByKeyboard::JoyRollPulsorKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:  g().joyrollrelease=FBWABS(g().joyrollrelease)+1; break;
  case DOWN:g().joyrollpress=FBWABS(g().joyrollpress)+1; break;
  default:break;
  }
}

void FlyByKeyboard::JoyBankPulsorKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:  g().joybankrelease=FBWABS(g().joybankrelease)+1; break;
  case DOWN:g().joybankpress=FBWABS(g().joybankpress)+1; break;
  default:break;
  }
}

void FlyByKeyboard::JumpKey(const KBData&,KBSTATE k) {
  switch (k) {
  case PRESS:
    g().jumpkey = (_Universe->AccessCockpit()->GetParent() && !_Universe->AccessCockpit()->GetParent()->isSubUnit());
    break;
  case UP:
  case RELEASE:
    g().jumpkey = false;
    break;
  default:
    break;
  }
}
void FlyByKeyboard::UpKey(const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP: g().uprelease++;
    break;
  case DOWN:g().uppress=FBWABS(g().uppress)+1;
    break;
  case PRESS: g().uppress=FBWABS(g().uppress);
    JoyStickToggleDisable();
    break;
  case RELEASE: g().uppress=-FBWABS(g().uppress);
    break;
  default:break;
  }
}
void FlyByKeyboard::KThrustRight (const KBData&,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:g().horizontal-=1;
    break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}
void FlyByKeyboard::KThrustLeft (const KBData&,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:g().horizontal+=1;
    break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}
void FlyByKeyboard::KThrustUp (const KBData&,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:g().vertical+=1;
    break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}
void FlyByKeyboard::KThrustDown (const KBData&,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:g().vertical-=1;
    break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}
void FlyByKeyboard::KThrustFront(const KBData&,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:g().axial+=1;
    break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}
void FlyByKeyboard::KThrustBack (const KBData&,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case DOWN:g().axial-=1;
    break;
  case UP:
  case PRESS:
  case RELEASE:
  case RESET:
    break;
  }
}

void FlyByKeyboard::DownKey (const KBData&,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case UP: g().downrelease++;
    break;
  case DOWN:g().downpress=FBWABS(g().downpress)+1;
    break;
  case PRESS: g().downpress=FBWABS(g().downpress);
    JoyStickToggleDisable();
    break;
  case RELEASE: g().downpress=-FBWABS(g().downpress);
    break;
  default:break;
  }
}


void FlyByKeyboard::LeftKey (const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP: g().leftrelease++;
    break;
  case DOWN:g().leftpress=FBWABS(g().leftpress)+1;
    break;
  case PRESS: g().leftpress=FBWABS(g().leftpress);
    JoyStickToggleDisable();
    break;
  case RELEASE: g().leftpress=-FBWABS(g().leftpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::KSwitchFlightMode (const KBData&,KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case PRESS:
    g().switchmode=true;
    break;
  default:
    break;
  }
}
void FlyByKeyboard::RightKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().rightrelease++;
    break;
  case DOWN:g().rightpress=FBWABS(g().rightpress)+1;
    break;
  case PRESS: g().rightpress=FBWABS(g().rightpress);
    JoyStickToggleDisable();
    break;
  case RELEASE: g().rightpress=-FBWABS(g().rightpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::ABKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().ABrelease++;
    break;
  case DOWN:g().ABpress=FBWABS(g().ABpress)+1;
    break;
  case PRESS: g().ABpress=FBWABS(g().ABpress);
    break;
  case RELEASE: g().ABpress=-FBWABS(g().ABpress);
    break;
  default:break;
  }
}

void FlyByKeyboard::AutoKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  if (k==PRESS) {
    g().realauto=true;
  }
}

void FlyByKeyboard::EngageSpecAuto (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  if (k==PRESS) {
    g().ASAP=true;
  }
}

void FlyByKeyboard::SwitchCombatModeKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  if (k==PRESS) {
    g().switch_combat_mode=true;
  }
}
void FlyByKeyboard::StopAutoKey (const KBData&,KBSTATE k) {
  
  if (g().dirty)  g().UnDirty();
  if (k==PRESS) {
    g().autopilot=true;
  }
}

void FlyByKeyboard::StopKey (const KBData&,KBSTATE k)  {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().stoppress=false;
    break;
  case DOWN:g().stoppress=true;
    break;
  default:break;
  }
}
void FlyByKeyboard::AccelKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().accelrelease++;
    break;
  case DOWN:g().accelpress=FBWABS(g().accelpress)+1;
    break;
  case PRESS: g().accelpress=FBWABS(g().accelpress);
    break;
  case RELEASE: g().accelpress=-FBWABS(g().accelpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::DecelKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().decelrelease++;
    break;
  case DOWN:g().decelpress=FBWABS(g().decelpress)+1;
    break;
  case PRESS: g().decelpress=FBWABS(g().decelpress);
    break;
  case RELEASE: g().decelpress=-FBWABS(g().decelpress);
    break;
  default:break;
  }
}


void FlyByKeyboard::StartKey (const KBData&,KBSTATE k)  {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().startpress=false;
    break;
  case DOWN:g().startpress=true;
    break;
  default:break;
  }
}




void FlyByKeyboard::RollRightKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().rollrightrelease++;
    break;
  case DOWN:g().rollrightpress=FBWABS(g().rollrightpress)+1;
    break;
  case PRESS: g().rollrightpress=FBWABS(g().rollrightpress);
    break;
  case RELEASE: g().rollrightpress=-FBWABS(g().rollrightpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::MatchSpeedKey (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    if (g().dirty)  g().UnDirty();
    g().matchspeed=true;
  }
}
void FlyByKeyboard::RollLeftKey (const KBData&,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().rollleftrelease++;
    break;
  case DOWN:g().rollleftpress=FBWABS(g().rollleftpress)+1;
    break;
  case PRESS: g().rollleftpress=FBWABS(g().rollleftpress);
    break;
  case RELEASE: g().rollleftpress=-FBWABS(g().rollleftpress);
    break;
  default:break;
  }
}

