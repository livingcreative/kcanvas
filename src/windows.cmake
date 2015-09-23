#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015
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

# Visual Studio specific defines and options
if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_USE_MATH_DEFINES)

	set(CONFIGS
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_MINSIZEREL
		CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_MINSIZEREL
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
	)

	# check if runtime specified, default is "static"
	if (NOT msvcruntime)
		set(msvcruntime "static")
	endif ()

	# replace runtime to static in all configurations, if defined
	if (msvcruntime STREQUAL "static")
		foreach (CONF ${CONFIGS})
			if (${CONF} MATCHES "/MD")
				string(REGEX REPLACE "/MD" "/MT" ${CONF} "${${CONF}}")
			endif ()
		endforeach ()
	endif ()

	# library output to "lib" directory
	set(LIBRARY_OUT_PATH "${LIBRARY_OUT_PATH}/$(Platform)")

endif (MSVC)
