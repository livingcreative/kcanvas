/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    cairo/platform.h
        Cairo implementation specific header (mostly for Linux)
*/

#pragma once


namespace k_canvas
{

    enum Impl
    {
        IMPL_NONE,
        IMPL_CAIRO // Cairo implementation
    };

    typedef void *kContext;
    typedef void *kPrinter;

} // namespace k_canvas
