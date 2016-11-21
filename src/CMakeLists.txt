#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015 - 2016
#
#  https://github.com/livingcreative/kcanvas
#
#  CMakeLists.txt
#      kcanvas API cmake project
#

cmake_minimum_required(VERSION 2.8)

project(kcanvas)

# relative path to public includes
set(INCLUDE_PATH ../include/kcanvas)

# relative path to library build destination
set(LIBRARY_OUT_PATH ${PROJECT_SOURCE_DIR}/../lib)


set(HEADERS
	# public include headers
	${INCLUDE_PATH}/canvas.h
	${INCLUDE_PATH}/canvastypes.h
	${INCLUDE_PATH}/canvasresources.h
	${INCLUDE_PATH}/canvasplatform.h

	# private source headers
	canvasimpl.h
	canvasimplplatform.h
)

set(SOURCES
	# platform independend sources
	canvas.cpp
	canvastypes.cpp
	canvasimpl.cpp
)

# Windows build
if (WIN32)
	include(win/windows.cmake)
endif ()


# MSVC specific build
if (MSVC)
	include(msvc.cmake)
endif ()

# GCC specific build
if (CMAKE_COMPILER_IS_GNUCXX)
	include(gcc.cmake)
endif ()

# public include directories
include_directories(${INCLUDE_PATH})

# kcommon dependencies
if (NOT kcommon)
	set(kcommon ../../kcommon/include)
endif ()
include_directories(${kcommon})


add_library(kcanvas STATIC ${SOURCES} ${HEADERS})
set_target_properties(
	kcanvas
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUT_PATH}
)
if (LIB_PDB_DIR)
	set_target_properties(
		kcanvas
		PROPERTIES
		PDB_OUTPUT_DIRECTORY ${LIB_PDB_DIR}
		PDB_NAME "kcanvas_vc$(PlatformToolsetVersion).pdb"
	)
endif ()
