#include "config.h"
#include <stdlib.h>
#include <set>
#include "configxml.h"
#include "audiolib.h"
#include "unit_generic.h"
#include "beam.h"
#include "unit_collide.h"
#include "lin_time.h"
#include "xml_serializer.h"
#include "vsfilesystem.h"
#include "file_main.h"
#include "universe_util.h"
#include "unit_util.h"
#include "script/mission.h"
#include "script/flightgroup.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/turretai.h"
#include "cmd/ai/communication.h"
#include "cmd/ai/navigation.h"
#include "cmd/ai/script.h"
#include "cmd/ai/missionscript.h"
#include "cmd/ai/flybywire.h"
#include "cmd/ai/aggressive.h"
#include "python/python_class.h"
#include "cmd/unit_factory.h"
#include "missile_generic.h"
#include "gfx/cockpit_generic.h"
#include "gfx/vsbox.h"
#include <algorithm>
#include "cmd/ai/ikarus.h"
#include "role_bitmask.h"
#include "unit_const_cache.h"
#include "gfx/warptrail.h"
#include "networking/netserver.h"
#include "networking/netclient.h"
#include "gfx/cockpit_generic.h"
#include "universe_generic.h"
#include "unit_bsp.h"
#include "gfx/bounding_box.h"
#include "csv.h"
#include "vs_random.h"
#include "galaxy_xml.h"
#include "gfx/camera.h"
#include "log.h"
#include "vs_log_modules.h"
#include "options.h"


using std::string;
using std::vector;

#if !defined(HAVE_STRCASECMP) && defined(HAVE_STRICMP)
# define strcasecmp(s1,s2) stricmp(s1,s2)
#endif
#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
# define strncasecmp(s1,s2,n) strnicmp(s1,s2,n)
#endif

#include "unit_find.h"
#include "pilot.h"
//cannot seem to get min and max working properly across win and lin without using namespace std
static float mymax (float a, float b) {return a<b?b:a;}
static float mymin (float a, float b) {return a<b?a:b;}

using namespace Orders;
extern void DestroyMount(Mount*);

extern vs_options game_options;

void Mount::SetMountPosition(const Vector &v)
{
	pos = v;
}


void Mount::SetMountOrientation(const Quaternion &t)
{
	orient = t;
}


void Unit::SetNetworkMode( bool mode)
{
	networked = mode;
}


void Unit::SetSerial(ObjSerial s)
{
	serial = s;
}


Unit::graphic_options::graphic_options()
{
	FaceCamera=Animating=missilelock=InWarp=unused1=WarpRamping=NoDamageParticles=0;
	specInterdictionOnline=1;
	NumAnimationPoints=0;
	RampCounter=0;
	MinWarpMultiplier=MaxWarpMultiplier=1;
}


void Unit::setFaceCamera()
{
	graphicOptions.FaceCamera=1;
}


void Unit::attackPreference(unsigned char c)
{
	attack_preference=c;
}


void Unit::unitRole(unsigned char c)
{
	unit_role=c;
}


void Unit::SetNebula(Nebula *neb)
{
	nebula = neb;
	if (!SubUnits.empty()) {
		un_fiter iter =SubUnits.fastIterator();
		Unit * un;
		while ((un = *iter) != NULL) {
			un->SetNebula(neb);
			++iter;
		}
	}
}


bool Unit::InCorrectStarSystem (StarSystem *active)
{
	return(active==activeStarSystem);
}


bool Unit::TransferUnitToSystem (unsigned int whichJumpQueue,class StarSystem *&previouslyActiveStarSystem, bool DoSightAndSound)
{
	return(false);
}


Unit::Computer& Unit::GetComputerData ()
{
	return(computer);
}


bool Unit::AutoPilotTo(Unit * un, bool automaticenergyrealloc)
{
	std::string tmp;
	return(AutoPilotToErrorMessage(un, automaticenergyrealloc,tmp));
}


void Unit::SetAfterBurn( float aft)
{
	afterburnenergy = aft;
}


void Unit::SetFuel( float f)
{
	fuel = f;
}


void Unit::SetEnergyRecharge( float enrech)
{
	recharge = enrech;
}


void Unit::SetMaxEnergy( float maxen)
{
	maxenergy = maxen;
}
Vector  Unit::GetWarpVelocity()const {
  Vector VelocityRef(0,0,0);
  {
      Unit * vr=const_cast<UnitContainer*>(&computer.velocity_ref)->GetUnit();
      if (vr)
          VelocityRef=vr->cumulative_velocity;
  }

  //return(cumulative_velocity*graphicOptions.WarpFieldStrength);
  Vector vel=cumulative_velocity-VelocityRef;
  float speed=vel.Magnitude();
  //return vel*graphicOptions.WarpFieldStrength;
  if (speed>0) {
    Vector veldir=vel*(1./speed);
    Vector facing=cumulative_transformation_matrix.getR();
    float ang=facing.Dot(veldir);
    float warpfield=graphicOptions.WarpFieldStrength;
    if (ang<0) warpfield=1./warpfield;
    return ang*facing*speed*(warpfield-1) + vel+VelocityRef;
  }else return VelocityRef;
}

void Unit::SetPosition(const QVector &pos)
{
	prev_physical_state.position = curr_physical_state.position = pos;
}


float Unit::DealDamageToHull (const Vector &pnt, float Damage)
{
	float * nullvar = NULL;		 //short fix
	return(DealDamageToHullReturnArmor( pnt, Damage, nullvar));
}


void Unit::GetOrientation(Vector &p, Vector &q, Vector &r) const
{
	Matrix m;
	curr_physical_state.to_matrix(m);
	p=m.getP();
	q=m.getQ();
	r=m.getR();
}


Vector Unit::GetNetAcceleration()
{
	Vector p, q, r;
	GetOrientation(p,q,r);
	Vector res(NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r );
	if (NetForce.i||NetForce.j||NetForce.k)
		res+=InvTransformNormal(identity_matrix,NetForce);
	return(res/GetMass());
}


float Unit::GetMaxAccelerationInDirectionOf(const Vector & ref, bool afterburn) const
{
	Vector p,q,r;
	GetOrientation(p,q,r);
	Vector lref(ref*p,ref*q,ref*r);
	float tp=(lref.i==0)?0:fabs(Limits().lateral/lref.i);
	float tq=(lref.j==0)?0:fabs(Limits().vertical/lref.j);
	float tr=(lref.k==0)?0:fabs(((lref.k>0)?Limits().forward:Limits().retro)/lref.k);
	float trqmin=(tr<tq)?tr:tq;
	float tm=tp<trqmin?tp:trqmin;
	return(lref.Magnitude()*tm/GetMass());
}


void Unit::SetVelocity (const Vector &v)
{
	Velocity = v;
}


void Unit::SetAngularVelocity (const Vector &v)
{
	AngularVelocity = v;
}


bool Unit::InRange (Unit *target, double & mm, bool cone, bool cap,bool lock) const
{
	if (this == target || target->CloakVisible() < .8)
		return(false);
	if (cone&&computer.radar.maxcone >- .98) {
		QVector delta( target->Position() - Position());
		mm = delta.Magnitude();
		if ((!lock) || (!(TargetLocked() && computer.target == target))) {
			double tempmm = mm - target->rSize();
			if (tempmm > 0.0001) {
				if ((ToLocalCoordinates (Vector(delta.i,delta.j,delta.k)).k/tempmm)<computer.radar.maxcone&&cone)
					return(false);
			}
		}
	}
	else {
		mm = (target->Position()-Position()).Magnitude();
	}
	//owner==target?!
	if (((mm-rSize() - target->rSize()) > computer.radar.maxrange) || target->rSize() < computer.radar.mintargetsize) {
		Flightgroup *fg = target->getFlightgroup();
		if ((target->rSize() < capship_size || (!cap)) && (fg == NULL?true:fg->name != "Base"))
			return(target->isUnit() == PLANETPTR);
	}
	return(true);
}


Unit* Unit::Target()
{
	return(computer.target.GetUnit());
}


Unit* Unit::VelocityReference()
{
	return(computer.velocity_ref.GetUnit());
}


Unit* Unit::Threat()
{
	return(computer.threat.GetUnit());
}


void Unit::RestoreGodliness()
{
	_Universe->AccessCockpit()->RestoreGodliness();
}


void Unit::Ref()
{
#ifdef CONTAINER_DEBUG
	CheckUnit(this);
#endif
	++ucref;
}


void    Unit::BackupState()
{
	this->old_state.setPosition( this->curr_physical_state.position);
	this->old_state.setOrientation( this->curr_physical_state.orientation);
	this->old_state.setVelocity( this->Velocity);
	this->old_state.setAcceleration( this->net_accel);
}


void Unit::BuildBSPTree(const char *filename, bool vplane, Mesh * hull)
{
	bsp_tree * bsp=NULL;
	bsp_tree temp_node;
	vector <bsp_polygon> tri;
	vector <bsp_tree> triplane;
	if (hull!=NULL) {
		hull->GetPolys (tri);
	}
	else {
		for (int j=0;j<nummesh();++j) {
			meshdata[j]->GetPolys(tri);
		}
	}
	for (unsigned int i=0;i<tri.size();++i) {
		if (!Cross (tri[i],temp_node)) {
			vector <bsp_polygon>::iterator ee = tri.begin();
			ee+=i;
			tri.erase(ee);
			i--;
			continue;
		}
		// Calculate 'd'
		temp_node.d = (double) ((temp_node.a*tri[i].v[0].i)+(temp_node.b*tri[i].v[0].j)+(temp_node.c*tri[i].v[0].k));
		temp_node.d*=-1.0f;
		triplane.push_back(temp_node);
		//                bsp=put_plane_in_tree3(bsp,&temp_node,&temp_poly3);
	}

	bsp = buildbsp (bsp,tri,triplane, vplane?VPLANE_ALL:0);
	if (bsp) {
		VSError err = fo.OpenCreateWrite( filename, BSPFile);
		if (err<=Ok) {
			write_bsp_tree(bsp,0);
			fo.Close();
			bsp_stats (bsp);
			FreeBSP (&bsp);
		}
	}

}


bool isMissile(const weapon_info *weap)
{
	static bool useProjectile=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","projectile_means_missile","false"));
	if (useProjectile&&weap->type==weapon_info::PROJECTILE)
		return true;
	if (useProjectile==false&&weap->size>=weapon_info::LIGHTMISSILE)
		return true;
	return false;
}


bool flickerDamage (Unit * un, float hullpercent)
{
#define damagelevel hullpercent
	static double counter=getNewTime();

	static float flickertime = XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","time","30"));
	static float flickerofftime = XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","off-time","2"));
	static float minflickercycle = XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","min-cycle","2"));
	static float flickeronprob= XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","num-times-per-second-on",".66"));
	static float hullfornoflicker= XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","hull-for-total-dark",".04"));
	float diff = getNewTime()-counter;
	if (diff>flickertime) {
		counter=getNewTime();
		diff=0;
	}
	float tmpflicker=flickertime*damagelevel;
	if (tmpflicker<minflickercycle) {
		tmpflicker=minflickercycle;
	}
	diff = fmod (diff,tmpflicker);
	//we know counter is somewhere between 0 and damage level
								 //cast this to an int for fun!
	unsigned int thus = ((unsigned int)(size_t)un)>>2;
	thus = thus % ((unsigned int)tmpflicker);
	diff = fmod (diff+thus,tmpflicker);
	if (flickerofftime>diff) {
		if (damagelevel>hullfornoflicker)
			return rand()>RAND_MAX*GetElapsedTime()*flickeronprob;
		else
			return true;
	}
	return false;
#undef damagelevel
}


//SERIOUSLY BROKEN
Vector ReflectNormal (const Vector &vel, const Vector & norm )
{
	//THIS ONE WORKS...but no...we don't want works	return norm * (2*vel.Dot(norm)) - vel;
	return norm*vel.Magnitude();
}


#define INVERSEFORCEDISTANCE 5400
extern void abletodock(int dock);
bool CrashForceDock(Unit * thus, Unit * dockingUn, bool force)
{
	Unit * un=dockingUn;
	int whichdockport=thus->CanDockWithMe(un,force);
	if (whichdockport!=-1) {
		if (Network==NULL) {
			QVector place=UniverseUtil::SafeEntrancePoint(un->Position(),un->rSize()*1.5);
			un->SetPosAndCumPos(place);
			if (un->ForceDock(thus,whichdockport)>0) {
				abletodock(3);
				un->UpgradeInterface(thus);
				return true;
			}
		}
		else {
			int playernum = _Universe->whichPlayerStarship( dockingUn );
			if( playernum>=0)
				Network[playernum].dockRequest( thus->GetSerial() );
			return false;
		}
	}
	return false;
}


void Unit::reactToCollision(Unit * smalle, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal,  float dist)
{
	clsptr smltyp = smalle->isUnit();
	if (smltyp==ENHANCEMENTPTR||smltyp==MISSILEPTR) {
		if (isUnit()!=ENHANCEMENTPTR&&isUnit()!=MISSILEPTR) {
			smalle->reactToCollision (this,smalllocation,smallnormal,biglocation,bignormal,dist);
			return;
		}
	}
	static bool crash_dock_unit=XMLSupport::parse_bool(vs_config->getVariable("physics","unit_collision_docks","false"));
	if (crash_dock_unit) {
		Unit * dockingun=smalle;
		Unit * thus=this;
		if (_Universe->isPlayerStarship(this)) {
			thus=smalle;
			dockingun=this;
		}
		if (_Universe->isPlayerStarship(dockingun)) {
			if (UnitUtil::getFlightgroupName(thus)=="Base") {
				static bool crash_dock_hangar=XMLSupport::parse_bool(vs_config->getVariable("physics","only_hangar_collision_docks","false"));

				if (CrashForceDock(thus,smalle,!crash_dock_hangar))
					return ;
			}
		}
	}
	//don't bounce if you can Juuuuuuuuuuuuuump
	if (!jumpReactToCollision(smalle)) {
		static float bouncepercent = XMLSupport::parse_float (vs_config->getVariable ("physics","BouncePercent",".1"));
		static float kilojoules_per_damage = XMLSupport::parse_float (vs_config->getVariable ("physics","kilojoules_per_unit_damage","5400"));
		static float collision_scale_factor=XMLSupport::parse_float(vs_config->getVariable("physics","collision_damage_scale","1.0"));
		static float inelastic_scale = XMLSupport::parse_float (vs_config->getVariable ("physics","inelastic_scale",".8"));
		static float mintime = XMLSupport::parse_float (vs_config->getVariable ("physics","minimum_time_between_recorded_player_collisions","0.1"));
		static float minvel = XMLSupport::parse_float (vs_config->getVariable ("physics","minimum_collision_velocity","5"));

		float m1=smalle->GetMass(),m2=GetMass();
		if (m1<1e-6f||m2<1e-6f) {
			if (m1<=0)m1=0.0f;
			if (m2<=0)m2=0.0f;
			m1+=1.0e-7f;
			m2+=1.0e-7f;
		}
		//Compute linear velocity of points of impact by taking into account angular velocities
		Vector small_velocity=smalle->GetVelocity()-smalle->GetAngularVelocity().Cross(smalllocation-smalle->Position());
		Vector big_velocity=GetVelocity()-GetAngularVelocity().Cross(biglocation-Position());
		//Compute reference frame conversions to align along force normals (newZ)(currently using bignormal - will experiment to see if both are needed for sufficient approximation)
		Vector orthoz=((m2*bignormal)-(m1*smallnormal)).Normalize();
		Vector orthox=MakeNonColinearVector(orthoz);
		Vector orthoy(0,0,0);
		//need z and non-colinear x to compute new basis trio. destroys x,y, preserves z.
		Orthogonize(orthox,orthoy,orthoz);
								 // transform matrix from normal aligned space
		Matrix fromNewRef(orthox,orthoy,orthoz);
		Matrix toNewRef=fromNewRef;
								 // transform matrix to normal aligned space
		fromNewRef.InvertRotationInto(toNewRef);
		Vector small_velocity_aligned=Transform(toNewRef,small_velocity);
		Vector big_velocity_aligned=Transform(toNewRef,big_velocity);
		//Compute elastic and inelastic terminal velocities (point object approximation)
								 // doesn't need aligning (I think)
		Vector Inelastic_vf = (m1/(m1+m2))*small_velocity + (m2/(m1+m2))*big_velocity;
		// compute along aligned dimension, then return to previous reference frame
		small_velocity_aligned.k= (small_velocity_aligned.k*(m1-m2)/(m1+m2)+(2.0f*m2/(m1+m2))*big_velocity_aligned.k);
		big_velocity_aligned.k=(big_velocity_aligned.k*(m2-m1)/(m1+m2)+(2.0f*m1/(m1+m2))*small_velocity_aligned.k);
		Vector SmallerElastic_vf =Transform(fromNewRef,small_velocity_aligned);
		Vector ThisElastic_vf = Transform(fromNewRef,big_velocity_aligned);

		// HACK ALERT:
		// following code referencing minvel and time between collisions attempts
		// to alleviate ping-pong problems due to collisions being detected
		// after the player has penetrated the hull of another vessel because of discretization of time.
		// this should eventually be replaced by instead figuring out where
		// the point of collision should have occurred, and moving the vessels to the
		// actual collision location before applying forces
		Cockpit * thcp = _Universe->isPlayerStarship (this);
		Cockpit * smcp = _Universe->isPlayerStarship (smalle);

		bool isnotplayerorhasbeenmintime=true;
		//Need to incorporate normals of colliding polygons somehow, without overiding directions of travel.
		//We'll use the point object approximation for the magnitude of damage, and then apply the force along the appropriate normals

		//ThisElastic_vf=((ThisElastic_vf.Magnitude()>minvel||!thcp)?ThisElastic_vf.Magnitude():minvel)*smallnormal;
		//SmallerElastic_vf=((SmallerElastic_vf.Magnitude()>minvel||!smcp)?SmallerElastic_vf.Magnitude():minvel)*bignormal;
		Vector ThisFinalVelocity=inelastic_scale*Inelastic_vf+(1.0f-inelastic_scale)*ThisElastic_vf;
		Vector SmallerFinalVelocity=inelastic_scale*Inelastic_vf+(1.0f-inelastic_scale)*SmallerElastic_vf;

		//float LargeKE = (0.5)*m2*GetVelocity().MagnitudeSquared();
		//float SmallKE = (0.5)*m1*smalle->GetVelocity().MagnitudeSquared();
		//float FinalInelasticKE = Inelastic_vf.MagnitudeSquared()*(0.5)*(m1+m2);
		//float InelasticDeltaKE = LargeKE +SmallKE - FinalInelasticKE;
								 // 1/2Mass*deltavfromnoenergyloss^2
		float LargeDeltaE=(0.5f)*m2*(ThisFinalVelocity-ThisElastic_vf).MagnitudeSquared();
								 // 1/2Mass*deltavfromnoenergyloss^2
		float SmallDeltaE=(0.5f)*m1*(SmallerFinalVelocity-SmallerElastic_vf).MagnitudeSquared();
		//Damage distribution (NOTE: currently arbitrary - no known good model for calculating how much energy object endures as a result of the collision)
		float large_damage=(0.25f*SmallDeltaE+0.75f*LargeDeltaE)/kilojoules_per_damage*collision_scale_factor;
		float small_damage=(0.25f*LargeDeltaE+0.75f*SmallDeltaE)/kilojoules_per_damage*collision_scale_factor;

		//Vector ThisDesiredVelocity = ThisElastic_vf*(1-inelastic_scale/2)+Inelastic_vf*inelastic_scale/2;
		//Vector SmallerDesiredVelocity = SmallerElastic_vf*(1-inelastic_scale)+Inelastic_vf*inelastic_scale;

		//FIXME need to resolve 2 problems -
		//1) SIMULATION_ATOM for small!= SIMULATION_ATOM for large (below smforce line should mostly address this)
		//2) Double counting due to collision occurring for each object in a different physics frame.
		Vector smforce = (SmallerFinalVelocity-small_velocity)*smalle->GetMass()/(SIMULATION_ATOM*((float)smalle->sim_atom_multiplier)/((float)this->sim_atom_multiplier));
		Vector thisforce = (ThisFinalVelocity-big_velocity)*GetMass()/SIMULATION_ATOM;

		if(thcp) {
			if((getNewTime()-thcp->TimeOfLastCollision)>mintime) {
				//if((ThisFinalVelocity-GetVelocity()).Magnitude()>minvel){
				thcp->TimeOfLastCollision=getNewTime();
				//}
			}
			else {
				isnotplayerorhasbeenmintime=false;
			}
		}

		if(smcp) {
			if((getNewTime()-smcp->TimeOfLastCollision)>mintime) {
				//if((SmallerFinalVelocity-smalle->GetVelocity()).Magnitude()>minvel){
				smcp->TimeOfLastCollision=getNewTime();
				//}
			}
			else {
				isnotplayerorhasbeenmintime=false;
			}
		}
		/*
		if(isnotplayerorhasbeenmintime){
			UniverseUtil::IOmessage(0,"game","all",string("c1")+XMLSupport::tostring((float)getNewTime())+string(" DE_s ")+XMLSupport::tostring(SmallDeltaE)+string(" DE_l ")+XMLSupport::tostring(LargeDeltaE)+string(" resultant damages ")+XMLSupport::tostring(small_damage)+string(" ")+XMLSupport::tostring(large_damage));//+string(" inelastic factor ")+XMLSupport::tostring(inelastic_scale)+string(" collision factor ")+XMLSupport::tostring(collision_scale_factor));
			UniverseUtil::IOmessage(0,"game","all",string("c2")+string(" TFV ")+XMLSupport::tostring(ThisFinalVelocity.Magnitude())+string(" SFV ")+XMLSupport::tostring(SmallerFinalVelocity.Magnitude())+string(" pVF ")+XMLSupport::tostring(Inelastic_vf.Magnitude())+string(" ")+XMLSupport::tostring(ThisElastic_vf.Magnitude())+string(" ")+XMLSupport::tostring(SmallerElastic_vf.Magnitude()));
		}
		*/
		if (Network!=NULL) {
			// Only player units can move in network mode.
			if (thcp) {
				this->ApplyForce(thisforce-smforce);
			} else if (smcp) {
				smalle->ApplyForce(smforce-thisforce);
			}
		} else {
			//Collision force caps primarily for AI-AI collisions. Once the AIs get a real collision avoidance system, we can turn damage for AI-AI collisions back on, and then we can remove these caps.
			static float maxTorqueMultiplier = XMLSupport::parse_float(vs_config->getVariable("physics","maxCollisionTorqueMultiplier",".67")); // value, in seconds of desired maximum recovery time
			static float maxForceMultiplier = XMLSupport::parse_float(vs_config->getVariable("physics","maxCollisionForceMultiplier","5")); // value, in seconds of desired maximum recovery time
			if((smalle->isUnit()!=MISSILEPTR)&&isnotplayerorhasbeenmintime) {

								 // for torque... smalllocation -- approximation hack of MR^2 for rotational inertia (moment of inertia currently just M)
				Vector torque = smforce/(smalle->radial_size*smalle->radial_size);
				Vector force = smforce-torque;

				float maxForce = maxForceMultiplier * (smalle->limits.forward+smalle->limits.retro+
						smalle->limits.lateral+smalle->limits.vertical);
				float maxTorque = maxTorqueMultiplier * (smalle->limits.yaw+
						smalle->limits.pitch+smalle->limits.roll);
				//Convert from frames to seconds, so that the specified value is meaningful
				maxForce=maxForce/(smalle->sim_atom_multiplier*SIMULATION_ATOM);
				maxTorque=maxTorque/(smalle->sim_atom_multiplier*SIMULATION_ATOM);
				float tMag = torque.Magnitude();
				float fMag = force.Magnitude();
				if (tMag > maxTorque)
					torque *= (maxTorque/tMag);
				if (fMag > maxForce)
					force *= (maxForce/fMag);
				smalle->ApplyTorque(torque,smalllocation);
				smalle->ApplyForce(force-torque);
			}
			if((this->isUnit()!=MISSILEPTR)&&isnotplayerorhasbeenmintime) {
								 // for torque ... biglocation -- approximation hack of MR^2 for rotational inertia
				Vector torque=thisforce/(radial_size*radial_size);
				Vector force = thisforce-torque;
				float maxForce = maxForceMultiplier * (limits.forward+limits.retro+
						limits.lateral+limits.vertical);
				float maxTorque = maxTorqueMultiplier * (limits.yaw+limits.pitch+limits.roll);
				//Convert from frames to seconds, so that the specified value is meaningful
				maxForce=maxForce/(this->sim_atom_multiplier*SIMULATION_ATOM);
				maxTorque=maxTorque/(this->sim_atom_multiplier*SIMULATION_ATOM);
				float tMag = torque.Magnitude();
				float fMag = force.Magnitude();
				if (tMag > maxTorque)
					torque *= (maxTorque/tMag);
				if (fMag > maxForce)
					force *= (maxForce/fMag);
				this->ApplyTorque (torque,biglocation);
				this->ApplyForce(force-torque);
			}
		}
		/*    smalle->curr_physical_state = smalle->prev_physical_state;
			  this->curr_physical_state = this->prev_physical_state;*/
		static int upgradefac = XMLSupport::parse_bool(vs_config->getVariable("physics","cargo_deals_collide_damage","false"))?-1:FactionUtil::GetUpgradeFaction();
		bool dealdamage=true;
		if (_Universe->AccessCamera())  {
			Vector smalldelta=(_Universe->AccessCamera()->GetPosition()-smalle->Position()).Cast();
			float smallmag=smalldelta.Magnitude();
			Vector thisdelta=(_Universe->AccessCamera()->GetPosition()-this->Position()).Cast();
			float thismag=thisdelta.Magnitude();
			static float collision_hack_distance=XMLSupport::parse_float(vs_config->getVariable("physics","collision_avoidance_hack_distance","10000"));
			static float front_collision_hack_distance=XMLSupport::parse_float(vs_config->getVariable("physics","front_collision_avoidance_hack_distance","200000"));

			if (thcp==NULL&&smcp==NULL) {
				if (smallmag>collision_hack_distance+this->rSize()&&thismag>collision_hack_distance) {
					static float front_collision_hack_angle=cos(3.1415926536f*XMLSupport::parse_float(vs_config->getVariable("physics","front_collision_avoidance_hack_angle","40"))/180.0f);

					if (smalldelta.Dot(_Universe->AccessCamera()->GetR())<smallmag*front_collision_hack_angle&&thisdelta.Dot(_Universe->AccessCamera()->GetR())<thismag*front_collision_hack_angle) {
						if (smallmag>front_collision_hack_distance+this->rSize()&&thismag>front_collision_hack_distance) {
							dealdamage=false;
						}
					}else {
						dealdamage=false;
					}
				}
			}
		}
		if (!_Universe->isPlayerStarship(this) && !_Universe->isPlayerStarship(smalle)) {
			if (this->isUnit()!=MISSILEPTR && smalle->isUnit()!=MISSILEPTR) {
				static bool collisionDamageToAI = XMLSupport::parse_bool(vs_config->getVariable("physics","collisionDamageToAI","false"));
				if (!collisionDamageToAI) {
					// HACK: Stupid AI ships always crash into each other.
					dealdamage=false;
				}
			}
		}
		if (dealdamage) {
			if (faction!=upgradefac)
				smalle->ApplyDamage (biglocation.Cast(),bignormal,small_damage,smalle,GFXColor(1,1,1,2),this->owner!=NULL?this->owner:this);
			/* Happens too often to be useful.
			else
				printf ("Damage avoided due to cargo\n"); */
			if (smalle->faction!=upgradefac)
				this->ApplyDamage (smalllocation.Cast(),smallnormal,large_damage,this,GFXColor(1,1,1,2),smalle->owner!=NULL?smalle->owner:smalle);
			/* Happens too often to be useful
			else
				printf ("Damage avoided due to cargo\n"); */
		}
		//OLDE METHODE
		//    smalle->ApplyDamage (biglocation.Cast(),bignormal,.33*g_game.difficulty*(  .5*fabs((smalle->GetVelocity()-this->GetVelocity()).MagnitudeSquared())*this->mass*SIMULATION_ATOM),smalle,GFXColor(1,1,1,2),NULL);
		//    this->ApplyDamage (smalllocation.Cast(),smallnormal, .33*g_game.difficulty*(.5*fabs((smalle->GetVelocity()-this->GetVelocity()).MagnitudeSquared())*smalle->mass*SIMULATION_ATOM),this,GFXColor(1,1,1,2),NULL);
		//each mesh with each mesh? naw that should be in one way collide
	}
}


static Unit * getFuelUpgrade ()
{
	return UnitFactory::createUnit("add_fuel",true,FactionUtil::GetUpgradeFaction());
}


static float getFuelAmt ()
{
	Unit * un = getFuelUpgrade();
	float ret = un->FuelData();
	un->Kill();
	return ret;
}


static float GetJumpFuelQuantity()
{
	static float f= getFuelAmt();
	return f;
}


void Unit::ActivateJumpDrive (int destination)
{
	//const int jumpfuelratio=1;
	if (((docked&(DOCKED|DOCKED_INSIDE))==0)&&jump.drive!=-2) {
		/*if (1warpenergy>=jump.energy&&(jump.energy>=0)) {*/
		jump.drive = destination;
		//float fuel_used=0;
		//    warpenergy-=jump.energy; //don't spend energy until later
		/*}*/
	}
}


void Unit::DeactivateJumpDrive ()
{
	if (jump.drive>=0) {
		jump.drive=-1;
	}
}


//copysign conflicted with c++11 double copysign(double,double)
//which does not do the same thing
float vs_copysignf (float x, float y)
{
	if (y>0)
		return x;
	else
		return -x;
}


float rand01 ()
{
	return ((float)rand()/(float)RAND_MAX);
}


float capship_size=500;

/* UGLYNESS short fix */
unsigned int apply_float_to_unsigned_int (float tmp)
{
	static unsigned long int seed = 2531011;
	seed +=214013;
	seed %=4294967295u;
	unsigned  int ans = (unsigned int) tmp;
	tmp -=ans;					 //now we have decimal;
	if (seed<(unsigned long int)(4294967295u*tmp))
		ans +=1;
	return ans;
}


std::string accelStarHandler (const XMLType &input,void *mythis)
{
	static float game_speed = XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
	static float game_accel = XMLSupport::parse_float (vs_config->getVariable ("physics","game_accel","1"));
	return XMLSupport::tostring(*input.w.f/(game_speed*game_accel));
}


std::string speedStarHandler (const XMLType &input,void *mythis)
{
	static float game_speed = XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
	return XMLSupport::tostring((*input.w.f)/game_speed);
}


static std::list<Unit*> Unitdeletequeue;
static Hashtable <void *, Unit, 2095> deletedUn;
int deathofvs=1;
void CheckUnit(Unit * un)
{
	if (deletedUn.Get (un)!=NULL) {
		while (deathofvs) {
			UNIT_LOG(logvs::NOTICE, "0x%zx died",(size_t)un);
		}
	}
}


void UncheckUnit (Unit * un)
{
	if (deletedUn.Get (un)!=NULL) {
		deletedUn.Delete (un);
	}
}


string GetUnitDir(const string & filename)
{
	return filename.substr(0,filename.find("."));
}


char * GetUnitDir (const char * filename)
{
	char * retval=strdup (filename);
	if (retval[0]=='\0')
		return retval;
	if (retval[1]=='\0')
		return retval;
	for (int i=0;retval[i]!=0;++i) {
		if (retval[i]=='.') {
			retval[i]='\0';
			break;
		}
	}
	return retval;
}


// From weapon_xml.cpp
std::string lookupMountSize (int s)
{
	std::string result;
	if (s&weapon_info::LIGHT) {
		result+="LIGHT ";
	}
	if (s&weapon_info::MEDIUM) {
		result+="MEDIUM ";
	}
	if (s&weapon_info::HEAVY) {
		result+="HEAVY ";
	}
	if (s&weapon_info::CAPSHIPLIGHT) {
		result+="CAPSHIP-LIGHT ";
	}
	if (s&weapon_info::CAPSHIPHEAVY) {
		result+="CAPSHIP-HEAVY ";
	}
	if (s&weapon_info::SPECIAL) {
		result+="SPECIAL ";
	}
	if (s&weapon_info::LIGHTMISSILE) {
		result+="LIGHT-MISSILE ";
	}
	if (s&weapon_info::MEDIUMMISSILE) {
		result+="MEDIUM-MISSILE ";
	}
	if (s&weapon_info::HEAVYMISSILE) {
		result+="HEAVY-MISSILE ";
	}
	if (s&weapon_info::CAPSHIPLIGHTMISSILE) {
		result+="LIGHT-CAPSHIP-MISSILE ";
	}
	if (s&weapon_info::CAPSHIPHEAVYMISSILE) {
		result+="HEAVY-CAPSHIP-MISSILE ";
	}
	if (s&weapon_info::SPECIALMISSILE) {
		result+="SPECIAL-MISSILE ";
	}
	if (s&weapon_info::AUTOTRACKING) {
		result+="AUTOTRACKING ";
	}
	return result;
}


/***********************************************************************************/
/**** UNIT STUFF                                                            */
/***********************************************************************************/
Unit::Unit( int /*dummy*/ ):cumulative_transformation_matrix(identity_matrix) {
ZeroAll();
image = new UnitImages;
sound = new UnitSounds;
aistate=NULL;
image->cockpit_damage=NULL;
//SetAI (new Order());
pilot= new Pilot(FactionUtil::GetNeutralFaction());
Init();
}


Unit::Unit():cumulative_transformation_matrix(identity_matrix)
{
	ZeroAll();
	image = new UnitImages;
	sound = new UnitSounds;
	aistate=NULL;
	image->cockpit_damage=NULL;
	//SetAI (new Order());
	pilot= new Pilot(FactionUtil::GetNeutralFaction());
	Init();
}


Unit::Unit (std::vector <Mesh *> & meshes, bool SubU, int fact):cumulative_transformation_matrix(identity_matrix)
{
	ZeroAll();
	image = new UnitImages;
	sound = new UnitSounds;
	pilot= new Pilot(fact);
	aistate=NULL;
	image->cockpit_damage=NULL;
	//SetAI (new Order());
	Init();
	hull=1000;
	maxhull=100000;
	this->faction = fact;
	graphicOptions.SubUnit = SubU;
	meshdata = meshes;
	meshes.clear();
	meshdata.push_back(NULL);
	calculate_extent(false);
	pilot->SetComm(this);
}


extern void update_ani_cache();
Unit::Unit(const char *filename, bool SubU, int faction,const std::string & unitModifications, Flightgroup *flightgrp,int fg_subnumber, string * netxml):cumulative_transformation_matrix(identity_matrix)
{
	ZeroAll();
	image = new UnitImages;
	sound = new UnitSounds;
	pilot= new Pilot(faction);
	aistate=NULL;
	image->cockpit_damage=NULL;
	//SetAI (new Order());
	Init( filename, SubU, faction, unitModifications, flightgrp, fg_subnumber, netxml);
	pilot->SetComm(this);
}


Unit::~Unit()
{
	free(image->cockpit_damage);
	if ((!killed)) {
		UNIT_LOG(logvs::WARN, "Assumed exit on unit %s(if not quitting, report error)",name.get().c_str());
	}
	if (ucref) {
		UNIT_LOG(logvs::ERROR, "DISASTER AREA!!!!");
	}
#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"stage %d %x %d\n", 0,this,ucref);
	fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d %x ", 1,planet);
	fflush (stderr);
	VSFileSystem::vs_fprintf (stderr,"%d %x\n", 2,image->hudImage);
	fflush (stderr);
#endif
	if (image->unitwriter)
		delete image->unitwriter;
	unsigned int i;

#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d %x", 3,image);
	fflush (stderr);
#endif
	delete image;
	delete sound;
	delete pilot;
#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d %x %x", 4,bspTree, bspShield);
	fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d", 5);
	fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d %x", 6,&mounts);
	fflush (stderr);
#endif

#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d %x ", 9, halos);
	fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d %x ", 1,&mounts);
	fflush (stderr);
#endif
#ifndef NO_MOUNT_STAR
	for( vector<Mount *>::iterator jj=mounts.begin(); jj!=mounts.end(); ++jj) {
		// Free all mounts elements
		if( (*jj)!=NULL)
			delete (*jj);
	}
#endif
	mounts.clear();
#ifdef DESTRUCTDEBUG
	VSFileSystem::vs_fprintf (stderr,"%d", 0);
	fflush (stderr);
#endif
	for(unsigned int meshcount = 0; meshcount < meshdata.size(); ++meshcount)
		if (meshdata[meshcount])
			delete meshdata[meshcount];
	meshdata.clear();
}


void Unit::ZeroAll( )
{
	sound            = NULL;
	ucref            = 0;
	networked        = false;
	serial           = 0;
	net_accel.i      = 0;
	net_accel.j      = 0;
	net_accel.k      = 0;
	SavedAccel.i     = 0;
	SavedAccel.j     = 0;
	SavedAccel.k     = 0;
	// old_state has a constructor
	damages          = NO_DAMAGE;
	// SubUnits has a constructor
	attack_preference=unit_role = 0;
	nebula           = NULL;
	activeStarSystem = NULL;
	// computer has a constructor
	// jump needs fixing
	selected         = false;
	// scanner needs fixing
	xml              = NULL;
	owner            = NULL;
	// prev_physical_state has a constructor
	// curr_physical_state has a constructor
	//cumulative_transformation_matrix has a constructor
	// cumulative_transformation has a constructor
	cumulative_velocity.i = 0;
	cumulative_velocity.j = 0;
	cumulative_velocity.k = 0;
	NetForce.i            = 0;
	NetForce.j            = 0;
	NetForce.k            = 0;
	NetLocalForce.i       = 0;
	NetLocalForce.j       = 0;
	NetLocalForce.k       = 0;
	NetTorque.i           = 0;
	NetTorque.j           = 0;
	NetTorque.k           = 0;
	NetLocalTorque.i      = 0;
	NetLocalTorque.j      = 0;
	NetLocalTorque.k      = 0;
	AngularVelocity.i     = 0;
	AngularVelocity.j     = 0;
	AngularVelocity.k     = 0;
	Velocity.i            = 0;
	Velocity.j            = 0;
	Velocity.k            = 0;
	image                 = NULL;
	Mass                  = 0;
	shieldtight           = 0;	 // this can be used to differentiate whether this is a capship or a fighter?
	fuel                  = 0;
	afterburnenergy       = 0;
	afterburntype         = 0;
	Momentofinertia       = 0;
	// limits has a constructor
	cloaking              = 0;
	cloakmin              = 0;
	radial_size           = 0;
	killed                = false;
	invisible             = 0;
	corner_min.i          = 0;
	corner_min.j          = 0;
	corner_min.k          = 0;
	corner_max.i          = 0;
	corner_max.j          = 0;
	corner_max.k          = 0;
	resolveforces         = false;
	// armor has a constructor
	// shield has a constructor
	hull                  = 0;
	maxhull               = 0;
	recharge              = 0;
	maxenergy             = 0;
	energy                = 0;
	maxwarpenergy         = 0;
	warpenergy            = 0;
	// target_fgid has a constructor
	aistate               = NULL;
	// CollideInfo has a constructor
	colTrees              = NULL;
	docked                = NOT_DOCKED;
	faction               = 0;
	flightgroup           = NULL;
	flightgroup_subnumber = 0;
	setTractorability(tractorImmune);
}


void Unit::Init()
{
	this->schedule_priority=Unit::scheduleDefault;
	for (unsigned int locind=0;locind<NUM_COLLIDE_MAPS;++locind) {
		set_null(location[locind]);
	}
	specInterdiction=0;
	sim_atom_multiplier=1;
	predicted_priority=1;
	cur_sim_queue_slot=rand()%SIM_QUEUE_SIZE;
	last_processed_sqs=0;
	do_subunit_scheduling=false;
	/*
	static vsUMap<Unit *, bool> m;
	if (m[this]) {
	  VSFileSystem::vs_fprintf (stderr,"already called this");
	}else {
	  m[this]=1;
	  }*/
	if( Network==NULL)
		this->networked=0;
	else
		this->networked=1;

	damages = NO_DAMAGE;

	graphicOptions.RecurseIntoSubUnitsOnCollision=false;
	graphicOptions.WarpFieldStrength=1;
	inertialmode=false;
	turretstatus=0;
	autopilotactive=false;
	this->unit_role=this->attack_preference=ROLES::getRole("INERT");
	this->computer.combat_mode=true;
#ifdef CONTAINER_DEBUG
	UncheckUnit (this);
#endif
	static float capsize = XMLSupport::parse_float(vs_config->getVariable("physics","capship_size","500"));

	capship_size=capsize;
	activeStarSystem=NULL;
	xml=NULL;
	docked=NOT_DOCKED;
	graphicOptions.SubUnit =0;
	jump.energy = 100;
	static float insys_jump_cost = XMLSupport::parse_float (vs_config->getVariable ("physics","insystem_jump_cost",".1"));
	jump.insysenergy=insys_jump_cost*jump.energy;
	jump.delay=5;
	jump.damage=0;
	jump.warpDriveRating=0;
	graphicOptions.FaceCamera=false;
	jump.drive=-2;				 // disabled
	afterburnenergy=0;
	nebula=NULL;
	limits.structurelimits=Vector(0,0,1);
	limits.limitmin=-1;
	cloaking=-1;
	image->repair_droid=0;
	image->next_repair_time=-FLT_MAX;
	image->next_repair_cargo=~0;
	image->ecm=0;
	image->cloakglass=false;
	image->CargoVolume=0;
	image->UpgradeVolume=0;
	this->HeatSink=0;

	image->unitwriter=NULL;
	cloakmin=image->cloakglass?1:0;
	image->equipment_volume=0;
	image->HiddenCargoVolume=0;
	image->cloakrate=100;
	image->cloakenergy=0;
	image->forcejump=false;
	sound->engine=-1;  sound->armor=-1;  sound->shield=-1;  sound->hull=-1; sound->explode=-1; sound->cloak=-1; sound->jump=-1;
	image->fireControlFunctionality=1.0f;
	image->fireControlFunctionalityMax=1.0f;
	image->SPECDriveFunctionality=1.0f;
	image->SPECDriveFunctionalityMax=1.0f;
	image->CommFunctionality=1.0f;
	image->CommFunctionalityMax=1.0f;
	image->LifeSupportFunctionality=1.0f;
	image->LifeSupportFunctionalityMax=1.0f;

	image->hudImage=NULL;

	// Freedom for the masses!!!!  //you'll have to justify why setting to this is better.
	owner = NULL;
	faction =0;
	resolveforces=true;
	colTrees=NULL;
	invisible=DEFAULTVIS;
	//origin.Set(0,0,0);
	corner_min.Set (FLT_MAX,FLT_MAX,FLT_MAX);
	corner_max.Set (-FLT_MAX,-FLT_MAX,-FLT_MAX);

	//BUCO! Must add shield tightness back into units.csv for great justice.
	static float default_shield_tightness = XMLSupport::parse_float(vs_config->getVariable("physics","default_shield_tightness","0"));
								 // was 0 // sphere mesh by default, but let's decide on it
	shieldtight=default_shield_tightness;
	energy=maxenergy=1;
	warpenergy=0;
	maxwarpenergy=0;
	recharge = 1;
	shield.recharge=shield.leak=0;
	this->shield.efficiency=1;
	shield.shield2fb.front=shield.shield2fb.back=shield.shield2fb.frontmax=shield.shield2fb.backmax=armor.frontrighttop=armor.backrighttop=armor.frontlefttop=armor.backlefttop=armor.frontrightbottom=armor.backrightbottom=armor.frontleftbottom=armor.backleftbottom=0;
	hull=1;						 //10;
	maxhull=1;					 //10;
	shield.number=0;

	image->explosion=NULL;
	image->timeexplode=0;
	killed=false;
	ucref=0;
	aistate = NULL;
	Identity(cumulative_transformation_matrix);
	cumulative_transformation = identity_transformation;
	curr_physical_state = prev_physical_state = identity_transformation;
	Mass = .01;
	fuel = 000;

	static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
	static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
	static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics",
		"autotracking",
		".93"));				 // DO NOT CHANGE see unit_customize.cpp

								 // DO NOT CHANGE see unit_customize.cpp
	static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));

	Momentofinertia = .01;
	AngularVelocity = myang;
	cumulative_velocity=Velocity = Vector(0,0,0);

	NetTorque =NetLocalTorque = Vector(0,0,0);
	NetForce = Vector(0,0,0);
	NetLocalForce=Vector(0,0,0);

	selected = false;
	//  image->selectionBox = NULL;

	limits.yaw = 2.55;
	limits.pitch = 2.55;
	limits.roll = 2.55;

	limits.lateral = 2;
	limits.vertical = 8;
	limits.forward = 2;
	limits.afterburn=5;
	limits.retro=2;
	VelocityReference(NULL);
	computer.threat.SetUnit (NULL);
	computer.threatlevel=0;
	computer.slide_start=computer.slide_end=0;
	computer.set_speed=0;
	computer.max_combat_speed=1;
	computer.max_combat_ab_speed=1;
	computer.max_yaw_right=computer.max_yaw_left=1;

	computer.max_pitch_down=computer.max_pitch_up=1;
	computer.max_roll_right=computer.max_roll_left=1;
	computer.NavPoint=Vector(0,0,0);
	computer.itts = false;
	computer.radar.maxrange=rr;
	computer.radar.locked=false;
	computer.radar.maxcone=-1;
	computer.radar.trackingcone = minTrackingNum;
	computer.radar.lockcone=lc;
	computer.radar.mintargetsize=0;
	computer.radar.iff=0;

	flightgroup=NULL;
	flightgroup_subnumber=0;

	// No cockpit reference here
	if (!image->cockpit_damage) {
		unsigned int numg= (1+MAXVDUS+UnitImages::NUMGAUGES)*2;
		image->cockpit_damage=(float*)malloc((numg)*sizeof(float));
		for (unsigned int damageiterator=0;damageiterator<numg;++damageiterator) {
			image->cockpit_damage[damageiterator]=1;
		}
	}

	/*
	yprrestricted=0;
	ymin = pmin = rmin = -PI;
	ymax = pmax = rmax = PI;
	ycur = pcur = rcur = 0;
	*/
	// Not needed here
	//static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
	// Not needed here
	//static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
	// Not needed here
	/*
	static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics",
											"autotracking",
										  ".93"));// DO NOT CHANGE see unit_customize.cpp
	*/
	// Not needed here
	//static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
	//  Fire();

}


std::string getMasterPartListUnitName();
using namespace VSFileSystem;
extern std::string GetReadPlayerSaveGame (int);
CSVRow GetUnitRow(const string & filename, bool subu, int faction, bool readLast, bool &read);
#if 0
static std::string csvUnit(const std::string & un)
{
	string::size_type i=un.find_last_of(".");
	string::size_type del=un.find_last_of("/\\:");
	if (i==std::string::npos) {
		return un+".csv";
	}
	if (del==std::string::npos||del<i) {
		return un.substr(0,i)+".csv";
	}
	return un+".csv";
}
#endif
void Unit::Init(const char *filename, bool SubU, int faction,const std::string & unitModifications, Flightgroup *flightgrp,int fg_subnumber, string * netxml)
{
	static bool UNITTAB = XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));
	CSVRow unitRow;
	this->Unit::Init();
	//if (!SubU)
	//  _Universe->AccessCockpit()->savegame->AddUnitToSave(filename,UNITPTR,FactionUtil::GetFaction(faction),(long)this);
	graphicOptions.SubUnit = SubU?1:0;
	graphicOptions.Animating=1;
	graphicOptions.RecurseIntoSubUnitsOnCollision=!isSubUnit();
	this->faction = faction;
	SetFg (flightgrp,fg_subnumber);
	VSFile f;
	VSFile f2;
	VSError err = Unspecified;
	VSFile unitTab;
	VSError taberr= Unspecified;;
	bool foundFile=false;
	if( netxml==NULL) {
		if (unitModifications.length()!=0) {
			string nonautosave=GetReadPlayerSaveGame(_Universe->CurrentCockpit());
			string filepath("");
			// In network mode we only look in the save subdir in HOME
			if( Network==NULL && !SERVER) {
				if (nonautosave.empty()) {
					VSFileSystem::CreateDirectoryHome (VSFileSystem::savedunitpath+"/"+unitModifications);
					filepath = unitModifications+"/"+string(filename);
				}
				else {
					VSFileSystem::CreateDirectoryHome (VSFileSystem::savedunitpath+"/"+nonautosave);
					filepath = nonautosave+"/"+string(filename);
				}
			}
			// This is not necessary as I think... to watch
			//VSFileSystem::vs_chdir( "save");

			// Try to open save
			if (filename[0]) {
				taberr=unitTab.OpenReadOnly( filepath+".csv", UnitSaveFile);
				if (taberr<=Ok) {
					unitTables.push_back(new CSVTable(unitTab,unitTables.back()->rootdir));
					unitTab.Close();
				}
				if (!UNITTAB)
					err = f.OpenReadOnly( filepath, UnitSaveFile);

			}
		}
	}
	if (netxml) {
		unitTables.push_back(new CSVTable(*netxml,unitTables.back()->rootdir));
	}
	// If save was not succesfull we try to open the unit file itself
	if( netxml==NULL) {
		if (filename[0]) {
			string subdir = "factions/"+FactionUtil::GetFactionName(faction);
			// begin deprecated code (5/11)
			if (UNITTAB) {

			}
			else {
				if( err>Ok) {
					f.SetSubDirectory(subdir);
					// No save found loading default unit
					err = f.OpenReadOnly (filename, UnitFile);
					if (err>Ok) {
						f.SetSubDirectory("");
						err = f.OpenReadOnly (filename, UnitFile);
					}
				}
				else {
					f2.SetSubDirectory(subdir);
					// Save found so just opening default unit to get its directory for further loading
					err = f2.OpenReadOnly (filename, UnitFile);
					if (err>Ok) {
						f2.SetSubDirectory("");
						err = f2.OpenReadOnly (filename, UnitFile);
					}
				}

			}
			//end deprecated code
		}
	}

	if (UNITTAB) {
		unitRow = GetUnitRow(filename,SubU,faction,true, foundFile);
	}
	else {
		foundFile = (err<=Ok);
	}

	this->filename = filename;

	if(!foundFile) {
		bool istemplate=(string::npos!=(string(filename).find(".template")));
		static bool usingtemplates = XMLSupport::parse_bool(vs_config->getVariable("data","usingtemplates","true"));
		if(!istemplate||(istemplate&&usingtemplates)){
            UNIT_LOG(logvs::NOTICE, "Unit file %s not found", filename);
/*
			VSFileSystem::vs_fprintf (stderr,"Assertion failed in Unit::Init -- Unit %s not found\n",filename);

			VSFileSystem::vs_fprintf (stderr,"Warning: Cannot locate %s\n",filename);
*/
		}
		meshdata.clear();
		meshdata.push_back(NULL);
		this->fullname=filename;
		this->name=string("LOAD_FAILED");
		calculate_extent(false);
		radial_size=1;
		//	    assert ("Unit Not Found"==NULL);
		//assert(0);
		if ((taberr<=Ok&&taberr!=Unspecified)||netxml) {
			delete unitTables.back();
			unitTables.pop_back();
		}
		pilot->SetComm(this);
		return;
	}

	this->name = this->filename;
	bool tmpbool;
	if (UNITTAB) {
		// load from table?

		// we have to set the root directory to where the saved unit would have come from.
		// saved only exists if taberr<=Ok && taberr!=Unspecified...that's why we pass in said boolean
		VSFileSystem::current_path.push_back(taberr<=Ok&&taberr!=Unspecified?GetUnitRow(filename,SubU,faction,false,tmpbool).getRoot():unitRow.getRoot());
		VSFileSystem::current_subdirectory.push_back("/"+unitRow["Directory"]);
		VSFileSystem::current_type.push_back(UnitFile);
		LoadRow(unitRow,unitModifications,netxml);
		VSFileSystem::current_type.pop_back();
		VSFileSystem::current_subdirectory.pop_back();
		VSFileSystem::current_path.pop_back();
		if ((taberr<=Ok&&taberr!=Unspecified)||netxml) {
			delete unitTables.back();
			unitTables.pop_back();
		}

	}
	else {
		if( netxml==NULL)
			Unit::LoadXML(f,unitModifications.c_str());
		else
			Unit::LoadXML( f, "", netxml);
		if( err<=Ok)
			f.Close();
		if( f2.Valid())
			f2.Close();
	}

	calculate_extent(false);
	pilot->SetComm(this);
	///	  ToggleWeapon(true);//change missiles to only fire 1
}


vector <Mesh *> Unit::StealMeshes()
{
	vector <Mesh *>ret;

	Mesh * shield = meshdata.empty()?NULL:meshdata.back();
	for (int i=0;i<nummesh();++i) {
		ret.push_back (meshdata[i]);
	}
	meshdata.clear();
	meshdata.push_back(shield);

	return ret;
}


static float tmpmax (float a , float b)
{
	return a>b?a:b;
}


bool CheckAccessory (Unit * tur)
{
	bool accessory = tur->name.get().find ("accessory")!=string::npos;
	if (accessory) {
		tur->SetAngularVelocity(tur->DownCoordinateLevel(Vector (tur->GetComputerData().max_pitch_up,
			tur->GetComputerData().max_yaw_right,
			tur->GetComputerData().max_roll_right)));
	}
	return accessory;
}


void Unit::calculate_extent(bool update_collide_queue)
{
	int a;
	corner_min=Vector (FLT_MAX,FLT_MAX,FLT_MAX);
	corner_max=Vector (-FLT_MAX,-FLT_MAX,-FLT_MAX);

	for(a=0; a<nummesh(); ++a) {
		corner_min = corner_min.Min(meshdata[a]->corner_min());
		corner_max = corner_max.Max(meshdata[a]->corner_max());
	}							 /* have subunits now in table*/
	const Unit * un;
	for(un_kiter iter = SubUnits.constIterator();(un = *iter) != NULL;++iter){
		corner_min = corner_min.Min(un->LocalPosition().Cast()+un->corner_min);
		corner_max = corner_max.Max(un->LocalPosition().Cast()+un->corner_max);
	}

	if (corner_min.i==FLT_MAX||corner_max.i==-FLT_MAX||!FINITE (corner_min.i)||!FINITE(corner_max.i)) {
		radial_size=0;
		corner_min.Set (0,0,0);
		corner_max.Set (0,0,0);
	}
	else {
		float tmp1 = corner_min.Magnitude();
		float tmp2 = corner_max.Magnitude();
		radial_size = tmp1>tmp2?tmp1:tmp2;
		//    if (!SubUnit)
		//      image->selectionBox = new Box(corner_min, corner_max);
	}
	if (!isSubUnit()&&update_collide_queue&&(maxhull>0)) {
		//only do it in Unit::CollideAll UpdateCollideQueue();
	}
	if (isUnit()==PLANETPTR) {
		radial_size = tmpmax(tmpmax(corner_max.i,corner_max.j),corner_max.k) ;
	}
}


StarSystem * Unit::getStarSystem ()
{

	if (activeStarSystem) {
		return activeStarSystem;
	}
	else {
		Cockpit * cp=_Universe->isPlayerStarship(this);
		if (cp) {
			if (cp->activeStarSystem)
				return cp->activeStarSystem;
		}
	}
	return _Universe->activeStarSystem();
}

const StarSystem * Unit::getStarSystem () const
{

	if (activeStarSystem) {
		return activeStarSystem;
	}
	else {
		Cockpit * cp=_Universe->isPlayerStarship(this);
		if (cp) {
			if (cp->activeStarSystem)
				return cp->activeStarSystem;
		}
	}
	return _Universe->activeStarSystem();
}


bool preEmptiveClientFire(const weapon_info*wi)
{
	static bool
		client_side_fire=XMLSupport::parse_bool(vs_config->getVariable("network","client_side_fire","true"));
	return (client_side_fire&&wi->type!=weapon_info::BEAM&&wi->type!=weapon_info::PROJECTILE);
}


void Unit::Fire (unsigned int weapon_type_bitmask, bool listen_to_owner)
{
	static bool can_fire_in_spec = XMLSupport::parse_bool(vs_config->getVariable("physics","can_fire_in_spec","false"));
	static bool can_fire_in_cloak = XMLSupport::parse_bool(vs_config->getVariable("physics","can_fire_in_cloak","false"));
	static bool verbose_debug = XMLSupport::parse_bool(vs_config->getVariable("data","verbose_debug","false"));
	if ((cloaking>=0&&can_fire_in_cloak==false)||(graphicOptions.InWarp&&can_fire_in_spec==false)) {
		UnFire();
		return;
	}
	unsigned int mountssize=mounts.size();
	int playernum = _Universe->whichPlayerStarship( this);
	vector<int> gunFireRequests;
	vector<int> missileFireRequests;
	vector<int> serverUnfireRequests;

	for (unsigned int counter=0;counter<mountssize;++counter) {
		unsigned int index=counter;
		Mount * i=&mounts[index];
		if (i->status!=Mount::ACTIVE)
			continue;
		if (i->bank==true) {
			unsigned int best=index;
			unsigned int j;
			for (j=index+1;j<mountssize;++j) {
				if (i->NextMountCloser(&mounts[j],this)) {
					best=j;
					if( SERVER&&(mounts[j].processed==Mount::FIRED||mounts[j].processed==Mount::PROCESSED))
						serverUnfireRequests.push_back(j);
					i->UnFire();
					i=&mounts[j];
				}
				else {
					if( SERVER&&(mounts[j].processed==Mount::FIRED||mounts[j].processed==Mount::PROCESSED))
						serverUnfireRequests.push_back(j);
					mounts[j].UnFire();
				}
				if (mounts[j].bank==false) {
					++j;
					break;
				}
			}
			counter=j-1;		 //will increment to the next one
			index=best;
		}

		const bool mis = isMissile(i->type);
		const bool locked_on = i->time_to_lock<=0;
		const bool lockable_weapon = i->type->LockTime>0;
		const bool autotracking_gun =(!mis)&&0!=(i->size&weapon_info::AUTOTRACKING)&&locked_on;
		const bool fire_non_autotrackers = (0==(weapon_type_bitmask&ROLES::FIRE_ONLY_AUTOTRACKERS));
		const bool locked_missile = (mis&&locked_on&&lockable_weapon);
		const bool missile_and_want_to_fire_missiles = (mis&&(weapon_type_bitmask&ROLES::FIRE_MISSILES));
		const bool gun_and_want_to_fire_guns =((!mis)&&(weapon_type_bitmask&ROLES::FIRE_GUNS));
		if(verbose_debug&&missile_and_want_to_fire_missiles&&locked_missile) {
			VSFileSystem::vs_fprintf (stderr,"\n about to fire locked missile \n");
		}
		bool want_to_fire=
			(fire_non_autotrackers||autotracking_gun||locked_missile)
			&&((ROLES::EVERYTHING_ELSE&weapon_type_bitmask&i->type->role_bits)||i->type->role_bits==0)
			&&((locked_on&&missile_and_want_to_fire_missiles)||gun_and_want_to_fire_guns);

		if ((*i).type->type==weapon_info::BEAM) {
			if ((*i).type->EnergyRate*SIMULATION_ATOM>energy) {
				// On server side send a PACKET TO ALL CLIENT TO NOTIFY UNFIRE
				// Including the one who fires to make sure it stops
				if( SERVER&&((*i).processed==Mount::FIRED||(*i).processed==Mount::PROCESSED))
					serverUnfireRequests.push_back(index);
				// NOT ONLY IN non-networking mode : anyway, the server will tell everyone including us to stop if not already done
				// if( !SERVER && Network==NULL)
				(*i).UnFire();
				continue;
			}
		}
		else {
			// Only in non-networking mode
			if (i->type->EnergyRate>energy) {
				if (!want_to_fire) {
					if( SERVER&&((*i).processed==Mount::FIRED||(*i).processed==Mount::PROCESSED))
						serverUnfireRequests.push_back(index);
					i->UnFire();
				}
				if ( Network==NULL)
					continue;

			}
		}

		if (want_to_fire) {
			// If in non-networking mode and mount fire has been accepted or if on server side
			if (Network!=NULL && (!SERVER) && i->processed!=Mount::ACCEPTED&&i->processed!=Mount::FIRED && i->processed!=Mount::REQUESTED && playernum>=0 && i->ammo!=0) {
				// Request a fire order to the server telling him the serial of the unit and the mount index (nm)
				if (mis) {
					missileFireRequests.push_back(index);
				}
				else {
					gunFireRequests.push_back(index);
				}
				// Mark the mount as fire requested
				i->processed = Mount::REQUESTED;
				// NETFIXME: REQUESTED was commented out.
			}
								 //projectile and beam weapons should be confirmed by server...not just fired off willy-nilly
			if( Network==NULL || SERVER || i->processed==Mount::ACCEPTED || preEmptiveClientFire(i->type)) {

				// If we are on server or if the weapon has been accepted for fire we fire
				if (i->Fire(this,owner==NULL?this:owner,mis,listen_to_owner)) {
					ObjSerial serid;

					if( missile_and_want_to_fire_missiles) {
						serid = getUniqueSerial();
						i->serial = serid;
					}
					else
						serid = 0;
					if( SERVER) {
						if (serid) {
							// One Serial ID per broadcast.  Not mush point in optimizing this.
							vector<int> indexvec;
							indexvec.push_back(index);
							VSServer->BroadcastFire( this->serial, indexvec, serid, this->energy, this->getStarSystem()->GetZone());
						}
						else {
							gunFireRequests.push_back(index);
						}
					}
					// We could only refresh energy on server side or in non-networking mode, on client side it is done with
					// info the server sends with ack for fire
					// FOR NOW WE TRUST THE CLIENT SINCE THE SERVER CAN REFUSE A FIRE
					// if( Network==NULL || SERVER)
					if (i->type->type==weapon_info::BEAM) {
						if (i->ref.gun)
						if ((!i->ref.gun->Dissolved())||i->ref.gun->Ready()) {
							energy -=i->type->EnergyRate*SIMULATION_ATOM;
						}

					}
					else if (isMissile(i->type)) {
						energy-=i->type->EnergyRate;
					}
					// IF WE REFRESH ENERGY FROM SERVER : Think to send the energy update to the firing client with ACK TO fireRequest
								 //fire only 1 missile at a time
					if (mis) weapon_type_bitmask &= (~ROLES::FIRE_MISSILES);
				}
			}
		}
		if (want_to_fire==false&&(i->processed==Mount::FIRED||i->processed==Mount::REQUESTED||i->processed==Mount::PROCESSED)) {
			i->UnFire();
			if (SERVER) {
				serverUnfireRequests.push_back(index);
			}
		}
	}
	if (!gunFireRequests.empty()) {
		if (SERVER) {
			VSServer->BroadcastFire( this->serial, gunFireRequests, 0, this->energy, this->getStarSystem()->GetZone());
		}
		else {
			char mis2 = false;
			Network[playernum].fireRequest( this->serial, gunFireRequests, mis2);
		}
	}
	if (SERVER && !serverUnfireRequests.empty()) {
		VSServer->BroadcastUnfire( this->serial, serverUnfireRequests, this->getStarSystem()->GetZone());
	}
	// Client missile requests can be grouped because clients only send a boolean, not a serial.
	if (!SERVER && !missileFireRequests.empty()) {
		char mis2 = true;
		Network[playernum].fireRequest( this->serial, missileFireRequests, mis2);
	}
}


const string Unit::getFgID()
{
	if(flightgroup!=NULL) {
		char buffer[32];
		sprintf(buffer,"-%d",flightgroup_subnumber);
		return flightgroup->name+buffer;
	}
	else {
		return fullname;
	}
};

void Unit::SetFaction (int faction)
{
	this->faction=faction;
	for (un_iter ui=getSubUnits();(*ui)!=NULL;++ui) {
		(*ui)->SetFaction(faction);
	}
}


//FIXME Daughter units should be able to be turrets (have y/p/r)
void Unit::SetResolveForces (bool ys)
{
	resolveforces = ys;
	/*
	for (int i=0;i<numsubunit;i++) {
	  subunits[i]->SetResolveForces (ys);
	}
	*/
}


void Unit::SetFg(Flightgroup * fg, int fg_subnumber)
{
	flightgroup=fg;
	flightgroup_subnumber=fg_subnumber;
}


void Unit::AddDestination (const std::string &dest)
{
	image->destination.push_back (dest);
}


const std::vector <std::string>& Unit::GetDestinations () const
{
	return image->destination;
}


float Unit::TrackingGuns(bool &missilelock)
{
	float trackingcone = 0;
	missilelock=false;
	for (int i=0;i<GetNumMounts();++i) {
		if (mounts[i].status==Mount::ACTIVE&&(mounts[i].size&weapon_info::AUTOTRACKING)) {
			trackingcone= computer.radar.trackingcone;
		}
		if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime>0&&mounts[i].time_to_lock<=0) {
			missilelock=true;
		}
	}
	return trackingcone;
}


void Unit::setAverageGunSpeed()
{
	float mrange=-1;
	float grange=-1;
	float speed=-1;
	bool beam=true;
	if (GetNumMounts()) {
		grange=0;
		speed=0;
		mrange=0;
		int nummt = 0;
		// this breaks the name, but... it _is_ more useful.
		for (int i=0;i<GetNumMounts();++i) {
			if (mounts[i].status==Mount::ACTIVE||mounts[i].status==Mount::INACTIVE) {
				if (isMissile(mounts[i].type)==false) {
					if (mounts[i].type->Range > grange) {
						grange=mounts[i].type->Range;
					}

					if (mounts[i].status==Mount::ACTIVE) {
						speed+=mounts[i].type->Speed;
						++nummt;
						beam&= (mounts[i].type->type==weapon_info::BEAM);
					}
				}
				else if(isMissile(mounts[i].type)) {
					if(mounts[i].type->Range > mrange) {
						mrange=mounts[i].type->Range;
					}
				}
			}
		}
		if(nummt) {
			if (beam)
				speed=FLT_MAX;
			else
				speed = speed/nummt;

		}
	}
	this->missilerange=mrange;
	this->gunrange=grange;
	this->gunspeed=speed;
}


QVector Unit::PositionITTS (const QVector& absposit,Vector velocity, float speed, bool steady_itts) const
{
	if (speed==FLT_MAX)
		return this->Position();
	float difficultyscale=1;
	if (g_game.difficulty<.99)
		GetVelocityDifficultyMult(difficultyscale);
	velocity = (cumulative_velocity.Scale(difficultyscale)-velocity);
	QVector posit (this->Position()-absposit);
	QVector curguess(posit);
	for (unsigned int i=0;i<3;++i) {
		float time = 0;
		if(speed>0.001) {
			time = curguess.Magnitude()/speed;
		}
		if (steady_itts) {
								 // ** jay
			curguess = posit+GetVelocity().Cast().Scale(time);
		}
		else {
			curguess = posit+velocity.Scale(time).Cast();
		}
	}
	return curguess+absposit;
}


static float tmpsqr (float x)
{
	return x*x;
}


float CloseEnoughCone (Unit * me)
{
	static float close_autotrack_cone = XMLSupport::parse_float (vs_config->getVariable ("physics","near_autotrack_cone",".9"));
	return close_autotrack_cone;
}


bool CloseEnoughToAutotrack (Unit * me, Unit * targ, float &cone)
{
	if (targ) {
		static float close_enough_to_autotrack = tmpsqr(XMLSupport::parse_float (vs_config->getVariable ("physics","close_enough_to_autotrack","4")));
		float dissqr = (me->curr_physical_state.position.Cast()-targ->curr_physical_state.position.Cast()).MagnitudeSquared();
		float movesqr=close_enough_to_autotrack*(me->prev_physical_state.position.Cast()-me->curr_physical_state.position.Cast()).MagnitudeSquared();
		if (dissqr<movesqr&&movesqr>0) {
			cone = CloseEnoughCone(me)*(movesqr-dissqr)/movesqr + 1*dissqr/movesqr;
			return true;
		}
	}
	return false;
}

// Caps at +/- 1 so as to account for floating point inaccuracies.
static inline float safeacos(float mycos) {
	if (mycos>1.)
		mycos=1.;
	if (mycos<-1.)
		mycos=-1;
	return acos(mycos);
}

float Unit::cosAngleTo (Unit * targ, float &dist, float speed, float range, bool turnmargin) const
{
	Vector Normal (cumulative_transformation_matrix.getR());
        Normalize(Normal);
	//   if (range!=FLT_MAX) {
	//     getAverageGunSpeed(speed,range);
	//   }
	QVector totarget (targ->PositionITTS(cumulative_transformation.position,cumulative_velocity, speed,false));
	totarget = totarget-cumulative_transformation.position;
	dist = totarget.Magnitude();

	// Trial code
	float turnlimit = tmpmax(tmpmax(computer.max_yaw_left,computer.max_yaw_right),tmpmax(computer.max_pitch_up,computer.max_pitch_down));
	float turnangle = SIMULATION_ATOM*tmpmax(turnlimit,tmpmax(SIMULATION_ATOM*.5*(limits.yaw+limits.pitch),sqrtf(AngularVelocity.i*AngularVelocity.i+AngularVelocity.j*AngularVelocity.j)));
	float ittsangle = safeacos(Normal.Cast().Dot(totarget.Scale(1./totarget.Magnitude())));
    QVector edgeLocation=(targ->cumulative_transformation_matrix.getP()*targ->rSize()+totarget);
    float radangle  = safeacos(edgeLocation.Cast().Scale(1./edgeLocation.Magnitude()).Dot(totarget.Normalize()));
	float rv        = ittsangle-radangle - (turnmargin?turnangle:0);

	float rsize = targ->rSize()+rSize();
	if ((!targ->GetDestinations().empty()&&jump.drive>=0)||(targ->faction==faction))
		rsize=0;				 //HACK so missions work well
	if (range != 0)
		dist = (dist-rsize)/range; else
		dist = 0;
	if (!FINITE(dist)||dist<0)
		dist=0;

	return (rv<0)?1:cos(rv);
}


float Unit::cosAngleFromMountTo (Unit * targ, float & dist) const
{
	float retval = -1;
	dist = FLT_MAX;
	float tmpcos;
	Matrix mat;
	for (int i=0;i<GetNumMounts();++i) {
		float tmpdist = .001;
		Transformation finaltrans (mounts[i].GetMountOrientation(),mounts[i].GetMountLocation().Cast());
		finaltrans.Compose (cumulative_transformation, cumulative_transformation_matrix);
		finaltrans.to_matrix (mat);
		Vector Normal (mat.getR());

		QVector totarget (targ->PositionITTS(finaltrans.position,cumulative_velocity, mounts[i].type->Speed,false));

		tmpcos = Normal.Dot (totarget.Cast());
		tmpdist = totarget.Magnitude();
		if (tmpcos>0) {
			tmpcos = tmpdist*tmpdist - tmpcos*tmpcos;
								 //one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
			tmpcos = targ->rSize()/tmpcos;
		}
		else {
			tmpcos /= tmpdist;
		}
								 //UNLIKELY DIV/0
		tmpdist /= mounts[i].type->Range;
		if (tmpdist < 1||tmpdist<dist) {
			if (tmpcos-tmpdist/2 > retval-dist/2) {
				dist = tmpdist;
				retval = tmpcos;
			}
		}
	}
	return retval;
}


#define PARANOIA .4
void Unit::Threaten (Unit * targ, float danger)
{
	if (!targ) {
		computer.threatlevel=danger;
		computer.threat.SetUnit (NULL);
	}
	else {
		if (targ->owner!=this&&this->owner!=targ&&danger>PARANOIA&&danger>computer.threatlevel) {
			computer.threat.SetUnit(targ);
			computer.threatlevel = danger;
		}
	}
}


std::string Unit::getCockpit () const
{
	return image->cockpitImage;
}


void Unit::Select()
{
	selected = true;
}


void Unit::Deselect()
{
	selected = false;
}


void disableSubUnits (Unit * uhn)
{
	Unit * un;
	for (un_iter i = uhn->getSubUnits();(un=*i)!=NULL;++i) {
		disableSubUnits(un);
	}
	for (unsigned int j=0;j<uhn->mounts.size();++j) {
		DestroyMount(&uhn->mounts[j]);
	}

}


un_iter Unit::getSubUnits ()
{
	return SubUnits.createIterator();
}


un_kiter Unit::viewSubUnits() const
{
	return SubUnits.constIterator();
}


void Unit::SetVisible(bool vis)
{
	if (vis) {
		invisible&=(~INVISCAMERA);
	}
	else {
		invisible|=INVISCAMERA;
	}
}


void Unit::SetAllVisible(bool vis)
{
	if (vis) {
		invisible&=(~INVISUNIT);
	}
	else {
		invisible|=INVISUNIT;
	}
}


void Unit::SetGlowVisible(bool vis)
{
	if (vis) {
		invisible&=(~INVISGLOW);
	}
	else {
		invisible|=INVISGLOW;
	}
}


float Unit::GetElasticity() {return .5;}

/***********************************************************************************/
/**** UNIT AI STUFF                                                                */
/***********************************************************************************/
void Unit::LoadAIScript(const std::string & s)
{
	//static bool init=false;
	//  static bool initsuccess= initPythonAI();
	if (s.find (".py")!=string::npos) {
		Order * ai = PythonClass <FireAt>::Factory (s);
		PrimeOrders (ai);
		return;
	}
	else {
		if (s.length()>0) {
			if (*s.begin()=='_') {
				mission->addModule (s.substr (1));
				PrimeOrders (new AImissionScript (s.substr(1)));
			}
			else {
				if (s=="ikarus") {
					PrimeOrders( new Orders::Ikarus ());
				}
				else {
					string ai_agg=s+".agg.xml";
					PrimeOrders( new Orders::AggressiveAI (ai_agg.c_str()));
				}
			}
		}
		else {
			PrimeOrders();
		}
	}
}


void Unit::eraseOrderType (unsigned int type)
{
	if (aistate) {
		aistate->eraseType(type);
	}
}


bool Unit::LoadLastPythonAIScript()
{
	Order * pyai = PythonClass <Orders::FireAt>::LastPythonClass();
	if (pyai) {
		PrimeOrders (pyai);
	}
	else if (!aistate) {
		PrimeOrders();
		return false;
	}
	return true;
}


bool Unit::EnqueueLastPythonAIScript()
{
	Order * pyai = PythonClass <Orders::FireAt>::LastPythonClass();
	if (pyai) {
		EnqueueAI (pyai);
	}
	else if (!aistate) {
		return false;
	}
	return true;
}


void Unit::PrimeOrders (Order * newAI)
{
	if (newAI) {
		if (aistate) {
			aistate->Destroy();
		}
		aistate = newAI;
		newAI->SetParent (this);
	}
	else {
		PrimeOrders();
	}
}


void Unit::PrimeOrders ()
{
	if (aistate) {
		aistate->Destroy();
		aistate=NULL;
	}
	aistate = new Order;		 //get 'er ready for enqueueing
	aistate->SetParent (this);
}


void Unit::PrimeOrdersLaunched()
{
	if (aistate) {
		aistate->Destroy();
		aistate=NULL;
	}
	//  aistate = new Orders::AggressiveAI ("interceptor.agg.xml"); // new Order; //get 'er ready for enqueueing
	Vector vec (0,0,10000);
	aistate = new ExecuteFor(new Orders::MatchVelocity(this->ClampVelocity(vec,true),Vector(0,0,0),true,true,false),4.0f);
	aistate->SetParent (this);
}


void Unit::SetAI(Order *newAI)
{
	newAI->SetParent(this);
	if (aistate) {
		aistate->ReplaceOrder (newAI);
	}
	else {
		aistate = newAI;
	}
}


void Unit::EnqueueAI(Order *newAI)
{
	newAI->SetParent(this);
	if (aistate) {
		aistate->EnqueueOrder (newAI);
	}
	else {
		aistate = newAI;
	}
}


void Unit::EnqueueAIFirst(Order *newAI)
{
	newAI->SetParent(this);
	if (aistate) {
		aistate->EnqueueOrderFirst (newAI);
	}
	else {
		aistate = newAI;
	}
}


void Unit::ExecuteAI()
{
	if (flightgroup) {
		Unit * leader = flightgroup->leader.GetUnit();
								 //no heirarchy in flight group
		if (leader?(flightgroup->leader_decision>-1)&&(leader->getFgSubnumber()>=getFgSubnumber()):true) {
			if (!leader) {
				flightgroup->leader_decision = flightgroup->nr_ships;
			}
			flightgroup->leader.SetUnit(this);
		}
		flightgroup->leader_decision--;

	}
	if(aistate) aistate->Execute();
	if (!SubUnits.empty()) {
		un_iter iter =getSubUnits();
		Unit * un;
		while ((un = *iter) != NULL) {
			un->ExecuteAI();	 //like dubya
			++iter;
		}
	}
}


string Unit::getFullAIDescription()
{
	if (getAIState()) {
		return getFgID()+":"+getAIState()->createFullOrderDescription(0).c_str();
	}
	else {
		return "no order";
	}
}


void Unit::getAverageGunSpeed (float & speed, float & range, float &mmrange) const
{
	speed =gunspeed;
	range= gunrange;
	mmrange=missilerange;
}


float Unit::getRelation (const Unit * targ) const
{
	return pilot->GetEffectiveRelationship (this, targ);
}


void Unit::setTargetFg(const string & primary,const string & secondary,const string & tertiary)
{
	target_fgid[0]=primary;
	target_fgid[1]=secondary;
	target_fgid[2]=tertiary;

	ReTargetFg(0);
}


void Unit::ReTargetFg(int which_target)
{
#if 0
	StarSystem *ssystem=_Universe->activeStarSystem();
	UnitCollection *unitlist=ssystem->getUnitList();
	un_iter uiter=unitlist->createIterator();

	GameUnit *found_target=NULL;
	int found_attackers=1000;
	for(GameUnit *other_unit = NULL;other_unit = *uiter;++uiter){
		string other_fgid=other_unit->getFgID();
		if(other_unit->matchesFg(target_fgid[which_target])) {
			// the other unit matches our primary target

			int num_attackers=other_unit->getNumAttackers();
			if(num_attackers<found_attackers) {
				// there's less ships attacking this target than the previous one
				found_target=other_unit;
				found_attackers=num_attackers;
				setTarget(found_target);
			}
		}
	}

	if(found_target==NULL) {
		// we haven't found a target yet, search again
		if(which_target<=1) {
			ReTargetFg(which_target+1);
		}
		else {
			// we can't find any target
			setTarget(NULL);
		}
	}
#endif
}


void Unit::SetTurretAI ()
{
	turretstatus=2;
	static bool talkinturrets = XMLSupport::parse_bool(vs_config->getVariable("AI","independent_turrets","false"));
	if (talkinturrets) {
		Unit * un;
		for(un_iter iter = getSubUnits();(un = *iter) != NULL;++iter){
			if (!CheckAccessory(un)) {
				un->EnqueueAIFirst (new Orders::FireAt(15.0f));
				un->EnqueueAIFirst (new Orders::FaceTarget (false,3));
			}
			un->SetTurretAI ();
		}
	}
	else {
		Unit * un;
		for(un_iter iter = getSubUnits();(un = *iter) != NULL;++iter){
			if (!CheckAccessory(un)) {
				if (un->aistate) {
					un->aistate->Destroy();
				}
				un->aistate = (new Orders::TurretAI());
				un->aistate->SetParent (un);
			}
			un->SetTurretAI ();
		}
	}
}


void Unit::DisableTurretAI ()
{
	turretstatus=1;
	Unit * un;
	for(un_iter iter = getSubUnits();(un = *iter) != NULL;++iter){
		if (un->aistate) {
			un->aistate->Destroy();
		}
		un->aistate = new Order; //get 'er ready for enqueueing
		un->aistate->SetParent (un);
		un->UnFire();
		un->DisableTurretAI ();
	}
}


/***********************************************************************************/
/**** UNIT_PHYSICS STUFF                                                           */
/***********************************************************************************/

extern signed char  ComputeAutoGuarantee ( Unit * un);
extern float getAutoRSize (Unit * orig,Unit * un, bool ignore_friend=false);
extern void SetShieldZero(Unit*);
double howFarToJump()
{
	static float tmp=XMLSupport::parse_float(vs_config->getVariable("physics","distance_to_warp","1000000000000.0"));
	return tmp;
}


QVector SystemLocation(const std::string & system)
{
	string xyz=_Universe->getGalaxyProperty(system,"xyz");
	QVector pos;
	if (xyz.size() && (sscanf(xyz.c_str(), "%lf %lf %lf", &pos.i, &pos.j, &pos.k)>=3)) {
		return pos;
	}
	else {
		return QVector(0,0,0);
	}
}


static std::string NearestSystem (const std::string & currentsystem,QVector pos)
{
	if (pos.i==0&&pos.j==0&&pos.k==0)
		return "";
	QVector posnorm=pos.Normalize();
	posnorm.Normalize();
	QVector cur = SystemLocation(currentsystem);
	if (cur.i==0&&cur.j==0&&cur.k==0) {
		return "";
	}
	double closest_distance=0.0;
	std::string closest_system;
	GalaxyXML::Galaxy * gal=_Universe->getGalaxy();
	GalaxyXML::SubHeirarchy * sectors= &gal->getHeirarchy();
	vsUMap<std::string,class GalaxyXML::SGalaxy>::iterator j,i =sectors->begin();

	for (;i!=sectors->end();++i) {
		GalaxyXML::SubHeirarchy * systems=&i->second.getHeirarchy();
		for (j=systems->begin();j!=systems->end();++j) {
			std::string place=j->second["xyz"];
			if (place.length()) {
				QVector pos2=QVector(0,0,0);
				sscanf(place.c_str(),"%lf %lf %lf",&pos2.i,&pos2.j,&pos2.k);
				if ((pos2.i!=0||pos2.j!=0||pos2.k!=0)&&(pos2.i!=cur.i||pos2.j!=cur.j||pos2.k!=cur.k)) {
					QVector dir=pos2-cur;
					QVector norm=dir;
					norm.Normalize();
					double test=posnorm.Dot(norm);
					if (test>.2) {
						//            test=1-test;
						double tmp=dir.MagnitudeSquared()/test/test/test;
						for (int cp=0;cp<_Universe->numPlayers();++cp) {
							std::string whereto=_Universe->AccessCockpit(cp)->GetNavSelectedSystem();
							if (whereto.length()==1+i->first.length()+j->first.length()) {
								if (whereto.substr(0,i->first.length())==i->first && whereto.substr(i->first.length()+1)==j->first) {
									static float SystemWarpTargetBonus=XMLSupport::parse_float(vs_config->getVariable("physics","target_distance_to_warp_bonus","1.33"));
									tmp/=SystemWarpTargetBonus;
								}
							}
						}
						if (tmp<closest_distance||closest_distance==0) {
							closest_distance=tmp;
							closest_system=i->first+"/"+j->first;
						}
					}
				}
			}
		}

	}
	return closest_system;
}


void Unit::UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc, Unit * superunit)
{
	static float VELOCITY_MAX=XMLSupport::parse_float(vs_config->getVariable ("physics","velocity_max","10000"));
	static float SPACE_DRAG=XMLSupport::parse_float(vs_config->getVariable ("physics","unit_space_drag","0.000000"));
	static float EXTRA_CARGO_SPACE_DRAG=XMLSupport::parse_float(vs_config->getVariable ("physics","extra_space_drag_for_cargo","0.005"));

	//Save information about when this happened
	unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
								 //Well, wasn't skipped actually, but...
	this->last_processed_sqs = cur_sim_frame;
	this->cur_sim_queue_slot = (cur_sim_frame+this->sim_atom_multiplier)%SIM_QUEUE_SIZE;

	if (maxhull < 0) {
		//if (this->owner)
		//this->Velocity = this->owner->Velocity; NOT ALLOWED TO DEREFERENCE OWNER

		this->Explode(true, 0);
	}
	Transformation old_physical_state = curr_physical_state;
	if (docked&DOCKING_UNITS) {
		PerformDockingOperations();
	}
	Repair();
	if (fuel<0)
		fuel=0;
	if (cloaking>=cloakmin) {
		static bool warp_energy_for_cloak=XMLSupport::parse_bool(vs_config->getVariable("physics","warp_energy_for_cloak","true"));
		if (image->cloakenergy*SIMULATION_ATOM>(warp_energy_for_cloak?warpenergy:energy)) {
			Cloak(false);		 //Decloak
		}
		else {
			SetShieldZero(this);
			if (image->cloakrate>0||cloaking==cloakmin) {
				if (warp_energy_for_cloak) {
					warpenergy-=(SIMULATION_ATOM*image->cloakenergy);
				}
				else {
					energy-=(SIMULATION_ATOM*image->cloakenergy);
				}
			}
			if (cloaking>cloakmin) {
				AUDAdjustSound (sound->cloak, cumulative_transformation.position,cumulative_velocity);
								 //short fix
				if ((cloaking==(2147483647)&&image->cloakrate>0)||(cloaking==cloakmin+1&&image->cloakrate<0)) {
					AUDStartPlaying (sound->cloak);
				}
								 //short fix
				cloaking-= (int)(image->cloakrate*SIMULATION_ATOM);
				if (cloaking<=cloakmin&&image->cloakrate>0) {
					//AUDStopPlaying (sound->cloak);
					cloaking=cloakmin;
				}
				if (cloaking<0&&image->cloakrate<0) {
					//AUDStopPlaying (sound->cloak);
								 //wraps short fix
					cloaking=-2147483647-1;
				}
			}
		}
	}

	// Only on server or non-networking
	// Do it everywhere -- "interpolation" for client-side.
	//  if( SERVER || Network==NULL)
	RegenShields();
	if (lastframe) {
		if (!(docked&(DOCKED|DOCKED_INSIDE)))
								 //the AIscript should take care
			prev_physical_state = curr_physical_state;
#ifdef FIX_TERRAIN
		if (planet) {
			if (!planet->dirty) {
				SetPlanetOrbitData (NULL);
			}
			else {
				planet->pps = planet->cps;
			}
		}
#endif
	}

	if (isUnit()==PLANETPTR) {
		((Planet *)this)->gravitate (uc);
	}
	else {
		if (resolveforces) {
								 //clamp velocity
			net_accel = ResolveForces (trans,transmat);

			if (Velocity.i>VELOCITY_MAX)
				Velocity.i=VELOCITY_MAX; else
				if (Velocity.i<-VELOCITY_MAX)
					Velocity.i=-VELOCITY_MAX;

			if (Velocity.j>VELOCITY_MAX)
				Velocity.j=VELOCITY_MAX; else
				if (Velocity.j<-VELOCITY_MAX)
					Velocity.j=-VELOCITY_MAX;

			if (Velocity.k>VELOCITY_MAX)
				Velocity.k=VELOCITY_MAX; else
				if (Velocity.k<-VELOCITY_MAX)
					Velocity.k=-VELOCITY_MAX;
		}
	}
	float difficulty;
	Cockpit * player_cockpit=GetVelocityDifficultyMult (difficulty);

	this->UpdatePhysics2( trans, old_physical_state, net_accel, difficulty, transmat, cum_vel, lastframe, uc);

	if (EXTRA_CARGO_SPACE_DRAG > 0) {
		int upgfac = FactionUtil::GetUpgradeFaction();
		if ((this->faction == upgfac) || (this->name=="eject") || (this->name=="Pilot"))
			Velocity = Velocity * (1 - EXTRA_CARGO_SPACE_DRAG);
	}

	if (SPACE_DRAG > 0)
		Velocity = Velocity * (1 - SPACE_DRAG);

	float dist_sqr_to_target=FLT_MAX;
	Unit * target = Unit::Target();
	bool increase_locking=false;
	if (target&&cloaking<0/*-1 or -32768*/) {
	if (target->isUnit()!=PLANETPTR) {
		Vector TargetPos (InvTransform (cumulative_transformation_matrix,(target->Position())).Cast());
		dist_sqr_to_target = TargetPos.MagnitudeSquared();
		TargetPos.Normalize();
		if (TargetPos.k>computer.radar.lockcone)
			increase_locking=true;
	}
	/* Update the velocity reference to the nearer significant unit/planet. */
	if (!computer.force_velocity_ref && activeStarSystem) {
		Unit *nextVelRef = activeStarSystem->nextSignificantUnit();
		if (nextVelRef) {
			if (computer.velocity_ref.GetUnit()) {
				double dist = UnitUtil::getSignificantDistance(this, computer.velocity_ref.GetUnit());
				double next_dist = UnitUtil::getSignificantDistance(this, nextVelRef);
				if (next_dist < dist) {
					computer.velocity_ref = nextVelRef;
				}
			} else {
				computer.velocity_ref = nextVelRef;
			}
		}
	}
}


static string LockingSoundName = vs_config->getVariable ("unitaudio","locking","locking.wav");
								 //enables spiffy wc2 torpedo music, default to normal though
static string LockingSoundTorpName = vs_config->getVariable ("unitaudio","locking_torp","locking.wav");
static int LockingSound = AUDCreateSoundWAV (LockingSoundName,true);
static int LockingSoundTorp = AUDCreateSoundWAV (LockingSoundTorpName,true);

bool locking=false;
bool touched=false;
for (int i=0;(int)i<GetNumMounts();++i) {
	//    if (increase_locking&&cloaking<0) {
	//      mounts[i].time_to_lock-=SIMULATION_ATOM;
	//    }

	if (((SERVER&&mounts[i].status==Mount::INACTIVE)||mounts[i].status==Mount::ACTIVE)&&cloaking<0&&mounts[i].ammo!=0) {
		if (player_cockpit) {
			touched=true;
		}
		if (increase_locking&&(dist_sqr_to_target<mounts[i].type->Range*mounts[i].type->Range)) {
			mounts[i].time_to_lock-=SIMULATION_ATOM;
			static bool ai_lock_cheat=XMLSupport::parse_bool(vs_config->getVariable ("physics","ai_lock_cheat","true"));
			if (!player_cockpit) {
				if (ai_lock_cheat) {
					mounts[i].time_to_lock=-1;
				}
			}
			else {
				int LockingPlay = LockingSound;

								 //enables spiffy wc2 torpedo music, default to normal though
				static bool LockTrumpsMusic =XMLSupport::parse_bool(vs_config->getVariable("unitaudio","locking_trumps_music","false"));
								 //enables spiffy wc2 torpedo music, default to normal though
				static bool TorpLockTrumpsMusic =XMLSupport::parse_bool(vs_config->getVariable("unitaudio","locking_torp_trumps_music","false"));

				if (mounts[i].type->LockTime>0) {
					static string LockedSoundName= vs_config->getVariable ("unitaudio","locked","locked.wav");
					static int LockedSound = AUDCreateSoundWAV (LockedSoundName,false);

					if (mounts[i].type->size==weapon_info::SPECIALMISSILE)
						LockingPlay = LockingSoundTorp; else
						LockingPlay = LockingSound;

					if (mounts[i].time_to_lock>-SIMULATION_ATOM&&mounts[i].time_to_lock<=0) {
						if (!AUDIsPlaying(LockedSound)) {
							UniverseUtil::musicMute(false);
							AUDStartPlaying(LockedSound);
							AUDStopPlaying(LockingSound);
							AUDStopPlaying(LockingSoundTorp);
						}
						AUDAdjustSound (LockedSound,Position(),GetVelocity());
					}
					else if (mounts[i].time_to_lock>0) {
						locking=true;
						if (!AUDIsPlaying(LockingPlay)) {
							if (LockingPlay == LockingSoundTorp)
								UniverseUtil::musicMute(TorpLockTrumpsMusic); else
								UniverseUtil::musicMute(LockTrumpsMusic);
							AUDStartPlaying(LockingSound);
						}
						AUDAdjustSound (LockingSound,Position(),GetVelocity());
					}
				}
			}
		}
		else {
			if (mounts[i].ammo!=0) {
				mounts[i].time_to_lock=mounts[i].type->LockTime;
			}
		}
	}
	else {
		if (mounts[i].ammo!=0) {
			mounts[i].time_to_lock=mounts[i].type->LockTime;
		}
	}
	if (mounts[i].type->type==weapon_info::BEAM) {
		if (mounts[i].ref.gun) {
			static bool must_lock_to_autotrack=XMLSupport::parse_bool(vs_config->getVariable("physics","must_lock_to_autotrack","true"));
			Unit * autotarg = ((mounts[i].size&weapon_info::AUTOTRACKING)&&(mounts[i].time_to_lock<=0)&&(player_cockpit==NULL||TargetLocked()||!must_lock_to_autotrack))?target:NULL;
			float trackingcone = computer.radar.trackingcone;
			if (CloseEnoughToAutotrack(this,target,trackingcone)) {
				if (autotarg) {
					if (computer.radar.trackingcone<trackingcone)
						trackingcone = computer.radar.trackingcone;
				}
				autotarg = target;
			}
			mounts[i].ref.gun->UpdatePhysics (cumulative_transformation, cumulative_transformation_matrix,autotarg,trackingcone, target,(HeatSink?HeatSink:1.0f)*mounts[i].functionality,this,superunit);
		}
	}
	else {
		mounts[i].ref.refire+=SIMULATION_ATOM*(HeatSink?HeatSink:1.0f)*mounts[i].functionality;
	}
	if (mounts[i].processed==Mount::FIRED) {
		Transformation t1;
		Matrix m1;
		t1=prev_physical_state;	 //a hack that will not work on turrets
		t1.Compose (trans,transmat);
		t1.to_matrix (m1);
		int autotrack=0;
		static bool must_lock_to_autotrack=XMLSupport::parse_bool(vs_config->getVariable("physics","must_lock_to_autotrack","true"));
		if ((0!=(mounts[i].size&weapon_info::AUTOTRACKING)&&((Network!=NULL&&!SERVER)||player_cockpit==NULL||TargetLocked()||!must_lock_to_autotrack))) {
			autotrack = computer.itts?2:1;
		}
		float trackingcone = computer.radar.trackingcone;
		if (CloseEnoughToAutotrack(this,target,trackingcone)) {
			if (autotrack) {
				if (trackingcone>computer.radar.trackingcone) {
					trackingcone = computer.radar.trackingcone;
				}
			}
			autotrack=2;
		}
		CollideMap::iterator hint[Unit::NUM_COLLIDE_MAPS];
		for (unsigned int locind =0;locind<Unit::NUM_COLLIDE_MAPS;++locind) {
			hint[locind]=(!is_null(superunit->location[locind]))?superunit->location[locind]:_Universe->activeStarSystem()->collidemap[locind]->begin();
		}

		if (!mounts[i].PhysicsAlignedFire (this,t1,m1,cumulative_velocity,(!isSubUnit()||owner==NULL)?this:owner,target,autotrack, trackingcone,hint)) {
			const weapon_info * typ = mounts[i].type;
			energy+=typ->EnergyRate*(typ->type==weapon_info::BEAM?SIMULATION_ATOM:1);
		}
		if (mounts[i].ammo==0&&isMissile(mounts[i].type), i) {
			//		  if (isPlayerStarship(this))
			//			  ToggleWeapon (true);
		}
	}
	else if (mounts[i].processed==Mount::UNFIRED||mounts[i].ref.refire>2*mounts[i].type->Refire()) {
		mounts[i].processed=Mount::UNFIRED;
		mounts[i].PhysicsAlignedUnfire();
	}
}


if (locking==false&&touched==true) {
	if (AUDIsPlaying(LockingSound)) {
		UniverseUtil::musicMute(false);
		AUDStopPlaying(LockingSound);
	}
	if (AUDIsPlaying(LockingSoundTorp)) {
		UniverseUtil::musicMute(false);
		AUDStopPlaying(LockingSoundTorp);
	}
}


bool dead=true;

UpdateSubunitPhysics(cumulative_transformation,cumulative_transformation_matrix,cumulative_velocity,lastframe,uc,superunit);
								 // can a unit get to another system without jumping?.
static bool warp_is_interstellar=XMLSupport::parse_bool (vs_config->getVariable ("physics","warp_is_interstellar","false"));
if (warp_is_interstellar&&(curr_physical_state.position.MagnitudeSquared()>howFarToJump()*howFarToJump()&&!isSubUnit())) {
	static bool direct=XMLSupport::parse_bool (vs_config->getVariable ("physics","direct_interstellar_journey","true"));
	bool jumpDirect = false;
	if (direct) {
		Cockpit *cp = _Universe->isPlayerStarship(this);
		if (NULL!=cp) {
			std::string sys = cp->GetNavSelectedSystem();
			if (!sys.empty()) {
				jumpDirect = true;
				_Universe->activeStarSystem()->JumpTo(this,NULL,sys,true,true);
			}
		}
	}
	if (!jumpDirect) {
		_Universe->activeStarSystem()->JumpTo(this,NULL,NearestSystem(_Universe->activeStarSystem()->getFileName(),curr_physical_state.position),true,true);
	}
}


// Really kill the unit only in non-networking or on server side
if (hull<0) {
	dead&= (image->explosion==NULL);
	if (dead)
		Kill();
}


else {
	//only do it in Unit::CollideAll
	/*if ((!isSubUnit())&&(!killed)&&(!(docked&DOCKED_INSIDE))) {
	  UpdateCollideQueue();
	}*/
	if (!isSubUnit()) {
		for (unsigned int locind=0;locind<Unit::NUM_COLLIDE_MAPS;++locind) {
			if (is_null(this->location[locind])) {
				this->getStarSystem()->collidemap[locind]->insert(Collidable(this));
			}
			else if (locind==Unit::UNIT_BOLT) {
								 // that update will propagate with the flatten
				this->getStarSystem()->collidemap[Unit::UNIT_BOLT]->changeKey(this->location[locind],Collidable(this));
			}
		}
	}
}


}

void Unit::UpdateSubunitPhysics (const Transformation &trans, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc, Unit * superunit)
{
	if (!SubUnits.empty()) {
		Unit * su;
		float backup=SIMULATION_ATOM;
		float basesimatom=(this->sim_atom_multiplier?backup/(float)this->sim_atom_multiplier:backup);
		unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
		for(un_iter iter = getSubUnits();(su = *iter) != NULL;++iter){
			if (this->sim_atom_multiplier&&su->sim_atom_multiplier) {
				//This ugly thing detects skipped frames.
				//This shouldn't happen during normal execution, as the interpolation will not be correct
				//when outside the expected range (that is, if the target queue slot is skipped).
				//BUT... this allows easy subunit simulation scattering by initializing cur_sim_frame
				//with random data.
								 //Normal crossing
				if (  ((su->last_processed_sqs<su->cur_sim_queue_slot)&&(cur_sim_frame>=su->cur_sim_queue_slot))
								 //Full round trip
					|| (su->last_processed_sqs==cur_sim_frame)
								 //Incomplete round trip - but including target frame
					||((su->last_processed_sqs>cur_sim_frame)&&((su->cur_sim_queue_slot<=cur_sim_frame)||(su->last_processed_sqs<su->cur_sim_queue_slot)))
				) {
					if (do_subunit_scheduling) {
						int priority=UnitUtil::getPhysicsPriority(su);
								 // Add some scattering
						priority = (priority+rand()%priority)/2;
						if (priority<1) priority=1;
						su->sim_atom_multiplier = this->sim_atom_multiplier*priority;
						if (su->sim_atom_multiplier > SIM_QUEUE_SIZE)
							su->sim_atom_multiplier = (SIM_QUEUE_SIZE/su->sim_atom_multiplier)*su->sim_atom_multiplier;
						if (su->sim_atom_multiplier < this->sim_atom_multiplier)
							su->sim_atom_multiplier = this->sim_atom_multiplier;
					} else su->sim_atom_multiplier = this->sim_atom_multiplier;

					SIMULATION_ATOM = basesimatom*(float)su->sim_atom_multiplier;
					Unit::UpdateSubunitPhysics(su,cumulative_transformation,cumulative_transformation_matrix,cumulative_velocity,lastframe,uc,superunit);
				}
			}
		}
		SIMULATION_ATOM = backup;
	}
}


void Unit::UpdateSubunitPhysics(Unit* subunit, const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool lastframe, UnitCollection *uc, Unit * superunit)
{
	subunit->UpdatePhysics(cumulative_transformation,cumulative_transformation_matrix,cumulative_velocity,lastframe,uc,superunit);
								 //short fix
	subunit->cloaking = (unsigned int) cloaking;
	if (hull<0) {
		subunit->Target(NULL);
		UnFire();				 //don't want to go off shooting while your body's splitting everywhere
		//DEPRECATEDsu->hull-=SIMULATION_ATOM;
	}
}
float CalculateNearestWarpUnit (const Unit *thus, float minmultiplier, Unit **nearest_unit, bool count_negative_warp_units) {
	static float autopilot_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_termination_distance","6000"));
	static float smallwarphack = XMLSupport::parse_float (vs_config->getVariable ("physics","minwarpeffectsize","100"));
	static float bigwarphack = XMLSupport::parse_float (vs_config->getVariable ("physics","maxwarpeffectsize","10000000"));
							 // Pi^2
	static float warpMultiplierMin=XMLSupport::parse_float(vs_config->getVariable("physics","warpMultiplierMin","9.86960440109"));
							 // C
	static float warpMultiplierMax=XMLSupport::parse_float(vs_config->getVariable("physics","warpMultiplierMax","300000000"));
							 // Pi^2 * C
	static float warpMaxEfVel=XMLSupport::parse_float(vs_config->getVariable("physics","warpMaxEfVel","2960881320"));
							 // Boundary between multiplier regions 1&2. 2 is "high" mult
	static double warpregion1=XMLSupport::parse_float(vs_config->getVariable("physics","warpregion1","5000000"));
							 // Boundary between multiplier regions 0&1 0 is mult=1
	static double warpregion0=XMLSupport::parse_float(vs_config->getVariable("physics","warpregion0","5000"));
							 // Mult at 1-2 boundary
	static double warpcruisemult=XMLSupport::parse_float(vs_config->getVariable("physics","warpcruisemult","5000"));
							 // degree of curve
	static double curvedegree=XMLSupport::parse_float(vs_config->getVariable("physics","warpcurvedegree","1.5"));
							 // coefficient so as to agree with above
	static double upcurvek=warpcruisemult/pow((warpregion1-warpregion0),curvedegree);
							 // inverse fractional effect of ship vs real big object
	static float def_inv_interdiction=1./XMLSupport::parse_float(vs_config->getVariable("physics","default_interdiction",".125"));
	Unit * planet;
	Unit * testthis=NULL;
	{
		NearestUnitLocator locatespec;
		findObjects(_Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY],thus->location[Unit::UNIT_ONLY],&locatespec);
		testthis=locatespec.retval.unit;
	}
	for (un_fiter iter = _Universe->activeStarSystem()->gravitationalUnits().fastIterator();(planet=*iter)||testthis;++iter) if (!planet||!planet->Killed()) {
		if (planet==NULL) {
			planet=testthis;
			testthis=NULL;
		}
		if (planet==thus) {
			continue;
		}
		float shiphack=1;
		if (planet->isUnit()!=PLANETPTR) {
			shiphack=def_inv_interdiction;
			if (planet->specInterdiction!=0&&planet->graphicOptions.specInterdictionOnline!=0&&(planet->specInterdiction>0||count_negative_warp_units)) {
				shiphack=1/fabs(planet->specInterdiction);
				if (thus->specInterdiction!=0&&thus->graphicOptions.specInterdictionOnline!=0) {
							 //only counters artificial interdiction ... or maybe it cheap ones shouldn't counter expensive ones!? or expensive ones should counter planets...this is safe now, for gameplay
					shiphack*=fabs(thus->specInterdiction);
				}
			}
		}
		float multipliertemp=1;
		float minsizeeffect = (planet->rSize()>smallwarphack)?planet->rSize():smallwarphack;
		float effectiverad = minsizeeffect*(1.0f+UniverseUtil::getPlanetRadiusPercent())+thus->rSize();
		if(effectiverad>bigwarphack){
			effectiverad=bigwarphack;
		}
		QVector dir=thus->Position()-planet->Position();
		double udist=dir.Magnitude();
		float sigdist=UnitUtil::getSignificantDistance(thus,planet);
		if(planet->isPlanet()&&udist<(1<<28)){ // If distance is viable as a float approximation and it's an actual celestial body
			udist = sigdist;
		}
		//QVector veldiff=thus->Velocity*thus->graphicOptions.WarpFieldStrength-planet->Velocity*planet->graphicOptions.WarpFieldStrength;
		//double velproj=veldiff.Dot(dir*(1./udist));OBSOLETE WITH NEW SPEC CALCULATIONS
		int itercount=0;
		do {
							 //+(velproj<0?velproj*SIMULATION_ATOM:0);
			double dist=udist;
			if (dist<0) dist=0;
			dist*=shiphack;
			if (dist>(effectiverad+warpregion0)) {
				multipliertemp=pow((dist-effectiverad-warpregion0),curvedegree)*upcurvek;
			}
			else {
				multipliertemp=1;
				//minmultiplier=1;
			}
			if (multipliertemp<minmultiplier) {
				minmultiplier=multipliertemp;
				*nearest_unit=planet;
				//eventually use new multiplier to compute
			}else break;
		}while(0);			 //++itercount<=1); only repeat 1 iter right now
		if (!testthis)
			break;			 //don't want the ++
	}
	return minmultiplier;
}

void Unit::AddVelocity(float difficulty)
{
    Vector VelocityRef(0,0,0);
    {
        Unit * vr=computer.velocity_ref.GetUnit();
        if (vr)
            VelocityRef=vr->cumulative_velocity;
    }
						 // for the heck of it.
	static float humanwarprampuptime=XMLSupport::parse_float (vs_config->getVariable ("physics","warprampuptime","5"));
								 // for the heck of it.
	static float compwarprampuptime=XMLSupport::parse_float (vs_config->getVariable ("physics","computerwarprampuptime","10"));
	static float warprampdowntime=XMLSupport::parse_float (vs_config->getVariable ("physics","warprampdowntime","0.5"));
	Vector v=Velocity-VelocityRef;
        float len=v.Magnitude();
        float lastWarpField=graphicOptions.WarpFieldStrength;
        if (len>.01)//only get velocity going in DIRECTIOn of cumulative transformation for warp calc...
          v=v*(cumulative_transformation_matrix.getR().Dot(v*(1./len)));
	bool  playa=_Universe->isPlayerStarship(this)?true:false;
	float warprampuptime=playa?humanwarprampuptime:compwarprampuptime;
								 // Warp Turning on/off
	if(graphicOptions.WarpRamping) {
								 // Warp Turning on
		if(graphicOptions.InWarp==1) {
			graphicOptions.RampCounter=warprampuptime;
		}						 //Warp Turning off
		else {
			graphicOptions.RampCounter=warprampdowntime;
		}
		graphicOptions.WarpRamping=0;
	}

	if(graphicOptions.InWarp==1||graphicOptions.RampCounter!=0) {
		static float autopilot_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_termination_distance","6000"));
		static float smallwarphack = XMLSupport::parse_float (vs_config->getVariable ("physics","minwarpeffectsize","100"));
		static float WARPMEMORYEFFECT = XMLSupport::parse_float (vs_config->getVariable ("physics","WarpMemoryEffect","0.9"));
								 // Pi^2
		static float warpMultiplierMin=XMLSupport::parse_float(vs_config->getVariable("physics","warpMultiplierMin","9.86960440109"));
								 // C
		static float warpMultiplierMax=XMLSupport::parse_float(vs_config->getVariable("physics","warpMultiplierMax","300000000"));
								 // Pi^2 * C
		static float warpMaxEfVel=XMLSupport::parse_float(vs_config->getVariable("physics","warpMaxEfVel","2960881320"));
								 // Boundary between multiplier regions 1&2. 2 is "high" mult
		static double warpregion1=XMLSupport::parse_float(vs_config->getVariable("physics","warpregion1","5000000"));
								 // Boundary between multiplier regions 0&1 0 is mult=1
		static double warpregion0=XMLSupport::parse_float(vs_config->getVariable("physics","warpregion0","5000"));
								 // Mult at 1-2 boundary
		static double warpcruisemult=XMLSupport::parse_float(vs_config->getVariable("physics","warpcruisemult","5000"));
								 // degree of curve
		static double curvedegree=XMLSupport::parse_float(vs_config->getVariable("physics","warpcurvedegree","1.5"));
								 // coefficient so as to agree with above
		static double upcurvek=warpcruisemult/pow((warpregion1-warpregion0),curvedegree);
								 // inverse fractional effect of ship vs real big object
		static float def_inv_interdiction=1./XMLSupport::parse_float(vs_config->getVariable("physics","default_interdiction",".125"));
		float minmultiplier=warpMultiplierMax*graphicOptions.MaxWarpMultiplier;
		Unit * nearest_unit=NULL;
		minmultiplier=CalculateNearestWarpUnit(this,minmultiplier,&nearest_unit,true);
		float rampmult=1;
		if(graphicOptions.RampCounter!=0) {
			graphicOptions.RampCounter-=SIMULATION_ATOM;
			if(graphicOptions.RampCounter<=0) {
				graphicOptions.RampCounter=0;
			}
			if (graphicOptions.InWarp==0&&graphicOptions.RampCounter>warprampdowntime) {
				graphicOptions.RampCounter=(1-graphicOptions.RampCounter/warprampuptime)*warprampdowntime;
			}
			if (graphicOptions.InWarp==1&&graphicOptions.RampCounter>warprampuptime) {
				graphicOptions.RampCounter=warprampuptime;
			}
			rampmult=(graphicOptions.InWarp)?1.0-((graphicOptions.RampCounter/warprampuptime)*(graphicOptions.RampCounter/warprampuptime)):(graphicOptions.RampCounter/warprampdowntime)*(graphicOptions.RampCounter/warprampdowntime);
		}
		if(minmultiplier<warpMultiplierMin*graphicOptions.MinWarpMultiplier) {
			minmultiplier=warpMultiplierMin*graphicOptions.MinWarpMultiplier;
		}
		if(minmultiplier>warpMultiplierMax*graphicOptions.MaxWarpMultiplier) {
								 //SOFT LIMIT
			minmultiplier=warpMultiplierMax*graphicOptions.MaxWarpMultiplier;
		}
		minmultiplier*=rampmult;
		if (minmultiplier < 1) {
			minmultiplier=1;
		}
		v*=minmultiplier;
		float vmag=sqrt(v.i*v.i+v.j*v.j+v.k*v.k);
		if(vmag>warpMaxEfVel) {
			v*=warpMaxEfVel/vmag;// HARD LIMIT
			minmultiplier*=warpMaxEfVel/vmag;
		}
                graphicOptions.WarpFieldStrength=minmultiplier;
	}
	else {
		graphicOptions.WarpFieldStrength=1;
                //not any more? lastWarpField=1;
	}
        if (graphicOptions.WarpFieldStrength!=1.0)
          v=GetWarpVelocity();
        else
          v=Velocity;
	static float WARPMEMORYEFFECT = XMLSupport::parse_float (vs_config->getVariable ("physics","WarpMemoryEffect","0.9"));
	graphicOptions.WarpFieldStrength=lastWarpField*WARPMEMORYEFFECT+(1.0-WARPMEMORYEFFECT)*graphicOptions.WarpFieldStrength;
	curr_physical_state.position = curr_physical_state.position +  (v*SIMULATION_ATOM*difficulty).Cast();
	/*
	if (!is_null(location)&&activeStarSystem){
	  location=activeStarSystem->collidemap->changeKey(location,Collidable(this));// do we need this?
	*/
	// now we do this later in update physics
	//I guess you have to, to be robust}
}


void Unit::UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc)
{

	// NETFIXME: used to check for (!cp):
	//	if( (Network==NULL && !SERVER) || (Network!=NULL && cp && !SERVER) || (SERVER && !cp))

	Cockpit * cp = _Universe->isPlayerStarship( this);
	// Only in non-networking OR networking && is a player OR SERVER && not a player
	if( (Network==NULL && !SERVER) || (Network!=NULL && cp && !SERVER) || (SERVER)) {
		if(AngularVelocity.i||AngularVelocity.j||AngularVelocity.k) {
			Rotate (SIMULATION_ATOM*(AngularVelocity));
		}
	}

	// NETFIXME: used to check for (!cp):
	//	if( SERVER && Network!=NULL && !cp)

	// SERVERSIDE ONLY : If it is not a player, it is a unit controlled by server so compute changes
	if( SERVER) {
		AddVelocity(difficulty);

		cumulative_transformation = curr_physical_state;
		cumulative_transformation.Compose (trans,transmat);
		cumulative_transformation.to_matrix (cumulative_transformation_matrix);
		cumulative_velocity = TransformNormal (transmat,Velocity)+cum_vel;
	}
}


static float tempmin (float a, float b)
{
	return a>b?b:a;
}


static QVector RealPosition (Unit * un)
{
	if (un->isSubUnit())
		return un->Position();
	return un->LocalPosition();
}


static QVector AutoSafeEntrancePoint (const QVector start, float rsize,Unit * goal)
{
	QVector def = UniverseUtil::SafeEntrancePoint(start,rsize);
	float bdis = (def-RealPosition(goal)).MagnitudeSquared();
	for (int i=-1;i<=1;++i) {
		for (int j=-1;j<=1;++j) {
			for (int k=-1;k<=1;k+=2) {
				QVector delta(i,j,k);delta.Normalize();
				QVector tmp =RealPosition(goal)+delta*(goal->rSize()+rsize);
				tmp=UniverseUtil::SafeEntrancePoint (tmp,rsize);
				float tmag = (tmp-RealPosition(goal)).MagnitudeSquared();
				if (tmag<bdis) {
					bdis = tmag;
					def = tmp;
				}
			}
		}
	}
	return def;
}


float globQueryShell (QVector pos,QVector dir,float rad);
std::string GenerateAutoError(Unit * me,Unit * targ)
{
	if (UnitUtil::isAsteroid(targ)) {
		static std::string err=XMLSupport::escaped_string(vs_config->getVariable("graphics","hud","AsteroidsNearMessage","#ff0000Asteroids Near#000000"));
		return err;
	}
	if (targ->isPlanet()) {
		static std::string err=XMLSupport::escaped_string(vs_config->getVariable("graphics","hud","PlanetNearMessage","#ff0000Planetary Hazard Near#000000"));
		return err;
	}
	if(targ->getRelation(me)<0) {
		static std::string err=XMLSupport::escaped_string(vs_config->getVariable("graphics","hud","EnemyNearMessage","#ff0000Enemy Near#000000"));
		return err;
	}
	static std::string err=XMLSupport::escaped_string(vs_config->getVariable("graphics","hud","StarshipNearMessage","#ff0000Starship Near#000000"));
	return err;
}


bool Unit::AutoPilotToErrorMessage (Unit * target, bool ignore_energy_requirements, std::string&failuremessage,int recursive_level)
{
	static bool auto_valid = XMLSupport::parse_bool (vs_config->getVariable ("physics","insystem_jump_or_timeless_auto-pilot","false"));
	if(!auto_valid) {
		static std::string err="No Insystem Jump";
		failuremessage=err;
		return false;
	}

	if (target->isUnit()==PLANETPTR) {
		un_iter i = target->getSubUnits();
		Unit * targ =*i;
		if (targ&&0==targ->graphicOptions.FaceCamera)
			return AutoPilotToErrorMessage(targ,ignore_energy_requirements,failuremessage,recursive_level);
	}
	//static float insys_jump_cost = XMLSupport::parse_float (vs_config->getVariable ("physics","insystem_jump_cost",".1"));
	if (warpenergy<jump.insysenergy) {
		if (!ignore_energy_requirements)
			return false;
	}
	signed char Guaranteed = ComputeAutoGuarantee (this);
	if (Guaranteed==Mission::AUTO_OFF) {
		return false;
	}
	static float autopilot_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_termination_distance","6000"));
	static float atd_no_enemies=XMLSupport::parse_float(vs_config->getVariable ("physics","auto_pilot_termination_distance_no_enemies",vs_config->getVariable ("physics","auto_pilot_termination_distance","6000")));
	static float autopilot_no_enemies_multiplier=XMLSupport::parse_float(vs_config->getVariable ("physics","auto_pilot_no_enemies_distance_multiplier","4"));
	//  static float autopilot_p_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_planet_termination_distance","60000"));
	if (isSubUnit()) {
		static std::string err="Return To Cockpit for Auto";
		failuremessage=err;
		return false;			 //we can't auto here;
	}
	StarSystem * ss = activeStarSystem;
	if (ss==NULL) {
		ss = _Universe->activeStarSystem();
	}

	Unit * un=NULL;
	QVector start (Position());
	QVector end (RealPosition(target));
	float totallength = (start-end).Magnitude();
	bool nanspace=false;
	if (!FINITE(totallength)) {
		nanspace=true;
		start=QVector(100000000.0,100000000.0,10000000000000.0);
		totallength = (start-end).Magnitude();
		if (!FINITE(totallength)) {
			end=QVector(200000000.0,100000000.0,10000000000000.0);;
			totallength=(start-end).Magnitude();
		}
	}
	QVector endne(end);

	float totpercent=1;
	if (totallength>1) {
		//    float apt = (target->isUnit()==PLANETPTR&&target->GetDestinations().empty())?autopilot_p_term_distance:autopilot_term_distance;
		float apt = (target->isUnit()==PLANETPTR)?(autopilot_term_distance+target->rSize()*UniverseUtil::getPlanetRadiusPercent()):autopilot_term_distance;
		float aptne=(target->isUnit()==PLANETPTR)?(atd_no_enemies+target->rSize()*UniverseUtil::getPlanetRadiusPercent()):atd_no_enemies;
		float percent = (getAutoRSize(this,this)+rSize()+target->rSize()+apt)/totallength;
		float percentne = (getAutoRSize(this,this)+rSize()+target->rSize()+aptne)/totallength;
		if (percentne>1) {
			endne=start;

		}
		else {
			endne=start*percentne+end*(1-percentne);
		}
		if (percent>1) {
			end=start;
			totpercent=0;
		}
		else {
			totpercent*=(1-percent);
			end = start*percent+end*(1-percent);
		}
	}
	bool ok=true;

	static bool teleport_autopilot= XMLSupport::parse_bool(vs_config->getVariable("physics","teleport_autopilot","true"));
	bool unsafe=false;
	if ((!teleport_autopilot)&&(!nanspace)) {
		if (Guaranteed==Mission::AUTO_NORMAL&&CloakVisible()>.5) {
			bool ignore_friendlies=true;
			for (un_iter i=ss->getUnitList().createIterator(); (un=*i)!=NULL; ++i) {
				static bool canflythruplanets= XMLSupport::parse_bool(vs_config->getVariable("physics","can_auto_through_planets","true"));
				if ((!(un->isUnit()==PLANETPTR&&canflythruplanets))&&un->isUnit()!=NEBULAPTR && (!UnitUtil::isSun(un))) {
					if (un!=this&&un!=target) {
						float tdis=(start-un->Position()).Magnitude()-rSize()-un->rSize();
						float nedis=(end-un->Position()).Magnitude()-rSize()-un->rSize();
						float trad=getAutoRSize(this,un,ignore_friendlies)+getAutoRSize (this,this,ignore_friendlies);
						if (tdis<=trad) {
							failuremessage=GenerateAutoError(this,un);
							return false;
						}
						if ((nedis<trad*autopilot_no_enemies_multiplier||tdis<=trad*autopilot_no_enemies_multiplier)&&un->getRelation(this)<0) {
							unsafe =true;
							failuremessage=GenerateAutoError(this,un);
						}
						float intersection = globQueryShell (start-un->Position(),end-start,getAutoRSize (this,un,ignore_friendlies)+un->rSize());
						if (intersection>0) {
							unsafe=true;
							end = start+ (end-start)*intersection;
							totpercent*=intersection;
							ok=false;
							failuremessage=GenerateAutoError(this,un);
						}
					}
				}
			}
		}
	}
	else if (!nanspace) {
		//just make sure we aren't in an asteroid field
		Unit * un;
		for (un_iter i=ss->getUnitList().createIterator(); (un=*i)!=NULL; ++i) {
			if (UnitUtil::isAsteroid(un)) {
				static float minasteroiddistance = XMLSupport::parse_float(vs_config->getVariable("physics","min_asteroid_distance","-100"));
				if (UnitUtil::getDistance(this,un)<minasteroiddistance) {
					failuremessage=GenerateAutoError(this,un);
					return false;//no auto in roid field
				}
			}
		}

	}
	bool nowhere=false;

	if (this!=target) {
		if ((end-start).MagnitudeSquared()<rSize()*rSize()) {
			failuremessage=XMLSupport::escaped_string(vs_config->getVariable("graphics","hud","AlreadyNearMessage","#ff0000Already Near#000000"));
			return false;
		}
		warpenergy-=totpercent*jump.insysenergy;
		if (unsafe==false&&totpercent==0)
			end=endne;
		QVector sep (UniverseUtil::SafeEntrancePoint(end,rSize()));
		if ((sep-end).MagnitudeSquared()>16*rSize()*rSize()) {
			//DOn't understand why rsize is so bigsep = AutoSafeEntrancePoint (end,(RealPosition(target)-end).Magnitude()-target->rSize(),target);
			sep = AutoSafeEntrancePoint (end,rSize(),target);
		}
		if ((sep-RealPosition(target)).MagnitudeSquared()>(RealPosition(this)-RealPosition(target)).MagnitudeSquared()) {
			sep= RealPosition(this);
			nowhere=true;
		}

		static bool auto_turn_towards =XMLSupport::parse_bool(vs_config->getVariable ("physics","auto_turn_towards","true"));
		if (auto_turn_towards) {
			for (int i=0;i<3;++i) {
				Vector methem(RealPosition(target).Cast()-sep.Cast());
				methem.Normalize();
				Vector p,q,r;
				GetOrientation(p,q,r);
				p=methem.Cross(r);
				float theta = p.Magnitude();
				if (theta*theta>.00001) {
					p*= (asin (theta)/theta);
					Rotate(p);
					GetOrientation (p,q,r);
				}
				if (r.Dot(methem)<0) {
					Rotate (p*(PI/theta));
				}
				Velocity=methem*Velocity.Magnitude();
			}
		}
		static string insys_jump_ani = vs_config->getVariable ("graphics","insys_jump_animation","warp.ani");
		if (

			//nowhere==false&&

		insys_jump_ani.length()) {
			static bool docache=true;
			if (docache) {
				UniverseUtil::cacheAnimation (insys_jump_ani);
				docache=false;
			}
			static float insys_jump_ani_size = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_animation_size","4"));
			static float insys_jump_ani_growth = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_animation_growth",".99"));
			UniverseUtil::playAnimationGrow (insys_jump_ani,RealPosition(this),rSize()*insys_jump_ani_size,insys_jump_ani_growth);

			Vector v(GetVelocity());
			v.Normalize();
			Vector p,q,r;GetOrientation(p,q,r);
			static float sec = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_ani_second_ahead","4"));
			UniverseUtil::playAnimationGrow (insys_jump_ani,sep+GetVelocity()*sec+v*rSize(),rSize()*8,.97);
			UniverseUtil::playAnimationGrow (insys_jump_ani,sep+GetVelocity()*sec+2*v*rSize()+r*4*rSize(),rSize()*16,.97);
		}
		static bool warptrail = XMLSupport::parse_bool (vs_config->getVariable ("graphics","warp_trail","true"));
		if (warptrail&&(!nowhere)) {
			static float warptrailtime = XMLSupport::parse_float (vs_config->getVariable ("graphics","warp_trail_time","20"));
			AddWarp(this,RealPosition(this),warptrailtime);
		}
		//    sep =UniverseUtil::SafeEntrancePoint (sep);
		if (!nowhere)
			SetCurPosition(sep);
		Cockpit * cp;
		if ((cp=_Universe->isPlayerStarship (this))!=NULL) {

			std::string followermessage;
			if (getFlightgroup()!=NULL) {
				Unit * other=NULL;
				if (recursive_level>0)
				for (un_iter ui=ss->getUnitList().createIterator(); NULL!=(other = *ui); ++ui) {
					Flightgroup * ff = other->getFlightgroup();
					bool leadah=(ff==getFlightgroup());
					if (ff) {
						if (ff->leader.GetUnit()==this) {
							leadah=true;
						}
					}
					Order * otherord = other->getAIState();
					if (otherord)
					if (otherord->PursueTarget (this,leadah)) {
						other->AutoPilotToErrorMessage(this,ignore_energy_requirements,followermessage,recursive_level-1);
						if (leadah) {
							if (NULL==_Universe->isPlayerStarship (other)) {
								other->SetPosition(AutoSafeEntrancePoint (LocalPosition(),other->rSize()*1.5,other));
							}
						}
					}

				}
			}

		}
	}
	return ok;
}


extern void ActivateAnimation(Unit * jp);
void TurnJumpOKLightOn(Unit * un, Cockpit * cp)
{
	if (cp) {
		if (un->GetWarpEnergy()>=un->GetJumpStatus().energy) {
			if (un->GetJumpStatus().drive>-2) {
				cp->jumpok=1;
			}
		}
	}
}


void Unit::DecreaseWarpEnergy(bool insys, float time)
{
	static float bleedfactor = XMLSupport::parse_float(vs_config->getVariable("physics","warpbleed","20"));
	static bool WCfuelhack = XMLSupport::parse_bool(vs_config->getVariable("physics","fuel_equals_warp","false"));
	if (WCfuelhack)
		this->warpenergy = this->fuel;
	this->warpenergy-=(insys?jump.insysenergy/bleedfactor:jump.energy)*time;
	if (this->warpenergy < 0)
		this->warpenergy = 0;

	if (WCfuelhack)
		this->fuel = this->warpenergy;
}


void Unit::IncreaseWarpEnergy(bool insys, float time)
{
	static bool WCfuelhack = XMLSupport::parse_bool(vs_config->getVariable("physics","fuel_equals_warp","false"));
	if (WCfuelhack)
		this->warpenergy = this->fuel;
	this->warpenergy+=(insys?jump.insysenergy:jump.energy)*time;
	if (this->warpenergy > this->maxwarpenergy)
		this->warpenergy = this->maxwarpenergy;
	if (WCfuelhack)
		this->fuel = this->warpenergy;
}


bool Unit::jumpReactToCollision (Unit * smalle)
{
	static bool ai_jump_cheat=XMLSupport::parse_bool(vs_config->getVariable("AI","jump_without_energy","false"));
	static bool nojumpinSPEC=XMLSupport::parse_bool(vs_config->getVariable("physics","noSPECJUMP","true"));
	bool SPEC_interference=(NULL!=_Universe->isPlayerStarship(smalle))?smalle->graphicOptions.InWarp&&nojumpinSPEC:(NULL!=_Universe->isPlayerStarship(this))?graphicOptions.InWarp&&nojumpinSPEC:false;
								 //only allow big with small
	if (!GetDestinations().empty()) {
		Cockpit * cp = _Universe->isPlayerStarship(smalle);
		if(!SPEC_interference||image->forcejump){
			TurnJumpOKLightOn(smalle,cp);
		} else {
			return false;
		}
		//ActivateAnimation(this);
								 // we have a drive
		if ((!SPEC_interference && (smalle->GetJumpStatus().drive>=0&&
								 // we have power
			(smalle->warpenergy>=smalle->GetJumpStatus().energy
								 // or we're being cheap
			||(ai_jump_cheat&&cp==NULL)
			)))
		||image->forcejump) {	 // or the jump is being forced?
			//NOW done in star_system_generic.cpp before TransferUnitToSystem smalle->warpenergy-=smalle->GetJumpStatus().energy;
			int dest = smalle->GetJumpStatus().drive;
			if (dest<0)
				dest=0;
			smalle->DeactivateJumpDrive();
			Unit * jumppoint = this;

			if( SERVER)
				VSServer->sendJump( smalle,this,GetDestinations()[dest%GetDestinations().size()]);
			else
				_Universe->activeStarSystem()->JumpTo (smalle, jumppoint, GetDestinations()[dest%GetDestinations().size()]);
			return true;
		}
		/* NOT NECESSARY ANYMORE SINCE THE CLIENT ONLY ASK FOR AUTH WITHOUT EXPECTING AN ANSWER
		else
		{
			if( SERVER)
				VSServer->sendJump( this->serial, false);
		}
		*/
		return true;
	}
	if (!smalle->GetDestinations().empty()) {
		Cockpit * cp = _Universe->isPlayerStarship(this);
		if(!SPEC_interference||smalle->image->forcejump){
			TurnJumpOKLightOn(this,cp);
		} else {
			return false;
		}
		//  ActivateAnimation(smalle);
		if ((!SPEC_interference
            &&((GetJumpStatus().drive>=0&&
			    (warpenergy>=GetJumpStatus().energy
			     ||(ai_jump_cheat&&cp==NULL)
			))))
		||smalle->image->forcejump) {
			warpenergy-=GetJumpStatus().energy;
			DeactivateJumpDrive();
			Unit * jumppoint = smalle;
			if( SERVER)
				VSServer->sendJump( this,smalle,smalle->GetDestinations()[GetJumpStatus().drive%smalle->GetDestinations().size()]);
			else
				_Universe->activeStarSystem()->JumpTo (this, jumppoint, smalle->GetDestinations()[GetJumpStatus().drive%smalle->GetDestinations().size()]);
			return true;
		}
		/*
		else
		{
			if( SERVER)
				VSServer->sendJump( this->serial, smalle->GetSerial(), false);
		}
		*/
		//not sure why you'd jump here... it's just a blooddddy else statementtttt
		return true;
	}
	else {
		/*
		if(0&& SERVER)
			VSServer->sendJump( this->serial, smalle->GetSerial(), false);
		*/
		//didn't even TRY to jump here--don't go about sending anything, d'oh!
	}
	return false;
}


Cockpit * Unit::GetVelocityDifficultyMult(float &difficulty) const
{
	difficulty=1;
	Cockpit * player_cockpit=_Universe->isPlayerStarship(this);
	if ((player_cockpit)==NULL) {
		static float exp = XMLSupport::parse_float (vs_config->getVariable ("physics","difficulty_speed_exponent",".2"));
		difficulty = pow(g_game.difficulty,exp);
	}
	return player_cockpit;
}


void Unit::Rotate (const Vector &axis)
{
	double theta = axis.Magnitude();
	double ootheta=0;
	if( theta==0) return;
	ootheta = 1/theta;
	float s = cos (theta * .5);
	Quaternion rot = Quaternion(s, axis * (sinf (theta*.5)*ootheta));

	if(theta < 0.0001) {
		rot = identity_quaternion;
	}
	curr_physical_state.orientation *= rot;

	if (limits.limitmin>-1) {
		Matrix mat;
		curr_physical_state.orientation.to_matrix (mat);
		if (limits.structurelimits.Dot (mat.getR())<limits.limitmin) {
			curr_physical_state.orientation=prev_physical_state.orientation;
			//VSFileSystem::vs_fprintf (stderr,"wierd case... with an i before the e\n", mat.getR().i,mat.getR().j,mat.getR().k);

		}
	}

}


void Unit::FireEngines (const Vector &Direction/*unit vector... might default to "r"*/,
float FuelSpeed,
float FMass)
{
	//mass -= FMass; //fuel is sent out Now we separated mass and fuel
	static float fuelpct=XMLSupport::parse_float(vs_config->getVariable("physics","FuelUsage","1"));
	fuel -= fuelpct*FMass;
	if (fuel <0) {

		FMass +=fuel;
		//mass -= fuel;
		fuel = 0;				 //ha ha!
	}
	NetForce += Direction *(FuelSpeed *FMass/GetElapsedTime());
}


								 //applies a force for the whole gameturn upon the center of mass
void Unit::ApplyForce(const Vector &Vforce)
{
	if (FINITE (Vforce.i)&&FINITE(Vforce.j)&&FINITE(Vforce.k)) {
		NetForce += Vforce;
	}
	else {
		UNIT_LOG(logvs::NOTICE, "fatal force");
	}
}


								 //applies a force for the whole gameturn upon the center of mass
void Unit::ApplyLocalForce(const Vector &Vforce)
{
	if (FINITE (Vforce.i)&&FINITE(Vforce.j)&&FINITE(Vforce.k)) {
		NetLocalForce += Vforce;
	}
	else {
		UNIT_LOG(logvs::NOTICE, "fatal local force");
	}
}


void Unit::Accelerate(const Vector &Vforce)
{
	if (FINITE (Vforce.i)&&FINITE(Vforce.j)&&FINITE(Vforce.k)) {
		NetForce += Vforce * GetMass();
	}
	else {
		UNIT_LOG(logvs::NOTICE, "fatal force");
	}
}


void Unit::ApplyTorque (const Vector &Vforce, const QVector &Location)
{
	//Not completely correct
	NetForce += Vforce;
	NetTorque += Vforce.Cross ((Location-curr_physical_state.position).Cast());
}


void Unit::ApplyLocalTorque (const Vector &Vforce, const Vector &Location)
{
	NetForce += Vforce;
	NetTorque += Vforce.Cross (Location);
}


								 //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
void Unit::ApplyBalancedLocalTorque (const Vector &Vforce, const Vector &Location)
{
	NetTorque += Vforce.Cross (Location);

}


void Unit::ApplyLocalTorque(const Vector &torque)
{
	/*  Vector p,q,r;
	Vector tmp(ClampTorque(torque));
	GetOrientation (p,q,r);
	VSFileSystem::vs_fprintf (stderr,"P: %f,%f,%f Q: %f,%f,%f",p.i,p.j,p.k,q.i,q.j,q.k);
	NetTorque+=tmp.i*p+tmp.j*q+tmp.k*r;
	*/
	NetLocalTorque+= ClampTorque(torque);
}


Vector Unit::MaxTorque(const Vector &torque)
{
	// torque is a normal
	return torque * (Vector(vs_copysignf(limits.pitch, torque.i),
		vs_copysignf(limits.yaw, torque.j),
		vs_copysignf(limits.roll, torque.k)) * torque);
}


float GetFuelUsage (bool afterburner)
{
	//  static float total_accel=XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"))*XMLSupport::parse_float (vs_config->getVariable("physics","game_accel","1"));
	static float normalfuelusage = XMLSupport::parse_float (vs_config->getVariable ("physics","FuelUsage","1"));
	static float abfuelusage = XMLSupport::parse_float (vs_config->getVariable ("physics","AfterburnerFuelUsage","4"));
	if (afterburner)
		return abfuelusage;
	return normalfuelusage;
}


Vector Unit::ClampTorque (const Vector &amt1)
{
	Vector Res=amt1;
	static bool WCfuelhack = XMLSupport::parse_bool(vs_config->getVariable("physics","fuel_equals_warp","false"));
	if (WCfuelhack)
		fuel = warpenergy;

	static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".4"));
	float fuelclamp=(fuel<=0)?staticfuelclamp:1;
	if (fabs(amt1.i)>fuelclamp*limits.pitch)
		Res.i=vs_copysignf(fuelclamp*limits.pitch,amt1.i);
	if (fabs(amt1.j)>fuelclamp*limits.yaw)
		Res.j=vs_copysignf(fuelclamp*limits.yaw,amt1.j);
	if (fabs(amt1.k)>fuelclamp*limits.roll)
		Res.k=vs_copysignf(fuelclamp*limits.roll,amt1.k);

	//static float fuelenergytomassconversionconstant = XMLSupport::parse_float(vs_config->getVariable ("physics","FuelEnergyDensity","343596000000000.0")); // note that we have KiloJoules, so it's to the 14th
	//static float Deuteriumconstant = XMLSupport::parse_float(vs_config->getVariable ("physics","DeuteriumRelativeEfficiency_Deuterium","1/0.6"));
	//static float Antimatterconstant = XMLSupport::parse_float(vs_config->getVariable ("physics","DeuteriumRelativeEfficiency_Antimatter","250/0.6"));
	static float Lithium6constant = XMLSupport::parse_float(vs_config->getVariable ("physics","LithiumRelativeEfficiency_Lithium","1"));
								 // Fuel Mass in metric tons expended per generation of 100MJ assuming 5,000,000m/s exit velocity
	static float FMEC_factor=XMLSupport::parse_float (vs_config->getVariable("physics","FMEC_factor","0.000000008"));
								 // 1/5,000,000 m/s
	static float FMEC_exit_vel_inverse=XMLSupport::parse_float (vs_config->getVariable("physics","FMEC_exit_vel","0.0000002"));
								 //HACK this forces the reaction to be Li-6+D fusion with efficiency governed by the getFuelUsage function
	fuel-=GetFuelUsage(false)*SIMULATION_ATOM*Res.Magnitude()*FMEC_exit_vel_inverse/Lithium6constant;
#ifndef __APPLE__
	if (ISNAN(fuel)) {
		UNIT_LOG(logvs::WARN, "FUEL is NAN");
		fuel=0;
	}
#endif
	if (fuel < 0) fuel = 0;

	if (warpenergy < 0) warpenergy = 0;
	if (WCfuelhack) warpenergy = fuel;

	return Res;
}


float Unit::Computer::max_speed() const
{
	static float combat_mode_mult = XMLSupport::parse_float (vs_config->getVariable ("physics","combat_speed_boost","100"));
	return (!combat_mode)?combat_mode_mult*max_combat_speed:max_combat_speed;
}


float Unit::Computer::max_ab_speed() const
{
	static float combat_mode_mult = XMLSupport::parse_float (vs_config->getVariable ("physics","combat_speed_boost","100"));
								 //same capped big speed as combat...else different
	return (!combat_mode)?combat_mode_mult*max_combat_speed:max_combat_ab_speed;
}


void Unit::SwitchCombatFlightMode()
{
	if (computer.combat_mode)
		computer.combat_mode=false;
	else
		computer.combat_mode=true;
}


bool Unit::CombatMode()
{
	return computer.combat_mode;
}


Vector Unit::ClampVelocity (const Vector & velocity, const bool afterburn)
{
	static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".4"));
	static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn",".1"));
	float fuelclamp=(fuel<=0)?staticfuelclamp:1;
	float abfuelclamp= (fuel<=0||(energy<afterburnenergy*SIMULATION_ATOM))?staticabfuelclamp:1;
	float limit = afterburn?(abfuelclamp*(computer.max_ab_speed()-computer.max_speed())+(fuelclamp*computer.max_speed())):fuelclamp*computer.max_speed();
	float tmp = velocity.Magnitude();
	if (tmp>fabs(limit)) {
		return velocity * (limit/tmp);
	}
	return velocity;
}


void Unit::ClearMounts()
{
	for (unsigned int j=0;j<mounts.size();++j) {
		DestroyMount(&mounts[j]);
		AUDDeleteSound(mounts[j].sound);
		if (mounts[j].ref.gun&&mounts[j].type->type==weapon_info::BEAM) {
								 //hope we're not killin' em twice...they don't go in gunqueue
			delete mounts[j].ref.gun;
			mounts[j].ref.gun=NULL;
		}
	}
	mounts.clear();
	Unit * su;
	for (un_iter i = getSubUnits(); (su= *i)!=NULL;++i) {
		su->ClearMounts();
	}
}


Vector Unit::ClampAngVel (const Vector & velocity)
{
	Vector res (velocity);
	if (res.i>=0) {
		if (res.i>computer.max_pitch_down) {
			res.i = computer.max_pitch_down;
		}
	}
	else {
		if (-res.i>computer.max_pitch_up) {
			res.i = -computer.max_pitch_up;
		}
	}
	if (res.j>=0) {
		if (res.j>computer.max_yaw_left) {
			res.j = computer.max_yaw_left;
		}
	}
	else {
		if (-res.j>computer.max_yaw_right) {
			res.j = -computer.max_yaw_right;
		}
	}
	if (res.k>=0) {
		if (res.k>computer.max_roll_left) {
			res.k = computer.max_roll_left;
		}
	}
	else {
		if (-res.k>computer.max_roll_right) {
			res.k = -computer.max_roll_right;
		}
	}
	return res;
}


Vector Unit::MaxThrust(const Vector &amt1)
{
	// amt1 is a normal
	return amt1 * (Vector(vs_copysignf(limits.lateral, amt1.i),
		vs_copysignf(limits.vertical, amt1.j),
		amt1.k>0?limits.forward:-limits.retro) * amt1);
}


/* misnomer..this doesn't get the max value of each axis
Vector Unit::ClampThrust(const Vector &amt1){
  // Yes, this can be a lot faster with LUT
  Vector norm = amt1;
  norm.Normalize();
  Vector max = MaxThrust(norm);

  if(max.Magnitude() > amt1.Magnitude())
	return amt1;
  else
	return max;
}
*/
//CMD_FLYBYWIRE depends on new version of Clampthrust... don't change without resolving it

Vector Unit::ClampThrust (const Vector &amt1, bool afterburn)
{
	static bool WCfuelhack = XMLSupport::parse_bool(vs_config->getVariable("physics","fuel_equals_warp","false"));
	static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".4"));
	static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn",".1"));
	static bool finegrainedFuelEfficiency = XMLSupport::parse_bool(vs_config->getVariable("physics","VariableFuelConsumption","false"));

	if (WCfuelhack) {
		//	  fuel = fuel <? warpenergy;
		//	  warpenergy = fuel <? warpenergy;
		if(fuel > warpenergy)
			fuel = warpenergy;
		if(fuel < warpenergy)
			warpenergy = fuel;
	}

	float instantenergy = afterburnenergy*SIMULATION_ATOM;
	if ((afterburntype == 0) && energy<instantenergy) {
		afterburn=false;
	}
	if ((afterburntype == 1) && fuel<0) {
		fuel = 0;
		afterburn=false;
	}
	if ((afterburntype == 2) && warpenergy<0) {
		warpenergy = 0;
		afterburn=false;
	}

	if(3==afterburntype){ // no afterburner -- we should really make these types an enum :-/
		afterburn=false;
	}

	Vector Res=amt1;

	float fuelclamp=(fuel<=0)?staticfuelclamp:1;
	float abfuelclamp= (fuel<=0)?staticabfuelclamp:1;
	if (fabs(amt1.i)>fabs(fuelclamp*limits.lateral))
		Res.i=vs_copysignf(fuelclamp*limits.lateral,amt1.i);
	if (fabs(amt1.j)>fabs(fuelclamp*limits.vertical))
		Res.j=vs_copysignf(fuelclamp*limits.vertical,amt1.j);
	float ablimit =
		afterburn
		?((limits.afterburn-limits.forward)*abfuelclamp+limits.forward*fuelclamp)
		:limits.forward;

	if (amt1.k>ablimit)
		Res.k=ablimit;
	if (amt1.k<-limits.retro)
		Res.k =-limits.retro;

	//static float Deuteriumconstant = XMLSupport::parse_float(vs_config->getVariable ("physics","DeuteriumRelativeEfficiency_Deuterium","1/0.6"));
	//static float Antimatterconstant = XMLSupport::parse_float(vs_config->getVariable ("physics","DeuteriumRelativeEfficiency_Antimatter","250/0.6"));
	static float Lithium6constant = XMLSupport::parse_float(vs_config->getVariable ("physics","DeuteriumRelativeEfficiency_Lithium","1"));
								 // Fuel Mass in metric tons expended per generation of 100MJ assuming 5,000,000m/s exit velocity
	static float FMEC_factor=XMLSupport::parse_float (vs_config->getVariable("physics","FMEC_factor","0.000000008"));
								 // 1/5,000,000 m/s
	static float FMEC_exit_vel_inverse=XMLSupport::parse_float (vs_config->getVariable("physics","FMEC_exit_vel","0.0000002"));

	if (afterburntype == 2){		 // Energy-consuming afterburner
								 //HACK this forces the reaction to be Li-6+Li-6 fusion with efficiency governed by the getFuelUsage function
		warpenergy-=afterburnenergy*GetFuelUsage(afterburn)*SIMULATION_ATOM*Res.Magnitude()*FMEC_exit_vel_inverse/Lithium6constant;
	}
	if (3 == afterburntype || afterburntype == 1) {	 // fuel-burning overdrive - uses afterburner efficiency. In NO_AFTERBURNER case, "afterburn" will always be false, so can reuse code.
								 //HACK this forces the reaction to be Li-6+Li-6 fusion with efficiency governed by the getFuelUsage function
		fuel-=((afterburn&&finegrainedFuelEfficiency)?afterburnenergy:GetFuelUsage(afterburn))*SIMULATION_ATOM*Res.Magnitude()*FMEC_exit_vel_inverse/Lithium6constant;
#ifndef __APPLE__
		if (ISNAN(fuel)) {
			UNIT_LOG(logvs::WARN, "Fuel is NAN A");
			fuel=0;
		}
#endif
	}
	if (afterburntype == 0) {	 // fuel-burning afterburner - uses default efficiency - appears to check for available energy? FIXME
								 //HACK this forces the reaction to be Li-6+Li-6 fusion with efficiency governed by the getFuelUsage function
		fuel-=GetFuelUsage(false)*SIMULATION_ATOM*Res.Magnitude()*FMEC_exit_vel_inverse/Lithium6constant;
#ifndef __APPLE__
		if (ISNAN(fuel)) {
			UNIT_LOG(logvs::WARN, "Fuel is NAN B");
			fuel=0;
		}
#endif
	}

	if ((afterburn) && (afterburntype == 0)) {
		energy -= instantenergy;
	}

	if (WCfuelhack) {
		if(fuel > warpenergy)
			fuel = warpenergy;
		if(fuel < warpenergy)
			warpenergy = fuel;
	}

	return Res;
}


void Unit::Thrust(const Vector &amt1,bool afterburn)
{
	Vector amt = ClampThrust(amt1,afterburn);
	ApplyLocalForce(amt);
}


void Unit::LateralThrust(float amt)
{
	if(amt>1.0) amt = 1.0;
	if(amt<-1.0) amt = -1.0;
	ApplyLocalForce(amt*limits.lateral * Vector(1,0,0));
}


void Unit::VerticalThrust(float amt)
{
	if(amt>1.0) amt = 1.0;
	if(amt<-1.0) amt = -1.0;
	ApplyLocalForce(amt*limits.vertical * Vector(0,1,0));
}


void Unit::LongitudinalThrust(float amt)
{
	if(amt>1.0) amt = 1.0;
	if(amt<-1.0) amt = -1.0;
	ApplyLocalForce(amt*limits.forward * Vector(0,0,1));
}


void Unit::YawTorque(float amt)
{
	if(amt>limits.yaw) amt = limits.yaw;
	else if(amt<-limits.yaw) amt = -limits.yaw;
	ApplyLocalTorque(amt * Vector(0,1,0));
}


void Unit::PitchTorque(float amt)
{
	if(amt>limits.pitch) amt = limits.pitch;
	else if(amt<-limits.pitch) amt = -limits.pitch;
	ApplyLocalTorque(amt * Vector(1,0,0));
}


void Unit::RollTorque(float amt)
{
	if(amt>limits.roll) amt = limits.roll;
	else if(amt<-limits.roll) amt = -limits.roll;
	ApplyLocalTorque(amt * Vector(0,0,1));
}


float WARPENERGYMULTIPLIER(Unit * un)
{
	static float warpenergymultiplier = XMLSupport::parse_float (vs_config->getVariable ("physics","warp_energy_multiplier","0.12"));
	static float playerwarpenergymultiplier = XMLSupport::parse_float (vs_config->getVariable ("physics","warp_energy_player_multiplier",".12"));
	bool player=_Universe->isPlayerStarship(un)!=NULL;
	Flightgroup * fg =un->getFlightgroup();
	if (fg&&!player) {
		player = _Universe->isPlayerStarship(fg->leader.GetUnit())!=NULL;
	}
	return player?playerwarpenergymultiplier:warpenergymultiplier;
}


								 //short fix
static bool applyto (float &shield, const float max, const float amt)
{
	shield+=amt;				 //short fix
	if (shield>max)
		shield=max;
	return (shield>=max)?1:0;
}


float totalShieldVal (const Shield & shield)
{
	float maxshield=0;
	switch (shield.number) {
		case 2:
			maxshield = shield.shield2fb.frontmax+shield.shield2fb.backmax;
			break;
		case 4:
			maxshield = shield.shield4fbrl.frontmax+shield.shield4fbrl.backmax+shield.shield4fbrl.leftmax+shield.shield4fbrl.rightmax;
			break;
		case 8:
			maxshield = shield.shield8.frontrighttopmax+shield.shield8.backrighttopmax+shield.shield8.frontlefttopmax+shield.shield8.backlefttopmax+shield.shield8.frontrightbottommax+shield.shield8.backrightbottommax+shield.shield8.frontleftbottommax+shield.shield8.backleftbottommax;
			break;
	}
	return maxshield;
}


float currentTotalShieldVal (const Shield & shield)
{
	float maxshield=0;
	switch (shield.number) {
		case 2:
			maxshield = shield.shield2fb.front+shield.shield2fb.back;
			break;
		case 4:
			maxshield = shield.shield4fbrl.front+shield.shield4fbrl.back+shield.shield4fbrl.left+shield.shield4fbrl.right;
			break;
		case 8:
			maxshield = shield.shield8.frontrighttop+shield.shield8.backrighttop+shield.shield8.frontlefttop+shield.shield8.backlefttop+shield.shield8.frontrightbottom+shield.shield8.backrightbottom+shield.shield8.frontleftbottom+shield.shield8.backleftbottom;
			break;
	}
	return maxshield;
}


float totalShieldEnergyCapacitance (const Shield & shield)
{
	static float shieldenergycap = XMLSupport::parse_float(vs_config->getVariable ("physics","shield_energy_capacitance",".2"));
	static bool use_max_shield_value = XMLSupport::parse_bool(vs_config->getVariable("physics","use_max_shield_energy_usage","false"));
	return (shieldenergycap * use_max_shield_value) ? totalShieldVal(shield) : currentTotalShieldVal(shield);
}


float Unit::MaxShieldVal() const
{
	float maxshield=0;
	switch (shield.number) {
		case 2:
			maxshield = .5*(shield.shield2fb.frontmax+shield.shield2fb.backmax);
			break;
		case 4:
			maxshield = .25*(shield.shield4fbrl.frontmax+shield.shield4fbrl.backmax+shield.shield4fbrl.leftmax+shield.shield4fbrl.rightmax);
			break;
		case 8:
			maxshield = .125*(shield.shield8.frontrighttopmax+shield.shield8.backrighttopmax+shield.shield8.frontlefttopmax+shield.shield8.backlefttopmax+shield.shield8.frontrightbottommax+shield.shield8.backrightbottommax+shield.shield8.frontleftbottommax+shield.shield8.backleftbottommax);
			break;
	}
	return maxshield;
}


void Unit::RechargeEnergy()
{
	static bool reactor_uses_fuel = XMLSupport::parse_bool(vs_config->getVariable("physics","reactor_uses_fuel","false"));
	if((!reactor_uses_fuel)||(fuel > 0)) {
		energy+=recharge*SIMULATION_ATOM;
	}
}


void Unit::RegenShields ()
{
	static bool shields_in_spec = XMLSupport::parse_bool(vs_config->getVariable("physics","shields_in_spec","false"));
	static float shieldenergycap = XMLSupport::parse_float(vs_config->getVariable ("physics","shield_energy_capacitance",".2"));
	static bool energy_before_shield=XMLSupport::parse_bool(vs_config->getVariable ("physics","engine_energy_priority","true"));
	static bool apply_difficulty_shields = XMLSupport::parse_bool (vs_config->getVariable("physics","difficulty_based_shield_recharge","true"));
	static float shield_maintenance_cost=XMLSupport::parse_float(vs_config->getVariable("physics","shield_maintenance_charge",".25"));
	static bool shields_require_power=XMLSupport::parse_bool(vs_config->getVariable ("physics","shields_require_passive_recharge_maintenance","true"));
	static float discharge_per_second=XMLSupport::parse_float (vs_config->getVariable("physics","speeding_discharge",".25"));
								 //approx
	const float dischargerate = (1-(1-discharge_per_second)*SIMULATION_ATOM);
	static float min_shield_discharge=XMLSupport::parse_float (vs_config->getVariable("physics","min_shield_speeding_discharge",".1"));
	static float speed_leniency = XMLSupport::parse_float (vs_config->getVariable("physics","speed_shield_drain_leniency","1.18"));
	static float low_power_mode = XMLSupport::parse_float(vs_config->getVariable("physics","low_power_mode_energy","10"));
	static float max_shield_lowers_recharge=XMLSupport::parse_float(vs_config->getVariable("physics","max_shield_recharge_drain","0"));
	static bool max_shield_lowers_capacitance=XMLSupport::parse_bool(vs_config->getVariable("physics","max_shield_lowers_capacitance","false"));
	static bool use_max_shield_value = XMLSupport::parse_bool(vs_config->getVariable("physics","use_max_shield_energy_usage","false"));
	static bool reactor_uses_fuel = XMLSupport::parse_bool(vs_config->getVariable("physics","reactor_uses_fuel","false"));
	static float reactor_idle_efficiency = XMLSupport::parse_float(vs_config->getVariable("physics","reactor_idle_efficiency","0.98"));
	static float VSD=XMLSupport::parse_float (vs_config->getVariable("physics","VSD_MJ_yield","5.4"));
								 // Fuel Mass in metric tons expended per generation of 100MJ
	static float FMEC_factor=XMLSupport::parse_float (vs_config->getVariable("physics","FMEC_factor","0.000000008"));
								 // Fudge factor -> increase fuel expenditure
	static float FMEC_efficiency=XMLSupport::parse_float (vs_config->getVariable("physics","FMEC_efficiency","2"));
	int rechargesh=1;			 // used ... oddly
	float maxshield=totalShieldEnergyCapacitance(shield);
	bool velocity_discharge=false;
	float rec=0;
	float precharge=energy;

	// Reactor energy
	if (!energy_before_shield) {
		RechargeEnergy();
	}

	// Shield energy drain
	if (shield.number) {
		// GAHHH reactor in units of 100MJ, shields in units of VSD=5.4MJ to make 1MJ of shield use 1/shieldenergycap MJ
		if(shields_in_spec||!graphicOptions.InWarp) {
			energy-=shield.recharge*VSD/(100*(shield.efficiency?shield.efficiency:1))/shieldenergycap*shield.number*shield_maintenance_cost*SIMULATION_ATOM*((apply_difficulty_shields)?g_game.difficulty:1);
			if(energy<0) {
				velocity_discharge=true;
				energy=0;
			}
		}
		rec = (velocity_discharge)?0:((shield.recharge*VSD/100*SIMULATION_ATOM*shield.number/shieldenergycap)>energy)?(energy*shieldenergycap*100/VSD/shield.number):shield.recharge*SIMULATION_ATOM;
		if (apply_difficulty_shields) {
			if (!_Universe->isPlayerStarship(this)) {
				rec*=g_game.difficulty;
			}
			else {
								 //sqrtf(g_game.difficulty);
				rec*=g_game.difficulty;
			}
		}
		/*
		if ((computer.max_combat_ab_speed>4)&&(GetVelocity().MagnitudeSquared()>(computer.max_combat_ab_speed*speed_leniency*computer.max_combat_ab_speed*speed_leniency))) {
			rec=0;
			velocity_discharge=true;
		}
		*/
		if (graphicOptions.InWarp&&!shields_in_spec) {
			rec=0;
			velocity_discharge=true;
		}
		if (GetNebula()!=NULL) {
			static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
			rec *=nebshields;
		}
	}

	//ECM energy drain
	if ((image->ecm>0)) {
		static float ecmadj = XMLSupport::parse_float(vs_config->getVariable ("physics","ecm_energy_cost",".05"));
		float sim_atom_ecm = ecmadj * image->ecm*SIMULATION_ATOM;
		if (energy>sim_atom_ecm) {
			energy-=sim_atom_ecm;
		}
		else {
			energy=0;
		}
	}

	//Shield regeneration
	switch (shield.number) {
		case 2:
			shield.shield2fb.front+=rec;
			shield.shield2fb.back+=rec;
			if (shield.shield2fb.front>shield.shield2fb.frontmax) {
				shield.shield2fb.front=shield.shield2fb.frontmax;
			}
			else {
				rechargesh=0;
			}
			if (shield.shield2fb.back>shield.shield2fb.backmax) {
				shield.shield2fb.back=shield.shield2fb.backmax;

			}
			else {
				rechargesh=0;
			}
			if (velocity_discharge) {
				if (shield.shield2fb.back>min_shield_discharge*shield.shield2fb.backmax)
					shield.shield2fb.back*=dischargerate;
				if (shield.shield2fb.front>min_shield_discharge*shield.shield2fb.frontmax)
					shield.shield2fb.front*=dischargerate;
			}
			rec=rec*2/shieldenergycap*VSD/100;
			break;
		case 4:
			rechargesh = applyto (shield.shield4fbrl.front,shield.shield4fbrl.frontmax,rec)*(applyto (shield.shield4fbrl.back,shield.shield4fbrl.backmax,rec))*applyto (shield.shield4fbrl.right,shield.shield4fbrl.rightmax,rec)*applyto (shield.shield4fbrl.left,shield.shield4fbrl.leftmax,rec);
			if (velocity_discharge) {
				if (shield.shield4fbrl.front>min_shield_discharge*shield.shield4fbrl.frontmax)
					shield.shield4fbrl.front*=dischargerate;
				if (shield.shield4fbrl.left>min_shield_discharge*shield.shield4fbrl.leftmax)
					shield.shield4fbrl.left*=dischargerate;
				if (shield.shield4fbrl.back>min_shield_discharge*shield.shield4fbrl.backmax)
					shield.shield4fbrl.back*=dischargerate;
				if (shield.shield4fbrl.right>min_shield_discharge*shield.shield4fbrl.rightmax)
					shield.shield4fbrl.right*=dischargerate;
			}
			rec=rec*4/shieldenergycap*VSD/100;
			break;
		case 8:
			rechargesh = applyto (shield.shield8.frontrighttop,shield.shield8.frontrighttopmax,rec)*(applyto (shield.shield8.backrighttop,shield.shield8.backrighttopmax,rec))*applyto (shield.shield8.frontlefttop,shield.shield8.frontlefttopmax,rec)*applyto (shield.shield8.backlefttop,shield.shield8.backlefttopmax,rec)*applyto (shield.shield8.frontrightbottom,shield.shield8.frontrightbottommax,rec)*(applyto (shield.shield8.backrightbottom,shield.shield8.backrightbottommax,rec))*applyto (shield.shield8.frontleftbottom,shield.shield8.frontleftbottommax,rec)*applyto (shield.shield8.backleftbottom,shield.shield8.backleftbottommax,rec);
			if (velocity_discharge) {
				if (shield.shield8.frontrighttop>min_shield_discharge*shield.shield8.frontrighttopmax)
					shield.shield8.frontrighttop*=dischargerate;
				if (shield.shield8.frontlefttop>min_shield_discharge*shield.shield8.frontlefttopmax)
					shield.shield8.frontlefttop*=dischargerate;
				if (shield.shield8.backrighttop>min_shield_discharge*shield.shield8.backrighttopmax)
					shield.shield8.backrighttop*=dischargerate;
				if (shield.shield8.backlefttop>min_shield_discharge*shield.shield8.backlefttopmax)
					shield.shield8.backlefttop*=dischargerate;
				if (shield.shield8.frontrightbottom>min_shield_discharge*shield.shield8.frontrightbottommax)
					shield.shield8.frontrightbottom*=dischargerate;
				if (shield.shield8.frontleftbottom>min_shield_discharge*shield.shield8.frontleftbottommax)
					shield.shield8.frontleftbottom*=dischargerate;
				if (shield.shield8.backrightbottom>min_shield_discharge*shield.shield8.backrightbottommax)
					shield.shield8.backrightbottom*=dischargerate;
				if (shield.shield8.backleftbottom>min_shield_discharge*shield.shield8.backleftbottommax)
					shield.shield8.backleftbottom*=dischargerate;
			}
			rec=rec*8/shieldenergycap*VSD/100;
			break;
	}
	if (shield.number) {
		if (rechargesh==0) {
			energy-=rec;
		}
		if (shields_require_power) {
			maxshield=0;
		}
		if (max_shield_lowers_recharge) {
			energy-=max_shield_lowers_recharge*SIMULATION_ATOM*maxshield*VSD/(100*(shield.efficiency?shield.efficiency:1));
		}
		if (!max_shield_lowers_capacitance) {
			maxshield=0;
		}
	}

	// Reactor energy
	if (energy_before_shield) {
		RechargeEnergy();
	}

	// Final energy computations
	float menergy = maxenergy;
	if (shield.number&&(menergy-maxshield<low_power_mode)) {
		menergy=maxshield+low_power_mode;
		if (_Universe->isPlayerStarship(this))
			if (rand()<.00005*RAND_MAX)
				UniverseUtil::IOmessage(0,"	game","all","**Warning** Power Supply Overdrawn: downgrade shield or purchase reactor capacitance!");
	}
	if(graphicOptions.InWarp) {	 //FIXME FIXME FIXME
		static float bleedfactor = XMLSupport::parse_float(vs_config->getVariable("physics","warpbleed","20"));
		float bleed=jump.insysenergy/bleedfactor*SIMULATION_ATOM;
		if(warpenergy>bleed) {
			warpenergy-=bleed;
		}
		else {
			graphicOptions.InWarp=0;
			graphicOptions.WarpRamping=1;
		}
	}

	float excessenergy=0;
	//NOTE: !shield.number => maxshield==0
	if (menergy>maxshield) {
								 //allow warp caps to absorb xtra power
		if (energy>menergy-maxshield) {
			excessenergy = energy - (menergy-maxshield);
			energy=menergy-maxshield;
			if (excessenergy >0) {
				warpenergy=warpenergy+WARPENERGYMULTIPLIER(this)*excessenergy;
				float mwe = maxwarpenergy;
				if (mwe<jump.energy&&mwe==0)
					mwe = jump.energy;
				if (warpenergy>mwe) {
					excessenergy=(warpenergy-mwe)/WARPENERGYMULTIPLIER(this);
					warpenergy=mwe;
				}
			}
		}
	}
	else {
		energy=0;
	}

	excessenergy=(excessenergy>precharge)?excessenergy-precharge:0;
	if(reactor_uses_fuel) {
		static float min_reactor_efficiency=XMLSupport::parse_float(vs_config->getVariable("physics","min_reactor_efficiency",".00001"));
		fuel-=FMEC_factor*((recharge*SIMULATION_ATOM-(reactor_idle_efficiency*excessenergy))/(min_reactor_efficiency+(image->LifeSupportFunctionality*(1-min_reactor_efficiency))));
		if (fuel<0) fuel=0;
		if (!FINITE(fuel)) {
			UNIT_LOG(logvs::WARN, "Fuel is nan C");
			fuel=0;
		}
	}

	energy=energy<0?0:energy;
}


Vector Unit::ResolveForces (const Transformation &trans, const Matrix &transmat)
{
	// First, save theoretical instantaneous acceleration (not time-quantized) for GetAcceleration()
	SavedAccel = GetNetAcceleration();

	Vector p, q, r;
	GetOrientation(p,q,r);
	Vector temp1 (NetLocalTorque.i*p+NetLocalTorque.j*q+NetLocalTorque.k *r);
	if (NetTorque.i||NetTorque.j||NetTorque.k) {
		temp1 += InvTransformNormal(transmat,NetTorque);
	}
	if (GetMoment()){
		temp1=temp1/GetMoment();
	} else{
		UNIT_LOG(logvs::NOTICE, "zero moment of inertia %s",name.get().c_str());
	}
	Vector temp (temp1*SIMULATION_ATOM);
	/*  //FIXME  does this shit happen!
		if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) */
	{
		if (!FINITE(temp.i)||FINITE (temp.j)||FINITE(temp.k)) {

		}
		AngularVelocity += temp;
		static float maxplayerrotationrate=XMLSupport::parse_float(vs_config->getVariable("physics","maxplayerrot","24"));
		static float maxnonplayerrotationrate=XMLSupport::parse_float(vs_config->getVariable("physics","maxNPCrot","360"));
		float caprate;
		if(_Universe->isPlayerStarship (this)){ // clamp to avoid vomit-comet effects
			caprate=maxplayerrotationrate;
		} else {
			caprate=maxnonplayerrotationrate;
		}
		if(AngularVelocity.MagnitudeSquared()>caprate*caprate){
			AngularVelocity=AngularVelocity.Normalize()*caprate;
		}

	}
								 //acceleration
	Vector temp2 = (NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r );
	if (!(FINITE(NetForce.i)&&FINITE(NetForce.j)&&FINITE(NetForce.k))) {
		cout << "NetForce skrewed";
	}

	if (NetForce.i||NetForce.j||NetForce.k) {
		temp2+=InvTransformNormal(transmat,NetForce);
	}

	temp2=temp2/GetMass();
	temp = temp2*SIMULATION_ATOM;
	if (!(FINITE(temp2.i)&&FINITE(temp2.j)&&FINITE(temp2.k))) {
		cout << "NetForce transform skrewed";
	}
	float oldmagsquared = Velocity.MagnitudeSquared();
	/*if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) */{    //FIXME
	Velocity += temp;
}


float newmagsquared = Velocity.MagnitudeSquared();
static float warpstretchcutoff= XMLSupport::parse_float (vs_config->getVariable( "graphics","warp_stretch_cutoff","500000"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
static float warpstretchoutcutoff= XMLSupport::parse_float (vs_config->getVariable( "graphics","warp_stretch_decel_cutoff","500000"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
static float cutsqr = warpstretchcutoff*warpstretchcutoff;
static float outcutsqr = warpstretchoutcutoff*warpstretchoutcutoff;
bool oldbig = oldmagsquared>cutsqr;
bool newbig = newmagsquared>cutsqr;
bool oldoutbig = oldmagsquared>outcutsqr;
bool newoutbig = newmagsquared>outcutsqr;
if ((newbig&&!oldbig)||(oldoutbig&&!newoutbig)) {

	static string insys_jump_ani = vs_config->getVariable ("graphics","insys_jump_animation","warp.ani");
	static bool docache=true;
	if (docache) {
		UniverseUtil::cacheAnimation (insys_jump_ani);
		docache=false;
	}

	Vector v(GetVelocity());
	v.Normalize();
	Vector p,q,r;GetOrientation(p,q,r);
	static float sec = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_ani_second_ahead","4"))/(XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_accel","1")));
	static float endsec = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_ani_second_ahead_end",".03"))/(XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_accel","1")));
	float tmpsec = oldbig?endsec:sec;
	UniverseUtil::playAnimationGrow (insys_jump_ani,RealPosition(this).Cast()+Velocity*tmpsec+v*rSize(),rSize()*8,1);
	//      UniverseUtil::playAnimationGrow (insys_jump_ani,RealPosition(this).Cast()+GetVelocity()*sec+2*v*rSize()+r*4*rSize(),rSize()*16,.97);

}


static float air_res_coef =XMLSupport::parse_float (active_missions[0]->getVariable ("air_resistance","0"));
static float lateral_air_res_coef =XMLSupport::parse_float (active_missions[0]->getVariable ("lateral_air_resistance","0"));

if (air_res_coef||lateral_air_res_coef) {
	float velmag = Velocity.Magnitude();
	Vector AirResistance = Velocity*(air_res_coef*velmag/GetMass())*(corner_max.i-corner_min.i)*(corner_max.j-corner_min.j);
	if (AirResistance.Magnitude()>velmag) {
		Velocity.Set(0,0,0);
	}
	else {
		Velocity = Velocity-AirResistance;
		if (lateral_air_res_coef) {
			Vector p,q,r;
			GetOrientation (p,q,r);
			Vector lateralVel= p*Velocity.Dot (p)+q*Velocity.Dot (q);
			AirResistance = lateralVel*(lateral_air_res_coef*velmag/GetMass())*(corner_max.i-corner_min.i)*(corner_max.j-corner_min.j);
			if (AirResistance.Magnitude ()> lateralVel.Magnitude()) {
				Velocity = r*Velocity.Dot(r);
			}
			else {
				Velocity = Velocity - AirResistance;
			}
		}

	}
}


NetForce = NetLocalForce = NetTorque = NetLocalTorque = Vector(0,0,0);

/*
  if (fabs (Velocity.i)+fabs(Velocity.j)+fabs(Velocity.k)> co10) {
  float magvel = Velocity.Magnitude(); float y = (1-magvel*magvel*oocc);
  temp = temp * powf (y,1.5);
  }*/

return temp2;
}


void Unit::SetOrientation (QVector q, QVector r)
{
	q.Normalize();
	r.Normalize();
	QVector p;
	CrossProduct (q,r,p);
	CrossProduct (r,p,q);
	curr_physical_state.orientation = Quaternion::from_vectors (p.Cast(),q.Cast(),r.Cast());
}


void Unit::SetOrientation (QVector p, QVector q, QVector r)
{
	q.Normalize();
	r.Normalize();
	p.Normalize();
	//  QVector p;
	//  CrossProduct (q,r,p);
	//  CrossProduct (r,p,q);
	curr_physical_state.orientation = Quaternion::from_vectors (p.Cast(),q.Cast(),r.Cast());
}


void Unit::SetOrientation(Quaternion Q)
{
	curr_physical_state.orientation=Q;
}


Vector Unit::UpCoordinateLevel (const Vector &v) const
{
	Matrix m;
	curr_physical_state.to_matrix(m);
#define MM(A,B) m.r[B*3+A]
	return Vector(v.i*MM(0,0)+v.j*MM(1,0)+v.k*MM(2,0),
		v.i*MM(0,1)+v.j*MM(1,1)+v.k*MM(2,1),
		v.i*MM(0,2)+v.j*MM(1,2)+v.k*MM(2,2));
#undef MM
}


Vector Unit::DownCoordinateLevel (const Vector &v) const
{
	Matrix m;
	curr_physical_state.to_matrix(m);
	return TransformNormal(m,v);
}


Vector Unit::ToLocalCoordinates(const Vector &v) const
{
	//Matrix m;
	//062201: not a cumulative transformation...in prev unit space  curr_physical_state.to_matrix(m);

#define MM(A,B) cumulative_transformation_matrix.r[B*3+A]
	return Vector(v.i*MM(0,0)+v.j*MM(1,0)+v.k*MM(2,0),
		v.i*MM(0,1)+v.j*MM(1,1)+v.k*MM(2,1),
		v.i*MM(0,2)+v.j*MM(1,2)+v.k*MM(2,2));
#undef MM
}


Vector Unit::ToWorldCoordinates(const Vector &v) const
{
	return TransformNormal(cumulative_transformation_matrix,v);

}


/***********************************************************************************/
/**** UNIT_DAMAGE STUFF                                                            */
/***********************************************************************************/

void Unit::LightShields(const Vector & pnt, const Vector & normal, float amt, const GFXColor &color)
{
	meshdata.back()->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector(0,0,0),mymin(1.0f,mymax(0.0f,amt)),color);
}


// NEW TESTING MODIFICATIONS
// We do it also on client side to display hits on shields/armor -> not to compute damage
// Damage are computed on server side and shield/armor data are sent with the DAMAGE SNAPSHOT
float Unit::ApplyLocalDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit,const GFXColor &color, float phasedamage)
{
	static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
	//  #ifdef REALLY_EASY
	Cockpit * cpt;
	if ( (cpt=_Universe->isPlayerStarship(this))!=NULL) {
		if (color.a!=2) {
			//    ApplyDamage (amt);
			static bool apply_difficulty_enemy_damage=XMLSupport::parse_bool(vs_config->getVariable("physics","difficulty_based_enemy_damage","true"));
			if (apply_difficulty_enemy_damage) {
				phasedamage*= g_game.difficulty;
				amt*=g_game.difficulty;
			}

		}
	}
	//  #endif
	float absamt= amt>=0?amt:-amt;
	float ppercentage=0;
	// We also do the following lock on client side in order not to display shield hits
	static bool nodockdamage = XMLSupport::parse_float (vs_config->getVariable("physics","no_damage_to_docked_ships","true"));
	if (nodockdamage) {
		if (DockedOrDocking()&(DOCKED_INSIDE|DOCKED)) {
			return -1;
		}
	}
	if (affectedUnit!=this) {
		affectedUnit->ApplyLocalDamage (pnt,normal,amt,affectedUnit,color,phasedamage);
		return -1;
	}

	if (aistate)
		aistate->ChooseTarget();

	float leakamt = phasedamage+amt*.01*shield.leak;
	amt *= 1-.01*shield.leak;
	// Percentage returned by DealDamageToShield
	float spercentage = 0;
	// If not a nebula or if shields recharge in nebula => WE COMPUTE SHIELDS DAMAGE AND APPLY
	if (GetNebula()==NULL||(nebshields>0)) {
		// Compute spercentage even in networking because doesn't apply damage on client side
		//if( Network==NULL || SERVER)
		float origabsamt=absamt;
		spercentage = DealDamageToShield (pnt,absamt);

		//percentage = DealDamageToShield (pnt,absamt);
		amt = amt>=0?absamt:-absamt;
								 //shields are up
		if (meshdata.back()&&spercentage>0&&(origabsamt-absamt>shield.recharge||amt==0)) {
			/*      meshdata[nummesh]->LocalFX.push_back (GFXLight (true,
				GFXColor(pnt.i+normal.i,pnt.j+normal.j,pnt.k+normal.k),
				GFXColor (.3,.3,.3), GFXColor (0,0,0,1),
				GFXColor (.5,.5,.5),GFXColor (1,0,.01)));*/
			//calculate percentage
			if (cpt)
				cpt->Shake (amt,0);
			if (GetNebula()==NULL)
				LightShields(pnt,normal,spercentage,color);
		}
	}
	// If shields failing or... => WE COMPUTE DAMAGE TO HULL
	if (shield.leak>0||!meshdata.back()||spercentage==0||absamt>0||phasedamage) {
		// ONLY in server or in non-networking
		// Compute spercentage even in networking because doesn't apply damage on client side
		//if( Network==NULL || SERVER)
		float tmp=this->GetHull();
		ppercentage = DealDamageToHull (pnt, leakamt+amt);
		if (cpt)
			cpt->Shake(amt+leakamt,tmp!=this->GetHull()?2:1);

		if (ppercentage!=-1) {	 //returns -1 on death--could delete
			for (int i=0;i<nummesh();++i) {
				if (ppercentage)
					meshdata[i]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector (0,0,0),ppercentage,color);
			}
		}
	}
	// If server and there is damage to shields or if unit is not killed (ppercentage>0)
	if( SERVER && (ppercentage>0 || spercentage>0)) {
#if 1					 //def NET_SHIELD_SYSTEM_1
		// FIRST METHOD : send each time a unit is hit all the damage info to all clients in the current zone
		// If server side, we send the unit serial + serialized shields + shield recharge + shield leak + ...
		Vector netpnt = pnt;
		Vector netnormal = normal;
		GFXColor col( color.r, color.g, color.b, color.a);
		VSServer->sendDamages( this->serial, this->getStarSystem()->GetZone(), hull, shield, armor, ppercentage, spercentage, amt, netpnt, netnormal, col);
		// This way the client computes damages based on what we send to him => less reliable
		//VSServer->sendDamages( this->serial, pnt, normal, amt, col, phasedamage);
#endif
#if 1
		// SECOND METHOD : we just put a flag on the unit telling its shield/armor data has changed
		if( spercentage>0)
			this->damages |= SHIELD_DAMAGED;
		if( ppercentage>0)
			this->damages |= ARMOR_DAMAGED;
#endif
	}

	return ppercentage>0?2.0f:1.0f;
}


void    Unit::ApplyNetDamage( Vector & pnt, Vector & normal, float amt, float ppercentage, float spercentage, GFXColor & color)
{
	static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
	Cockpit * cpt;
	if ( (cpt=_Universe->isPlayerStarship(this))!=NULL) {

	}
	if( GetNebula()==NULL||nebshields>0) {
								 //shields are up
		if (meshdata.back()&&spercentage>0&&amt==0) {
			if (GetNebula()==NULL)
				meshdata.back()->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector(0,0,0),spercentage,color);
			if (cpt)
				cpt->Shake (amt,0);
		}
	}
	if (shield.leak>0||!meshdata.back()||spercentage==0||amt>0) {
		if (ppercentage!=-1) {	 //returns -1 on death--could delete
			for (int i=0;i<nummesh();++i) {
				if (ppercentage) {
					meshdata[i]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector (0,0,0),ppercentage,color);
					if (cpt)
						cpt->Shake (amt,2);
				}

			}
		}
	}
}


Unit * findUnitInStarsystem (void * unitDoNotDereference)
{
	Unit * un;
	for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();(un=*i)!=NULL;++i) {
		if (un==unitDoNotDereference)
			return un;
	}
	return NULL;
}


extern void ScoreKill (Cockpit * cp, Unit * killer, Unit * killedUnit);
// Changed order of things -> Vectors and ApplyLocalDamage are computed before Cockpit thing now
void AllUnitsCloseAndEngage(Unit*,int faction);
void Unit::ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit, const GFXColor & color, void * ownerDoNotDereference, float phasedamage)
{
	Cockpit * cp = _Universe->isPlayerStarshipVoid (ownerDoNotDereference);
	float hullpercent=GetHullPercent();
	// Only on client side
	bool mykilled = hull<0;
	Vector localpnt (InvTransform(cumulative_transformation_matrix,pnt));
	Vector localnorm (ToLocalCoordinates (normal));
	// Only call when on servre side or non-networking
	// If networking damages are applied as they are received
	static float hull_percent_for_comm=XMLSupport::parse_float(vs_config->getVariable("AI","HullPercentForComm",".75"));
	bool armor_damage=false;
	if( SERVER || Network==NULL) {
		armor_damage=(ApplyLocalDamage(localpnt, localnorm, amt,affectedUnit,color,phasedamage)==2);
	}
	if (!Network && cp) {
		static int MadnessForShieldDamage=XMLSupport::parse_bool(vs_config->getVariable("AI","ShieldDamageAnger","1"));
		static int MadnessForHullDamage=XMLSupport::parse_bool(vs_config->getVariable("AI","HullDamageAnger","10"));
		int howmany= armor_damage?MadnessForHullDamage:MadnessForShieldDamage;
		for (int i=0;i<howmany;++i) {
			//now we can dereference it because we checked it against the parent
			CommunicationMessage c(reinterpret_cast<Unit *>(ownerDoNotDereference),this,NULL,0);
			c.SetCurrentState(c.fsm->GetHitNode(),NULL,0);
			if (this->getAIState()) this->getAIState()->Communicate (c);
		}
								 //the dark danger is real!
		Threaten (reinterpret_cast<Unit*>(ownerDoNotDereference),10);
	}
	else if (!Network) {
								 // if only the damage contained which faction it belonged to
		pilot->DoHit(this,ownerDoNotDereference,FactionUtil::GetNeutralFaction());
	}

	if (hull<0) {
		ClearMounts();
		if (!mykilled) {
			if (cp) {
				ScoreKill (cp,reinterpret_cast<Unit *>(ownerDoNotDereference),this);
			}
			else {
				Unit * tmp;
				if ((tmp=findUnitInStarsystem(ownerDoNotDereference))!=NULL) {
					if ((NULL!=(cp=_Universe->isPlayerStarshipVoid(tmp->owner)))
					&&(cp->GetParent()!=NULL)) {
						ScoreKill(cp,cp->GetParent(),this);
					}
					else {
						ScoreKill(NULL,tmp,this);
					}
				}
			}
		}
	}
	else if (hullpercent>=hull_percent_for_comm&&((float)GetHullPercent())<hull_percent_for_comm&&(cp||_Universe->isPlayerStarship(this))) {
		Unit * computerai=NULL;
		Unit * player = NULL;
		if (cp==NULL) {
			computerai=findUnitInStarsystem(ownerDoNotDereference);
			player = this;
		}
		else {
			computerai=this;
								 //cp != NULL
			player = cp->GetParent();
		}
		if (computerai&&player&&computerai->getAIState()&&player->getAIState()&&computerai->isUnit()==UNITPTR&&player->isUnit()==UNITPTR) {

			unsigned char gender;
			vector <Animation *>* anim = computerai->pilot->getCommFaces(gender);
			if (cp) {
				static bool assistallyinneed=XMLSupport::parse_bool(vs_config->getVariable("AI","assist_friend_in_need","true"));
				if (assistallyinneed)
					AllUnitsCloseAndEngage(player,computerai->faction);
			}
			if (GetHullPercent()>0||!cp) {
				CommunicationMessage c(computerai,player,anim,gender);
				c.SetCurrentState(cp?c.fsm->GetDamagedNode():c.fsm->GetDealtDamageNode(),anim,gender);
				player->getAIState()->Communicate(c);
			}
		}
	}
}


// NUMGAUGES has been moved to images.h in UnitImages
void Unit::DamageRandSys(float dam, const Vector &vec, float randnum, float degrees)
{

	float deg = fabs(180*atan2 (vec.i,vec.k)/M_PI);
	//float randnum=rand01();
	//float degrees=deg;
	randnum=rand01();
	static float inv_min_dam=1.0f-XMLSupport::parse_float(vs_config->getVariable("physics","min_damage",".001"));
	static float inv_max_dam=1.0f-XMLSupport::parse_float(vs_config->getVariable("physics","min_damage",".999"));

	if (dam<inv_max_dam) dam=inv_max_dam;
	if (dam>inv_min_dam) dam=inv_min_dam;
	degrees=deg;
	if (degrees>180) {
		degrees=360-degrees;
	}
	if (degrees>=0&&degrees<20) {
		int which= rand()%(1+UnitImages::NUMGAUGES+MAXVDUS);
		image->cockpit_damage[which]*=dam;
		if (image->cockpit_damage[which]<.1) {
			image->cockpit_damage[which]=0;
		}
		//DAMAGE COCKPIT
		if (randnum>=.85) {
								 //Set the speed to a random speed
			computer.set_speed=(rand01()*computer.max_speed()*(5/3))-(computer.max_speed()*(2/3));
		}
		else if (randnum>=.775) {
			computer.itts=false; //Set the computer to not have an itts
		}
		else if (randnum>=.7) {
			if (computer.radar.iff>0) {
								 //set the radar to not have color
				computer.radar.iff-=1;
			}
		}
		else if (randnum>=.5) {
			// THIS IS NOT YET SUPPORTED IN NETWORKING
			computer.target=NULL;//set the target to NULL
		}
		else if (randnum>=.4) {
			limits.retro*=dam;
		}
		else if (randnum>=.3275) {
			static float maxdam=XMLSupport::parse_float(vs_config->getVariable("physics","max_radar_cone_damage",".9"));
			computer.radar.maxcone+=(1-dam);
			if (computer.radar.maxcone>maxdam)
				computer.radar.maxcone=maxdam;
		}
		else if (randnum>=.325) {
			static float maxdam=XMLSupport::parse_float(vs_config->getVariable("physics","max_radar_lockcone_damage",".95"));
			computer.radar.lockcone+=(1-dam);
			if (computer.radar.lockcone>maxdam)
				computer.radar.lockcone=maxdam;
		}
		else if (randnum>=.25) {
			static float maxdam=XMLSupport::parse_float(vs_config->getVariable("physics","max_radar_trackcone_damage",".98"));
			computer.radar.trackingcone+=(1-dam);
			if (computer.radar.trackingcone>maxdam)
				computer.radar.trackingcone=maxdam;
		}
		else if (randnum>=.175) {
			computer.radar.maxrange*=dam;
		}
		else {
			int which= rand()%(1+UnitImages::NUMGAUGES+MAXVDUS);
			image->cockpit_damage[which]*=dam;
			if (image->cockpit_damage[which]<.1) {
				image->cockpit_damage[which]=0;
			}
		}
		damages |= COMPUTER_DAMAGED;
		return;
	}
	static float thruster_hit_chance=XMLSupport::parse_float(vs_config->getVariable("physics","thruster_hit_chance",".25"));
	if (rand01()<thruster_hit_chance) {
		//DAMAGE ROLL/YAW/PITCH/THRUST
		float orandnum=rand01()*.82+.18;
		if (randnum>=.9) {
			computer.max_pitch_up*=orandnum;
		}
		else if (randnum>=.8) {
			computer.max_yaw_right*=orandnum;
		}
		else if (randnum>=.6) {
			computer.max_yaw_left*=orandnum;
		}
		else if (randnum>=.4) {
			computer.max_pitch_down*=orandnum;
		}
		else if (randnum>=.2) {
			computer.max_roll_right*=orandnum;
		}
		else if (randnum>=.18) {
			computer.max_roll_left*=orandnum;
		}
		else if (randnum>=.17) {
			limits.roll*=dam;
		}
		else if (randnum>=.10) {
			limits.yaw*=dam;
		}
		else if (randnum>=.03) {
			limits.pitch*=dam;
		}
		else {
			limits.lateral*=dam;
		}
		damages |= LIMITS_DAMAGED;
		return;
	}
	if (degrees>=20&&degrees<35) {
		//DAMAGE MOUNT
		if (randnum>=.65&&randnum<.9) {
			image->ecm*=float_to_int(dam);
		}
		else if (GetNumMounts()) {
			unsigned int whichmount=rand()%GetNumMounts();
			if (randnum>=.9) {
				DestroyMount(&mounts[whichmount]);
			}
			else if (mounts[whichmount].ammo>0&&randnum>=.75) {
				mounts[whichmount].ammo*=float_to_int(dam);
			}
			else if (randnum>=.7) {
				mounts[whichmount].time_to_lock+=(100-(100*dam));
			}
			else if (randnum>=.2) {
				mounts[whichmount].functionality*=dam;
			}
			else {
				mounts[whichmount].maxfunctionality*=dam;
			}
		}
		damages |= MOUNT_DAMAGED;
		return;
	}
	if (degrees>=35&&degrees<60) {
		//DAMAGE FUEL
		static float fuel_damage_prob = 1.f -
			XMLSupport::parse_float(vs_config->getVariable("physics","fuel_damage_prob",".25"));
		static float warpenergy_damage_prob = fuel_damage_prob -
			XMLSupport::parse_float(vs_config->getVariable("physics","warpenergy_damage_prob","0.05"));
		static float ab_damage_prob = warpenergy_damage_prob -
			XMLSupport::parse_float(vs_config->getVariable("physics","ab_damage_prob",".2"));
		static float cargovolume_damage_prob = ab_damage_prob -
			XMLSupport::parse_float(vs_config->getVariable("physics","cargovolume_damage_prob",".15"));
		static float upgradevolume_damage_prob = cargovolume_damage_prob -
			XMLSupport::parse_float(vs_config->getVariable("physics","upgradevolume_damage_prob",".1"));
		static float cargo_damage_prob = upgradevolume_damage_prob -
			XMLSupport::parse_float(vs_config->getVariable("physics","cargo_damage_prob","1"));
		if (randnum>=fuel_damage_prob) {
			fuel*=dam;
		}
		else if (randnum>=warpenergy_damage_prob) {
			warpenergy*=dam;
		}
		else if (randnum>=ab_damage_prob) {
			this->afterburnenergy+=((1-dam)*recharge);
		}
		else if (randnum>=cargovolume_damage_prob) {
			image->CargoVolume*=dam;
		}
		else if (randnum>=upgradevolume_damage_prob) {
			image->UpgradeVolume*=dam;
		}
		else if (randnum>=cargo_damage_prob) {
			//Do something NASTY to the cargo
			if (image->cargo.size()>0) {
				size_t i=0;
				unsigned int cargorand_o=rand();
				unsigned int cargorand;
				do {
					cargorand=(cargorand_o+i)%image->cargo.size();
				} while ((image->cargo[cargorand].quantity==0||image->cargo[cargorand].mission)&&++i<image->cargo.size());
				image->cargo[cargorand].quantity*=float_to_int(dam);
			}
		}
		damages |= CARGOFUEL_DAMAGED;
		return;
	}

	if (degrees>=90&&degrees<120) {
		//DAMAGE Shield
		//DAMAGE cloak
		if (randnum>=.95) {
			this->cloaking=-1;
			damages |= CLOAK_DAMAGED;
		}
		else if (randnum>=.78) {
			image->cloakenergy+=((1-dam)*recharge);
			damages |= CLOAK_DAMAGED;
		}
		else if (randnum>=.7) {
			cloakmin+=(rand()%(32000-cloakmin));
			damages |= CLOAK_DAMAGED;
		}
		switch (shield.number) {
			case 2:
				if (randnum>=.25&&randnum<.75) {
					shield.shield2fb.frontmax*=dam;
				}
				else {
					shield.shield2fb.backmax*=dam;
				}
				break;
			case 4:
				if (randnum>=.5&&randnum<.75) {
					shield.shield4fbrl.frontmax*=dam;
				}
				else if (randnum>=.75) {
					shield.shield4fbrl.backmax*=dam;
				}
				else if (randnum>=.25) {
					shield.shield4fbrl.leftmax*=dam;
				}
				else {
					shield.shield4fbrl.rightmax*=dam;
				}
				break;
			case 8:
				if (randnum<.125) {
					shield.shield8.frontrighttopmax*=dam;
				}
				else if (randnum<.25) {
					shield.shield8.backrighttopmax*=dam;
				}
				else if (randnum<.375) {
					shield.shield8.frontlefttopmax*=dam;
				}
				else if (randnum<.5) {
					shield.shield8.backrighttopmax*=dam;
				}
				else if (randnum<.625) {
					shield.shield8.frontrightbottommax*=dam;
				}
				else if (randnum<.75) {
					shield.shield8.backrightbottommax*=dam;
				}
				else if (randnum<.875) {
					shield.shield8.frontleftbottommax*=dam;
				}
				else {
					shield.shield8.backrightbottommax*=dam;
				}
				break;
		}
		damages |= SHIELD_DAMAGED;
		return;
	}
	if (degrees>=120&&degrees<150) {
		//DAMAGE Reactor
		//DAMAGE JUMP
		if (randnum>=.9) {
			static char max_shield_leak=(char)mymax(0.0,mymin(100.0,XMLSupport::parse_float(vs_config->getVariable("physics","max_shield_leak","90"))));
			static char min_shield_leak=(char)mymax(0.0,mymin(100.0,XMLSupport::parse_float(vs_config->getVariable("physics","max_shield_leak","0"))));
			char newleak=float_to_int(mymax(min_shield_leak,mymax(max_shield_leak,(char)((randnum-.9)*10.0*100.0))));
			if (shield.leak<newleak)
				shield.leak=newleak;
		}
		else if (randnum>=.7) {
			shield.recharge*=dam;
		}
		else if (randnum>=.5) {
			static float mindam=XMLSupport::parse_float(vs_config->getVariable("physics","min_recharge_shot_damage","0.5"));
			if (dam<mindam)
				dam=mindam;
			this->recharge*=dam;
		}
		else if (randnum>=.3) {
			static float mindam=XMLSupport::parse_float(vs_config->getVariable("physics","min_maxenergy_shot_damage","0.2"));
			if (dam<mindam)
				dam=mindam;
			this->maxenergy*=dam;
		}
		else if (randnum>=.2) {
			this->jump.damage+= float_to_int(100*(1-dam));
		}
		else {
			if (image->repair_droid>0)
				image->repair_droid--;
		}
		damages |= JUMP_DAMAGED;
		return;
	}
	if (degrees>=150&&degrees<=180) {
		//DAMAGE ENGINES
		if (randnum>=.8) {
			computer.max_combat_ab_speed*=dam;
		}
		else if (randnum>=.6) {
			computer.max_combat_speed*=dam;
		}
		else if (randnum>=.4) {
			limits.afterburn*=dam;
		}
		else if (randnum>=.2) {
			limits.vertical*=dam;
		}
		else {
			limits.forward*=dam;
		}
		damages |= LIMITS_DAMAGED;
		return;
	}
}


void Unit::Kill(bool erasefromsave, bool quitting)
{

	//if (erasefromsave)
	//  _Universe->AccessCockpit()->savegame->RemoveUnitFromSave((long)this);

	if (this->colTrees)
		this->colTrees->Dec();	 //might delete
	this->colTrees=NULL;
	if (this->sound->engine!=-1) {
		AUDStopPlaying (this->sound->engine);
		AUDDeleteSound (this->sound->engine);
	}
	if (this->sound->explode!=-1) {
		AUDStopPlaying (this->sound->explode);
		AUDDeleteSound (this->sound->explode);
	}
	if (this->sound->shield!=-1) {
		AUDStopPlaying (this->sound->shield);
		AUDDeleteSound (this->sound->shield);
	}
	if (this->sound->armor!=-1) {
		AUDStopPlaying (this->sound->armor);
		AUDDeleteSound (this->sound->armor);
	}
	if (this->sound->hull!=-1) {
		AUDStopPlaying (this->sound->hull);
		AUDDeleteSound (this->sound->hull);
	}
	if (this->sound->cloak!=-1) {
		AUDStopPlaying (this->sound->cloak);
		AUDDeleteSound (this->sound->cloak);
	}
	ClearMounts();

	if( SERVER && this->serial ) {
		VSServer->sendKill( this->serial, this->getStarSystem()->GetZone());
		this->serial=0;
	}

	if (docked&(DOCKING_UNITS)) {
		static float survival = XMLSupport::parse_float( vs_config->getVariable("physics","survival_chance_on_base_death","0.1") );
		static float player_survival = XMLSupport::parse_float( vs_config->getVariable("physics","player_survival_chance_on_base_death","1.0") );
		static int i_survival = float_to_int((RAND_MAX*survival));
		static int i_player_survival = float_to_int((RAND_MAX*player_survival));

		vector <Unit *> dockedun;
		unsigned int i;
		for (i=0;i<image->dockedunits.size();++i) {
			Unit * un;
			if (NULL!=(un=image->dockedunits[i]->uc.GetUnit()))
				dockedun.push_back (un);
		}
		while (!dockedun.empty()) {
			if (Network) _Universe->netLock(true);
			dockedun.back()->UnDock(this);
			if (Network) _Universe->netLock(false);
			if (rand() <= (UnitUtil::isPlayerStarship(dockedun.back())?i_player_survival:i_survival))
				dockedun.back()->Kill();
			dockedun.pop_back();
		}
	}

	//eraticate everything. naturally (see previous line) we won't erraticate beams erraticated above
	if (!isSubUnit())
		RemoveFromSystem();
	killed = true;
	computer.target.SetUnit (NULL);

	//God I can't believe this next line cost me 1 GIG of memory until I added it
	computer.threat.SetUnit (NULL);
	computer.velocity_ref.SetUnit(NULL);
	computer.force_velocity_ref = true;

	if(aistate) {
		aistate->ClearMessages();
		aistate->Destroy();
	}
	aistate=NULL;
	Unit *un;
	for(un_iter iter = getSubUnits();(un = *iter) != NULL;++iter){
		un->Kill();
	}
	if (isUnit()!=MISSILEPTR) {
		UNIT_LOG(logvs::INFO, "UNIT HAS DIED: %s %s (file %s)",name.get().c_str(),
				 fullname.c_str(), filename.get().c_str());
	}
	if (ucref==0) {
		Unitdeletequeue.push_back(this);
		if (flightgroup) {
			if (flightgroup->leader.GetUnit()==this) {
				flightgroup->leader.SetUnit(NULL);
			}
		}

#ifdef DESTRUCTDEBUG
		VSFileSystem::vs_fprintf (stderr,"%s 0x%x - %d\n",name.c_str(),this,Unitdeletequeue.size());
#endif
	}
}


void Unit::leach (float damShield, float damShieldRecharge, float damEnRecharge)
{
	recharge*=damEnRecharge;
	shield.recharge*=damShieldRecharge;
	switch (shield.number) {
		case 2:
			shield.shield2fb.frontmax*=damShield;
			shield.shield2fb.backmax*=damShield;
			break;
		case 4:
			shield.shield4fbrl.frontmax*=damShield;
			shield.shield4fbrl.backmax*=damShield;
			shield.shield4fbrl.leftmax*=damShield;
			shield.shield4fbrl.rightmax*=damShield;
			break;
		case 8:
			shield.shield8.frontrighttopmax*=damShield;
			shield.shield8.backrighttopmax*=damShield;
			shield.shield8.frontlefttopmax*=damShield;
			shield.shield8.backlefttopmax*=damShield;
			shield.shield8.frontrightbottommax*=damShield;
			shield.shield8.backrightbottommax*=damShield;
			shield.shield8.frontleftbottommax*=damShield;
			shield.shield8.backleftbottommax*=damShield;
			break;
	}
}


void Unit::UnRef()
{
#ifdef CONTAINER_DEBUG
	CheckUnit(this);
#endif
	ucref--;
	if (killed&&ucref==0) {
#ifdef CONTAINER_DEBUG
		deletedUn.Put (this,this);
#endif
								 //delete
		Unitdeletequeue.push_back(this);
#ifdef DESTRUCTDEBUG
		VSFileSystem::vs_fprintf (stderr,"%s 0x%x - %d\n",name.c_str(),this,Unitdeletequeue.size());
#endif
	}
}


float Unit::ExplosionRadius()
{
	static float expsize=XMLSupport::parse_float(vs_config->getVariable ("graphics","explosion_size","3"));
	return expsize*rSize();
}


								 //short fix
void Unit::ArmorData (float armor[8]) const
{
	armor[0]=this->armor.frontrighttop;
	armor[1]=this->armor.backrighttop;
	armor[2]=this->armor.frontlefttop;
	armor[3]=this->armor.backlefttop;
	armor[4]=this->armor.frontrightbottom;
	armor[5]=this->armor.backrightbottom;
	armor[6]=this->armor.frontleftbottom;
	armor[7]=this->armor.backleftbottom;
}


float Unit::WarpCapData () const
{
	return maxwarpenergy;
}


float Unit::FuelData () const
{
	return fuel;
}


float Unit::WarpEnergyData() const
{
	if (maxwarpenergy>0)
		return ((float)warpenergy)/((float)maxwarpenergy);
	if (jump.energy>0)
		return ((float)warpenergy)/((float)jump.energy);
	return 0.0f;
}


float Unit::EnergyData() const
{
	static bool max_shield_lowers_capacitance=XMLSupport::parse_bool(vs_config->getVariable("physics","max_shield_lowers_capacitance","false"));
	if (max_shield_lowers_capacitance) {
		if (maxenergy<=totalShieldEnergyCapacitance(shield)) {
			return 0;
		}
		return ((float)energy)/(maxenergy-totalShieldEnergyCapacitance(shield));
	}
	else {
		return ((float)energy)/maxenergy;
	}
}


float Unit::FShieldData() const
{
	switch (shield.number) {
		case 2: { if( shield.shield2fb.frontmax!=0) return shield.shield2fb.front/shield.shield2fb.frontmax;}
		break;
		case 4: { if( shield.shield4fbrl.frontmax!=0) return (shield.shield4fbrl.front)/shield.shield4fbrl.frontmax;}
		break;
		case 8: { if( shield.shield8.frontrighttopmax!=0||shield.shield8.frontrightbottommax!=0||shield.shield8.frontlefttopmax!=0||shield.shield8.frontleftbottommax!=0) return (shield.shield8.frontrighttop+shield.shield8.frontrightbottom+shield.shield8.frontlefttop+shield.shield8.frontleftbottom)/(shield.shield8.frontrighttopmax+shield.shield8.frontrightbottommax+shield.shield8.frontlefttopmax+shield.shield8.frontleftbottommax);}
		break;
	}
	return 0;
}


float Unit::BShieldData() const
{
	switch (shield.number) {
		case 2: { if( shield.shield2fb.backmax!=0) return shield.shield2fb.back/shield.shield2fb.backmax;}
		break;
		case 4: { if( shield.shield4fbrl.backmax!=0) return (shield.shield4fbrl.back)/shield.shield4fbrl.backmax;}
		break;
		case 8: { if( shield.shield8.backrighttopmax!=0||shield.shield8.backrightbottommax!=0||shield.shield8.backlefttopmax!=0||shield.shield8.backleftbottommax!=0) return (shield.shield8.backrighttop+shield.shield8.backrightbottom+shield.shield8.backlefttop+shield.shield8.backleftbottom)/(shield.shield8.backrighttopmax+shield.shield8.backrightbottommax+shield.shield8.backlefttopmax+shield.shield8.backleftbottommax);}
		break;
	}
	return 0;
}


float Unit::LShieldData() const
{
	switch (shield.number) {
		case 2: return 0;		 //no data, captain
		case 4: { if( shield.shield4fbrl.leftmax!=0) return (shield.shield4fbrl.left)/shield.shield4fbrl.leftmax;}
		break;
		case 8: { if( shield.shield8.backlefttopmax!=0||shield.shield8.backleftbottommax!=0||shield.shield8.frontlefttopmax!=0||shield.shield8.frontleftbottommax!=0) return (shield.shield8.backlefttop+shield.shield8.backleftbottom+shield.shield8.frontlefttop+shield.shield8.frontleftbottom)/(shield.shield8.backlefttopmax+shield.shield8.backleftbottommax+shield.shield8.frontlefttopmax+shield.shield8.frontleftbottommax);}
		break;
	}
	return 0;
}


float Unit::RShieldData() const
{
	switch (shield.number) {
		case 2: return 0;		 //don't react to stuff we have no data on
		case 4: { if( shield.shield4fbrl.rightmax!=0) return (shield.shield4fbrl.right)/shield.shield4fbrl.rightmax;}
		break;
		case 8: { if( shield.shield8.backrighttopmax!=0||shield.shield8.backrightbottommax!=0||shield.shield8.frontrighttopmax!=0||shield.shield8.frontrightbottommax!=0) return (shield.shield8.backrighttop+shield.shield8.backrightbottom+shield.shield8.frontrighttop+shield.shield8.frontrightbottom)/(shield.shield8.backrighttopmax+shield.shield8.backrightbottommax+shield.shield8.frontrighttopmax+shield.shield8.frontrightbottommax);}
		break;
	}
	return 0;
}


void Unit::ProcessDeleteQueue()
{
	while (!Unitdeletequeue.empty()) {
#ifdef DESTRUCTDEBUG
		VSFileSystem::vs_fprintf (stderr,"Eliminatin' 0x%x - %d",Unitdeletequeue.back(),Unitdeletequeue.size());
		fflush (stderr);
		VSFileSystem::vs_fprintf (stderr,"Eliminatin' %s\n",Unitdeletequeue.back()->name.c_str());
#endif
#ifdef DESTRUCTDEBUG
		if (Unitdeletequeue.back()->isSubUnit()) {

			VSFileSystem::vs_fprintf (stderr,"Subunit Deleting (related to double dipping)");

		}
#endif
		Unit * mydeleter = Unitdeletequeue.back();
		Unitdeletequeue.pop_back();
		delete mydeleter;		 ///might modify unitdeletequeue

#ifdef DESTRUCTDEBUG
		VSFileSystem::vs_fprintf (stderr,"Completed %d\n",Unitdeletequeue.size());
		fflush (stderr);
#endif

	}
}


Unit * makeBlankUpgrade (const std::string & templnam, int faction)
{
	Unit * bl =  UnitFactory::createServerSideUnit (templnam.c_str(),true,faction);
	for (int i= bl->numCargo()-1;i>=0;i--) {
		int q =bl->GetCargo (i).quantity;
		bl->RemoveCargo (i,q);
	}
	bl->Mass=0;
	return bl;
}


static const string LOAD_FAILED = "LOAD_FAILED";

const Unit * makeFinalBlankUpgrade (const std::string & name, int faction)
{
	char * unitdir = GetUnitDir(name.c_str());
	string limiternam = name;
	if (unitdir!=name)
		limiternam=string(unitdir)+string(".blank");
	free (unitdir);
	const Unit * lim= UnitConstCache::getCachedConst (StringIntKey(limiternam,faction));
	if (!lim)
		lim = UnitConstCache::setCachedConst(StringIntKey(limiternam,faction),makeBlankUpgrade(limiternam,faction));
	if (lim->name == LOAD_FAILED) {
		lim=NULL;
	}
	return lim;
}


const Unit * makeTemplateUpgrade (const std::string & name, int faction)
{
	char * unitdir = GetUnitDir(name.c_str());
	string limiternam = string(unitdir)+string(".template");
	free (unitdir);
	const Unit * lim= UnitConstCache::getCachedConst (StringIntKey(limiternam,faction));
	if (!lim)
		lim = UnitConstCache::setCachedConst(StringIntKey(limiternam,faction),UnitFactory::createUnit(limiternam.c_str(),true,faction));
	if (lim->name == LOAD_FAILED) {
		lim=NULL;
	}
	return lim;
}


const Unit * loadUnitByCache(const std::string & name,int faction)
{
	const Unit * temprate= UnitConstCache::getCachedConst (StringIntKey(name,faction));
	if (!temprate)
		temprate = UnitConstCache::setCachedConst(StringIntKey(name,faction),UnitFactory::createUnit(name.c_str(),true,faction));
	return temprate;
}


bool DestroySystem (float hull, float maxhull, float numhits)
{
	static float damage_chance=XMLSupport::parse_float(vs_config->getVariable ("physics","damage_chance",".005"));
	static float guaranteed_chance=XMLSupport::parse_float(vs_config->getVariable("physics","definite_damage_chance",".1"));
	float chance = 1-(damage_chance*(guaranteed_chance+(maxhull-hull)/maxhull));
	if (numhits>1)
		chance=pow (chance,numhits);
	return (rand01()>chance);
}


bool DestroyPlayerSystem (float hull, float maxhull, float numhits)
{
	static float damage_chance=XMLSupport::parse_float(vs_config->getVariable ("physics","damage_player_chance",".5"));
	static float guaranteed_chance=XMLSupport::parse_float(vs_config->getVariable("physics","definite_damage_chance",".1"));
	float chance = 1-(damage_chance*(guaranteed_chance+(maxhull-hull)/maxhull));
	if (numhits>1)
		chance=pow (chance,numhits);
	bool ret = (rand01()>chance);
	if (ret) {
		//		printf("DAAAAAAMAGED!!!!\n");
	}
	return ret;
}


const char * DamagedCategory="upgrades/Damaged/";
								 //short fix
float Unit::DealDamageToHullReturnArmor (const Vector & pnt, float damage, float * &targ)
{
	float percent;
#ifndef ISUCK
	if (hull<0) {
		return -1;
	}
#endif
	if (pnt.i>0) {
		if (pnt.j>0) {
			if(pnt.k>0) {
				targ=&armor.frontlefttop;
			}
			else {
				targ=&armor.backlefttop;
			}
		}
		else {
			if(pnt.k>0) {
				targ=&armor.frontleftbottom;
			}
			else {
				targ=&armor.backleftbottom;
			}
		}
	}
	else {
		if (pnt.j>0) {
			if(pnt.k>0) {
				targ=&armor.frontrighttop;
			}
			else {
				targ=&armor.backrighttop;
			}
		}
		else {
			if(pnt.k>0) {
				targ=&armor.frontrightbottom;
			}
			else {
				targ=&armor.backrightbottom;
			}
		}
	}
								 //short fix
	unsigned int biggerthan=float_to_int(*targ);
	float absdamage = damage>=0?damage:-damage;
	float denom=(*targ+hull);
	percent = (denom>absdamage&&denom!=0)?absdamage/denom:(denom==0?0.0:1.0);

	// ONLY APLY DAMAGE ON SERVER SIDE
	if( Network==NULL || SERVER) {
		if( percent == -1)
			return -1;
		static float damage_factor_for_sound=XMLSupport::parse_float(vs_config->getVariable("audio","damage_factor_for_sound",".001"));
		bool did_hull_damage=true;
		if (absdamage<*targ) {
			if ((*targ)*damage_factor_for_sound<=absdamage) {
				ArmorDamageSound( pnt);
			}
								 //short fix
			*targ -= apply_float_to_unsigned_int(absdamage);
			did_hull_damage=false;
		}
		static bool system_damage_on_armor=XMLSupport::parse_bool(vs_config->getVariable("physics","system_damage_on_armor","false"));
		if (system_damage_on_armor||did_hull_damage) {
			if (did_hull_damage) {
				absdamage -= *targ;
				damage= damage>=0?absdamage:-absdamage;
				*targ= 0;
			}
			if (numCargo()>0) {
				if (DestroySystem(hull,maxhull,numCargo())) {
					int which = rand()%numCargo();
					static std::string Restricted_items=vs_config->getVariable("physics","indestructable_cargo_items","");
								 //why not downgrade _add GetCargo(which).content.find("add_")!=0&&
					if (GetCargo(which).GetCategory().find("upgrades/")==0&& GetCargo(which).GetCategory().find(DamagedCategory)!=0 &&GetCargo(which).GetContent().find("mult_")!=0&&Restricted_items.find(GetCargo(which).GetContent())==string::npos) {
						int lenupgrades = strlen("upgrades/");
						GetCargo(which).category = string(DamagedCategory)+GetCargo(which).GetCategory().substr(lenupgrades);
						static bool NotActuallyDowngrade=XMLSupport::parse_bool(vs_config->getVariable("physics","separate_system_flakiness_component","false"));
						if (!NotActuallyDowngrade) {
							const Unit * downgrade=loadUnitByCache(GetCargo(which).content,FactionUtil::GetFactionIndex("upgrades"));
							if (downgrade) {
								if (0==downgrade->GetNumMounts()&&downgrade->SubUnits.empty()) {
									double percentage=0;
									this->Downgrade(downgrade,0,0,percentage,NULL);
								}
							}
						}
					}
				}
			}

			bool isplayer = _Universe->isPlayerStarship(this);
								 //hull > damage is similar to hull>absdamage|| damage<0
			if ((!isplayer)||_Universe->AccessCockpit()->godliness<=0||hull>damage||system_damage_on_armor) {
				static float system_failure=XMLSupport::parse_float(vs_config->getVariable ("physics","indiscriminate_system_destruction",".25"));
				if ((!isplayer)&&DestroySystem(hull,maxhull,1)) {

					DamageRandSys(system_failure*rand01()+(1-system_failure)*(1-(hull>0?absdamage/hull:1.0f)),pnt);
				}
				else if (isplayer&&DestroyPlayerSystem(hull,maxhull,1)) {
					DamageRandSys(system_failure*rand01()+(1-system_failure)*(1-(hull>0?absdamage/hull:1.0f)),pnt);
				}

				if (did_hull_damage) {
					if (damage>0) {
						if (hull*damage_factor_for_sound<=damage) {
							HullDamageSound( pnt);
						}
								 //FIXME
						hull -=damage;
					}
					else {// DISABLING WEAPON CODE HERE
						static float disabling_constant=XMLSupport::parse_float(vs_config->getVariable("physics","disabling_weapon_constant","1"));
						if (hull>0)
							image->LifeSupportFunctionality+=disabling_constant*damage/hull;
						if (image->LifeSupportFunctionality<0) {
							image->LifeSupportFunctionalityMax+=image->LifeSupportFunctionality;
							image->LifeSupportFunctionality=0;
							if (image->LifeSupportFunctionalityMax<0)
								image->LifeSupportFunctionalityMax=0;
						}
						/*
						recharge+=damage;
						shield.recharge+=damage;
						if (recharge<0)
							recharge=0;
						if (shield.recharge<0)
							shield.recharge=0;*/
					}
				}
			}
			else {
				_Universe->AccessCockpit()->godliness-=absdamage;
				if (DestroyPlayerSystem(hull,maxhull,1)) {
								 //get system damage...but live!
					DamageRandSys(rand01()*.5+.2,pnt);
				}
			}
		}
		//if (*targ>biggerthan)
		//VSFileSystem::vs_fprintf (stderr,"errore fatale mit den armorn")
		;
		if (hull <0) {
			int neutralfac=FactionUtil::GetNeutralFaction();
			int upgradesfac=FactionUtil::GetUpgradeFaction();

			static float cargoejectpercent = XMLSupport::parse_float(vs_config->getVariable ("physics","eject_cargo_percent","1"));

			static float hulldamtoeject = XMLSupport::parse_float(vs_config->getVariable ("physics","hull_damage_to_eject","100"));
			//			if (!isSubUnit()&&hull>-hulldamtoeject) {
			if (hull>-hulldamtoeject) {
				static float autoejectpercent = XMLSupport::parse_float(vs_config->getVariable ("physics","autoeject_percent",".5"));
				if (SERVER||Network==NULL)
				if (rand()<(RAND_MAX*autoejectpercent)&&isUnit()==UNITPTR) {
					static bool player_autoeject=XMLSupport::parse_bool(vs_config->getVariable("physics","player_autoeject","true"));
					if (faction!=neutralfac&&faction!=upgradesfac&&(player_autoeject||NULL==_Universe->isPlayerStarship(this)))
						EjectCargo ((unsigned int)-1);
				}
			}
			static unsigned int max_dump_cargo=XMLSupport::parse_int(vs_config->getVariable("physics","max_dumped_cargo","15"));
			unsigned int dumpedcargo=0;
			if (SERVER||Network==NULL)
			if (faction!=neutralfac&&faction!=upgradesfac) {
				for (unsigned int i=0;i<numCargo();++i) {
					if (rand()<(RAND_MAX*cargoejectpercent)&&dumpedcargo++<max_dump_cargo) {
						EjectCargo(i);
					}
				}
			}

#ifdef ISUCK
			Destroy();
#endif
			PrimeOrders();
			maxenergy=energy=0;

			Split (rand()%3+1);
#ifndef ISUCK
			Destroy();
			return -1;
#endif
		}
	}
	/////////////////////////////
	if (!FINITE (percent))
		percent = 0;
	return percent;
}


bool withinShield(const ShieldFacing& facing, float theta, float rho)
{
	float theta360=theta+2*3.1415926536;
	return rho>=facing.rhomin&&rho<facing.rhomax&&((theta>=facing.thetamin&&theta<facing.thetamax)||(theta360>=facing.thetamin&&theta360<facing.thetamax));
}


float Unit::DealDamageToShield (const Vector &pnt, float &damage)
{
	int index;
	float percent=0;
	float * targ=NULL;			 //short fix
	float theta = atan2 (pnt.i,pnt.k);
	float rho=atan(pnt.j/sqrt(pnt.k*pnt.k+pnt.i*pnt.i));
	// ONLY APPLY DAMAGES IN NON-NETWORKING OR ON SERVER SIDE
	for (int i=0;i<shield.number;++i) {
		if (withinShield(shield.range[i],theta,rho)) {
			if (shield.shield.max[i]) {
								 //comparing with max
				float tmp=damage/shield.shield.max[i];
				if (tmp>percent) percent=tmp;
			}
			targ=&shield.shield.cur[i];
			if( Network==NULL || SERVER) {
				if (damage>*targ) {
					damage -= *targ;
					*targ=0;
				}
				else {
								 //short fix
					*targ-=damage;
					damage = 0;
					break;
				}
			}
		}
	}
	if (!FINITE (percent))
		percent = 0;
	return percent;
}


bool Unit::ShieldUp (const Vector &pnt) const
{
	const int shieldmin=5;
	int index;
	static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
	if (nebula!=NULL||nebshields>0)
		return false;
	switch (shield.number) {
		case 2:
			return ((pnt.k>0)?(shield.shield2fb.front):(shield.shield2fb.back))>shieldmin;
			break;
		case 8:
			if (pnt.i>0) {
				if(pnt.j>0) {
					if(pnt.k>0) {
						return shield.shield8.frontlefttop>shieldmin;
					}
					else {
						return shield.shield8.backlefttop>shieldmin;
					}
				}
				else {
					if(pnt.k>0) {
						return shield.shield8.frontleftbottom>shieldmin;
					}
					else {
						return shield.shield8.backleftbottom>shieldmin;
					}
				}
			}
			else {
				if(pnt.j>0) {
					if(pnt.k>0) {
						return shield.shield8.frontrighttop>shieldmin;
					}
					else {
						return shield.shield8.backrighttop>shieldmin;
					}
				}
				else {
					if(pnt.k>0) {
						return shield.shield8.frontrightbottom>shieldmin;
					}
					else {
						return shield.shield8.backrightbottom>shieldmin;
					}
				}
			}

			break;
		case 4:
			if (fabs(pnt.k)>fabs (pnt.i)) {
				if (pnt.k>0) {
					return shield.shield4fbrl.front>shieldmin;
				}
				else {
					return shield.shield4fbrl.back>shieldmin;
				}
			}
			else {
				if (pnt.i>0) {
					return shield.shield4fbrl.left>shieldmin;
				}
				else {
					return shield.shield4fbrl.right>shieldmin;
				}
			}
			return false;
		default:
			return false;
	}
}


/***********************************************************************************/
/**** UNIT_WEAPON STUFF                                                            */
/***********************************************************************************/

void Unit::TargetTurret (Unit * targ)
{
	if (!SubUnits.empty()) {
		Unit * su;
		bool inrange = (targ!=NULL)?InRange(targ):true;
		if (inrange) {
			for(un_iter iter = getSubUnits();(su = *iter) != NULL;++iter){
				su->Target (targ);
				su->TargetTurret(targ);
			}
		}
	}
}


void WarpPursuit(Unit* un, StarSystem * sourcess, const std::string & destination)
{
	static bool AINotUseJump=XMLSupport::parse_bool(vs_config->getVariable("physics","no_ai_jump_points","false"));
	if (AINotUseJump) {
		static float seconds_per_parsec=XMLSupport::parse_float(vs_config->getVariable("physics","seconds_per_parsec","10"));
		float ttime=(SystemLocation(sourcess->getFileName())-SystemLocation(destination)).Magnitude()*seconds_per_parsec;
		un->jump.delay+=float_to_int(ttime);
		sourcess->JumpTo(un,NULL,destination,true,true);
		un->jump.delay-=float_to_int(ttime);
	}

}


// WARNING : WHEN TURRETS WE MAY NOT WANT TO ASK THE SERVER FOR INFOS ! ONLY FOR LOCAL PLAYERS (_Universe-isStarship())
void Unit::LockTarget(bool myboo)
{
	computer.radar.locked=myboo;

	if (myboo&&computer.radar.canlock==false&&false==UnitUtil::isSignificant(Target())) {
		computer.radar.locked=false;
	}
}


void Unit::Target (Unit *targ)
{
	if (targ==this) {
		return;
	}
	ObjSerial oldtarg=0;
	if (computer.target.GetUnit()) {
		oldtarg = computer.target.GetUnit()->GetSerial();
	}

	if (!(activeStarSystem==NULL||activeStarSystem==_Universe->activeStarSystem())) {

		if (SERVER&&computer.target.GetUnit()!=NULL)
			VSServer->BroadcastTarget(GetSerial(), oldtarg, 0, this->getStarSystem()->GetZone());
		computer.target.SetUnit(NULL);
		return;
		/*
		VSFileSystem::vs_fprintf (stderr,"bad target system");
		const int BADTARGETSYSTEM=0;
		assert (BADTARGETSYSTEM);
		*/
	}
	if (targ) {
		if (targ->activeStarSystem==_Universe->activeStarSystem()||targ->activeStarSystem==NULL) {
			if (targ!=Unit::Target()) {
				for (int i=0;i<GetNumMounts();++i) {
					mounts[i].time_to_lock = mounts[i].type->LockTime;
				}
				if (SERVER&&computer.target.GetUnit()!=targ)
					VSServer->BroadcastTarget(GetSerial(), oldtarg, targ->GetSerial(), this->getStarSystem()->GetZone());
				computer.target.SetUnit(targ);
				LockTarget(false);
			}
		}
		else {
			if (jump.drive!=-1) {
				bool found=false;
				un_iter i= _Universe->activeStarSystem()->getUnitList().createIterator();
				Unit * u;
				for (;(u=*i)!=NULL;++i) {
					if (!u->GetDestinations().empty()) {
						if (std::find (u->GetDestinations().begin(),u->GetDestinations().end(),targ->activeStarSystem->getFileName())!=u->GetDestinations().end()) {
							Target (u);
							ActivateJumpDrive(0);
							found=true;
						}
					}
				}
				if (!found&&!_Universe->isPlayerStarship(this)) {
					WarpPursuit(this,_Universe->activeStarSystem(),targ->getStarSystem()->getFileName());
				}
			}
			else {
				if (SERVER&&computer.target.GetUnit()!=NULL)
					VSServer->BroadcastTarget(GetSerial(), oldtarg, 0, this->getStarSystem()->GetZone());
				computer.target.SetUnit(NULL);
			}
		}
	}
	else {
		if (SERVER&&computer.target.GetUnit()!=NULL)
			VSServer->BroadcastTarget(GetSerial(), oldtarg, 0, this->getStarSystem()->GetZone());
		computer.target.SetUnit(NULL);
	}
}


void Unit::VelocityReference (Unit *targ)
{
	computer.force_velocity_ref = !!targ;
	computer.velocity_ref.SetUnit(targ);
}


void Unit::SetOwner(Unit *target)
{
	owner=target;
}


void Unit::Cloak (bool loak)
{
	damages|=CLOAK_DAMAGED;
	if (loak) {
		static bool warp_energy_for_cloak=XMLSupport::parse_bool(vs_config->getVariable("physics","warp_energy_for_cloak","true"));
		if (image->cloakenergy<(warp_energy_for_cloak?warpenergy:energy)) {
			image->cloakrate =(image->cloakrate>=0)?image->cloakrate:-image->cloakrate;
								 //short fix
			if (cloaking<-1&&image->cloakrate!=0) {
								 //short fix
				cloaking=2147483647;
			}
			else {

			}
		}
	}
	else {
		image->cloakrate= (image->cloakrate>=0)?-image->cloakrate:image->cloakrate;
		if (cloaking==cloakmin)
			++cloaking;
	}
}


void Unit::SelectAllWeapon (bool Missile)
{
	for (int i=0;i<GetNumMounts();++i) {
		if (mounts[i].status<Mount::DESTROYED) {
			if (mounts[i].type->size!=weapon_info::SPECIAL) {
				mounts[i].Activate (Missile);
			}
		}
	}
}


void Mount::Activate (bool Missile)
{
	if ((isMissile(type))==Missile) {
		if (status==INACTIVE)
			status = ACTIVE;
	}
}


///Sets this gun to inactive, unless unchosen or destroyed
void Mount::DeActive (bool Missile)
{
	if ((isMissile(type))==Missile) {
		if (status==ACTIVE)
			status = INACTIVE;
	}
}


void Unit::UnFire ()
{
	if (this->GetNumMounts()==0) {
		Unit * tur=NULL;
		for (un_iter i=this->getSubUnits();(tur=*i)!=NULL;++i) {
			tur->UnFire();
		}
	}
	else {
		int playernum = _Universe->whichPlayerStarship( this);
		vector<int> unFireRequests;
		for (int i=0;i<GetNumMounts();++i) {
			if (mounts[i].status!=Mount::ACTIVE)
				continue;
			if ((SERVER || (Network && playernum>=0)) && mounts[i].processed != Mount::UNFIRED)
				unFireRequests.push_back(i);
			mounts[i].UnFire();	 //turns off beams;
		}
		if (!unFireRequests.empty()) {
			if (SERVER) {
				VSServer->BroadcastUnfire( this->serial, unFireRequests, this->getStarSystem()->GetZone());
			}
			else {
				Network[playernum].unfireRequest( this->serial, unFireRequests);
			}
		}
	}
}


///cycles through the loop twice turning on all matching to ms weapons of size or after size
void Unit::ActivateGuns (const weapon_info * sz, bool ms)
{
	for (int j=0;j<2;++j) {
		for (int i=0;i<GetNumMounts();++i) {
			if (mounts[i].type==sz) {
				if (mounts[i].status<Mount::DESTROYED&&mounts[i].ammo!=0&&(isMissile(mounts[i].type)==ms)) {
					mounts[i].Activate(ms);
				}
				else {
					sz = mounts[(i+1)%GetNumMounts()].type;
				}
			}
		}
	}
}


typedef std::set<int> WeaponGroup;

template<bool FORWARD> class WeaponComparator
{
	public:
		bool operator() (const WeaponGroup &a, const WeaponGroup &b) const
		{
			if (a.size()==b.size()) {
				for (WeaponGroup::const_iterator iterA=a.begin(), iterB=b.begin();
					iterA!=a.end()&&iterB!=b.end();
				++iterA, ++iterB) {
					if ((*iterA)<(*iterB)) {
						return FORWARD;
					}
					else if ((*iterB)<(*iterA)) {
						return (!FORWARD);
					}
				}
				return false;
			}
			else if (a.size()<b.size()) {
				return FORWARD;
			}
			else {
				return (!FORWARD);
			}
		}

		typedef std::set<WeaponGroup, WeaponComparator<FORWARD> > WeaponGroupSet;

		static bool checkmount(Unit *un, int i, bool missile) {
			return (un->mounts[i].status<Mount::DESTROYED&&((isMissile(un->mounts[i].type))==missile)&&un->mounts[i].ammo!=0);
		}

		static bool isSpecial(const Mount &mount) {
			return mount.type->size==weapon_info::SPECIAL||mount.type->size==weapon_info::SPECIALMISSILE;
		}

		static bool notSpecial(const Mount &mount) {
			return !isSpecial(mount);
		}

		static void ToggleWeaponSet(Unit *un, bool missile) {
			if (un->mounts.size()==0) {
				Unit * tur=NULL;
				for (un_iter i=un->getSubUnits();(tur=*i)!=NULL;++i) {
					ToggleWeaponSet(tur,missile);
				}
				return;
			}
			WeaponGroup lightMissiles;
			WeaponGroup heavyMissiles;
			WeaponGroup allWeapons;
			WeaponGroup allWeaponsNoSpecial;
			WeaponGroupSet myset;
			unsigned int i;
			typename WeaponGroupSet::const_iterator iter;
			UNIT_LOG(logvs::INFO, "ToggleWeaponSet: %s", FORWARD?"true":"false");
			for (i=0;i<un->mounts.size();++i) {
				if (checkmount(un,i,missile)) {
					WeaponGroup mygroup;
					for (unsigned int j=0;j<un->mounts.size();++j) {
						if (un->mounts[j].type==un->mounts[i].type) {
							if (checkmount(un,j,missile)) {
								mygroup.insert(j);
							}
						}
					}
								 // WIll ignore if already there.
					myset.insert(mygroup);
					allWeapons.insert(i);
					if (notSpecial(un->mounts[i])) {
						allWeaponsNoSpecial.insert(i);
					}
				}
			}
			const WeaponGroupSet mypairset (myset);
			for (iter=mypairset.begin();iter!=mypairset.end();++iter) {
				if ((*iter).size()&&notSpecial(un->mounts[(*((*iter).begin()))])) {
					typename WeaponGroupSet::const_iterator iter2;
					for (iter2=mypairset.begin();iter2!=mypairset.end();++iter2) {
						if ((*iter2).size()&&notSpecial(un->mounts[(*((*iter2).begin()))])) {
							WeaponGroup myGroup;
							set_union((*iter).begin(), (*iter).end(), (*iter2).begin(), (*iter2).end(),
								inserter(myGroup, myGroup.begin()));
							myset.insert(myGroup);
						}
					}
				}
			}
			static bool allow_special_with_weapons=XMLSupport::parse_bool(vs_config->getVariable("physics","special_and_normal_gun_combo","true"));
			if (allow_special_with_weapons)
				myset.insert(allWeapons);
			myset.insert(allWeaponsNoSpecial);
			for (iter=myset.begin();iter!=myset.end();++iter) {
                if (UNIT_LOG_START(logvs::INFO, "WeaponGroup: ") > 0) {
                    for (WeaponGroup::const_iterator iter2=(*iter).begin();iter2!=(*iter).end();++iter2) {
                        logvs::log_printf("%d:%s ", *iter2, un->mounts[*iter2].type->weapon_name.c_str());
                    }
                    UNIT_LOG_END(logvs::INFO, NULL);
                }
			}
			WeaponGroup activeWeapons;
			int logging = UNIT_LOG_START(logvs::INFO, "CURRENT: ");
			for (i=0;i<un->mounts.size();++i) {
				if (un->mounts[i].status==Mount::ACTIVE&&checkmount(un,i,missile)) {
					activeWeapons.insert(i);
					if (logging) logvs::log_printf("%d:%s ", i, un->mounts[i].type->weapon_name.c_str());
				}
			}
			if (logging) UNIT_LOG_END(logvs::INFO, NULL);
			iter=myset.upper_bound(activeWeapons);
			if (iter==myset.end()) {
				iter=myset.begin();
			}
			if (iter==myset.end()) {
				return;
			}
			for (i=0;i<un->mounts.size();++i) {
				un->mounts[i].DeActive(missile);
			}
			logging = UNIT_LOG_START(logvs::INFO, "ACTIVE: ");
			for (WeaponGroup::const_iterator iter2=(*iter).begin();iter2!=(*iter).end();++iter2) {
				if (logging) logvs::log_printf("%d:%s ", *iter2, un->mounts[*iter2].type->weapon_name.c_str());
				un->mounts[*iter2].Activate(missile);
			}
			if (logging) UNIT_LOG_END(logvs::INFO, NULL);
			UNIT_LOG(logvs::INFO, "ToggleWeapon end...");
		}
};

void Unit::ToggleWeapon (bool missile, bool forward)
{
	if (forward) {
		WeaponComparator<true>::ToggleWeaponSet(this, missile);
	}
	else {
		WeaponComparator<false>::ToggleWeaponSet(this, missile);
	}
}


/*
///In short I have NO CLUE how this works! It just...grudgingly does
void Unit::ToggleWeapon (bool Missile) {
  int activecount=0;
  int totalcount=0;
  bool lasttotal=true;
//  weapon_info::MOUNT_SIZE sz = weapon_info::NOWEAP;
  const weapon_info * sz=NULL;
  if (GetNumMounts()<1)
	return;
  sz = mounts[0].type;
  if (Missile) {
	  int whichmissile=-2;//-2 means not choosen -1 means all
	  int lastmissile=-2;
	  int count=0;
	  for (int i=0;i<GetNumMounts();++i) {
		  if (mounts[i].type->type==weapon_info::PROJECTILE&&mounts[i].status<Mount::DESTROYED) {
			  if( mounts[i].status==Mount::ACTIVE) {
				  if (whichmissile==-2) {
					  whichmissile=count;
				  }else {
					  whichmissile=-1;
				  }
			  }
			  lastmissile=count;
			  count++;
		  }
	  }
	  if (lastmissile!=-2) {
		  bool found=false;

		  if (whichmissile==-1||whichmissile!=lastmissile){
			  whichmissile++;
			  //activate whichmissile
			  int count=0;
			  for (unsigned int i=0;i<GetNumMounts();++i) {
				  if (mounts[i].type->type==weapon_info::PROJECTILE&&mounts[i].status<Mount::DESTROYED) {
					  if (count==whichmissile) {
						  mounts[i].status = Mount::ACTIVE;
						  found=true;
					  }else {
						  mounts[i].status = Mount::INACTIVE;
					  }
					  count++;
				  }
			  }
		  }

		  if (!found||whichmissile==lastmissile||whichmissile==-2) {
			  //activate all
			  SelectAllWeapon(true);
		  }

	  }
  }else {
  for (int i=0;i<GetNumMounts();i++) {
	  if ((mounts[i].type->type==weapon_info::PROJECTILE)==Missile&&!Missile&&mounts[i].status<Mount::DESTROYED) {
		  if (mounts[i].type->size!=weapon_info::SPECIAL)
			  totalcount++;
	  lasttotal=false;
	  if (mounts[i].status==Mount::ACTIVE) {
	activecount++;
	lasttotal=true;
	mounts[i].DeActive (Missile);
	if (i==GetNumMounts()-1) {
	  sz=mounts[0].type;
	}else {
	  sz =mounts[i+1].type;
	}
	  }
	}
	if ((mounts[i].type->type==weapon_info::PROJECTILE)==Missile&&Missile&&mounts[i].status<Mount::DESTROYED) {
	  if (mounts[i].status==Mount::ACTIVE) {
	activecount++;//totalcount=0;
	mounts[i].DeActive (Missile);
	if (lasttotal) {
	  totalcount=(i+1)%GetNumMounts();
	  if (i==GetNumMounts()-1) {
		sz = mounts[0].type;
	  }else {
		sz =mounts[i+1].type;
	  }
	}
	lasttotal=false;
	  }
	}
  }
  if (Missile) {
	int i=totalcount;
	for (int j=0;j<2;j++) {
	  for (;i<GetNumMounts();i++) {
	if (mounts[i].type==sz) {
	  if ((mounts[i].type->type==weapon_info::PROJECTILE)) {
		mounts[i].Activate(true);
		return;
	  }else {
		sz = mounts[(i+1)%GetNumMounts()].type;
	  }
	}
	  }
	  i=0;
	}
  }
  if (totalcount==activecount) {
	  ActivateGuns (mounts[0].type,Missile);
  } else {
	if (lasttotal) {
	  SelectAllWeapon(Missile);
	}else {
	  ActivateGuns (sz,Missile);
	}
  }
  }
}
*/

void Unit::SetRecursiveOwner(Unit *target)
{
	owner=target;
	Unit * su;
	for(un_iter iter = getSubUnits();(su = *iter) != NULL;++iter)
		su->SetRecursiveOwner (target);
}


int Unit::LockMissile() const
{
	bool missilelock=false;
	bool dumblock=false;
	for (int i=0;i<GetNumMounts();++i) {
		if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime>0&&mounts[i].time_to_lock<=0&&isMissile(mounts[i].type)) {
			missilelock=true;
		}
		else {
			if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime==0&&isMissile(mounts[i].type)&&mounts[i].time_to_lock<=0) {
				dumblock=true;
			}
		}
	}
	return (missilelock?1:(dumblock?-1:0));
}


/***********************************************************************************/
/**** UNIT_COLLIDE STUFF                                                            */
/***********************************************************************************/

// virtual
bool Unit::Explode(bool draw, float timeit)
{
	return true;
}


void Unit::Destroy()
{
	if (!killed) {
		if (hull >= 0)
			hull = -1;
		for (int beamcount=0;beamcount<GetNumMounts();++beamcount) {
			DestroyMount(&mounts[beamcount]);
		}
		// The server send a kill notification to all concerned clients but not if it is an upgrade

		if( SERVER && this->serial ) {
			VSServer->sendKill( this->serial, this->getStarSystem()->GetZone());
			this->serial=0;
		}

		if (!Explode(false,SIMULATION_ATOM)) {

			Kill();
		}
	}
}


void Unit::SetCollisionParent (Unit * name)
{
	assert (0);					 //deprecated... many less collisions with subunits out of the table
#if 0
	for (int i=0;i<numsubunit;++i) {
		subunits[i]->CollideInfo.object.u = name;
		subunits[i]->SetCollisionParent (name);
	}
#endif
}


double Unit::getMinDis (const QVector &pnt)
{
	float minsofar=1e+10;
	float tmpvar;
	int i;
	Vector TargetPoint (cumulative_transformation_matrix.getP());

#ifdef VARIABLE_LENGTH_PQR
								 //the scale factor of the current UNIT
	float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#endif
	for (i=0;i<nummesh();++i) {

		TargetPoint = (Transform(cumulative_transformation_matrix,meshdata[i]->Position()).Cast()-pnt).Cast();
		tmpvar = sqrtf (TargetPoint.Dot (TargetPoint))-meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
			*SizeScaleFactor
#endif
			;
		if (tmpvar<minsofar) {
			minsofar = tmpvar;
		}
	}
	Unit * su;
	for(un_iter ui = getSubUnits();(su = *ui) != NULL;++ui){
		tmpvar = su->getMinDis (pnt);
		if (tmpvar<minsofar) {
			minsofar=tmpvar;
		}
	}
	return minsofar;
}


// This function should not be used on server side
extern vector<Vector> perplines;
extern vector <int> turretcontrol;
float Unit::querySphereClickList (const QVector &st, const QVector &dir, float err) const
{
	int i;
	float retval=0;
	float adjretval=0;
	const Matrix * tmpo = &cumulative_transformation_matrix;

	Vector TargetPoint (tmpo->getP());
	for (i=0;i<nummesh();++i) {
		TargetPoint = Transform (*tmpo,meshdata[i]->Position());
		Vector origPoint = TargetPoint;

		perplines.push_back(TargetPoint);
		//find distance away from the line now :-)
		//find scale factor of end on start to get line.
		QVector tst = TargetPoint.Cast()-st;
		//Vector tst = TargetPoint;
		float k = tst.Dot (dir);
		TargetPoint = (tst - k*(dir)).Cast();
		/*
		cerr << origPoint << "-" << st << " = " << tst << " projected length " << k << " along direction " << dir << endl;
		cerr << "projected line " << st << " - " << st + k*dir << endl;
		cerr << "length of orthogonal projection " << TargetPoint.Magnitude() << ", " << "radius " << meshdata[i]->rSize() << endl;
		*/
		perplines.push_back(origPoint-TargetPoint);

		///      VSFileSystem::vs_fprintf (stderr, "i%f,j%f,k%f end %f,%f,%f>, k %f distance %f, rSize %f\n", st.i,st.j,st.k,end.i,end.j,end.k,k,TargetPoint.Dot(TargetPoint), meshdata[i]->rSize());

		if (TargetPoint.Dot (TargetPoint)<
			err*err+
			meshdata[i]->rSize()*meshdata[i]->rSize()+2*err*meshdata[i]->rSize()
		) {
			if (retval==0) {
				retval = k;
				adjretval=k;
				if (adjretval<0) {
					adjretval+=meshdata[i]->rSize();
					if (adjretval>0)
						adjretval=.001;
				}
			}
			else {
				if (retval>0&&k<retval&&k>-meshdata[i]->rSize()) {
					retval = k;
					adjretval=k;
					if (adjretval<0) {
						adjretval+=meshdata[i]->rSize();
						if (adjretval>0)
							adjretval=.001;
					}
				}
				if (retval<0&&k+meshdata[i]->rSize()>retval) {
					retval = k;
					adjretval=k+meshdata[i]->rSize();
					if (adjretval>0)
								 //THRESHOLD;
						adjretval=.001;
				}
			}
		}
	}
	const Unit * su;
	for(un_kiter ui = viewSubUnits();(su = *ui) != NULL;++ui){
		float tmp=su->querySphereClickList (st,dir,err);
		if (tmp==0) {
			continue;
		}
		if (retval==0) {
			retval = tmp;
		}
		else {
			if (adjretval>0&&tmp<adjretval) {
				retval = tmp;
				adjretval=tmp;
			}
			if (adjretval<0&&tmp>adjretval) {
				retval = tmp;
				adjretval=tmp;
			}
		}
	}

	return adjretval;
}


bool Unit::queryBoundingBox (const QVector &pnt, float err)
{
	int i;
	BoundingBox * bbox=NULL;
	for (i=0;i<nummesh();++i) {
		bbox = meshdata[i]->getBoundingBox();
		bbox->Transform (cumulative_transformation_matrix);
		if (bbox->Within(pnt,err)) {
			delete bbox;
			return true;
		}
		delete bbox;
	}
	Unit * su;
	for(un_iter ui = getSubUnits();(su = *ui) != NULL;++ui){
		if ((su)->queryBoundingBox (pnt,err)) {
			return true;
		}
	}
	return false;
}


int Unit::queryBoundingBox (const QVector &origin, const Vector &direction, float err)
{
	int i;
	int retval=0;
	BoundingBox * bbox=NULL;
	for (i=0;i<nummesh();++i) {
		bbox = meshdata[i]->getBoundingBox();
		bbox->Transform (cumulative_transformation_matrix);
		switch (bbox->Intersect(origin,direction.Cast(),err)) {
			case 1:delete bbox;
			return 1;
			case -1:delete bbox;
			retval =-1;
			break;
			case 0: delete bbox;
			break;
		}
	}
	Unit  * su;
	for(un_iter iter = getSubUnits();(su = *iter) != NULL;++iter){
		switch (su->queryBoundingBox (origin,direction,err)) {
			case 1:
				return 1;
			case -1:
				retval= -1;
				break;
			case 0:
				break;
		}
	}
	return retval;
}


/***********************************************************************************/
/**** UNIT_DOCK STUFF                                                            */
/***********************************************************************************/

bool Unit::EndRequestClearance(Unit *targ)
{
	std::vector <Unit *>::iterator lookcleared;
	if ((lookcleared = std::find (targ->image->clearedunits.begin(),targ->image->clearedunits.end(),this))!=targ->image->clearedunits.end()) {
		int whichdockport;
		targ->image->clearedunits.erase (lookcleared);
		return true;
	}
	else {
		return false;
	}
}


bool Unit::RequestClearance (Unit * dockingunit)
{
#if 0
	static float clearencetime=(XMLSupport::parse_float (vs_config->getVariable ("general","dockingtime","20")));
	EnqueueAIFirst (new ExecuteFor (new Orders::MatchVelocity (Vector(0,0,0),
		Vector(0,0,0),
		true,
		false,
		true),clearencetime));
#endif
	if (std::find (image->clearedunits.begin(),image->clearedunits.end(),dockingunit)==image->clearedunits.end())
		image->clearedunits.push_back (dockingunit);
	return true;
}


void Unit::FreeDockingPort (unsigned int i)
{
	if (image->dockedunits.size()==1) {
		docked&= (~DOCKING_UNITS);
	}
	unsigned int whichdock =image->dockedunits[i]->whichdock;
	image->dockingports[whichdock].used=false;
	image->dockedunits[i]->uc.SetUnit(NULL);
	delete image->dockedunits[i];
	image->dockedunits.erase (image->dockedunits.begin()+i);

}


static Transformation HoldPositionWithRespectTo (Transformation holder, const Transformation &changeold, const Transformation &changenew)
{
	Quaternion bak = holder.orientation;
	holder.position=holder.position-changeold.position;

	Quaternion invandrest =changeold.orientation.Conjugate();
	invandrest*=  changenew.orientation;
	holder.orientation*=invandrest;
	Matrix m;

	invandrest.to_matrix(m);
	holder.position = TransformNormal (m,holder.position);

	holder.position=holder.position+changenew.position;
	static bool changeddockedorient=(XMLSupport::parse_bool (vs_config->getVariable ("physics","change_docking_orientation","false")));
	if (!changeddockedorient) {
		holder.orientation = bak;
	}
	return holder;
}


extern void ExecuteDirector();
extern void SwitchUnits (Unit *,Unit*);

void Unit::PerformDockingOperations ()
{
	for (unsigned int i=0;i<image->dockedunits.size();++i) {
		Unit * un;
		if ((un=image->dockedunits[i]->uc.GetUnit())==NULL) {
			FreeDockingPort (i);
			i--;
			continue;
		}
		//Transformation t = un->prev_physical_state;
		float tmp;				 //short fix
		un->prev_physical_state=un->curr_physical_state;
		un->curr_physical_state =HoldPositionWithRespectTo (un->curr_physical_state,prev_physical_state,curr_physical_state);
		un->NetForce=Vector(0,0,0);
		un->NetLocalForce=Vector(0,0,0);
		un->NetTorque=Vector(0,0,0);
		un->NetLocalTorque=Vector (0,0,0);
		un->AngularVelocity=Vector (0,0,0);
		un->Velocity=Vector (0,0,0);
		if (un==_Universe->AccessCockpit()->GetParent()) {
			///CHOOSE NEW MISSION
			for (unsigned int i=0;i<image->clearedunits.size();++i) {
								 //this is a hack because we don't have an interface to say "I want to buy a ship"  this does it if you press shift-c in the base
				if (image->clearedunits[i]==un) {
					image->clearedunits.erase(image->clearedunits.begin()+i);
					un->UpgradeInterface(this);
				}
			}
		}
		//now we know the unit's still alive... what do we do to him *G*
		///force him in a box...err where he is
	}
}


std::set <Unit *> arrested_list_do_not_dereference;
bool Unit::RefillWarpEnergy()
{
	static bool WCfuelhack = XMLSupport::parse_bool(vs_config->getVariable("physics","fuel_equals_warp","false"));
	if (WCfuelhack)
		this->warpenergy = this->fuel;
	float tmp=this->maxwarpenergy;
	if (tmp<this->jump.energy)
		tmp=this->jump.energy;
	if (tmp>this->warpenergy) {
		this->warpenergy=tmp;

		if (WCfuelhack)
			this->fuel = this->warpenergy;

		return true;
	}
	return false;
}


void      UpdateMasterPartList(Unit*);
int Unit::ForceDock (Unit * utdw, int whichdockport)
{
	if (utdw->image->dockingports.size()<=whichdockport)
		return 0;

	utdw->image->dockingports[whichdockport].used=true;

	utdw->docked|=DOCKING_UNITS;
	utdw->image->dockedunits.push_back (new DockedUnits (this,whichdockport));
	// NETFIXME: Broken on server.
	if ((!Network)&&(!SERVER)&&utdw->image->dockingports[whichdockport].internal) {
		RemoveFromSystem();
		SetVisible(false);
		docked|=DOCKED_INSIDE;
	}
	else {
		docked|= DOCKED;
	}
	image->DockedTo.SetUnit (utdw);
	computer.set_speed=0;
	if (this==_Universe->AccessCockpit()->GetParent()) {
		this->RestoreGodliness();
		//_Universe->AccessCockpit()->RestoreGodliness();
	}
	UpdateMasterPartList(UniverseUtil::GetMasterPartList());
	int cockpit=UnitUtil::isPlayerStarship(this);

	static float MinimumCapacityToRefuelOnLand = XMLSupport::parse_float (vs_config->getVariable ("physics","MinimumWarpCapToRefuelDockeesAutomatically","0"));
	float capdata = utdw->WarpCapData();
	if ((capdata >= MinimumCapacityToRefuelOnLand) && (this->RefillWarpEnergy())) {
		if (cockpit>=0&&cockpit<_Universe->numPlayers()) {
			static float docking_fee = XMLSupport::parse_float (vs_config->getVariable("general","fuel_docking_fee","0"));
			_Universe->AccessCockpit(cockpit)->credits-=docking_fee;
		}
	}
	if ((capdata < MinimumCapacityToRefuelOnLand) && (this->faction == utdw->faction )) {
		if (utdw->WarpEnergyData() > this->WarpEnergyData() && utdw->WarpEnergyData() > this->jump.energy) {
			this->IncreaseWarpEnergy(false,this->jump.energy);
			utdw->DecreaseWarpEnergy(false,this->jump.energy);
		}
		if (utdw->WarpEnergyData() < this->WarpEnergyData() && this->WarpEnergyData() > utdw->jump.energy) {
			utdw->IncreaseWarpEnergy(false,utdw->jump.energy);
			this->DecreaseWarpEnergy(false,utdw->jump.energy);
		}

	}

	if (cockpit>=0&&cockpit<_Universe->numPlayers()) {
		static float docking_fee = XMLSupport::parse_float (vs_config->getVariable("general","docking_fee","0"));
		if (_Universe->AccessCockpit(cockpit)->credits>=docking_fee) {
			_Universe->AccessCockpit(cockpit)->credits-=docking_fee;
		}
		else if (_Universe->AccessCockpit(cockpit)->credits>=0) {
			_Universe->AccessCockpit(cockpit)->credits=0;
		}
	}

	std::set<Unit *>::iterator arrested=arrested_list_do_not_dereference.find(this);
	if (arrested!=arrested_list_do_not_dereference.end()) {
		arrested_list_do_not_dereference.erase (arrested);
		//do this for jail time
		for (unsigned int j=0;j<100000;++j) {
			for (unsigned int i=0;i<active_missions.size();++i) {

				ExecuteDirector();
			}
		}
	}
	return whichdockport+1;
}


int Unit::Dock (Unit * utdw)
{
	// Do only if non networking mode or if server (for both Network==NULL)
	if( Network==NULL) {
		if (docked&(DOCKED_INSIDE|DOCKED))
			return 0;
		std::vector <Unit *>::iterator lookcleared;
		if ((lookcleared = std::find (utdw->image->clearedunits.begin(),
		utdw->image->clearedunits.end(),this))!=utdw->image->clearedunits.end()) {
			int whichdockport;
			if ((whichdockport=utdw->CanDockWithMe(this))!=-1) {
				utdw->image->clearedunits.erase (lookcleared);
				return ForceDock(utdw,whichdockport);
			}
		}
		return 0;
	}
	else {
		// Send a dock request
		int playernum = _Universe->whichPlayerStarship( this);
		if( playernum>=0)
			Network[playernum].dockRequest( utdw->serial);
	}
	return 0;
}


inline bool insideDock (const DockingPorts &dock, const QVector & pos, float radius)
{
	if (dock.used)
		return false;
	double rad=dock.radius+radius;
	return (pos-dock.pos).MagnitudeSquared()<rad*rad;
	if (dock.internal) {
		if ((pos.i+radius<dock.max.i)&&
			(pos.j+radius<dock.max.j)&&
			(pos.k+radius<dock.max.k)&&
			(pos.i-radius>dock.min.i)&&
			(pos.j-radius>dock.min.j)&&
		(pos.k-radius>dock.min.k)) {
			return true;
		}
	}
	else {
		if ((pos-dock.pos).Magnitude()<dock.radius+radius&&
			(pos.i-radius<dock.max.i)&&
			(pos.j-radius<dock.max.j)&&
			(pos.k-radius<dock.max.k)&&
			(pos.i+radius>dock.min.i)&&
			(pos.j+radius>dock.min.j)&&
		(pos.k+radius>dock.min.k)) {
			return true;
		}
	}
	return false;
}


int Unit::CanDockWithMe(Unit * un, bool force)
{
	//  don't need to check relation: already cleared.
	for (unsigned int i=0;i<image->dockingports.size();++i) {
		if (un->image->dockingports.size()) {
			for (unsigned int j=0;j<un->image->dockingports.size();++j) {
				if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,Transform (un->cumulative_transformation_matrix,un->image->dockingports[j].pos.Cast())),un->image->dockingports[j].radius)) {
					if (((un->docked&(DOCKED_INSIDE|DOCKED))==0)&&(!(docked&DOCKED_INSIDE))) {
						return i;
					}
				}
			}
		}
		else {
			if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,un->Position()),un->rSize())) {
				return i;
			}
		}
	}
	if (force) {
		for (unsigned int i=0;i<image->dockingports.size();++i) {
			if (!image->dockingports[i].used) {
				return i;
			}
		}
	}
	//  }
	return -1;
}


bool Unit::IsCleared (const Unit * DockingUnit) const
{
	return (std::find (image->clearedunits.begin(),image->clearedunits.end(),DockingUnit)!=image->clearedunits.end());
}


bool Unit::hasPendingClearanceRequests() const
{
	return image && (image->clearedunits.size()>0);
}


bool Unit::isDocked (const Unit* d) const
{
	if (!d)
		return false;
	if (!(d->docked&(DOCKED_INSIDE|DOCKED))) {
		return false;
	}
	for (unsigned int i=0;i<image->dockedunits.size();++i) {
		Unit * un;
		if ((un=image->dockedunits[i]->uc.GetUnit())!=NULL) {
			if (un==d) {
				return true;
			}
		}
	}
	return false;
}


extern vector <int> switchunit;
extern vector <int> turretcontrol;

bool Unit::UnDock (Unit * utdw)
{
	unsigned int i=0;

	//   this->is_ejectdock = false;
	//   is_ejectdock = false;

	if (this->name=="return_to_cockpit") {
		//    this->is_ejectdock = true;
		if (this->faction == utdw->faction)
			this->owner = utdw;
		else
			this->owner = NULL;
	}

	UNIT_LOG(logvs::NOTICE, "Asking to undock");
	if( Network!=NULL && !SERVER && !_Universe->netLocked()) {
		UNIT_LOG(logvs::NOTICE,"Sending an undock notification");
		int playernum = _Universe->whichPlayerStarship( this);
		if( playernum>=0)
			Network[playernum].undockRequest( utdw->serial);
	}
	for (i=0;i<utdw->image->dockedunits.size();++i) {
		if (utdw->image->dockedunits[i]->uc.GetUnit()==this) {
			utdw->FreeDockingPort (i);
			i--;
			SetVisible(true);;
			docked&=(~(DOCKED_INSIDE|DOCKED));
			image->DockedTo.SetUnit (NULL);
			Velocity=utdw->Velocity;
			static float launch_speed=XMLSupport::parse_float(vs_config->getVariable("physics","launch_speed","-1"));
			static bool auto_turn_towards =XMLSupport::parse_bool(vs_config->getVariable ("physics","undock_turn_away","true"));
			if (Network||SERVER) {
				auto_turn_towards = false;
				launch_speed = -1;
			}

			if (launch_speed>0)
				computer.set_speed=launch_speed;
			if (auto_turn_towards) {
				for (int i=0;i<3;++i) {
					Vector methem(RealPosition(this)-RealPosition(utdw).Cast());
					methem.Normalize();
					Vector p,q,r;
					GetOrientation(p,q,r);
					p=methem.Cross(r);
					float theta = p.Magnitude();
					if (theta*theta>.00001) {
						p*= (asin (theta)/theta);
						Rotate(p);
						GetOrientation (p,q,r);
					}
					if (r.Dot(methem)<0) {
						Rotate (p*(PI/theta));
					}

				}
			}

			if (name=="return_to_cockpit" || this->name=="return_to_cockpit") {
				while (turretcontrol.size()<=_Universe->CurrentCockpit())
					turretcontrol.push_back(0);
				turretcontrol[_Universe->CurrentCockpit()]=1;
			}

			return true;
		}
	}
	return false;
}


/***********************************************************************************/
/**** UNIT_CUSTOMIZE STUFF                                                            */
/***********************************************************************************/
#define UPGRADEOK 1
#define NOTTHERE 0
#define CAUSESDOWNGRADE -1
#define LIMITEDBYTEMPLATE -2

const Unit* getUnitFromUpgradeName(const string& upgradeName, int myUnitFaction = 0);

typedef double (*adder) (double a, double b);
typedef double (*percenter) (double a, double b, double c);
typedef bool (*comparer) (double a, double b);

bool GreaterZero (double a, double b)
{
	return a>=0;
}


double AddUp (double a, double b)
{
	return a+b;
}


double MultUp (double a, double b)
{
	return a*b;
}


double GetsB (double a, double b)
{
	return b;
}


bool AGreaterB (double a, double b)
{
	return a>b;
}


double SubtractUp(double a, double b)
{
	return a-b;
}


double SubtractClamp (double a, double b)
{
	return (a-b<0)?0:a-b;
}


bool ALessB (double a, double b)
{
	return a<b;
}


double computePercent (double old, double upgrade, double newb)
{
	if (newb)
		return old/newb;
	else
		return 0;
}


double computeWorsePercent (double old,double upgrade, double isnew)
{
	if (old)
		return isnew/old;
	else
		return 1;
}


double computeAdderPercent (double a,double b, double c) {return 0;}
double computeMultPercent (double a,double b, double c) {return 0;}
double computeDowngradePercent (double old, double upgrade, double isnew)
{
	if (upgrade) {
		return (old-isnew)/upgrade;
	}
	else {
		return 0;
	}
}


static int UpgradeFloat (double &result,double tobeupgraded, double upgrador, double templatelimit, double (*myadd) (double,double), bool (*betterthan) (double a, double b), double nothing,  double completeminimum, double (*computepercentage) (double oldvar, double upgrador, double newvar), double & percentage, bool forcedowngrade, bool usetemplate, double at_least_this,bool (*atLeastthiscompare)( double a, double b)=AGreaterB, bool clamp=false, bool force_nothing=false)
{
								 //if upgrador is better than nothing
	if (upgrador!=nothing||force_nothing) {
		if (clamp) {
			if (tobeupgraded>upgrador)
				upgrador=tobeupgraded;
		}
		float newsum = (*myadd)(tobeupgraded,upgrador);
								 //if we're downgrading
		if (!force_nothing&&newsum < tobeupgraded&&at_least_this>=upgrador&&at_least_this>newsum&&at_least_this>=tobeupgraded) {
			return newsum==upgrador?CAUSESDOWNGRADE:NOTTHERE;
		}
		if (newsum!=tobeupgraded&&(((*betterthan)(newsum, tobeupgraded)||forcedowngrade))) {
			if (((*betterthan)(newsum,templatelimit)&&usetemplate)||newsum<completeminimum) {
				if (!forcedowngrade)
					return LIMITEDBYTEMPLATE;
				if (newsum<completeminimum)
					newsum=completeminimum;
				else
					newsum = templatelimit;
			}
			///we know we can replace result with newsum
			percentage = (*computepercentage)(tobeupgraded,upgrador,newsum);
			if ((*atLeastthiscompare)(at_least_this,newsum)&&(!force_nothing)) {
				if ((*atLeastthiscompare)(at_least_this,tobeupgraded)) {
								 //no shift
					newsum = tobeupgraded;
				}
				else {
								 //set it to its min
					newsum = at_least_this;
				}
			}
			result=newsum;
			return UPGRADEOK;
		}
		else {
			return CAUSESDOWNGRADE;
		}
	}
	else {
		return NOTTHERE;
	}
}


int UpgradeBoolval (int a, int upga, bool touchme, bool downgrade, int &numave,double &percentage, bool force_nothing)
{
	if (downgrade) {
		if (a&&upga) {
			if (touchme) (a=false);
			++numave;
			++percentage;
		}
	}
	else {
		if (!a&&upga) {
			if (touchme) a=true;
			++numave;
			++percentage;
		}
		else if (force_nothing && a  && !upga) {
			if (touchme) a=false;
			++numave;
			++percentage;
		}
	}
	return a;
}


void YoinkNewlines (char * input_buffer)
{
	for (int i=0;input_buffer[i]!='\0';++i) {
		if (input_buffer[i]=='\n'||input_buffer[i]=='\r') {
			input_buffer[i]='\0';
		}
	}
}


bool Quit (const char *input_buffer)
{
	if (strcasecmp (input_buffer,"exit")==0||
	strcasecmp (input_buffer,"quit")==0) {
		return true;
	}
	return false;
}


using std::string;
void Tokenize(const string& str,
vector<string>& tokens,
const string& delimiters = " ")
{
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}


std::string CheckBasicSizes (const std::string & tokens)
{
	if (tokens.find ("small")!=string::npos) {
		return "small";
	}
	if (tokens.find ("medium")!=string::npos) {
		return "medium";
	}
	if (tokens.find ("large")!=string::npos) {
		return "large";
	}
	if (tokens.find ("cargo")!=string::npos) {
		return "cargo";
	}
	if (tokens.find ("LR")!=string::npos||tokens.find ("massive")!=string::npos) {
		return "massive";
	}
	return "";
}


class VCString : public std::string
{
	public:
		VCString(){}
		VCString(const string & s): string(s){}
};
std::map<VCString,VCString> parseTurretSizes ()
{
	using namespace VSFileSystem;
	std::map<VCString,VCString> t;
	VSFile f;
	VSError err = f.OpenReadOnly( "units/subunits/size.txt", UnknownFile);
	if (err<=Ok) {
		int siz = f.Size();
		char * filedata= (char *)malloc (siz+1);
		filedata[siz]=0;
		while (f.ReadLine(filedata,siz)==Ok) {

			std::string x(filedata);
			string::size_type len= x.find (",");
			if (len!=std::string::npos) {
				std::string y = x.substr (len+1);
				x = x.substr(0,len);
				len = y.find(",");
				y = y.substr(0,len);
				sscanf (y.c_str(),"%s",filedata);
				y = filedata;
				VCString key (x);
				VCString value (y);
				t[key]=value;
			}
		}
		free(filedata);
		f.Close();
	}
	return t;
}


std::string getTurretSize (const std::string &size)
{
	static std::map<VCString,VCString> turretmap = parseTurretSizes();
	std::map<VCString,VCString>::iterator h= turretmap.find(size);
	if (h!=turretmap.end()) {
		return (*h).second;
	}
	vector <string> tokens;
	Tokenize (size,tokens,"_");
	for (unsigned int i=0;i<tokens.size();++i) {
		if (tokens[i].find ("turret")!=string::npos) {
			string temp = CheckBasicSizes (tokens[i]);
			if (!temp.empty()) {
				return temp;
			}
		}
		else {
			return tokens[i];
		}
	}
	return "capital";
}


bool Unit::UpgradeMounts (const Unit *up, int mountoffset, bool touchme, bool downgrade, int &numave, const Unit * templ, double &percentage){
	int j;
	int i;
	bool cancompletefully=true;
	for (i=0,j=mountoffset;i<up->GetNumMounts()&&i<GetNumMounts()/*i should be GetNumMounts(), s'ok*/;++i,++j) {
								 //only mess with this if the upgrador has active mounts
	if (up->mounts[i].status==Mount::ACTIVE||up->mounts[i].status==Mount::INACTIVE) {
								 //make sure since we're offsetting the starting we don't overrun the mounts
		bool isammo=(string::npos!=string(up->name).find("_ammo")); // is this ammo for a weapon rather than an actual weapon
		bool ismissiletype=(0!=(up->mounts[i].type->size&(weapon_info::CAPSHIPHEAVYMISSILE|weapon_info::SPECIALMISSILE|weapon_info::MEDIUMMISSILE|weapon_info::LIGHTMISSILE|weapon_info::HEAVYMISSILE|weapon_info::CAPSHIPLIGHTMISSILE)));

		int jmod=j%GetNumMounts();
		if (!downgrade) {		 //if we wish to add guns instead of remove

			if (up->mounts[i].type->weapon_name.find("_UPGRADE") == string::npos) { //check for capability increase rather than actual weapon upgrade
								 //only look at this mount if it can fit in the rack
				if (up->mounts[i].type->size==(up->mounts[i].type->size&mounts[jmod].size)) {
					if (up->mounts[i].type->weapon_name!=mounts[jmod].type->weapon_name || mounts[jmod].status==Mount::DESTROYED || mounts[jmod].status==Mount::UNCHOSEN) {
						// If missile, can upgrade directly, if other type of ammo, needs actual gun to be present.
						if(isammo&&!ismissiletype){
						   cancompletefully=false;
						}else {
							++numave;//ok now we can compute percentage of used parts
							Mount upmount(up->mounts[i]);
								if (templ) {
									if (templ->GetNumMounts()>jmod) {
										if (templ->mounts[jmod].volume!=-1) {
											if (upmount.ammo*upmount.type->volume>templ->mounts[jmod].volume) {
												upmount.ammo = (int)((templ->mounts[jmod].volume+1)/upmount.type->volume);
											}
										}
									}
								}
									 //compute here
							percentage+=mounts[jmod].Percentage(&upmount);
								 //if we wish to modify the mounts
							if (touchme) {
								 //switch this mount with the upgrador mount
								mounts[jmod].ReplaceMounts (this,&upmount);
							}
						}
					}
					else {
						if(isammo&&up->mounts[i].type->weapon_name==mounts[jmod].type->weapon_name){ // if is ammo and is same weapon type
							int tmpammo = mounts[jmod].ammo;
							if (mounts[jmod].ammo!=-1&&up->mounts[i].ammo!=-1) {
								tmpammo+=up->mounts[i].ammo;
								if (templ) {
									if (templ->GetNumMounts()>jmod) {
										if (templ->mounts[jmod].volume!=-1) {
											if (templ->mounts[jmod].volume<mounts[jmod].type->volume*tmpammo) {
												tmpammo=(int)floor(.125+((0+templ->mounts[jmod].volume)/mounts[jmod].type->volume));
											}
										}
									}
								}
								if (tmpammo*mounts[jmod].type->volume > mounts[jmod].volume) {
									tmpammo = (int)floor(.125+((0+mounts[jmod].volume)/mounts[jmod].type->volume));
								}
								if (tmpammo>mounts[jmod].ammo) {
									cancompletefully=true;
									if (touchme)
										mounts[jmod].ammo = tmpammo;
								}
								else {
									cancompletefully=false;
								}
							}
						}else {
							cancompletefully=false;
						}
					}
				}
				else {
								 //since we cannot fit the mount in the slot we cannot complete fully
					cancompletefully=false;
				}
			}
			else {
				unsigned int siz=0;
				siz = ~siz;
				if (templ) {
					if (templ->GetNumMounts()>jmod) {
						siz = templ->mounts[jmod].size;
					}
				}
				if (((siz&up->mounts[i].size)|mounts[jmod].size)!=mounts[jmod].size) {
					if (touchme) {
						mounts[jmod].size|=up->mounts[i].size;
					}
					++numave;
					++percentage;

				}
				else {
					cancompletefully=false;
				}
				//we need to |= the mount type
			}
		} // DOWNGRADE
		else {
			if (up->mounts[i].type->weapon_name!="MOUNT_UPGRADE") {
				bool found=false;//we haven't found a matching gun to remove

								 ///go through all guns
				for (unsigned int k=0;k<(unsigned int)GetNumMounts();++k) {
								 //we want to start with bias
					int jkmod = (jmod+k)%GetNumMounts();
					if(Mount::UNCHOSEN==mounts[jkmod].status){
						// can't sell weapon that's already been sold/removed
						continue;
					}
								 ///search for right mount to remove starting from j. this is the right name
					if (strcasecmp(mounts[jkmod].type->weapon_name.c_str(),up->mounts[i].type->weapon_name.c_str())==0) {
								 //we got one, but check if we're trying to sell non-existent ammo
						if(isammo&&mounts[jkmod].ammo<=0){
							// whether it's gun ammo or a missile, you can't remove ammo from an infinite source, and you can't remove ammo if there isn't any
							continue;
						}else {
							found=true;
						}
								 ///calculate scrap value (if damaged)
						percentage+=mounts[jkmod].Percentage(&up->mounts[i]);
								 //if we modify
						if (touchme) {
								 //if downgrading ammo based upgrade, checks for infinite ammo
							if (isammo&&up->mounts[i].ammo&&up->mounts[i].ammo!=-1&&mounts[jkmod].ammo!=-1) {
								 //remove upgrade-worth, else remove remaining
								mounts[jkmod].ammo-=(mounts[jkmod].ammo>=up->mounts[i].ammo)?up->mounts[i].ammo:mounts[jkmod].ammo;
								 //if none left
								if(!mounts[jkmod].ammo) {
									///deactivate weapon
									if(ismissiletype){
										mounts[jkmod].status=Mount::UNCHOSEN;
									}
								}
							}
							else {
								 ///deactivate weapon
								mounts[jkmod].status=Mount::UNCHOSEN;
								mounts[jkmod].ammo=-1; //remove all ammo
							}
						}
						break;
					}
				}

				if (!found)
								 //we did not find a matching weapon to remove
					cancompletefully=false;
			}
			else {
				bool found=false;
				static   bool downmount =XMLSupport::parse_bool (vs_config->getVariable ("physics","can_downgrade_mount_upgrades","false"));
				if (downmount ) {

								 ///go through all guns
					for (unsigned int k=0;k<(unsigned int)GetNumMounts();++k) {
								 //we want to start with bias
						int jkmod = (jmod+k)%GetNumMounts();
						if ((up->mounts[i].size&mounts[jkmod].size)==(up->mounts[i].size)) {
							if (touchme) {
								mounts[jkmod].size&=(~up->mounts[i].size);
							}
							++percentage;
							++numave;
							found=true;
						}
					}
				}
				if (!found)
					cancompletefully=false;
			}
		}
	}
}


if (i<up->GetNumMounts()) {
	cancompletefully=false;		 //if we didn't reach the last mount that we wished to upgrade, we did not fully complete
}


return cancompletefully;
}


Unit * CreateGenericTurret (const std::string & tur,int faction)
{
	return new Unit (tur.c_str(),true,faction,"",0,0);
}


bool Unit::UpgradeSubUnits (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)
{
	return UpgradeSubUnitsWithFactory( up, subunitoffset, touchme, downgrade, numave, percentage,&CreateGenericTurret);
}


bool Unit::UpgradeSubUnitsWithFactory (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage, Unit * (*createupgradesubunit) (const std::string & s, int faction))
{
	bool cancompletefully=true;
	int j;
	std::string turSize;
	un_iter ui;
	bool found=false;
	for (j=0,ui=getSubUnits();(*ui)!=NULL&&j<subunitoffset;++ui,++j) {
	}							 ///set the turrets to the offset
	un_kiter upturrets;
	Unit * giveAway;

	giveAway=*ui;
	if (giveAway==NULL) {
		return true;
	}
	bool hasAnyTurrets=false;
	turSize = getTurretSize (giveAway->name);
								 //begin goign through other unit's turrets
	for (upturrets=up->viewSubUnits();((*upturrets)!=NULL)&&((*ui)!=NULL); ++ui,++upturrets) {
		hasAnyTurrets = true;
		const Unit *addtome;

		addtome=*upturrets;		 //set pointers

		bool foundthis=false;
								 //if the new turret has any size at all
		if (turSize == getTurretSize (addtome->name)&&addtome->rSize()&&(turSize+"_blank"!=addtome->name.get())) {
			if (!downgrade||addtome->name==giveAway->name) {
				found=true;
				foundthis=true;
				++numave;		 //add it
								 //add up percentage equal to ratio of sizes
				percentage+=(giveAway->rSize()/addtome->rSize());
			}
		}
		if (foundthis) {
			if (touchme) {		 //if we wish to modify,
				Transformation addToMeCur = giveAway->curr_physical_state;
				Transformation addToMePrev = giveAway->prev_physical_state;
				//	upturrets.postinsert (giveAway);//add it to the second unit
				giveAway->Kill();//risky??
				ui.remove();	 //remove the turret from the first unit

								 //if we are upgrading swap them
				if (!downgrade) {
					Unit * addToMeNew = (*createupgradesubunit)(addtome->name,addtome->faction);
					addToMeNew->curr_physical_state = addToMeCur;
					addToMeNew->SetFaction(faction);
					addToMeNew->prev_physical_state = addToMePrev;
								 //add unit to your ship
					ui.preinsert(addToMeNew);
					//	  upturrets.remove();//remove unit from being a turret on other ship
								 //set recursive owner
					addToMeNew->SetRecursiveOwner(this);
				}
				else {
					Unit * un;	 //make garbage unit
					// NOT 100% SURE A GENERIC UNIT CAN FIT (WAS GAME UNIT CREATION)
								 //give a default do-nothing unit
					ui.preinsert (un=UnitFactory::createUnit("upgrading_dummy_unit",true,faction));
					//WHAT?!?!?!?! 102302	  ui.preinsert (un=new Unit(0));//give a default do-nothing unit
					un->SetFaction(faction);
					un->curr_physical_state = addToMeCur;
					un->prev_physical_state = addToMePrev;
					un->limits.yaw=0;
					un->limits.pitch=0;
					un->limits.roll=0;
					un->limits.lateral = un->limits.retro = un->limits.forward = un->limits.afterburn=0.0;

					un->name = turSize+"_blank";
					if (un->image->unitwriter!=NULL) {
						un->image->unitwriter->setName (un->name);
					}
					un->SetRecursiveOwner(this);
				}
			}
		}
	}

	if (!found) {
		return !hasAnyTurrets;
	}
	if ((*upturrets)!=NULL)
		return false;
	return cancompletefully;
}

static void GCCBugCheckFloat(float *f, int offset) {
      if (f[offset]>1) {

        f[offset]=1;//keep it real
      }

}

bool Unit::canUpgrade (const Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, const Unit * templ, bool force_change_on_nothing, bool gen_downgrade_list)
{
	return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,false,false,additive,force,percentage,this, force_change_on_nothing,gen_downgrade_list);
}


bool Unit::Upgrade (const Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, const Unit * templ, bool force_change_on_nothing, bool gen_downgrade_list)
{
	return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,true,false,additive,force,percentage,this, force_change_on_nothing,gen_downgrade_list);
}


bool Unit::canDowngrade (const Unit *downgradeor, int mountoffset, int subunitoffset, double & percentage, const Unit * downgradelimit, bool gen_downgrade_list)
{
	return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,false,true,false,true,percentage,downgradelimit,false,gen_downgrade_list);
}


bool Unit::Downgrade (const Unit * downgradeor, int mountoffset, int subunitoffset,  double & percentage,const Unit * downgradelimit, bool gen_downgrade_list)
{
	return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,true,true,false,true,percentage,downgradelimit,false,gen_downgrade_list);
}


double ComputeMinDowngradePercent()
{
	static float MyPercentMin = XMLSupport::parse_float (vs_config->getVariable("general","remove_downgrades_less_than_percent",".9"));
	return MyPercentMin;
}


class DoubleName
{
	public:
		string s;
		double d;
		DoubleName (const string & ss,double dd) {
			d =dd;s=ss;
		}
		DoubleName () {
			d = -FLT_MAX;
		}
};

extern int GetModeFromName (const char *);

extern Unit * CreateGameTurret (const std::string & tur,int faction);

extern char * GetUnitDir (const char *);
double Unit::Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts)
{
#if 0
	if (shield.number==2) {
		UNIT_LOG(logvs::VERBOSE, "shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
	}
	else {
		UNIT_LOG(logvs::VERBOSE, "shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);

	}
#endif
	int upgradefac=FactionUtil::GetUpgradeFaction();
	const Unit * up = UnitConstCache::getCachedConst (StringIntKey(file,upgradefac));
	if (!up) {
		up = UnitConstCache::setCachedConst (StringIntKey (file,upgradefac),
			UnitFactory::createUnit (file.c_str(),true,upgradefac));
	}
	char * unitdir  = GetUnitDir(this->name.get().c_str());
	string templnam = string(unitdir)+".template";
	const Unit * templ = UnitConstCache::getCachedConst (StringIntKey(templnam,this->faction));
	if (templ==NULL) {
		templ = UnitConstCache::setCachedConst (StringIntKey(templnam,this->faction),UnitFactory::createUnit (templnam.c_str(),true,this->faction));
	}
	free (unitdir);
	double percentage=0;
	if (up->name!="LOAD_FAILED") {

		for  (int i=0;percentage==0;++i ) {
			if (!this->Unit::Upgrade(up,mountoffset+i, subunitoffset+i, GetModeFromName(file.c_str()),force, percentage,(templ->name=="LOAD_FAILED")?NULL:templ),false,false) {
				percentage=0;
			}
			if (!loop_through_mounts||(i+1>=this->GetNumMounts ())||percentage>0) {
				break;
			}
		}
	}
#if 0
	if (shield.number==2) {
		UNIT_LOG(logvs::VERBOSE, "shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
	}
	else {
		UNIT_LOG(logvs::VERBOSE, "shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);

	}
#endif

	return percentage;
}


vsUMap<int, DoubleName> downgrademap;
int curdowngrademapoffset = 5*sizeof (Unit);
bool AddToDowngradeMap (const std::string & name,double value, int unitoffset,vsUMap<int,DoubleName> &tempdowngrademap)
{
	using vsUMap;
	vsUMap<int,DoubleName>::iterator i =downgrademap.find (unitoffset);
	if (i!=downgrademap.end()) {
		if ((*i).second.d<=value) {
			tempdowngrademap[unitoffset] = DoubleName (name,value);
			return true;
		}
	}
	else {
		tempdowngrademap[unitoffset] = DoubleName (name,value);
		return true;
	}
	return false;
}


void ClearDowngradeMap ()
{
	downgrademap.clear();
}


std::set<std::string> GetListOfDowngrades ()
{
	using vsUMap;
	vsUMap<int,DoubleName>::iterator i =downgrademap.begin();
	std::set<std::string> retval;
	for (;i!=downgrademap.end();++i) {
		retval.insert ((*i).second.s);
	}

	//  return std::vector<std::string> (retval.begin(),retval.end());
	return retval;
}


typedef vsUMap<const char*,bool> UnitHasRecursiveData;
typedef vsUMap<std::string,UnitHasRecursiveData> FactionHasRecursiveData;
typedef std::vector<FactionHasRecursiveData> HasRecursiveData;

static HasRecursiveData has_recursive_data;
static std::string upgradeString("Upgrades");

static bool cell_has_recursive_data(const string &name, int fac, const char*key)
{
	if (fac<has_recursive_data.size()) {
		FactionHasRecursiveData::const_iterator iter=has_recursive_data[fac].find(name);
		if (iter != has_recursive_data[fac].end()) {
			UnitHasRecursiveData::const_iterator iter2 = iter->second.find(key);
			if (iter2 != iter->second.end())
				return iter2->second;
		}
	}
	else {
		has_recursive_data.resize(fac+1);
	}

	bool retval=false;
	string faction = FactionUtil::GetFactionName(fac);
	string lkey = key;
	string::size_type lkstart = 0;
	string::size_type lkend = lkey.find('|');
								 // Big short circuit - avoids recursion
	while (!retval && (lkstart != string::npos)) {
		string skey = lkey.substr(lkstart,(lkend==string::npos)?string::npos:lkend-lkstart);
		string lus = UniverseUtil::LookupUnitStat(name,faction,skey);

		retval=(lus.length()!=0);

		lkstart = (lkend != string::npos) ? lkend+1 : string::npos;
		lkend = lkey.find('|',lkstart);
	}
	if (!retval) {				 // Big short circuit - avoids recursion
		string::size_type when;
		string upgrades=UniverseUtil::LookupUnitStat(name,faction,upgradeString);
		string::size_type ofs=0;
		while (!retval&&((when=upgrades.find('{',ofs))!=string::npos)) {
			string::size_type where = upgrades.find('}',when+1);
			string upgrade = upgrades.substr(when+1,((where!=string::npos)?where-when-1:string::npos));
			retval=cell_has_recursive_data(upgrade,fac,key);
			ofs = where+1;
		}
	}
	has_recursive_data[fac][name][key]=retval;
	return retval;
}


bool Unit::UpAndDownGrade (const Unit * up, const Unit * templ, int mountoffset, int subunitoffset, bool touchme, bool downgrade, int additive, bool forcetransaction, double &percentage, const Unit * downgradelimit,bool force_change_on_nothing,bool gen_downgrade_list)
{
	percentage=0;
	if (Network && !_Universe->netLocked() && touchme) {
		int playernum = _Universe->whichPlayerStarship( this );
		if (playernum>=0) {
			ObjSerial buySerial = downgrade?0:serial,
				sellSerial = downgrade?serial:0;
			Network[playernum].cargoRequest( buySerial, sellSerial,
				up->name, 0, mountoffset, subunitoffset);
		}
		return false;
	}
	if (SERVER && touchme && !_Universe->netLocked() && getStarSystem() ) {
		// Server may not go here if it wants to send an atomic upgrade message.
		ObjSerial buySerial = downgrade?0:serial,
			sellSerial = downgrade?serial:0;
		VSServer->BroadcastCargoUpgrade( serial, buySerial, sellSerial,
										 up->name, 0,0,0,false,0,
										 mountoffset, subunitoffset, getStarSystem()->GetZone());
	}
	static bool csv_cell_null_check=XMLSupport::parse_bool(vs_config->getVariable("data","empty_cell_check","true"));
	int numave=0;
	bool cancompletefully=true;
	bool can_be_redeemed=false;
	bool needs_redemption=false;
	if (mountoffset>=0)
		cancompletefully=UpgradeMounts(up,mountoffset,touchme,downgrade,numave,templ,percentage);
	bool cancompletefully1=true;
	if (subunitoffset>=0)
		cancompletefully1=UpgradeSubUnits(up,subunitoffset,touchme,downgrade,numave,percentage);
	cancompletefully=cancompletefully&&cancompletefully1;
	adder Adder;
	comparer Comparer;
	percenter Percenter;
	vsUMap<int, DoubleName> tempdownmap;
	if (cancompletefully&&cancompletefully1&&downgrade) {
		if (percentage>0)
			AddToDowngradeMap (up->name,1,curdowngrademapoffset++,tempdownmap);
	}

	float tmax_speed = up->computer.max_combat_speed;
	float tmax_ab_speed = up->computer.max_combat_ab_speed;
	float tmax_yaw_right = up->computer.max_yaw_right;
	float tmax_yaw_left = up->computer.max_yaw_left;
	float tmax_pitch_up = up->computer.max_pitch_up;
	float tmax_pitch_down = up->computer.max_pitch_down;
	float tmax_roll_right = up->computer.max_roll_right;
	float tmax_roll_left = up->computer.max_roll_left;
	float tlimits_yaw=up->limits.yaw;
	float tlimits_roll=up->limits.roll;
	float tlimits_pitch=up->limits.pitch;
	float tlimits_lateral = up->limits.lateral;
	float tlimits_vertical = up->limits.vertical;
	float tlimits_forward = up->limits.forward;
	float tlimits_retro = up->limits.retro;
	float tlimits_afterburn = up->limits.afterburn;
	if (downgrade) {
		Adder=&SubtractUp;
		Percenter=&computeDowngradePercent;
		Comparer = &GreaterZero;
	}
	else {
		if (additive==1) {
			Adder=&AddUp;
			Percenter=&computeAdderPercent;
		}
		else if (additive==2) {
			Adder=&MultUp;
			Percenter=&computeMultPercent;
			tmax_speed = XMLSupport::parse_float (speedStarHandler (XMLType (&tmax_speed),NULL));
			tmax_ab_speed = XMLSupport::parse_float (speedStarHandler (XMLType (&tmax_ab_speed),NULL));
			tmax_yaw_right = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_yaw_right),NULL));
			tmax_yaw_left = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_yaw_left),NULL));
			tmax_pitch_up = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_pitch_up),NULL));
			tmax_pitch_down = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_pitch_down),NULL));
			tmax_roll_right = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_roll_right),NULL));
			tmax_roll_left = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_roll_left),NULL));

			tlimits_yaw = XMLSupport::parse_float (angleStarHandler (XMLType (&tlimits_yaw),NULL));
			tlimits_pitch = XMLSupport::parse_float (angleStarHandler (XMLType (&tlimits_pitch),NULL));
			tlimits_roll = XMLSupport::parse_float (angleStarHandler (XMLType (&tlimits_roll),NULL));

			tlimits_forward = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_forward),NULL));
			tlimits_retro = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_retro),NULL));
			tlimits_lateral = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_lateral),NULL));
			tlimits_vertical = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_vertical),NULL));
			tlimits_afterburn = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_afterburn),NULL));
		}
		else {
			Adder=&GetsB;
			Percenter=&computePercent;
		}
		Comparer=AGreaterB;
	}
	double resultdoub;
	int retval;
	double temppercent;

	static Unit * blankship=NULL;
	static bool initblankship=false;
	if (!initblankship) {
		blankship=this;
		initblankship=true;
		blankship= UnitFactory::createServerSideUnit ("upgrading_dummy_unit",true,FactionUtil::GetUpgradeFaction());
	}

#define STDUPGRADE_SPECIFY_DEFAULTS(my,oth,temp,noth,dgradelimer,dgradelimerdefault,clamp,value_to_lookat) \
		retval=(UpgradeFloat(resultdoub,my,oth,(templ!=NULL)?temp:0,Adder,Comparer,noth,noth,Percenter, temppercent,forcetransaction,templ!=NULL,(downgradelimit!=NULL)?dgradelimer:dgradelimerdefault,AGreaterB,clamp,force_change_on_nothing)); \
		if (retval==UPGRADEOK) \
		{ \
			if (touchme) { my=resultdoub; } \
			percentage+=temppercent; \
			++numave; \
			can_be_redeemed=true; \
			if (gen_downgrade_list) \
			AddToDowngradeMap (up->name,oth,((char *)&value_to_lookat)-(char *)this,tempdownmap); \
		} \
		else if (retval!=NOTTHERE) \
		{ \
			if (retval==CAUSESDOWNGRADE) \
			needs_redemption=true; \
			else \
			cancompletefully=false; \
		}
#define STDUPGRADE(my,oth,temp,noth) { STDUPGRADE_SPECIFY_DEFAULTS (my,oth,temp,noth,downgradelimit->my,blankship->my,false,this->my); }
#define STDUPGRADECLAMP(my,oth,temp,noth) { STDUPGRADE_SPECIFY_DEFAULTS (my,oth,temp,noth,downgradelimit->my,blankship->my,!force_change_on_nothing,this->my); }

							// set up vars for "LookupUnitStat" to check for empty cells
							string upgrade_name=up->name;

	// Check warp stuff
	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Spec_Interdiction|Warp_Min_Multiplier|Warp_Max_Multiplier")) {
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Spec_Interdiction")) {
			bool neg = specInterdiction<0;
			bool upneg=up->specInterdiction<0;
			bool interdictionUnits=specInterdiction>0;
			specInterdiction=fabs(specInterdiction);
			STDUPGRADE(specInterdiction,fabs(up->specInterdiction),upneg?fabs(templ->specInterdiction):templ->specInterdiction,0);
			if (upneg) {
				specInterdiction=-specInterdiction;
			}
			if (interdictionUnits!=(specInterdiction>0)) {
				StarSystem *ss = activeStarSystem;
				if (_Universe->getNumActiveStarSystem()&&!ss) ss=_Universe->activeStarSystem();
				if (ss) {
					Unit * un;
					for (un_iter i = ss->gravitationalUnits().createIterator();(un=*i) != NULL;++i) {
						if (un==this){
							i.remove();
							// NOTE: I think we can only be in here once
							break;
						}
					}
					if (!interdictionUnits) {
						// will interdict
						ss->gravitationalUnits().prepend(this);
					}
				}
			}
		}

		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Warp_Min_Multiplier"))
			STDUPGRADE(graphicOptions.MinWarpMultiplier,up->graphicOptions.MinWarpMultiplier,templ->graphicOptions.MinWarpMultiplier,1);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Warp_Max_Multiplier"))
			STDUPGRADE(graphicOptions.MaxWarpMultiplier,up->graphicOptions.MaxWarpMultiplier,templ->graphicOptions.MaxWarpMultiplier,1);
	}

	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Armor_Front_Top_Right")) {
		STDUPGRADE(armor.frontrighttop,up->armor.frontrighttop,templ->armor.frontrighttop,0);
		STDUPGRADE(armor.backrighttop,up->armor.backrighttop,templ->armor.backrighttop,0);
		STDUPGRADE(armor.frontlefttop,up->armor.frontlefttop,templ->armor.frontlefttop,0);
		STDUPGRADE(armor.backlefttop,up->armor.backlefttop,templ->armor.backlefttop,0);
		STDUPGRADE(armor.frontrightbottom,up->armor.frontrightbottom,templ->armor.frontrightbottom,0);
		STDUPGRADE(armor.backrightbottom,up->armor.backrightbottom,templ->armor.backrightbottom,0);
		STDUPGRADE(armor.frontleftbottom,up->armor.frontleftbottom,templ->armor.frontleftbottom,0);
		STDUPGRADE(armor.backleftbottom,up->armor.backleftbottom,templ->armor.backleftbottom,0);
	}

	float tmp=shield.recharge;
	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Shield_Recharge"))
		STDUPGRADE(shield.recharge,up->shield.recharge,templ->shield.recharge,0);
	bool upgradedrecharge=(tmp!=shield.recharge);

	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Hull"))
		STDUPGRADE(hull,up->hull,templ->hull,0);
	if ((maxhull<hull) && (hull!=0))
		maxhull=hull;

	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Reactor_Recharge"))
		STDUPGRADE(recharge,up->recharge,templ->recharge,0);

	static bool unittable=XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));

	// Uncommon fields (capacities... rates... etc...)
	image->ecm = abs(image->ecm);
	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Heat_Sink_Rating|Repair_Droid|Hold_Volume|Upgrade_Storage_Volume|Equipment_Space|Hidden_Cargo_Volume|ECM_Rating|Primary_Capacitor|Warp_Capacitor")) {
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Heat_Sink_Rating"))
			STDUPGRADE(HeatSink,up->HeatSink,templ->HeatSink,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Repair_Droid"))
			STDUPGRADE(image->repair_droid,up->image->repair_droid,templ->image->repair_droid,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Hold_Volume"))
			STDUPGRADE(image->CargoVolume,up->image->CargoVolume,templ->image->CargoVolume,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Upgrade_Storage_Volume"))
			STDUPGRADE(image->UpgradeVolume,up->image->UpgradeVolume,templ->image->UpgradeVolume,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Equipment_Space"))
			STDUPGRADE(image->equipment_volume,up->image->equipment_volume,templ->image->equipment_volume,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Hidden_Cargo_Volume"))
			STDUPGRADE(image->HiddenCargoVolume,up->image->HiddenCargoVolume,templ->image->HiddenCargoVolume,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"ECM_Rating"))
			STDUPGRADE(image->ecm,abs(up->image->ecm),abs(templ->image->ecm),0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Primary_Capacitor"))
			STDUPGRADE(maxenergy,up->maxenergy,templ->maxenergy,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Warp_Capacitor"))
			STDUPGRADE(maxwarpenergy,up->maxwarpenergy,templ->maxwarpenergy,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Warp_Drive_Rating"))
			STDUPGRADE(jump.warpDriveRating,up->jump.warpDriveRating,templ->jump.warpDriveRating,0);
	}

	// Maneuvering stuff
	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Maneuver_Yaw|Maneuver_Pitch|Maneuver_Roll|Left_Accel|Top_Accel|Retro_Accel|Forward_Accel|Afterburner_Accel|Default_Speed_Governor|Afterburner_Speed_Governor|Yaw_Governor|Pitch_Governor|Roll_Speed_Governor")) {
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Maneuver_Yaw"))
			STDUPGRADE(limits.yaw,tlimits_yaw,templ->limits.yaw,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Maneuver_Pitch"))
			STDUPGRADE(limits.pitch,tlimits_pitch,templ->limits.pitch,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Maneuver_Roll"))
			STDUPGRADE(limits.roll,tlimits_roll,templ->limits.roll,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Left_Accel"))
			STDUPGRADE(limits.lateral,tlimits_lateral,templ->limits.lateral,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Top_Accel"))
			STDUPGRADE(limits.vertical,tlimits_vertical,templ->limits.vertical,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Retro_Accel"))
			STDUPGRADE(limits.retro,tlimits_retro,templ->limits.retro,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Forward_Accel"))
			STDUPGRADE(limits.forward,tlimits_forward,templ->limits.forward,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Afterburner_Accel"))
			STDUPGRADE(limits.afterburn,tlimits_afterburn,templ->limits.afterburn,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Fuel_Capacity"))
			STDUPGRADE(fuel,up->fuel,templ->fuel,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Default_Speed_Governor"))
			STDUPGRADE(computer.max_combat_speed,tmax_speed,templ->computer.max_combat_speed,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Afterburner_Speed_Governor"))
			STDUPGRADE(computer.max_combat_ab_speed,tmax_ab_speed,templ->computer.max_combat_ab_speed,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Yaw_Governor")) {
			STDUPGRADE(computer.max_yaw_right,tmax_yaw_right,templ->computer.max_yaw_right,0);
			STDUPGRADE(computer.max_yaw_left,tmax_yaw_left,templ->computer.max_yaw_left,0);
		}
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Pitch_Governor")) {
			STDUPGRADE(computer.max_pitch_down,tmax_pitch_down,templ->computer.max_pitch_down,0);
			STDUPGRADE(computer.max_pitch_up,tmax_pitch_up,templ->computer.max_pitch_up,0);
		}
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Roll_Speed_Governor")) {
			STDUPGRADE(computer.max_roll_left,tmax_roll_left,templ->computer.max_roll_left,0);
			STDUPGRADE(computer.max_roll_right,tmax_roll_right,templ->computer.max_roll_right,0);
		}
	}

	//FIXME - do cell lookup later here
	static bool UpgradeCockpitDamage = XMLSupport::parse_bool (vs_config->getVariable("physics","upgrade_cockpit_damage","false"));
	if (UpgradeCockpitDamage) {
		STDUPGRADE(image->fireControlFunctionality,up->image->fireControlFunctionality,templ->image->fireControlFunctionality,(unittable?0:1));
		STDUPGRADE(image->fireControlFunctionalityMax,up->image->fireControlFunctionalityMax,templ->image->fireControlFunctionalityMax,(unittable?0:1));
		STDUPGRADE(image->SPECDriveFunctionality,up->image->SPECDriveFunctionality,templ->image->SPECDriveFunctionality,(unittable?0:1));
		STDUPGRADE(image->SPECDriveFunctionalityMax,up->image->SPECDriveFunctionalityMax,templ->image->SPECDriveFunctionalityMax,(unittable?0:1));
		STDUPGRADE(image->CommFunctionality,up->image->CommFunctionality,templ->image->CommFunctionality,(unittable?0:1));
		STDUPGRADE(image->CommFunctionalityMax,up->image->CommFunctionalityMax,templ->image->CommFunctionalityMax,(unittable?0:1));
		STDUPGRADE(image->LifeSupportFunctionality,up->image->LifeSupportFunctionality,templ->image->LifeSupportFunctionality,(unittable?0:1));
		STDUPGRADE(image->LifeSupportFunctionalityMax,up->image->LifeSupportFunctionalityMax,templ->image->LifeSupportFunctionalityMax,(unittable?0:1));
		unsigned int upgrmax=(UnitImages::NUMGAUGES+1+MAXVDUS)*2;
		for (unsigned int upgr=0;upgr<upgrmax;upgr++) {
			STDUPGRADE(image->cockpit_damage[upgr],up->image->cockpit_damage[upgr],templ->image->cockpit_damage[upgr],(unittable?0:1));
		}
		for (unsigned int upgr=0;upgr<upgrmax;++upgr) {
			GCCBugCheckFloat(image->cockpit_damage,upgr);
		}
	}

	bool upgradedshield=false;
	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Shield_Front_Top_Right")) {
		if (shield.number==up->shield.number) {
			float a,b,c,d;
			float aa,bb,cc,dd;
			switch (shield.number) {
				case 2:
					a=shield.shield2fb.frontmax;
					b=shield.shield2fb.backmax;
					STDUPGRADE(shield.shield2fb.frontmax,up->shield.shield2fb.frontmax,templ->shield.shield2fb.frontmax,0);
					STDUPGRADE(shield.shield2fb.backmax,up->shield.shield2fb.backmax,templ->shield.shield2fb.backmax,0);
					if (shield.shield2fb.frontmax!=a) shield.shield2fb.front=shield.shield2fb.frontmax;
					if (shield.shield2fb.backmax!=b) shield.shield2fb.back=shield.shield2fb.backmax;
					break;
				case 4:
					a=shield.shield4fbrl.frontmax;
					b=shield.shield4fbrl.backmax;
					c=shield.shield4fbrl.leftmax;
					d=shield.shield4fbrl.rightmax;
					STDUPGRADE(shield.shield4fbrl.frontmax,up->shield.shield4fbrl.frontmax,templ->shield.shield4fbrl.frontmax,0);
					STDUPGRADE(shield.shield4fbrl.backmax,up->shield.shield4fbrl.backmax,templ->shield.shield4fbrl.backmax,0);
					STDUPGRADE(shield.shield4fbrl.leftmax,up->shield.shield4fbrl.leftmax,templ->shield.shield4fbrl.leftmax,0);
					STDUPGRADE(shield.shield4fbrl.rightmax,up->shield.shield4fbrl.rightmax,templ->shield.shield4fbrl.rightmax,0);
					if (a!=shield.shield4fbrl.frontmax) shield.shield4fbrl.front=shield.shield4fbrl.frontmax;
					if (b!=shield.shield4fbrl.backmax)shield.shield4fbrl.back=shield.shield4fbrl.backmax;
					if (c!=shield.shield4fbrl.leftmax)shield.shield4fbrl.left=shield.shield4fbrl.leftmax;
					if (d!=shield.shield4fbrl.rightmax)shield.shield4fbrl.right=shield.shield4fbrl.rightmax;
					break;
				case 8:
					a=shield.shield8.frontrighttopmax;
					b=shield.shield8.backrighttopmax;
					c=shield.shield8.frontlefttopmax;
					d=shield.shield8.backlefttopmax;
					aa=shield.shield8.frontrightbottommax;
					bb=shield.shield8.backrightbottommax;
					cc=shield.shield8.frontleftbottommax;
					dd=shield.shield8.backleftbottommax;

					STDUPGRADE(shield.shield8.frontrighttopmax,up->shield.shield8.frontrighttopmax,templ->shield.shield8.frontrighttopmax,0);
					STDUPGRADE(shield.shield8.backrighttopmax,up->shield.shield8.backrighttopmax,templ->shield.shield8.backrighttopmax,0);
					STDUPGRADE(shield.shield8.frontlefttopmax,up->shield.shield8.frontlefttopmax,templ->shield.shield8.frontlefttopmax,0);
					STDUPGRADE(shield.shield8.backlefttopmax,up->shield.shield8.backlefttopmax,templ->shield.shield8.backlefttopmax,0);
					STDUPGRADE(shield.shield8.frontrightbottommax,up->shield.shield8.frontrightbottommax,templ->shield.shield8.frontrightbottommax,0);
					STDUPGRADE(shield.shield8.backrightbottommax,up->shield.shield8.backrightbottommax,templ->shield.shield8.backrightbottommax,0);
					STDUPGRADE(shield.shield8.frontleftbottommax,up->shield.shield8.frontleftbottommax,templ->shield.shield8.frontleftbottommax,0);
					STDUPGRADE(shield.shield8.backleftbottommax,up->shield.shield8.backleftbottommax,templ->shield.shield8.backleftbottommax,0);

					if (a!=shield.shield8.frontrighttopmax)shield.shield8.frontrighttop=shield.shield8.frontrighttopmax;
					if (b!=shield.shield8.backrighttopmax)shield.shield8.backrighttop=shield.shield8.backrighttopmax;
					if (c!=shield.shield8.frontlefttopmax)shield.shield8.frontlefttop=shield.shield8.frontlefttopmax;
					if (d!=shield.shield8.backlefttopmax)shield.shield8.backlefttop=shield.shield8.backlefttopmax;
					if (aa!=shield.shield8.frontrightbottommax)shield.shield8.frontrightbottom=shield.shield8.frontrightbottommax;
					if (bb!=shield.shield8.backrightbottommax)shield.shield8.backrightbottom=shield.shield8.backrightbottommax;
					if (cc!=shield.shield8.frontleftbottommax)shield.shield8.frontleftbottom=shield.shield8.frontleftbottommax;
					if (dd!=shield.shield8.backleftbottommax)shield.shield8.backleftbottom=shield.shield8.backleftbottommax;

					break;
			}
			if (touchme&&retval==UPGRADEOK) {
				upgradedshield=true;
			}
		}
		else {
			if (up->FShieldData()>0||up->RShieldData()>0|| up->LShieldData()>0||up->BShieldData()>0) {
				cancompletefully=false;
			}
		}
	}
	if (upgradedshield||upgradedrecharge) {
		if (up->shield.efficiency) {
			shield.efficiency=up->shield.efficiency;
			if (templ) {
				if (shield.efficiency>templ->shield.efficiency) {
					shield.efficiency=templ->shield.efficiency;
				}
			}
		}
	}

	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Shield_Leak")) {
		double myleak = 100-shield.leak;
		double upleak = 100-up->shield.leak;
		double templeak = 100-(templ!=NULL?templ->shield.leak:0);
		bool ccf = cancompletefully;
		STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,100,100,false,shield.leak);
		if (touchme&&myleak<=100&&myleak>=0)shield.leak=(char)100-myleak;
		cancompletefully = ccf;
	}

								 // DO NOT CHANGE see unit_customize.cpp
	static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));
								 //DO NOT CHANGE! see unit.cpp:258
	static float tc =XMLSupport::parse_float (vs_config->getVariable ("physics","autotracking",".93"));
	static bool use_template_maxrange= XMLSupport::parse_bool (vs_config->getVariable("physics","use_upgrade_template_maxrange","true"));

	// Radar stuff
	if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Radar_Range|Radar_Color|ITTS|Can_Lock|Max_Cone|Lock_Cone|Tracking_Cone")) {
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Radar_Range"))
			STDUPGRADECLAMP(computer.radar.maxrange,up->computer.radar.maxrange,use_template_maxrange?templ->computer.radar.maxrange:FLT_MAX,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Radar_Color"))
			STDUPGRADE(computer.radar.iff,up->computer.radar.iff,templ->computer.radar.iff,0);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"ITTS"))
			computer.itts=UpgradeBoolval(computer.itts,up->computer.itts,touchme,downgrade,numave,percentage,force_change_on_nothing);
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Can_Lock"))
			computer.radar.canlock=UpgradeBoolval(computer.radar.canlock,up->computer.radar.canlock,touchme,downgrade,numave,percentage,force_change_on_nothing);

		// Do the two reversed ones below
		bool ccf = cancompletefully;
		if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Max_Cone")) {
			double myleak = 1-computer.radar.maxcone;
			double upleak = 1-up->computer.radar.maxcone;
			double templeak = 1-(templ!=NULL?templ->computer.radar.maxcone:-1);
			STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,0,0,false,computer.radar.maxcone);
			if (touchme)computer.radar.maxcone=1-myleak;
		}
		if (up->computer.radar.lockcone!=lc) {
			double myleak = 1-computer.radar.lockcone;
			double upleak = 1-up->computer.radar.lockcone;
			double templeak = 1-(templ!=NULL?templ->computer.radar.lockcone:-1);
			if (templeak == 1-lc)
				templeak=2;
			if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Lock_Cone")) {
				STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,0,0,false,computer.radar.lockcone);
				if (touchme)computer.radar.lockcone=1-myleak;
			}
		}
		if (up->computer.radar.trackingcone!=tc) {
			double myleak = 1-computer.radar.trackingcone;
			double upleak = 1-up->computer.radar.trackingcone;
			double templeak = 1-(templ!=NULL?templ->computer.radar.trackingcone:-1);
			if (templeak==1-tc)
				templeak=2;
			if(!csv_cell_null_check||force_change_on_nothing||cell_has_recursive_data(upgrade_name,up->faction,"Tracking_Cone")) {
				STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,0,0,false,computer.radar.trackingcone);
				if (touchme)computer.radar.trackingcone=1-myleak;
			}
		}
		cancompletefully=ccf;
	}

	//NO CLUE FOR BELOW
	if (downgrade) {
		//    STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
		if (jump.drive>=-1&&up->jump.drive>=-1) {
			if (touchme) jump.drive=-2;
			++numave;
			percentage+=.5*((float)(100-jump.damage))/(101-up->jump.damage);
			if (gen_downgrade_list)
				AddToDowngradeMap (up->name,up->jump.drive,((char *)&this->jump.drive)-((char *)this),tempdownmap);
		}
		if (cloaking!=-1&&up->cloaking!=-1) {
			if (touchme) cloaking=-1;
			++numave;
			++percentage;
			if (gen_downgrade_list)
				AddToDowngradeMap (up->name,up->cloaking,((char *)&this->cloaking)-((char *)this),tempdownmap);
		}

		// NOTE: Afterburner type 2 (jmp)
		// NOTE: Afterburner type 1 (gas)
		// NOTE: Afterburner type 0 (pwr)

		if (afterburnenergy<32767&&afterburnenergy<=up->afterburnenergy&&up->afterburnenergy!=32767&&up->afterburnenergy!=0) {
			if (touchme) afterburnenergy=32767, afterburntype=0;
			++numave;
			++percentage;
			if (gen_downgrade_list)
				AddToDowngradeMap (up->name,up->afterburntype,((char *)&this->afterburnenergy)-((char *)this),tempdownmap);
		}

	}
	else {
		// we are upgrading!
		if (touchme) {
			for (unsigned int i=0;i<up->image->cargo.size();++i) {
				if (CanAddCargo(up->image->cargo[i])) {
					AddCargo(up->image->cargo[i],false);
				}

			}

		}
		if ((cloaking==-1&&up->cloaking!=-1)||force_change_on_nothing) {
			if (touchme) {cloaking=up->cloaking;cloakmin=up->cloakmin;image->cloakrate=up->image->cloakrate; image->cloakglass=up->image->cloakglass;image->cloakenergy=up->image->cloakenergy;}
			++numave;

		}
		else if (cloaking!=-1&& up->cloaking!=-1) {
			cancompletefully=false;
		}

		// NOTE: Afterburner type 2 (jmp)
		// NOTE: Afterburner type 1 (gas)
		// NOTE: Afterburner type 0 (pwr)

		if (((afterburnenergy>up->afterburnenergy
              || (afterburntype!=up->afterburntype&&up->afterburnenergy!=32767))
             && up->afterburnenergy>0) || force_change_on_nothing) {
			++numave;
			if (touchme) afterburnenergy=up->afterburnenergy, afterburntype=up->afterburntype;
		}
		else if (afterburnenergy<=up->afterburnenergy&&afterburnenergy>=0&&up->afterburnenergy>0&&up->afterburnenergy<32767) {
			cancompletefully=false;
		}
		if ((jump.drive==-2 && up->jump.drive>=-1) || force_change_on_nothing) {
			if (touchme) {jump.drive = up->jump.drive;jump.damage=0;}
			++numave;
		}
		else if (jump.drive>=-1&&up->jump.drive>=-1) {
			cancompletefully=false;
		}
	}
	if (needs_redemption) {
		if (!can_be_redeemed) {
			cancompletefully=false;
		}
	}
	if(0==numave){ // Doesn't upgrade anything -- JS_NUDGE -- may want to revisit this later
		percentage=1.0;
	}
	if (numave){
		percentage=percentage/numave;
	}
	if (0&&touchme&&up->Mass&&numave) {
		float multiplyer =((downgrade)?-1:1);
		Mass +=multiplyer*percentage*up->Mass;
		if (Mass<(templ?templ->Mass:.000000001))
			Mass=(templ?templ->Mass:.000000001);
		Momentofinertia +=multiplyer*percentage*up->Momentofinertia;
		if (Momentofinertia<(templ?templ->Momentofinertia:0.00000001)) {
			Momentofinertia=(templ?templ->Momentofinertia:0.00000001);
		}
	}
	if (gen_downgrade_list) {
		float MyPercentMin = ComputeMinDowngradePercent();
		if (downgrade && percentage > MyPercentMin) {
			for (vsUMap<int,DoubleName>::iterator i = tempdownmap.begin();i!=tempdownmap.end();++i) {
				downgrademap[(*i).first]=(*i).second;
			}
		}
	}

	return cancompletefully;
}


bool Unit::ReduceToTemplate()
{
	vector <Cargo> savedCargo;
	savedCargo.swap(image->cargo);
	vector <Mount> savedWeap;
	savedWeap.swap(mounts);
	int upfac = FactionUtil::GetUpgradeFaction();
	const Unit * temprate = makeFinalBlankUpgrade (name,faction);
	bool success=false;
	double pct=0;
	if (temprate&&temprate->name!=string("LOAD_FAILED")) {
		success = Upgrade(temprate,-1,-1,0,true,pct,NULL,true);
		if (pct>0)
			success=true;
	}
	savedCargo.swap(image->cargo);
	savedWeap.swap(mounts);
	return success;
}


Vector Unit::MountPercentOperational (int whichmount)
{
	if (whichmount<0||whichmount>=mounts.size()) return Vector(-1,-1,-1);
	return Vector(mounts[whichmount].functionality,
		mounts[whichmount].maxfunctionality,
		((mounts[whichmount].status==Mount::ACTIVE||mounts[whichmount].status==Mount::INACTIVE)?0.0:(Mount::UNCHOSEN?2.0:1.0)));
}


int Unit::RepairCost ()
{
	int cost =1;
	unsigned int i;
	for (i=0;i < (1+MAXVDUS+UnitImages::NUMGAUGES)*2;++i) {
		if (image->cockpit_damage[i]<1) {
			++cost;
		}
	}
	if (image->fireControlFunctionality<1)
		++cost;
	if (image->fireControlFunctionalityMax<1)
		++cost;
	if (image->SPECDriveFunctionality<1)
		++cost;
	if (image->SPECDriveFunctionalityMax<1)
		++cost;
	if (image->CommFunctionality<1)
		++cost;
	if (image->CommFunctionalityMax<1)
		++cost;
	if (image->LifeSupportFunctionality<1)
		++cost;
	if (image->LifeSupportFunctionalityMax<1)
		++cost;

	for (i=0;i<numCargo();++i) {
		if (GetCargo(i).GetCategory().find(DamagedCategory)==0)
			++cost;
	}
	return cost;
}


int Unit::RepairUpgrade ()
{
	vector <Cargo> savedCargo;
	savedCargo.swap(image->cargo);
	vector <Mount> savedWeap;
	savedWeap.swap(mounts);
	int upfac = FactionUtil::GetUpgradeFaction();
	const Unit * temprate = makeFinalBlankUpgrade (name,faction);
	int success=0;
	double pct=0;
	if (temprate&&temprate->name!=string("LOAD_FAILED")) {
		success = Upgrade(temprate,-1,-1,0,false,pct,NULL,false)?1:0;
		if (pct>0)
			success=1;
	}
	savedCargo.swap(image->cargo);
	savedWeap.swap(mounts);
	UnitImages * im= &GetImageInformation();
	for (int i=0;i < (1+MAXVDUS+UnitImages::NUMGAUGES)*2;++i) {
		if (im->cockpit_damage[i]<1) {
			im->cockpit_damage[i]=1;
			success+=1;
			pct = 1;
		}
	}
	if (im->fireControlFunctionality<1) {
		im->fireControlFunctionality=1;
		pct=1;
		success+=1;
	}
	if (im->fireControlFunctionalityMax<1) {
		im->fireControlFunctionalityMax=1;
		pct=1;
		success+=1;
	}
	if (im->SPECDriveFunctionality<1) {
		im->SPECDriveFunctionality=1;
		pct=1;
		success+=1;
	}
	if (im->SPECDriveFunctionalityMax<1) {
		im->SPECDriveFunctionalityMax=1;
		pct=1;
		success+=1;
	}
	if (im->CommFunctionality<1) {
		im->CommFunctionality=1;
		pct=1;
		success+=1;
	}
	if (im->CommFunctionalityMax<1) {
		im->CommFunctionalityMax=1;
		pct=1;
		success+=1;
	}
	if (im->LifeSupportFunctionality<1) {
		im->LifeSupportFunctionality=1;
		pct=1;
		success+=1;
	}
	if (im->LifeSupportFunctionalityMax<1) {
		im->LifeSupportFunctionalityMax=1;
		pct=1;
		success+=1;
	}

	bool ret = success && pct>0;
	static bool ComponentBasedUpgrades = XMLSupport::parse_bool (vs_config->getVariable("physics","component_based_upgrades","false"));
	if (ComponentBasedUpgrades) {
		for (unsigned int i=0;i<numCargo();++i) {
			if (GetCargo(i).GetCategory().find(DamagedCategory)==0) {
				++success;
				static int damlen = strlen(DamagedCategory);
				GetCargo(i).category="upgrades/"+GetCargo(i).GetCategory().substr(damlen);
			}
		}
	}
	else if (ret) {
		const Unit * maxrecharge= makeTemplateUpgrade(name.get()+".template",faction);

		Unit * mpl = UnitFactory::getMasterPartList();
		for (unsigned int i=0;i<mpl->numCargo();++i) {
			if (mpl->GetCargo(i).GetCategory().find("upgrades")==0) {
				const Unit * up = loadUnitByCache(mpl->GetCargo(i).content,upfac);
				//now we analyzify up!
				if (up->MaxShieldVal()==MaxShieldVal()&&up->shield.recharge>shield.recharge) {
					shield.recharge = up->shield.recharge;
					if (maxrecharge)
						if (shield.recharge>maxrecharge->shield.recharge)
							shield.recharge=maxrecharge->shield.recharge;
				}
				if (up->maxenergy==maxenergy&&up->recharge>recharge) {
					recharge = up->recharge;
					if (recharge>maxrecharge->recharge)
						recharge= maxrecharge->recharge;

				}
			}
		}

	}
	return success;
}

float RepairPrice(float operational, float price) {
  return .5*price*(1-operational)*g_game.difficulty;
}

extern bool isWeapon (const std::string & name);

// item must be non-null... but baseUnit or credits may be NULL.
bool Unit::RepairUpgradeCargo(Cargo *item, Unit *baseUnit, float *credits) {
	double itemPrice = baseUnit?baseUnit->PriceCargo(item->content):item->price;
	if (isWeapon(item->category)) {
		const Unit * upgrade=getUnitFromUpgradeName(item->content,this->faction);
		if (upgrade->GetNumMounts()) {
			double price = itemPrice; // RepairPrice probably won't work for mounts.
			if (!credits || price<=(*credits)) {
				if (credits) (*credits)-=price;
				const Mount * mnt = &upgrade->mounts[0];
				unsigned int nummounts=this->GetNumMounts();
				bool complete=false;
				for (unsigned int i=0;i<nummounts;++i) {
					if (mnt->type->weapon_name==this->mounts[i].type->weapon_name) {
						if (this->mounts[i].status==Mount::DESTROYED){
							this->mounts[i].status=Mount::INACTIVE;
							complete=true;
						}
						if (this->mounts[i].functionality<1.0f){
							this->mounts[i].functionality=1.0f;
							complete=true;
						}
						if (this->mounts[i].maxfunctionality<1.0f){
							this->mounts[i].maxfunctionality=1.0f;
							complete=true;
						}
						if (complete) break;
					}
				}
				return complete;
			}
		}
		return false;
	} else {
		Cargo sold;
		const int quantity=1;
		bool notadditive=(item->GetContent().find("add_")!=0&&item->GetContent().find("mult_")!=0);
		if (notadditive||item->GetCategory().find(DamagedCategory)==0) {
			Cargo itemCopy = *item;     // Copy this because we reload master list before we need it.

			//this->SellCargo(item->content, quantity, _Universe->AccessCockpit()->credits, sold, baseUnit);
			//UnitUtil::RecomputeUnitUpgrades(this);
			const Unit * un=  getUnitFromUpgradeName(item->content,this->faction);
			if (un) {
				double percentage = UnitUtil::PercentOperational(this,item->content,item->category,false);
				double price = RepairPrice(percentage,itemPrice);
				if (!credits || price<=(*credits)) {
					if (credits) (*credits)-=price;
					if (notadditive)
						this->Upgrade(un,0,0,0,true,percentage,makeTemplateUpgrade(this->name,this->faction));
					if (item->GetCategory().find(DamagedCategory)==0) {
						unsigned int where;
						Cargo * c=this->GetCargo(item->content,where);
						if (c) c->category="upgrades/"+c->GetCategory().substr(strlen(DamagedCategory));
					}
					return true;
				}
			}
		}
	}
	return false;
}

/***********************************************************************************/
/**** UNIT_CARGO STUFF                                                            */
/***********************************************************************************/

/***************** UNCOMMENT GETMASTERPARTLIST WHEN MODIFIED FACTION STUFF !!!!!! */

float Unit::PriceCargo (const std::string &s)
{
	Cargo tmp;
	tmp.content=s;
	vector <Cargo>::iterator mycargo = std::find (image->cargo.begin(),image->cargo.end(),tmp);
	if (mycargo==image->cargo.end()) {
		Unit * mpl = UnitFactory::getMasterPartList();
		if (this!=mpl) {
			return mpl->PriceCargo(s);
		}
		else {
			static float spacejunk=parse_float (vs_config->getVariable ("cargo","space_junk_price","10"));
			return spacejunk;
		}
	}
	float price;
	/*
	if (mycargo==image->cargo.end()) {
	Cargo * masterlist;
	if ((masterlist=GetMasterPartList (s.c_str()))!=NULL) {
	  price =masterlist->price;
	} else {
	  static float spacejunk=parse_float (vs_config->getVariable ("cargo","space_junk_price","10"));
	  price = spacejunk;
	}
	} else {
	*/
	price = (*mycargo).price;
	//}
	return price;
}


static const GFXColor disable (1,0,0,1);
extern int GetModeFromName (const char *);
extern double ComputeMinDowngradePercent();

vector <CargoColor>& Unit::FilterDowngradeList (vector <CargoColor> & mylist, bool downgrade)
{
	const Unit * templ=NULL;
	const Unit * downgradelimit=NULL;
	static bool staticrem =XMLSupport::parse_bool (vs_config->getVariable ("general","remove_impossible_downgrades","true"));
	static float MyPercentMin= ComputeMinDowngradePercent();
	int upgrfac=FactionUtil::GetUpgradeFaction();

	for (unsigned int i=0;i<mylist.size();++i) {
		bool removethis=true/*staticrem*/;
		int mode=GetModeFromName(mylist[i].cargo.GetContent().c_str());
		if (mode!=2 || (!downgrade)) {
			const Unit * NewPart =  UnitConstCache::getCachedConst (StringIntKey (mylist[i].cargo.GetContent().c_str(),upgrfac));
			if (!NewPart) {
				NewPart= UnitConstCache::setCachedConst (StringIntKey (mylist[i].cargo.GetContent(),upgrfac),UnitFactory::createUnit(mylist[i].cargo.GetContent().c_str(),false,upgrfac));
			}
			if (NewPart->name==string("LOAD_FAILED")) {
				const Unit * NewPart = UnitConstCache::getCachedConst (StringIntKey (mylist[i].cargo.GetContent().c_str(),faction));
				if (!NewPart) {
					NewPart= UnitConstCache::setCachedConst (StringIntKey (mylist[i].cargo.content, faction),
						UnitFactory::createUnit(mylist[i].cargo.GetContent().c_str(),false,faction));
				}
			}
			if (NewPart->name!=string("LOAD_FAILED")) {
				int maxmountcheck = NewPart->GetNumMounts()?GetNumMounts():1;
				char * unitdir  = GetUnitDir(name.get().c_str());
				string templnam = string(unitdir)+".template";
				string limiternam = string(unitdir)+".blank";

				if (!downgrade) {
					templ = UnitConstCache::getCachedConst (StringIntKey(templnam,faction));
					if (templ==NULL) {
						templ = UnitConstCache::setCachedConst (StringIntKey(templnam,faction),UnitFactory::createUnit (templnam.c_str(),true,this->faction));
					}
					if (templ->name == std::string("LOAD_FAILED")) templ = NULL;

				}
				else {
					downgradelimit = UnitConstCache::getCachedConst (StringIntKey(limiternam,faction));
					if (downgradelimit==NULL) {
						downgradelimit = UnitConstCache::setCachedConst (StringIntKey (limiternam,faction),UnitFactory::createUnit(limiternam.c_str(),true,this->faction));
					}
					if (downgradelimit->name == std::string("LOAD_FAILED")) downgradelimit = NULL;
				}
				free (unitdir);
				for (int m=0;m<maxmountcheck;++m) {
					int s =0;
					for (un_iter ui=getSubUnits();s==0||((*ui)!=NULL);++ui,++s) {
						double percent=1;
						if (downgrade) {
							if (canDowngrade (NewPart,m,s,percent,downgradelimit)) {
								if (percent>MyPercentMin) {
									removethis=false;
									break;
								}
							}
						}
						else {

							if (canUpgrade (NewPart,m,s,mode,false/*force*/, percent,templ)) {
							removethis=false;
							break;
						}
					}

					if (*ui==NULL) {
						break;
					}
				}
			}
		}

	}
	else {
		removethis=true;
	}
	if (removethis) {
		if (downgrade&&staticrem) {
			mylist.erase (mylist.begin()+i);
			i--;
		}
		else {
			mylist[i].color=disable;
		}
	}
}


return mylist;
}


vector <CargoColor>& Unit::FilterUpgradeList (vector <CargoColor> & mylist)
{
	static bool filtercargoprice = XMLSupport::parse_bool (vs_config->getVariable ("cargo","filter_expensive_cargo","false"));
	if (filtercargoprice) {
		Cockpit * cp = _Universe->isPlayerStarship (this);
		if (cp) {
			for (unsigned int i=0;i<mylist.size();++i) {
				if (mylist[i].cargo.price>cp->credits) {
					//	      mylist.erase (mylist.begin()+i);
					//	      i--;
					mylist[i].color=disable;
				}
			}
		}
	}
	return FilterDowngradeList(mylist,false);
}


inline float uniformrand (float min, float max)
{
	return ((float)(rand ())/RAND_MAX)*(max-min)+min;
}


inline QVector randVector (float min, float max)
{
	return QVector (uniformrand(min,max),
		uniformrand(min,max),
		uniformrand(min,max));
}


void Unit::TurretFAW()
{
	turretstatus=3;
	Unit * un;
	for(un_iter iter = getSubUnits();(un = *iter)!=NULL;++iter){
		if (!CheckAccessory(un)) {
			un->EnqueueAIFirst (new Orders::FireAt(15.0f));
			un->EnqueueAIFirst (new Orders::FaceTarget (false,3));
		}
		un->TurretFAW();
	}

}


extern int SelectDockPort(Unit *, Unit*parent);
//extern unsigned int current_cockpit;
void Unit::EjectCargo (unsigned int index)
{
	Cargo * tmp=NULL;
	Cargo ejectedPilot;
	Cargo dockedPilot;
	string name;
	bool isplayer = false;
	//  if (index==((unsigned int)-1)) { is ejecting normally
	//  if (index==((unsigned int)-2)) { is ejecting for eject-dock

	Cockpit * cp = NULL;
	if (index==((unsigned int)-2)) {
		int pilotnum = _Universe->CurrentCockpit();
								 // this calls the unit's existence, by the way.
		name = "return_to_cockpit";
		if (NULL!=(cp = _Universe->isPlayerStarship (this))) {
			isplayer = true;
			string playernum =string("player")+((pilotnum==0)?string(""):XMLSupport::tostring(pilotnum));
			//name = vs_config->getVariable(playernum,"callsign","TigerShark");
		}
		//    dockedPilot.content="eject";
								 // we will have to check for this on undock to return to the parent unit!
		dockedPilot.content="return_to_cockpit";
		dockedPilot.mass=.1;
		dockedPilot.volume=1;
		tmp = &dockedPilot;
	}
	if (index==((unsigned int)-1)) {
		int pilotnum = _Universe->CurrentCockpit();
		name = "Pilot";
		if (NULL!=(cp = _Universe->isPlayerStarship (this))) {
			string playernum =string("player")+((pilotnum==0)?string(""):XMLSupport::tostring(pilotnum));
			//name = vs_config->getVariable(playernum,"callsign","TigerShark");
			isplayer = true;
		}
		ejectedPilot.content="eject";
		ejectedPilot.mass=.1;
		ejectedPilot.volume=1;
		tmp = &ejectedPilot;
	}
	if (index<numCargo()) {
		tmp = &GetCargo (index);
	}
	static float cargotime=XMLSupport::parse_float(vs_config->getVariable("physics","cargo_live_time","600"));
	if (tmp) {
		string tmpcontent=tmp->content;
		if (tmp->mission)
			tmpcontent="Mission_Cargo";

		const int ulen=strlen("upgrades");

		//prevents a number of bad things, incl. impossible speeds and people getting rich on broken stuff

		if ((!tmp->mission)&&memcmp (tmp->GetCategory().c_str(),"upgrades",ulen)==0) {
			tmpcontent="Space_Salvage";
		}

		// this happens if it's a ship
		if (tmp->quantity>0) {
			const int sslen=strlen("starships");
			Unit * cargo = NULL;
			if (tmp->GetCategory().length()>=(unsigned int)sslen) {
				if ((!tmp->mission)&&memcmp (tmp->GetCategory().c_str(),"starships",sslen)==0) {
					string ans = tmpcontent;
					string::size_type blank = ans.find (".blank");
					if (blank != string::npos) {
						ans = ans.substr (0,blank);
					}
					Flightgroup * fg = this->getFlightgroup();
					int fgsnumber=0;
					if (fg!=NULL) {
						fgsnumber=fg->nr_ships;
						++(fg->nr_ships);
						++(fg->nr_ships_left);
					}
					cargo = UnitFactory::createUnit (ans.c_str(),false,faction,"",fg,fgsnumber,NULL,getUniqueSerial());
					cargo->PrimeOrders();
					cargo->SetAI (new Orders::AggressiveAI ("default.agg.xml"));
					cargo->SetTurretAI();
					//he's alive!!!!!
				}
			}
			float arot=0;
			static float grot=XMLSupport::parse_float(vs_config->getVariable("graphics","generic_cargo_rotation_speed","1"))*3.1415926536/180;
			if (!cargo) {
				static float crot=XMLSupport::parse_float(vs_config->getVariable("graphics","cargo_rotation_speed","60"))*3.1415926536/180;
				static float erot=XMLSupport::parse_float(vs_config->getVariable("graphics","eject_rotation_speed","0"))*3.1415926536/180;

				if (tmpcontent=="eject") {
					if (isplayer) {
						//     			  cargo->faction = this->faction;
						Flightgroup * fg = this->getFlightgroup();
						int fgsnumber=0;
						if (fg!=NULL) {
							fgsnumber=fg->nr_ships;
							++(fg->nr_ships);
							++(fg->nr_ships_left);
						}
						cargo = UnitFactory::createUnit ("eject",false,faction,"",fg,fgsnumber,NULL,getUniqueSerial());
					}
					else {
						int fac = FactionUtil::GetUpgradeFaction();
						cargo = UnitFactory::createUnit ("eject",false,fac,"",NULL,0,NULL,getUniqueSerial());
					}
					if (owner)
						cargo->owner = owner;
					else
						cargo->owner = this;

					arot=erot;
					static bool eject_attacks=XMLSupport::parse_bool(vs_config->getVariable("AI","eject_attacks","false"));
					if (eject_attacks) {
						cargo->PrimeOrders();
								 // generally fraidycat AI
						cargo->SetAI (new Orders::AggressiveAI ("default.agg.xml"));
					}
					//                          cargo->SetTurretAI();

					// Meat. Docking should happen here

				}
				else if (tmpcontent=="return_to_cockpit") {
					if (isplayer) {
						//     			  cargo->faction = this->faction;
						Flightgroup * fg = this->getFlightgroup();
						int fgsnumber=0;
						if (fg!=NULL) {
							fgsnumber=fg->nr_ships;
							++(fg->nr_ships);
							++(fg->nr_ships_left);
						}
						cargo = UnitFactory::createUnit ("return_to_cockpit",false,faction,"",fg,fgsnumber,NULL,getUniqueSerial());
						if (owner)
							cargo->owner = owner;
						else
							cargo->owner = this;
					}
					else {
						int fac = FactionUtil::GetUpgradeFaction();
						static float ejectcargotime=XMLSupport::parse_float(vs_config->getVariable("physics","eject_live_time",SERVER?"200":"0"));
						if (cargotime==0.0) {
							cargo = UnitFactory::createUnit ("eject",false,fac,"",NULL,0,NULL,getUniqueSerial());
						}
						else {
							cargo = UnitFactory::createMissile ("eject",
								fac,"",
								0,
								0,
								ejectcargotime,
								1,
								1,
								1,
								getUniqueSerial());
						}
					}

					arot=erot;
					cargo->PrimeOrders();
					Order * ai = cargo->aistate;
					cargo->aistate = NULL;

					//						  cargo->is_ejectdock = true; // ugly, but i hope it doesn't mess anything up. Checked by undocking.
					//						  this->is_ejectdock = false;
					//	                      cargo->PrimeOrders (new Orders::DockingOps (this, ai,actually_dock!=0));
					//                          cargo->SetAI (new Orders::DockingOps (this, ai,actually_dock!=0));
					//						  cargo->SetTurretAI();

				}
				else {
					string tmpnam = tmpcontent+".cargo";
					static std::string nam("Name");
					float rot=crot;
					if (UniverseUtil::LookupUnitStat(tmpnam,"upgrades",nam).length()==0) {
						tmpnam="generic_cargo";
						rot=grot;
					}
					int upgrfac=FactionUtil::GetUpgradeFaction();
					cargo = UnitFactory::createMissile (tmpnam.c_str(),
						upgrfac,
						"",
						0,
						0,
						cargotime,
						1,
						1,
						1,
						getUniqueSerial()
						);
					arot=rot;
					//cargo->PrimeOrders();
					//cargo->SetAI (new Orders::AggressiveAI ("cargo.agg.xml"));
				}

			}

			if (cargo->name=="LOAD_FAILED") {

				cargo->Kill();
				cargo = UnitFactory::createMissile ("generic_cargo",
					FactionUtil::GetUpgradeFaction(),"",
					0,
					0,
					cargotime,
					1,
					1,
					1,
					getUniqueSerial());
				arot=grot;

			}
			Vector rotation(vsrandom.uniformInc(-arot,arot),vsrandom.uniformInc(-arot,arot),vsrandom.uniformInc(-arot,arot));
			static bool all_rotate_same=XMLSupport::parse_bool(vs_config->getVariable("graphics","cargo_rotates_at_same_speed","true"));
			if (all_rotate_same&&arot!=0) {
				float tmp=rotation.Magnitude();
				if (tmp>.001) {
					rotation.Scale(1/tmp);
					rotation*=arot;
				}
			}
			if (0 && cargo->rSize()>=rSize()) {
				cargo->Kill();
			}
			else {

				Vector tmpvel=-Velocity;
				if (tmpvel.MagnitudeSquared()<.00001) {
					tmpvel=randVector(-rSize(),rSize()).Cast();
					if (tmpvel.MagnitudeSquared()<.00001) {
						tmpvel=Vector(1,1,1);
					}
				}
				tmpvel.Normalize();

				if ((SelectDockPort (this, this) > -1 )) {
				// it's a starship, AND we have a docking port to launch it from (cargo mines count)
					//cargo->SetPosAndCumPos (Position()+DockingPortLocations()[1].pos.Cast());

					static float eject_cargo_offset=XMLSupport::parse_float(vs_config->getVariable("physics","eject_distance","20"));
					QVector loc (Transform (this->GetTransformation(),this->DockingPortLocations()[0].pos.Cast()));
					loc += tmpvel*1.5*rSize() + randVector(-.5*rSize()+(index==-1?eject_cargo_offset/2:0), .5*rSize()+(index==-1?eject_cargo_offset:0));
					cargo->SetPosAndCumPos (loc);
					Vector p,q,r;
					this->GetOrientation(p,q,r);
					cargo->SetOrientation (p,q,r);
					if (owner)
						cargo->owner = owner;
					else
						cargo->owner = this;

					//		cargo->SetAngularVelocity(); // how do we make this aim in the same direction? ideall
				}
				else {
					cargo->SetPosAndCumPos (Position()+tmpvel*1.5*rSize()+randVector(-.5*rSize(), .5*rSize()));
					cargo->SetAngularVelocity(rotation);
				}
				static float velmul=XMLSupport::parse_float(vs_config->getVariable("physics","eject_cargo_speed","1"));
				cargo->SetOwner (this);
				cargo->SetVelocity(Velocity*velmul+randVector(-.25,.25).Cast());
				cargo->Mass = tmp->mass;
				if (name.length()>0) {
					cargo->name=name;
				}
				else {
					if (tmp) {
						cargo->name=tmpcontent;
					}
				}
				if (cp&&_Universe->numPlayers()==1) {
					cargo->SetOwner(NULL);
					PrimeOrders();
					cargo->SetTurretAI();
					cargo->faction=faction;
								 // changes control to that cockpit
					cp->SetParent (cargo,"","",Position());
					if (tmpcontent=="return_to_cockpit") {
						if ((game_options.simulate_while_at_base)||(_Universe->numPlayers()>1))
							this->TurretFAW();

								 // make unit a sitting duck in the mean time
						SwitchUnits (NULL,this);
						if (owner)
							cargo->owner = owner;
						else
							cargo->owner = this;
						PrimeOrders();
						//this->SetAI (new Orders::AggressiveAI ("cargo.agg.xml"));// make unit a sitting duck in the mean time

						//this->SetTurretAI();  // but defend yourself

						cargo->SetOwner(this);
						cargo->Position()=this->Position();
						cargo->SetPosAndCumPos(this->Position());
								 // claims to be docked, stops speed and taking damage etc. but doesn't seem to call the base script
						cargo->ForceDock(this, 0);
						abletodock(3);
								 // actually calls the interface, meow. yay!
						cargo->UpgradeInterface(this);
						//          cargo->Kill();
						//this->UpgradeInterface(this); // this seriously breaks the game...
						//          DockedScript(cargo,this);      // this just don't work.
						if ((game_options.simulate_while_at_base)||(_Universe->numPlayers()>1))
							this->TurretFAW();

					}
					else {
						SwitchUnits (NULL,cargo);
						if (owner)
							cargo->owner = owner;
						else
							cargo->owner = this;

					}			 // switching NULL gives "dead" ai to the unit I ejected from, by the way.
				}
				_Universe->activeStarSystem()->AddUnit(cargo);
				if ((unsigned int) index!=((unsigned int)-1)&&(unsigned int)index!=((unsigned int)-2)) {
					if (index<image->cargo.size()) {
						RemoveCargo (index,1,true);
					}
				}
			}
		}

	}
}


int Unit::RemoveCargo (unsigned int i, int quantity,bool eraseZero)
{
	if (!(i<image->cargo.size())) {
		UNIT_LOG(logvs::WARN, "(previously) FATAL problem...removing cargo that is past the end of array bounds.");
		return 0;
	}
	Cargo *carg = &(image->cargo[i]);
	if (quantity>carg->quantity)
		quantity=carg->quantity;
	if (Network && !_Universe->netLocked()) {
		int playernum = _Universe->whichPlayerStarship( this);

		if (playernum>=0) {
			Network[playernum].cargoRequest( 0, this->serial, carg->GetContent(), quantity, 0, 0);
		} else {
			return 0;
		}
		return 0;
	}
	static bool usemass = XMLSupport::parse_bool(vs_config->getVariable ("physics","use_cargo_mass","true"));
	if (usemass)
		Mass-=quantity*carg->mass;
	if (SERVER && !_Universe->netLocked() && getStarSystem()) {
		VSServer->BroadcastCargoUpgrade( this->serial, 0, this->serial, carg->GetContent(),
							carg->price, carg->mass, carg->volume, carg->mission,
							quantity, 0, 0, getStarSystem()->GetZone());
	}
	carg->quantity-=quantity;
	if (carg->quantity<=0&&eraseZero)
		image->cargo.erase (image->cargo.begin()+i);
	return quantity;
}


void Unit::AddCargo (const Cargo &carg, bool sort)
{
	if (Network && !_Universe->netLocked()) {
		int playernum = _Universe->whichPlayerStarship( this);
		if (playernum>=0) {
			Network[playernum].cargoRequest( this->serial, 0, carg.GetContent(), carg.quantity, 0, 0);
		} else {
			return;
		}
		return;
	}
	if (SERVER && !_Universe->netLocked() && getStarSystem()) {
		VSServer->BroadcastCargoUpgrade( this->serial, this->serial, 0, carg.GetContent(),
								carg.price, carg.mass, carg.volume, carg.mission,
								carg.quantity, 0, 0, getStarSystem()->GetZone());
	}
	static bool usemass = XMLSupport::parse_bool(vs_config->getVariable ("physics","use_cargo_mass","true"));
	if (usemass)
		Mass+=carg.quantity*carg.mass;
	image->cargo.push_back (carg);
	if (sort)
		SortCargo();
}


bool cargoIsUpgrade(const Cargo& c)
{
	return c.GetCategory().find("upgrades")==0;
}


float Unit::getHiddenCargoVolume()const
{
	return image->HiddenCargoVolume;
}


bool Unit::CanAddCargo (const Cargo &carg)const
{
	// Always can, in this case (this accounts for some odd precision issues)
	if ((carg.quantity == 0) || (carg.volume == 0))
		return true;

	// Test volume availability
	bool upgradep=cargoIsUpgrade(carg);
	float total_volume=carg.quantity*carg.volume + (upgradep?getUpgradeVolume():getCargoVolume());
	if  (total_volume<=(upgradep?getEmptyUpgradeVolume():getEmptyCargoVolume()))
		return true;

	// Hm... not in main unit... perhaps a subunit can take it
	const Unit * un;
	for (un_kiter i=viewSubUnits();(un = *i)!=NULL;++i)
		if (un->CanAddCargo (carg))
			return true;

	// Bad luck
	return false;
}


// The cargo volume of this ship when empty.  Max cargo volume.
float Unit::getEmptyCargoVolume(void) const
{
	return image->CargoVolume;
}


float Unit::getEmptyUpgradeVolume(void) const
{
	return image->UpgradeVolume;
}


float Unit::getCargoVolume(void) const
{
	float result = 0.0;
	for(int i=0; i<image->cargo.size(); ++i) {
		if (!cargoIsUpgrade(image->cargo[i]))
			result += image->cargo[i].quantity*image->cargo[i].volume;
	}

	return result;
}


float Unit::getUpgradeVolume(void) const
{
	float result = 0.0;
	for(int i=0; i<image->cargo.size(); ++i) {
		if (cargoIsUpgrade(image->cargo[i]))
			result += image->cargo[i].quantity*image->cargo[i].volume;
	}

	return result;
}


UnitImages &Unit::GetImageInformation()
{
	return *image;
}


Cargo& Unit::GetCargo (unsigned int i)
{
	return image->cargo[i];
}


const Cargo& Unit::GetCargo (unsigned int i) const
{
	return image->cargo[i];
}


class CatCompare
{
	public:
		bool operator ()(const Cargo &a,const Cargo& b) {
			std::string::const_iterator aiter=a.GetCategory().begin();
			std::string::const_iterator aend=a.GetCategory().end();
			std::string::const_iterator biter=b.GetCategory().begin();
			std::string::const_iterator bend=b.GetCategory().end();
			for (;aiter!=aend&&biter!=bend;++aiter,++biter) {
				char achar=*aiter;
				char bchar=*biter;
				if (achar<bchar)
					return true;
				if (achar>bchar)
					return false;
			}
			//    return a.category<b.category;
			return false;
		}

};
void Unit::GetSortedCargoCat (const std::string &cat, size_t &begin, size_t &end)
{
	vector<Cargo>::iterator Begin=image->cargo.begin();
	vector<Cargo>::iterator End=image->cargo.end();
	vector<Cargo>::iterator lbound=image->cargo.end();
	vector<Cargo>::iterator ubound=image->cargo.end();

	Cargo beginningtype;
	beginningtype.category=cat;
	CatCompare Comp;
	lbound=std::lower_bound(Begin,End,beginningtype,Comp);
	beginningtype.content="zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";
	ubound=std::upper_bound(Begin,End,beginningtype,Comp);
	begin=lbound-Begin;
	end=ubound-Begin;

}


Unit& GetUnitMasterPartList ()
{
	return *UnitFactory::getMasterPartList( );
}


bool myless(const Cargo & a, const Cargo& b)
{
	return a<b;
}

Cargo *Unit::GetCargo(const std::string &s, unsigned int &i) {
  const Unit * thus=this;
  if (thus->GetCargo(s,i)) {
    return &GetCargo(i);
  }
  return NULL;
}
const Cargo* Unit::GetCargo (const std::string &s, unsigned int &i)const
{
	static Hashtable<string,unsigned int,2047> index_cache_table;
	Unit * mpl=UnitFactory::getMasterPartList();
	if (this==mpl) {
		unsigned int *ind=index_cache_table.Get(s);
		if (ind) {
			if (*ind<image->cargo.size()) {
				Cargo * guess=&image->cargo[*ind];
				if (guess->content==s) {
					i=*ind;
					return guess;
				}
			}
		}
		Cargo searchfor;
		searchfor.content=s;
		vector<Cargo>::iterator tmp=std::find(image->cargo.begin(),image->cargo.end(),searchfor);
		if (tmp==image->cargo.end())
			return NULL;
		if ((*tmp).content==searchfor.content) {
			i= (tmp-image->cargo.begin());
			if (this==mpl) {
				unsigned int * tmp=new unsigned int;
				*tmp =i;
				if (index_cache_table.Get(s)) {
					index_cache_table.Delete(s);
				}
								 //memory leak--should not be reached though, ever
				index_cache_table.Put(s,tmp);
			}
			return &(*tmp);
		}
		return NULL;
	}
	Cargo searchfor;
	searchfor.content=s;
	vector<Cargo>::iterator tmp =(std::find(image->cargo.begin(),image->cargo.end(),searchfor));
	if (tmp==image->cargo.end())
		return NULL;
	i= (tmp-image->cargo.begin());
	return &(*tmp);

}


unsigned int Unit::numCargo ()const
{
	return image->cargo.size();
}


std::string Unit::GetManifest (unsigned int i, Unit * scanningUnit, const Vector &oldspd) const
{
	///FIXME somehow mangle string
	string mangled = image->cargo[i].content;
	static float scramblingmanifest=XMLSupport::parse_float (vs_config->getVariable ("general","PercentageSpeedChangeToFaultSearch",".5"));
	{							 // Keep inside subblock, otherwice MSVC will throw an error while redefining 'i'
		bool last=true;
		for (string::iterator i=mangled.begin();i!=mangled.end();++i) {
			if (last)
				(*i)=toupper(*i);
			last=(*i==' '||*i=='_');
		}
	}
	if (CourseDeviation (oldspd,GetVelocity())>scramblingmanifest) {
		for (string::iterator i=mangled.begin();i!=mangled.end();++i) {
			(*i)+=(rand()%3-1);
		}
	}

	return mangled;
}


bool Unit::SellCargo (unsigned int i, int quantity, float &creds, Cargo & carg, Unit *buyer)
{
	if (i<0||i>=image->cargo.size()||!buyer->CanAddCargo(image->cargo[i])||Mass<image->cargo[i].mass)
		return false;
	carg = image->cargo[i];
	if (quantity>image->cargo[i].quantity)
		quantity=image->cargo[i].quantity;
	carg.price=buyer->PriceCargo (image->cargo[i].content);
	if (!Network || _Universe->netLocked()) {
		// Don't give cash back until server acknowledges purchase.
		creds+=quantity*carg.price;
	}
	if (SERVER && !_Universe->netLocked()) {
		VSServer->sendCredits(serial, creds);
	}
	carg.quantity=quantity;
	buyer->AddCargo (carg);

	RemoveCargo (i,quantity);
	return true;
}


bool Unit::SellCargo (const std::string &s, int quantity, float & creds, Cargo &carg, Unit *buyer)
{
	Cargo tmp;
	tmp.content=s;
	vector <Cargo>::iterator mycargo = std::find (image->cargo.begin(),image->cargo.end(),tmp);
	if (mycargo==image->cargo.end())
		return false;

	return SellCargo (mycargo-image->cargo.begin(),quantity,creds,carg,buyer);
}

bool Unit::BuyCargo (const Cargo &carg, float & creds)
{
	if (!CanAddCargo(carg)||creds<carg.quantity*carg.price) {
		return false;
	}
	AddCargo (carg);
	creds-=carg.quantity*carg.price;
	if (Network && !_Universe->netLocked()) {
		creds=0;
	}
	if (SERVER && !_Universe->netLocked()) {
		VSServer->sendCredits(serial, creds);
	}
	return true;
}


bool Unit::BuyCargo (unsigned int i, unsigned int quantity, Unit * seller, float&creds)
{
	Cargo soldcargo= seller->image->cargo[i];
	if (quantity>(unsigned int)soldcargo.quantity)
		quantity=soldcargo.quantity;
	if (quantity==0)
		return false;
	soldcargo.quantity=quantity;
	if (BuyCargo (soldcargo,creds)) {
		seller->RemoveCargo (i,quantity,false);
		return true;
	}
	return false;
}


bool Unit::BuyCargo (const std::string &cargo,unsigned int quantity, Unit * seller, float & creds)
{
	unsigned int i;
	if (seller->GetCargo(cargo,i)) {
		return BuyCargo (i,quantity,seller,creds);
	}
	return false;
}


Cargo * GetMasterPartList(const char *input_buffer)
{
	unsigned int i;
	return GetUnitMasterPartList().GetCargo (input_buffer,i);
}


void Unit::ImportPartList (const std::string& category, float price, float pricedev,  float quantity, float quantdev)
{
	unsigned int numcarg = GetUnitMasterPartList().numCargo();
	float minprice=FLT_MAX;
	float maxprice=0;
	for (unsigned int j=0;j<numcarg;++j) {
		if (GetUnitMasterPartList().GetCargo(j).category==category) {
			float price = GetUnitMasterPartList().GetCargo(j).price;
			if (price < minprice)
				minprice = price;
			else if (price > maxprice)
				maxprice = price;
		}
	}
	for (unsigned int i=0;i<numcarg;++i) {
		Cargo c= GetUnitMasterPartList().GetCargo (i);
		if (c.category==category) {

			static float aveweight = fabs(XMLSupport::parse_float (vs_config->getVariable ("cargo","price_recenter_factor","0")));
			c.quantity=float_to_int(quantity-quantdev);
			float baseprice=c.price;
			c.price*=price-pricedev;

			//stupid way
			c.quantity+=float_to_int((quantdev*2+1)*((double)rand())/(((double)RAND_MAX)+1));
			c.price+=pricedev*2*((float)rand())/RAND_MAX;
			c.price=fabs(c.price);
			c.price=(c.price +(baseprice*aveweight))/ (aveweight+1);
			if (c.quantity<=0) {
				c.quantity=0;
			}
			else {
				//quantity more than zero
				if (maxprice>minprice+.01) {
					float renormprice = (baseprice-minprice)/(maxprice-minprice);
					static float maxpricequantadj = XMLSupport::parse_float (vs_config->getVariable ("cargo","max_price_quant_adj","5"));
					static float minpricequantadj = XMLSupport::parse_float (vs_config->getVariable ("cargo","min_price_quant_adj","1"));
					static float powah = XMLSupport::parse_float (vs_config->getVariable ("cargo","price_quant_adj_power","1"));
					renormprice = pow(renormprice,powah);
					renormprice *= (maxpricequantadj-minpricequantadj);
					renormprice+=1;
					if (renormprice>.001) {
						c.quantity/=float_to_int(renormprice);
						if (c.quantity<1)
							c.quantity=1;
					}
				}
			}
			static float minprice = XMLSupport::parse_float(vs_config->getVariable("cargo","min_cargo_price","0.01"));
			if (c.price <minprice)
				c.price=minprice;
			c.quantity=abs (c.quantity);
			AddCargo(c,false);
		}
	}
}


std::string Unit::massSerializer (const XMLType &input, void *mythis)
{
	Unit * un = (Unit *)mythis;
	float mass = un->Mass;
	static bool usemass = XMLSupport::parse_bool(vs_config->getVariable ("physics","use_cargo_mass","true"));
	for (unsigned int i=0;i<un->image->cargo.size();++i) {
		if (un->image->cargo[i].quantity>0) {
			if (usemass)
				mass-=un->image->cargo[i].mass*un->image->cargo[i].quantity;
		}
	}
	return XMLSupport::tostring((float)mass);
}


std::string Unit::shieldSerializer (const XMLType &input, void * mythis)
{
	Unit * un=(Unit *)mythis;
	switch (un->shield.number) {
		case 2:
			return tostring(un->shield.shield2fb.frontmax)+string("\" back=\"")+tostring(un->shield.shield2fb.backmax);
		case 8:
			return string("\" frontrighttop=\"")+tostring(un->shield.shield8.frontrighttop)+string("\" backrighttop=\"")+tostring(un->shield.shield8.backrighttop)+string("\" frontlefttop=\"")+tostring(un->shield.shield8.frontlefttop)+string("\" backlefttop=\"")+tostring(un->shield.shield8.backlefttop)+string("\" frontrightbottom=\"")+tostring(un->shield.shield8.frontrightbottom)+string("\" backrightbottom=\"")+tostring(un->shield.shield8.backrightbottom)+string("\" frontleftbottom=\"")+tostring(un->shield.shield8.frontleftbottom)+string("\" backleftbottom=\"")+tostring(un->shield.shield8.backleftbottom);
		case 4:
		default:
			return tostring(un->shield.shield4fbrl.frontmax)+string("\" back=\"")+tostring(un->shield.shield4fbrl.backmax)+string("\" left=\"")+tostring(un->shield.shield4fbrl.leftmax)+string("\" right=\"")+tostring(un->shield.shield4fbrl.rightmax);
	}
	return string("");
}


std::string Unit::mountSerializer (const XMLType &input, void * mythis)
{
	Unit * un=(Unit *)mythis;
	int i=input.w.hardint;
	if (un->GetNumMounts()>i) {
		string result(lookupMountSize(un->mounts[i].size));
		if (un->mounts[i].status==Mount::INACTIVE||un->mounts[i].status==Mount::ACTIVE)
			result+=string("\" weapon=\"")+(un->mounts[i].type->weapon_name);
		if (un->mounts[i].ammo!=-1)
			result+=string("\" ammo=\"")+XMLSupport::tostring(un->mounts[i].ammo);
		if (un->mounts[i].volume!=-1) {
			result+=string("\" volume=\"")+XMLSupport::tostring(un->mounts[i].volume);
		}
		result+=string("\" xyscale=\"")+XMLSupport::tostring(un->mounts[i].xyscale)+string("\" zscale=\"")+XMLSupport::tostring(un->mounts[i].zscale);
		Matrix m;
		Transformation(un->mounts[i].GetMountOrientation(),un->mounts[i].GetMountLocation().Cast()).to_matrix(m);
		result+=string ("\" x=\"")+tostring((float)(m.p.i/parse_float(input.str)));
		result+=string ("\" y=\"")+tostring((float)(m.p.j/parse_float(input.str)));
		result+=string ("\" z=\"")+tostring((float)(m.p.k/parse_float(input.str)));

		result+=string ("\" qi=\"")+tostring(m.getQ().i);
		result+=string ("\" qj=\"")+tostring(m.getQ().j);
		result+=string ("\" qk=\"")+tostring(m.getQ().k);

		result+=string ("\" ri=\"")+tostring(m.getR().i);
		result+=string ("\" rj=\"")+tostring(m.getR().j);
		result+=string ("\" rk=\"")+tostring(m.getR().k);
		return result;
	}
	else {
		return string("");
	}
}


std::string Unit::subunitSerializer (const XMLType &input, void * mythis)
{
	Unit * un=(Unit *)mythis;
	int index=input.w.hardint;
	Unit *su;
	int i=0;
	for (un_iter ui=un->getSubUnits();(su=*ui)!=NULL;++ui,++i) {
		if (i==index) {
			if (su->image->unitwriter) {
				return su->image->unitwriter->getName();
			}
			return su->name;
		}
	}
	return string("destroyed_blank");
}


void Unit::setUnitRole(const std::string &s)
{
	unitRole(ROLES::getRole(s));
}


void Unit::setAttackPreference(const std::string &s)
{
	attackPreference(ROLES::getRole(s));
}


std::string Unit::getUnitRole() const
{
	return ROLES::getRole(unitRole());
}


std::string Unit::getAttackPreference() const
{
	return ROLES::getRole(attackPreference());
}


//legacy function for python
void Unit::setCombatRole(const std::string &s)
{
	unitRole(ROLES::getRole(s));
	attackPreference(ROLES::getRole(s));
}


//legacy function for python
std::string Unit::getCombatRole() const
{
	static unsigned char inert= ROLES::getRole("INERT");
	unsigned char retA=unitRole();
	unsigned char retB=attackPreference();
								 //often missions used this to render items either uninteresting or not attacking...so want to prioritize that behavior
	if (retA==inert||retB==inert) return "INERT";
	return ROLES::getRole(retA);
}


void Unit::SortCargo()
{
	Unit *un=this;
	std::sort (un->image->cargo.begin(),un->image->cargo.end());

	for (unsigned int i=0;i+1<un->image->cargo.size();++i) {
		if (un->image->cargo[i].content==un->image->cargo[i+1].content) {
			float tmpmass = un->image->cargo[i].quantity*un->image->cargo[i].mass+un->image->cargo[i+1].quantity*un->image->cargo[i+1].mass;
			float tmpvolume = un->image->cargo[i].quantity*un->image->cargo[i].volume+un->image->cargo[i+1].quantity*un->image->cargo[i+1].volume;
			un->image->cargo[i].quantity+=un->image->cargo[i+1].quantity;
			if (un->image->cargo[i].quantity) {
				tmpmass/=un->image->cargo[i].quantity;
				tmpvolume/=un->image->cargo[i].quantity;
			}
			un->image->cargo[i].volume=tmpvolume;
			un->image->cargo[i].mission = (un->image->cargo[i].mission||un->image->cargo[i+1].mission);
			un->image->cargo[i].mass=tmpmass;
								 //group up similar ones
			un->image->cargo.erase(un->image->cargo.begin()+(i+1));
			i--;
		}

	}
}


using XMLSupport::tostring;
using namespace std;
std::string CargoToString (const Cargo& cargo)
{
	string missioncargo;
	if (cargo.mission) {
		missioncargo = string("\" missioncargo=\"")+XMLSupport::tostring(cargo.mission);
	}
	return string ("\t\t\t<Cargo mass=\"")+XMLSupport::tostring((float)cargo.mass)+string("\" price=\"") +XMLSupport::tostring((float)cargo.price)+ string("\" volume=\"")+XMLSupport::tostring((float)cargo.volume)+string("\" quantity=\"")+XMLSupport::tostring((int)cargo.quantity)+string("\" file=\"")+cargo.GetContent()+missioncargo+ string("\"/>\n");
}


std::string Unit::cargoSerializer (const XMLType &input, void * mythis)
{
	Unit * un= (Unit *)mythis;
	if (un->image->cargo.size()==0) {
		return string("0");
	}
	un->SortCargo();
	string retval("");
	if (!(un->image->cargo.empty())) {
		retval= un->image->cargo[0].GetCategory()+string ("\">\n")+CargoToString(un->image->cargo[0]);

		for (unsigned int kk=1;kk<un->image->cargo.size();++kk) {
			if (un->image->cargo[kk].category!=un->image->cargo[kk-1].category) {
				retval+=string("\t\t</Category>\n\t\t<Category file=\"")+un->image->cargo[kk].GetCategory()+string ("\">\n");
			}
			retval+=CargoToString(un->image->cargo[kk]);
		}
		retval+=string("\t\t</Category>\n\t\t<Category file=\"nothing");
	}
	else {
								 //nothing
		retval= string ("nothing");
	}
	return retval;
}


float Unit::CourseDeviation (const Vector &OriginalCourse, const Vector &FinalCourse) const
{
	if (ViewComputerData().max_ab_speed()>.001)
		return ((OriginalCourse-(FinalCourse)).Magnitude()/ViewComputerData().max_ab_speed());
	else
		return (FinalCourse-OriginalCourse).Magnitude();
}


/***************************************************************************************/
/*** STAR SYSTEM JUMP STUFF                                                          ***/
/***************************************************************************************/

bool Unit::TransferUnitToSystem (StarSystem * Current)
{
	if (getStarSystem()->RemoveUnit (this)) {
		this->RemoveFromSystem();
		this->Target(NULL);
		Current->AddUnit (this);

		Cockpit * an_active_cockpit = _Universe->isPlayerStarship(this);
		if (an_active_cockpit!=NULL) {
			an_active_cockpit->activeStarSystem=Current;
			an_active_cockpit->visitSystem (Current->getFileName());
			//	vector<float> *v = &an_active_cockpit->savegame->getMissionData(string("visited_")+pendingjump[kk]->dest->getFileName());
			//	if (v->empty())v->push_back (1.0);else (*v)[0]=1.0;
		}

		activeStarSystem = Current;
		return true;
	}
	else {
		UNIT_LOG(logvs::ERROR, "Fatal Error: cannot remove starship from critical system");
	}
	return false;
}


/***************************************************************************************/
/*** UNIT_REPAIR STUFF                                                               ***/
/***************************************************************************************/
extern float rand01();
bool isWeapon (const std::string & name)
{
	if (name.find("Weapon")!=std::string::npos) {
		return true;
	}
	if (name.find("SubUnit")!=std::string::npos) {
		return true;
	}
	if (name.find("Ammunition")!=std::string::npos) {
		return true;
	}
	return false;
}


void Unit::Repair()
{
	//note work slows down under time compression!

	static float repairtime=XMLSupport::parse_float(vs_config->getVariable ("physics","RepairDroidTime","180"));
	static float checktime =XMLSupport::parse_float(vs_config->getVariable ("physics","RepairDroidCheckTime","5"));
	static bool  continuous=XMLSupport::parse_bool (vs_config->getVariable ("physics","RepairDroidContinuous","true"));
	if ((repairtime<=0)||(checktime<=0)) return;

	/*
	if (image->next_repair_time==FLT_MAX)
	  image->next_repair_time = UniverseUtil::GetGameTime();
	*/

	//float workunit = image->repair_droid*SIMULATION_ATOM/repairtime;//a droid completes 1 work unit in repairtime
	//if (image->repair_droid&&vsrandom.uniformInc(0,1)<workunit) {
	if (image->repair_droid) {
		if (image->next_repair_time==-FLT_MAX||image->next_repair_time<=UniverseUtil::GetGameTime()) {
			unsigned int numcargo=numCargo();
			if (numcargo>0) {
				if (image->next_repair_cargo>=numCargo()) {
					image->next_repair_cargo=0;
				}
				Cargo *carg = &GetCargo(image->next_repair_cargo);
				float percentoperational=1;
				if (carg->GetCategory().find("upgrades/")==0
					&&carg->GetCategory().find(DamagedCategory)!=0
					&&carg->GetContent().find("add_")!=0
					&&carg->GetContent().find("mult_")!=0
				&&((percentoperational=UnitUtil::PercentOperational(this,carg->content,carg->category,true))<1.f)) {
					if (image->next_repair_time==-FLT_MAX) {
						image->next_repair_time=UniverseUtil::GetGameTime()+repairtime*(1-percentoperational);
					}
					else {
						//ACtually fix the cargo here
						static int upfac = FactionUtil::GetUpgradeFaction();
						const Unit * up=getUnitFromUpgradeName(carg->content,upfac);
						static std::string loadfailed("LOAD_FAILED");
						if (up->name == loadfailed) {
							UNIT_LOG(logvs::WARN, "Bug: Load failed cargo encountered: report to hellcatv@hotmail.com");
						}
						else {
							double percentage=0;
								 //don't want to repair these things
							if (up->SubUnits.empty()&&up->GetNumMounts()==0) {
								this->Upgrade(up,0,0,0,true,percentage,makeTemplateUpgrade(this->name,this->faction),false,false);
								if (percentage==0) {
									UNIT_LOG(logvs::WARN, "Failed repair for unit %s, cargo item %d: %s (%s) - please report error",name.get().c_str(),image->next_repair_cargo,carg->GetContent().c_str(),carg->GetCategory().c_str());
								}
							}
						}
						image->next_repair_time=-FLT_MAX;
						++(image->next_repair_cargo);
					}
				}
				else {
					++(image->next_repair_cargo);
				}

			}

		}
		float ammt_repair = SIMULATION_ATOM/repairtime;

#define REPAIRINTEGRATED(functionality,max_functionality) \
			if (functionality<max_functionality) \
			{ \
				(functionality)+=ammt_repair; \
				if ((functionality)>(max_functionality)) \
				(functionality) = (max_functionality); \
			}
				REPAIRINTEGRATED(image->LifeSupportFunctionality,image->LifeSupportFunctionalityMax);
		REPAIRINTEGRATED(image->fireControlFunctionality,image->fireControlFunctionalityMax);
		REPAIRINTEGRATED(image->SPECDriveFunctionality,image->SPECDriveFunctionalityMax);
		REPAIRINTEGRATED(image->CommFunctionality,image->CommFunctionalityMax);
#undef REPAIRINTEGRATED

		unsigned int numg=(1+UnitImages::NUMGAUGES+MAXVDUS);
		unsigned int which= vsrandom.genrand_int31()%numg;
		static float hud_repair_quantity=XMLSupport::parse_float(vs_config->getVariable("physics","hud_repair_unit",".25"));

								 //total damage
		if (image->cockpit_damage[which]<image->cockpit_damage[which+numg]) {
			image->cockpit_damage[which]+=hud_repair_quantity;
			if (image->cockpit_damage[which]>image->cockpit_damage[which+numg]) {
								 //total damage
				image->cockpit_damage[which]=image->cockpit_damage[which+numg];
			}
		}
		if (mounts.size()) {
			static float mount_repair_quantity=XMLSupport::parse_float(vs_config->getVariable("physics","mount_repair_unit",".25"));
			unsigned int i= vsrandom.genrand_int31()%mounts.size();
			if (mounts[i].functionality<mounts[i].maxfunctionality) {
				mounts[i].functionality+=mount_repair_quantity;
				if (mounts[i].functionality>mounts[i].maxfunctionality) {
					mounts[i].functionality=mounts[i].maxfunctionality;
				}
			}
		}
	}
}


bool Unit::isTractorable(enum tractorHow how) const
{
	if (how!=tractorImmune)
		return ((getTractorability() & how) == how); else
		return getTractorability() == tractorImmune;
}


void Unit::setTractorability(enum tractorHow how)
{
	tractorability_flags = how;
}


enum Unit::tractorHow Unit::getTractorability() const
{
	static bool tractorability_mask_init=false;
	static unsigned char tractorability_mask;

	if (!tractorability_mask_init) {
		std::string stractorability_mask = vs_config->getVariable("physics","PlayerTractorabilityMask","p");
		if (!stractorability_mask.empty()) {
			tractorability_mask = tractorImmune;
			if (stractorability_mask.find_first_of("pP")!=string::npos)
				tractorability_mask |= tractorPush;
			if (stractorability_mask.find_first_of("iI")!=string::npos)
				tractorability_mask |= tractorIn;
		} else tractorability_mask = tractorPush;
		tractorability_mask_init = true;
	}

	unsigned char tflags;
	if (_Universe->isPlayerStarship(this)!=NULL)
		tflags = tractorability_flags&tractorability_mask; else
		tflags = tractorability_flags;

	return (Unit::tractorHow)(tflags);
}


void Unit::RequestPhysics()
{
	// Request ASAP physics
	if (getStarSystem())
		getStarSystem()->RequestPhysics(this,cur_sim_queue_slot);
}
