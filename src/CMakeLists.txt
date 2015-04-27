#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015
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

# public include directories
include_directories(${INCLUDE_PATH})

# kcommon dependencies
if (NOT kcommon)
	set(kcommon ../../kcommon/include)
endif ()
include_directories(${kcommon})

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
	include(windows.cmake)
endif ()

add_library(kcanvas STATIC ${SOURCES} ${HEADERS})
set_target_properties(
	kcanvas
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUT_PATH}
)