#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015 - 2016
#
#  https://github.com/livingcreative/kcanvas
#
#  windows.cmake
#      cmake include file for Windows platform build
#

# windows platform build headers
set(HEADERS
	${HEADERS}
	${INCLUDE_PATH}/win/platform.h
	win/canvasimpld2d.h
	win/canvasimplgdiplus.h
)

# windows platform build sources
set(SOURCES
	${SOURCES}
	win/platform.cpp
	win/canvasimpld2d.cpp
	win/canvasimplgdiplus.cpp
)
