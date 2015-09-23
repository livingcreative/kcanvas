/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    mac/platform.h
        OS X Quartz implementation specific header
*/

#pragma once


namespace k_canvas
{

    enum Impl
    {
        IMPL_NONE,
        IMPL_QUARTZ // Quartz implementation
    };

    typedef void *kContext;
    typedef void *kPrinter;

} // namespace k_canvas
