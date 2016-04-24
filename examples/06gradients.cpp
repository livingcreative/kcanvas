/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    06gradients.cpp
        Gradients and gradient brushes example
*/

#include "kcanvas/canvas.h"


using namespace k_canvas;


const char *TITLE = "kcanvas - Gradients and gradient brushes example";


void Initialize()
{}

void Example(kCanvas &canvas)
{
    kFont font("Tahoma", 12);
    kBrush black(kColor::Black);


    canvas.Text(kPoint(), "Linear gradient", -1, font, black);

    kGradient gradient0(kColor(255, 177, 125), kColor(237, 28, 36));
    kGradient gradient1(kColor(130, 218, 255), kColor(63, 72, 204));

    kBrush linear0(kPoint(0, 25), kPoint(90, 115), gradient0);
    canvas.Rectangle(kRect(0, 25, 90, 115), nullptr, &linear0);

    kBrush linear1(kPoint(190, 25), kPoint(100, 115), gradient1);
    canvas.Rectangle(kRect(100, 25, 190, 115), nullptr, &linear1);


    canvas.Text(kPoint(200, 0), "Radial gradient", -1, font, black);

    kBrush radial0(kPoint(245, 70), kPoint(0, 0), kSize(40, 40), gradient0);
    canvas.Rectangle(kRect(200, 25, 290, 115), nullptr, &radial0);

    kBrush radial1(kPoint(345, 70), kPoint(20, 20), kSize(40, 40), gradient1);
    canvas.Rectangle(kRect(300, 25, 390, 115), nullptr, &radial1);


    canvas.Text(kPoint(400, 0), "Linear gradient 12 colors", -1, font, black);

    const kGradientStop stops[] = {
        kGradientStop(kColor(255, 0, 0), 0.08f),
        kGradientStop(kColor(255, 128, 0), 0.16f),
        kGradientStop(kColor(255, 255, 0), 0.25f),
        kGradientStop(kColor(128, 255, 0), 0.33f),
        kGradientStop(kColor(0, 255, 0), 0.41f),
        kGradientStop(kColor(0, 255, 128), 0.5f),
        kGradientStop(kColor(0, 255, 255), 0.58f),
        kGradientStop(kColor(0, 128, 255), 0.66f),
        kGradientStop(kColor(0, 0, 255), 0.75f),
        kGradientStop(kColor(128, 0, 255), 0.83f),
        kGradientStop(kColor(255, 0, 255), 0.91f),
        kGradientStop(kColor(255, 0, 128), 1)
    };
    kGradient gradient2(stops, 12);
    kBrush linear2(kPoint(400, 25), kPoint(590, 25), gradient2);
    canvas.Rectangle(kRect(400, 25, 590, 65), nullptr, &linear2);
}

void Shutdown()
{}
