/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvasplatform.h
        all platform specific includes and defines are hosted here
*/

#pragma once

// Cairo implementation (mostly for Linux)
#ifdef _CAIRO
#include "cairo/platform.h"
#else

// Windows platform implementation
#ifdef _WIN32
#include "win/platform.h"
#endif

#endif
