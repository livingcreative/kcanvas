/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    02basicfilledshapes.cpp
        Baisc filled shapes example
*/

#include "kcanvas/canvas.h"


using namespace k_canvas;


const char *TITLE = "kcanvas - Basic filled shapes example";


void Initialize()
{}

void Example(kCanvas &canvas)
{
    kPen pen(kColor(255, 127, 39), 4);
    kBrush brush(kColor(255, 201, 14));

    canvas.Rectangle(kRect(10, 10, 50, 50), &pen, &brush);
    canvas.RoundedRectangle(kRect(60, 10, 100, 50), kSize(5, 5), &pen, &brush);
    canvas.Ellipse(kRect(110, 10, 150, 50), &pen, &brush);

    const kPoint points[] = {
        kPoint(180, 10),
        kPoint(200, 30),
        kPoint(180, 50),
        kPoint(160, 30)
    };
    canvas.Polygon(points, 4, &pen, &brush);

    const kPoint pointsbezier[] = {
        kPoint(230, 10),

        kPoint(232, 20),
        kPoint(240, 28),
        kPoint(250, 30),

        kPoint(240, 32),
        kPoint(232, 40),
        kPoint(230, 50),

        kPoint(228, 40),
        kPoint(220, 32),
        kPoint(210, 30),

        kPoint(220, 28),
        kPoint(228, 20)
    };
    canvas.PolygonBezier(pointsbezier, 12, &pen, &brush);
}

void Shutdown()
{}
