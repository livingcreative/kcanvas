/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    win/canvasimpld2d.cpp
        canvas API Direct2D implementation
*/

#include "canvasimpld2d.h"
#include <codecvt>
#include <locale>


using namespace c_util;
using namespace k_canvas;
using namespace impl;
using namespace std;


/*
 -------------------------------------------------------------------------------
 internal utility functions
 -------------------------------------------------------------------------------
*/

static inline D2D1_POINT_2F p2pD2D(const kPoint &p)
{
    D2D1_POINT_2F pt;
    pt.x = p.x;
    pt.y = p.y;
    return pt;
}

static inline D2D1_RECT_F r2rD2D(const kRect &rect)
{
    D2D1_RECT_F rc;
    rc.left = rect.left;
    rc.top = rect.top;
    rc.right = rect.right;
    rc.bottom = rect.bottom;
    return rc;
}

static inline D2D1_ROUNDED_RECT rr2rD2D(const kRect &rect, const kSize &round)
{
    D2D1_ROUNDED_RECT rc;
    rc.rect = r2rD2D(rect);
    rc.radiusX = round.width;
    rc.radiusY = round.height;
    return rc;
}

static inline D2D1_ELLIPSE r2eD2D(const kRect &rect)
{
    D2D1_ELLIPSE el;
    el.point = p2pD2D(rect.getCenter());
    el.radiusX = rect.width() * 0.5f;
    el.radiusY = rect.height() * 0.5f;
    return el;
}

static inline D2D1_COLOR_F c2c(const kColor &color)
{
    D2D1_COLOR_F clr;
    kColorReal colorf(color);
    clr.r = colorf.r;
    clr.g = colorf.g;
    clr.b = colorf.b;
    clr.a = colorf.a;
    return clr;
}

static inline D2D1_MATRIX_3X2_F t2t(const kTransform &transform)
{
    D2D1_MATRIX_3X2_F m;
    m._11 = transform.m00;
    m._12 = transform.m01;
    m._21 = transform.m10;
    m._22 = transform.m11;
    m._31 = transform.m20;
    m._32 = transform.m21;
    return m;
}

// Direct2D uses its own units for fornt size definition
// this function converts font point units (1/72") to Direct2D font units
static inline float PointSizeToFontSize(float size)
{
    return size * (1.0f / 72.0f * 96.0f);
}

// Direct2D font and glyph metrics returned in "font design" units
// this function converts font design units into pixel values
static inline float PointSizeToDesignUnitsRatio(float size, float designunitsperem)
{
    return PointSizeToFontSize(size) / designunitsperem;
}

// Just a COM interface safe release utility function template
template <typename T>
static inline void SafeRelease(T &object)
{
    if (object) {
        object->Release();
        object = nullptr;
    }
}


// these are really ugly macros definitions to help access internal factory and resource
// data
// they should be changed to somewhat more reliable
// TODO: getFactory() - is slow, think about speeding these methods
#define P_RT static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_rt
#define P_F static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_factory
#define P_DW static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_dwrite_factory

#define pen_not_empty pen
#define brush_not_empty brush && resourceData<BrushData>(brush).p_style != kBrushStyle::Clear

#define _pen reinterpret_cast<ID2D1Brush*>(native(pen)[kD2DPen::RESOURCE_BRUSH]), resourceData<PenData>(pen).p_width, reinterpret_cast<ID2D1StrokeStyle*>(native(pen)[kD2DPen::RESOURCE_STYLE])
#define _pen_width resourceData<PenData>(pen).p_width
#define _brush reinterpret_cast<ID2D1Brush*>(native(brush)[kD2DBrush::RESOURCE_BRUSH])
#define _font_face reinterpret_cast<IDWriteFontFace*>(kCanvasImplD2D::native(font)[kD2DFont::RESOURCE_FONTFACE])
#define _font_size kCanvasImplD2D::resourceData<FontData>(font).p_size
#define _font_style resourceData<FontData>(font).p_style


/*
 -------------------------------------------------------------------------------
 kGradientImplD2D object implementation
 -------------------------------------------------------------------------------
*/

kGradientImplD2D::kGradientImplD2D() :
    p_gradient(nullptr)
{}

kGradientImplD2D::~kGradientImplD2D()
{
    SafeRelease(p_gradient);
}

void kGradientImplD2D::Initialize(const kGradientStop *stops, size_t count, kExtendType extend)
{
    D2D1_GRADIENT_STOP *gs = new D2D1_GRADIENT_STOP[count];
    for (size_t n = 0; n < count; ++n) {
        gs[n].color = c2c(stops[n].color);
        gs[n].position = stops[n].position;
    }
    P_RT->CreateGradientStopCollection(gs, count, &p_gradient);
    delete[] gs;
}


/*
 -------------------------------------------------------------------------------
 kPathImplD2D object implementation
 -------------------------------------------------------------------------------
*/

kPathImplD2D::kPathImplD2D() :
    p_path(nullptr),
    p_sink(nullptr),
    p_opened(false)
{
    p_cp.x = 0;
    p_cp.y = 0;
}

kPathImplD2D::~kPathImplD2D()
{
    CloseSink();
    SafeRelease(p_path);
}

void kPathImplD2D::MoveTo(const kPoint &p)
{
    CloseFigure(true);
    p_cp = p2pD2D(p);
}

void kPathImplD2D::LineTo(const kPoint &p)
{
    OpenFigure();
    p_cp = p2pD2D(p);
    p_sink->AddLine(p_cp);
}

void kPathImplD2D::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{
    OpenFigure();
    D2D1_BEZIER_SEGMENT seg;
    seg.point1 = p2pD2D(p1);
    seg.point2 = p2pD2D(p2);
    seg.point3 = p2pD2D(p3);
    p_cp = seg.point3;
    p_sink->AddBezier(&seg);
}

void kPathImplD2D::PolyLineTo(const kPoint *points, size_t count)
{
    OpenFigure();

    D2D1_POINT_2F cache[32];
    while (count) {
        int cnt = min(count, 32);
        for (int n = 0; n < cnt; n++) {
            cache[n] = p2pD2D(*points++);
        }

        p_sink->AddLines(cache, cnt);
        count -= cnt;
    }

    p_cp = p2pD2D(*(points - 1));
}

void kPathImplD2D::PolyBezierTo(const kPoint *points, size_t count)
{
    OpenFigure();

    size_t curr_pt = 0;
    while (curr_pt < count) {
        int cnt = min(30, count - curr_pt) / 3;
        if (cnt == 0) {
            break;
        }

        D2D1_BEZIER_SEGMENT segments[10];
        for (int n = 0; n < cnt; n++) {
            segments[n].point1 = p2pD2D(points[curr_pt++]);
            segments[n].point2 = p2pD2D(points[curr_pt++]);
            segments[n].point3 = p2pD2D(points[curr_pt++]);
        }
        p_sink->AddBeziers(segments, cnt);
    }

    p_cp = p2pD2D(points[curr_pt - 1]);
}

void kPathImplD2D::Text(const char *text, int count, const kFontBase *font, kTextOrigin origin)
{
    if (p_sink) {
        CloseFigure(true);
    } else {
        OpenSink();
    }

    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = count == -1 ?
        convert.from_bytes(text) :
        convert.from_bytes(text, text + count);

    size_t length = t.length();
    size_t pos = 0;

    const size_t BUFFER_LEN = 256;
    UINT16 indices[BUFFER_LEN];
    DWRITE_GLYPH_METRICS abc[BUFFER_LEN];
    DWRITE_GLYPH_OFFSET offsets[BUFFER_LEN];
    UINT32 codepoints[BUFFER_LEN];

    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);

    FLOAT fontEmSize = PointSizeToFontSize(_font_size);
    FLOAT k = fontEmSize / m.designUnitsPerEm;
    FLOAT originy = origin == kTextOrigin::BaseLine ? 0 : -m.ascent * k;

    while (pos < length) {
        size_t curlen = umin(length - pos, BUFFER_LEN);

        for (size_t n = 0; n < curlen; ++n) {
            codepoints[n] = t[n + pos];
            offsets[n].advanceOffset = p_cp.x;
            offsets[n].ascenderOffset = + originy - p_cp.y;
        }
        _font_face->GetGlyphIndices(codepoints, curlen, indices);

        _font_face->GetDesignGlyphMetrics(indices, curlen, abc, FALSE);
        for (size_t n = 0; n < curlen; ++n) {
            p_cp.x += abc[n].advanceWidth * k;
        }

        _font_face->GetGlyphRunOutline(
            fontEmSize, indices, nullptr, offsets, curlen,
            FALSE, FALSE, p_sink
        );

        pos += curlen;
    }

    // This is looks like a bug! GetGlyphRunOutline somehow changes
    // geometry sink fill mode. MSDN docs says that SetFillMode() can be called
    // only before first OpenFigure() call.
    p_sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);
}

void kPathImplD2D::Close()
{
    CloseFigure(false);
}

void kPathImplD2D::Clear()
{
    CloseSink();
    p_path->Release();
    ID2D1PathGeometry *path;
    P_F->CreatePathGeometry(&path);
    p_path = path;
}

void kPathImplD2D::Commit()
{
    CloseSink();
}

void kPathImplD2D::FromPath(const kPathImpl *source, const kTransform &transform)
{
    p_path = static_cast<const kPathImplD2D*>(source)->MakeTransformedPath(t2t(transform));
}

ID2D1Geometry* kPathImplD2D::MakeTransformedPath(const D2D1_MATRIX_3X2_F &transform) const
{
    ID2D1TransformedGeometry *geometry;
    P_F->CreateTransformedGeometry(p_path, transform, &geometry);
    return geometry;
}

void kPathImplD2D::OpenSink()
{
    if (!p_sink) {
        ID2D1PathGeometry *path = nullptr;
        if (!p_path) {
            P_F->CreatePathGeometry(&path);
            p_path = path;
        }
        path->Open(&p_sink);
        // TODO: think of adding fill mode to kcanvas API
        //p_sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);
        p_opened = false;
    }
}
void kPathImplD2D::CloseSink()
{
    if (p_sink) {
        CloseFigure(true);
        p_sink->Close();
        p_sink->Release();
        p_sink = nullptr;
    }
}

void kPathImplD2D::OpenFigure()
{
    if (!p_opened) {
        OpenSink();
        p_sink->BeginFigure(p_cp, D2D1_FIGURE_BEGIN_FILLED);
        p_opened = true;
    }
}

void kPathImplD2D::CloseFigure(bool opened)
{
    if (p_opened) {
        p_sink->EndFigure(opened ? D2D1_FIGURE_END_OPEN : D2D1_FIGURE_END_CLOSED);
        p_opened = false;
    }
}


/*
 -------------------------------------------------------------------------------
 kBitmapImplD2D object implementation
 -------------------------------------------------------------------------------
*/

kBitmapImplD2D::kBitmapImplD2D() :
    p_bitmap(nullptr)
{}

kBitmapImplD2D::~kBitmapImplD2D()
{
    SafeRelease(p_bitmap);
}

static const DXGI_FORMAT bitmapformats[3] = {
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_A8_UNORM
};

static const D2D1_ALPHA_MODE alfamodes[3] = {
    D2D1_ALPHA_MODE_UNKNOWN,
    D2D1_ALPHA_MODE_PREMULTIPLIED,
    D2D1_ALPHA_MODE_STRAIGHT
};

void kBitmapImplD2D::Initialize(size_t width, size_t height, kBitmapFormat format)
{
    D2D1_SIZE_U sz;
    sz.width = width;
    sz.height = height;

    D2D1_BITMAP_PROPERTIES props;
    props.dpiX = 0;
    props.dpiY = 0;
    props.pixelFormat.format = bitmapformats[size_t(format)];
    props.pixelFormat.alphaMode = alfamodes[size_t(format)];

    P_RT->CreateBitmap(sz, nullptr, 0, props, &p_bitmap);
}

void kBitmapImplD2D::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data)
{
    D2D1_SIZE_F size = p_bitmap->GetSize();

    kRectInt bitmaprect(0, 0, int(size.width), int(size.height));
    kRectInt update = updaterect ? bitmaprect.intersectionwith(*updaterect) : bitmaprect;

    D2D1_RECT_U rect;
    rect.left = update.left;
    rect.top = update.top;
    rect.right = update.right;
    rect.bottom = update.bottom;
    p_bitmap->CopyFromMemory(&rect, data, sourcepitch);
}


/*
 -------------------------------------------------------------------------------
 kCanvasImplD2D object implementation
 -------------------------------------------------------------------------------
*/

kCanvasImplD2D::kCanvasImplD2D(const CanvasFactory *factory) :
    boundDC(0),
    maskBrush(nullptr),
    maskLayer(nullptr)
{}

kCanvasImplD2D::~kCanvasImplD2D()
{
    Unbind();
}

bool kCanvasImplD2D::BindToBitmap(const kBitmapImpl *target)
{
    return false;
}

bool kCanvasImplD2D::BindToPrinter(kPrinter printer)
{
    return false;
}

bool kCanvasImplD2D::BindToContext(kContext context)
{
    if (boundDC) {
        return false;
    }

    boundDC = HDC(context);

    RECT rc;
    memset(&rc, 0, sizeof(RECT));
    if (HWND wnd = WindowFromDC(HDC(context))) {
        if (wnd != GetDesktopWindow()) {
            GetClientRect(wnd, &rc);
        }
    } else if (HGDIOBJ hbm = GetCurrentObject(HDC(context), OBJ_BITMAP)) {
        BITMAP bm;
        memset(&bm, 0, sizeof(BITMAP));
        GetObject(hbm, sizeof(BITMAP), &bm);
        rc.left = rc.top = 0;
        rc.right = bm.bmWidth;
        rc.bottom = bm.bmHeight;
    }

    P_RT->BindDC(HDC(context), &rc);
    P_RT->BeginDraw();

    return true;
}

bool kCanvasImplD2D::Unbind()
{
    if (boundDC) {
        SetMask(nullptr);
        P_RT->EndDraw();
        boundDC = 0;
        return true;
    }
    return false;
}

void kCanvasImplD2D::Line(const kPoint &a, const kPoint &b, const kPenBase *pen)
{
    if (pen_not_empty) {
        P_RT->DrawLine(p2pD2D(a), p2pD2D(b), _pen);
    }
}

void kCanvasImplD2D::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen)
{
    if (pen_not_empty) {
        kPoint points[] = {p1, p2, p3, p4};

        ID2D1PathGeometry *g = GeometryFromPointsBezier(points, 4, false);
        P_RT->DrawGeometry(g, _pen);
        g->Release();
    }
}

void kCanvasImplD2D::PolyLine(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ID2D1PathGeometry *g = GeometryFromPoints(points, count, false);
        P_RT->DrawGeometry(g, _pen);
        g->Release();
    }
}

void kCanvasImplD2D::PolyBezier(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ID2D1PathGeometry *g = GeometryFromPointsBezier(points, count, false);
        P_RT->DrawGeometry(g, _pen);
        g->Release();
    }
}

void kCanvasImplD2D::Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        P_RT->FillRectangle(r2rD2D(rect), _brush);
    }
    if (pen_not_empty) {
        P_RT->DrawRectangle(r2rD2D(rect), _pen);
    }
}

void kCanvasImplD2D::RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        P_RT->FillRoundedRectangle(rr2rD2D(rect, round), _brush);
    }
    if (pen_not_empty) {
        P_RT->DrawRoundedRectangle(rr2rD2D(rect, round), _pen);
    }
}

void kCanvasImplD2D::Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        P_RT->FillEllipse(r2eD2D(rect), _brush);
    }
    if (pen_not_empty) {
        P_RT->DrawEllipse(r2eD2D(rect), _pen);
    }
}

void kCanvasImplD2D::Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    ID2D1PathGeometry *g = GeometryFromPoints(points, count, true);

    if (brush_not_empty) {
        P_RT->FillGeometry(g, _brush);
    }
    if (pen_not_empty) {
        P_RT->DrawGeometry(g, _pen);
    }

    g->Release();
}

void kCanvasImplD2D::PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    ID2D1PathGeometry *g = GeometryFromPointsBezier(points, count, true);

    if (brush_not_empty) {
        P_RT->FillGeometry(g, _brush);
    }
    if (pen_not_empty) {
        P_RT->DrawGeometry(g, _pen);
    }

    g->Release();
}

void kCanvasImplD2D::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush)
{
    const kPathImplD2D *p = reinterpret_cast<const kPathImplD2D*>(path);

    if (brush_not_empty) {
        P_RT->FillGeometry(p->p_path, _brush);
    }

    if (pen_not_empty) {
        P_RT->DrawGeometry(p->p_path, _pen);
    }
}

void kCanvasImplD2D::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform)
{
    const kPathImplD2D *p = reinterpret_cast<const kPathImplD2D*>(path);
    ID2D1Geometry *tp = p->MakeTransformedPath(t2t(transform));

    if (brush_not_empty) {
        P_RT->FillGeometry(tp, _brush);
    }

    if (pen_not_empty) {
        P_RT->DrawGeometry(tp, _pen);
    }

    tp->Release();
}

void kCanvasImplD2D::DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, float sourcealpha)
{
    const kBitmapImplD2D *p = reinterpret_cast<const kBitmapImplD2D*>(bitmap);
    P_RT->DrawBitmap(
        p->p_bitmap,
        r2rD2D(kRect(origin, destsize)),
        sourcealpha,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        r2rD2D(kRect(source, sourcesize))
    );
}

void kCanvasImplD2D::DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{
    const kBitmapImplD2D *p = reinterpret_cast<const kBitmapImplD2D*>(mask);

    D2D1_ANTIALIAS_MODE aa = P_RT->GetAntialiasMode();
    P_RT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

    P_RT->FillOpacityMask(
        p->p_bitmap, _brush,
        D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
        r2rD2D(kRect(origin, destsize)), r2rD2D(kRect(source, sourcesize))
    );

    P_RT->SetAntialiasMode(aa);
}

void kCanvasImplD2D::GetFontMetrics(const kFontBase *font, kFontMetrics *metrics)
{
    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);
    float k = PointSizeToDesignUnitsRatio(_font_size, m.designUnitsPerEm);

    metrics->ascent = m.ascent * k;
    metrics->descent = m.descent * k;
    metrics->height = metrics->ascent + metrics->descent;
    metrics->linegap = m.lineGap * k;
    metrics->underlinepos = m.underlinePosition * k;
    metrics->underlinewidth = m.underlineThickness * k;
    metrics->strikethroughpos = m.strikethroughPosition * k;
    metrics->strikethroughwidth = m.strikethroughThickness * k;
}

void kCanvasImplD2D::GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics)
{
    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);
    float k = PointSizeToDesignUnitsRatio(_font_size, m.designUnitsPerEm);

    kGlyphMetrics *cm = metrics;
    for (size_t n = first; n <= last; ++n) {
        // TODO: optimize for whole range
        UINT16 index = 0;
        _font_face->GetGlyphIndices(&n, 1, &index);

        DWRITE_GLYPH_METRICS abc;
        _font_face->GetDesignGlyphMetrics(&index, 1, &abc, FALSE);

        cm->a = abc.leftSideBearing * k;
        cm->b = abc.advanceWidth * k;
        cm->c = abc.rightSideBearing * k;

        ++cm;
    }
}

kSize kCanvasImplD2D::TextSize(const char *text, int count, const kFontBase *font, kSize *bounds)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = count == -1 ?
        convert.from_bytes(text) :
        convert.from_bytes(text, text + count);

    size_t length = t.length();
    size_t pos = 0;

    kSize result;

    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);
    float k = PointSizeToDesignUnitsRatio(_font_size, m.designUnitsPerEm);

    result.height = (m.ascent + m.descent) * k;

    const size_t BUFFER_LEN = 256;
    DWRITE_GLYPH_METRICS abc[BUFFER_LEN];
    UINT32 codepoints[BUFFER_LEN];
    UINT16 indices[BUFFER_LEN];

    while (pos < length) {
        size_t curlen = umin(length - pos, BUFFER_LEN);

        for (size_t n = 0; n < curlen; ++n) {
            codepoints[n] = t[n + pos];
        }
        _font_face->GetGlyphIndices(codepoints, curlen, indices);

        _font_face->GetDesignGlyphMetrics(indices, curlen, abc, FALSE);
        for (size_t n = 0; n < curlen; ++n) {
            result.width += abc[n].advanceWidth * k;
        }

        pos += curlen;

        if (bounds && pos == length) {
            *bounds = result;
            bounds->width -= abc[pos - 1].rightSideBearing * k;
            bounds->height += m.lineGap * k;
        }
    }

    return result;
}

void kCanvasImplD2D::Text(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = count == -1 ?
        convert.from_bytes(text) :
        convert.from_bytes(text, text + count);

    size_t length = t.length();
    size_t pos = 0;

    const size_t BUFFER_LEN = 256;
    UINT16 indices[BUFFER_LEN];
    DWRITE_GLYPH_METRICS abc[BUFFER_LEN];
    UINT32 codepoints[BUFFER_LEN];

    DWRITE_GLYPH_RUN run;
    run.fontFace      = _font_face;
    run.fontEmSize    = PointSizeToFontSize(_font_size);
    run.glyphIndices  = indices;
    run.glyphAdvances = nullptr;
    run.glyphOffsets  = nullptr;
    run.isSideways    = FALSE;
    run.bidiLevel     = 0;

    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);
    FLOAT k = run.fontEmSize / m.designUnitsPerEm;
    FLOAT originy = origin == kTextOrigin::BaseLine ? 0 : m.ascent * k;

    D2D1_POINT_2F cp = p2pD2D(p + kPoint(0, originy));
    while (pos < length) {
        size_t curlen = umin(length - pos, BUFFER_LEN);

        for (size_t n = 0; n < curlen; ++n) {
            codepoints[n] = t[n + pos];
        }
        _font_face->GetGlyphIndices(codepoints, curlen, indices);

        _font_face->GetDesignGlyphMetrics(indices, curlen, abc, run.isSideways);
        FLOAT advance = 0;
        for (size_t n = 0; n < curlen; ++n) {
            advance += abc[n].advanceWidth * k;
        }

        run.glyphCount = curlen;
        P_RT->DrawGlyphRun(cp, &run, _brush);
        cp.x += advance;

        pos += curlen;
    }

    // underlines and strikethroughs drawn as lines
    if (_font_style & kFontStyle::Underline) {
        kScalar offset = (m.ascent - m.underlinePosition + m.underlineThickness * 0.5f) * k;
        P_RT->DrawLine(
            p2pD2D(p + kPoint(0, offset)), p2pD2D(kPoint(cp.x, p.y + offset)),
            _brush, m.underlineThickness * k
        );
    }
    if (_font_style & kFontStyle::Strikethrough) {
        kScalar offset = (m.ascent - m.strikethroughPosition + m.strikethroughThickness * 0.5f) * k;
        P_RT->DrawLine(
            p2pD2D(p + kPoint(0, offset)), p2pD2D(kPoint(cp.x, p.y + offset)),
            _brush, m.strikethroughThickness * k
        );
    }
}

void kCanvasImplD2D::SetMask(kBitmapImpl *mask)
{
    if (maskLayer) {
        P_RT->PopLayer();
        SafeRelease(maskLayer);
        SafeRelease(maskBrush);
    }

    if (mask) {
        // create bitmap brush for masking
        D2D1_BITMAP_BRUSH_PROPERTIES brushprops;
        brushprops.extendModeX = D2D1_EXTEND_MODE_WRAP;
        brushprops.extendModeY = D2D1_EXTEND_MODE_WRAP;
        brushprops.interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
        P_RT->CreateBitmapBrush(
            static_cast<kBitmapImplD2D*>(mask)->p_bitmap,
            brushprops, &maskBrush
        );

        // create mask layer
        P_RT->CreateLayer(&maskLayer);

        D2D1_LAYER_PARAMETERS layerprops;
        layerprops.contentBounds = D2D1::InfiniteRect();
        layerprops.geometricMask = nullptr;
        layerprops.maskAntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
        layerprops.maskTransform = D2D1::IdentityMatrix();
        layerprops.opacity = 1.0f;
        layerprops.opacityBrush = maskBrush;
        layerprops.layerOptions = D2D1_LAYER_OPTIONS_NONE;
        P_RT->PushLayer(layerprops, maskLayer);
    }
}

ID2D1PathGeometry* kCanvasImplD2D::GeometryFromPoints(const kPoint *points, size_t count, bool closed)
{
    ID2D1PathGeometry *g;
    ID2D1GeometrySink *sink;
    P_F->CreatePathGeometry(&g);
    g->Open(&sink);

    sink->BeginFigure(p2pD2D(*points), D2D1_FIGURE_BEGIN_FILLED);

    size_t curr_pt = 1;
    while (curr_pt < count) {
        int cnt = min(32, count - curr_pt);
        D2D1_POINT_2F pts[32];
        for (int n = 0; n < cnt; n++) {
            pts[n] = p2pD2D(points[n + curr_pt]);
        }
        curr_pt += cnt;
        sink->AddLines(pts, cnt);
    }

    sink->EndFigure(closed ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
    sink->Close();

    sink->Release();

    return g;
}

ID2D1PathGeometry* kCanvasImplD2D::GeometryFromPointsBezier(const kPoint *points, size_t count, bool closed)
{
    ID2D1PathGeometry *g;
    ID2D1GeometrySink *sink;
    P_F->CreatePathGeometry(&g);
    g->Open(&sink);

    sink->BeginFigure(p2pD2D(*points), D2D1_FIGURE_BEGIN_FILLED);

    size_t curr_pt = 1;
    while (curr_pt < count) {
        int cnt = min(30, count - curr_pt) / 3;
        if (cnt == 0) {
            break;
        }

        D2D1_BEZIER_SEGMENT segments[10];
        for (int n = 0; n < cnt; n++) {
            segments[n].point1 = p2pD2D(points[curr_pt++]);
            segments[n].point2 = p2pD2D(points[curr_pt++]);
            segments[n].point3 = p2pD2D(points[curr_pt++]);
        }
        sink->AddBeziers(segments, cnt);
    }

    if (closed && (count - curr_pt) == 2) {
        D2D1_BEZIER_SEGMENT segment;
        segment.point1 = p2pD2D(points[curr_pt++]);
        segment.point2 = p2pD2D(points[curr_pt++]);
        segment.point3 = p2pD2D(points[0]);
        sink->AddBezier(segment);
    }

    sink->EndFigure(closed ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
    sink->Close();

    sink->Release();

    return g;
}


/*
 -------------------------------------------------------------------------------
 kD2DStroke object implementation
 -------------------------------------------------------------------------------
*/

static const D2D1_CAP_STYLE capstyles[3] = {
    D2D1_CAP_STYLE_FLAT,
    D2D1_CAP_STYLE_SQUARE,
    D2D1_CAP_STYLE_ROUND
};

static const D2D1_LINE_JOIN joinstyles[3] = {
    D2D1_LINE_JOIN_MITER,
    D2D1_LINE_JOIN_BEVEL,
    D2D1_LINE_JOIN_ROUND
};

kD2DStroke::kD2DStroke(const StrokeData &stroke)
{
    D2D1_STROKE_STYLE_PROPERTIES props;
    props.dashCap = capstyles[size_t(stroke.p_dashcap)];
    props.dashOffset = stroke.p_dashoffset;

    float strokepattern[16];
    float *dashes = nullptr;
    UINT dash_count = 0;

    float dashlen = 2.0f;
    float dashspace = 2.0f;
    float dotlen = 1.0f;
    float dotspace = 1.0f;

    if (stroke.p_dashcap != kCapStyle::Flat) {
        dashlen = 1.0f;
        dashspace = 3.0f;
        dotlen = 0.0f;
        dotspace = 2.0f;
    }

    switch (stroke.p_style) {
        case kStrokeStyle::Dot:
            props.dashStyle = D2D1_DASH_STYLE_CUSTOM;
            strokepattern[0] = dotlen;
            strokepattern[1] = dotspace;
            dashes = strokepattern;
            dash_count = 2;
            break;

        case kStrokeStyle::Dash:
            props.dashStyle = D2D1_DASH_STYLE_CUSTOM;
            strokepattern[0] = dashlen;
            strokepattern[1] = dashspace;
            dashes = strokepattern;
            dash_count = 2;
            break;

        case kStrokeStyle::DashDot:
            props.dashStyle = D2D1_DASH_STYLE_CUSTOM;
            strokepattern[0] = dashlen;
            strokepattern[1] = dotspace;
            strokepattern[2] = dotlen;
            strokepattern[3] = dotspace;
            dashes = strokepattern;
            dash_count = 4;
            break;

        case kStrokeStyle::DashDotDot:
            props.dashStyle = D2D1_DASH_STYLE_CUSTOM;
            strokepattern[0] = dashlen;
            strokepattern[1] = dotspace;
            strokepattern[2] = dotlen;
            strokepattern[3] = dotspace;
            strokepattern[4] = dotlen;
            strokepattern[5] = dotspace;
            dashes = strokepattern;
            dash_count = 6;
            break;

        case kStrokeStyle::Custom:
            props.dashStyle = D2D1_DASH_STYLE_CUSTOM;
            for (size_t n = 0; n < stroke.p_count; ++n) {
                strokepattern[n] = stroke.p_stroke[n];
            }
            dashes = strokepattern;
            dash_count = stroke.p_count;
            break;

        default:
            props.dashStyle = D2D1_DASH_STYLE_SOLID;
    }

    props.startCap = capstyles[size_t(stroke.p_startcap)];
    props.endCap = capstyles[size_t(stroke.p_endcap)];
    props.lineJoin = joinstyles[size_t(stroke.p_join)];
    // TODO: add miter limit property to kStroke
    props.miterLimit = 99.0; 

    P_F->CreateStrokeStyle(props, dashes, dash_count, &p_strokestyle);
}

kD2DStroke::~kD2DStroke()
{
    if (p_strokestyle) {
        UINT count = p_strokestyle->Release();
        int z = count;
    }
}


/*
 -------------------------------------------------------------------------------
 kD2DPen object implementation
 -------------------------------------------------------------------------------
*/

kD2DPen::kD2DPen(const PenData &pen)
{
    void *native[2];

    pen.p_brush->setupNativeResources(native);
    p_brush = reinterpret_cast<ID2D1Brush*>(native[0]);
    p_brush->AddRef();

    pen.p_stroke->setupNativeResources(native);
    p_strokestyle = reinterpret_cast<ID2D1StrokeStyle*>(native[0]);
    p_strokestyle->AddRef();

    p_width = pen.p_width;
}

kD2DPen::~kD2DPen()
{
    SafeRelease(p_brush);
    SafeRelease(p_strokestyle);
}


/*
 -------------------------------------------------------------------------------
 kD2DBrush object implementation
 -------------------------------------------------------------------------------
*/

kD2DBrush::kD2DBrush(const BrushData &brush) :
    p_brush(nullptr)
{
    if (brush.p_style == kBrushStyle::Clear) {
        return;
    }

    ID2D1GradientStopCollection *gradient = nullptr;
    if (brush.p_style == kBrushStyle::LinearGradient || brush.p_style == kBrushStyle::RadialGradient) {
        gradient = reinterpret_cast<kGradientImplD2D*>(brush.p_gradient)->p_gradient;
    }

    switch (brush.p_style) {
        case kBrushStyle::Solid:
            P_RT->CreateSolidColorBrush(
                c2c(brush.p_color),
                reinterpret_cast<ID2D1SolidColorBrush**>(&p_brush)
            );
            break;

        case kBrushStyle::LinearGradient: {
            D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES p = {
                p2pD2D(brush.p_start),
                p2pD2D(brush.p_end)
            };
            P_RT->CreateLinearGradientBrush(
                p, gradient,
                reinterpret_cast<ID2D1LinearGradientBrush**>(&p_brush)
            );
            break;
        }

        case kBrushStyle::RadialGradient: {
            D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES p;
            p.center = p2pD2D(brush.p_start);
            p.gradientOriginOffset = p2pD2D(brush.p_end);
            p.radiusX = brush.p_radius.width;
            p.radiusY = brush.p_radius.height;
            P_RT->CreateRadialGradientBrush(
                p, gradient,
                reinterpret_cast<ID2D1RadialGradientBrush**>(&p_brush)
            );
            break;
        }

        case kBrushStyle::Bitmap: {
            ID2D1Bitmap *bitmap = reinterpret_cast<kBitmapImplD2D*>(brush.p_bitmap)->p_bitmap;

            const D2D1_EXTEND_MODE modes[2] = {
                D2D1_EXTEND_MODE_CLAMP,
                D2D1_EXTEND_MODE_WRAP
            };

            D2D1_BITMAP_BRUSH_PROPERTIES p;
            p.extendModeX = modes[size_t(brush.p_xextend)];
            p.extendModeY = modes[size_t(brush.p_yextend)];
            p.interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;

            P_RT->CreateBitmapBrush(
                bitmap, p,
                reinterpret_cast<ID2D1BitmapBrush**>(&p_brush)
            );
            break;
        }
    }
}

kD2DBrush::~kD2DBrush()
{
    SafeRelease(p_brush);
}


/*
 -------------------------------------------------------------------------------
 kD2DFont object implementation
 -------------------------------------------------------------------------------
*/

kD2DFont::kD2DFont(const FontData &font)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = convert.from_bytes(font.p_facename);

    IDWriteTextFormat *format = nullptr;
    P_DW->CreateTextFormat(
        t.c_str(), nullptr,
        font.p_style & kFontStyle::Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        font.p_style & kFontStyle::Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        PointSizeToFontSize(font.p_size),
        // TODO: add font locale support, if needed
        L"",
        &format
    );

    IDWriteFontCollection *c;
    WCHAR n[256];
    format->GetFontCollection(&c);
    format->GetFontFamilyName(n, 256);
    UINT32 i;
    BOOL e;
    c->FindFamilyName(n, &i, &e);
    IDWriteFontFamily *ff;
    c->GetFontFamily(i, &ff);
    ff->GetFirstMatchingFont(
        format->GetFontWeight(), format->GetFontStretch(),
        format->GetFontStyle(), &p_font
    );

    p_font->CreateFontFace(&p_face);

    format->Release();
}

kD2DFont::~kD2DFont()
{
    SafeRelease(p_font);
    SafeRelease(p_face);
}


/*
 -------------------------------------------------------------------------------
 kD2DStrokeAllocator implementation
 -------------------------------------------------------------------------------
*/

kD2DStroke* kD2DStrokeAllocator::createResource(const StrokeData &stroke)
{
    return new kD2DStroke(stroke);
}

void kD2DStrokeAllocator::deleteResource(kD2DStroke *stroke)
{
    delete stroke;
}


/*
 -------------------------------------------------------------------------------
 kD2DPenAllocator implementation
 -------------------------------------------------------------------------------
*/

kD2DPen* kD2DPenAllocator::createResource(const PenData &pen)
{
    return new kD2DPen(pen);
}

void kD2DPenAllocator::deleteResource(kD2DPen *pen)
{
    delete pen;
}


/*
 -------------------------------------------------------------------------------
 kD2DBrushAllocator implementation
 -------------------------------------------------------------------------------
*/

kD2DBrush* kD2DBrushAllocator::createResource(const BrushData &brush)
{
    return new kD2DBrush(brush);
}

void kD2DBrushAllocator::deleteResource(kD2DBrush *brush)
{
    delete brush;
}


/*
 -------------------------------------------------------------------------------
 kD2DFontAllocator implementation
 -------------------------------------------------------------------------------
*/

kD2DFont* kD2DFontAllocator::createResource(const FontData &font)
{
    return new kD2DFont(font);
}

void kD2DFontAllocator::deleteResource(kD2DFont *font)
{
    delete font;
}


typedef HRESULT (WINAPI *D2D1CreateFactoryPtr) (
    __in D2D1_FACTORY_TYPE factoryType,
    __in REFIID riid,
    __in_opt CONST D2D1_FACTORY_OPTIONS *pFactoryOptions,
    __out void **ppIFactory
);

typedef HRESULT __declspec(dllimport) (WINAPI *DWriteCreateFactoryPtr) (
    __in DWRITE_FACTORY_TYPE factoryType,
    __in REFIID iid,
    __out IUnknown **factory
);

CanvasFactoryD2D::CanvasFactoryD2D() :
    p_d2d1_dll(0),
    p_dwrite_dll(0),
    p_factory(nullptr),
    p_dwrite_factory(nullptr),
    p_rt(nullptr)
{
    p_d2d1_dll = LoadLibraryA("d2d1.dll");
    if (p_d2d1_dll == nullptr) {
        return;
    }

    p_dwrite_dll = LoadLibraryA("dwrite.dll");
    if (p_dwrite_dll == nullptr) {
        return;
    }

    D2D1CreateFactoryPtr D2D1CreateFactory = D2D1CreateFactoryPtr(
        GetProcAddress(p_d2d1_dll, "D2D1CreateFactory")
    );
    D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_ID2D1Factory,
        nullptr, (void**)&p_factory
    );
    if (p_factory) {
        D2D1_RENDER_TARGET_PROPERTIES props;
        props.dpiX = 0;
        props.dpiY = 0;
        props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
        props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        props.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
        props.usage = D2D1_RENDER_TARGET_USAGE_NONE;

        p_factory->CreateDCRenderTarget(&props, &p_rt);
        p_rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    }

    DWriteCreateFactoryPtr DWriteCreateFactory = DWriteCreateFactoryPtr(
        GetProcAddress(p_dwrite_dll, "DWriteCreateFactory")
    );
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&p_dwrite_factory)
    );
}

CanvasFactoryD2D::~CanvasFactoryD2D()
{
    SafeRelease(p_rt);
    SafeRelease(p_factory);
    SafeRelease(p_dwrite_factory);

    if (p_d2d1_dll) {
        FreeLibrary(p_d2d1_dll);
    }

    if (p_dwrite_dll) {
        FreeLibrary(p_dwrite_dll);
    }
}

bool CanvasFactoryD2D::initialized()
{
    return p_factory && p_dwrite_factory;
}
