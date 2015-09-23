/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    cairo/canvasimplcairo.cpp
        canvas API Cairo implementation
*/

#include "canvasimplcairo.h"


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
 kGradientImplCairo object implementation
 -------------------------------------------------------------------------------
*/

kGradientImplCairo::kGradientImplCairo()
{}

kGradientImplCairo::~kGradientImplCairo()
{}

void kGradientImplCairo::Initialize(const kGradientStop *stops, size_t count, kExtendType extend)
{}


/*
 -------------------------------------------------------------------------------
 kPathImplCairo object implementation
 -------------------------------------------------------------------------------
*/

kPathImplCairo::kPathImplCairo() :
    kPathImplDefault()
{}

kPathImplCairo::~kPathImplCairo()
{}

void kPathImplCairo::FromPath(const kPathImpl *source, const kTransform &transform)
{}


/*
 -------------------------------------------------------------------------------
 kBitmapImplCairo object implementation
 -------------------------------------------------------------------------------
*/

kBitmapImplCairo::kBitmapImplCairo() :
    p_bitmap(nullptr),
    p_data(nullptr),
    p_width(0),
    p_height(0)
{}

kBitmapImplCairo::~kBitmapImplCairo()
{
    cairo_surface_destroy(p_bitmap);
    delete[] p_data;
}

void kBitmapImplCairo::Initialize(size_t width, size_t height, kBitmapFormat format)
{
    p_width = width;
    p_height = height;
    p_pitch = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, int(width));
    p_data = new unsigned char[p_pitch * p_height];
    p_bitmap = cairo_image_surface_create_for_data(p_data, CAIRO_FORMAT_ARGB32, int(width), int(height), int(p_pitch));
}

void kBitmapImplCairo::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data)
{
    unsigned char *dst = p_data;
    const unsigned char *src = reinterpret_cast<const unsigned char*>(data);

    for (size_t y = 0; y < p_height; ++y) {
        memcpy(dst, src, p_width * 4);
        dst += p_pitch;
        src += sourcepitch;
    }
}


/*
 -------------------------------------------------------------------------------
 kCanvasImplCairo object implementation
 -------------------------------------------------------------------------------
*/

kCanvasImplCairo::kCanvasImplCairo(const CanvasFactory *factory) :
    boundContext(0)
{}

kCanvasImplCairo::~kCanvasImplCairo()
{
    Unbind();
}

bool kCanvasImplCairo::BindToBitmap(const kBitmapImpl *target, const kRectInt *rect)
{
    if (boundContext) {
        return false;
    }

    return true;
}

bool kCanvasImplCairo::BindToPrinter(kPrinter printer)
{
    return false;
}

bool kCanvasImplCairo::BindToContext(kContext context, const kRectInt *rect)
{
    if (boundContext) {
        return false;
    }

    boundContext = reinterpret_cast<cairo_t*>(context);
    return true;
}

bool kCanvasImplCairo::Unbind()
{
    if (boundContext) {
        boundContext = 0;
        return true;
    }
    return false;
}

void kCanvasImplCairo::Line(const kPoint &a, const kPoint &b, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        cairo_move_to(boundContext, a.x + 0.5, a.y + 0.5);
        cairo_line_to(boundContext, b.x + 0.5, b.y + 0.5);

        cairo_stroke(boundContext);
    }
}

void kCanvasImplCairo::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen)
{}

void kCanvasImplCairo::PolyLine(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        cairo_translate(boundContext, 0.5, 0.5);
        cairo_move_to(boundContext, points[0].x, points[0].y);
        for (size_t n = 1; n < count; n++) {
            cairo_line_to(boundContext, points[n].x, points[n].y);
        }
        cairo_stroke(boundContext);
        cairo_translate(boundContext, -0.5, -0.5);
    }
}

void kCanvasImplCairo::PolyBezier(const kPoint *points, size_t count, const kPenBase *pen)
{}

void kCanvasImplCairo::Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        ApplyBrush(brush);
        cairo_rectangle(boundContext, rect.left, rect.top, rect.width(), rect.height());
        cairo_fill(boundContext);
    }

    if (pen_not_empty) {
        ApplyPen(pen);
        cairo_rectangle(
            boundContext,
            rect.left + 0.5f, rect.top + 0.5f,
            rect.width(), rect.height()
        );
        cairo_stroke(boundContext);
    }
}

void kCanvasImplCairo::RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplCairo::Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    cairo_scale(boundContext, 1, rect.height() / rect.width());
    cairo_arc(
        boundContext,
        rect.getCenter().x, rect.getCenter().y,
        rect.width() * 0.5f, 0, 2 * M_PI
    );

    if (brush_not_empty) {
        ApplyBrush(brush);
        pen_not_empty ?
            cairo_fill_preserve(boundContext) :
            cairo_fill(boundContext);
    }

    if (pen_not_empty) {
        ApplyPen(pen);
        cairo_stroke(boundContext);
    }
}

void kCanvasImplCairo::Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    cairo_move_to(boundContext, points[0].x, points[0].y);
    for (size_t n = 1; n < count; n++) {
        cairo_line_to(boundContext, points[n].x, points[n].y);
    }
    cairo_close_path(boundContext);

    if (brush_not_empty) {
        ApplyBrush(brush);

        pen_not_empty ?
            cairo_fill_preserve(boundContext) :
            cairo_fill(boundContext);
    }

    if (pen_not_empty) {
        ApplyPen(pen);
        cairo_stroke(boundContext);
    }
}

void kCanvasImplCairo::PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{}

void kCanvasImplCairo::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush)
{
    PathToCairoPath(path);

    if (brush_not_empty) {
        ApplyBrush(brush);

        pen_not_empty ?
            cairo_fill_preserve(boundContext) :
            cairo_fill(boundContext);
    }

    if (pen_not_empty) {
        ApplyPen(pen);
        cairo_stroke(boundContext);
    }
}

void kCanvasImplCairo::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform)
{}

void kCanvasImplCairo::DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, float sourcealpha)
{
    const kBitmapImplCairo *bmp = static_cast<const kBitmapImplCairo*>(bitmap);

    cairo_set_source_surface(boundContext, bmp->p_bitmap, origin.x - source.x, origin.y - source.y);
    cairo_rectangle(boundContext, origin.x, origin.y, destsize.width, destsize.height);
    cairo_fill(boundContext);
}

void kCanvasImplCairo::DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{}

void kCanvasImplCairo::GetFontMetrics(const kFontBase *font, kFontMetrics *metrics)
{}

void kCanvasImplCairo::GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics)
{}

kSize kCanvasImplCairo::TextSize(const char *text, int count, const kFontBase *font, kSize *bounds)
{
    bool notbound = boundContext == nullptr;
    cairo_surface_t *surface = nullptr;
    if (notbound) {
        surface = cairo_image_surface_create(CAIRO_FORMAT_A8, 1, 1);
        boundContext = cairo_create(surface);
    }

    ApplyFont(font);

    cairo_text_extents_t t_ext;
    cairo_text_extents(boundContext, text, &t_ext);
    cairo_font_extents_t f_ext;
    cairo_font_extents(boundContext, &f_ext);

    if (notbound) {
        cairo_destroy(boundContext);
        cairo_surface_destroy(surface);
        boundContext = nullptr;
    }

    return kSize(float(t_ext.x_advance), float(f_ext.height));
}

void kCanvasImplCairo::Text(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin)
{
    ApplyFont(font);
    ApplyBrush(brush);

    cairo_font_extents_t ext;
    cairo_font_extents(boundContext, &ext);

    cairo_move_to(boundContext, p.x, p.y + ext.ascent);
    cairo_show_text(boundContext, text);
}

void kCanvasImplCairo::BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{}

void kCanvasImplCairo::BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform)
{}

void kCanvasImplCairo::BeginClippedDrawingByRect(const kRect &clip)
{}

void kCanvasImplCairo::EndClippedDrawing()
{}

void kCanvasImplCairo::SetTransform(const kTransform &transform)
{}

void kCanvasImplCairo::PathToCairoPath(const kPathImpl *path)
{
    const kPathImplCairo *cairopath = static_cast<const kPathImplCairo*>(path);

    for (std::vector<kPathImplCairo::Command>::const_iterator it = cairopath->p_commands.begin(); it < cairopath->p_commands.begin() + cairopath->p_curr_command; it++) {
        switch (it->command) {
            case kPathImplCairo::PC_MOVETO: {
                kPoint p = cairopath->p_points[it->start_index];
                cairo_move_to(boundContext, p.x, p.y);
                break;
            }

            case kPathImplCairo::PC_LINETO: {
                kPoint p = cairopath->p_points[it->start_index];
                cairo_line_to(boundContext, p.x, p.y);
                break;
            }

            case kPathImplCairo::PC_BEZIERTO: {
                const kPoint *pts = cairopath->p_points.data() + it->start_index;
                cairo_curve_to(boundContext, pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y);
                break;
            }

            case kPathImplCairo::PC_POLYLINETO: {
                for (size_t n = 0; n < it->element_count; n++) {
                    kPoint p = *(cairopath->p_points.data() + it->start_index + n);
                    cairo_line_to(boundContext, p.x, p.y);
                }
                break;
            }

            case kPathImplCairo::PC_POLYBEZIERTO: {
                // TODO
                break;
            }

            case kPathImplCairo::PC_TEXT: {
                // TODO
                break;
            }

            case kPathImplCairo::PC_CLOSE: {
                cairo_close_path(boundContext);
                break;
            }
        }
    }
}

void kCanvasImplCairo::ApplyPen(const kPenBase *pen)
{
    reinterpret_cast<kCairoPen*>(native(pen)[kCairoPen::RESOURCE_PEN])->ApplyToContext(boundContext);
}

void kCanvasImplCairo::ApplyBrush(const kBrushBase *brush)
{
    reinterpret_cast<kCairoBrush*>(native(brush)[kCairoBrush::RESOURCE_BRUSH])->ApplyToContext(boundContext);
}

void kCanvasImplCairo::ApplyFont(const kFontBase *font)
{
    reinterpret_cast<kCairoFont*>(native(font)[kCairoFont::RESOURCE_FONT])->ApplyToContext(boundContext);
}


/*
 -------------------------------------------------------------------------------
 kCairoStroke object implementation
 -------------------------------------------------------------------------------
*/

kCairoStroke::kCairoStroke(const StrokeData &stroke)
{}

kCairoStroke::~kCairoStroke()
{}


/*
 -------------------------------------------------------------------------------
 kCairoPen object implementation
 -------------------------------------------------------------------------------
*/

kCairoPen::kCairoPen(const PenData &pen)
{}

kCairoPen::~kCairoPen()
{}

void kCairoPen::ApplyToContext(cairo_t *context) const
{
    cairo_set_line_width(context, p_width);
    cairo_set_source_rgba(context, p_color.r, p_color.g, p_color.b, p_color.a);
    switch (p_style) {
        case kStrokeStyle::Solid:
            cairo_set_dash(context, nullptr, 0, 0);
            break;

        case kStrokeStyle::Dot: {
            double dots = p_width;
            cairo_set_dash(context, &dots, 1, 0.5);
            break;
        }
    }
}


/*
 -------------------------------------------------------------------------------
 kCairoBrush object implementation
 -------------------------------------------------------------------------------
*/

kCairoBrush::kCairoBrush(const BrushData &brush)
{}

kCairoBrush::~kCairoBrush()
{}

void kCairoBrush::ApplyToContext(cairo_t *context) const
{
    cairo_set_source_rgba(context, p_color.r, p_color.g, p_color.b, p_color.a);
}


/*
 -------------------------------------------------------------------------------
 kCairoFont object implementation
 -------------------------------------------------------------------------------
*/

kCairoFont::kCairoFont(const FontData &font) :
    p_slant(font.p_style & kFontStyle::Italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL),
    p_weight(font.p_style & kFontStyle::Bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL),
    p_size(font.p_size)
{
    // TODO: check overflow
    strcpy(p_face, font.p_facename);
}

kCairoFont::~kCairoFont()
{}

void kCairoFont::ApplyToContext(cairo_t *context) const
{
    cairo_select_font_face(context, p_face, p_slant, p_weight);
    cairo_set_font_size(context, p_size * (96.0f / 72.0f));
}


/*
 -------------------------------------------------------------------------------
 kCairoStrokeAllocator implementation
 -------------------------------------------------------------------------------
*/

kCairoStroke* kCairoStrokeAllocator::createResource(const StrokeData &stroke)
{
    return new kCairoStroke(stroke);
}

void kCairoStrokeAllocator::deleteResource(kCairoStroke *stroke)
{
    delete stroke;
}


/*
 -------------------------------------------------------------------------------
 kCairoPenAllocator implementation
 -------------------------------------------------------------------------------
*/

kCairoPen* kCairoPenAllocator::createResource(const PenData &pen)
{
    return new kCairoPen(pen);
}

void kCairoPenAllocator::deleteResource(kCairoPen *pen)
{
    delete pen;
}


/*
 -------------------------------------------------------------------------------
 kCairoBrushAllocator implementation
 -------------------------------------------------------------------------------
*/

kCairoBrush* kCairoBrushAllocator::createResource(const BrushData &brush)
{
    return new kCairoBrush(brush);
}

void kCairoBrushAllocator::deleteResource(kCairoBrush *brush)
{
    delete brush;
}


/*
 -------------------------------------------------------------------------------
 kCairoFontAllocator implementation
 -------------------------------------------------------------------------------
*/

kCairoFont* kCairoFontAllocator::createResource(const FontData &font)
{
    return new kCairoFont(font);
}

void kCairoFontAllocator::deleteResource(kCairoFont *font)
{
    delete font;
}


CanvasFactoryCairo::CanvasFactoryCairo()
{}

CanvasFactoryCairo::~CanvasFactoryCairo()
{}

bool CanvasFactoryCairo::initialized()
{
    return true;
}
