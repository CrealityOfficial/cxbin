__cc_find(Trimesh2)
__cc_find(mmesh)
__cc_find(Boost)
__cc_find(Zlib)
__cc_find(Zip)
__cc_find(Stringutil)
__cc_find(Tinyxml)
__cc_find(Lib3mf)
__cc_find(ColladaDom)
__cc_find(ImageProc)
__cc_find(FreeImage)
__assert_target(trimesh2)
__assert_target(zlib)
__assert_target(stringutil)
__assert_target(tinyxml)

set(SRCS cxbin/interface.h
		 cxbin/load.h
		 cxbin/save.h
		 cxbin/convert.h
		 cxbin/loaderimpl.h
		 cxbin/saverimpl.h
		 cxbin/format.h
		 cxbin/analysis.h
		 cxbin/impl/load.cpp
		 cxbin/impl/save.cpp
		 cxbin/impl/convert.cpp
		 cxbin/impl/cxbinmanager.h
		 cxbin/impl/cxbinmanager.cpp
		 cxbin/impl/inner.h
		 cxbin/impl/inner.cpp
		 cxbin/impl/format.cpp
		 cxbin/impl/analysis.cpp
		 cxbin/impl/loaderimpl.cpp
		 cxbin/plugin/pluginstl.h
		 cxbin/plugin/pluginstl.cpp
		 cxbin/plugin/pluginply.h
		 cxbin/plugin/pluginply.cpp
		 cxbin/plugin/pluginobj.h
		 cxbin/plugin/pluginobj.cpp
		 cxbin/plugin/plugin3ds.h
		 cxbin/plugin/plugin3ds.cpp
		 cxbin/plugin/pluginoff.h
		 cxbin/plugin/pluginoff.cpp	
		 cxbin/plugin/pluginwrl.h
		 cxbin/plugin/pluginwrl.cpp
		 cxbin/plugin/pluginoff.cpp
		 )
		 
set(LIBS trimesh2 stringutil tinyxml zlib boost_nowide imageproc mmesh freeImage)
set(DEFS TIXML_USE_STL Z_PREFIX BOOST_ALL_NO_LIB)

if(CC_BC_EMCC)
    list(APPEND DEFS TRIMESH_MAPBUF_RGB)
endif()

if(NOT CC_BC_WIN)
	list(APPEND DEFS __USE_FILE_OFFSET64)
endif()

if(TARGET lib3MF)
	list(APPEND LIBS lib3MF zip)
	list(APPEND SRCS cxbin/plugin/plugin3mf.h
					 cxbin/plugin/plugin3mf.cpp
					 )
endif()

if(TARGET lib3mf)
	list(APPEND LIBS lib3mf zip)
	list(APPEND SRCS cxbin/plugin/plugin3mf.h
					 cxbin/plugin/plugin3mf.cpp
					 )
endif()

if(TARGET colladadom)
	list(APPEND SRCS cxbin/plugin/plugindaedom.h
					 cxbin/plugin/plugindaedom.cpp
			)

	list(APPEND LIBS colladadom colladadom141 boost_filesystem)
	list(APPEND DEFS DAE_USE_COLLDADA_DOM)
else()
	list(APPEND SRCS cxbin/plugin/plugindae.h
					 cxbin/plugin/plugindae.cpp
			)
endif()

if(CC_GLOBAL_FORCE_STATIC)
	list(APPEND DEFS FREEIMAGE_LIB)
endif()

__add_common_library(cxbin)

if(CC_INSTALL_CXBIN)
	__install_directory_specif(${CMAKE_CURRENT_SOURCE_DIR}/ cxbin)
endif()
