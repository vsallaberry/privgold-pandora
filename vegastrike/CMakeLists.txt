# create a build directory somewhere outside of this source tree
# run cmake <path to source tree> inside it
# run ccmake and hand tune any config variables/paths you need to, save
# run make ...or build your project files
# valid targets are vegastrike, vssetup, and vegaserver (though it's broken right now)
# ccmake will let you set the Build type (debug,release,basic)


cmake_minimum_required(VERSION 2.4)

project (vsUTCS)
set(VERSION "0.4.4.5")

#SET(CMAKE_CXX_STANDARD_REQUIRED 1) ## ERROR instead of DECAYING value if not supported
if(NOT CMAKE_CXX_STANDARD)
    SET(CMAKE_CXX_STANDARD 11)
elseif(NOT CMAKE_CXX_STANDARD MATCHES "(98|11|14|17|20|23)")
    set(requested_cxx_standard ${CMAKE_CXX_STANDARD})
    if(CMAKE_CXX_STANDARD EQUAL "03")
        SET(CMAKE_CXX_STANDARD 98)
    else()
        SET(CMAKE_CXX_STANDARD 11)
    endif()
    message(WARNING "CMAKE_CXX_STANDARD has unknown value (${requested_cxx_standard}),using ${CMAKE_CXX_STANDARD}")
    unset(requested_cxx_standard)
endif(NOT CMAKE_CXX_STANDARD)
#set (CMAKE_CXX_EXTENSIONS FALSE)

include_directories(${vsUTCS_SOURCE_DIR}/src
					${vsUTCS_SOURCE_DIR}/src/cmd
					${vsUTCS_BINARY_DIR} )

add_library(netlowlevel    src/networking/lowlevel/netbuffer.cpp
						   src/networking/lowlevel/netui.cpp
						   src/networking/lowlevel/packet.cpp
						   src/networking/lowlevel/packetmem.cpp
						   src/networking/lowlevel/vsnet_address.cpp
						   src/networking/lowlevel/vsnet_clientstate.cpp
						   src/networking/lowlevel/vsnet_cmd.cpp
						   src/networking/lowlevel/vsnet_debug.cpp
						   src/networking/lowlevel/vsnet_dloadenum.cpp
						   src/networking/lowlevel/vsnet_dloadmgr.cpp
						   src/networking/lowlevel/vsnet_notify.cpp
						   src/networking/lowlevel/vsnet_oss.cpp
						   src/networking/lowlevel/vsnet_parsecmd.cpp
						   src/networking/lowlevel/vsnet_pipe.cpp
						   src/networking/lowlevel/vsnet_serversocket.cpp
						   src/networking/lowlevel/vsnet_socket.cpp
						   src/networking/lowlevel/vsnet_socketbase.cpp
						   src/networking/lowlevel/vsnet_socketflags.cpp
						   src/networking/lowlevel/vsnet_sockethttp.cpp
						   src/networking/lowlevel/vsnet_socketset.cpp
						   src/networking/lowlevel/vsnet_sockettcp.cpp
						   src/networking/lowlevel/vsnet_socketudp.cpp
						   src/networking/lowlevel/vsnet_thread_non.cpp
						   src/networking/lowlevel/vsnet_thread_p.cpp
						   src/networking/lowlevel/vsnet_thread_sdl.cpp )

SET(LIBNET_SOURCES src/networking/inet_file.cpp
				   src/networking/inet.cpp )

add_library(netgeneric    src/networking/accountsxml.cpp
						  src/networking/client.cpp
						  src/networking/fileutil.cpp
						  src/networking/savenet_util.cpp
						  src/networking/cubicsplines.cpp
						  src/networking/mangle.cpp
						  src/networking/netclient_clients.cpp
						  src/networking/netclient_devices.cpp
						  src/networking/netclient_login.cpp
						  src/networking/netclient.cpp
						  src/networking/netserver_acct.cpp
						  src/networking/netserver_clients.cpp
						  src/networking/netserver_devices.cpp
						  src/networking/netserver_login.cpp
						  src/networking/netserver_net.cpp
						  src/networking/netserver.cpp
						  src/networking/prediction.cpp
						  src/networking/zonemgr.cpp )

SET(LIBNETCLIENT_SOURCES src/networking/networkcomm.cpp
						 src/networking/webcam_support.cpp )

SET(LIBNETSERVER_SOURCES src/networking/networkcomm_server.cpp )

SET(LIBGUI_SOURCES src/gui/button.cpp
				   src/gui/control.cpp
				   src/gui/eventmanager.cpp
				   src/gui/eventresponder.cpp
				   src/gui/font.cpp
				   src/gui/general.cpp
				   src/gui/glut_support.cpp
				   src/gui/groupcontrol.cpp
				   src/gui/guidefs.cpp
				   src/gui/guitexture.cpp
				   src/gui/modaldialog.cpp
				   src/gui/newbutton.cpp
				   src/gui/painttext.cpp
				   src/gui/picker.cpp
				   src/gui/scroller.cpp
				   src/gui/simplepicker.cpp
				   src/gui/slider.cpp
				   src/gui/staticdisplay.cpp
				   src/gui/text_area.cpp
				   src/gui/textinputdisplay.cpp
				   src/gui/window.cpp
				   src/gui/windowcontroller.cpp )

SET(LIBPYTHON_SOURCES src/python/init.cpp
					  src/python/python_compile.cpp
					  src/python/unit_exports.cpp
					  src/python/unit_exports1.cpp
					  src/python/unit_exports2.cpp
					  src/python/unit_exports3.cpp
					  src/python/unit_method_defs.cpp
					  src/python/unit_wrapper.cpp
					  src/python/universe_util_export.cpp )

SET(LIBSCRIPT_SOURCES src/cmd/script/director_generic.cpp
					  src/cmd/script/mission_script.cpp
					  src/cmd/script/mission.cpp
					  src/cmd/script/msgcenter.cpp
					  src/cmd/script/pythonmission.cpp
					  src/cmd/script/script_call_olist.cpp
					  src/cmd/script/script_call_omap.cpp
					  src/cmd/script/script_call_order.cpp
					  src/cmd/script/script_call_string.cpp
					  src/cmd/script/script_call_unit_generic.cpp
					  src/cmd/script/script_callbacks.cpp
					  src/cmd/script/script_expression.cpp
					  src/cmd/script/script_generic.cpp
					  src/cmd/script/script_statement.cpp
					  src/cmd/script/script_util.cpp
					  src/cmd/script/script_variables.cpp )

SET(LIBCMDSCRIPTSERVER_SOURCES src/cmd/script/flightgroup_server.cpp
							   src/cmd/script/script_call_unit_server.cpp
							   src/cmd/script/c_alike/c_alike.tab.cpp
							   src/cmd/script/c_alike/lex.yy.cpp )


SET(LIBAI_SOURCES src/cmd/ai/aggressive.cpp
				  src/cmd/ai/comm_ai.cpp
				  src/cmd/ai/communication_xml.cpp
				  src/cmd/ai/communication.cpp
				  src/cmd/ai/docking.cpp
				  src/cmd/ai/event_xml.cpp
				  src/cmd/ai/fire.cpp
				  src/cmd/ai/fireall.cpp
				  src/cmd/ai/flybywire.cpp
				  src/cmd/ai/hard_coded_scripts.cpp
				  src/cmd/ai/ikarus.cpp
				  src/cmd/ai/missionscript.cpp
				  src/cmd/ai/navigation.cpp
				  src/cmd/ai/order_comm.cpp
				  src/cmd/ai/order.cpp
				  src/cmd/ai/script.cpp
				  src/cmd/ai/tactics.cpp
				  src/cmd/ai/turretai.cpp
				  src/cmd/ai/warpto.cpp )

add_library(OPcollide  src/cmd/collide2/Ice/IceAABB.cpp
			   		   src/cmd/collide2/Ice/IceContainer.cpp
			   		   src/cmd/collide2/Ice/IceHPoint.cpp
			   		   src/cmd/collide2/Ice/IceIndexedTriangle.cpp
			   		   src/cmd/collide2/Ice/IceMatrix3x3.cpp
			   		   src/cmd/collide2/Ice/IceMatrix4x4.cpp
			   		   src/cmd/collide2/Ice/IceOBB.cpp
			   		   src/cmd/collide2/Ice/IcePlane.cpp
			   		   src/cmd/collide2/Ice/IcePoint.cpp
			   		   src/cmd/collide2/Ice/IceRandom.cpp
			   		   src/cmd/collide2/Ice/IceRay.cpp
			   		   src/cmd/collide2/Ice/IceRevisitedRadix.cpp
			   		   src/cmd/collide2/Ice/IceSegment.cpp
			   		   src/cmd/collide2/Ice/IceTriangle.cpp
			   		   src/cmd/collide2/Ice/IceUtils.cpp
			   		   src/cmd/collide2/csgeom2/opbox.cpp
			   		   src/cmd/collide2/csgeom2/opmatrix3.cpp
			   		   src/cmd/collide2/csgeom2/opvector3.cpp
			   		   src/cmd/collide2/OPC_AABBCollider.cpp
			   		   src/cmd/collide2/OPC_AABBTree.cpp
			   		   src/cmd/collide2/OPC_BaseModel.cpp
			   		   src/cmd/collide2/OPC_BoxPruning.cpp
			   		   src/cmd/collide2/OPC_Collider.cpp
			   		   src/cmd/collide2/OPC_Common.cpp
			   		   src/cmd/collide2/OPC_HybridModel.cpp
			   		   src/cmd/collide2/OPC_LSSCollider.cpp
			   		   src/cmd/collide2/OPC_MeshInterface.cpp
			   		   src/cmd/collide2/OPC_Model.cpp
			   		   src/cmd/collide2/OPC_OBBCollider.cpp
			   		   src/cmd/collide2/OPC_OptimizedTree.cpp
			   		   src/cmd/collide2/OPC_Picking.cpp
			   		   src/cmd/collide2/OPC_PlanesCollider.cpp
			   		   src/cmd/collide2/OPC_RayCollider.cpp
			   		   src/cmd/collide2/OPC_SphereCollider.cpp
			   		   src/cmd/collide2/OPC_SweepAndPrune.cpp
			   		   src/cmd/collide2/OPC_TreeBuilders.cpp
			   		   src/cmd/collide2/OPC_TreeCollider.cpp
			   		   src/cmd/collide2/OPC_VolumeCollider.cpp
			   		   src/cmd/collide2/CSopcodecollider.cpp )

SET(LIBCMD_SOURCES src/cmd/alphacurve.cpp
				   src/cmd/asteroid_generic.cpp
				   src/cmd/beam_generic.cpp
				   src/cmd/bolt_generic.cpp
				   src/cmd/building_generic.cpp
				   src/cmd/collection.cpp
				   src/cmd/collide_map.cpp
				   src/cmd/collide.cpp
				   src/cmd/container.cpp
				   src/cmd/csv.cpp
				   src/cmd/faction_xml.cpp
				   src/cmd/missile_generic.cpp
				   src/cmd/mount.cpp
				   src/cmd/nebula_generic.cpp
				   src/cmd/planet_generic.cpp
				   src/cmd/role_bitmask.cpp
				   src/cmd/unit_bsp.cpp
				   src/cmd/unit_collide.cpp
				   src/cmd/unit_const_cache.cpp
				   src/cmd/unit_csv.cpp
				   src/cmd/unit_factory_generic.cpp
				   src/cmd/unit_functions_generic.cpp
				   src/cmd/unit_generic.cpp
				   src/cmd/fg_util.cpp
				   src/cmd/pilot.cpp
				   src/cmd/unit_util_generic.cpp
				   src/cmd/unit_xml.cpp
				   src/cmd/weapon_xml.cpp )

SET(LIBCMDSERVER_SOURCES src/cmd/atmosphere_server.cpp
						 src/cmd/base_util_server.cpp
						 src/cmd/beam_server.cpp
						 src/cmd/bolt_server.cpp
						 src/cmd/unit_factory_server.cpp
						 src/cmd/unit_functions_server.cpp
						 src/cmd/unit_util_server.cpp )


SET(LIBGFXGENERIC_SOURCES src/gfx/bounding_box.cpp
						  src/gfx/bsp.cpp
						  src/gfx/cockpit_generic.cpp
						  src/gfx/lerp.cpp
						  src/gfx/matrix.cpp
						  src/gfx/mesh_bxm.cpp
						  src/gfx/mesh_bin.cpp
						  src/gfx/mesh_poly.cpp
						  src/gfx/mesh_xml.cpp
						  src/gfx/mesh.cpp
						  src/gfx/quaternion.cpp
						  src/gfx/sphere_generic.cpp
						  src/gfx/vec.cpp )

SET(LIBGFXCLIENT_SOURCES src/gfx/ani_texture.cpp
						 src/gfx/animation.cpp
						 src/gfx/aux_logo.cpp
						 src/gfx/aux_palette.cpp
						 src/gfx/aux_texture.cpp
						 src/gfx/background.cpp
						 src/gfx/camera.cpp
						 src/gfx/cockpit_xml.cpp
						 src/gfx/cockpit.cpp
						 src/gfx/coord_select.cpp
						 src/gfx/env_map_gent.cpp
						 src/gfx/gauge.cpp
						 src/gfx/halo_system.cpp
						 src/gfx/halo.cpp
						 src/gfx/hud.cpp
						 src/gfx/jpeg_memory.cpp
						 src/gfx/loc_select.cpp
						 src/gfx/masks.cpp
						 src/gfx/mesh_fx.cpp
						 src/gfx/mesh_gfx.cpp
						 src/gfx/nav/criteria_xml.cpp
						 src/gfx/nav/criteria.cpp
						 src/gfx/nav/drawgalaxy.cpp
						 src/gfx/nav/drawlist.cpp
						 src/gfx/nav/drawsystem.cpp
						 src/gfx/nav/navcomputer.cpp
						 src/gfx/nav/navgetxmldata.cpp
						 src/gfx/nav/navpath.cpp
						 src/gfx/nav/navscreen.cpp
						 src/gfx/nav/navscreenoccupied.cpp
						 src/gfx/particle.cpp
						 src/gfx/pipelined_texture.cpp
						 src/gfx/quadsquare_cull.cpp
						 src/gfx/quadsquare_render.cpp
						 src/gfx/quadsquare_update.cpp
						 src/gfx/quadsquare.cpp
						 src/gfx/quadtree_xml.cpp
						 src/gfx/quadtree.cpp
						 src/gfx/ring.cpp
						 src/gfx/screenshot.cpp
						 src/gfx/sphere.cpp
						 src/gfx/sprite.cpp
						 src/gfx/star.cpp
						 src/gfx/stream_texture.cpp
						 src/gfx/technique.cpp
						 src/gfx/tex_transform.cpp
						 src/gfx/vdu.cpp
						 src/gfx/vid_file.cpp
						 src/ffmpeg_init.cpp
						 src/gfx/vsbox.cpp
						 src/gfx/vsimage.cpp
						 src/gfx/warptrail.cpp )

SET(LIBGFXSERVER_SOURCES src/gfx/sphere_server.cpp
						 src/gfx/vsbox_server.cpp
						 src/gfx/mesh_server.cpp
						 src/gfx/background_server.cpp )


#add_library(posh  src/posh.cpp )

SET(LIBROOTGENERIC_SOURCES src/configxml.cpp
						   src/easydom.cpp
						   src/endianness.cpp
						   src/macosx_math.cpp
						   src/faction_generic.cpp
						   src/faction_util_generic.cpp
						   src/galaxy_gen.cpp
						   src/galaxy_xml.cpp
						   src/galaxy.cpp
						   src/hashtable.cpp
						   src/lin_time.cpp
						   src/load_mission.cpp
						   src/pk3.cpp
						   src/posh.cpp
						   src/savegame.cpp
						   src/star_system_generic.cpp
						   src/star_system_xml.cpp
						   src/stardate.cpp
						   src/universe_generic.cpp
						   src/universe_util_generic.cpp
						   src/vs_globals.cpp
						   src/common/common.cpp
						   src/vsfilesystem.cpp
						   src/VSFileXMLSerializer.cpp
						   src/xml_serializer.cpp
						   src/xml_support.cpp
						   src/XMLDocument.cpp
						   src/options.cpp
                           src/log.cpp
                           src/unicode.cpp
                           src/unicode_combine.cpp
                           src/multimap.cpp
                           src/vs_log_modules.cpp)

SET(LIBROOTCLIENT_SOURCES src/cg_global.cpp
						  src/command.cpp
						  src/config_xml.cpp
						  src/debug_vs.cpp
						  src/faction_util.cpp
						  src/force_feedback.cpp
						  src/gamemenu.cpp
						  src/gfxlib_struct.cpp
						  src/in_joystick.cpp
						  src/in_kb.cpp
						  src/in_main.cpp
						  src/in_mouse.cpp
						  src/in_sdl.cpp
						  src/main_loop.cpp
						  src/physics.cpp
						  src/rendertext.cpp
						  src/ship_commands.cpp
						  src/star_system_jump.cpp
						  src/star_system.cpp
						  src/universe_util.cpp
						  src/universe.cpp)

SET(LIBROOTSERVER_SOURCES  src/faction_util_server.cpp
						   src/force_feedback_server.cpp
						   src/gfxlib_struct_server.cpp
						   src/libaudioserver.cpp
						   src/libserver.cpp
						   src/universe_util_server.cpp )

SET(LIBAUDIO_SOURCES src/audio/CodecRegistry.cpp
					 src/audio/Stream.cpp
					 src/audio/Sound.cpp
					 src/audio/Source.cpp
					 src/audio/SourceTemplate.cpp
					 src/audio/SceneManager.cpp
					 src/audio/Renderer.cpp
					 src/audio/utils.cpp
					 src/audio/RenderableSource.cpp
					 src/audio/Listener.cpp
					 src/audio/Scene.cpp
					 src/audio/SimpleSource.cpp
					 src/audio/SimpleScene.cpp
					 src/audio/codecs/Codec.cpp
					 src/audio/codecs/FFStream.cpp
					 src/audio/codecs/OggStream.cpp
					 src/audio/codecs/OggData.cpp
					 src/audio/codecs/OggCodec.cpp
					 src/audio/codecs/FFCodec.cpp )


add_library(engine_com ${LIBAI_SOURCES}
					   ${LIBCMD_SOURCES}
					   ${LIBNET_SOURCES}
					   ${LIBPYTHON_SOURCES}
					   ${LIBROOTGENERIC_SOURCES}
					   ${LIBSCRIPT_SOURCES}
					   ${LIBGFXGENERIC_SOURCES} )

SET(VEGASTRIKE_SOURCES ${LIBGUI_SOURCES}
					   ${LIBNETCLIENT_SOURCES}
					   ${LIBROOTCLIENT_SOURCES}
					   ${LIBGFXCLIENT_SOURCES}
					   ${LIBAUDIO_SOURCES}
					   src/aldrv/al_globals.cpp
					   src/aldrv/al_init.cpp
					   src/aldrv/al_listen.cpp
					   src/aldrv/al_sound.cpp
					   src/cmd/ai/firekeyboard.cpp
					   src/cmd/ai/flyjoystick.cpp
					   src/cmd/ai/flykeyboard.cpp
					   src/cmd/ai/input_dfa.cpp
					   src/cmd/asteroid.cpp
					   src/cmd/atmosphere.cpp
					   src/cmd/base_init.cpp
					   src/cmd/base_interface.cpp
					   src/cmd/base_util.cpp
					   src/cmd/base_write_python.cpp
					   src/cmd/base_write_xml.cpp
					   src/cmd/base_xml.cpp
					   src/cmd/basecomputer.cpp
					   src/cmd/beam.cpp
					   src/cmd/bolt.cpp
					   src/cmd/briefing.cpp
					   src/cmd/building.cpp
					   src/cmd/click_list.cpp
					   src/cmd/cont_terrain.cpp
					   src/cmd/music.cpp
					   src/cmd/nebula.cpp
					   src/cmd/planet.cpp
					   src/cmd/script/c_alike/c_alike.tab.cpp
					   src/cmd/script/c_alike/lex.yy.cpp
					   src/cmd/script/director.cpp
					   src/cmd/script/flightgroup.cpp
					   src/cmd/script/script_call_briefing.cpp
					   src/cmd/script/script_call_unit.cpp
					   src/cmd/terrain.cpp
					   src/cmd/unit_factory.cpp
					   src/cmd/unit_functions.cpp
					   src/cmd/unit_interface.cpp
					   src/cmd/unit_util.cpp
					   src/gldrv/gl_program.cpp
					   src/gldrv/gl_clip.cpp
					   src/gldrv/gl_fog.cpp
					   src/gldrv/gl_globals.cpp
					   src/gldrv/gl_init.cpp
					   src/gldrv/gl_light_pick.cpp
					   src/gldrv/gl_light_state.cpp
					   src/gldrv/gl_light.cpp
					   src/gldrv/gl_material.cpp
					   src/gldrv/gl_matrix.cpp
					   src/gldrv/gl_misc.cpp
					   src/gldrv/gl_quad_list.cpp
					   src/gldrv/gl_sphere_list.cpp
					   src/gldrv/gl_state.cpp
					   src/gldrv/sdds.cpp
					   src/gldrv/gl_texture.cpp
					   src/gldrv/gl_vertex_list.cpp
					   src/gldrv/winsys.cpp
					   src/main.cpp
					   src/python/briefing_wrapper.cpp )

if (WIN32 AND NOT VS_WIN32_CONSOLE)
    set(VEGASTRIKE_SOURCES ${VEGASTRIKE_SOURCES} Resources.rc)
endif()

SET(VEGASERVER_SOURCES ${LIBCMDSERVER_SOURCES}
					   ${LIBNETSERVER_SOURCES}
					   ${LIBROOTSERVER_SOURCES}
					   ${LIBCMDSCRIPTSERVER_SOURCES}
					   ${LIBGFXSERVER_SOURCES}
					   src/vegaserver.cpp
					   src/cmd/script/director_server.cpp
					   src/gldrv/gl_vertex_list.cpp
					   src/gldrv/gl_sphere_list_server.cpp)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

###
message (STATUS "Building on system ${CMAKE_SYSTEM_NAME}")

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    SET(HOSTOS "APPLE")
    SET(MACOSX 1)
    SET(__APPLE 1)
    SET(WITH_MACOSX_BUNDLE 1)
    #add_definitions("-D_THREAD_SAFE" "-D_REENTRANT")
    #add_definitions("-D_GLIBCXX_USE_CXX11_ABI=0")
    IF(NOT VS_MORE_LINK_FLAGS)
        #SET(VS_MORE_LINK_FLAGS "-force_flat_namespace -Xlinker -multiply_defined -Xlinker warning -falign-loops=8 -falign-jumps=8 -falign-functions=8")
        #"-falign-loops=2" "-falign-jumps=2" "-falign-functions=2" ### ORIG on 32bits hosts
    ENDIF(NOT VS_MORE_LINK_FLAGS)
else()
  if(UNIX)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		SET(HOSTOS "LINUX")
		SET(_GNU_SOURCE 1)
	else(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		SET(HOSTOS "BSD")
	endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

  elseif(WIN32)
	SET(HOSTOS "WIN32")
  else(UNIX)
	SET(HOSTOS "APPLE")
    SET(MACOSX 1)
    SET(__APPLE 1)
    SET(WITH_MACOSX_BUNDLE 1)
  endif(UNIX)
endif()

## Get cmake command line arguments
get_cmake_property(CACHE_VARS CACHE_VARIABLES)
foreach(CACHE_VAR ${CACHE_VARS})
  get_property(CACHE_VAR_HELPSTRING CACHE ${CACHE_VAR} PROPERTY HELPSTRING)
  if(CACHE_VAR_HELPSTRING STREQUAL "No help, variable specified on the command line.")
    get_property(CACHE_VAR_TYPE CACHE ${CACHE_VAR} PROPERTY TYPE)
    if(CACHE_VAR_TYPE STREQUAL "UNINITIALIZED")
      set(CACHE_VAR_TYPE)
    else()
      set(CACHE_VAR_TYPE :${CACHE_VAR_TYPE})
    endif()
    set(CMAKE_COMMAND_ARGS "${CMAKE_COMMAND_ARGS} -D${CACHE_VAR}${CACHE_VAR_TYPE}=\"${${CACHE_VAR}}\"")
  endif()
endforeach()
string(REPLACE "\"" "\\\"" CMAKE_COMMAND_ARGS "${CMAKE_COMMAND_ARGS}")

##############################################################################
# CMAKE_BUILD_TYPE
#
if(NOT VS_OPTIMIZE)
    set(VS_OPTIMIZE "-O2")
endif(NOT VS_OPTIMIZE)
if(NOT VS_DEBUG_LEVEL)
    set(VS_DEBUG_LEVEL "-g2")
endif(NOT VS_DEBUG_LEVEL)
# Debug
SET(CMAKE_CXX_FLAGS_DEBUG " -DBOOST_PYTHON_NO_PY_SIGNATURES -DVS_DEBUG_LOG -include config.h -pipe ${VS_DEBUG_LEVEL} -Wall -Wno-unused-function -Wno-unused-variable -O0 -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C++ compiler during debug builds."
    FORCE )
SET( CMAKE_C_FLAGS_DEBUG " -DBOOST_PYTHON_NO_PY_SIGNATURES -DVS_DEBUG_LOG -include config.h -pipe ${VS_DEBUG_LEVEL} -Wall -Wno-unused-function -Wno-unused-variable -O0 -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C compiler during debug builds."
    FORCE )

# Release
SET(CMAKE_CXX_FLAGS_RELEASE " -DBOOST_PYTHON_NO_PY_SIGNATURES -DNDEBUG -include config.h -pipe ${VS_OPTIMIZE} -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C++ compiler during release builds."
    FORCE )
SET( CMAKE_C_FLAGS_RELEASE " -DBOOST_PYTHON_NO_PY_SIGNATURES -DNDEBUG -include config.h -pipe ${VS_OPTIMIZE} -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C compiler during release builds."
    FORCE )

# NativeRelease
SET(CMAKE_CXX_FLAGS_NATIVERELEASE " -DBOOST_PYTHON_NO_PY_SIGNATURES -DNDEBUG -include config.h -pipe ${VS_OPTIMIZE} -march=native -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C++ compiler during release builds."
    FORCE )
SET( CMAKE_C_FLAGS_NATIVERELEASE " -DBOOST_PYTHON_NO_PY_SIGNATURES -DNDEBUG -include config.h -pipe ${VS_OPTIMIZE} -march=native -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C compiler during release builds."
    FORCE )

# RelWithDebInfo
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO " -DBOOST_PYTHON_NO_PY_SIGNATURES -DVS_DEBUG_LOG -include config.h -pipe ${VS_OPTIMIZE} ${VS_DEBUG_LEVEL} -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C++ compiler during release builds - optimized with debug info."
    FORCE )
SET( CMAKE_C_FLAGS_RELWITHDEBINFO " -DBOOST_PYTHON_NO_PY_SIGNATURES -DVS_DEBUG_LOG -include config.h -pipe ${VS_OPTIMIZE} ${VS_DEBUG_LEVEL} -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C compiler during release builds - optimized with debug info."
    FORCE )

# Setup maintainer mode flags
SET(CMAKE_CXX_FLAGS_MAINTAINER " -DBOOST_PYTHON_NO_PY_SIGNATURES -DVS_DEBUG_LOG -include config.h -pipe ${VS_DEBUG_LEVEL} -Wall -Wno-unused-function -Wno-unused-variable -O0 -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C++ compiler during maintainer builds."
    FORCE )
SET( CMAKE_C_FLAGS_MAINTAINER " -DBOOST_PYTHON_NO_PY_SIGNATURES -DVS_DEBUG_LOG -include config.h -pipe ${VS_DEBUG_LEVEL} -Wall -Wno-unused-function -Wno-unused-variable -O0 -fvisibility=hidden ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C compiler during maintainer builds."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    "" CACHE STRING
    "Flags used for linking binaries during maintainer builds."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
    "" CACHE STRING
    "Flags used by the shared libraries linker during maintainer builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_MAINTAINER
    CMAKE_C_FLAGS_MAINTAINER
    CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    CMAKE_SHARED_LINKER_FLAGS_MAINTAINER )

# Setup profiler mode flags
SET(CMAKE_CXX_FLAGS_PROFILER "-pg ${VS_DEBUG_LEVEL} -DBOOST_PYTHON_NO_PY_SIGNATURES -include config.h -pipe -Wall -Wno-unused-function -Wno-unused-variable ${VS_OPTIMIZE} ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C++ compiler during profiler builds."
    FORCE )
SET( CMAKE_C_FLAGS_PROFILER "-pg ${VS_DEBUG_LEVEL} -DBOOST_PYTHON_NO_PY_SIGNATURES -include config.h -pipe -Wall -Wno-unused-function -Wno-unused-variable ${VS_OPTIMIZE} ${CMAKE_CXX_FLAGS}" CACHE STRING
    "Flags used by the C compiler during profiler builds."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_PROFILER
    "" CACHE STRING
    "Flags used for linking binaries during profiler builds."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_PROFILER
    "" CACHE STRING
    "Flags used by the shared libraries linker during profiler builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_PROFILER
    CMAKE_C_FLAGS_PROFILER
    CMAKE_EXE_LINKER_FLAGS_PROFILER
    CMAKE_SHARED_LINKER_FLAGS_PROFILER )

# Set the default build type to maintainer
IF(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
ENDIF(CMAKE_BUILD_TYPE)
IF(NOT CMAKE_BUILD_TYPE
        OR NOT (CMAKE_BUILD_TYPE MATCHES "^\(MAINTAINER|DEBUG|RELEASE|NATIVERELEASE|RELWITHDEBINFO|MINSIZEREL|PROFILER\)$"))
  SET(CMAKE_BUILD_TYPE "MAINTAINER" CACHE STRING
      "Choose the type of build, options are: None Debug Release NativeRelease RelWithDebInfo MinSizeRel Maintainer Profiler."
      FORCE)
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES "[Mm][Aa][Ii][Nn][Tt][Aa][Ii][Nn][Ee][Rr]")
  ADD_DEFINITIONS(-DGCC_HASVISIBILITY)
ENDIF(CMAKE_BUILD_TYPE MATCHES "[Mm][Aa][Ii][Nn][Tt][Aa][Ii][Nn][Ee][Rr]")
##############################################################################

# Report CXX standard
message(STATUS "Using CXX Standard: ${CMAKE_CXX_STANDARD}")

##############################################################################
# Main executables
IF(NOT DISABLE_CLIENT)
	add_executable(vegastrike ${VEGASTRIKE_SOURCES})
ENDIF(NOT DISABLE_CLIENT)

add_executable(vegaserver ${VEGASERVER_SOURCES})

##############################################################################
## VSA PAckage info
find_package(PkgConfig)

include (CheckIncludeFile)
include (CheckIncludeFileCXX)

# Let cmake find our in-tree modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${vsUTCS_SOURCE_DIR}/cmake")

# Find PYTHON2
### With pkg-config
foreach(tmp_python_pkg "Python2" "python2" "Python<3" "python<3" "python" "Python")
    pkg_check_modules(PYTHON ${tmp_python_pkg})
    if(PYTHON_FOUND)
        set(PYTHON_LIBRARY ${PYTHON_LINK_LIBRARIES})
        set(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS})
        find_program(PYTHON_EXECUTABLE NAMES python${PYTHON_VERSION} python HINTS ${PYTHON_PREFIX}/bin;${PYTHON_PREFIX})
        break()
    endif()
endforeach()
if(NOT PYTHON_LIBRARY OR NOT PYTHON_INCLUDE_DIR)
    find_package(PythonInterp 2) # deprecated, use Python or Pyhton2 (CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    find_package(PythonLibs 2) # deprecated, use Python or Pyhton2 (CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    if(PythonLibs_FOUND)
        SET(PYTHON_LIBRARY ${PYTHON_LIBRARIES})
        SET(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS})
    endif()
endif()
IF(NOT VEGASTRIKE_LIBDIR_NAME)
    SET(VEGASTRIKE_LIBDIR_NAME "lib")
ENDIF()
IF(PYTHON_INCLUDE_DIR AND PYTHON_LIBRARY)
    SET(TST_INCLUDES ${PYTHON_INCLUDE_DIR})
    SET(TST_LIBS ${PYTHON_LIBRARY})
    SET(HAVE_PYTHON 1)
    message(STATUS "  Python includes:${PYTHON_INCLUDE_DIR}, libs:${PYTHON_LIBRARY}, bin:${PYTHON_EXECUTABLE}")
    # As we don't want to use all python libs in dev mode, we need to specify them one by one.
    # In release(bundle) mode, no issue, as we provide only the libs we want.
    SET(VEGASTRIKE_PYTHON_DYNLIB_PATH "${VEGASTRIKE_LIBDIR_NAME}/pythonlibs")
    SET(VS_PYTHON_DYNLIB_BUILDDIR "${CMAKE_BINARY_DIR}/${VEGASTRIKE_PYTHON_DYNLIB_PATH}")
    SET(VS_PYTHON_DYNLIB_NAMES _collections _io _socket _symtable datetime fcntl operator select
        time _functools _scproxy _struct binascii dbm itertools math readline termios unicodedata
        _csv _ssl _hashlib _random pyexpat parser _subprocess)
    unset(tmp_pythonlibs)
    file (MAKE_DIRECTORY "${VS_PYTHON_DYNLIB_BUILDDIR}")
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -E -c "import sys; print(\";\".join(sys.path))"
                    OUTPUT_VARIABLE VS_PYTHON_DYNLIB_PATHS OUTPUT_STRIP_TRAILING_WHITESPACE)
    FOREACH(tmp_dyn_name ${VS_PYTHON_DYNLIB_NAMES})
        find_file(tmp_dyn_path NAMES ${tmp_dyn_name}.so ${tmp_dyn_name}.dll ${tmp_dyn_name}.dylib ${tmp_dyn_name}.pyd
                  HINTS ${VS_PYTHON_DYNLIB_PATHS})
        if (tmp_dyn_path)
            get_filename_component(tmp_basename "${tmp_dyn_path}" NAME)
            set(tmp_pythonlibs "${tmp_pythonlibs} ${tmp_basename}")
            if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.14)
                file(CREATE_LINK "${tmp_dyn_path}" "${VS_PYTHON_DYNLIB_BUILDDIR}/${tmp_basename}" SYMBOLIC COPY_ON_ERROR)
            else()
                file(COPY "${tmp_dyn_path}" DESTINATION "${VS_PYTHON_DYNLIB_BUILDDIR}")
            endif()
        endif()
        unset(tmp_dyn_path CACHE)
    ENDFOREACH()
    message(STATUS "  python dynamic libs:${tmp_pythonlibs}")
ELSE(PYTHON_INCLUDE_DIR AND PYTHON_LIBRARY)
    message(FATAL_ERROR "Can't find python")
ENDIF(PYTHON_INCLUDE_DIR AND PYTHON_LIBRARY)

# Find Boost with boost_python library
#SET(BOOST_INTERNAL 0)
IF(NOT BOOST_INTERNAL) ## VSA commented to force internal
    find_package(Boost COMPONENTS python)
ENDIF(NOT BOOST_INTERNAL) ## VSA commented to force internal
IF(Boost_FOUND)
    SET(BOOST_PYTHON_NO_PY_SIGNATURES 1)
	SET(TST_INCLUDES ${TST_INCLUDES} ${Boost_INCLUDE_DIRS})
	SET(TST_LIBS ${Boost_LIBRARIES} ${TST_LIBS})
	include_directories(${TST_INCLUDES})
ELSE(Boost_FOUND)
    SET(BOOST_PYTHON_NO_PY_SIGNATURES 1)
	IF(NOT BOOST_INTERNAL)
        IF(CMAKE_CXX_STANDARD LESS 11)
            SET(BOOST_INTERNAL 1_35)
        ELSE(APPLE)
            SET(BOOST_INTERNAL 1_50)
        ENDIF(APPLE)
    ENDIF(NOT BOOST_INTERNAL)
    add_definitions("-DBOOST_PYTHON_STATIC_LIB" )
    message("-- Using Internal Boost ! (" ${BOOST_INTERNAL} ")")

    SET(TST_INCLUDES ${TST_INCLUDES} ${vsUTCS_SOURCE_DIR}/boost/${BOOST_INTERNAL})
    include_directories(${TST_INCLUDES})
	add_subdirectory(boost/${BOOST_INTERNAL})
	SET(TST_LIBS boost_python ${TST_LIBS})
	IF(NOT DISABLE_CLIENT)
		add_dependencies(vegastrike boost_python)
	ENDIF(NOT DISABLE_CLIENT)
	add_dependencies(vegaserver boost_python)
ENDIF(Boost_FOUND)

IF(NOT DISABLE_CLIENT) ##########

#Find GL
find_package(OpenGL REQUIRED)
IF(OPENGL_FOUND AND OPENGL_GLU_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${OPENGL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${OPENGL_LIBRARIES})
	message("-- OpenGL found : ${OPENGL_LIBRARIES}")
ELSE(OPENGL_FOUND AND OPENGL_GLU_FOUND)
	message("Why you no have GL?")
ENDIF(OPENGL_FOUND AND OPENGL_GLU_FOUND)

#Find AGL
IF(MACOSX)
    find_library(AGL_LIBRARY AGL)
    IF(AGL_LIBRARY)
        #find_path(AGL_INCLUDE_DIR AGL/agl.h DOC "Include for AGL on OS X")
        #SET(TST_INCLUDES ${TST_INCLUDES} ${AGL_INCLUDE_DIR})
        #SET(TST_LIBS ${TST_LIBS} ${AGL_LIBRARY})
        message("-- AGL found : ${AGL_LIBRARY}")
    ELSE(AGL_FOUND)
        message("// AGL not found")
    ENDIF(AGL_LIBRARY)
ENDIF(MACOSX)

#Find GLUT
if(MACOSX)
    find_library(GLUT_LIBRARIES GLUT)
    find_path(GLUT_INCLUDE_DIR GLUT/glut.h)
    IF(GLUT_LIBRARIES AND GLUT_INCLUDE_DIR)
        SET(GLUT_FOUND 1)
    ENDIF(GLUT_LIBRARIES AND GLUT_INCLUDE_DIR)
else(MACOSX)
    find_package(GLUT REQUIRED)
endif(MACOSX)
IF(GLUT_FOUND)
    SET(TST_INCLUDES ${TST_INCLUDES} ${GLUT_INCLUDE_DIR})
    SET(TST_LIBS ${TST_LIBS} ${GLUT_LIBRARIES})
    message("-- GLUT found : ${GLUT_LIBRARIES}")
ELSE(GLUT_FOUND)
    message("I can't build this, missing GLUT")
ENDIF(GLUT_FOUND)

# Force static libraries
SET(VS_BAK_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
IF(VS_STATIC_LIBS)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
ENDIF(VS_STATIC_LIBS)

#Find OpenAL
find_package(OpenAL REQUIRED)
IF(OPENAL_FOUND)
	message("-- Found OpenAL")
	SET(TST_INCLUDES ${TST_INCLUDES} ${OPENAL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${OPENAL_LIBRARY})
	SET(HAVE_AL 1)
ELSE(OPENAL_FOUND)
	message("We aint got no sound")
ENDIF(OPENAL_FOUND)

#Find SDL
IF(NOT SDL_DISABLE)
    find_package(SDL)
ENDIF()
IF(SDL_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${SDL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${SDL_LIBRARY})
    message("-- SDL Found (includes:${SDL_INCLUDE_DIR} libs:${SDL_LIBRARIES}")
	SET(HAVE_SDL 1)
    IF(NOT SDL_WINDOWING_DISABLE)
        SET(SDL_WINDOWING 1)
    ENDIF(NOT SDL_WINDOWING_DISABLE)
ELSE(SDL_FOUND)
	message("How will we render to OpenGL without SDL?")
ENDIF(SDL_FOUND)

#find Vorbis
find_package(Vorbis REQUIRED)
IF(Vorbis_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${Vorbis_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${Vorbis_LIBRARIES})
	SET(HAVE_OGG 1)
ELSE(Vorbis_FOUND)
	message("Can't find Vorbis libs")
ENDIF(Vorbis_FOUND)

#Find JPEG # VSA MOVED because of Include dir priority
find_package(JPEG REQUIRED)
IF(JPEG_FOUND)
    SET(HAVE_JPEG 1)
	SET(TST_INCLUDES ${JPEG_INCLUDE_DIR} ${TST_INCLUDES})
	SET(TST_LIBS ${TST_LIBS} ${JPEG_LIBRARIES})
    message("-- JPEG INCLUDES: ${JPEG_INCLUDE_DIR}")
ELSE(JPEG_FOUND)
	message("How are we gonna open jpegs?")
ENDIF(JPEG_FOUND)

#Find PNG # VSA MOVED because of Include dir priority
find_package(PNG REQUIRED)
IF(PNG_FOUND)
    SET(HAVE_PNG 1)
	SET(TST_INCLUDES ${PNG_INCLUDE_DIRS} ${TST_INCLUDES})
	SET(TST_LIBS ${TST_LIBS} ${PNG_LIBRARIES})
    message("-- PNG INCLUDES: ${PNG_INCLUDE_DIRS}")
ELSE(PNG_FOUND)
	message("Can't find PNG lib")
ENDIF(PNG_FOUND)

ENDIF(NOT DISABLE_CLIENT) ##########

#find Zlib
find_package(ZLIB REQUIRED)
IF(ZLIB_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${ZLIB_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${ZLIB_LIBRARIES})
ELSE(ZLIB_FOUND)
	message("For the love of god, no zlib here")
ENDIF(ZLIB_FOUND)

#find Expat
find_package(EXPAT REQUIRED)
IF(EXPAT_FOUND)
    SET(HAVE_EXPAT 1)
	SET(TST_INCLUDES ${TST_INCLUDES} ${EXPAT_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${EXPAT_LIBRARIES})
ELSE(EXPAT_FOUND)
	message("We can't find expat for XML crap")
ENDIF(EXPAT_FOUND)

#find Math
find_package(MATH REQUIRED)
IF(MATH_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${MATH_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${MATH_LIBRARIES})
ELSE(MATH_FOUND)
	message("Without math we have nothing")
ENDIF(MATH_FOUND)

#Find FFMpeg
find_package(FFMPEG)
IF(FFMPEG_FOUND)
	SET(TST_INCLUDES ${FFMPEG_INCLUDE_DIRS} ${TST_INCLUDES})
	SET(TST_LIBS ${TST_LIBS} ${FFMPEG_LIBRARIES})
    add_definitions(${FFMPEG_DEFINITIONS})
    SET(HAVE_FFMPEG 1 )
ELSE(FFMPEG_FOUND)
	message("What the hell, no FFMpeg")
ENDIF(FFMPEG_FOUND)

FIND_LIBRARY(DL_LIB dl)
FIND_LIBRARY(UTIL_LIB util)
if (WIN32)
  FIND_LIBRARY(REGEX_LIB regex)
  FIND_LIBRARY(SHLWAPI_LIB shlwapi)
endif()

if (DL_LIB)
    SET(TST_LIBS ${TST_LIBS} ${DL_LIB})
endif(DL_LIB)
if (UTIL_LIB)
    SET(TST_LIBS ${TST_LIBS} ${UTIL_LIB})
endif(UTIL_LIB)
if (REGEX_LIB)
    SET(TST_LIBS ${TST_LIBS} ${REGEX_LIB})
endif(REGEX_LIB)
if (SHLWAPI_LIB)
    SET(TST_LIBS ${TST_LIBS} ${SHLWAPI_LIB})
endif(SHLWAPI_LIB)

# Still need to add CEGUI and OGRE find packages


#These are boolean options the user can enable/disable via ccmake
OPTION(USE_NET_THREAD_POSIX "Enable Posix net threads" OFF)
if(NOT USE_NET_THREAD_POSIX)
	SET(USE_NET_THREAD_NONE 1)
endif(NOT USE_NET_THREAD_POSIX)



#config.h generation
INCLUDE(CheckFunctionExists)
INCLUDE (CheckSymbolExists)
CHECK_INCLUDE_FILE(wchar.h HAVE_WCHAR_H)
CHECK_INCLUDE_FILE(locale.h HAVE_LOCALE_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(wctype.h HAVE_WCTYPE_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)
CHECK_INCLUDE_FILE(zlib.h HAVE_ZLIB_H)
CHECK_INCLUDE_FILE(math.h HAVE_MATH_H)
CHECK_INCLUDE_FILE(fnmatch.h HAVE_FNMATCH_H)
CHECK_INCLUDE_FILE(regex.h HAVE_REGEX_H)

if(DEFINED CMAKE_REQUIRED_FLAGS)
    set(saved_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
endif()
set(CMAKE_REQUIRED_FLAGS "-std=gnu++${CMAKE_CXX_STANDARD}")
CHECK_INCLUDE_FILE_CXX(climits HAVE_CLIMITS)
CHECK_INCLUDE_FILE_CXX(cwctype HAVE_CWCTYPE)
CHECK_INCLUDE_FILE_CXX(cwchar HAVE_CWCHAR)
CHECK_INCLUDE_FILE_CXX(clocale HAVE_CLOCALE)
CHECK_INCLUDE_FILE_CXX(locale HAVE_LOCALE)
CHECK_INCLUDE_FILE_CXX(codecvt HAVE_CODECVT)
CHECK_INCLUDE_FILE_CXX(unordered_map HAVE_UNORDERED_MAP)
CHECK_INCLUDE_FILE_CXX(tr1/unordered_map HAVE_TR1_UNORDERED_MAP)
### test source sample
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
#include <atomic>
int main() {
  std::atomic<int> u{5};
  return u;
}" HAVE_STDLIB_ATOMIC)
if(DEFINED saved_CMAKE_REQUIRED_FLAGS)
    set(CMAKE_REQUIRED_FLAGS ${saved_CMAKE_REQUIRED_FLAGS})
    unset(saved_CMAKE_REQUIRED_FLAGS)
else()
    unset(CMAKE_REQUIRED_FLAGS)
endif()

CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
if(HAVE_MATH_H)
    CHECK_SYMBOL_EXISTS(isnan math.h HAVE_ISNAN)
	if(NOT HAVE_ISNAN)
		CHECK_FUNCTION_EXISTS(_isnan HAVE__ISNAN)
	endif(NOT HAVE_ISNAN)
    CHECK_SYMBOL_EXISTS(isfinite math.h HAVE_ISFINITE)
    CHECK_SYMBOL_EXISTS(finite math.h HAVE_FINITE)
	if(NOT HAVE_FINITE)
		CHECK_FUNCTION_EXISTS(_finite HAVE__finite)
	endif(NOT HAVE_FINITE)
endif(HAVE_MATH_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_STAT_H)
if(HAVE_STAT_H)
	CHECK_FUNCTION_EXISTS(lstat HAVE_LSTAT)
endif(HAVE_STAT_H)

CHECK_SYMBOL_EXISTS(GLX_ARB_get_proc_address GL/glx.h HAVE_GLXGETPROCADDRESSARB)

if (WIN32)
    set(SYMBOL_LOOKUP_INCLUDES "windows.h;string.h;stdlib.h;stdio.h")
    if (CYGWIN OR MINGW)
        set(SYMBOL_LOOKUP_INCLUDES "${SYMBOL_LOOKUP_INCLUDES};io.h")
    else()
        set(SYMBOL_LOOKUP_INCLUDES "${SYMBOL_LOOKUP_INCLUDES};direct.h")
    endif()
else()
    set(SYMBOL_LOOKUP_INCLUDES "stdlib.h;string.h;unistd.h;stdio.h")
endif()

CHECK_SYMBOL_EXISTS(strcasecmp "${SYMBOL_LOOKUP_INCLUDES}" HAVE_STRCASECMP)
CHECK_SYMBOL_EXISTS(strncasecmp "${SYMBOL_LOOKUP_INCLUDES}" HAVE_STRNCASECMP)
CHECK_SYMBOL_EXISTS(stricmp "${SYMBOL_LOOKUP_INCLUDES}" HAVE_STRICMP)
CHECK_SYMBOL_EXISTS(strnicmp "${SYMBOL_LOOKUP_INCLUDES}" HAVE_STRNICMP)
CHECK_SYMBOL_EXISTS(setenv "${SYMBOL_LOOKUP_INCLUDES}" HAVE_SETENV)
if (NOT HAVE_SETENV)
    CHECK_FUNCTION_EXISTS(setenv HAVE_SETENV)
endif()

## If you want to use SHGetKnownFolder instead of SHGetFolderPathW,
## -> replace FALSE by TRUE in following IF() but this will not work before Vista
if (FALSE AND WIN32)
    CHECK_SYMBOL_EXISTS(FOLDERID_LocalAppData "windows.h;shlobj.h" HAVE_FOLDERID_LOCALAPPDATA)
    CHECK_SYMBOL_EXISTS(FOLDERID_AppData "windows.h;shlobj.h" HAVE_FOLDERID_APPDATA)
    CHECK_SYMBOL_EXISTS(SHGetKnownFolderPath "windows.h;shlobj.h" HAVE_SHGETKNOWNFOLDERPATH)
endif()

## VSA / need update in
##            * cmake/config.h.in (add HAVE_INTMAX_T, then config.h is filled according to variable HAVE_INTMAX_T)
##            * src/cmd/collide2/opcodetypes.h: define CS_HAVE_INTMAX_T according to HAVE_INTMAX_T)
if(DEFINED CMAKE_EXTRA_INCLUDE_FILES)
    set(saved_CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES})
endif()
if (WIN32)
    set(CMAKE_EXTRA_INCLUDE_FILES "windows.h")
endif()
include (CheckTypeSize)
check_type_size(intmax_t INTMAX_T)
check_type_size(uintmax_t UINTMAX_T)
check_type_size(ptrdiff_t PTRDIFF_T)
check_type_size(boolean BOOLEAN)
if(DEFINED saved_CMAKE_EXTRA_INCLUDE_FILES)
    set(CMAKE_EXTRA_INCLUDE_FILES ${saved_CMAKE_EXTRA_INCLUDE_FILES})
    unset(saved_CMAKE_EXTRA_INCLUDE_FILES)
else()
    unset(CMAKE_EXTRA_INCLUDE_FILES)
endif()
## ! VSA

find_library(PTHREAD_LIBRARY NAMES pthread)

CONFIGURE_FILE(${vsUTCS_SOURCE_DIR}/cmake/config.h.in ${vsUTCS_BINARY_DIR}/config.h)

#version.h generation
add_definitions("-DHAVE_VERSION_H")
add_custom_target(get_scm_revision ALL DEPENDS get_scm_revision_output)
add_custom_command(OUTPUT get_scm_revision_output # trick so that make considers as PHONY and always run it
                   COMMAND ${CMAKE_COMMAND} -DGETSCM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
                                            -DGETSCM_VERSION_FILE="${CMAKE_CURRENT_BINARY_DIR}/version.h"
                                            -DGETSCM_PRODUCT_VERSION="${VERSION}"
                                            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GetSCMrevision.cmake")
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/version.h" PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE DEPENDS get_scm_revision)

#end config.h generation
#SET(CMAKE_CXX_FLAGS "-include config.h;-pipe;"${CMAKE_CXX_FLAGS})

#Setup shared lib and include listings for vegastrike and vegaserver.
SET(TST_LIBS engine_com OPcollide netgeneric netlowlevel ${TST_LIBS})
if (WIN32)
    set(TST_LIBS ${TST_LIBS} wsock32 ws2_32)
endif (WIN32)
SET(TST_SERVER_LIBS netgeneric engine_com netlowlevel OPcollide ${TST_LIBS})
include_directories(${TST_INCLUDES})

if (PTHREAD_LIBRARY)
    SET(PTHREAD_LDFLAGS " -lpthread")
else()
    UNSET(PTHREAD_LDFLAGS)
endif()

IF(NOT DISABLE_CLIENT)
	target_link_libraries(vegastrike ${TST_LIBS})
    set_target_properties(vegastrike PROPERTIES LINK_FLAGS "-L/usr/lib ${VS_MORE_LINK_FLAGS}${PTHREAD_LDFLAGS}${VS_WINDOWS_APP_LDFLAGS}")
    #version.h generation
    add_dependencies(vegastrike get_scm_revision)
    if (WIN32 AND (CYGWIN OR MINGW) AND NOT VS_WIN32_CONSOLE)
        target_link_libraries(vegastrike "-mwindows")
        find_program(WINDRES windres)
    endif()
ENDIF(NOT DISABLE_CLIENT)

# Vssetup Sub build file
add_subdirectory(setup)
# Vslauncher Sub build file
add_subdirectory(launcher)

#ADD_CUSTOM_TARGET(vssetup DEPENDS setup/vssetup COMMAND ln -fs ${vsUTCS_BINARY_DIR}/setup/vssetup vssetup || true)

# Add other utilies here
add_subdirectory(objconv)
add_subdirectory(tools)
add_subdirectory(tests)
#ADD_CUSTOM_TARGET(mesher DEPENDS objconv/mesher COMMAND ln -fs ${vsUTCS_BINARY_DIR}/objconv/mesher mesher || true)


target_link_libraries(vegaserver ${TST_SERVER_LIBS})
set_target_properties(vegaserver PROPERTIES LINK_FLAGS "-L/usr/lib -L${vsUTCS_BINARY_DIR}${PTHREAD_LDFLAGS}")
add_dependencies(vegaserver mesh_tool)
add_dependencies(vegaserver get_scm_revision)
if (GTK2_FOUND)
    add_dependencies(vegaserver vssetup)
elseif (DIALOG_FOUND)
    add_dependencies(vegaserver vssetup_dlg)
endif(GTK2_FOUND)

# Debug crap
#message("Linking with : ${TST_LIBS}")
#message("Including : ${TST_INCLUDES}")
#message("including : ${TEMP_DIRECTORY}")
get_directory_property(TEMP_DIRECTORIES INCLUDE_DIRECTORIES)
IF(NOT DISABLE_CLIENT)
    set(TEMP_CFLAGS "${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
    #get_target_property(TEMP_CFLAGS0 vegastrike INCLUDE_DIRECTORIES)
    #get_target_property(TEMP_CFLAGS1 vegastrike COMPILE_OPTIONS)
    #get_target_property(TEMP_CFLAGS2 vegastrike COMPILE_FLAGS)
    #get_target_property(TEMP_CFLAGS3 vegastrike COMPILE_DEFINITIONS)
    #get_target_property(TEMP_CFLAGS4 vegastrike COMPILE_FEATURES)
    #set(TEMP_CFLAGS "${TEMP_CFLAGS0} ${TEMP_CFLAGS1} ${TEMP_CFLAGS2} ${TEMP_CFLAGS3} ${TEMP_CFLAGS4}")
    get_target_property(TEMP_LDFLAGS     vegastrike LINK_FLAGS)
    get_target_property(TEMP_LIBRARIES   vegastrike LINK_LIBRARIES)
ELSE()
    set(TEMP_CFLAGS "${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}} ${TST_INCLUDES}")
    #get_directory_property(TEMP_CFLAGS1    COMPILE_OPTIONS)
    #get_directory_property(TEMP_CFLAGS2    COMPILE_FLAGS)
    #get_directory_property(TEMP_CFLAGS3    COMPILE_DEFINITIONS)
    #get_directory_property(TEMP_CFLAGS4    COMPILE_FEATURES)
    #set(TEMP_CFLAGS "${TEMP_CFLAGS} ${TEMP_CFLAGS1} ${TEMP_CFLAGS2} ${TEMP_CFLAGS3} ${TEMP_CFLAGS4}")
    set(TEMP_LDFLAGS ${TST_LIBS})
    set(TEMP_LIBRARIES "")
ENDIF()
message("")
message("----------------------------------------")
message(STATUS "Building with:")
message(STATUS "  CXXFLAGS (${CMAKE_BUILD_TYPE}) : ${TEMP_CFLAGS}")
message(STATUS "  INCLUDES : ${TEMP_DIRECTORIES}")
message(STATUS "  LIBS     : ${TEMP_LDFLAGS} ${TEMP_LIBRARIES}")
message("----------------------------------------")
message("")
# End Debug Crap

