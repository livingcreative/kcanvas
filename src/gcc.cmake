#
#      KCANVAS PROJECT
#
#  Common 2D graphics API abstraction with multiple back-end support
#
#  (c) livingcreative, 2015 - 2016
#
#  https://github.com/livingcreative/kcanvas
#
#  gcc.cmake
#      cmake include file for GCC specific build
#

# GCC specific defines and options
if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (CMAKE_COMPILER_IS_GNUCXX)
