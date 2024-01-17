/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    canvas.cpp
        main implementation file
        implements platform independend API functionality
*/

#include "canvas.h"
#include "canvasimpl.h"
#include "unicodeconverter.h"
#include <cstring>


using namespace k_canvas;
using namespace impl;
using namespace c_util;
using namespace c_geometry;


/*
 -------------------------------------------------------------------------------
 kSharedResourceBase object implementation
 -------------------------------------------------------------------------------
*/

template <typename Tdata>
kResourceObject* kSharedResourceBase<Tdata>::getResource(const Tdata &data) const
{
    return CanvasFactory::GetResource(data);
}


/*
 -------------------------------------------------------------------------------
 kStroke object implementation
 -------------------------------------------------------------------------------
*/

// helper function to copy strokes data
static void CopyStrokesData(StrokeData &data, const kScalar *strokes, size_t count)
{
    // TODO: handle strokes null case or invalid count cases
    if (data.p_style != kStrokeStyle::Custom) {
        // no custom stroke pattern accepted for styles other than Custom
        data.p_count = 0;
    } else {
        data.p_count = umin(count, size_t(MAX_STROKES));
        memcpy(data.p_stroke, strokes, data.p_count * sizeof(kScalar));
    }
}

kStroke::kStroke(kStrokeStyle style, kScalar dashoffset, const kScalar *strokes, size_t count)
{
    p_data.p_style = style;
    p_data.p_startcap = kCapStyle::Flat;
    p_data.p_endcap = kCapStyle::Flat;
    p_data.p_dashcap = kCapStyle::Flat;
    p_data.p_dashoffset = dashoffset;
    p_data.p_join = kLineJoin::Miter;
    CopyStrokesData(p_data, strokes, count);
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
    CopyStrokesData(p_data, strokes, count);
}

kStroke::~kStroke()
{}

kStroke::kStroke(const kStroke &source) :
    kStrokeBase(source)
{}

kStroke& kStroke::operator=(const kStroke &source)
{
    p_data = source.p_data;
    AssignResource(source);
    return *this;
}


/*
 -------------------------------------------------------------------------------
 kGradient object implementation
 -------------------------------------------------------------------------------
*/

kGradient::kGradient(const kColor start, const kColor end, kExtendType extend) :
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

kGradient::kGradient(kGradient &&source) :
    p_impl(source.p_impl)
{
    source.p_impl = nullptr;
}

kGradient &kGradient::operator=(kGradient &&source)
{
    ReleaseResource(p_impl);
    p_impl = source.p_impl;
    source.p_impl = nullptr;

    return *this;
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

kPen::kPen(const kColor color, kScalar width, kStrokeStyle style, const kScalar *strokes, size_t count)
{
    // stroke style and brush created implicitly
    kBrush brush(color);
    kStroke stroke(style, (int(width) & 1) * .5f, strokes, count);

    p_data.p_width = width;
    p_data.p_stroke = stroke.getResource();
    p_data.p_brush = brush.getResource();
}

kPen::kPen(const kColor color, kScalar width, const kStroke &stroke)
{
    // brush created implicitly
    kBrush brush(color);

    p_data.p_width = width;
    p_data.p_brush = brush.getResource();
    p_data.p_stroke = stroke.getResource();
}

kPen::kPen(const kBrush &brush, kScalar width, const kStroke &stroke)
{
    p_data.p_width = width;
    p_data.p_brush = brush.getResource();
    p_data.p_stroke = stroke.getResource();
}

kPen::~kPen()
{
    ReleaseResource(p_data.p_brush);
    ReleaseResource(p_data.p_stroke);
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

kPen &kPen::operator=(const kPen &source)
{
    p_data.p_width = source.p_data.p_width;

    AssignReferencedResource(source.p_data.p_stroke, p_data.p_stroke);
    AssignReferencedResource(source.p_data.p_brush, p_data.p_brush);

    AssignResource(source);

    return *this;
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

kBrush::kBrush(const kColor color)
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

kBrush::kBrush(kExtendType xextend, kExtendType yextend, const kBitmap &bitmap)
{
    p_data.p_style = kBrushStyle::Bitmap;
    p_data.p_gradient = nullptr;
    p_data.p_xextend = xextend;
    p_data.p_yextend = yextend;
    p_data.p_bitmap = bitmap.p_impl;
    p_data.p_bitmap->addref();
}

kBrush::~kBrush()
{
    ReleaseResource(p_data.p_gradient);
    ReleaseResource(p_data.p_bitmap);
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

kBrush &kBrush::operator=(const kBrush &source)
{
    p_data.p_style    = source.p_data.p_style;
    p_data.p_color    = source.p_data.p_color;
    p_data.p_start    = source.p_data.p_start;
    p_data.p_end      = source.p_data.p_end;
    p_data.p_radius   = source.p_data.p_radius;
    p_data.p_xextend  = source.p_data.p_xextend;
    p_data.p_yextend  = source.p_data.p_yextend;
    p_data.p_gradient = source.p_data.p_gradient;

    AssignReferencedResource(source.p_data.p_gradient, p_data.p_gradient);
    AssignReferencedResource(source.p_data.p_bitmap, p_data.p_bitmap);

    AssignResource(source);

    return *this;
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

kFont::~kFont()
{}

kFont::kFont(const kFont &source) :
    kFontBase(source)
{}

kFont &kFont::operator=(const kFont &source)
{
    p_data = source.p_data;
    AssignResource(source);
    return *this;
}


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
    m.rotate(-start);

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

kPath::kPath(impl::kPathImpl *result) :
    p_impl(result)
{
    // after Constructor's class Build() call, constructor doesn't own path impl.
    // object, so here's no addref() call should be made
}

kPath::kPath(const kPath &source, const kTransform &transform) :
    p_impl(CanvasFactory::CreatePath())
{
    p_impl->FromPath(source.p_impl, transform);
}

kPath::~kPath()
{
    ReleaseResource(p_impl);
}

kPath::kPath(kPath &&source) :
    p_impl(source.p_impl)
{
    source.p_impl = nullptr;
}

kPath &kPath::operator=(kPath &&source)
{
    ReleaseResource(p_impl);
    p_impl = source.p_impl;
    source.p_impl = nullptr;

    return *this;
}

kPath::Constructor kPath::Create()
{
    Constructor result;
    return result;
}


kPath::Constructor::Constructor() :
    p_impl(CanvasFactory::CreatePath())
{}

kPath::Constructor::Constructor(Constructor &source) :
    p_impl(source.p_impl)
{
    p_impl->addref();
}

kPath::Constructor::~Constructor()
{
    ReleaseResource(p_impl);
}

kPath::Constructor &kPath::Constructor::MoveTo(const kPoint &point)
{
    p_impl->MoveTo(point);
    return *this;
}

kPath::Constructor &kPath::Constructor::LineTo(const kPoint &point)
{
    p_impl->LineTo(point);
    return *this;
}

kPath::Constructor &kPath::Constructor::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{
    p_impl->BezierTo(p1, p2, p3);
    return *this;
}

kPath::Constructor &kPath::Constructor::ArcTo(const kRect &rect, kScalar start, kScalar end)
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

    return *this;
}

kPath::Constructor &kPath::Constructor::PolyLineTo(const kPoint *points, size_t count)
{
    p_impl->PolyLineTo(points, count);
    return *this;
}

kPath::Constructor &kPath::Constructor::PolyBezierTo(const kPoint *points, size_t count)
{
    p_impl->PolyBezierTo(points, count);
    return *this;
}

kPath::Constructor &kPath::Constructor::Text(const char *text, int count, const kFont &font, kTextOrigin origin)
{
    font.needResource();
    p_impl->Text(text, count, &font, origin);
    return *this;
}

kPath::Constructor &kPath::Constructor::Close()
{
    p_impl->Close();
    return *this;
}

impl::kPathImpl *kPath::Constructor::Build()
{
    auto result = p_impl;
    result->Commit();

    p_impl = nullptr;

    return result;
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

kBitmap::kBitmap(kBitmap &&source) :
    p_impl(source.p_impl),
    p_width(source.p_width),
    p_height(source.p_height),
    p_format(source.p_format)
{
    source.p_impl = nullptr;
    source.p_width = 0;
    source.p_height = 0;
}

kBitmap &kBitmap::operator=(kBitmap &&source)
{
    p_impl = source.p_impl;
    p_width = source.p_width;
    p_height = source.p_height;
    p_format = source.p_format;

    source.p_impl = nullptr;
    source.p_width = 0;
    source.p_height = 0;

    return *this;
}

void kBitmap::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, const void *data)
{
    p_impl->Update(updaterect, sourceformat, sourcepitch, data);
}


/*
 -------------------------------------------------------------------------------
 kWord & kWordBreaker helper classes
 -------------------------------------------------------------------------------
    Breaks text into words and other elements
*/

// defines word inside text by pointer to first char, length and type
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

// helps to split source text into words
class WordBreaker
{
public:
    // initialize word breaker with text, length and flags
    WordBreaker(const char *text, size_t length, kTextFlags flags) :
        p_text(reinterpret_cast<const unsigned char *>(text)),
        p_pos(0),
        p_length(length),
        p_linebreaks((flags & kTextFlags::IgnoreLineBreaks) == 0),
        p_tabstops((flags & kTextFlags::UseTabs) != 0)
    {}

    // extract next word from text
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

void kTextService::GetFontMetrics(const kFont &font, kFontMetrics &metrics)
{
    font.needResource();
    p_impl->GetFontMetrics(&font, metrics);
}

void kTextService::GetGlyphMetrics(const kFont &font, size_t first, size_t last, kGlyphMetrics *metrics)
{
    font.needResource();
    p_impl->GetGlyphMetrics(&font, first, last, metrics);
}

// helper function to count and measure word glyphs up until specified right edge from starting x position
static size_t MeasureUntil(kCanvasImpl *impl, const kFont &font, const char *text, size_t count, kScalar x, kScalar right, kScalar &actualwidth)
{
    size_t result = 0;
    kScalar startx = x;
    for (; result < count; ++result) {
        kGlyphMetrics gm;
        size_t glyph = *text++;
        impl->GetGlyphMetrics(&font, glyph, glyph, &gm);

        if ((x + gm.advance) > right) {
            break;
        }

        x += gm.advance;
    }
    actualwidth = x - startx;
    return result;
}

// helper layout callback
//      performs text to word split and layout of individual word blocks
//      each layed out word passed to provided callback function
//      text mesure functions use it to compute final text dimensions and metrics
//      text painting functions use it to render text with computed position
//
// this is "common" flow layout, eventually it should be changed to klayout generic
// algorithms
template <typename T, typename C>
static kSize TextLayout(
    kCanvasImpl *impl, const char *text, int count, const kFont *font,
    const T *properties, kScalar maxwidth, const C &callback, kRect &bounds,
    kFontMetrics *fontmetrics = nullptr
)
{
    // get basic space glyph and overall font metrics
    kGlyphMetrics spaceglyph;
    impl->GetGlyphMetrics(font, ' ', ' ', &spaceglyph);

    kFontMetrics fm;
    impl->GetFontMetrics(font, fm);
    if (fontmetrics) {
        *fontmetrics = fm;
    }

    // fill in properties

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
    bool mergespaces = (flags & kTextFlags::MergeSpaces) != 0;

    // perform layout

    WordBreaker wordbreaker(text, count, flags);

    kScalar tabwidth = 0;
    kSize result;
    kPoint cp(indent, 0);

    kScalar leftbound = 0;
    kScalar rightbound = 0;

    Word word;
    bool breaktonextline = false;
    while (wordbreaker.NextWord(word)) {
        switch (word.type) {
            case Word::Text: {
                // TODO
                // here might be tricky situation when the word itself bigger than
                // provided width to fit it in
                // in this case word should be broken in subwords which fit required width
                // propose an options for that or do it by default?
                //      in case without doing long word break - words will fall outside
                //      provided bounds

                kScalar wordwidth = impl->TextSize(word.text, word.length, font).width;

                // don't do line break if word doesn't fit and it's first word in a line
                breaktonextline =
                    breaktonextline ||
                    (multiline && cp.x > 0 && (cp.x + wordwidth) > maxwidth);

                if (breaktonextline) {
                    cp.x = 0;
                    cp.y += fm.height + fm.linegap + interval;
                    result.height = cp.y;
                }

                kGlyphMetrics gm;
                // take word's first glyph metrics for adjusting left bound
                size_t glyph = utf8codepoint(word.text);
                impl->GetGlyphMetrics(font, glyph, glyph, &gm);

                if ((cp.x + gm.leftbearing) < leftbound) {
                    leftbound = cp.x + gm.leftbearing;
                }

                // take word's last glyph metrics for adjusting right bound
                if (word.length > 1) {
                    glyph = word.text[word.length - 1];
                    impl->GetGlyphMetrics(font, glyph, glyph, &gm);
                }

                callback(cp, word.text, word.length, wordwidth, breaktonextline);
                cp.x += wordwidth;

                breaktonextline = false;

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
                    breaktonextline = true;
                } else {
                    cp.x += spaceglyph.advance;
                }
                break;

            case Word::Space:
                // NOTE: now all spaces at end of line are ignored if word-wrapping
                // occurs, actually spaces should wrap to next line too (except first one)
                //      this behaviour should be examined
                cp.x += spaceglyph.advance *
                    (mergespaces ? 1 : word.length);
                break;

            case Word::Tab:
                if (!usetabs) {
                    cp.x += spaceglyph.advance;
                } else {
                    while (tabwidth <= cp.x) {
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

    return result;
}

#define LAYOUT_CALLBACK_PARAMS\
    const kPoint &cp,\
    const char *text, size_t count,\
    kScalar width, bool newline

// TextLayout function does all the bounds computation, so its actual layout output not
// needed, this null callback just does nothing for reported words
class NullLayoutCallback
{
public:
    void operator()(LAYOUT_CALLBACK_PARAMS) const {}
};

kSize kTextService::TextSize(const char *text, int count, const kFont &font, const kTextSizeProperties *properties, kRect *bounds)
{
    kSize result;

    if (count == -1) {
        count = int(strlen(text));
    }
    if (count == 0) {
        return result;
    }

    font.needResource();

    // perform computation with layout helper function
    NullLayoutCallback callback;
    kRect resultbounds;
    result = TextLayout(
        p_impl, text, count, &font, properties,
        properties ? properties->bounds.width : 1e37f,
        callback, resultbounds
    );

    if (properties && properties->flags & kTextFlags::StrictBounds) {
        result.width = umin(result.width, properties->bounds.width);
        result.height = umin(result.height, properties->bounds.height);
    }

    if (bounds) {
        *bounds = resultbounds;
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

void kCanvas::Clear()
{
    p_impl->Clear();
}

void kCanvas::Line(const kPoint &a, const kPoint &b, const kPen &pen)
{
    pen.needResource();
    p_impl->Line(a, b, &pen);
}

void kCanvas::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPen &pen)
{
    pen.needResource();
    p_impl->Bezier(p1, p2, p3, p4, &pen);
}

void kCanvas::Arc(const kRect &rect, kScalar start, kScalar end, const kPen &pen)
{
    kPoint points[16];
    size_t count;
    ArcToBezierPoints(rect, start, end, points, count);
    PolyBezier(points, count, pen);
}

void kCanvas::PolyLine(const kPoint *points, size_t count, const kPen &pen)
{
    pen.needResource();
    p_impl->PolyLine(points, count, &pen);
}

void kCanvas::PolyBezier(const kPoint *points, size_t count, const kPen &pen)
{
    pen.needResource();
    p_impl->PolyBezier(points, count, &pen);
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

void kCanvas::DrawPath(const kPath &path, const kPen *pen, const kBrush *brush)
{
    needResources(pen, brush);
    p_impl->DrawPath(path.p_impl, pen, brush);
}

void kCanvas::DrawPath(const kPath &path, const kPen *pen, const kBrush *brush, const kPoint &offset)
{
    kTransform tfm;
    tfm.translate(offset.x, offset.y);
    DrawPath(path, pen, brush, tfm);
}

void kCanvas::DrawPath(const kPath &path, const kPen *pen, const kBrush *brush, const kTransform &transform)
{
    needResources(pen, brush);
    p_impl->DrawPath(path.p_impl, pen, brush, transform);
}

void kCanvas::DrawBitmap(const kBitmap &bitmap, const kPoint &origin, kScalar sourcealpha)
{
    kSize sz = bitmap.size();
    p_impl->DrawBitmap(bitmap.p_impl, origin, sz, kPoint(), sz, sourcealpha);
}

void kCanvas::DrawBitmap(const kBitmap &bitmap, const kPoint &origin, const kPoint &source, const kSize &size, kScalar sourcealpha)
{
    p_impl->DrawBitmap(bitmap.p_impl, origin, size, source, size, sourcealpha);
}

void kCanvas::DrawBitmap(const kBitmap &bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha)
{
    p_impl->DrawBitmap(bitmap.p_impl, origin, destsize, source, sourcesize, sourcealpha);
}

void kCanvas::DrawMask(const kBitmap &mask, kBrush &brush, const kPoint &origin)
{
    brush.needResource();
    kSize sz = mask.size();
    p_impl->DrawMask(mask.p_impl, &brush, origin, sz, kPoint(), sz);
}

void kCanvas::DrawMask(const kBitmap &mask, kBrush &brush, const kPoint &origin, const kPoint &source, const kSize &size)
{
    brush.needResource();
    p_impl->DrawMask(mask.p_impl, &brush, origin, size, source, size);
}

void kCanvas::DrawMask(const kBitmap &mask, kBrush &brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{
    brush.needResource();
    p_impl->DrawMask(mask.p_impl, &brush, origin, destsize, source, sourcesize);
}

void kCanvas::Text(const kPoint &p, const char *text, int count, const kFont &font, const kBrush &brush, kTextOrigin origin)
{
    if (count == -1) {
        count = int(strlen(text));
    }

    if (count == 0) {
        return;
    }

    brush.needResource();
    font.needResource();

    p_impl->Text(p, text, count, &font, &brush, origin);
}

// this callback class for TextLayout function actually renders provided word blocks
// it's used when it's possible to render layout result as is
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

// struct for storing TextLayout output provided by callback function
struct CachedWord
{
    const char *text;
    size_t      count;
    kPoint      position;
    kScalar     width;
    bool        newline;
};

typedef std::vector<CachedWord> CachedWordsList;

// this callback class for TextLayout helper function stores layout results in a cache
// before any actual output occurs
// when additional alignment and adjustment work required for some of text render behaviors
// it's being done on cached layout output
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

void kCanvas::Text(const kRect &rect, const char *text, int count, const kFont &font, const kBrush &brush, const kTextOutProperties *properties)
{
    if (count == -1) {
        count = int(strlen(text));
    }
    if (count == 0) {
        return;
    }

    brush.needResource();
    font.needResource();

    bool cliptobounds = properties ?
        (properties->flags & kTextFlags::ClipToBounds) != 0 : false;

    if (cliptobounds) {
        p_impl->BeginClippedDrawingByRect(rect);
    }

    kRect resultbounds;

    // this condition indicates whether it's possible to directly draw layout result
    bool directoutput =
        properties == nullptr || (
            properties->horzalign == kTextHorizontalAlignment::Left &&
            properties->vertalign == kTextVerticalAlignment::Top &&
            (properties->flags & kTextFlags::Ellipses) == 0
        );

    if (directoutput) {
        // no additional computation after layout required - pass render callback to
        // layout helper
        RenderLayoutCallback callback(rect.getLeftTop(), p_impl, &font, &brush);
        TextLayout(
            p_impl, text, count, &font, properties,
            rect.width(), callback, resultbounds
        );
    } else {
        // additional computation required after layout, collect layout data into cache

        kFontMetrics fm;

        CachedWordsList cache;
        cache.reserve(256);

        CacheLayoutCallback callback(cache);

        bool ellipses = (properties->flags & kTextFlags::Ellipses) != 0;

        kSize size = TextLayout(
            p_impl, text, count, &font, properties,
            rect.width(), callback, resultbounds,
            &fm
        );

        // compute bounds which will be used for text alignment
        kSize alignboundssize;
        alignboundssize.width = umax(size.width, rect.width());
        alignboundssize.height = umax(size.height, rect.height());

        // compute overall vertical offset for vertical alignment
        kScalar verticaloffset = 0;
        switch (properties->vertalign) {
            case kTextVerticalAlignment::Middle:
                verticaloffset = alignboundssize.height * 0.5f - size.height * 0.5f;
                break;

            case kTextVerticalAlignment::Bottom:
                verticaloffset = alignboundssize.height - size.height;
                break;
        }

        // compute ellipses width, if required
        kScalar ellipseswidth = 0;
        if (ellipses) {
            kGlyphMetrics gm;
            p_impl->GetGlyphMetrics(&font, '.', '.', &gm);
            ellipseswidth = gm.advance * 3;
        }

        // walk through cached layout result and render text line by line
        // with applying horizontal alignment and other options
        size_t cw = 0;
        size_t sz = cache.size();
        kScalar y = 0;
        while (cw < sz) {
            // search for row bounds
            size_t start = cw;
            while (cw < sz && !cache[cw].newline) {
                ++cw;
            }

            // compute total width of current line of text
            kScalar width = cache[cw - 1].position.x + cache[cw - 1].width;

            // compute horizontal alignment
            // interwordspacing is used for additional spacing to add between words
            // in order to get line fit whole text block width
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

            // compute current output position
            kPoint cp = rect.getLeftTop() + kPoint(totaloffset, verticaloffset);

            bool lasttextline = false;
            if (ellipses) {
                // check if this line of text is last row and next row
                // can't fit in bounds (so ellipses should be painted)
                kScalar nextrowbottom =
                    cp.y + y + fm.height * 2 + fm.linegap + properties->interval;
                lasttextline = nextrowbottom > rect.bottom;
            }

            // output line words one by one
            bool stopoutput = false;
            for (size_t n = start; n < cw; ++n) {
                const CachedWord &w = cache[n];

                if (ellipses) {
                    // check if word crosses right bound (this is possible for
                    // single line text or for adding ellipses on a last visible row)
                    kScalar wordrightbound = cp.x + w.position.x + w.width;

                    // if this is last visible line of text - adjust right bound to fit
                    // ellipses
                    if (lasttextline) {
                        wordrightbound += ellipseswidth;
                    }

                    // check if word is last in a visible row
                    bool lastword = lasttextline && n == (cw - 1);

                    if (wordrightbound > rect.right || lastword) {
                        cp += w.position;

                        if ((cp.x + w.width + ellipseswidth) < rect.right) {
                            p_impl->Text(
                                cp, w.text, w.count,
                                &font, &brush, kTextOrigin::Top
                            );
                            cp.x += w.width;
                        } else {
                            // measure word glyphs one by one while there's enough space
                            // to fit ellipses after word, then paint fitted glyphs of the word
                            kScalar fitwidth;
                            size_t c = MeasureUntil(p_impl, font, w.text, w.count, cp.x, rect.right - ellipseswidth, fitwidth);

                            p_impl->Text(cp, w.text, c, &font, &brush, kTextOrigin::Top);
                            cp.x += fitwidth;
                        }

                        p_impl->Text(cp, "...", 3, &font, &brush, kTextOrigin::Top);

                        // TODO: currently first word out of bounds will stop text output
                        // this seems to be layout issue, since it doesn't break long words
                        stopoutput = true;
                        break;
                    }
                }

                p_impl->Text(
                    cp + w.position , w.text, w.count,
                    &font, &brush, kTextOrigin::Top
                );
                cp.x += interwordspacing;
            }

            if (stopoutput) {
                break;
            }

            if (cw < sz) {
                cache[cw].newline = false;
                y += fm.height + fm.linegap + properties->interval;
            }
        }
    }

    if (cliptobounds) {
        p_impl->EndClippedDrawing();
    }
}

void kCanvas::BeginClippedDrawing(const kBitmap &mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{
    p_impl->BeginClippedDrawingByMask(mask.p_impl, transform, xextend, yextend);
}

void kCanvas::BeginClippedDrawing(const kPath &clip, const kTransform &transform)
{
    p_impl->BeginClippedDrawingByPath(clip.p_impl, transform);
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
    p_transform =
        p_transform_stack.size() ?
        p_transform_stack.back() * transform :
        transform;

    p_impl->SetTransform(p_transform);
}

void kCanvas::PushTransform(const kTransform &transform)
{
    p_transform_stack.push_back(p_transform);

    p_transform = p_transform * transform;
    p_impl->SetTransform(p_transform);
}

void kCanvas::PopTransform()
{
    if (p_transform_stack.size()) {
        p_transform = p_transform_stack.back();
        p_impl->SetTransform(p_transform);
        p_transform_stack.pop_back();
    }
}


/*
 -------------------------------------------------------------------------------
 kBitmapCanvas implementation
 -------------------------------------------------------------------------------
*/

kBitmapCanvas::kBitmapCanvas(const kBitmap &target, const kRectInt *rect) :
    kCanvas()
{
    p_impl->BindToBitmap(target.p_impl, rect);
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
