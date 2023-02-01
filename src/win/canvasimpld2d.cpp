/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    win/canvasimpld2d.cpp
        canvas API Direct2D implementation
*/

#include "canvasimpld2d.h"
#include "../unicodeconverter.h"

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

static inline D2D1_COLOR_F c2c(const kColor color)
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

static const D2D1_EXTEND_MODE extendmodes[2] = {
    D2D1_EXTEND_MODE_CLAMP,
    D2D1_EXTEND_MODE_WRAP
};


// these are really ugly macros definitions to help access internal factory and resource
// data
// they should be changed to somewhat more reliable
// TODO: getFactory() - is slow, think about speeding these methods
#define P_RT static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_rt
#define P_F  static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_factory
#define P_DW static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_dwrite_factory
#define P_FC static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_fontcollection

#define pen_not_empty pen && native(pen)[kD2DPen::RESOURCE_STYLE]
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
    P_RT->CreateGradientStopCollection(gs, UINT32(count), &p_gradient);
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
        size_t cnt = min(count, 32);
        for (size_t n = 0; n < cnt; n++) {
            cache[n] = p2pD2D(*points++);
        }

        p_sink->AddLines(cache, UINT32(cnt));
        count -= cnt;
    }

    p_cp = p2pD2D(*(points - 1));
}

void kPathImplD2D::PolyBezierTo(const kPoint *points, size_t count)
{
    OpenFigure();

    size_t curr_pt = 0;
    while (curr_pt < count) {
        size_t cnt = min(30, count - curr_pt) / 3;
        if (cnt == 0) {
            break;
        }

        D2D1_BEZIER_SEGMENT segments[10];
        for (size_t n = 0; n < cnt; n++) {
            segments[n].point1 = p2pD2D(points[curr_pt++]);
            segments[n].point2 = p2pD2D(points[curr_pt++]);
            segments[n].point3 = p2pD2D(points[curr_pt++]);
        }
        p_sink->AddBeziers(segments, UINT32(cnt));
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

    auto utf32text = count == -1 ? utf8toutf32(text) : utf8toutf32(text, count);

    auto length = utf32text.length();
    auto pos = size_t(0);

    const size_t BUFFER_LEN = 256;
    UINT16 indices[BUFFER_LEN];
    DWRITE_GLYPH_METRICS abc[BUFFER_LEN];
    DWRITE_GLYPH_OFFSET offsets[BUFFER_LEN];

    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);

    FLOAT fontEmSize = PointSizeToFontSize(_font_size);
    FLOAT k = fontEmSize / m.designUnitsPerEm;
    FLOAT originy = origin == kTextOrigin::BaseLine ? 0 : -m.ascent * k;

    while (pos < length) {
        size_t curlen = umin(length - pos, BUFFER_LEN);

        for (size_t n = 0; n < curlen; ++n) {
            offsets[n].advanceOffset = p_cp.x;
            offsets[n].ascenderOffset = + originy - p_cp.y;
        }
        _font_face->GetGlyphIndices(reinterpret_cast<const UINT32*>(utf32text.data()), UINT32(curlen), indices);

        _font_face->GetDesignGlyphMetrics(indices, UINT32(curlen), abc, FALSE);
        for (size_t n = 0; n < curlen; ++n) {
            p_cp.x += abc[n].advanceWidth * k;
        }

        _font_face->GetGlyphRunOutline(
            fontEmSize, indices, nullptr, offsets, UINT32(curlen),
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
    SafeRelease(p_path);
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
    sz.width = UINT32(width);
    sz.height = UINT32(height);

    D2D1_BITMAP_PROPERTIES props;
    props.dpiX = 0;
    props.dpiY = 0;
    props.pixelFormat.format = bitmapformats[size_t(format)];
    props.pixelFormat.alphaMode = alfamodes[size_t(format)];

    P_RT->CreateBitmap(sz, nullptr, 0, props, &p_bitmap);
}

void kBitmapImplD2D::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, const void *data)
{
    D2D1_SIZE_F size = p_bitmap->GetSize();

    kRectInt bitmaprect(0, 0, int(size.width), int(size.height));
    kRectInt update(updaterect ? bitmaprect.intersectionwith(*updaterect) : bitmaprect);

    D2D1_RECT_U rect;
    rect.left = update.left;
    rect.top = update.top;
    rect.right = update.right;
    rect.bottom = update.bottom;
    p_bitmap->CopyFromMemory(&rect, data, UINT32(sourcepitch));
}


/*
 -------------------------------------------------------------------------------
 kCanvasImplD2D object implementation
 -------------------------------------------------------------------------------
*/

kCanvasImplD2D::kCanvasImplD2D(const CanvasFactory *factory) :
    boundDC(0),
    boundBitmap(nullptr),
    clipStack(),
    origin()
{}

kCanvasImplD2D::~kCanvasImplD2D()
{
    Unbind();
}

void kCanvasImplD2D::Clear()
{
    P_RT->Clear();
}

bool kCanvasImplD2D::BindToBitmap(const kBitmapImpl *target, const kRectInt *rect)
{
    if (boundDC) {
        return false;
    }

    const kBitmapImplD2D *bitmap = static_cast<const kBitmapImplD2D*>(target);
    boundBitmap = bitmap->p_bitmap;
    boundBitmap->AddRef();

    D2D1_SIZE_U size = boundBitmap->GetPixelSize();
    kRectInt bitmaprect(0, 0, size.width, size.height);
    renderRect = kRectInt(rect ? bitmaprect.intersectionwith(*rect) : bitmaprect);

    boundDC = CreateCompatibleDC(0);

    BITMAPINFOHEADER bmi = {};
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = renderRect.width();
    bmi.biHeight = renderRect.height();

    HBITMAP targetBitmap = CreateDIBSection(
        boundDC, reinterpret_cast<const BITMAPINFO*>(&bmi),
        DIB_RGB_COLORS, &bitmapBits, 0, 0
    );

    prevBitmap = SelectObject(boundDC, HGDIOBJ(targetBitmap));

    RECT rc;
    rc.left = rc.top = 0;
    rc.right = renderRect.width();
    rc.bottom = renderRect.height();
    P_RT->BindDC(boundDC, &rc);
    P_RT->BeginDraw();

    // windows bitmaps are bottom top oriented, it's better to swap Y axis
    // for all the rendering to fast copy back from bitmap memory without
    // additional flipping
    origin.scale(1, -1);
    origin.translateby(
        kScalar(-renderRect.left),
        kScalar(renderRect.height() + renderRect.top)
    );
    P_RT->SetTransform(t2t(origin));

    // draw original contents of the bitmap to bound clean dc bitmap
    P_RT->DrawBitmap(boundBitmap);

    return true;
}

bool kCanvasImplD2D::BindToPrinter(kPrinter printer)
{
    return false;
}

bool kCanvasImplD2D::BindToContext(kContext context, const kRectInt *rect)
{
    if (boundDC) {
        return false;
    }

    boundDC = HDC(context);

    RECT rc;
    if (rect) {
        rc.left = rect->left;
        rc.top = rect->top;
        rc.right = rect->right;
        rc.bottom = rect->bottom;
    } else {
        memset(&rc, 0, sizeof(RECT));
        if (HWND wnd = WindowFromDC(boundDC)) {
            if (wnd != GetDesktopWindow()) {
                GetClientRect(wnd, &rc);
            }
        } else if (HGDIOBJ hbm = GetCurrentObject(boundDC, OBJ_BITMAP)) {
            BITMAP bm;
            memset(&bm, 0, sizeof(BITMAP));
            GetObject(hbm, sizeof(BITMAP), &bm);
            rc.left = rc.top = 0;
            rc.right = bm.bmWidth;
            rc.bottom = bm.bmHeight;
        }
    }

    P_RT->BindDC(boundDC, &rc);
    P_RT->BeginDraw();

    origin.translate(kScalar(-rc.left), kScalar(-rc.top));
    P_RT->SetTransform(t2t(origin));

    return true;
}

bool kCanvasImplD2D::Unbind()
{
    if (boundDC) {
        while (clipStack.size()) {
            EndClippedDrawing();
        }

        P_RT->EndDraw();

        if (boundBitmap) {
            // copy bitmap back to bound bitmap resource
            D2D1_SIZE_U size = boundBitmap->GetPixelSize();
            D2D1_RECT_U rect;
            rect.left = renderRect.left;
            rect.top = renderRect.top;
            rect.right = renderRect.right;
            rect.bottom = renderRect.bottom;
            boundBitmap->CopyFromMemory(&rect, bitmapBits, renderRect.width() * 4);
            SafeRelease(boundBitmap);

            DeleteObject(SelectObject(boundDC, prevBitmap));
            DeleteDC(boundDC);

            // restore transform (cause global single RT used)
            //P_RT->SetTransform(D2D1::IdentityMatrix());
        }

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
    if (!p->p_path) {
        return;
    }

    if (brush_not_empty) {
        P_RT->FillGeometry(p->p_path, _brush);
    }

    if (pen_not_empty) {
        P_RT->DrawGeometry(p->p_path, _pen);
    }
}

void kCanvasImplD2D::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform)
{
    // Q: why just not to change global transform?
    // A: because global transform will scale pen/brush properties

    const kPathImplD2D *p = reinterpret_cast<const kPathImplD2D*>(path);
    if (!p->p_path) {
        return;
    }

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

void kCanvasImplD2D::GetFontMetrics(const kFontBase *font, kFontMetrics &metrics)
{
    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);
    float k = PointSizeToDesignUnitsRatio(_font_size, m.designUnitsPerEm);

    metrics.ascent = m.ascent * k;
    metrics.descent = m.descent * k;
    metrics.height = metrics.ascent + metrics.descent;
    metrics.linegap = m.lineGap * k;
    metrics.underlinepos = m.underlinePosition * k;
    metrics.underlinewidth = m.underlineThickness * k;
    metrics.strikethroughpos = m.strikethroughPosition * k;
    metrics.strikethroughwidth = m.strikethroughThickness * k;
    metrics.capheight = m.capHeight * k;
    metrics.xheight = m.xHeight * k;
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
        UINT32 nn = UINT32(n);
        _font_face->GetGlyphIndices(&nn, 1, &index);

        DWRITE_GLYPH_METRICS abc;
        _font_face->GetDesignGlyphMetrics(&index, 1, &abc, FALSE);

        cm->leftbearing = abc.leftSideBearing * k;
        cm->advance = abc.advanceWidth * k;
        cm->rightbearing = -abc.rightSideBearing * k;

        ++cm;
    }
}

kSize kCanvasImplD2D::TextSize(const char *text, size_t count, const kFontBase *font)
{
    auto utf32text = utf8toutf32(text, count);

    auto length = utf32text.length();
    auto pos = size_t(0);

    kSize result;

    DWRITE_FONT_METRICS m;
    _font_face->GetMetrics(&m);
    float k = PointSizeToDesignUnitsRatio(_font_size, m.designUnitsPerEm);

    result.height = (m.ascent + m.descent) * k;

    const size_t BUFFER_LEN = 256;
    DWRITE_GLYPH_METRICS abc[BUFFER_LEN];
    UINT16 indices[BUFFER_LEN];

    while (pos < length) {
        size_t curlen = umin(length - pos, BUFFER_LEN);

        GetGlyphRunMetrics(utf32text.data() + pos, curlen, font, abc, indices);
        for (size_t n = 0; n < curlen; ++n) {
            result.width += abc[n].advanceWidth * k;
        }

        pos += curlen;
    }

    return result;
}

void kCanvasImplD2D::Text(const kPoint &p, const char *text, size_t count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin)
{
    auto utf32text = utf8toutf32(text, count);

    auto length = utf32text.length();
    auto pos = size_t(0);

    const size_t BUFFER_LEN = 256;
    UINT16 indices[BUFFER_LEN];
    DWRITE_GLYPH_METRICS abc[BUFFER_LEN];

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

        // TODO: pass isSideways
        GetGlyphRunMetrics(utf32text.data() + pos, curlen, font, abc, indices);
        FLOAT advance = 0;
        for (size_t n = 0; n < curlen; ++n) {
            advance += abc[n].advanceWidth * k;
        }

        run.glyphCount = UINT32(curlen);
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

void kCanvasImplD2D::BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{
    Clip masklayer;

    // create bitmap brush for masking
    D2D1_BITMAP_BRUSH_PROPERTIES brushprops;
    brushprops.extendModeX = extendmodes[size_t(xextend)];
    brushprops.extendModeY = extendmodes[size_t(yextend)];
    brushprops.interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
    P_RT->CreateBitmapBrush(
        static_cast<const kBitmapImplD2D*>(mask)->p_bitmap,
        brushprops, &masklayer.brush
    );
    masklayer.brush->SetTransform(t2t(transform));

    // create mask layer
    P_RT->CreateLayer(&masklayer.layer);

    D2D1_LAYER_PARAMETERS layerprops;
    layerprops.contentBounds = D2D1::InfiniteRect();
    layerprops.geometricMask = nullptr;
    layerprops.maskAntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
    layerprops.maskTransform = D2D1::IdentityMatrix();
    layerprops.opacity = 1.0f;
    layerprops.opacityBrush = masklayer.brush;
    layerprops.layerOptions = D2D1_LAYER_OPTIONS_NONE;
    P_RT->PushLayer(layerprops, masklayer.layer);

    clipStack.push_back(masklayer);
}

void kCanvasImplD2D::BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform)
{
    Clip cliplayer;
    cliplayer.brush = nullptr;

    // create mask layer
    P_RT->CreateLayer(&cliplayer.layer);

    D2D1_LAYER_PARAMETERS layerprops;
    layerprops.contentBounds = D2D1::InfiniteRect();
    layerprops.geometricMask = static_cast<const kPathImplD2D*>(clip)->p_path;
    layerprops.maskAntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
    layerprops.maskTransform = t2t(transform);
    layerprops.opacity = 1.0f;
    layerprops.opacityBrush = nullptr;
    layerprops.layerOptions = D2D1_LAYER_OPTIONS_NONE;
    P_RT->PushLayer(layerprops, cliplayer.layer);

    clipStack.push_back(cliplayer);
}

void kCanvasImplD2D::BeginClippedDrawingByRect(const kRect &clip)
{
    Clip cliplayer;
    cliplayer.layer = nullptr;
    cliplayer.brush = nullptr;

    P_RT->PushAxisAlignedClip(r2rD2D(clip), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    clipStack.push_back(cliplayer);
}

void kCanvasImplD2D::EndClippedDrawing()
{
    Clip &clip = clipStack.back();

    clip.layer || clip.brush ? P_RT->PopLayer() : P_RT->PopAxisAlignedClip();

    SafeRelease(clip.layer);
    SafeRelease(clip.brush);

    clipStack.pop_back();
}

void kCanvasImplD2D::SetTransform(const kTransform &transform)
{
    // transform always should be combined with origin
    P_RT->SetTransform(t2t(transform * origin));
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
        size_t cnt = min(32, count - curr_pt);
        D2D1_POINT_2F pts[32];
        for (size_t n = 0; n < cnt; n++) {
            pts[n] = p2pD2D(points[n + curr_pt]);
        }
        curr_pt += cnt;
        sink->AddLines(pts, UINT32(cnt));
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
        size_t cnt = min(30, count - curr_pt) / 3;
        if (cnt == 0) {
            break;
        }

        D2D1_BEZIER_SEGMENT segments[10];
        for (size_t n = 0; n < cnt; n++) {
            segments[n].point1 = p2pD2D(points[curr_pt++]);
            segments[n].point2 = p2pD2D(points[curr_pt++]);
            segments[n].point3 = p2pD2D(points[curr_pt++]);
        }
        sink->AddBeziers(segments, UINT32(cnt));
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

void kCanvasImplD2D::GetGlyphRunMetrics(
    const char32_t *codepoints, size_t curlen, const kFontBase *font,
    DWRITE_GLYPH_METRICS *abc, UINT16 *indices
)
{
    _font_face->GetGlyphIndices(reinterpret_cast<const UINT32*>(codepoints), UINT32(curlen), indices);
    _font_face->GetDesignGlyphMetrics(indices, UINT32(curlen), abc, FALSE);
}


/*
 -------------------------------------------------------------------------------
 kD2DStroke object implementation
 -------------------------------------------------------------------------------
*/

kD2DStroke::kD2DStroke(const StrokeData &stroke)
{
    D2D1_STROKE_STYLE_PROPERTIES props;
    props.dashCap = capstyles[size_t(stroke.p_dashcap)];
    props.dashOffset = stroke.p_dashoffset;

    float strokepattern[16];
    float *dashes = nullptr;
    UINT dash_count = 0;

    float dashlen = 3.0f;
    float dashspace = 1.0f;
    float dotlen = 1.0f;
    float dotspace = 1.0f;

    if (stroke.p_dashcap != kCapStyle::Flat) {
        dashlen = 2.0f;
        dashspace = 2.0f;
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
            dash_count = UINT(stroke.p_count);
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

    if (pen.p_brush) {
        pen.p_brush->setupNativeResources(native);
        p_brush = reinterpret_cast<ID2D1Brush*>(native[0]);
        p_brush->AddRef();
    } else {
        p_brush = nullptr;
    }

    if (pen.p_stroke) {
        pen.p_stroke->setupNativeResources(native);
        p_strokestyle = reinterpret_cast<ID2D1StrokeStyle*>(native[0]);
        p_strokestyle->AddRef();
    } else {
        p_strokestyle = nullptr;
    }

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

            D2D1_BITMAP_BRUSH_PROPERTIES p;
            p.extendModeX = extendmodes[size_t(brush.p_xextend)];
            p.extendModeY = extendmodes[size_t(brush.p_yextend)];
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
    wstring facename =
        font.p_facename[0] == 0 ? L"System" : utf8toutf16(font.p_facename);

    IDWriteTextFormat *format = nullptr;
    P_DW->CreateTextFormat(
        facename.c_str(), nullptr,
        font.p_style & kFontStyle::Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        font.p_style & kFontStyle::Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        PointSizeToFontSize(font.p_size),
        // TODO: add font locale support, if needed
        L"",
        &format
    );

    auto fontcollection = P_FC;

    WCHAR fontfamilyname[256];
    format->GetFontFamilyName(fontfamilyname, 256);
    UINT32 i;
    BOOL e;
    if (fontcollection->FindFamilyName(fontfamilyname, &i, &e) != S_OK) {
        // in case something goes wrong use just 0 font family
        i = 0;
    }

    IDWriteFontFamily *fontfamily;
    fontcollection->GetFontFamily(i, &fontfamily);
    fontfamily->GetFirstMatchingFont(
        format->GetFontWeight(), format->GetFontStretch(),
        format->GetFontStyle(), &p_font
    );

    p_font->CreateFontFace(&p_face);

    format->Release();
    fontfamily->Release();
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
    p_rt(nullptr),
    p_fontcollection(nullptr)
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
    if (p_dwrite_factory) {
        p_dwrite_factory->GetSystemFontCollection(&p_fontcollection);
    }
}

CanvasFactoryD2D::~CanvasFactoryD2D()
{
    SafeRelease(p_fontcollection);
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
