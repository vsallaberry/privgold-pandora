#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
include_directories("..")

SET(TRISORT_SOURCES trisort.cpp)
add_executable(trisort ${TRISORT_SOURCES})

SET(ASTEROID_SOURCES asteroidgen.cpp)
add_executable(asteroidgen ${ASTEROID_SOURCES})

SET(REPLACE_SOURCES replace.cpp)
add_executable(replace ${REPLACE_SOURCES})

#find Expat
find_package(EXPAT REQUIRED)
IF(EXPAT_FOUND)
    SET(OBJCONV_EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIRS})
	SET(OBJCONV_EXPAT_LIBRARIES ${EXPAT_LIBRARIES})
ELSE(EXPAT_FOUND)
	message("We can't find expat for XML stuff")
ENDIF(EXPAT_FOUND)

SET(MESHER_SOURCES
	mesher/Converter.cpp
	mesher/from_BFXM.cpp
	mesher/from_obj.cpp
	mesher/main.cpp
	mesher/to_BFXM.cpp
	mesher/to_obj.cpp
	../src/xml_support.cpp
	mesher/Modules/BFXM_to_Wavefront.cpp
	mesher/Modules/BFXM_to_XMesh.cpp
	mesher/Modules/Convert.cpp
	mesher/Modules/Dims.cpp
	mesher/Modules/OldSyntax.cpp
	mesher/Modules/Wavefront_to_BFXM.cpp
	mesher/Modules/XMesh_to_BFXM.cpp
)
if(HAVE_OGRE)
    set(MESHER_SOURCES ${MESHER_SOURCES}
        mesher/to_OgreMesh.cpp
        mesher/Modules/XMesh_to_Ogre.cpp)
endif(HAVE_OGRE)

set(${MSH_INCLUDES} mesher)
add_executable(mesh_tool ${MESHER_SOURCES})
#set_source_files_properties(${VSSETUP_GTK_SOURCES} PROPERTIES COMPILE_OPTIONS "${GTK_CFLAGS}")
#target_compile_options(mesh_tool PUBLIC "${MSH_INCLUDES} ${OBJCONV_EXPAT_INCLUDE_DIRS}")
target_include_directories(mesh_tool PUBLIC ${MSH_INCLUDES} ${OBJCONV_EXPAT_INCLUDE_DIRS})
target_link_libraries(mesh_tool ${MSH_LIBS} ${OBJCONV_EXPAT_LIBRARIES})

set (TEMPGEN_SOURCES
  MeshParse/template_generator.cpp
  ../src/xml_support.cpp
  ../src/hashtable.cpp)
add_executable(tempgen ${TEMPGEN_SOURCES})
target_include_directories(mesh_tool PUBLIC ${OBJCONV_EXPAT_INCLUDE_DIRS})
target_link_libraries(tempgen ${OBJCONV_EXPAT_LIBRARIES})

set(MESH_XML_SOURCES
  MeshParse/template_generator.cpp
  ../src/xml_support.cpp)
add_executable(mesh_xml ${MESH_XML_SOURCES})
target_include_directories(mesh_xml PUBLIC ${OBJCONV_EXPAT_INCLUDE_DIRS})
target_link_libraries(mesh_xml ${OBJCONV_EXPAT_LIBRARIES})

set(BASE_MAKER_SOURCES
  basemaker/base_maker.cpp
  basemaker/base_maker_texture.cpp)
add_executable(base_maker ${BASE_MAKER_SOURCES})
target_link_libraries(base_maker ${TST_LIBS})

