/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    08clippingandmasking.cpp
        Clipping and masking example
*/

#include "kcanvas/canvas.h"
#include "common/bmp.h"


using namespace c_util;
using namespace k_canvas;


const char *TITLE = "kcanvas - Clipping and masking example";


// this is global variables, don't use this in real projects ;)
// they are used only for simplicity of examples
static kBitmap *mask = nullptr;


void Initialize()
{
    // Example applications are supposed to be run from "bin" directory
    // inside kcanvas project directory, so all paths to media data are relative to
    // "bin" directory

    // Load mask (monochrome) bitmap
    {
        BitmapData data("../../examples/media/mask.bmp");
        if (data.bpp() == 8) {
            mask = new kBitmap(data.width(), data.height(), kBitmapFormat::Mask8Bit);
            mask->Update(nullptr, kBitmapFormat::Mask8Bit, data.pitch(), data.data());
        }
    }
}


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
    kPen pen(kColor(255, 127, 39), 4);
    kBrush brush(kColor(255, 201, 14));

    kTransform tfm;
    tfm.scale(0.5f, 0.5f);
    tfm.translateby(-25, 15);
    canvas.BeginClippedDrawing(*mask, tfm, kExtendType::Wrap, kExtendType::Wrap);

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


void Example(kCanvas &canvas)
{
    if (!mask) {
        kFont font("Tahoma", 12);
        kBrush brush(kColor::Black);

        canvas.Text(kPoint(5), "Couldn't load required media!", -1, font, brush);

        return;
    }

    ClipByPathExample(canvas);

    canvas.SetTransform(kTransform::construct::translate(250, 0));
    ClipByMaskExample(canvas);
}

void Shutdown()
{
    delete mask;
}
