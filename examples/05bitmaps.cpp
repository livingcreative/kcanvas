/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    05bitmaps.cpp
        Bitmaps example
*/

#include "kcanvas/canvas.h"
#include "common/bmp.h"


using namespace c_util;
using namespace k_canvas;


const char *TITLE = "kcanvas - Bitmaps example";


// this is global variables, don't use this in real projects ;)
// they are used only for simplicity of examples
static kBitmap *bitmap = nullptr;
static kBitmap *mask = nullptr;


void Initialize()
{
    // Example applications are supposed to be run from "bin" directory
    // inside kcanvas project directory, so all paths to media data are relative to
    // "bin" directory

    // Load color bitmap with alpha channel
    {
        BitmapData data("../../examples/media/stone.bmp");
        if (data.bpp() == 24) {
            bitmap = new kBitmap(data.width(), data.height(), kBitmapFormat::Color32BitAlphaPremultiplied);
            bitmap->Update(nullptr, kBitmapFormat::Color32BitAlphaPremultiplied, data.pitch(), data.data());
        }
    }

    // Load mask (monochrome) bitmap
    {
        BitmapData data("../../examples/media/mask.bmp");
        if (data.bpp() == 8) {
            mask = new kBitmap(data.width(), data.height(), kBitmapFormat::Mask8Bit);
            mask->Update(nullptr, kBitmapFormat::Mask8Bit, data.pitch(), data.data());
        }
    }
}


static void DrawBitmapExample(kCanvas &canvas)
{
    // paint whole bitmap at (10, 10) with 0.75 opacity
    canvas.DrawBitmap(*bitmap, kPoint(10, 10), 0.75f);
    // paint part of the bitmap
    canvas.DrawBitmap(*bitmap, kPoint(50, 40), kPoint(40, 30), kSize(80));
    // paint scaled part of the bitmap
    canvas.DrawBitmap(
        *bitmap,
        kPoint(270, 10), kSize(256),
        kPoint(40, 30), kSize(80)
    );
}

static void FillMaskExample(kCanvas &canvas)
{
    kGradient gradient0(kColor(255, 177, 125), kColor(237, 28, 36));
    kGradient gradient1(kColor(130, 218, 255), kColor(63, 72, 204));
    kBrush fill0(kPoint(10, 10), kPoint(230, 410), gradient0);
    kBrush fill1(kPoint(10, 10), kPoint(230, 245), gradient1);

    for (int n = 0; n < 5; ++n) {
        canvas.DrawMask(
            *mask, n & 1 ? fill0 : fill1,
            kPoint(kScalar(n * 30) + 10, 10), kSize(kScalar(n * 10) + 50),
            kPoint(), mask->size()
        );
    }
}

static void BitmapBrushExample(kCanvas &canvas)
{
    kBrush brush(kExtendType::Wrap, kExtendType::Wrap, *bitmap);
    canvas.RoundedRectangle(kRect(10, 25, 100, 115), kSize(10), nullptr, &brush);
    canvas.Ellipse(kRect(120, 25, 210, 115), nullptr, &brush);
}


void Example(kCanvas &canvas)
{
    if (!bitmap || !mask) {
        kFont font("Tahoma", 12);
        kBrush brush(kColor::Black);

        canvas.Text(kPoint(5), "Couldn't load required media!", -1, font, brush);

        return;
    }

    DrawBitmapExample(canvas);

    canvas.SetTransform(kTransform::construct::translate(550, 0));
    FillMaskExample(canvas);

    canvas.SetTransform(kTransform::construct::translate(800, 0));
    BitmapBrushExample(canvas);
}

void Shutdown()
{
    delete bitmap;
    delete mask;
}
