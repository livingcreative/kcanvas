/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    win/platform.cpp
        canvas Windows platform implementations descriptors
*/

#include "canvasimplgdiplus.h"
#include "canvasimpld2d.h"

namespace k_canvas
{
    namespace impl
    {

        CanvasFactory* CreateD2DFactory()
        {
            return new CanvasFactoryD2D();
        }

        CanvasFactory* CreateGDIPlusFactory()
        {
            return new CanvasFactoryGDIPlus();
        }

        FACTORY_DESCRIPTORS_BEGIN()
        FACTORY_DESCRIPTOR(IMPL_D2D, CreateD2DFactory)
        FACTORY_DESCRIPTOR(IMPL_GDIPLUS, CreateGDIPlusFactory)
        FACTORY_DESCRIPTORS_END()

    } // namespace impl
} // namespace k_canvas
