/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

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
    kPath path;

    // explicitly closed figure
    path.MoveTo(kPoint(10, 10));
    path.LineTo(kPoint(50, 10));
    path.LineTo(kPoint(50, 50));
    path.LineTo(kPoint(10, 50));
    path.Close();

    // opened figure (when stroking, line from
    // last point to first is not painted)
    path.MoveTo(kPoint(80, 10));
    path.LineTo(kPoint(120, 10));
    path.LineTo(kPoint(120, 50));
    path.LineTo(kPoint(80, 50));

    // explicitly closed figures
    path.MoveTo(kPoint(150, 10));
    path.LineTo(kPoint(210, 10));
    path.LineTo(kPoint(210, 50));
    path.LineTo(kPoint(150, 50));
    path.Close();

    path.MoveTo(kPoint(10, 70));
    path.LineTo(kPoint(130, 70));
    path.LineTo(kPoint(130, 150));
    path.LineTo(kPoint(10, 150));
    path.Close();

    path.MoveTo(kPoint(90, 110));
    path.LineTo(kPoint(210, 110));
    path.LineTo(kPoint(210, 190));
    path.LineTo(kPoint(90, 190));
    path.Close();

    path.MoveTo(kPoint(230, 10));
    path.ArcTo(kRect(230, 10, 330, 110), 0, 90);
    path.LineTo(kPoint(330, 110));

    // text
    kFont font("Tahoma", 30, kFontStyle::Bold); 
    path.MoveTo(kPoint(140, 110));
    path.Text("Text path", -1, font);

    path.Commit();

    kPen pen(kColor(36, 92, 196), 2);

    kGradient gradient(kColor(255, 177, 125), kColor(237, 28, 36));
    kBrush brush(kPoint(0, 0), kPoint(330, 190), gradient);

    canvas.DrawPath(path, &pen, &brush);
}

void Shutdown()
{}
