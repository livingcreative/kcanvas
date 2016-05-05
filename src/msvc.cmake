#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015
#
#  https://github.com/livingcreative/kcanvas
#
#  msvc.cmake
#      cmake include file for MSVC specific build
#

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
	if (LIBRARY_OUT_PATH)
		set(LIBRARY_OUT_PATH "${LIBRARY_OUT_PATH}/$(Platform)")

		# this var is used to set PDB output path and name for kcanvas.lib
		set(LIB_PDB_DIR ${LIBRARY_OUT_PATH})
	endif ()

	# link libraries path
	if (LIBS_PATH)
		set(LIBS_PATH "${LIBS_PATH}/$(Platform)/$(Configuration)")
	endif ()

	# link libraries names
	if (LIBS)
		foreach (LIB ${LIBS})
			set(${LIB} ${LIB}.lib)
		endforeach ()
	endif ()

endif (MSVC)
