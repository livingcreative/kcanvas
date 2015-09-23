/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    mac/platform.cpp
        OS X Quartz implementation descriptors
*/

#include "canvasimplquartz.h"


namespace k_canvas
{
    namespace impl
    {

        CanvasFactory* CreateQuartzFactory()
        {
            return new CanvasFactoryQuartz();
        }

        FACTORY_DESCRIPTORS_BEGIN()
        FACTORY_DESCRIPTOR(IMPL_QUARTZ, CreateQuartzFactory)
        FACTORY_DESCRIPTORS_END()

    } // namespace impl
} // namespace k_canvas
