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


static inline D2D1_POINT_2F p2pD2D(const kPoint &p)
{
    D2D1_POINT_2F pt;
    pt.x = p.x;
    pt.y = p.y;
    return pt;
}

static inline D2D1_RECT_F r2rD2D(const kRect &rect, kScalar pen_width)
{
    D2D1_RECT_F rc;
    kScalar offset = 0.5f * (roundint(pen_width) & 1);
    rc.left = rect.left + offset;
    rc.top = rect.top + offset;
    rc.right = rect.right - offset;
    rc.bottom = rect.bottom - offset;
    return rc;
}

static inline D2D1_RECT_F r2rD2D_fill(const kRect &rect)
{
    D2D1_RECT_F rc;
    rc.left = rect.left;
    rc.top = rect.top;
    rc.right = rect.right;
    rc.bottom = rect.bottom;
    return rc;
}

static inline D2D1_ROUNDED_RECT rr2rD2D(const kRect &rect, const kSize &round, kScalar pen_width)
{
    D2D1_ROUNDED_RECT rc;
    rc.rect = r2rD2D(rect, pen_width);
    rc.radiusX = round.width;
    rc.radiusY = round.height;
    return rc;
}

static inline D2D1_ROUNDED_RECT rr2rD2D_fill(const kRect &rect, const kSize &round)
{
    D2D1_ROUNDED_RECT rc;
    rc.rect = r2rD2D_fill(rect);
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


// TODO: getFactory() - is slow, think about speeding these methods
#define P_RT static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_rt
#define P_F static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_factory
#define P_DW static_cast<CanvasFactoryD2D*>(CanvasFactory::getFactory())->p_dwrite_factory


kGradientImplD2D::kGradientImplD2D() :
    p_gradient(nullptr)
{}

kGradientImplD2D::~kGradientImplD2D()
{
    if (p_gradient) {
        p_gradient->Release();
    }
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


kPathImplD2D::kPathImplD2D() :
    p_sink(nullptr),
    p_opened(false)
{
    P_F->CreatePathGeometry(&p_path);
}

kPathImplD2D::~kPathImplD2D()
{
    CloseSink();
    if (p_path) {
        p_path->Release();
    }
}

void kPathImplD2D::MoveTo(const kPoint &p)
{
    CloseFigure(true);
    OpenFigure(p2pD2D(p));
}

void kPathImplD2D::LineTo(const kPoint &p)
{
    OpenFigure(p2pD2D(kPoint()));
    p_sink->AddLine(p2pD2D(p));
}

void kPathImplD2D::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{
    OpenFigure(p2pD2D(kPoint()));
    D2D1_BEZIER_SEGMENT seg;
    seg.point1 = p2pD2D(p1);
    seg.point2 = p2pD2D(p2);
    seg.point3 = p2pD2D(p3);
    p_sink->AddBezier(&seg);
}

void kPathImplD2D::PolyLineTo(const kPoint *points, size_t count)
{
    D2D1_POINT_2F cache[32];

    if (!p_opened) {
        OpenFigure(p2pD2D(*points++));
        count--;
    }

    while (count) {
        int cnt = min(count, 32);
        for (int n = 0; n < cnt; n++) {
            cache[n] = p2pD2D(*points++);
        }

        p_sink->AddLines(cache, cnt);
        count -= cnt;
    }
}

void kPathImplD2D::PolyBezierTo(const kPoint *points, size_t count)
{
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
}

void kPathImplD2D::Text(const char *text, const kFontBase *font)
{
    CloseFigure(true);
    // TODO: implement text
}

void kPathImplD2D::Close()
{
    CloseFigure(false);
}

void kPathImplD2D::Clear()
{
    CloseSink();
    p_path->Release();
    P_F->CreatePathGeometry(&p_path);
}

void kPathImplD2D::Commit()
{
    CloseSink();
}

void kPathImplD2D::OpenSink()
{
    if (!p_sink) {
        p_path->Open(&p_sink);
        // TODO: think of adding fill mode to kcanvas API
        //p_sink->SetFillMode(D2D1_FILL_MODE_WINDING);
        p_opened = false;
    }
}
void kPathImplD2D::CloseSink() const
{
    if (p_sink) {
        CloseFigure(true);
        p_sink->Close();
        p_sink->Release();
        p_sink = nullptr;
    }
}

void kPathImplD2D::OpenFigure(const D2D1_POINT_2F &p)
{
    if (!p_opened) {
        OpenSink();
        p_sink->BeginFigure(p, D2D1_FIGURE_BEGIN_FILLED);
        p_opened = true;
    }
}

void kPathImplD2D::CloseFigure(bool opened) const
{
    if (p_opened) {
        p_sink->EndFigure(opened ? D2D1_FIGURE_END_OPEN : D2D1_FIGURE_END_CLOSED);
        p_opened = false;
    }
}



kBitmapImplD2D::kBitmapImplD2D() :
    p_bitmap(nullptr)
{}

kBitmapImplD2D::~kBitmapImplD2D()
{
    if (p_bitmap) {
        p_bitmap->Release();
    }
}

void kBitmapImplD2D::Initialize(size_t width, size_t height, kBitmapFormat format)
{
    D2D1_SIZE_U sz;
    sz.width = width;
    sz.height = height;

    D2D1_BITMAP_PROPERTIES props;
    props.dpiX = 0;
    props.dpiY = 0;
    props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

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



kCanvasImplD2D::kCanvasImplD2D(const CanvasFactory *factory) :
    boundDC(0)
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
    //P_RT->Clear();

    return true;
}

bool kCanvasImplD2D::Unbind()
{
    if (boundDC) {
        P_RT->EndDraw();
        boundDC = 0;
        return true;
    }
    return false;
}


#define pen_not_empty pen //&& penData(pen).p_style != kPenStyle::Clear
#define brush_not_empty brush && brushData(brush).p_style != kBrushStyle::Clear

#define _pen reinterpret_cast<ID2D1Brush*>(native(pen)[kD2DPen::RESOURCE_BRUSH]), penData(pen).p_width, reinterpret_cast<ID2D1StrokeStyle*>(native(pen)[kD2DPen::RESOURCE_STYLE])
#define _pen_width penData(pen).p_width
#define _brush reinterpret_cast<ID2D1Brush*>(native(brush)[kD2DBrush::RESOURCE_BRUSH])
#define _font_format (reinterpret_cast<kD2DFont*>(font.resource))->getTextFormat()

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

        ID2D1PathGeometry *g = GeomteryFromPointsBezier(points, 4, false);
        P_RT->DrawGeometry(g, _pen);
        g->Release();
    }
}

void kCanvasImplD2D::PolyLine(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ID2D1PathGeometry *g = GeomteryFromPoints(points, count, false);
        P_RT->DrawGeometry(g, _pen);
        g->Release();
    }
}

void kCanvasImplD2D::PolyBezier(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        ID2D1PathGeometry *g = GeomteryFromPointsBezier(points, count, false);
        P_RT->DrawGeometry(g, _pen);
        g->Release();
    }
}

void kCanvasImplD2D::Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        P_RT->FillRectangle(r2rD2D_fill(rect), _brush);
    }
    if (pen_not_empty) {
        P_RT->DrawRectangle(r2rD2D(rect, _pen_width), _pen);

        ID2D1StrokeStyle *stroke = reinterpret_cast<ID2D1StrokeStyle*>(native(pen)[kD2DPen::RESOURCE_STYLE]);
        stroke->AddRef();
        UINT count = stroke->Release();
        int z = count;
    }
}

void kCanvasImplD2D::RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        P_RT->FillRoundedRectangle(rr2rD2D_fill(rect, round), _brush);
    }
    if (pen_not_empty) {
        P_RT->DrawRoundedRectangle(rr2rD2D(rect, round, _pen_width), _pen);
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
    ID2D1PathGeometry *g = GeomteryFromPoints(points, count, true);

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
    ID2D1PathGeometry *g = GeomteryFromPointsBezier(points, count, true);

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
    // TODO: path commitment should be checked inside kCanvas implementation
    p->CloseSink();
    
    if (brush_not_empty) {
        P_RT->FillGeometry(p->p_path, _brush);
    }
    
    if (pen_not_empty) {
        P_RT->DrawGeometry(p->p_path, _pen);
    }
}

void kCanvasImplD2D::DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, float sourcealpha)
{
    const kBitmapImplD2D *p = reinterpret_cast<const kBitmapImplD2D*>(bitmap);
    P_RT->DrawBitmap(
        p->p_bitmap,
        r2rD2D_fill(kRect(origin, destsize)),
        sourcealpha,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        r2rD2D_fill(kRect(source, sourcesize))
    );
}

kSize kCanvasImplD2D::TextSize(const char *text, int count, const kFontBase *font)
{
    return kSize();
    /*
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = convert.from_bytes(text);

    IDWriteTextLayout *layout;
    DWRITE_TEXT_METRICS tm;
    P_DW->CreateTextLayout(t.c_str(), t.length(), _font_format, 0.0f, 0.0f, &layout);
    layout->GetMetrics(&tm);
    kSize res(tm.width, tm.height);
    layout->Release();
    return res;
    */
}

void kCanvasImplD2D::TextOut(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush)
{
    /*
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = convert.from_bytes(text);

    P_RT->DrawTextW(
        t.c_str(), t.length(), _font_format,
        D2D1::RectF(p.x, p.y, p.x + 9999, p.y + 9999), _brush
    );
    */
}

ID2D1PathGeometry* kCanvasImplD2D::GeomteryFromPoints(const kPoint *points, size_t count, bool closed)
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

ID2D1PathGeometry* kCanvasImplD2D::GeomteryFromPointsBezier(const kPoint *points, size_t count, bool closed)
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
    props.miterLimit = /*pen.p_width*/99.0; // TODO: check miter limit

    P_F->CreateStrokeStyle(props, dashes, dash_count, &p_strokestyle);
}

kD2DStroke::~kD2DStroke()
{
    if (p_strokestyle) {
        UINT count = p_strokestyle->Release();
        int z = count;
    }
}


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
    if (p_brush) {
        p_brush->Release();
    }

    if (p_strokestyle) {
        p_strokestyle->Release();
    }
}


kD2DBrush::kD2DBrush(const BrushData &brush) :
    p_brush(nullptr),
    p_gradient(nullptr),
    p_bitmap(nullptr)
{
    if (brush.p_style == kBrushStyle::Clear) {
        return;
    }

    if (brush.p_style == kBrushStyle::LinearGradient || brush.p_style == kBrushStyle::RadialGradient) {
        p_gradient = reinterpret_cast<kGradientImplD2D*>(brush.p_gradient)->p_gradient;
        p_gradient->AddRef();
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
                p, p_gradient,
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
                p, p_gradient,
                reinterpret_cast<ID2D1RadialGradientBrush**>(&p_brush)
            );
            break;
        }

        case kBrushStyle::Bitmap: {
            p_bitmap = reinterpret_cast<kBitmapImplD2D*>(brush.p_bitmap)->p_bitmap;
            p_bitmap->AddRef();

            const D2D1_EXTEND_MODE modes[2] = {
                D2D1_EXTEND_MODE_CLAMP,
                D2D1_EXTEND_MODE_WRAP
            };

            D2D1_BITMAP_BRUSH_PROPERTIES p;
            p.extendModeX = modes[size_t(brush.p_xextend)];
            p.extendModeY = modes[size_t(brush.p_yextend)];
            p.interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;

            P_RT->CreateBitmapBrush(
                p_bitmap,
                p,
                reinterpret_cast<ID2D1BitmapBrush**>(&p_brush)
            );
            break;
        }
    }
}

kD2DBrush::~kD2DBrush()
{
    if (p_brush) {
        p_brush->Release();
    }
    if (p_gradient) {
        p_gradient->Release();
    }
    if (p_bitmap) {
        p_bitmap->Release();
    }
}


kD2DFont::kD2DFont(const FontData &font)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = convert.from_bytes(font.p_facename);

    P_DW->CreateTextFormat(
        t.c_str(), nullptr,
        font.p_style & kFontStyle::Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        font.p_style & kFontStyle::Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font.p_size / 72.0f * 96.0f,
        L"",
        &p_textformat
    );
    p_textformat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

    IDWriteFontCollection *c;
    WCHAR n[256];
    p_textformat->GetFontCollection(&c);
    p_textformat->GetFontFamilyName(n, 256);
    UINT32 i;
    BOOL e;
    c->FindFamilyName(n, &i, &e);
    IDWriteFontFamily *ff;
    c->GetFontFamily(i, &ff);
    ff->GetFirstMatchingFont(
        p_textformat->GetFontWeight(), p_textformat->GetFontStretch(),
        p_textformat->GetFontStyle(), &p_font
    );
}

kD2DFont::~kD2DFont()
{
    if (p_textformat) {
        p_textformat->Release();
    }
    if (p_font) {
        p_font->Release();
    }
}


kD2DStroke* kD2DStrokeAllocator::createResource(const StrokeData &stroke)
{
    //OutputDebugStringA("PEN created\n");
    return new kD2DStroke(stroke);
}

void kD2DStrokeAllocator::deleteResource(kD2DStroke *stroke)
{
    //OutputDebugStringA("PEN destroyed\n");
    delete stroke;
}



kD2DPen* kD2DPenAllocator::createResource(const PenData &pen)
{
    //OutputDebugStringA("PEN created\n");
    return new kD2DPen(pen);
}

void kD2DPenAllocator::deleteResource(kD2DPen *pen)
{
    //OutputDebugStringA("PEN destroyed\n");
    delete pen;
}


kD2DBrush* kD2DBrushAllocator::createResource(const BrushData &brush)
{
    //OutputDebugStringA("BRUSH created\n");
    return new kD2DBrush(brush);
}

void kD2DBrushAllocator::deleteResource(kD2DBrush *brush)
{
    //OutputDebugStringA("BRUSH destroyed\n");
    delete brush;
}

void kD2DBrushAllocator::adjustResource(kD2DBrush *resource, const BrushData &data)
{
    switch (data.p_style) {
        case kBrushStyle::LinearGradient: {
            ID2D1LinearGradientBrush *br = reinterpret_cast<ID2D1LinearGradientBrush*>(resource->getBrush());
            br->SetStartPoint(p2pD2D(data.p_start));
            br->SetEndPoint(p2pD2D(data.p_end));
            break;
        }
    }
}


kD2DFont* kD2DFontAllocator::createResource(const FontData &font)
{
    //OutputDebugStringA("FONT created\n");
    return new kD2DFont(font);
}

void kD2DFontAllocator::deleteResource(kD2DFont *font)
{
    //OutputDebugStringA("FONT destroyed\n");
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
        //p_rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

        //TADMRendererD2D.FClipStack := TListByte.Create();
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
    if (p_rt) {
        p_rt->Release();
    }

    if (p_factory) {
        p_factory->Release();
    }

    if (p_dwrite_factory) {
        p_dwrite_factory->Release();
    }

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
