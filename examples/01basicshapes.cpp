/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    01basicshapes.cpp
        Baisc shapes example
*/

#include "kcanvas/canvas.h"


using namespace k_canvas;


const char *TITLE = "kcanvas - Basic shapes example";


void Initialize()
{}

void Example(kCanvas &canvas)
{
    kPen pen(kColor(0, 162, 232), 5);

    canvas.Line(kPoint(10, 10), kPoint(50, 50), pen);
    canvas.Bezier(
        kPoint(60, 10), kPoint(90, 10),
        kPoint(100, 20), kPoint(100, 50),
        pen
    );
    canvas.Arc(kRect(110, 10, 150, 50), 0, 270, pen);

    const kPoint linepoints[] = {
        kPoint(180, 10),
        kPoint(200, 30),
        kPoint(180, 50),
        kPoint(160, 30)
    };
    canvas.PolyLine(linepoints, 4, pen);

    const kPoint linepointsbezier[] = {
        kPoint(230, 10),

        kPoint(232, 20),
        kPoint(240, 28),
        kPoint(250, 30),

        kPoint(240, 32),
        kPoint(232, 40),
        kPoint(230, 50),

        kPoint(228, 40),
        kPoint(220, 30),
        kPoint(210, 30)
    };
    canvas.PolyBezier(linepointsbezier, 10, pen);
}

void Shutdown()
{}
