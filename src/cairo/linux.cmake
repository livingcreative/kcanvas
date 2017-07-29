#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015 - 2017
#
#  https://github.com/livingcreative/kcanvas
#
#  linux.cmake
#      cmake include file for Linux platform build
#

# windows platform build headers
set(HEADERS
	${HEADERS}
	${INCLUDE_PATH}/cairo/platform.h
	cairo/canvasimplcairo.h
)

# windows platform build sources
set(SOURCES
	${SOURCES}
	cairo/platform.cpp
	cairo/canvasimplcairo.cpp
)

add_definitions(-D_CAIRO)
