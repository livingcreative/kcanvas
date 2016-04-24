/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    common/example.h
        common example application interface
        every example needs to implement functions declared here
*/

#pragma once


namespace  k_canvas
{
    // forward declarations
    class kCanvas;
}


// TITLE should be defined in specific example, this text will be window title
extern const char *TITLE;


// Initialize - initialize example application
//      called once at application startup
void Initialize();

// Example - perform example painting on given canvas object
//      called every time application window needs to be repainted
void Example(k_canvas::kCanvas &canvas);

// Shutdown - shutdown example application
//      called once at application shutdown
void Shutdown();
