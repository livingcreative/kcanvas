/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    07strokestyles.cpp
        Stroke styles and pens example
*/

#include "kcanvas/canvas.h"


using namespace c_util;
using namespace k_canvas;


const char *TITLE = "kcanvas - Stroke styles and pens example";


void Initialize()
{}


static void LineWidthExample(kCanvas &canvas)
{
    kFont font("Tahoma", 12);
    kBrush black(kColor::Black);

    const kScalar widths[6] = { 1, 2, 3, 8, 16, 30 };
    const kScalar offsets[6] = { 0.5f, 0, 0.5f, 0, 0, 0 };
    kScalar y = 25;
    for (int n = 0; n < 6; ++n) {
        kPen pen(kColor(196, 36, 92), widths[n]);
        canvas.Line(
            kPoint(10, y + offsets[n]),
            kPoint(110, y + offsets[n]), pen
        );

        char buf[32];
        sprintf(buf, "%.fpx", widths[n]);
        canvas.Text(
            kPoint(120, y + offsets[n] - 10),
            buf, -1, font, black
        );

        y += umax(widths[n], kScalar(10)) * 2;
    }
}

static void LineStyleExample(kCanvas &canvas)
{
    kFont font("Tahoma", 12);
    kBrush black(kColor::Black);

    canvas.Text(kPoint(120, 15), "Solid", -1, font, black);
    canvas.Text(kPoint(120, 30), "Dot", -1, font, black);
    canvas.Text(kPoint(120, 45), "Dash", -1, font, black);
    canvas.Text(kPoint(120, 60), "Dash dot", -1, font, black);
    canvas.Text(kPoint(120, 75), "Dash dot dot", -1, font, black);
    canvas.Text(kPoint(120, 90), "Flat cap", -1, font, black);
    canvas.Text(kPoint(120, 105), "Square cap", -1, font, black);
    canvas.Text(kPoint(120, 120), "Round cap", -1, font, black);

    const kStrokeStyle styles[5] = {
        kStrokeStyle::Solid, kStrokeStyle::Dot, kStrokeStyle::Dash,
        kStrokeStyle::DashDot, kStrokeStyle::DashDotDot
    };

    const kScalar linewidth = 4;

    kScalar y = 25;
    for (int n = 0; n < 5; ++n) {
        kPen pen(kColor(196, 36, 92), linewidth, styles[n]);
        canvas.Line(kPoint(10, y), kPoint(110, y), pen);
        y += 15;
    }

    kStroke flat(kStrokeStyle::Solid, kLineJoin::Miter);
    kPen pen0(kColor(36, 196, 92), 10, flat);
    canvas.Line(kPoint(10, y), kPoint(110, y), pen0);
    y += 15;

    kStroke square(
        kStrokeStyle::Solid, kLineJoin::Miter,
        kCapStyle::Square, kCapStyle::Square, kCapStyle::Square
    );
    kPen pen1(kColor(36, 196, 92), 10, square);
    canvas.Line(kPoint(10, y), kPoint(110, y), pen1);
    y += 15;

    kStroke round(
        kStrokeStyle::Solid, kLineJoin::Miter,
        kCapStyle::Round, kCapStyle::Round, kCapStyle::Round
    );
    kPen pen2(kColor(36, 196, 92), 10, round);
    canvas.Line(kPoint(10, y), kPoint(110, y), pen2);
    y += 15;

    kStroke rounddash(
        kStrokeStyle::Dash, kLineJoin::Miter,
        kCapStyle::Flat, kCapStyle::Flat, kCapStyle::Round
    );
    kPen pen3(kColor(36, 196, 92), linewidth, rounddash);
    canvas.Line(kPoint(10, y), kPoint(110, y), pen3);
    y += 15;

    kStroke rounddashdot(
        kStrokeStyle::DashDot, kLineJoin::Miter,
        kCapStyle::Flat, kCapStyle::Flat, kCapStyle::Round
    );
    kPen pen4(kColor(36, 196, 92), linewidth, rounddashdot);
    canvas.Line(kPoint(10, y), kPoint(110, y), pen4);
    y += 15;

    kStroke rounddot(
        kStrokeStyle::Dot, kLineJoin::Miter,
        kCapStyle::Flat, kCapStyle::Flat, kCapStyle::Round
    );
    kPen pen5(kColor(36, 196, 92), linewidth, rounddot);
    canvas.Line(kPoint(10, y), kPoint(110, y), pen5);
}

static void LineJoinExample(kCanvas &canvas)
{
    kFont font("Tahoma", 12);
    kBrush black(kColor::Black);

    const kPoint linepoints[] = {
        kPoint(30, 30),
        kPoint(50, 50),
        kPoint(30, 170),
        kPoint(10, 50)
    };


    canvas.Text(kPoint(10, 200), "Miter", -1, font, black);

    kStroke miter(kStrokeStyle::Solid, kLineJoin::Miter);
    kPen penmiter(kColor(36, 92, 196), 10, miter);
    canvas.PolyLine(linepoints, 4, penmiter);


    kTransform t = kTransform::construct::translate(60, 0);
    canvas.PushTransform(t);

    canvas.Text(kPoint(10, 200), "Bevel", -1, font, black);

    kStroke bevel(kStrokeStyle::Solid, kLineJoin::Bevel);
    kPen penbevel(kColor(36, 92, 196), 10, bevel);
    canvas.PolyLine(linepoints, 4, penbevel);


    t.translateby(60, 0);
    canvas.SetTransform(t);

    canvas.Text(kPoint(10, 200), "Round", -1, font, black);

    kStroke round(kStrokeStyle::Solid, kLineJoin::Round);
    kPen penround(kColor(36, 92, 196), 10, round);
    canvas.PolyLine(linepoints, 4, penround);

    canvas.PopTransform();
}


void Example(kCanvas &canvas)
{
    LineWidthExample(canvas);

    canvas.SetTransform(kTransform::construct::translate(200, 0));
    LineStyleExample(canvas);

    canvas.SetTransform(kTransform::construct::translate(450, 0));
    LineJoinExample(canvas);
}

void Shutdown()
{}
