/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvas.cpp
        main implementation file
        implements platform independend API functionality
*/

#include "canvas.h"
#include "canvasimpl.h"


using namespace k_canvas;
using namespace impl;
using namespace c_util;
using namespace c_geometry;


/*
 -------------------------------------------------------------------------------
 kStroke object implementation
 -------------------------------------------------------------------------------
*/

kStroke::kStroke(kStrokeStyle style, kScalar dashoffset, const kScalar *strokes, size_t count)
{
    p_data.p_style = style;
    p_data.p_startcap = kCapStyle::Flat;
    p_data.p_endcap = kCapStyle::Flat;
    p_data.p_dashcap = kCapStyle::Flat;
    p_data.p_dashoffset = dashoffset;
    p_data.p_join = kLineJoin::Miter;
    p_data.p_count = 0;
}

kStroke::kStroke(
    kStrokeStyle style,
    kLineJoin join,
    kCapStyle startcap,
    kCapStyle endcap,
    kCapStyle dashcap,
    kScalar dashoffset,
    const kScalar *strokes, size_t count
)
{
    p_data.p_style = style;
    p_data.p_startcap = startcap;
    p_data.p_endcap = endcap;
    p_data.p_dashcap = dashcap;
    p_data.p_dashoffset = dashoffset;
    p_data.p_join = join;
    p_data.p_count = 0;
}

kStroke::~kStroke()
{}


/*
 -------------------------------------------------------------------------------
 kGradient object implementation
 -------------------------------------------------------------------------------
*/

kGradient::kGradient(const kColor &start, const kColor &end, kExtendType extend) :
    p_impl(CanvasFactory::CreateGradient())
{
    kGradientStop stops[2] = {
        kGradientStop(start, 0),
        kGradientStop(end, 1)
    };
    p_impl->Initialize(stops, 2, extend);
}

kGradient::kGradient(const kGradientStop *stops, size_t count, kExtendType extend) :
    p_impl(CanvasFactory::CreateGradient())
{
    p_impl->Initialize(stops, count, extend);
}

kGradient::~kGradient()
{
    ReleaseResource(p_impl);
}


/*
 -------------------------------------------------------------------------------
 kPen object implementation
 -------------------------------------------------------------------------------
*/

kPen::kPen()
{
    p_data.p_stroke = nullptr;
    p_data.p_brush = nullptr;
}

kPen::kPen(const kColor &color, kScalar width, kStrokeStyle style, const kScalar *strokes, size_t count)
{
    // stroke style and brush created implicitly
    kBrush brush(color);
    kStroke stroke(style, 0, strokes, count);

    p_data.p_width = width;
    p_data.p_stroke = stroke.getResource();
    p_data.p_brush = brush.getResource();
}

kPen::kPen(const kColor &color, kScalar width, const kStroke *stroke)
{
    kBrush brush(color);

    p_data.p_width = width;
    p_data.p_brush = brush.getResource();
    p_data.p_stroke = stroke->getResource();
}

kPen::kPen(const kBrush &brush, kScalar width, const kStroke *stroke)
{
    p_data.p_width = width;
    p_data.p_brush = brush.getResource();
    p_data.p_stroke = stroke->getResource();
}

kPen::kPen(const kPen &source) :
    kPenBase(source)
{
    if (source.p_data.p_brush) {
        p_data.p_brush->addref();
    }
    if (source.p_data.p_stroke) {
        p_data.p_stroke->addref();
    }
}

kPen::~kPen()
{
    ReleaseResource(p_data.p_brush);
    ReleaseResource(p_data.p_stroke);
}


/*
 -------------------------------------------------------------------------------
 kBrush object implementation
 -------------------------------------------------------------------------------
*/

kBrush::kBrush()
{
    p_data.p_style = kBrushStyle::Clear;
    p_data.p_gradient = nullptr;
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(const kColor &color)
{
    p_data.p_style = kBrushStyle::Solid;
    p_data.p_color = color;
    p_data.p_gradient = nullptr;
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(const kPoint &start, const kPoint &end, const kGradient &gradient)
{
    p_data.p_style = kBrushStyle::LinearGradient;
    p_data.p_start = start;
    p_data.p_end = end;
    p_data.p_gradient = gradient.p_impl;
    p_data.p_gradient->addref();
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(const kPoint &center, const kPoint &offset, const kSize &radius, const kGradient &gradient)
{
    p_data.p_style = kBrushStyle::RadialGradient;
    p_data.p_start = center;
    p_data.p_end = offset;
    p_data.p_radius = radius;
    p_data.p_gradient = gradient.p_impl;
    p_data.p_gradient->addref();
    p_data.p_bitmap = nullptr;
}

kBrush::kBrush(kExtendType xextend, kExtendType yextend, const kBitmap *bitmap)
{
    p_data.p_style = kBrushStyle::Bitmap;
    p_data.p_gradient = nullptr;
    p_data.p_xextend = xextend;
    p_data.p_yextend = yextend;
    p_data.p_bitmap = bitmap->p_impl;
    p_data.p_bitmap->addref();
}

kBrush::kBrush(const kBrush &source) :
    kBrushBase(source)
{
    if (source.p_data.p_gradient) {
        p_data.p_gradient->addref();
    }
    if (source.p_data.p_bitmap) {
        p_data.p_bitmap->addref();
    }
}

kBrush::~kBrush()
{
    ReleaseResource(p_data.p_gradient);
    ReleaseResource(p_data.p_bitmap);
}


/*
 -------------------------------------------------------------------------------
 kFont object implementation
 -------------------------------------------------------------------------------
*/

kFont::kFont()
{
    // TODO: implement default font
}

kFont::kFont(const char *facename, kScalar size, uint32_t style)
{
    strncpy(p_data.p_facename, facename, MAX_FONT_FACE_LENGTH);
    p_data.p_facename[MAX_FONT_FACE_LENGTH - 1] = 0;
    p_data.p_size = size;
    p_data.p_style = style;
}

//kFont::kFont(const kFont &source) :
//    kFontBase(source)
//{}

kFont::~kFont()
{}


// helper routine for converting arc to set of bezier segments

static const size_t MAX_ARC_POINTS = 16;

static void ArcToBezierPoints(const kRect &rect, kScalar start, kScalar end, kPoint *points, size_t &count)
{
    typedef vec2<kScalar> vec;
    typedef mat2x2<kScalar> mat;


    kScalar radius = rect.width() * kScalar(0.5);
    kScalar k = rect.height() / rect.width();
    kPoint center = rect.getCenter();

    vec rk = vec(kScalar(1.0), k);

    kScalar angle = end - start;
    kScalar sgn = sign(angle);
    angle = abs(angle);

    mat m;
    m.rotate(start);

    vec cur = m * vec(0, -radius);

    count = 0;
    points[count++] = cur.topoint<vec::type>() + center;

    while (angle > 0) {
        kScalar seg_angle = umin(kScalar(89.9), angle);
        kScalar k = kScalar(4.0) / kScalar(3.0) * tan(kScalar(0.25) * radians(seg_angle) * sgn);
        vec cp;

        cp = cur * rk + vec(-cur.y, cur.x) * rk * k;
        points[count++] = cp.topoint<vec::type>() + center;

        m.rotate(-seg_angle * sgn);
        cur = m * cur;
        cp = cur * rk + vec(cur.y, -cur.x) * rk * k;
        points[count++] = cp.topoint<vec::type>() + center;

        points[count++] = (cur * rk).topoint<vec::type>() + center;

        if ((count + 3) >= MAX_ARC_POINTS) {
            break;
        }

        angle -= seg_angle;
    }
}


/*
 -------------------------------------------------------------------------------
 kPath object implementation
 -------------------------------------------------------------------------------
*/

kPath::kPath() :
    p_impl(CanvasFactory::CreatePath())
{}

kPath::kPath(const kPath &source, const kTransform &transform) :
    p_impl(CanvasFactory::CreatePath())
{
    p_impl->FromPath(source.p_impl, transform);
}

kPath::~kPath()
{
    ReleaseResource(p_impl);
}

void kPath::MoveTo(const kPoint &point)
{
    p_impl->MoveTo(point);
}

void kPath::LineTo(const kPoint &point)
{
    p_impl->LineTo(point);
}

void kPath::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{
    p_impl->BezierTo(p1, p2, p3);
}

void kPath::ArcTo(const kRect &rect, kScalar start, kScalar end)
{
    // compute bezier segments' points
    kPoint arcpoints[MAX_ARC_POINTS];
    size_t count;
    ArcToBezierPoints(rect, start, end, arcpoints, count);

    // there's no way to exactly specify arc's starting point coords
    // so line is always added before arc's first segment
    p_impl->LineTo(*arcpoints);

    kPoint *cp = arcpoints + 1;
    --count;

    // add bezier segments to path
    for (size_t seg = 0; seg < (count / 3); ++seg) {
        p_impl->BezierTo(cp[0], cp[1], cp[2]);
        cp += 3;
    }
}

void kPath::PolyLineTo(const kPoint *points, size_t count)
{
    p_impl->PolyLineTo(points, count);
}

void kPath::PolyBezierTo(const kPoint *points, size_t count)
{
    p_impl->PolyBezierTo(points, count);
}

void kPath::Text(const char *text, int count, const kFont *font, kTextOrigin origin)
{
    if (font) {
        font->needResource();
        p_impl->Text(text, count, font, origin);
    }
}

void kPath::Close()
{
    p_impl->Close();
}

void kPath::Clear()
{
    p_impl->Clear();
}

void kPath::Commit()
{
    p_impl->Commit();
}


/*
 -------------------------------------------------------------------------------
 kBitmap object implementation
 -------------------------------------------------------------------------------
*/

kBitmap::kBitmap(size_t width, size_t height, kBitmapFormat format) :
    p_impl(CanvasFactory::CreateBitmap()),
    p_width(width),
    p_height(height),
    p_format(format)
{
    p_impl->Initialize(width, height, format);
}

kBitmap::~kBitmap()
{
    ReleaseResource(p_impl);
}

void kBitmap::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data)
{
    p_impl->Update(updaterect, sourceformat, sourcepitch, data);
}


/*
 -------------------------------------------------------------------------------
 kWord & kWordBreaker helper classes
 -------------------------------------------------------------------------------
    Breaks text into words and other elements
*/

class Word
{
public:
    enum Type
    {
        Text,
        Space,
        Tab,
        LineBreak
    };

public:
    const char *text;
    size_t      length;
    Type        type;
};

class WordBreaker
{
public:
    WordBreaker(const char *text, size_t length, kTextFlags flags) :
        p_text(reinterpret_cast<const unsigned char *>(text)),
        p_pos(0),
        p_length(length),
        p_linebreaks((flags & kTextFlags::IgnoreLineBreaks) == 0),
        p_tabstops((flags & kTextFlags::UseTabs) != 0)
    {}

    bool NextWord(Word &word)
    {
        if (p_pos == p_length) {
            return false;
        }

        word.text = reinterpret_cast<const char *>(p_text + p_pos);

        // try to find word boundaries
        size_t start = p_pos;
        while (p_pos < p_length) {
            if (p_text[p_pos] <= ' ') {
                break;
            }
            ++p_pos;
        }

        // if there were some characters this part of text is a word
        if (p_pos > start) {
            word.length = p_pos - start;
            word.type = Word::Text;
            return true;
        }

        // loop through spacing characters
        word.length = 0;
        while (p_pos < p_length) {
            bool returnspaces = false;

            switch (p_text[p_pos]) {
                // check for tab stop character
                case '\t':
                    if (p_tabstops) {
                        if (word.length) {
                            returnspaces = true;
                        } else {
                            word.type = Word::Tab;
                            ++p_pos;
                            return true;
                        }
                    } else {
                        ++p_pos;
                        ++word.length;
                    }
                    break;

                // check for line break sequence
                case '\n':
                case '\r': {
                    size_t linebreak = 0;

                    // check for one or two characters line breaks (LF, CRLF or LFCR)
                    if ((p_length - p_pos) == 1) {
                        linebreak = 1;
                    } else {
                        if ((p_text[p_pos] == '\r' && p_text[p_pos + 1] == '\n') ||
                            (p_text[p_pos] == '\n' && p_text[p_pos + 1] == '\r')) {
                            linebreak = 2;
                        } else {
                            linebreak = 1;
                        }
                    }

                    if (p_linebreaks) {
                        if (word.length) {
                            returnspaces = true;
                        } else {
                            word.type = Word::LineBreak;
                            p_pos += linebreak;
                            return true;
                        }
                    } else {
                        p_pos += linebreak;
                        ++word.length;
                    }
                    break;
                }

                default:
                    if (p_text[p_pos] > ' ') {
                        returnspaces = true;
                    } else {
                        ++p_pos;
                        ++word.length;
                    }
            }

            if (returnspaces) {
                break;
            }
        }

        word.type = Word::Space;
        return true;
    }

private:
    const unsigned char *p_text;
    size_t               p_pos;
    size_t               p_length;
    bool                 p_linebreaks;
    bool                 p_tabstops;
};


/*
 -------------------------------------------------------------------------------
 kTextService implementation
 -------------------------------------------------------------------------------
*/

kTextService::kTextService() :
    p_impl(CanvasFactory::CreateCanvas())
{}

kTextService::~kTextService()
{
    delete p_impl;
}

void kTextService::GetFontMetrics(const kFont *font, kFontMetrics &metrics)
{
    if (font) {
        font->needResource();
        p_impl->GetFontMetrics(font, metrics);
    }
}

void kTextService::GetGlyphMetrics(const kFont *font, size_t first, size_t last, kGlyphMetrics *metrics)
{
    if (font) {
        font->needResource();
        p_impl->GetGlyphMetrics(font, first, last, metrics);
    }
}

template <typename T, typename C>
static kSize TextLayout(
    kCanvasImpl *impl, const char *text, int count, const kFont *font,
    const T *properties, kScalar maxwidth, const C &callback, kRect &bounds,
    kFontMetrics *fontmetrics = nullptr
)
{
    kGlyphMetrics spaceglyph;
    impl->GetGlyphMetrics(font, ' ', ' ', &spaceglyph);

    kFontMetrics fm;
    impl->GetFontMetrics(font, fm);
    if (fontmetrics) {
        *fontmetrics = fm;
    }

    kTextFlags flags;
    kScalar    interval;
    kScalar    indent;
    kScalar    defaulttabwidth;

    if (properties) {
        flags = properties->flags;
        interval = properties->interval;
        indent = properties->indent;
        defaulttabwidth = properties->defaulttabwidth;
        if (defaulttabwidth < spaceglyph.advance) {
            defaulttabwidth = spaceglyph.advance;
        }
    } else {
        defaulttabwidth = 0;
        indent = 0;
        interval = 0;
        flags = kTextFlags::IgnoreLineBreaks;
    }

    bool multiline = (flags & kTextFlags::Multiline) != 0;
    bool ignorelinebreaks = !multiline || (flags & kTextFlags::IgnoreLineBreaks) != 0;
    bool usetabs = (flags & kTextFlags::UseTabs) != 0;

    WordBreaker wordbreaker(text, count, flags);

    kScalar tabwidth = 0;
    kSize result;
    kPoint cp(indent, 0);

    kScalar leftbound = 0;
    kScalar rightbound = 0;

    Word word;
    while (wordbreaker.NextWord(word)) {
        switch (word.type) {
            case Word::Text: {
                kScalar wordwidth = impl->TextSize(word.text, int(word.length), font).width;
                bool breaktonextline = multiline && (cp.x + wordwidth) > maxwidth;
                if (breaktonextline) {
                    cp.x = 0;
                    cp.y += fm.height + fm.linegap + interval;
                    result.height = cp.y;
                }

                kGlyphMetrics gm;
                // take word first glyph metrics
                size_t glyph = word.text[0];
                impl->GetGlyphMetrics(font, glyph, glyph, &gm);

                if ((cp.x + gm.leftbearing) < leftbound) {
                    leftbound = cp.x + gm.leftbearing;
                }

                if (word.length > 1) {
                    glyph = word.text[word.length - 1];
                    impl->GetGlyphMetrics(font, glyph, glyph, &gm);
                }

                callback(cp, word.text, int(word.length), wordwidth, breaktonextline);
                cp.x += wordwidth;

                if (cp.x > result.width) {
                    result.width = cp.x;
                }

                if ((cp.x + gm.rightbearing) > rightbound) {
                    rightbound = cp.x + gm.rightbearing;
                }

                break;
            }

            case Word::LineBreak:
                if (multiline && !ignorelinebreaks) {
                    cp.x = 0;
                    cp.y += fm.height + fm.linegap + interval;
                    result.height = cp.y;
                } else {
                    cp.x += spaceglyph.advance;
                }
                break;

            case Word::Space:
                cp.x += spaceglyph.advance * word.length;
                break;

            case Word::Tab:
                if (!usetabs) {
                    cp.x += spaceglyph.advance;
                } else {
                    while (tabwidth < cp.x) {
                        tabwidth += defaulttabwidth;
                    }
                    cp.x = tabwidth;
                }
                break;
        }
    }

    if (cp.x > 0.0f) {
        if (cp.x > result.width) {
            result.width = cp.x;
        }
        result.height += fm.height;
    }

    bounds.left = leftbound;
    bounds.top = 0;
    bounds.right = rightbound;
    bounds.bottom = result.height + fm.linegap;
    // NOTE: think about break at text end, the linegap will be added twice

    return result;
}

#define LAYOUT_CALLBACK_PARAMS\
    const kPoint &cp,\
    const char *text, int count,\
    kScalar width, bool newline

class NullLayoutCallback
{
public:
    void operator()(LAYOUT_CALLBACK_PARAMS) const
    {}
};

kSize kTextService::TextSize(const char *text, int count, const kFont *font, const kTextSizeProperties *properties, kRect *bounds)
{
    kSize result;

    if (font) {
        if (count == -1) {
            count = int(strlen(text));
        }
        if (count == 0) {
            return result;
        }

        font->needResource();

        NullLayoutCallback callback;
        kRect resultbounds;
        result = TextLayout(
            p_impl, text, count, font, properties,
            properties ? properties->bounds.width : 1e37f,
            callback, resultbounds
        );

        if (properties) {
            if (properties->bounds.width > result.width) {
                result.width = properties->bounds.width;
            }

            if (properties->bounds.height > result.height) {
                result.height = properties->bounds.height;
            }

            if (properties->flags & kTextFlags::StrictBounds) {
                result.width = umin(result.width, properties->bounds.width);
                result.height = umin(result.height, properties->bounds.height);
            }
        }

        if (bounds) {
            *bounds = resultbounds;
        }
    }

    return result;
}


/*
 -------------------------------------------------------------------------------
 kCanvas implementation
 -------------------------------------------------------------------------------
*/

bool kCanvas::Initialize(Impl implementation)
{
    // TODO
    return true;
}

bool kCanvas::Shutdown()
{
    if (CanvasFactory::getImpl() == IMPL_NONE) {
        return false;
    }
    CanvasFactory::destroyFactory();
    return true;
}

void kCanvas::needResources(const kPen *pen, const kBrush *brush)
{
    if (pen) {
        pen->needResource();
    }
    if (brush) {
        brush->needResource();
    }
}

void kCanvas::Line(const kPoint &a, const kPoint &b, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->Line(a, b, pen);
    }
}

void kCanvas::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->Bezier(p1, p2, p3, p4, pen);
    }
}

void kCanvas::Arc(const kRect &rect, kScalar start, kScalar end, const kPen *pen)
{
    kPoint points[16];
    size_t count;
    ArcToBezierPoints(rect, start, end, points, count);
    PolyBezier(points, count, pen);
}

void kCanvas::PolyLine(const kPoint *points, size_t count, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->PolyLine(points, count, pen);
    }
}

void kCanvas::PolyBezier(const kPoint *points, size_t count, const kPen *pen)
{
    if (pen) {
        pen->needResource();
        p_impl->PolyBezier(points, count, pen);
    }
}

void kCanvas::Rectangle(const kRect &rect, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->Rectangle(rect, pen, brush);
}

void kCanvas::RoundedRectangle(const kRect &rect, const kSize &round, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->RoundedRectangle(rect, round, pen, brush);
}

void kCanvas::Ellipse(const kRect &rect, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->Ellipse(rect, pen, brush);
}

void kCanvas::Polygon(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->Polygon(points, count, pen, brush);
}

void kCanvas::PolygonBezier(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->PolygonBezier(points, count, pen, brush);
}

void kCanvas::DrawPath(const kPath *path, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->DrawPath(path->p_impl, pen, brush);
}

void kCanvas::DrawPath(const kPath *path, const kPen *pen, const kBrush *brush, const kPoint &offset)
{
    kTransform tfm;
    tfm.translate(offset.x, offset.y);
    DrawPath(path, pen, brush, tfm);
}

void kCanvas::DrawPath(const kPath *path, const kPen *pen, const kBrush *brush, const kTransform &transform)
{
    needResources(pen, brush);
    p_impl->DrawPath(path->p_impl, pen, brush, transform);
}

void kCanvas::DrawBitmap(const kBitmap *bitmap, const kPoint &origin, kScalar sourcealpha)
{
    kSize sz(kScalar(bitmap->width()), kScalar(bitmap->height()));
    p_impl->DrawBitmap(bitmap->p_impl, origin, sz, kPoint(), sz, sourcealpha);
}

void kCanvas::DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kPoint &source, const kSize &size, kScalar sourcealpha)
{
    p_impl->DrawBitmap(bitmap->p_impl, origin, size, source, size, sourcealpha);
}

void kCanvas::DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha)
{
    p_impl->DrawBitmap(bitmap->p_impl, origin, destsize, source, sourcesize, sourcealpha);
}

void kCanvas::DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin)
{
    if (brush) {
        brush->needResource();
        kSize sz(kScalar(mask->width()), kScalar(mask->height()));
        p_impl->DrawMask(mask->p_impl, brush, origin, sz, kPoint(), sz);
    }
}

void kCanvas::DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin, const kPoint &source, const kSize &size)
{
    if (brush) {
        brush->needResource();
        p_impl->DrawMask(mask->p_impl, brush, origin, size, source, size);
    }
}

void kCanvas::DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{
    if (brush) {
        brush->needResource();
        p_impl->DrawMask(mask->p_impl, brush, origin, destsize, source, sourcesize);
    }
}

void kCanvas::Text(const kPoint &p, const char *text, int count, const kFont *font, const kBrush *brush, kTextOrigin origin)
{
    if (brush && font) {
        if (brush) {
            brush->needResource();
        }
        if (font) {
            font->needResource();
        }

        if (count == -1) {
            count = int(strlen(text));
        }
        p_impl->Text(p, text, count, font, brush, origin);
    }
}

class RenderLayoutCallback
{
public:
    RenderLayoutCallback(
        const kPoint &origin, kCanvasImpl *impl,
        const kFont *font, const kBrush *brush
    ) :
        p_origin(origin),
        p_impl(impl),
        p_font(font),
        p_brush(brush)
    {}

    void operator()(LAYOUT_CALLBACK_PARAMS) const
    {
        p_impl->Text(p_origin + cp, text, count, p_font, p_brush, kTextOrigin::Top);
    }

private:
    kPoint        p_origin;
    kCanvasImpl  *p_impl;
    const kFont  *p_font;
    const kBrush *p_brush;
};


struct CachedWord
{
    const char *text;
    size_t      count;
    kPoint      position;
    kScalar     width;
    bool        newline;
};

typedef std::vector<CachedWord> CachedWordsList;

class CacheLayoutCallback
{
public:
    CacheLayoutCallback(CachedWordsList &cache) :
        p_cache(cache)
    {}

    void operator()(LAYOUT_CALLBACK_PARAMS) const
    {
        CachedWord word = {
            text, count, cp, width, newline
        };
        p_cache.push_back(word);
    }

private:
    CachedWordsList &p_cache;
};

void kCanvas::Text(const kRect &rect, const char *text, int count, const kFont *font, const kBrush *brush, const kTextOutProperties *properties)
{
    if (brush && font) {
        if (brush) {
            brush->needResource();
        }
        if (font) {
            font->needResource();
        }

        bool cliptobounds = properties ?
            (properties->flags & kTextFlags::ClipToBounds) != 0 : false;

        if (cliptobounds) {
            p_impl->BeginClippedDrawingByRect(rect);
        }

        kRect resultbounds;

        bool directoutput =
            properties == nullptr || (
                properties->horzalign == kTextHorizontalAlignment::Left &&
                properties->vertalign == kTextVerticalAlignment::Top &&
                (properties->flags & kTextFlags::Ellipses) == 0
            );

        if (directoutput) {
            RenderLayoutCallback callback(rect.getLeftTop(), p_impl, font, brush);
            TextLayout(
                p_impl, text, count, font, properties,
                rect.width(), callback, resultbounds
            );
        } else {
            kFontMetrics fm;

            CachedWordsList cache;
            cache.reserve(256);

            CacheLayoutCallback callback(cache);

            bool ellipses = (properties->flags & kTextFlags::Ellipses) != 0;

            kSize size = TextLayout(
                p_impl, text, count, font, properties,
                rect.width(), callback, resultbounds,
                ellipses ? &fm : nullptr
            );

            kSize alignboundssize;
            alignboundssize.width = umax(size.width, rect.width());
            alignboundssize.height = umax(size.height, rect.height());

            kScalar verticaloffset = 0;
            switch (properties->vertalign) {
                case kTextVerticalAlignment::Middle:
                    verticaloffset = alignboundssize.height * 0.5f - size.height * 0.5f;
                    break;

                case kTextVerticalAlignment::Bottom:
                    verticaloffset = alignboundssize.height - size.height;
                    break;
            }

            kScalar ellipseswidth = 0;
            if (ellipses) {
                kGlyphMetrics gm;
                p_impl->GetGlyphMetrics(font, '.', '.', &gm);
                ellipseswidth = gm.advance * 3;
            }

            size_t cw = 0;
            size_t sz = cache.size();
            while (cw < sz) {
                // search for row bounds
                size_t start = cw;
                while (cw < sz && !cache[cw].newline) {
                    ++cw;
                }

                kScalar width = cache[cw - 1].position.x + cache[cw - 1].width;

                kScalar totaloffset = 0;
                kScalar interwordspacing = 0;

                switch (properties->horzalign) {
                    case kTextHorizontalAlignment::Center:
                        totaloffset = alignboundssize.width * 0.5f - width * 0.5f;
                        break;

                    case kTextHorizontalAlignment::Right:
                        totaloffset = alignboundssize.width - width;
                        break;

                    case kTextHorizontalAlignment::Justify:
                        if (cw < sz) {
                            size_t spacecount = cw - start - 1;
                            if (spacecount) {
                                interwordspacing =
                                    (alignboundssize.width - width) /
                                    spacecount;
                            }
                        }
                        break;
                }

                bool stopoutput = false;
                kPoint cp = rect.getLeftTop() + kPoint(totaloffset, verticaloffset);
                for (size_t n = start; n < cw; ++n) {
                    const CachedWord &w = cache[n];

                    if (ellipses) {
                        // check if word crosses right bound (this is possible for
                        // single line text)
                        bool boundcrossing = (cp.x + w.position.x + w.width) > rect.right;

                        kScalar nextrowbottom =
                            cp.y + w.position.y + fm.height * 2 +
                            fm.linegap + properties->interval;

                        // check if this word is last word in a row and next row
                        // can't fit in bounds (so ellipses should be painted)
                        bool lastword = n == (cw - 1) && nextrowbottom > rect.bottom;

                        if (boundcrossing || lastword) {
                            // output word glyphs one by one while there's enough space
                            // to fit ellipses after word
                            cp += w.position;

                            if ((cp.x + w.width + ellipseswidth) < rect.right) {
                                p_impl->Text(
                                    cp, w.text, int(w.count),
                                    font, brush, kTextOrigin::Top
                                );
                                cp.x += w.width;
                            } else {
                                for (size_t c = 0; c < w.count; ++c) {
                                    kGlyphMetrics gm;
                                    size_t glyph = w.text[c];
                                    p_impl->GetGlyphMetrics(font, glyph, glyph, &gm);

                                    if ((cp.x + gm.advance) > (rect.right - ellipseswidth)) {
                                        break;
                                    }

                                    p_impl->Text(cp, w.text + c, 1, font, brush, kTextOrigin::Top);

                                    cp.x += gm.advance;
                                }
                            }

                            p_impl->Text(cp, "...", 3, font, brush, kTextOrigin::Top);

                            stopoutput = true;
                            break;
                        }
                    }

                    p_impl->Text(
                        cp + w.position , w.text, int(w.count),
                        font, brush, kTextOrigin::Top
                    );
                    cp.x += interwordspacing;
                }

                if (stopoutput) {
                    break;
                }

                if (cw < sz) {
                    cache[cw].newline = false;
                }
            }
        }

        if (cliptobounds) {
            p_impl->EndClippedDrawing();
        }
    }
}

void kCanvas::BeginClippedDrawing(const kBitmap *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{
    p_impl->BeginClippedDrawingByMask(mask->p_impl, transform, xextend, yextend);
}

void kCanvas::BeginClippedDrawing(const kPath *clip, const kTransform &transform)
{
    p_impl->BeginClippedDrawingByPath(clip->p_impl, transform);
}

void kCanvas::BeginClippedDrawing(const kRect &clip)
{
    p_impl->BeginClippedDrawingByRect(clip);
}

void kCanvas::EndClippedDrawing()
{
    p_impl->EndClippedDrawing();
}

void kCanvas::SetTransform(const kTransform &transform)
{
    kTransform t =
        p_transform_stack.size() > 1 ?
        p_transform_stack[p_transform_stack.size() - 1] * transform :
        transform;

    if (p_transform_stack.size()) {
        p_transform_stack.back() = t;
    }
    p_impl->SetTransform(t);
}

void kCanvas::PushTransform(const kTransform &transform)
{
    kTransform t =
        p_transform_stack.size() ?
        p_transform_stack.back() * transform :
        transform;

    p_transform_stack.push_back(t);

    p_impl->SetTransform(t);
}

void kCanvas::PopTransform()
{
    if (p_transform_stack.size()) {
        p_transform_stack.pop_back();
        p_impl->SetTransform(
            p_transform_stack.size() ?
                p_transform_stack.back() :
                kTransform()
        );
    }
}


/*
 -------------------------------------------------------------------------------
 kBitmapCanvas implementation
 -------------------------------------------------------------------------------
*/

kBitmapCanvas::kBitmapCanvas(const kBitmap *target, const kRectInt *rect) :
    kCanvas()
{
    p_impl->BindToBitmap(target->p_impl, rect);
}

kBitmapCanvas::~kBitmapCanvas()
{
    p_impl->Unbind();
}


/*
 -------------------------------------------------------------------------------
 kContextCanvas implementation
 -------------------------------------------------------------------------------
*/

kContextCanvas::kContextCanvas(kContext context, const kRectInt *rect) :
    kCanvas()
{
    p_impl->BindToContext(context, rect);
}

kContextCanvas::~kContextCanvas()
{
    p_impl->Unbind();
}
