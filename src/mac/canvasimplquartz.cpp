/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    mac/canvasimplquartz.cpp
        canvas API Quartz implementation
*/

#include "canvasimplquartz.h"


using namespace c_util;
using namespace k_canvas;
using namespace impl;
using namespace std;


/*
 -------------------------------------------------------------------------------
 internal utility functions
 -------------------------------------------------------------------------------
*/

// TODO: check for pen Clear syle
#define pen_not_empty pen
#define brush_not_empty brush && resourceData<BrushData>(brush).p_style != kBrushStyle::Clear


/*
 -------------------------------------------------------------------------------
 kGradientImplQuartz object implementation
 -------------------------------------------------------------------------------
*/

kGradientImplQuartz::kGradientImplQuartz()
{}

kGradientImplQuartz::~kGradientImplQuartz()
{}

void kGradientImplQuartz::Initialize(const kGradientStop *stops, size_t count, kExtendType extend)
{}


/*
 -------------------------------------------------------------------------------
 kPathImplQuartz object implementation
 -------------------------------------------------------------------------------
*/

kPathImplQuartz::kPathImplQuartz() :
    kPathImplDefault()
{}

kPathImplQuartz::~kPathImplQuartz()
{}

void kPathImplQuartz::FromPath(const kPathImpl *source, const kTransform &transform)
{}


/*
 -------------------------------------------------------------------------------
 kBitmapImplQuartz object implementation
 -------------------------------------------------------------------------------
*/

kBitmapImplQuartz::kBitmapImplQuartz() :
    p_width(0),
    p_height(0)
{}

kBitmapImplQuartz::~kBitmapImplQuartz()
{}

void kBitmapImplQuartz::Initialize(size_t width, size_t height, kBitmapFormat format)
{}

void kBitmapImplQuartz::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data)
{}


/*
 -------------------------------------------------------------------------------
 kCanvasImplQuartz object implementation
 -------------------------------------------------------------------------------
*/

kCanvasImplQuartz::kCanvasImplQuartz(const CanvasFactory *factory) :
    boundContext(0)
{}

kCanvasImplQuartz::~kCanvasImplQuartz()
{
    Unbind();
}

bool kCanvasImplQuartz::BindToBitmap(const kBitmapImpl *target, const kRectInt *rect)
{
    if (boundContext) {
        return false;
    }

    return true;
}

bool kCanvasImplQuartz::BindToPrinter(kPrinter printer)
{
    return false;
}

bool kCanvasImplQuartz::BindToContext(kContext context, const kRectInt *rect)
{
    if (boundContext) {
        return false;
    }

    boundContext = reinterpret_cast<CGContextRef>(context);
    return true;
}

bool kCanvasImplQuartz::Unbind()
{
    if (boundContext) {
        boundContext = 0;
        return true;
    }
    return false;
}

void kCanvasImplQuartz::Line(const kPoint &a, const kPoint &b, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        CGContextMoveToPoint(boundContext, a.x, a.y);
        CGContextAddLineToPoint(boundContext, b.x, b.y);

        CGContextStrokePath(boundContext);
    }
}

void kCanvasImplQuartz::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen)
{}

void kCanvasImplQuartz::PolyLine(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        CGContextMoveToPoint(boundContext, points[0].x, points[0].y);
        for (int n = 1; n < count; n++) {
            CGContextAddLineToPoint(boundContext, points[n].x, points[n].y);
        }
        CGContextStrokePath(boundContext);
    }
}

void kCanvasImplQuartz::PolyBezier(const kPoint *points, size_t count, const kPenBase *pen)
{}

void kCanvasImplQuartz::Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    CGRect cgrect = CGRectMake(rect.Left, rect.Top, rect.width(), rect.height());

    if (brush_not_empty) {
        ApplyBrush(brush);
        CGContextFillRect(boundContext, cgrect);
    }

    if (pen_not_empty) {
        ApplyPen(pen);
        CGContextStrokeRect(boundContext, cgrect);
    }
}

void kCanvasImplQuartz::RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplQuartz::Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    CGRect cgrect = CGRectMake(rect.Left, rect.Top, rect.width(), rect.height());

    if (brush_not_empty) {
        ApplyBrush(brush);
        CGContextFillEllipseInRect(boundContext, cgrect);
    }

    if (pen_not_empty) {
        ApplyPen(pen);
        CGContextStrokeEllipseInRect(boundContext, cgrect);
    }
}

void kCanvasImplQuartz::Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        ApplyBrush(brush);

        CGContextMoveToPoint(boundContext, points[0].x, points[0].y);
        for (int n = 1; n < count; n++) {
            CGContextAddLineToPoint(boundContext, points[n].x, points[n].y);
        }

        CGContextFillPath(boundContext);
    }

    if (pen_not_empty) {
        ApplyPen(pen);

        CGContextMoveToPoint(boundContext, points[0].x, points[0].y);
        for (int n = 1; n < count; n++) {
            CGContextAddLineToPoint(boundContext, points[n].x, points[n].y);
        }

        CGContextFillPath(boundContext);
    }
}

void kCanvasImplQuartz::PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplQuartz::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush)
{
    PathToQuartzPath(path);

    if (brush_not_empty) {
        ApplyBrush(brush);
        CGContextFillPath(boundContext);
    }

    if (pen_not_empty) {
        ApplyPen(pen);
        CGContextStrokePath(boundContext);
    }
}

void kCanvasImplQuartz::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform)
{}

void kCanvasImplQuartz::DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, float sourcealpha)
{
    const kBitmapImplQuartz *bmp = static_cast<const kBitmapImplQuartz*>(bitmap);
}

void kCanvasImplQuartz::DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{}

void kCanvasImplQuartz::GetFontMetrics(const kFontBase *font, kFontMetrics *metrics)
{}

void kCanvasImplQuartz::GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics)
{}

kSize kCanvasImplQuartz::TextSize(const char *text, int count, const kFontBase *font, kSize *bounds)
{
    return kSize();
}

void kCanvasImplQuartz::Text(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin)
{
    /*
    CFStringRef string = CFStringCreateWithCharactersNoCopy(kCFAllocatorSystemDefault, (UniChar*)text, len, kCFAllocatorSystemDefault);

    CFStringRef keys[] = { kCTFontAttributeName };
    CFTypeRef values[] = { static_cast<const kQuartzFont*>(font.resource)->getFontRef() };

    CFDictionaryRef attributes = CFDictionaryCreate(
        kCFAllocatorDefault,
        (const void**)&keys,
        (const void**)&values,
        sizeof(keys) / sizeof(keys[0]),
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );

    CFAttributedStringRef attrString = CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
    CFRelease(string);
    CFRelease(attributes);

    CTLineRef line = CTLineCreateWithAttributedString(attrString);
    CGContextSetTextPosition(boundContext, p.x, p.y);
    CTLineDraw(line, boundContext);
    CFRelease(line);
    */
}

void kCanvasImplQuartz::BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{}

void kCanvasImplQuartz::BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform)
{}

void kCanvasImplQuartz::BeginClippedDrawingByRect(const kRect &clip)
{}

void kCanvasImplQuartz::EndClippedDrawing()
{}

void kCanvasImplQuartz::SetTransform(const kTransform &transform)
{}

void kCanvasImplQuartz::PathToQuartzPath(const kPathImpl *path)
{
    const kPathImplQuartz *quartzpath = static_cast<const kPathImplQuartz*>(path);

    for (std::vector<kPathImplQuartz::Command>::const_iterator it = quartzpath->p_commands.begin(); it < quartzpath->p_commands.begin() + quartzpath->p_curr_command; it++) {
        switch (it->command) {
            case kPathImplQuartz::PC_MOVETO: {
                kPoint p = quartzpath->p_points[it->start_index];
                CGContextMoveToPoint(boundContext, p.x, p.y);
                break;
            }

            case kPathImplQuartz::PC_LINETO: {
                kPoint p = quartzpath->p_points[it->start_index];
                CGContextAddLineToPoint(boundContext, p.x, p.y);
                break;
            }

            case kPathImplQuartz::PC_BEZIERTO: {
                const kPoint *pts = quartzpath->p_points.data() + it->start_index;
                CGContextAddCurveToPoint(boundContext, pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y);
                break;
            }

            case kPathImplQuartz::PC_POLYLINETO: {
                for (int n = 0; n < it->point_count; n++) {
                    kPoint p = *(quartzpath->p_points.data() + it->start_index + n);
                    CGContextAddLineToPoint(boundContext, p.x, p.y);
                }
                break;
            }

            case kPathImplQuartz::PC_POLYBEZIERTO: {
                // TODO
                break;
            }

            case kPathImplQuartz::PC_TEXT: {
                // TODO
                break;
            }

            case kPathImplQuartz::PC_CLOSE: {
                CGContextClosePath(boundContext);
                break;
            }
        }
    }
}

void kCanvasImplQuartz::ApplyPen(const kPenBase *pen)
{
    reinterpret_cast<kQuartzPen*>(native(pen)[kQuartzPen::RESOURCE_PEN])->ApplyToContext(boundContext);
}

void kCanvasImplQuartz::ApplyBrush(const kBrushBase *brush)
{
    reinterpret_cast<kQuartzBrush*>(native(brush)[kQuartzBrush::RESOURCE_BRUSH])->ApplyToContext(boundContext);
}

void kCanvasImplQuartz::ApplyFont(const kFontBase *font)
{
    reinterpret_cast<kQuartzFont*>(native(font)[kQuartzFont::RESOURCE_FONT])->ApplyToContext(boundContext);
}


/*
 -------------------------------------------------------------------------------
 kQuartzStroke object implementation
 -------------------------------------------------------------------------------
*/

kQuartzStroke::kQuartzStroke(const StrokeData &stroke)
{}

kQuartzStroke::~kQuartzStroke()
{}


/*
 -------------------------------------------------------------------------------
 kQuartzPen object implementation
 -------------------------------------------------------------------------------
*/

kQuartzPen::kQuartzPen(const PenData &pen)
{}

kQuartzPen::~kQuartzPen()
{}

void kQuartzPen::ApplyToContext(CGContextRef context) const
{}


/*
 -------------------------------------------------------------------------------
 kQuartzBrush object implementation
 -------------------------------------------------------------------------------
*/

kQuartzBrush::kQuartzBrush(const BrushData &brush)
{}

kQuartzBrush::~kQuartzBrush()
{}

void kQuartzBrush::ApplyToContext(CGContextRef context) const
{}


/*
 -------------------------------------------------------------------------------
 kQuartzFont object implementation
 -------------------------------------------------------------------------------
*/

kQuartzFont::kQuartzFont(const FontData &font)
{}

kQuartzFont::~kQuartzFont()
{}

void kQuartzFont::ApplyToContext(CGContextRef context) const
{}


/*
 -------------------------------------------------------------------------------
 kQuartzStrokeAllocator implementation
 -------------------------------------------------------------------------------
*/

kQuartzStroke* kQuartzStrokeAllocator::createResource(const StrokeData &stroke)
{
    return new kQuartzStroke(stroke);
}

void kQuartzStrokeAllocator::deleteResource(kQuartzStroke *stroke)
{
    delete stroke;
}


/*
 -------------------------------------------------------------------------------
 kQuartzPenAllocator implementation
 -------------------------------------------------------------------------------
*/

kQuartzPen* kQuartzPenAllocator::createResource(const PenData &pen)
{
    return new kQuartzPen(pen);
}

void kQuartzPenAllocator::deleteResource(kQuartzPen *pen)
{
    delete pen;
}


/*
 -------------------------------------------------------------------------------
 kQuartzBrushAllocator implementation
 -------------------------------------------------------------------------------
*/

kQuartzBrush* kQuartzBrushAllocator::createResource(const BrushData &brush)
{
    return new kQuartzBrush(brush);
}

void kQuartzBrushAllocator::deleteResource(kQuartzBrush *brush)
{
    delete brush;
}


/*
 -------------------------------------------------------------------------------
 kQuartzFontAllocator implementation
 -------------------------------------------------------------------------------
*/

kQuartzFont* kQuartzFontAllocator::createResource(const FontData &font)
{
    return new kQuartzFont(font);
}

void kQuartzFontAllocator::deleteResource(kQuartzFont *font)
{
    delete font;
}


CanvasFactoryQuartz::CanvasFactoryQuartz()
{}

CanvasFactoryQuartz::~CanvasFactoryQuartz()
{}

bool CanvasFactoryQuartz::initialized()
{
    return true;
}
