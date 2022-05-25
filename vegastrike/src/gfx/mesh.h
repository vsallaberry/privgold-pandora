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
#ifndef _MESH_H_
#define _MESH_H_

#include <string>
#include <vector>
#include "xml_support.h"
#include "quaternion.h"
#include "matrix.h"
#include "gfxlib.h"
#include "gfxlib_struct.h"
#include "vsfilesystem.h"
#include "cmd/unit_generic.h"
#include "gfx/technique.h"

class Planet;
class Unit;
class Logo;
class AnimatedTexture;
class Texture;
struct GFXVertex;
class GFXVertexList;
class GFXQuadstrip;
struct GFXMaterial;
class BoundingBox;

#define MESH_HASTHABLE_SIZE 503

///Struct vertex format returned by GetPolys
///Struct polygon format returned by GetPolys
struct bsp_polygon {
    std::vector <Vector> v;
};
/**
 * Mesh FX stores various lights that light up shield or hull for damage
 * They may be merged and they grow and shring based on their TTL and TTD and delta values
 * They must be updated every frame or every physics frame if not drawn (pass in time 
 */
class MeshFX : public GFXLight {
public:
  ///The ammt of change that such meshFX objects attenuation get
  float delta;
  ///The Time to live of the current light effect
  float TTL;
  ///After it has achieved its time to live max it has to slowly fade out and die
  float TTD;
  MeshFX ():GFXLight(){TTL=TTD=delta=0;}
  ///Makes a meshFX given TTL and delta values.
  MeshFX (const float TTL, const float delta, const bool enabled, const GFXColor &vect, const GFXColor &diffuse= GFXColor (0,0,0,1), const GFXColor &specular=GFXColor (0,0,0,1), const GFXColor &ambient=GFXColor(0,0,0,1), const GFXColor&attenuate=GFXColor(1,0,0));
  ///Merges two MeshFX in a given way to seamlessly blend multiple hits on a shield
  void MergeLights (const MeshFX & other);
  ///updates the growth and death of the FX. Returns false if dead
  bool Update (float ttime);//if false::dead
};
/**
 * Stores relevant info needed to draw a mesh given only the orig
 */
struct MeshDrawContext {
  ///The matrix in world space
  Matrix mat;
  ///The special FX vector pointing to all active special FX
  std::vector <MeshFX> *SpecialFX;
  GFXColor CloakFX;
  enum CLK {NONE=0x0,CLOAK=0x1,FOG=0x2, NEARINVIS=0x4, GLASSCLOAK=0x8, RENORMALIZE=0x10};
  char cloaked;
  char mesh_seq;
  unsigned char damage;//0 is perfect 255 is dead
  MeshDrawContext(const Matrix & m):mat(m),CloakFX(1,1,1,1),cloaked(NONE),damage(0)
  { }
};
using XMLSupport::EnumMap;
using XMLSupport::AttributeList;

#define NUM_MESH_SEQUENCE 5
#define NUM_ZBUF_SEQ 4
#define MESH_SPECIAL_FX_ONLY 3

/**
 * Mesh is the basic textured drawable
 * Mesh has 1 texture and 1 vertex list (with possibly multiple primitives inside
 * Meshes have a center-location but do not need to be translated to be drawn
 * Meshes store various LOD's and originals in the orig pointer. These may be accessed
 * in order to draw quickly a whole series of meshes.
 * Unless DrawNow is invoked, Drawing only stores the mesh on teh appropriate draw queue so 
 * they may be drawn at a later date
 * Also meshe contain Logos, flags based on squadron and faction that may be user-edited and appear in pleasing
 * places on the hull.
 */
class Mesh
{
private:
    //make sure to only use TempGetTexture when xml-> is valid \|/
    Texture * TempGetTexture (struct MeshXML * , int index, const std::string & factionname) const;
	Texture * TempGetTexture (struct MeshXML *, const std::string & filename, const std::string & factionname,GFXBOOL detail) const;
  ///Stores all the load-time vertex info in the XML struct FIXME light calculations
  ///Loads XML data into this mesh.
  void LoadXML(const char *filename, const Vector & scale, int faction, class Flightgroup * fg, bool orig,const std::vector<std::string> &overrideTexture);
  void LoadXML(VSFileSystem::VSFile & f, const Vector & scale, int faction, class Flightgroup * fg, bool orig, const std::vector<std::string> &overrideTexture);
  ///loads binary data into this mesh
  void LoadBinary (const char * filename, int faction);
  ///Creates all logos with given XML data info
  void CreateLogos(struct MeshXML *, int faction, class Flightgroup *fg);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  
  void beginElement(struct MeshXML *xml, const std::string &name, const AttributeList &attributes);
  void endElement(struct MeshXML *xml, const std::string &name);
  
protected:
  void PostProcessLoading(struct MeshXML *xml,const std::vector<std::string> &overrideTexture);
  void initTechnique(const std::string &technique);
  
private:
  Mesh( const char *filename, const Vector & scalex,int faction,class Flightgroup * fg, bool orig, const std::vector<std::string> &textureOverride=std::vector<std::string>());

protected:
  // only may be called from subclass. orig request may be denied if unit was in past usage. (not likely in the case where a unit must be constructed in orig)
  Mesh( const std::string & filename, const Vector & scalex,int faction,class Flightgroup * fg, bool orig=false);

  ///Loads a mesh that has been found in the hash table into this mesh (copying original data)
  bool LoadExistant (Mesh *mesh);
  bool LoadExistant (const std::string & filehash, const Vector & scale, int faction);
  ///the position of the center of this mesh for collision detection
  Vector local_pos; 
  ///The hash table of all meshes
  static Hashtable<std::string, Mesh, MESH_HASTHABLE_SIZE> meshHashTable;
  static Hashtable<std::string, std::vector<int>,MESH_HASTHABLE_SIZE> animationSequences;
  ///The refcount:: how many meshes are referencing the appropriate original
  int refcount;
  ///bounding box
  Vector mx;  Vector mn;
  ///The radial size of this mesh
  float radialSize;
  ///num lods contained in the array of Mesh "orig"
  int numlods;
  float framespersecond;//for animation
  Mesh *orig;
  ///The size that this LOD (if original) comes into effect
  float lodsize;
  ///The number of force logos on this mesh (original)
  Logo *forcelogos; int numforcelogo; 
  ///The number of squad logos on this mesh (original)
  Logo *squadlogos;  int numsquadlogo;
  ///tri,quad,line, strips, etc
  GFXVertexList *vlist;
  ///The number of the appropriate material for this mesh (default 0)
  unsigned int myMatNum;
  ///The technique used to render this mesh
  TechniquePtr technique;
  ///The decal relevant to this mesh
  std::vector <Texture *> Decal;
  Texture * detailTexture;
  std::vector <Vector> detailPlanes;
  float polygon_offset;
  ///whether this should be environment mapped 0x1 and 0x2 for if it should be lit (ored together)
  char envMapAndLit;
  ///Whether this original will be drawn this frame
  GFXBOOL will_be_drawn;  
  ///The blend functions
  bool convex;
  unsigned char alphatest;
  enum BLENDFUNC blendSrc;  enum BLENDFUNC blendDst;

  /// Support for reorganized rendering
  std::vector<MeshDrawContext> *draw_queue;
  /// How transparent this mesh is (in what order should it be rendered in 
  int draw_sequence;
  ///The name of this unit
  std::string hash_name;
  ///Setting all values to defaults (good for mesh copying and stuff)
  void InitUnit();
  ///Needs to have access to our class
  friend class OrigMeshContainer;
  ///The enabled light effects on this mesh
  std::vector <MeshFX> LocalFX;
  ///Returing the mesh relevant to "size" pixels LOD of this mesh
  Mesh *getLOD (float lod, bool bBypassDamping=true);

private:
  ///Implement fixed-function draw queue processing (the referenced pass must be of Fixed type) - internal usage
  void ProcessFixedDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr);
  
  ///Implement programmable draw queue processing (the referenced pass must be of Shader type) - internal usage
  void ProcessShaderDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr);
  
  ///Activate a texture unit - internal usage
  void activateTextureUnit(const Technique::Pass::TextureUnit &tu, bool deflt = false);

public:
  Mesh();
  Mesh(const Mesh &m);
  
  float getFramesPerSecond()const;
  float getCurrentFrame() const;
  void setCurrentFrame(float);
  int getNumAnimationFrames(const std::string & which="")const;
  int getNumLOD()const;
  int getNumTextureFrames();
  float getTextureFramesPerSecond();
  double getTextureCumulativeTime();
  void setTextureCumulativeTime(double );
  bool getConvex()const{return this->convex;}
  void setConvex(bool b);
  virtual int MeshType() {return 0;}
  BLENDFUNC getBlendSrc() {return blendSrc;}
  BLENDFUNC getBlendDst() {return blendDst;}		
  ///Loading a mesh from an XML file.  faction specifies the logos.  Orig is for internal (LOD) use only!
//private:  
//public:
	static Mesh* LoadMesh(const char * filename, const Vector & scalex, int faction, class Flightgroup * fg, const std::vector<std::string> &textureOverride=std::vector<std::string>());
  static std::vector<Mesh*> LoadMeshes(const char * filename, const Vector & scalex, int faction, class Flightgroup * fg, const std::vector<std::string> &textureOverride=std::vector<std::string>());
  static std::vector<Mesh*> LoadMeshes(VSFileSystem::VSFile & f, const Vector & scalex, int faction, class Flightgroup * fg, const std::string & hash_name, const std::vector<std::string> &textureOverride=std::vector<std::string>());

  ///Forks the mesh across the plane a,b,c,d into two separate meshes...upon which this may be deleted
  void Fork (Mesh * &one, Mesh * &two, float a, float b, float c, float d);
  ///Destructor... kills orig if refcount of orig becomes zero
  virtual ~Mesh();
  ///Gets number of specialFX
  unsigned int numFX () {return LocalFX.size();}
  ///Turns on SpecialFX
  void EnableSpecialFX();
  unsigned int numTextures() {return Decal.size();}
  Texture * texture (int i) {return Decal[i];}
  void SetBlendMode (BLENDFUNC src, BLENDFUNC dst, bool lodcascade=false);
  ///Gets all polygons in this mesh for BSP computation
  void GetPolys(std::vector <bsp_polygon> &);
  ///Sets the material of this mesh to mat (affects original as well)
  void SetMaterial (const GFXMaterial & mat);
  // Gets the material back from the mesh.
  const GFXMaterial &GetMaterial ();
  ///If it has already been drawn this frame
  GFXBOOL HasBeenDrawn() {return will_be_drawn;} 
  ///so one can query if it has or not been drawn
  void UnDraw() {will_be_drawn=GFXFALSE;}
  ///Returns center of this mesh
  Vector &Position() {return local_pos;}
  ///Draws lod pixel wide mesh at Transformation LATER
  void Draw(float lod, const Matrix &m = identity_matrix, float toofar=1, int cloak=-1, float nebdist=0, unsigned char damage=0,bool renormalize_normals=false); //short fix
  ///Draws lod pixels wide, mesh at Transformation NOW. If centered, then will center on camera and disable cull
  void DrawNow(float lod, bool centered, const Matrix &m= identity_matrix, int cloak=-1,float nebdist=0); //short fix
  ///Will draw all undrawn meshes of this type
  virtual void ProcessDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr);
  ///Will draw all undrawn far meshes beyond the range of zbuffer (better be convex).
  virtual void SelectCullFace (int whichdrawqueue);
  virtual void RestoreCullFace (int whichdrawqueue);
  static void ProcessZFarMeshes (bool nocamerasetup=false);
  ///Will draw all undrawn meshes in total If pushSpclFX, the last series of meshes will be drawn with other lighting off
  static void ProcessUndrawnMeshes(bool pushSpecialEffects=false,bool nocamerasetup=false);
  ///Sets whether or not this unit should be environment mapped
  void forceCullFace (GFXBOOL newValue) {if (newValue) envMapAndLit = ((envMapAndLit&~0x8)|0x4); if (!newValue) envMapAndLit = ((envMapAndLit&~0x4)|0x8);}
  GFXBOOL getCullFaceForcedOn() {return ((envMapAndLit&0x4)!=0);}
  GFXBOOL getCullFaceForcedOff() {return ((envMapAndLit&0x8)!=0);}
  void setEnvMap(GFXBOOL newValue, bool lodcascade=false) {envMapAndLit = (newValue?(envMapAndLit|0x1):(envMapAndLit&(~0x1))); if (lodcascade&&orig) for (int i=0; i<numlods; i++) orig[i].setEnvMap(newValue); }
  GFXBOOL getEnvMap() {return ((envMapAndLit&0x1)!=0);}
  void setLighting(GFXBOOL newValue, bool lodcascade=false){envMapAndLit = (newValue?(envMapAndLit|0x2):(envMapAndLit&(~0x2))); if (lodcascade&&orig) for (int i=0; i<numlods; i++) orig[i].setLighting(newValue); }
  GFXBOOL getLighting() {return ((envMapAndLit&0x2)!=0);}
  ///Returns bounding box values
  Vector corner_min() { return mn; }  Vector corner_max() { return mx; }
  ///Returns a physical boudning box in 3space instead of in current unit space
  BoundingBox * getBoundingBox();
  ///queries this bounding box with a vector and radius
  bool queryBoundingBox (const QVector &start,const float err);
  ///Queries bounding box with a ray
  bool queryBoundingBox (const QVector &start, const QVector & end, const float err); 
  ///returns the radial size of this 
  float rSize () {return radialSize;}
  virtual float clipRadialSize() {return radialSize;}
  ///based on TTL, etc, updates shield effects
  void UpdateFX(float ttime);
  ///Adds a new damage effect with %age damage to the part of the unit. Color specifies the shield oclor
  void AddDamageFX (const Vector &LocalPos, const Vector &LocalNorm, const float percentage, const GFXColor &color=GFXColor (1,1,1,1)); 
  void setVirtualBoundingBox(const Vector & mn, const Vector &mx, float rsize) {
        radialSize=rsize;
        this->mn=mn;
		this->mx=mx;
  }
};
#endif

