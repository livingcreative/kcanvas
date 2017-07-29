/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    04paths.cpp
        Paths example
*/

#include "kcanvas/canvas.h"


using namespace k_canvas;


const char *TITLE = "kcanvas - Paths example";


void Initialize()
{}

void Example(kCanvas &canvas)
{
    kFont font("Tahoma", 30, kFontStyle::Bold); 

    kPath path = kPath::Create()

        // explicitly closed figure
        .MoveTo(kPoint(10, 10))
        .LineTo(kPoint(50, 10))
        .LineTo(kPoint(50, 50))
        .LineTo(kPoint(10, 50))
        .Close()

        // opened figure (when stroking, line from
        // last point to first is not painted)
        .MoveTo(kPoint(80, 10))
        .LineTo(kPoint(120, 10))
        .LineTo(kPoint(120, 50))
        .LineTo(kPoint(80, 50))

        // explicitly closed figures
        .MoveTo(kPoint(150, 10))
        .LineTo(kPoint(210, 10))
        .LineTo(kPoint(210, 50))
        .LineTo(kPoint(150, 50))
        .Close()

        .MoveTo(kPoint(10, 70))
        .LineTo(kPoint(130, 70))
        .LineTo(kPoint(130, 150))
        .LineTo(kPoint(10, 150))
        .Close()

        .MoveTo(kPoint(90, 110))
        .LineTo(kPoint(210, 110))
        .LineTo(kPoint(210, 190))
        .LineTo(kPoint(90, 190))
        .Close()

        .MoveTo(kPoint(230, 10))
        .ArcTo(kRect(230, 10, 330, 110), 0, 90)
        .LineTo(kPoint(330, 110))


        // text
        .MoveTo(kPoint(140, 110))
        .Text("Text path", -1, font)

        .Build();

    kPen pen(kColor(36, 92, 196), 2);

    kGradient gradient(kColor(255, 177, 125), kColor(237, 28, 36));
    kBrush brush(kPoint(0, 0), kPoint(330, 190), gradient);

    canvas.DrawPath(path, &pen, &brush);
}

void Shutdown()
{}
