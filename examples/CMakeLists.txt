#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015
#
#  https://github.com/livingcreative/kcanvas
#
#  examples/CMakeLists.txt
#      kcanvas API examples cmake project
#

cmake_minimum_required(VERSION 2.8)

project(kcanvasexamples)

# relative path to public includes
set(INCLUDE_PATH ../include)

# relative path to executabels build destination
set(BINARY_OUT_PATH ${PROJECT_SOURCE_DIR}/../bin)

# relative path to libs
set(LIBS_PATH ${PROJECT_SOURCE_DIR}/../lib)


set(HEADERS
	# public include headers
	${INCLUDE_PATH}/kcanvas/canvas.h

	# private source headers
	common/example.h
	common/bmp.h
)

set(SOURCES
	# platform independend sources
	common/bmp.cpp
)

set(LIBS
	# kcanvas library
	kcanvas
)

# Windows build
if (WIN32)
	include(common/win/windows.cmake)
	set(PLATFORMPROP WIN32)
endif ()


# MSVC specific build
if (MSVC)
	include(../src/msvc.cmake)
endif ()


# public include directories
include_directories(${INCLUDE_PATH})

# kcommon dependencies
if (NOT kcommon)
	set(kcommon ../../kcommon/include)
endif ()
include_directories(${kcommon})

# library dependencies
link_directories(${LIBS_PATH})


# basic shapes demo
add_executable(basicshapes ${PLATFORMPROP} ${SOURCES} "01basicshapes.cpp" ${HEADERS})
set_target_properties(
	basicshapes
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(basicshapes ${LIBS})

# basic filled shapes demo
add_executable(basicfilledshapes ${PLATFORMPROP} ${SOURCES} "02basicfilledshapes.cpp" ${HEADERS})
set_target_properties(
	basicfilledshapes
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(basicfilledshapes ${LIBS})

# text and fonts demo
add_executable(textandfonts ${PLATFORMPROP} ${SOURCES} "03textandfonts.cpp" ${HEADERS})
set_target_properties(
	textandfonts
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(textandfonts ${LIBS})

# paths demo
add_executable(paths ${PLATFORMPROP} ${SOURCES} "04paths.cpp" ${HEADERS})
set_target_properties(
	paths
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(paths ${LIBS})

# bitmaps demo
add_executable(bitmaps ${PLATFORMPROP} ${SOURCES} "05bitmaps.cpp" ${HEADERS})
set_target_properties(
	bitmaps
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(bitmaps ${LIBS})

# gradients demo
add_executable(gradients ${PLATFORMPROP} ${SOURCES} "06gradients.cpp" ${HEADERS})
set_target_properties(
	gradients
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(gradients ${LIBS})

# stroke styles demo
add_executable(strokestyles ${PLATFORMPROP} ${SOURCES} "07strokestyles.cpp" ${HEADERS})
set_target_properties(
	strokestyles
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(strokestyles ${LIBS})

# clipping and masking demo
add_executable(clippingandmasking ${PLATFORMPROP} ${SOURCES} "08clippingandmasking.cpp" ${HEADERS})
set_target_properties(
	clippingandmasking
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(clippingandmasking ${LIBS})
