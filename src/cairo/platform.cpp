/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    cairo/platform.cpp
        canvas Cairo API implementation descriptors (mostly for Linux)
*/

#include "canvasimplcairo.h"


namespace k_canvas
{
    namespace impl
    {

        CanvasFactory* CreateCairoFactory()
        {
            return new CanvasFactoryCairo();
        }

        FACTORY_DESCRIPTORS_BEGIN()
        FACTORY_DESCRIPTOR(IMPL_CAIRO, CreateCairoFactory)
        FACTORY_DESCRIPTORS_END()

    } // namespace impl
} // namespace k_canvas
