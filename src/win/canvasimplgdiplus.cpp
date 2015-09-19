/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    win/canvasimplgdiplus.cpp
        canvas API GDI+ implementation
*/

#include "canvasimplgdiplus.h"
//#include <codecvt>
//#include <locale>


using namespace c_util;
using namespace k_canvas;
using namespace impl;
using namespace std;


/*
 -------------------------------------------------------------------------------
 internal utility functions
 -------------------------------------------------------------------------------
*/



/*
 -------------------------------------------------------------------------------
 kGradientImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kGradientImplGDIPlus::kGradientImplGDIPlus()
{}

kGradientImplGDIPlus::~kGradientImplGDIPlus()
{}

void kGradientImplGDIPlus::Initialize(const kGradientStop *stops, size_t count, kExtendType extend)
{}


/*
 -------------------------------------------------------------------------------
 kPathImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kPathImplGDIPlus::kPathImplGDIPlus()
{}

kPathImplGDIPlus::~kPathImplGDIPlus()
{}

void kPathImplGDIPlus::MoveTo(const kPoint &p)
{}

void kPathImplGDIPlus::LineTo(const kPoint &p)
{}

void kPathImplGDIPlus::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{}

void kPathImplGDIPlus::PolyLineTo(const kPoint *points, size_t count)
{}

void kPathImplGDIPlus::PolyBezierTo(const kPoint *points, size_t count)
{}

void kPathImplGDIPlus::Text(const char *text, int count, const kFontBase *font, kTextOrigin origin)
{}

void kPathImplGDIPlus::Close()
{}

void kPathImplGDIPlus::Clear()
{}

void kPathImplGDIPlus::Commit()
{}

void kPathImplGDIPlus::FromPath(const kPathImpl *source, const kTransform &transform)
{}


/*
 -------------------------------------------------------------------------------
 kBitmapImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kBitmapImplGDIPlus::kBitmapImplGDIPlus()
{}

kBitmapImplGDIPlus::~kBitmapImplGDIPlus()
{}

void kBitmapImplGDIPlus::Initialize(size_t width, size_t height, kBitmapFormat format)
{}

void kBitmapImplGDIPlus::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data)
{}


/*
 -------------------------------------------------------------------------------
 kCanvasImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kCanvasImplGDIPlus::kCanvasImplGDIPlus(const CanvasFactory *factory)
{}

kCanvasImplGDIPlus::~kCanvasImplGDIPlus()
{
    Unbind();
}

bool kCanvasImplGDIPlus::BindToBitmap(const kBitmapImpl *target, const kRectInt *rect)
{
    return true;
}

bool kCanvasImplGDIPlus::BindToPrinter(kPrinter printer)
{
    return false;
}

bool kCanvasImplGDIPlus::BindToContext(kContext context, const kRectInt *rect)
{
    return true;
}

bool kCanvasImplGDIPlus::Unbind()
{
    return true;
}

void kCanvasImplGDIPlus::Line(const kPoint &a, const kPoint &b, const kPenBase *pen)
{}

void kCanvasImplGDIPlus::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen)
{}

void kCanvasImplGDIPlus::PolyLine(const kPoint *points, size_t count, const kPenBase *pen)
{}

void kCanvasImplGDIPlus::PolyBezier(const kPoint *points, size_t count, const kPenBase *pen)
{}

void kCanvasImplGDIPlus::Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplGDIPlus::RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplGDIPlus::Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplGDIPlus::Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplGDIPlus::PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplGDIPlus::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplGDIPlus::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform)
{}

void kCanvasImplGDIPlus::DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, float sourcealpha)
{}

void kCanvasImplGDIPlus::DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{}

void kCanvasImplGDIPlus::GetFontMetrics(const kFontBase *font, kFontMetrics *metrics)
{}

void kCanvasImplGDIPlus::GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics)
{}

kSize kCanvasImplGDIPlus::TextSize(const char *text, int count, const kFontBase *font, kSize *bounds)
{
    return kSize();
}

void kCanvasImplGDIPlus::Text(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin)
{}

void kCanvasImplGDIPlus::BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{}

void kCanvasImplGDIPlus::BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform)
{}

void kCanvasImplGDIPlus::BeginClippedDrawingByRect(const kRect &clip)
{}

void kCanvasImplGDIPlus::EndClippedDrawing()
{}

void kCanvasImplGDIPlus::SetTransform(const kTransform &transform)
{}


/*
 -------------------------------------------------------------------------------
 kGDIPlusStroke object implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusStroke::kGDIPlusStroke(const StrokeData &stroke)
{}

kGDIPlusStroke::~kGDIPlusStroke()
{}


/*
 -------------------------------------------------------------------------------
 kGDIPlusPen object implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusPen::kGDIPlusPen(const PenData &pen)
{}

kGDIPlusPen::~kGDIPlusPen()
{}


/*
 -------------------------------------------------------------------------------
 kGDIPlusBrush object implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusBrush::kGDIPlusBrush(const BrushData &brush)
{}

kGDIPlusBrush::~kGDIPlusBrush()
{}


/*
 -------------------------------------------------------------------------------
 kD2DFont object implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusFont::kGDIPlusFont(const FontData &font)
{}

kGDIPlusFont::~kGDIPlusFont()
{}


/*
 -------------------------------------------------------------------------------
 kGDIPlusStrokeAllocator implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusStroke* kGDIPlusStrokeAllocator::createResource(const StrokeData &stroke)
{
    return new kGDIPlusStroke(stroke);
}

void kGDIPlusStrokeAllocator::deleteResource(kGDIPlusStroke *stroke)
{
    delete stroke;
}


/*
 -------------------------------------------------------------------------------
 kGDIPlusPenAllocator implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusPen* kGDIPlusPenAllocator::createResource(const PenData &pen)
{
    return new kGDIPlusPen(pen);
}

void kGDIPlusPenAllocator::deleteResource(kGDIPlusPen *pen)
{
    delete pen;
}


/*
 -------------------------------------------------------------------------------
 kGDIPlusBrushAllocator implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusBrush* kGDIPlusBrushAllocator::createResource(const BrushData &brush)
{
    return new kGDIPlusBrush(brush);
}

void kGDIPlusBrushAllocator::deleteResource(kGDIPlusBrush *brush)
{
    delete brush;
}


/*
 -------------------------------------------------------------------------------
 kGDIPlusFontAllocator implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusFont* kGDIPlusFontAllocator::createResource(const FontData &font)
{
    return new kGDIPlusFont(font);
}

void kGDIPlusFontAllocator::deleteResource(kGDIPlusFont *font)
{
    delete font;
}


CanvasFactoryGDIPlus::CanvasFactoryGDIPlus()
{}

CanvasFactoryGDIPlus::~CanvasFactoryGDIPlus()
{}

bool CanvasFactoryGDIPlus::initialized()
{
    return true;
}
