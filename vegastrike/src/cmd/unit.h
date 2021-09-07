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
#ifndef _GAMEUNIT_H_
#define _GAMEUNIT_H_
#ifdef VS_DEBUG
#define CONTAINER_DEBUG
#endif
struct GFXColor;

#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include <string>
//#include "gfx/vdu.h"
#include "gfx/cockpit.h"
#include "gfx/halo_system.h"
#include "script/flightgroup.h"
#include "unit_generic.h"
using std::string;
#include "gfxlib.h"

/**
 * GameUnit contains any physical object that may collide with something
 * And may be physically affected by forces.
 * Units are assumed to have various damage and explode when they are dead.
 * Units may have any number of weapons which, themselves may be units
 * the aistate indicates how the unit will behave in the upcoming phys frame
 */
template <class UnitType> class GameUnit: public UnitType
{ 
public:

  UnitImages &GetImageInformation();

  bool RequestClearance (Unit * dockingunit);

  ///Loads a user interface for the user to upgrade his ship
  void UpgradeInterface (Unit * base);
  ///The name (type) of this unit shouldn't be public

protected:

/***************************************************************************************/
/**** CONSTRUCTORS / DESCTRUCTOR                                                    ****/
/***************************************************************************************/
  /** Default constructor. This is just to figure out where default
   *  constructors are used. The useless argument will be removed
   *  again later.
   */
  GameUnit( int dummy );

  /** Constructor that creates aa mesh with meshes as submeshes (number
   *  of them) as either as subunit with faction faction
   */
  GameUnit (std::vector <Mesh *> &meshes  , bool Subunit, int faction);

  /** Constructor that creates a mesh from an XML file If it is a
   *  customizedUnit, it will check in that directory in the home dir for
   *  the unit.
   */
  GameUnit( const char *filename,
        bool        SubUnit,
       int         faction,
       std::string customizedUnit=string(""),
       Flightgroup *flightgroup=NULL,
       int         fg_subnumber=0, string * netxml=NULL);

  ~GameUnit();

  friend class UnitFactory;

public:
  virtual void Cloak(bool cloak);
/***************************************************************************************/
/**** GFX/MESHES STUFF                                                              ****/
/***************************************************************************************/

public:
  HaloSystem halos;
  double sparkle_accum;

protected:
  int nummesh()const {return ((int)this->meshdata.size())-1;}

public:

	
  //vector <Mesh *> StealMeshes();
  ///Process all meshes to be deleted
  ///Split this mesh with into 2^level submeshes at arbitrary planes
  void Split (int level);
  void FixGauges();
  ///Sets the camera to be within this unit.
  void UpdateHudMatrix(int whichcam);
  ///What's the HudImage of this unit
  VSSprite * getHudImage ()const ;
  ///Draws this unit with the transformation and matrix (should be equiv) separately
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix &m = identity_matrix);
  virtual void DrawNow(const Matrix &m = identity_matrix, float lod=1000000000);
  ///Deprecated
  //virtual void ProcessDrawQueue() {}
  void addHalo( const char * filename, const QVector &loc, const Vector &size, const GFXColor & col, std::string halo_type,float halo_speed)
  {
	 halos.AddHalo (filename,loc,size,col,halo_type,halo_speed);
  }

/***************************************************************************************/
/**** STAR SYSTEM STUFF                                                             ****/
/***************************************************************************************/

public:
  void SetPlanetOrbitData (PlanetaryTransform *trans);
  PlanetaryTransform *GetPlanetOrbit () const;
  //bool TransferUnitToSystem (StarSystem *NewSystem);
  bool TransferUnitToSystem (unsigned int whichJumpQueue, class StarSystem *&previouslyActiveStarSystem, bool DoSightAndSound);
  ///Begin and continue explosion
  bool Explode(bool draw, float timeit);

/***************************************************************************************/
/**** COLLISION STUFF                                                               ****/
/***************************************************************************************/

protected:
  ///fils in corner_min,corner_max and radial_size
  //void calculate_extent(bool update_collide_queue);
  ///returns -1 if unit cannot dock, otherwise returns which dock it can dock at

public:
  ///Updates the collide Queue with any possible change in sectors
  ///Queries if this unit is within a given frustum
  bool queryFrustum (double frustum[6][4]) const;

  /**
   *Queries bounding box with a point, radius err
   */
  bool queryBoundingBox (const QVector &pnt, float err);
  /**
   *Queries the bounding box with a ray.  1 if ray hits in front... -1 if ray
   * hits behind.
   * 0 if ray misses 
   */
  int queryBoundingBox(const QVector &origin,const Vector &direction, float err);
  /**Queries the bounding sphere with a duo of mouse coordinates that project
   * to the center of a ship and compare with a sphere...pretty fast*/
  bool querySphereClickList (int,int, float err, Camera *activeCam);
  //virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
  ///returns true if jump possible even if not taken
  //bool jumpReactToCollision (Unit *smaller);

/***************************************************************************************/
/**** PHYSICS STUFF                                                                    */
/***************************************************************************************/

  //bool AutoPilotTo(Unit * un, bool ignore_friendlies=false);

  ///Updates physics given unit space transformations and if this is the last physics frame in the current gfx frame
  virtual void UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  //class Cockpit * GetVelocityDifficultyMult(float &) const;
  ///Thrusts by ammt and clamps accordingly (afterburn or not)
  void Thrust(const Vector &amt,bool afterburn = false);
  ///Resolves forces of given unit on a physics frame
  Vector ResolveForces (const Transformation &, const Matrix&);
  //these functions play the damage sounds
  virtual void ArmorDamageSound( const Vector &pnt);
  virtual void HullDamageSound( const Vector &pnt);
  ///applies damage from the given pnt to the shield, and returns % damage applied and applies lighitn
  float DealDamageToShield (const Vector & pnt, float &Damage);

/***************************************************************************************/
/**** CUSTOMIZE/UPGRADE STUFF                                                          */
/***************************************************************************************/

  bool UpgradeSubUnits (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage);
  double Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts);


/********************************************/
/**** XML struct                            */
/********************************************/

  ///Holds temporary values for inter-function XML communication Saves deprecated restr info
  /*
	struct XML {
	  vector<Mount *> mountz;
	  vector<Mesh*> meshes;
	  vectorstring> meshes_str;
	  Mesh * shieldmesh;
	  Mesh * bspmesh;
	  Mesh * rapidmesh;
	  void * data;
	  vector<Unit*> units;
	  int unitlevel;
	  bool hasBSP;
	  bool hasColTree;
	  enum restr {YRESTR=1, PRESTR=2, RRESTR=4};
	  const char * unitModifications;
	  char yprrestricted;
	  float unitscale;
	  float ymin, ymax, ycur;
	  float pmin, pmax, pcur;
	  float rmin, rmax, rcur;
	  std::string cargo_category;
	  std::string hudimage;
	  int damageiterator;
	};
	*/
	inline Matrix WarpMatrix ( const Matrix& ctm) const{
		static float cutoff =XMLSupport::parse_float (vs_config->getVariable( "graphics","warp_stretch_cutoff","50000"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
		static float cutoffcutoff=  cutoff*cutoff;
                static bool only_stretch_in_warp=XMLSupport::parse_bool(vs_config->getVariable("graphics","only_stretch_in_warp","true"));
		if (this->GetWarpVelocity().MagnitudeSquared() < cutoffcutoff||(only_stretch_in_warp&&this->graphicOptions.InWarp==0)) {
			return ctm;
		}else {
			Matrix k(ctm);
			
			float speed = this->GetWarpVelocity().Magnitude();
			//Matrix scalar=identity_matrix;
			static float maxregion0stretch = XMLSupport::parse_float (vs_config->getVariable("graphics","warp_stretch_region0_max","1"));
			
			static float maxstretch = XMLSupport::parse_float (vs_config->getVariable("graphics","warp_stretch_max","4"));
			static float maxspeed = XMLSupport::parse_float (vs_config->getVariable("graphics","warp_stretch_max_speed","1000000"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
			static float maxregion0speed = XMLSupport::parse_float (vs_config->getVariable("graphics","warp_stretch_max_region0_speed","100000"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
			float stretchregion0length = maxregion0stretch*(speed-cutoff)/(maxregion0speed-cutoff);
			float stretchlength = (maxstretch-maxregion0stretch)*(speed-maxregion0speed)/(maxspeed-maxregion0speed+.06125)+maxregion0stretch;
			
			if (stretchlength>maxstretch)
				stretchlength= maxstretch;
                        if (stretchregion0length>maxregion0stretch)
                          stretchregion0length= maxregion0stretch;
			ScaleMatrix(k,Vector(1,1,1+(speed>maxregion0speed?stretchlength:stretchregion0length)));			
			
/*			Vector v(Vector(1,1,1)+ctm.getR().Scale(stretchlength).Vabs());
			
			k.r[0]*=v.i;
			k.r[1]*=v.j;
			k.r[2]*=v.k;
			
			k.r[3]*=v.i;
			k.r[4]*=v.j;
			k.r[5]*=v.k;
			
			k.r[6]*=v.i;
			k.r[7]*=v.j;
			k.r[8]*=v.k;		*/
			return k;
		}
	}

};

/****************************************************************************/
/**** Since we now use templates, we have to include GameUnit related    ****/
/**** .cpp files in unit.h to avoid problems with some compilers         ****/
/****************************************************************************/
#include "unit.cpp"


#endif
