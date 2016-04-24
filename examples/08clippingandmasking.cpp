/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    08clippingandmasking.cpp
        Clipping and masking example
*/

#include "kcanvas/canvas.h"


using namespace c_util;
using namespace k_canvas;


const char *TITLE = "kcanvas - Clipping and masking example";


void Initialize()
{}


static void ClipByPathExample(kCanvas &canvas)
{
    kPen pen(kColor(63, 72, 204), 4);
    kBrush brush(kColor(130, 218, 255));
    kFont font("Tahoma", 30, kFontStyle::Bold);

    kPath textpath;
    textpath.MoveTo(kPoint(10, 20));
    textpath.Text("Text clip path", -1, font);
    textpath.Commit();

    canvas.BeginClippedDrawing(textpath, kTransform::construct::translate(-15, 0));

    canvas.Rectangle(kRect(10, 25, 50, 65), &pen, &brush);
    canvas.RoundedRectangle(kRect(60, 25, 100, 65), kSize(5, 5), &pen, &brush);
    canvas.Ellipse(kRect(110, 25, 150, 65), &pen, &brush);

    const kPoint points[] = {
        kPoint(180, 25),
        kPoint(200, 45),
        kPoint(180, 65),
        kPoint(160, 45)
    };
    canvas.Polygon(points, 4, &pen, &brush);

    canvas.EndClippedDrawing();
}

static void ClipByMaskExample(kCanvas &canvas)
{
    // TODO: clip by mask example
}


void Example(kCanvas &canvas)
{
    ClipByPathExample(canvas);

    canvas.SetTransform(kTransform::construct::translate(250, 0));
    ClipByMaskExample(canvas);
}

void Shutdown()
{}
