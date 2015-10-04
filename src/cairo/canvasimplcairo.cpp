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
using namespace c_geometry;
using namespace k_canvas;
using namespace impl;
using namespace std;


/*
 -------------------------------------------------------------------------------
 internal utility functions
 -------------------------------------------------------------------------------
*/

static const cairo_line_cap_t linecapstyles[3] = {
    CAIRO_LINE_CAP_BUTT,
    CAIRO_LINE_CAP_SQUARE,
    CAIRO_LINE_CAP_ROUND
};

static const cairo_line_join_t joinstyles[3] = {
    CAIRO_LINE_JOIN_MITER,
    CAIRO_LINE_JOIN_BEVEL,
    CAIRO_LINE_JOIN_ROUND
};

// TODO: check for pen Clear syle
#define pen_not_empty pen
#define brush_not_empty brush && resourceData<BrushData>(brush).p_style != kBrushStyle::Clear


/*
 -------------------------------------------------------------------------------
 kGradientImplCairo object implementation
 -------------------------------------------------------------------------------
*/

kGradientImplCairo::kGradientImplCairo() :
    p_stops(nullptr),
    p_count(0)
{}

kGradientImplCairo::~kGradientImplCairo()
{
    delete[] p_stops;
}

void kGradientImplCairo::Initialize(const kGradientStop *stops, size_t count, kExtendType extend)
{
    p_stops = new kGradientStop[count];
    p_count = count;
    memcpy(p_stops, stops, count * sizeof(kGradientStop));
    p_extend = extend;
}


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

static const cairo_format_t formats[3] = {
    CAIRO_FORMAT_INVALID,
    CAIRO_FORMAT_ARGB32,
    CAIRO_FORMAT_A8
};

void kBitmapImplCairo::Initialize(size_t width, size_t height, kBitmapFormat format)
{
    p_width = width;
    p_height = height;
    p_pitch = cairo_format_stride_for_width(formats[size_t(format)], int(width));
    p_data = new unsigned char[p_pitch * p_height];

    // TODO: check this is explicitly needed
    memset(p_data, 0, p_pitch * p_height);

    p_bitmap = cairo_image_surface_create_for_data(p_data, formats[size_t(format)], int(width), int(height), int(p_pitch));
}

void kBitmapImplCairo::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data)
{
    unsigned char *dst = p_data;
    const unsigned char *src = reinterpret_cast<const unsigned char*>(data);

    for (size_t y = 0; y < p_height; ++y) {
        memcpy(dst, src, p_pitch);
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
    boundContext(0),
    releaseContext(false)
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

    const kBitmapImplCairo *bitmap = static_cast<const kBitmapImplCairo*>(target);

    boundContext = cairo_create(bitmap->p_bitmap);
    releaseContext = true;

    // TODO: adjust top level clip for bitmap rendering

    if (rect) {
        bounds = *rect;
    } else {
        bounds = kRectInt(0, 0, bitmap->p_width, bitmap->p_height);
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
    releaseContext = false;

    if (rect) {
        bounds = *rect;
    } else {
        double left, top, right, bottom;
        cairo_clip_extents(boundContext, &left, &top, &right, &bottom);
        bounds = kRectInt(
            0, 0,
            int(right - left),
            int(bottom - top)
        );
    }

    return true;
}

bool kCanvasImplCairo::Unbind()
{
    if (boundContext) {
        while (clipStack.size()) {
            PopClip();
        }

        if (releaseContext) {
            cairo_destroy(boundContext);
        }

        boundContext = 0;
        return true;
    }
    return false;
}

void kCanvasImplCairo::Line(const kPoint &a, const kPoint &b, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        cairo_move_to(boundContext, a.x, a.y);
        cairo_line_to(boundContext, b.x, b.y);

        cairo_stroke(boundContext);
    }
}

void kCanvasImplCairo::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        cairo_move_to(boundContext, p1.x + 0.5, p1.y + 0.5);
        cairo_curve_to(
            boundContext,
            p2.x + 0.5, p2.y + 0.5,
            p3.x + 0.5, p3.y + 0.5,
            p4.x + 0.5, p4.y + 0.5
        );

        cairo_stroke(boundContext);
    }
}

void kCanvasImplCairo::PolyLine(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        cairo_translate(boundContext, 0.5, 0.5);
        cairo_move_to(boundContext, points[0].x, points[0].y);
        for (size_t n = 1; n < count; ++n) {
            cairo_line_to(boundContext, points[n].x, points[n].y);
        }
        cairo_stroke(boundContext);
        cairo_translate(boundContext, -0.5, -0.5);
    }
}

void kCanvasImplCairo::PolyBezier(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ApplyPen(pen);

        cairo_translate(boundContext, 0.5, 0.5);
        cairo_move_to(boundContext, points[0].x, points[0].y);
        for (size_t n = 1; n < count; n += 3) {
            cairo_curve_to(
                boundContext,
                points[n + 0].x, points[n + 0].y,
                points[n + 1].x, points[n + 1].y,
                points[n + 2].x, points[n + 2].y
            );
        }

        cairo_stroke(boundContext);
        cairo_translate(boundContext, -0.5, -0.5);
    }
}

void kCanvasImplCairo::Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    cairo_rectangle(boundContext, rect.left, rect.top, rect.width(), rect.height());
    FillAndStroke(pen, brush);
}

void kCanvasImplCairo::RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush)
{
    cairo_move_to(boundContext, rect.left, rect.top + round.height);

    cairo_arc(
        boundContext,
        rect.left + round.width, rect.top + round.height,
        round.width, radians(180.0), radians(270.0)
    );

    cairo_arc(
        boundContext,
        rect.right - round.width, rect.top + round.height,
        round.width, radians(270.0), radians(360.0)
    );

    cairo_arc(
        boundContext,
        rect.right - round.width, rect.bottom - round.height,
        round.width, radians(0.0), radians(90.0)
    );

    cairo_arc(
        boundContext,
        rect.left + round.width, rect.bottom - round.height,
        round.width, radians(90.0), radians(180.0)
    );

    cairo_close_path(boundContext);

    FillAndStroke(pen, brush);
}

void kCanvasImplCairo::Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    // TODO: this is bad behaviour
    cairo_scale(boundContext, 1, rect.height() / rect.width());

    cairo_arc(
        boundContext,
        rect.getCenter().x, rect.getCenter().y,
        rect.width() * 0.5f, 0, 2 * M_PI
    );

    FillAndStroke(pen, brush);

    cairo_scale(boundContext, 1, rect.width() / rect.height());
}

void kCanvasImplCairo::Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    cairo_move_to(boundContext, points[0].x, points[0].y);
    for (size_t n = 1; n < count; n++) {
        cairo_line_to(boundContext, points[n].x, points[n].y);
    }
    cairo_close_path(boundContext);

    FillAndStroke(pen, brush);
}

void kCanvasImplCairo::PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    cairo_translate(boundContext, 0.5, 0.5);
    cairo_move_to(boundContext, points[0].x, points[0].y);
    const kPoint *cp = points + 1;
    for (size_t n = 1; n < (count - 2); n += 3) {
        cairo_curve_to(
            boundContext,
            cp[0].x, cp[0].y,
            cp[1].x, cp[1].y,
            cp[2].x, cp[2].y
        );
        cp += 3;
    }

    cairo_curve_to(
        boundContext,
        cp[0].x, cp[0].y,
        cp[1].x, cp[1].y,
        points[0].x, points[0].y
    );

    cairo_close_path(boundContext);

    FillAndStroke(pen, brush);
}

void kCanvasImplCairo::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush)
{
    PathToCairoPath(path, kTransform());
    FillAndStroke(pen, brush);
}

void kCanvasImplCairo::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform)
{
    PathToCairoPath(path, transform);
    FillAndStroke(pen, brush);
}

void kCanvasImplCairo::DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, float sourcealpha)
{
    const kBitmapImplCairo *bmp = static_cast<const kBitmapImplCairo*>(bitmap);

    cairo_pattern_t *p = cairo_pattern_create_for_surface(bmp->p_bitmap);
    float kx = sourcesize.width / destsize.width;
    float ky = sourcesize.height / destsize.height;
    cairo_matrix_t m = {
        kx, 0,
        0, ky,
        -origin.x * kx + source.x, -origin.y * ky + source.y
    };
    cairo_pattern_set_matrix(p, &m);

    cairo_save(boundContext);
    cairo_set_source(boundContext, p);

    cairo_rectangle(boundContext, origin.x, origin.y, destsize.width, destsize.height);
    cairo_clip(boundContext);

    cairo_paint_with_alpha(boundContext, sourcealpha);
    cairo_restore(boundContext);

    cairo_pattern_destroy(p);
}

void kCanvasImplCairo::DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{
    cairo_pattern_t *p = cairo_pattern_create_for_surface(static_cast<const kBitmapImplCairo*>(mask)->p_bitmap);
    float kx = sourcesize.width / destsize.width;
    float ky = sourcesize.height / destsize.height;
    cairo_matrix_t m = {
        kx, 0,
        0, ky,
        -origin.x * kx + source.x, -origin.y * ky + source.y
    };
    cairo_pattern_set_matrix(p, &m);

    cairo_save(boundContext);
    cairo_rectangle(boundContext, origin.x, origin.y, destsize.width, destsize.height);
    cairo_clip(boundContext);

    ApplyBrush(brush);
    cairo_mask(boundContext, p);

    cairo_restore(boundContext);

    cairo_pattern_destroy(p);
}

void kCanvasImplCairo::GetFontMetrics(const kFontBase *font, kFontMetrics *metrics)
{
    ApplyFont(font);
    cairo_font_extents_t ext;
    cairo_font_extents(boundContext, &ext);

    metrics->ascent = kScalar(ext.ascent);
    metrics->descent = kScalar(ext.descent);
    metrics->height = kScalar(ext.height);
    // TODO: additional font metrics
    metrics->linegap = 0;
    metrics->capheight = 0;
    metrics->xheight = 0;
    metrics->underlinepos = 0;
    metrics->underlinewidth = 0;
    metrics->strikethroughpos = 0;
    metrics->strikethroughwidth = 0;
}

void kCanvasImplCairo::GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics)
{
    ApplyFont(font);
    cairo_text_extents_t ext;
    cairo_glyph_t glyphs[256];
    for (size_t g = first; g <= last; ++g) {
        glyphs[g - first].index = g;
    }
    cairo_glyph_extents(boundContext, glyphs, last - first + 1, &ext);

    metrics->a = kScalar(ext.x_bearing);
    metrics->b = kScalar(ext.x_advance - ext.width);
    metrics->c = kScalar(ext.x_advance - ext.x_bearing);
}

kSize kCanvasImplCairo::TextSize(const char *text, int count, const kFontBase *font, kSize *bounds)
{
    bool notbound = boundContext == nullptr;
    cairo_surface_t *surface = nullptr;
    if (notbound) {
        surface = cairo_image_surface_create(CAIRO_FORMAT_A8, 1, 1);
        boundContext = cairo_create(surface);
    }

    char buffer[4096];
    PrepareText(text, count, buffer);

    ApplyFont(font);

    cairo_text_extents_t t_ext;
    cairo_text_extents(boundContext, buffer, &t_ext);
    cairo_font_extents_t f_ext;
    cairo_font_extents(boundContext, &f_ext);

    if (bounds) {
        // TODO: adjust bounds, fix space measurement
        bounds->width = kScalar(t_ext.width);
        bounds->height = kScalar(t_ext.height);
    }

    if (notbound) {
        cairo_destroy(boundContext);
        cairo_surface_destroy(surface);
        boundContext = nullptr;
    }

    return kSize(float(t_ext.width), float(f_ext.height));
}

void kCanvasImplCairo::Text(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin)
{
    char buffer[4096];
    PrepareText(text, count, buffer);

    ApplyFont(font);
    ApplyBrush(brush);

    cairo_font_extents_t ext;
    cairo_font_extents(boundContext, &ext);

    cairo_move_to(boundContext, p.x, p.y + ext.ascent);
    cairo_show_text(boundContext, buffer);

    kFontStyle style = resourceData<FontData>(font).p_style;
    if (style & kFontStyle::Underline) {
        // TODO: font underline
    }
    if (style & kFontStyle::Strikethrough) {
        // TODO: font strikethrough
    }
}

void kCanvasImplCairo::BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{
    Clip &clip = PushClip(false);

    clip.surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32,
        bounds.width(), bounds.height()
    );

    clip.pattern = cairo_pattern_create_for_surface(
        static_cast<const kBitmapImplCairo*>(mask)->p_bitmap
    );

    clip.cairo = boundContext;

    cairo_matrix_t org;
    cairo_get_matrix(boundContext, &org);

    // TODO: pattern transform is a hack, check it for correctness

    float invx = 1.0f / transform.m00;
    float invy = 1.0f / transform.m11;

    cairo_matrix_t m = {
        invx, -transform.m01,
        -transform.m10, invy,
        -invx * (transform.m20 + org.x0), -invy * (transform.m21 + org.y0)
    };

    cairo_pattern_set_matrix(clip.pattern, &m);
    cairo_pattern_set_extend(clip.pattern, CAIRO_EXTEND_REPEAT);

    boundContext = cairo_create(clip.surface);
    cairo_set_matrix(boundContext, &org);
}

void kCanvasImplCairo::BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform)
{
    PushClip(true);

    PathToCairoPath(clip, transform);
    cairo_clip(boundContext);
}

void kCanvasImplCairo::BeginClippedDrawingByRect(const kRect &clip)
{
    PushClip(true);

    cairo_rectangle(boundContext, clip.left, clip.top, clip.width(), clip.height());
    cairo_clip(boundContext);
}

void kCanvasImplCairo::EndClippedDrawing()
{
    PopClip();
}

void kCanvasImplCairo::SetTransform(const kTransform &transform)
{
    cairo_matrix_t m = {
        transform.m00, transform.m01,
        transform.m10, transform.m11,
        transform.m20, transform.m21
    };
    cairo_set_matrix(boundContext, &m);
}

void kCanvasImplCairo::PathToCairoPath(const kPathImpl *path, const kTransform &transform)
{
    const kPathImplCairo *cairopath = static_cast<const kPathImplCairo*>(path);

    cairo_set_fill_rule(boundContext, CAIRO_FILL_RULE_EVEN_ODD);

    for (std::vector<kPathImplCairo::Command>::const_iterator it = cairopath->p_commands.begin(); it < cairopath->p_commands.begin() + cairopath->p_curr_command; it++) {
        switch (it->command) {
            case kPathImplCairo::PC_MOVETO: {
                kPoint p = (transform * vec2f(cairopath->p_points[it->start_index])).topoint<kScalar>();
                cairo_move_to(boundContext, p.x, p.y);
                break;
            }

            case kPathImplCairo::PC_LINETO: {
                kPoint p = (transform * vec2f(cairopath->p_points[it->start_index])).topoint<kScalar>();
                cairo_line_to(boundContext, p.x, p.y);
                break;
            }

            case kPathImplCairo::PC_BEZIERTO: {
                kPoint transformed[3];
                for (size_t n = 0; n < 3; ++n) {
                    transformed[n] = (transform * vec2f(cairopath->p_points[it->start_index + n])).topoint<kScalar>();
                }
                cairo_curve_to(
                    boundContext,
                    transformed[0].x, transformed[0].y,
                    transformed[1].x, transformed[1].y,
                    transformed[2].x, transformed[2].y
                );
                break;
            }

            case kPathImplCairo::PC_POLYLINETO: {
                for (size_t n = 0; n < it->element_count; n++) {
                    kPoint p = (transform * vec2f(cairopath->p_points[it->start_index + n])).topoint<kScalar>();
                    cairo_line_to(boundContext, p.x, p.y);
                }
                break;
            }

            case kPathImplCairo::PC_POLYBEZIERTO: {
                for (size_t n = 0; n < it->element_count; n += 3) {
                    kPoint transformed[3];
                    for (size_t p = 0; p < 3; ++p) {
                        transformed[p] = (transform * vec2f(cairopath->p_points[it->start_index + n + p])).topoint<kScalar>();
                    }
                    cairo_curve_to(
                        boundContext,
                        transformed[0].x, transformed[0].y,
                        transformed[1].x, transformed[1].y,
                        transformed[2].x, transformed[2].y
                    );
                }
                break;
            }

            case kPathImplCairo::PC_TEXT: {
                static_cast<kCairoFont*>(it->font)->ApplyToContext(boundContext);

                cairo_save(boundContext);
                cairo_matrix_t m = {
                    transform.m00, transform.m01,
                    transform.m10, transform.m11,
                    transform.m20, transform.m21
                };
                cairo_transform(boundContext, &m);

                cairo_font_extents_t ext;
                cairo_font_extents(boundContext, &ext);
                cairo_rel_move_to(boundContext, 0, ext.ascent);
                cairo_text_path(boundContext, cairopath->p_text[it->start_index].c_str());

                cairo_restore(boundContext);

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

void kCanvasImplCairo::FillAndStroke(const kPenBase *pen, const kBrushBase *brush)
{
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

kCanvasImplCairo::Clip& kCanvasImplCairo::PushClip(bool save)
{
    Clip clip = {};

    if (save) {
        cairo_save(boundContext);
    }

    clipStack.push_back(clip);
    return clipStack.back();
}

void kCanvasImplCairo::PopClip()
{
    Clip clip = clipStack.back();
    clipStack.pop_back();

    if (clip.surface) {
        cairo_destroy(boundContext);
        boundContext = clip.cairo;

        cairo_save(boundContext);
        cairo_reset_clip(boundContext);
        cairo_identity_matrix(boundContext);

        cairo_set_source_surface(boundContext, clip.surface, 0, 0);
        cairo_mask(boundContext, clip.pattern);

        cairo_restore(boundContext);

        cairo_surface_destroy(clip.surface);
        cairo_pattern_destroy(clip.pattern);
    } else {
        cairo_restore(boundContext);
    }
}

void kCanvasImplCairo::PrepareText(const char *source, int length, char *buffer)
{
    // TODO: unsafe copy, optimize length/copy
    if (length == -1) {
        length = strlen(source);
    }
    strncpy(buffer, source, length);
    buffer[length] = 0;

    while (*buffer) {
        if (*buffer == '\n') {
            *buffer = ' ';
        }
        ++buffer;
    }
}


/*
 -------------------------------------------------------------------------------
 kCairoStroke object implementation
 -------------------------------------------------------------------------------
*/

kCairoStroke::kCairoStroke(const StrokeData &stroke) :
    p_data(stroke)
{}

kCairoStroke::~kCairoStroke()
{}

void kCairoStroke::ApplyToContext(cairo_t *context, float width) const
{
    switch (p_data.p_style) {
        case kStrokeStyle::Solid:
            cairo_set_dash(context, nullptr, 0, 0);
            break;

        case kStrokeStyle::Dot: {
            double dots = width;
            cairo_set_dash(context, &dots, 1, p_data.p_dashoffset);
            break;
        }

        case kStrokeStyle::Dash: {
            double dots[2] = { width * 3, width };
            cairo_set_dash(context, dots, 2, p_data.p_dashoffset);
            break;
        }

        case kStrokeStyle::DashDot: {
            double dots[4] = { width * 3, width, width, width };
            cairo_set_dash(context, dots, 4, p_data.p_dashoffset);
            break;
        }

        case kStrokeStyle::DashDotDot: {
            double dots[6] = { width * 3, width, width, width, width, width };
            cairo_set_dash(context, dots, 6, p_data.p_dashoffset);
            break;
        }
    }

    cairo_set_line_cap(context, linecapstyles[size_t(p_data.p_startcap)]);
    cairo_set_line_join(context, joinstyles[size_t(p_data.p_join)]);
}


/*
 -------------------------------------------------------------------------------
 kCairoPen object implementation
 -------------------------------------------------------------------------------
*/

kCairoPen::kCairoPen(const PenData &pen) :
    p_brush(nullptr),
    p_stroke(nullptr),
    p_width(pen.p_width)
{
    if (pen.p_brush) {
        void *native[1];
        pen.p_brush->setupNativeResources(native);
        p_brush = reinterpret_cast<kCairoBrush*>(native[0]);
    }

    if (pen.p_stroke) {
        void *native[1];
        pen.p_stroke->setupNativeResources(native);
        p_stroke = reinterpret_cast<kCairoStroke*>(native[0]);
    }
}

kCairoPen::~kCairoPen()
{}

void kCairoPen::ApplyToContext(cairo_t *context) const
{
    cairo_set_line_width(context, p_width);
    p_brush->ApplyToContext(context);
    p_stroke->ApplyToContext(context, p_width);
}


/*
 -------------------------------------------------------------------------------
 kCairoBrush object implementation
 -------------------------------------------------------------------------------
*/

kCairoBrush::kCairoBrush(const BrushData &brush) :
    p_style(brush.p_style),
    p_color(brush.p_color),
    p_pattern(nullptr)
{
    switch (p_style) {
        case kBrushStyle::LinearGradient: {
            p_pattern = cairo_pattern_create_linear(
                brush.p_start.x, brush.p_start.y,
                brush.p_end.x, brush.p_end.y
            );

            kGradientImplCairo *gradient = static_cast<kGradientImplCairo*>(brush.p_gradient);
            for (size_t n = 0; n < gradient->p_count; ++n) {
                kColorReal color(gradient->p_stops[n].color);
                cairo_pattern_add_color_stop_rgba(
                    p_pattern,
                    gradient->p_stops[n].position,
                    color.r, color.g, color.b, color.a
                );
            }
            break;
        }

        case kBrushStyle::RadialGradient: {
            p_pattern = cairo_pattern_create_radial(
                brush.p_start.x + brush.p_end.x, brush.p_start.y + brush.p_end.y, 0,
                brush.p_start.x, brush.p_start.y, brush.p_radius.width
            );

            kGradientImplCairo *gradient = static_cast<kGradientImplCairo*>(brush.p_gradient);
            for (size_t n = 0; n < gradient->p_count; ++n) {
                kColorReal color(gradient->p_stops[n].color);
                cairo_pattern_add_color_stop_rgba(
                    p_pattern,
                    gradient->p_stops[n].position,
                    color.r, color.g, color.b, color.a
                );
            }
            break;
        }

        case kBrushStyle::Bitmap:
            p_pattern = cairo_pattern_create_for_surface(
                static_cast<kBitmapImplCairo*>(brush.p_bitmap)->p_bitmap
            );
            cairo_pattern_set_extend(p_pattern, CAIRO_EXTEND_REPEAT);
            break;
    }
}

kCairoBrush::~kCairoBrush()
{
    if (p_pattern) {
        cairo_pattern_destroy(p_pattern);
    }
}

void kCairoBrush::ApplyToContext(cairo_t *context) const
{
    switch (p_style) {
        case kBrushStyle::Clear:
            cairo_set_source_rgba(context, 0, 0, 0, 0);
            break;

        case kBrushStyle::Solid:
            cairo_set_source_rgba(context, p_color.r, p_color.g, p_color.b, p_color.a);
            break;

        case kBrushStyle::LinearGradient:
        case kBrushStyle::RadialGradient:
        case kBrushStyle::Bitmap:
            cairo_set_source(context, p_pattern);
            break;
    }
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
