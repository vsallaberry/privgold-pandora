ACLOCAL_AMFLAGS = -I m4scripts




#BOOST_BEGIN -- Data between the BOOST_AUTO and BOOST_END tags auto generated.
if !BOOSTSYSTEM
BOOSTDIR=boost
if BOOST1_28
BOOSTSOURCES = \
	boost/1_28/src/module.cpp \
	boost/1_28/src/cross_module.cpp \
	boost/1_28/src/types.cpp \
	boost/1_28/src/extension_class.cpp \
	boost/1_28/src/errors.cpp \
	boost/1_28/src/functions.cpp \
	boost/1_28/src/init_function.cpp \
	boost/1_28/src/module_builder.cpp \
	boost/1_28/src/objects.cpp \
	boost/1_28/src/object/life_support.cpp \
	boost/1_28/src/object/function.cpp \
	boost/1_28/src/converter/callback.cpp \
	boost/1_28/src/classes.cpp \
	boost/1_28/src/conversions.cpp
else
if BOOST1_35
BOOSTSOURCES_CXXFLAGS = -DBOOST_PYTHON_NO_PY_SIGNATURES -DBOOST_PYTHON_STATIC_LIB
BOOSTSOURCES = \
  boost/1_35/src/list.cpp \
  boost/1_35/src/module.cpp \
  boost/1_35/src/object_protocol.cpp \
  boost/1_35/src/dict.cpp \
  boost/1_35/src/numeric.cpp \
  boost/1_35/src/errors.cpp \
  boost/1_35/src/exec.cpp \
  boost/1_35/src/str.cpp \
  boost/1_35/src/import.cpp \
  boost/1_35/src/wrapper.cpp \
  boost/1_35/src/object_operators.cpp \
  boost/1_35/src/long.cpp \
  boost/1_35/src/object/pickle_support.cpp \
  boost/1_35/src/object/life_support.cpp \
  boost/1_35/src/object/inheritance.cpp \
  boost/1_35/src/object/function_doc_signature.cpp \
  boost/1_35/src/object/stl_iterator.cpp \
  boost/1_35/src/object/class.cpp \
  boost/1_35/src/object/enum.cpp \
  boost/1_35/src/object/function.cpp \
  boost/1_35/src/object/iterator.cpp \
  boost/1_35/src/converter/from_python.cpp \
  boost/1_35/src/converter/registry.cpp \
  boost/1_35/src/converter/arg_to_python_base.cpp \
  boost/1_35/src/converter/builtin_converters.cpp \
  boost/1_35/src/converter/type_id.cpp \
  boost/1_35/src/tuple.cpp
else
if BOOST1_50
BOOSTSOURCES_CXXFLAGS = -DBOOST_PYTHON_NO_PY_SIGNATURES -DBOOST_PYTHON_STATIC_LIB
BOOSTSOURCES = \
  boost/1_50/src/list.cpp \
  boost/1_50/src/module.cpp \
  boost/1_50/src/object_protocol.cpp \
  boost/1_50/src/dict.cpp \
  boost/1_50/src/numeric.cpp \
  boost/1_50/src/errors.cpp \
  boost/1_50/src/exec.cpp \
  boost/1_50/src/str.cpp \
  boost/1_50/src/import.cpp \
  boost/1_50/src/wrapper.cpp \
  boost/1_50/src/object_operators.cpp \
  boost/1_50/src/long.cpp \
  boost/1_50/src/object/pickle_support.cpp \
  boost/1_50/src/object/life_support.cpp \
  boost/1_50/src/object/inheritance.cpp \
  boost/1_50/src/object/function_doc_signature.cpp \
  boost/1_50/src/object/stl_iterator.cpp \
  boost/1_50/src/object/class.cpp \
  boost/1_50/src/object/enum.cpp \
  boost/1_50/src/object/function.cpp \
  boost/1_50/src/object/iterator.cpp \
  boost/1_50/src/converter/from_python.cpp \
  boost/1_50/src/converter/registry.cpp \
  boost/1_50/src/converter/arg_to_python_base.cpp \
  boost/1_50/src/converter/builtin_converters.cpp \
  boost/1_50/src/converter/type_id.cpp \
  boost/1_50/src/tuple.cpp
else
BOOSTSOURCES=
endif
endif
endif

#This was in the acct server and networking library stuff
#if MACOSX
#incl2 = \
#	 -I/System/Library/Frameworks/QuickTime.framework/Headers \
#	 -I/System/Library/Frameworks/Carbon.framework/Versions/A/Headers
#endif

endif

PRECOMPILED = src/precompiled.h

EXTRA_DIST = \
  src/cmd/unit.cpp \
  src/cmd/unit_click.cpp \
  src/cmd/unit_customize.cpp \
  src/cmd/unit_damage.cpp \
  src/cmd/unit_jump.cpp \
  src/cmd/unit_physics.cpp \
  vc98/vegastrike.dsp \
  vc98/vegastrike.dsw \
  README \
  README.MACOSX.rtf \
  README.sgi \
  NEWS \
  ToDo.txt \
  Resources.rc \
  resource.h \
  ts.ico \
  ChangeLog \
  AUTHORS \
  vegastrike.spec \
  bootstrap-sh \
  launcher/launcher.rc \
  launcher/resource.h \
  launcher/icon1.ico \
  launcher/icon2.ico \
  src/cmd/script/c_alike/c_alike.lex \
  src/cmd/script/c_alike/c_alike.yacc

#include src/Makefile

LIBNET = \
  src/networking/inet_file.cpp \
  src/networking/inet_file.h \
  src/networking/inet.cpp \
  src/networking/inet.h

libacctserver = \
  src/networking/accountsxml.cpp \
  src/networking/accountsxml.h \
  src/networking/acctserver.h \
  src/networking/client.cpp \
  src/networking/client.h \
  src/networking/const.h \
  src/networking/fileutil.cpp \
  src/networking/fileutil.h \
  src/networking/savenet_util.cpp \
  src/networking/savenet_util.h

libnetgeneric = \
  $(libacctserver) \
  src/networking/cubicsplines.cpp \
  src/networking/cubicsplines.h \
  src/networking/mangle.cpp \
  src/networking/mangle.h \
  src/networking/netclient_clients.cpp \
  src/networking/netclient_devices.cpp \
  src/networking/netclient_login.cpp \
  src/networking/netclient.cpp \
  src/networking/netclient.h \
  src/networking/netserver_acct.cpp \
  src/networking/netserver_clients.cpp \
  src/networking/netserver_devices.cpp \
  src/networking/netserver_login.cpp \
  src/networking/netserver_net.cpp \
  src/networking/netserver.cpp \
  src/networking/netserver.h \
  src/networking/prediction.cpp \
  src/networking/prediction.h \
  src/networking/zonemgr.cpp \
  src/networking/zonemgr.h

libnetclientserver = \
  $(libnetgeneric)

LIBNETCLIENT = \
  $(libnetclientserver) \
  src/networking/networkcomm.cpp \
  src/networking/networkcomm.h \
  src/networking/webcam_support.h \
  src/networking/webcam_support.cpp

LIBNETSERVER = \
  $(libnetclientserver) \
  src/networking/networkcomm_server.cpp

LIBGUI = \
  src/gui/button.cpp \
  src/gui/button.h \
  src/gui/control.cpp \
  src/gui/control.h \
  src/gui/eventmanager.cpp \
  src/gui/eventmanager.h \
  src/gui/eventresponder.cpp \
  src/gui/eventresponder.h \
  src/gui/font.cpp \
  src/gui/font.h \
  src/gui/general.cpp \
  src/gui/general.h \
  src/gui/glut_support.cpp \
  src/gui/glut_support.h \
  src/gui/groupcontrol.cpp \
  src/gui/groupcontrol.h \
  src/gui/guidefs.cpp \
  src/gui/guidefs.h \
  src/gui/guitexture.cpp \
  src/gui/guitexture.h  \
  src/gui/modaldialog.cpp \
  src/gui/modaldialog.h \
  src/gui/newbutton.cpp \
  src/gui/newbutton.h \
  src/gui/painttext.cpp \
  src/gui/painttext.h \
  src/gui/picker.cpp \
  src/gui/picker.h \
  src/gui/scroller.cpp \
  src/gui/scroller.h \
  src/gui/simplepicker.cpp \
  src/gui/simplepicker.h \
  src/gui/slider.cpp \
  src/gui/slider.h \
  src/gui/staticdisplay.cpp \
  src/gui/staticdisplay.h \
  src/gui/text_area.cpp \
  src/gui/text_area.h \
  src/gui/textinputdisplay.cpp \
  src/gui/textinputdisplay.h \
  src/gui/window.cpp \
  src/gui/window.h \
  src/gui/windowcontroller.cpp \
  src/gui/windowcontroller.h

LIBPYTHON = \
  src/python/define_odd_unit_functions.h \
  src/python/init.cpp \
  src/python/init.h \
  src/python/python_class.h \
  src/python/python_compile.cpp \
  src/python/python_compile.h \
  src/python/python_unit_wrap.h \
  src/python/star_system_exports.h \
  src/python/unit_exports.cpp \
  src/python/unit_exports.h \
  src/python/unit_exports1.cpp \
  src/python/unit_exports2.cpp \
  src/python/unit_exports3.cpp \
  src/python/unit_from_to_python.h \
  src/python/unit_method_defs.cpp \
  src/python/unit_wrapper_class.h \
  src/python/unit_wrapper.cpp \
  src/python/universe_util_export.cpp

LIBSCRIPT = \
  src/cmd/script/director_generic.cpp \
  src/cmd/script/mission_script.cpp \
  src/cmd/script/mission_types.h \
  src/cmd/script/mission.cpp \
  src/cmd/script/mission.h \
  src/cmd/script/msgcenter.cpp \
  src/cmd/script/msgcenter.h \
  src/cmd/script/pythonmission.cpp \
  src/cmd/script/pythonmission.h \
  src/cmd/script/script_call_olist.cpp \
  src/cmd/script/script_call_omap.cpp \
  src/cmd/script/script_call_order.cpp \
  src/cmd/script/script_call_string.cpp \
  src/cmd/script/script_call_unit_generic.cpp \
  src/cmd/script/script_callbacks.cpp \
  src/cmd/script/script_expression.cpp \
  src/cmd/script/script_generic.cpp \
  src/cmd/script/script_statement.cpp \
  src/cmd/script/script_util.cpp \
  src/cmd/script/script_variables.cpp

LIBAI = \
  src/cmd/ai/aggressive.cpp \
  src/cmd/ai/aggressive.h \
  src/cmd/ai/comm_ai.cpp \
  src/cmd/ai/comm_ai.h \
  src/cmd/ai/communication_xml.cpp \
  src/cmd/ai/communication.cpp \
  src/cmd/ai/communication.h \
  src/cmd/ai/docking.cpp \
  src/cmd/ai/docking.h \
  src/cmd/ai/event_xml.cpp \
  src/cmd/ai/event_xml.h \
  src/cmd/ai/fire.cpp \
  src/cmd/ai/fire.h \
  src/cmd/ai/fireall.cpp \
  src/cmd/ai/fireall.h \
  src/cmd/ai/firekeyboard.h \
  src/cmd/ai/flybywire.cpp \
  src/cmd/ai/flybywire.h \
  src/cmd/ai/flykeyboard.h \
  src/cmd/ai/hard_coded_scripts.cpp \
  src/cmd/ai/hard_coded_scripts.h \
  src/cmd/ai/ikarus.cpp \
  src/cmd/ai/ikarus.h \
  src/cmd/ai/missionscript.cpp \
  src/cmd/ai/missionscript.h \
  src/cmd/ai/navigation.cpp \
  src/cmd/ai/navigation.h \
  src/cmd/ai/order_comm.cpp \
  src/cmd/ai/order.cpp \
  src/cmd/ai/order.h \
  src/cmd/ai/script.cpp \
  src/cmd/ai/script.h \
  src/cmd/ai/script.h \
  src/cmd/ai/tactics.cpp \
  src/cmd/ai/tactics.h \
  src/cmd/ai/turretai.cpp \
  src/cmd/ai/turretai.h \
  src/cmd/ai/warpto.cpp \
  src/cmd/ai/warpto.h


libcollide2= \
  src/cmd/collide2/Ice/IceAABB.cpp \
  src/cmd/collide2/Ice/IceAABB.h \
  src/cmd/collide2/Ice/IceAxes.h \
  src/cmd/collide2/Ice/IceBoundingSphere.h \
  src/cmd/collide2/Ice/IceContainer.cpp \
  src/cmd/collide2/Ice/IceContainer.h \
  src/cmd/collide2/Ice/IceFPU.h \
  src/cmd/collide2/Ice/IceHPoint.cpp \
  src/cmd/collide2/Ice/IceHPoint.h \
  src/cmd/collide2/Ice/IceIndexedTriangle.cpp \
  src/cmd/collide2/Ice/IceIndexedTriangle.h \
  src/cmd/collide2/Ice/IceLSS.h \
  src/cmd/collide2/Ice/IceMatrix3x3.cpp \
  src/cmd/collide2/Ice/IceMatrix3x3.h \
  src/cmd/collide2/Ice/IceMatrix4x4.cpp \
  src/cmd/collide2/Ice/IceMatrix4x4.h \
  src/cmd/collide2/Ice/IceMemoryMacros.h \
  src/cmd/collide2/Ice/IceOBB.cpp \
  src/cmd/collide2/Ice/IceOBB.h \
  src/cmd/collide2/Ice/IcePairs.h \
  src/cmd/collide2/Ice/IcePlane.cpp \
  src/cmd/collide2/Ice/IcePlane.h \
  src/cmd/collide2/Ice/IcePoint.cpp \
  src/cmd/collide2/Ice/IcePoint.h \
  src/cmd/collide2/Ice/IcePreprocessor.h \
  src/cmd/collide2/Ice/IceRandom.cpp \
  src/cmd/collide2/Ice/IceRandom.h \
  src/cmd/collide2/Ice/IceRay.cpp \
  src/cmd/collide2/Ice/IceRay.h \
  src/cmd/collide2/Ice/IceRevisitedRadix.cpp \
  src/cmd/collide2/Ice/IceRevisitedRadix.h \
  src/cmd/collide2/Ice/IceSegment.cpp \
  src/cmd/collide2/Ice/IceSegment.h \
  src/cmd/collide2/Ice/IceTriangle.cpp \
  src/cmd/collide2/Ice/IceTriangle.h \
  src/cmd/collide2/Ice/IceTrilist.h \
  src/cmd/collide2/Ice/IceTypes.h \
  src/cmd/collide2/Ice/IceUtils.cpp \
  src/cmd/collide2/Ice/IceUtils.h \
  src/cmd/collide2/Ice/Stdafx.h \
  src/cmd/collide2/Stdafx.h \
  src/cmd/collide2/Opcode.h \
  src/cmd/collide2/opcodesysdef.h \
  src/cmd/collide2/opcodetypes.h \
  src/cmd/collide2/opcodeextern.h \
  src/cmd/collide2/csgeom2/opbox.cpp \
  src/cmd/collide2/csgeom2/opbox.h \
  src/cmd/collide2/csgeom2/opmatrix3.cpp \
  src/cmd/collide2/csgeom2/opmatrix3.h \
  src/cmd/collide2/csgeom2/opvector3.cpp \
  src/cmd/collide2/csgeom2/opvector.h \
  src/cmd/collide2/csgeom2/optransfrm.h \
  src/cmd/collide2/OPC_AABBCollider.cpp \
  src/cmd/collide2/OPC_AABBCollider.h \
  src/cmd/collide2/OPC_AABBTree.cpp \
  src/cmd/collide2/OPC_AABBTree.h \
  src/cmd/collide2/OPC_BaseModel.cpp \
  src/cmd/collide2/OPC_BaseModel.h \
  src/cmd/collide2/OPC_BoxBoxOverlap.h \
  src/cmd/collide2/OPC_BoxPruning.cpp \
  src/cmd/collide2/OPC_BoxPruning.h \
  src/cmd/collide2/OPC_Collider.cpp \
  src/cmd/collide2/OPC_Collider.h \
  src/cmd/collide2/OPC_Common.cpp \
  src/cmd/collide2/OPC_Common.h \
  src/cmd/collide2/OPC_IceHook.h \
  src/cmd/collide2/OPC_HybridModel.cpp \
  src/cmd/collide2/OPC_HybridModel.h \
  src/cmd/collide2/OPC_LSSAABBOverlap.h \
  src/cmd/collide2/OPC_LSSCollider.cpp \
  src/cmd/collide2/OPC_LSSCollider.h \
  src/cmd/collide2/OPC_LSSTriOverlap.h \
  src/cmd/collide2/OPC_MeshInterface.cpp \
  src/cmd/collide2/OPC_MeshInterface.h \
  src/cmd/collide2/OPC_Model.cpp \
  src/cmd/collide2/OPC_Model.h \
  src/cmd/collide2/OPC_OBBCollider.cpp \
  src/cmd/collide2/OPC_OBBCollider.h \
  src/cmd/collide2/OPC_OptimizedTree.cpp \
  src/cmd/collide2/OPC_OptimizedTree.h \
  src/cmd/collide2/OPC_Picking.cpp \
  src/cmd/collide2/OPC_Picking.h \
  src/cmd/collide2/OPC_PlanesAABBOverlap.h \
  src/cmd/collide2/OPC_PlanesCollider.cpp \
  src/cmd/collide2/OPC_PlanesCollider.h \
  src/cmd/collide2/OPC_PlanesTriOverlap.h \
  src/cmd/collide2/OPC_RayAABBOverlap.h \
  src/cmd/collide2/OPC_RayCollider.cpp \
  src/cmd/collide2/OPC_RayCollider.h \
  src/cmd/collide2/OPC_RayTriOverlap.h \
  src/cmd/collide2/OPC_Settings.h \
  src/cmd/collide2/OPC_SphereAABBOverlap.h \
  src/cmd/collide2/OPC_SphereCollider.cpp \
  src/cmd/collide2/OPC_SphereCollider.h \
  src/cmd/collide2/OPC_SphereTriOverlap.h \
  src/cmd/collide2/OPC_SweepAndPrune.cpp \
  src/cmd/collide2/OPC_SweepAndPrune.h \
  src/cmd/collide2/OPC_TreeBuilders.cpp \
  src/cmd/collide2/OPC_TreeBuilders.h \
  src/cmd/collide2/OPC_TreeCollider.cpp \
  src/cmd/collide2/OPC_TreeCollider.h \
  src/cmd/collide2/OPC_TriBoxOverlap.h \
  src/cmd/collide2/OPC_TriTriOverlap.h \
  src/cmd/collide2/OPC_VolumeCollider.cpp \
  src/cmd/collide2/OPC_VolumeCollider.h \
  src/cmd/collide2/CSopcodecollider.cpp \
  src/cmd/collide2/CSopcodecollider.h

LIBCMD = \
  src/cmd/alphacurve.cpp \
  src/cmd/alphacurve.h \
  src/cmd/asteroid_generic.cpp \
  src/cmd/asteroid_generic.h \
  src/cmd/beam_generic.cpp \
  src/cmd/beam.h \
  src/cmd/click_list.h \
  src/cmd/bolt_generic.cpp \
  src/cmd/bolt.h \
  src/cmd/building_generic.cpp \
  src/cmd/building_generic.h \
  src/cmd/collection.cpp \
  src/cmd/collection.h \
  src/cmd/collide_map.cpp \
  src/cmd/collide_map.h \
  src/cmd/collide.cpp \
  src/cmd/collide.h \
  src/cmd/container.cpp \
  src/cmd/container.h \
  src/cmd/csv.cpp \
  src/cmd/csv.h \
  src/cmd/enhancement_generic.h \
  src/cmd/faction_xml.cpp \
  src/cmd/images.h \
  src/cmd/iterator.h \
  src/cmd/key_mutable_set.h \
  src/cmd/missile_generic.cpp \
  src/cmd/missile_generic.h \
  src/cmd/mount.cpp \
  src/cmd/nebula_generic.cpp \
  src/cmd/nebula_generic.h \
  src/cmd/planet_generic.cpp \
  src/cmd/planet_generic.h \
  src/cmd/role_bitmask.cpp \
  src/cmd/role_bitmask.h \
  src/cmd/unit_bsp.cpp \
  src/cmd/unit_bsp.h \
  src/cmd/unit_collide.cpp \
  src/cmd/unit_collide.h \
  src/cmd/unit_const_cache.cpp \
  src/cmd/unit_const_cache.h \
  src/cmd/unit_csv.cpp \
  src/cmd/unit_factory_generic.cpp \
  src/cmd/unit_factory.h \
  src/cmd/unit_functions_generic.cpp \
  src/cmd/unit_generic.cpp \
  src/cmd/unit_generic.h \
  src/cmd/fg_util.cpp \
  src/cmd/fg_util.h \
  src/cmd/pilot.cpp \
  src/cmd/pilot.h \
  src/cmd/unit_util_generic.cpp \
  src/cmd/unit_util.h \
  src/cmd/unit_xml.cpp \
  src/cmd/unit_xml.h \
  src/cmd/weapon_xml.cpp \
  src/cmd/weapon_xml.h

if VS_MAKE_GTKUTILS
  GTKUTILS_PROG=vssetup vslauncher-gui
else
  GTKUTILS_PROG=
endif

if VS_MAKE_DIALOGUTILS
  DIALOGUTILS_PROG=vssetup_dlg
else
  DIALOGUTILS_PROG=
endif


if VS_MAKE_NO_CLIENT
  CLIENT_PROG=
else
  CLIENT_PROG= \
    vegastrike \
    base_maker
endif

bin_PROGRAMS = \
  $(CLIENT_PROG) \
  asteroidgen \
  mesher \
  mesh_xml \
  tempgen \
  replace \
  trisort \
  vegaserver \
  vsrextract \
  vsrmake \
  unicode-conv \
  test \
  $(GTKUTILS_PROG) \
  $(DIALOGUTILS_PROG) \
  vslauncher

all $(bin_PROGRAMS): version.h #get_scm_revision

noinst_LIBRARIES_1=\
  libnetlowlevel.a
if !BOOSTSYSTEM
noinst_LIBRARIES=$(noinst_LIBRARIES_1)\
  libboost_python.a
else
noinst_LIBRARIES=$(noinst_LIBRARIES_1)
endif

LIBCMDSERVER = \
  src/cmd/atmosphere_server.cpp \
  src/cmd/base_util_server.cpp \
  src/cmd/beam_server.cpp \
  src/cmd/bolt_server.cpp \
  src/cmd/unit_factory_server.cpp \
  src/cmd/unit_functions_server.cpp \
  src/cmd/unit_util_server.cpp

LIBSCRIPTSERVER = \
  src/cmd/script/flightgroup_server.cpp \
  src/cmd/script/flightgroup.h \
  src/cmd/script/script_call_unit_server.cpp \
  src/cmd/script/c_alike/c_alike.tab.cpp \
  src/cmd/script/c_alike/lex.yy.cpp

if !BOOSTSYSTEM
libboost_python_a_SOURCES = \
  $(BOOSTSOURCES)
endif


LIBGFX = \
  src/gfx/aux_texture.h \
  src/gfx/bounding_box.cpp \
  src/gfx/bounding_box.h \
  src/gfx/bsp.cpp \
  src/gfx/bsp.h \
  src/gfx/cockpit_generic.cpp \
  src/gfx/cockpit_generic.h \
  src/gfx/halo_system.h \
  src/gfx/lerp.cpp \
  src/gfx/lerp.h \
  src/gfx/matrix.cpp \
  src/gfx/matrix.h \
  src/gfx/mesh_bxm.cpp \
  src/gfx/mesh_bxm.h \
  src/gfx/mesh_bin.cpp \
  src/gfx/mesh_io.h \
  src/gfx/mesh_poly.cpp \
  src/gfx/mesh_xml.cpp \
  src/gfx/mesh_xml.h \
  src/gfx/mesh.cpp \
  src/gfx/mesh.h \
  src/gfx/quaternion.cpp \
  src/gfx/quaternion.h \
  src/gfx/sphere_generic.cpp \
  src/gfx/vec.cpp \
  src/gfx/vec.h \
  src/gfx/vertex.h

LIBGFXCLIENT = \
  src/gfx/ani_texture.cpp \
  src/gfx/ani_texture.h \
  src/gfx/animation.cpp \
  src/gfx/animation.h \
  src/gfx/aux_logo.cpp \
  src/gfx/aux_logo.h \
  src/gfx/aux_palette.cpp \
  src/gfx/aux_palette.h \
  src/gfx/aux_texture.cpp \
  src/gfx/aux_texture.h \
  src/gfx/background.cpp \
  src/gfx/background.h \
  src/gfx/camera.cpp \
  src/gfx/camera.h \
  src/gfx/car_assist.h \
  src/gfx/cockpit_xml.cpp \
  src/gfx/cockpit.cpp \
  src/gfx/cockpit.h \
  src/gfx/coord_select.cpp \
  src/gfx/coord_select.h \
  src/gfx/decalqueue.h \
  src/gfx/env_map_gent.cpp \
  src/gfx/env_map_gent.h \
  src/gfx/gauge.cpp \
  src/gfx/gauge.h \
  src/gfx/halo_system.cpp \
  src/gfx/halo.cpp \
  src/gfx/halo.h \
  src/gfx/hud.cpp \
  src/gfx/hud.h \
  src/gfx/jpeg_memory.cpp \
  src/gfx/jpeg_memory.h \
  src/gfx/loc_select.cpp \
  src/gfx/loc_select.h \
  src/gfx/masks.cpp \
  src/gfx/masks.h \
  src/gfx/mesh_fx.cpp \
  src/gfx/mesh_fx.h \
  src/gfx/mesh_gfx.cpp \
  src/gfx/nav/criteria_xml.cpp \
  src/gfx/nav/criteria.cpp \
  src/gfx/nav/criteria.h \
  src/gfx/nav/drawgalaxy.cpp \
  src/gfx/nav/drawlist.cpp \
  src/gfx/nav/drawlist.h \
  src/gfx/nav/drawsystem.cpp \
  src/gfx/nav/navcomputer.cpp \
  src/gfx/nav/navcomputer.h \
  src/gfx/nav/navgetxmldata.cpp \
  src/gfx/nav/navgetxmldata.h \
  src/gfx/nav/navitemstodraw.h \
  src/gfx/nav/navitemtypes.h \
  src/gfx/nav/navparse.h \
  src/gfx/nav/navpath.cpp \
  src/gfx/nav/navpath.h \
  src/gfx/nav/navscreen.cpp \
  src/gfx/nav/navscreen.h \
  src/gfx/nav/navscreenoccupied.cpp \
  src/gfx/nav/navscreenoccupied.h \
  src/gfx/nonlinear_transform.h \
  src/gfx/particle.cpp \
  src/gfx/particle.h \
  src/gfx/pipelined_texture.cpp \
  src/gfx/pipelined_texture.h \
  src/gfx/planetary_transform.h \
  src/gfx/point_to_cam.h \
  src/gfx/quadsquare_cull.cpp \
  src/gfx/quadsquare_render.cpp \
  src/gfx/quadsquare_update.cpp \
  src/gfx/quadsquare.cpp \
  src/gfx/quadsquare.h \
  src/gfx/quadtree_xml.cpp \
  src/gfx/quadtree.cpp \
  src/gfx/quadtree.h \
  src/gfx/ring.cpp \
  src/gfx/ring.h \
  src/gfx/screenshot.cpp \
  src/gfx/screenshot.h \
  src/gfx/sphere.cpp \
  src/gfx/sphere.h \
  src/gfx/sprite.cpp \
  src/gfx/sprite.h \
  src/gfx/star.cpp \
  src/gfx/star.h \
  src/gfx/stream_texture.cpp \
  src/gfx/stream_texture.h \
  src/gfx/tex_transform.cpp \
  src/gfx/technique.h \
  src/gfx/technique.cpp \
  src/gfx/vdu.cpp \
  src/gfx/vdu.h \
  src/gfx/vid_file.h \
  src/gfx/vid_file.cpp \
  src/ffmpeg_init.h \
  src/ffmpeg_init.cpp \
  src/gfx/vsbox.cpp \
  src/gfx/vsbox.h \
  src/gfx/vsimage.cpp \
  src/gfx/vsimage.h \
  src/gfx/warptrail.cpp \
  src/gfx/warptrail.h \
  src/gfx/xvector.h


LIBGFXSERVER = \
  src/gfx/sphere_server.cpp \
  src/gfx/vsbox_server.cpp \
  src/gfx/mesh_server.cpp \
  src/gfx/background_server.cpp

libnetlowlevel_a_SOURCES = \
  src/networking/lowlevel/GCPtr.h \
  src/networking/lowlevel/netbuffer.cpp \
  src/networking/lowlevel/netbuffer.h \
  src/networking/lowlevel/netui.cpp \
  src/networking/lowlevel/netui.h \
  src/networking/lowlevel/packet.cpp \
  src/networking/lowlevel/packet.h \
  src/networking/lowlevel/packetmem.cpp \
  src/networking/lowlevel/packetmem.h \
  src/networking/lowlevel/vsnet_address.cpp \
  src/networking/lowlevel/vsnet_address.h \
  src/networking/lowlevel/vsnet_clientstate.cpp \
  src/networking/lowlevel/vsnet_clientstate.h \
  src/networking/lowlevel/vsnet_cmd.cpp \
  src/networking/lowlevel/vsnet_cmd.h \
  src/networking/lowlevel/vsnet_debug.cpp \
  src/networking/lowlevel/vsnet_debug.h \
  src/networking/lowlevel/vsnet_dloadenum.cpp \
  src/networking/lowlevel/vsnet_dloadenum.h \
  src/networking/lowlevel/vsnet_dloadmgr.cpp \
  src/networking/lowlevel/vsnet_dloadmgr.h \
  src/networking/lowlevel/vsnet_headers.h \
  src/networking/lowlevel/vsnet_err.h \
  src/networking/lowlevel/vsnet_notify.cpp \
  src/networking/lowlevel/vsnet_notify.h \
  src/networking/lowlevel/vsnet_oss.cpp \
  src/networking/lowlevel/vsnet_oss.h \
  src/networking/lowlevel/vsnet_parsecmd.cpp \
  src/networking/lowlevel/vsnet_parsecmd.h \
  src/networking/lowlevel/vsnet_pipe.cpp \
  src/networking/lowlevel/vsnet_pipe.h \
  src/networking/lowlevel/vsnet_queue.h \
  src/networking/lowlevel/vsnet_serversocket.cpp \
  src/networking/lowlevel/vsnet_serversocket.h \
  src/networking/lowlevel/vsnet_socket.cpp \
  src/networking/lowlevel/vsnet_socket.h \
  src/networking/lowlevel/vsnet_socketbase.cpp \
  src/networking/lowlevel/vsnet_socketbase.h \
  src/networking/lowlevel/vsnet_socketflags.cpp \
  src/networking/lowlevel/vsnet_socketflags.h \
  src/networking/lowlevel/vsnet_sockethttp.cpp \
  src/networking/lowlevel/vsnet_socketset.cpp \
  src/networking/lowlevel/vsnet_socketset.h \
  src/networking/lowlevel/vsnet_sockettcp.cpp \
  src/networking/lowlevel/vsnet_sockettcp.h \
  src/networking/lowlevel/vsnet_socketudp.cpp \
  src/networking/lowlevel/vsnet_socketudp.h \
  src/networking/lowlevel/vsnet_thread_non.cpp \
  src/networking/lowlevel/vsnet_thread_p.cpp \
  src/networking/lowlevel/vsnet_thread_sdl.cpp \
  src/networking/lowlevel/vsnet_thread.h


vegastrike_LDADD_1 = $(noinst_LIBRARIES) \
	-lz
if BOOSTSYSTEM
  vegastrike_LDADD = $(vegastrike_LDADD_1) \
    -lboost_python-st
else
  vegastrike_LDADD = $(vegastrike_LDADD_1)
endif

.PHONY: get_scm_revision
version.h: get_scm_revision
	@true
get_scm_revision:
	@echo "-- checking scm revision..."
	@scm_rev="$$(cd "$(top_srcdir)" && git --no-pager describe --abbrev=8 --dirty --always 2>/dev/null || printf -- 'unknown')"; \
	scm_remote="$$(cd "$(top_srcdir)" && git --no-pager remote get-url origin 2>/dev/null || printf -- 'unknown')"; \
	scm_ver_rcnum=$$(echo "$(PACKAGE_VERSION)" | sed -e 's/^[^0-9]*\([0-9.]*\).*$$/\1/' -e 's/[.]/,/g'); \
	printf -- "%s\n" "#ifndef _AUTOMAKE_GETSCM_VERSION_H" "# define _AUTOMAKE_GETSCM_VERSION_H" \
					 "# define SCM_VERSION \"$(PACKAGE_VERSION)\"" \
					 "# define SCM_VERSION_RCNUM $${scm_ver_rcnum}" \
					 "# define SCM_REVISION \"$${scm_rev}\"" \
					 "# define SCM_REMOTE \"$${scm_remote}\"" \
					 "#endif" > version.h.txt && diff -q version.h.txt version.h 2>/dev/null || cp version.h.txt version.h

vegastrike_DEPENDENCIES = $(noinst_LIBRARIES)

vegaserver_LDADD_1 = $(noinst_LIBRARIES) \
	-lz
if BOOSTSYSTEM
  vegaserver_LDADD = $(vegaserver_LDADD_1) \
    -lboost_python-st
else
  vegaserver_LDADD = $(vegaserver_LDADD_1)
endif

vegaserver_DEPENDENCIES = $(noinst_LIBRARIES)

librootclient = \
  src/audiolib.h \
  src/cg_global.cpp \
  src/command.cpp \
  src/command.h \
  src/config_xml.cpp \
  src/config_xml.h \
  src/debug_vs.cpp \
  src/debug_vs.h \
  src/faction_util.cpp \
  src/force_feedback.cpp \
  src/force_feedback.h \
  src/functors.h \
  src/fmod.h \
  src/gamemenu.h \
  src/gamemenu.cpp \
  src/gfxlib_struct.cpp \
  src/gfxlib_struct.h \
  src/gfxlib.h \
  src/in_handler.h \
  src/in_joystick.cpp \
  src/in_joystick.h \
  src/in_kb.cpp \
  src/in_kb.h \
  src/in_kb_data.h \
  src/in_main.cpp \
  src/in_main.h \
  src/in_mouse.cpp \
  src/in_mouse.h \
  src/in_sdl.cpp \
  src/linecollide.h \
  src/main_loop.cpp \
  src/main_loop.h \
  src/physics.cpp \
  src/physics.h \
  src/profile.h \
  src/rendertext.cpp \
  src/rendertext.h \
  src/resizable.h \
  src/ship_commands.cpp \
  src/ship_commands.h \
  src/star_system_jump.cpp \
  src/star_system.cpp \
  src/star_system.h \
  src/universe_util.cpp \
  src/universe_util.h \
  src/universe.cpp \
  src/universe.h

# Disabled from librootclient since MMORPG depends on SDL which is not
# easily available on Macs. -Patrick
#  src/mmorpgclient.cpp
#  src/mmorpgclient.h

librootgeneric = \
  src/cg_global.h \
  src/configxml.cpp \
  src/configxml.h \
  src/easydom.cpp \
  src/easydom.h \
  src/endianness.cpp \
  src/endianness.h \
  src/macosx_math.cpp \
  src/macosx_math.h \
  src/faction_generic.cpp \
  src/faction_generic.h \
  src/faction_util_generic.cpp \
  src/file_main.h \
  src/galaxy_gen.cpp \
  src/galaxy_gen.h \
  src/galaxy_xml.cpp \
  src/galaxy_xml.h \
  src/galaxy.cpp \
  src/hashtable.cpp \
  src/hashtable.h \
  src/in.h \
  src/lin_time.cpp \
  src/lin_time.h \
  src/load_mission.cpp \
  src/load_mission.h \
  src/pk3.cpp \
  src/pk3.h \
  src/posh.cpp \
  src/posh.h \
  src/savegame.cpp \
  src/savegame.h \
  src/SharedPool.h \
  src/star_system_generic.cpp \
  src/star_system_generic.h \
  src/star_system_xml.cpp \
  src/save_util.h \
  src/stardate.cpp \
  src/stardate.h \
  src/universe_generic.cpp \
  src/universe_generic.h \
  src/universe_util_generic.cpp \
  src/universe_util.h \
  src/vegastrike.h \
  src/vs_globals.cpp \
  src/vs_globals.h \
  src/vs_random.h \
  src/common/common.cpp \
  src/common/common.h \
  src/vsfilesystem.cpp \
  src/vsfilesystem.h \
  src/xml_serializer.cpp \
  src/xml_serializer.h \
  src/xml_support.cpp \
  src/xml_support.h \
  src/options.cpp \
  src/options.h \
  src/log.cpp \
  src/log.h \
  src/unicode.cpp \
  src/unicode_combine.cpp \
  src/unicode.h \
  src/vs_log_modules.cpp \
  src/vs_log_modules.h \
  src/multimap.cpp \
  src/multimap.h

librootserver = \
  src/faction_util_server.cpp \
  src/force_feedback_server.cpp \
  src/gfxlib_struct_server.cpp \
  src/libaudioserver.cpp \
  src/libserver.cpp \
  src/universe_util_server.cpp

LIBAUDIO = \
  src/audio/CodecRegistry.cpp \
  src/audio/CodecRegistry.h \
  src/audio/Exceptions.h \
  src/audio/Format.h \
  src/audio/Stream.h \
  src/audio/Stream.cpp \
  src/audio/Sound.h \
  src/audio/Sound.cpp \
  src/audio/Source.h \
  src/audio/Source.cpp \
  src/audio/Types.h \
  src/audio/Vector.h \
  src/audio/SourceTemplate.h \
  src/audio/SourceTemplate.cpp \
  src/audio/SceneManager.h \
  src/audio/SceneManager.cpp \
  src/audio/Renderer.h \
  src/audio/Renderer.cpp \
  src/audio/utils.h \
  src/audio/utils.cpp \
  src/audio/RenderableSource.h \
  src/audio/RenderableSource.cpp \
  src/audio/Listener.h \
  src/audio/Listener.cpp \
  src/audio/Scene.h \
  src/audio/Scene.cpp \
  src/audio/SimpleSource.h \
  src/audio/SimpleSource.cpp \
  src/audio/SimpleScene.h \
  src/audio/SimpleScene.cpp \
  src/audio/codecs/Codec.cpp \
  src/audio/codecs/Codec.h \
  src/audio/codecs/FFStream.cpp \
  src/audio/codecs/FFStream.h \
  src/audio/codecs/OggStream.cpp \
  src/audio/codecs/OggStream.h \
  src/audio/codecs/OggData.cpp \
  src/audio/codecs/OggData.h \
  src/audio/codecs/OggCodec.cpp \
  src/audio/codecs/OggCodec.h \
  src/audio/codecs/FFCodec.cpp \
  src/audio/codecs/FFCodec.h

LIBENGINE_COMMON = \
  $(PRECOMPILED) \
  $(LIBAI) \
  $(LIBCMD) \
  $(libcollide2) \
  $(LIBNET) \
  $(LIBPYTHON) \
  $(librootgeneric) \
  $(LIBSCRIPT) \
  $(LIBGFX) \
  src/XMLDocument.h \
  src/XMLDocument.cpp \
  src/VSFileXMLSerializer.h \
  src/VSFileXMLSerializer.cpp
vegaserver_SOURCES = \
  $(LIBENGINE_COMMON) \
  $(LIBCMDSERVER) \
  $(LIBNETSERVER) \
  $(librootserver) \
  $(LIBUNIUTIL) \
  $(LIBSCRIPTSERVER) \
  $(LIBGFXSERVER) \
  src/vegaserver.cpp \
  src/cmd/script/director_server.cpp \
  src/gldrv/gl_vertex_list.cpp \
  src/gldrv/gl_sphere_list_server.cpp

#vegastrike_CXXFLAGS = -include $(PRECOMPILED)
vegastrike_SOURCES = \
  $(LIBENGINE_COMMON) \
  $(LIBGUI) \
  $(LIBNETCLIENT) \
  $(librootclient) \
  $(LIBGFXCLIENT) \
  $(LIBAUDIO) \
  src/aldrv/al_globals.cpp \
  src/aldrv/al_globals.h \
  src/aldrv/al_init.cpp \
  src/aldrv/al_listen.cpp \
  src/aldrv/al_sound.cpp \
  src/cmd/ai/firekeyboard.cpp \
  src/cmd/ai/firekeyboard.h \
  src/cmd/ai/flyjoystick.cpp \
  src/cmd/ai/flyjoystick.h \
  src/cmd/ai/flykeyboard.cpp \
  src/cmd/ai/flykeyboard.h \
  src/cmd/ai/input_dfa.cpp \
  src/cmd/ai/input_dfa.h \
  src/cmd/asteroid.cpp \
  src/cmd/asteroid.h \
  src/cmd/atmosphere.cpp \
  src/cmd/atmosphere.h \
  src/cmd/base_init.cpp \
  src/cmd/base_interface.cpp \
  src/cmd/base_util.cpp \
  src/cmd/base_util.h \
  src/cmd/base_write_python.cpp \
  src/cmd/base_write_xml.cpp \
  src/cmd/base_xml.cpp \
  src/cmd/base.h \
  src/cmd/basecomputer.cpp \
  src/cmd/basecomputer.h \
  src/cmd/beam.cpp \
  src/cmd/beam.h \
  src/cmd/bolt.cpp \
  src/cmd/briefing.cpp \
  src/cmd/briefing.h \
  src/cmd/building.cpp \
  src/cmd/building.h \
  src/cmd/click_list.cpp \
  src/cmd/click_list.h \
  src/cmd/cont_terrain.cpp \
  src/cmd/cont_terrain.h \
  src/cmd/enhancement.h \
  src/cmd/missile.h \
  src/cmd/music.cpp \
  src/cmd/music.h \
  src/cmd/nebula.cpp \
  src/cmd/nebula.h \
  src/cmd/planet.cpp \
  src/cmd/planet.h \
  src/cmd/script/c_alike/c_alike.tab.cpp \
  src/cmd/script/c_alike/lex.yy.cpp \
  src/cmd/script/director.cpp \
  src/cmd/script/flightgroup.cpp \
  src/cmd/script/flightgroup.h \
  src/cmd/script/script_call_briefing.cpp \
  src/cmd/script/script_call_unit.cpp \
  src/cmd/terrain.cpp \
  src/cmd/terrain.h \
  src/cmd/unit_armorshield.h \
  src/cmd/unit_csv.h \
  src/cmd/unit_factory.cpp \
  src/cmd/unit_find.h \
  src/cmd/unit_functions.cpp \
  src/cmd/unit_interface.cpp \
  src/cmd/unit_interface.h \
  src/cmd/unit_template.h \
  src/cmd/unit_util.cpp \
  src/cmd/unit_util.h \
  src/cmd/unit.h \
  src/gldrv/gl_program.cpp \
  src/gldrv/gl_clip.cpp \
  src/gldrv/gl_fog.cpp \
  src/gldrv/gl_globals.cpp \
  src/gldrv/gl_globals.h \
  src/gldrv/gl_include.h \
  src/gldrv/gl_init.cpp \
  src/gldrv/gl_init.h \
  src/gldrv/gl_light_pick.cpp \
  src/gldrv/gl_light_state.cpp \
  src/gldrv/gl_light.cpp \
  src/gldrv/gl_light.h \
  src/gldrv/gl_material.cpp \
  src/gldrv/gl_matrix.cpp \
  src/gldrv/gl_matrix.h \
  src/gldrv/gl_misc.cpp \
  src/gldrv/gl_quad_list.cpp \
  src/gldrv/gl_sphere_list.cpp \
  src/gldrv/gl_state.cpp \
  src/gldrv/sdds.cpp \
  src/gldrv/sdds.h \
  src/gldrv/gl_texture.cpp \
  src/gldrv/gl_vertex_list.cpp \
  src/gldrv/gl_undefined_extensions.h \
  src/gldrv/glx_undefined_extensions.h \
  src/gldrv/gl_include.h \
  src/gldrv/hashtable_3d.h \
  src/gldrv/winsys.cpp \
  src/gldrv/winsys.h \
  src/main.cpp \
  src/python/briefing_wrapper.cpp


pkgdatadir = $(prefix)/objconv
pkgdata_DATA = objconv/3ds2xml objconv/obj2xml objconv/wcp2xml
EXTRA_DIST += $(pkgdata_DATA)



#base_maker_CXXFLAGS += -I../.. -I../../src $(BOOST_CPPFLAGS) -g
# ToDo: Fix so -L/usr/... is found in configure
#base_maker_LDFLAGS = -lGL -lGLU -lSDL -lglut -ljpeg -lpng \
#base_maker_LDFLAGS = -lGL -lGLU -lglut -ljpeg -lpng \
#  -lpthread -L/usr/X11R6/lib -lXi -lXmu
base_maker_SOURCES = \
  objconv/basemaker/base_maker.cpp \
  objconv/basemaker/base_maker_texture.cpp \
  objconv/basemaker/base_maker_texture.h

mesher_CXXFLAGS = $(OGRE_CFLAGS) -I../../src -I../..
mesher_LDFLAGS = $(OGRE_LIBS)
mesher_SRC = \
  objconv/mesher/Converter.cpp \
  objconv/mesher/expat.h  \
  objconv/mesher/from_BFXM.cpp \
  objconv/mesher/from_BFXM.h \
  objconv/mesher/from_obj.cpp \
  objconv/mesher/from_obj.h \
  src/gfx/mesh_io.h \
  objconv/mesher/mesh.h  \
  objconv/mesher/to_BFXM.cpp \
  objconv/mesher/to_BFXM.h \
  objconv/mesher/to_obj.cpp \
  objconv/mesher/to_obj.h \
  objconv/mesher/main.cpp \
  objconv/mesher/PrecompiledHeaders/Converter.cpp \
  objconv/mesher/PrecompiledHeaders/Converter.h \
  objconv/mesher/Modules/BFXM_to_Wavefront.cpp \
  objconv/mesher/Modules/Wavefront_to_BFXM.cpp \
  objconv/mesher/Modules/BFXM_to_XMesh.cpp \
  objconv/mesher/Modules/XMesh_to_BFXM.cpp \
  objconv/mesher/Modules/Convert.cpp \
  objconv/mesher/Modules/Dims.cpp \
  objconv/mesher/Modules/OldSyntax.cpp \
  src/endianness.h \
  src/hashtable.cpp \
  src/hashtable.h \
  src/xml_support.cpp \
  src/xml_support.h
if HAVE_OGRE
mesher_SOURCES = $(mesher_SRC) \
  objconv/mesher/to_OgreMesh.cpp \
  objconv/mesher/Modules/XMesh_to_Ogre.cpp \
  objconv/mesher/to_OgreMesh.h
else
mesher_SOURCES = $(mesher_SRC)
endif
tempgen_LDFLAGS = -lexpat
tempgen_CPPFLAGS = -I../../src
tempgen_SOURCES = \
  objconv/MeshParse/template_generator.cpp \
  src/xml_support.cpp \
  src/xml_support.h \
  src/hashtable.cpp \
  src/hashtable.h

mesh_xml_LDFLAGS = -lexpat
mesh_xml_CPPFLAGS = -I../../src
mesh_xml_SOURCES = \
  objconv/MeshParse/template_generator.cpp \
  src/xml_support.cpp \
  src/xml_support.h

trisort_SOURCES = \
  objconv/trisort.cpp \
  objconv/trisort.h

replace_SOURCES = \
  objconv/replace.cpp

asteroidgen_SOURCES = \
  objconv/asteroidgen.cpp

vsrmake_SOURCES= \
  tools/common.cpp \
  tools/common.h \
  tools/vsrmake.cpp \
  tools/vsrtools.h

vsrextract_SOURCES= \
  tools/common.cpp \
  tools/common.h \
  tools/vsrextract.cpp \
  tools/vsrtools.h

unicode_conv_LDFLAGS =
unicode_conv_CXXFLAGS=-I.. -DVS_LOG_NO_XML
unicode_conv_SOURCES= \
  tools/unicode-conv.cpp \
  tools/unicode.link.cpp \
  src/unicode_combine.cpp \
  src/common/common.cpp

test_LDFLAGS =
test_CXXFLAGS=-I.. -DVS_LOG_NO_XML -DVS_UNIT_TESTS -DVS_UTF8_ITERATOR_TESTS -DVS_FUNCTORS_TESTS
test_SOURCES= \
  tests/test.cpp \
  tests/log.link.cpp \
  tests/unicode.link.cpp \
  src/unicode_combine.cpp \
  src/common/common.cpp \
  tests/multimap.link.cpp

vegastrike_launch_tests: test
	./test
.PHONY: vegastrike_launch_tests
check: vegastrike_launch_tests

#i am a novice about autoconf/automake, for the moment this works
# and allows building file with different flags than for main target
tests/unicode.link.cpp: $(top_srcdir)/src/unicode.cpp
	cp $< $@ || copy $< $@
tests/multimap.link.cpp: $(top_srcdir)/src/multimap.cpp
	cp $< $@ || copy $< $@
tests/log.link.cpp: $(top_srcdir)/src/log.cpp
	cp $< $@ || copy $< $@
tools/unicode.link.cpp: $(top_srcdir)/src/unicode.cpp
	cp $< $@ || copy $< $@
tools/log.link.cpp: $(top_srcdir)/src/log.cpp
	cp $< $@ || copy $< $@

VSSETUP_COMMON_SOURCES = \
    setup/src/c/setup.cpp \
    setup/src/include/central.cpp \
    setup/src/include/central.h \
    setup/src/include/display.h \
    setup/src/include/file.cpp \
    setup/src/include/file.h \
    setup/src/include/general.cpp \
    setup/src/include/general.h \
    src/unicode.cpp \
    src/unicode_combine.cpp \
    src/unicode.h \
    src/common/common.cpp \
    src/common/common.h \
    setup/log.link.cpp \
    src/log.h

setup/log.link.cpp: $(top_srcdir)/src/log.cpp
	cp $< $@ || copy $< $@
vssetup_LDFLAGS = $(GTK_LIBS)
vssetup_CXXFLAGS= $(GTK_CFLAGS) -DVS_LOG_NO_XML
vssetup_SOURCES = \
    $(VSSETUP_COMMON_SOURCES) \
    setup/src/include/display_gtk.cpp

vssetup_dlg_LDFLAGS = $(DIALOG_LIBS)
vssetup_dlg_CXXFLAGS= $(DIALOG_CFLAGS) -DVS_LOG_NO_XML
vssetup_dlg_SOURCES = \
    $(VSSETUP_COMMON_SOURCES) \
    setup/src/include/display_dialog.cpp

VSLAUNCHER_COMMON_SOURCES = \
	launcher/launcher.h \
	launcher/log.link.cpp \
	launcher/main.cpp \
    launcher/general.cpp \
    launcher/general.h \
    src/unicode.cpp \
    src/unicode_combine.cpp \
    src/unicode.h \
    src/common/common.cpp \
    src/common/common.h

VSLAUNCHER_GUI_SOURCES = launcher/saveinterface.cpp
VSLAUNCHER_NOGUI_SOURCES = launcher/nogui.cpp

vslauncher_gui_LDFLAGS = $(GTK_LIBS) $(VS_APPLE_CARBON_LIBRARY)
vslauncher_LDFLAGS = $(GTK_LIBS) $(VS_APPLE_CARBON_LIBRARY)

vslauncher_gui_CXXFLAGS = -DVS_LOG_NO_XML $(GTK_CFLAGS)
vslauncher_CXXFLAGS = -DVS_LOG_NO_XML

launcher/log.link.cpp: $(top_srcdir)/src/log.cpp
	cp $< $@ || copy $< $@

vslauncher_gui_SOURCES = \
	$(VSLAUNCHER_COMMON_SOURCES) \
	$(VSLAUNCHER_GUI_SOURCES)

vslauncher_SOURCES = \
	$(VSLAUNCHER_COMMON_SOURCES) \
	$(VSLAUNCHER_NOGUI_SOURCES)

if USE_PRECOMPILED_HEADERS
PRECOMPILED_FLAGS = -include $(PRECOMPILED)
else
PRECOMPILED_FLAGS =
endif

AM_CXXFLAGS = $(PRECOMPILED_FLAGS)

#vslauncher_LDFLAGS = $(GTK_LIBS)
#vslauncher_CXXFLAGS= $(GTK_CFLAGS)
#vslauncher_SOURCES= \
#    launcher/saveinterface.cpp \
#    launcher/general.cpp \
#    launcher/general.h \
#    src/common/common.cpp \
#    src/common/common.h

#src/cmd/script/c_alike/c_alike.tab.cpp: src/cmd/script/c_alike/c_alike.yacc

#src/cmd/script/c_alike/lex.yy.cpp: src/cmd/script/c_alike/c_alike.lex
#	flex -o src/cmd/script/c_alike/lex.yy.cpp src/cmd/script/c_alike/c_alike.lex

#The following is a way to get around the HUGE number of files that boost contains.
if !BOOSTSYSTEM
dist-hook:
	$(mkdir_p) $(distdir)/$(BOOSTDIR)
	for x in `find $(BOOSTDIR) -maxdepth 1 ! -wholename $(BOOSTDIR) -type d -printf '%f\n' |sort -r`; do \
	  find $(BOOSTDIR)/$$x/boost "(" -name CVS -prune ")" , "(" ! -name CVS -type d -printf "D %p\n" ")" , "(" -type f -printf "F %p\n" ")" \
	    | $(AWK) '$$1=="D"{print("'"$(mkdir_p) $(distdir)/"'"$$2)}; $$1=="F"{print("cp -p "$$2"'" $(distdir)/"'/"$$2)};'  | $(SHELL); \
done
endif

## BOOST 1_35 patch
if BOOST1_35
all: boost_patched_gcc44

boost_patched_gcc44: $(top_srcdir)/boost_gcc44.patch
	cd $(top_srcdir) && patch -N -p0 < boost_gcc44.patch || true

endif
## ! BOOST 1_35 patch

%.h.gch: %.h
	$(CXXCOMPILE) -o $@ $<

.PHONY: precompiled
precompiled: $(PRECOMPILED).gch

.PHONY: headers
headers:
	find . -name '*.h' -exec make '{}.gch' ';'

.PHONY: clean-local
clean-local:
	rm -f `find . -name '*.h.gch'`
noinst_HEADERS = src/gfx/vid_file.h
