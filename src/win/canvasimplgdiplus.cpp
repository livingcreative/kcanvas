/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    win/canvasimplgdiplus.cpp
        canvas API GDI+ implementation
*/

#include "canvasimplgdiplus.h"
#include <codecvt>
#include <locale>
#include <algorithm>


using namespace c_util;
using namespace k_canvas;
using namespace impl;
using namespace std;
using namespace Gdiplus;


/*
 -------------------------------------------------------------------------------
 internal utility functions
 -------------------------------------------------------------------------------
*/

static inline PointF p2pGDIP(const kPoint &p)
{
    return PointF(p.x, p.y);
}

static inline RectF r2rGDIP(const kRect &rect)
{
    return RectF(rect.left, rect.top, rect.width(), rect.height());
}

static inline Color c2cGDIP(const kColor &color)
{
    return Color(color.a, color.r, color.g, color.b);
}

static inline INT roundup(REAL value)
{
    INT result = INT(value);
    if ((value - result) > 0.0f) {
        ++result;
    }
    return result;
}


static const LineCap linecapstyles[3] = {
    LineCapFlat,
    LineCapSquare,
    LineCapRound
};

static const DashCap dashcapstyles[3] = {
    DashCapFlat,
    DashCapFlat,
    DashCapRound
};

static const LineJoin gpjoinstyles[3] = {
    LineJoinMiter,
    LineJoinBevel,
    LineJoinRound
};

static const WrapMode wrapmodes[2] = {
    WrapModeClamp, // TODO: not working with linear gradient brush
    WrapModeTile
};


#define pen_not_empty pen && native(pen)[kGDIPlusPen::RESOURCE_PEN]
#define brush_not_empty brush && resourceData<BrushData>(brush).p_style != kBrushStyle::Clear

#define _pen reinterpret_cast<Pen*>(native(pen)[kGDIPlusPen::RESOURCE_PEN])
#define _brush reinterpret_cast<Brush*>(native(brush)[kGDIPlusBrush::RESOURCE_BRUSH])
#define _brushsec reinterpret_cast<Brush*>(native(brush)[kGDIPlusBrush::RESUURCE_SECONDARYBRUSH])
#define _brushclip reinterpret_cast<GraphicsPath*>(native(brush)[kGDIPlusBrush::RESUURCE_SECONDARYBRUSHCLIP])
#define _font reinterpret_cast<Font*>(kCanvasImplGDIPlus::native(font)[kGDIPlusFont::RESOURCE_FONT])
#define _fontgdi reinterpret_cast<HFONT>(kCanvasImplGDIPlus::native(font)[kGDIPlusFont::RESOURCE_GDIFONT])


/*
 -------------------------------------------------------------------------------
 kGradientImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kGradientImplGDIPlus::kGradientImplGDIPlus() :
    p_stops(nullptr),
    p_count(0)
{}

kGradientImplGDIPlus::~kGradientImplGDIPlus()
{
    delete[] p_stops;
}

void kGradientImplGDIPlus::Initialize(const kGradientStop *stops, size_t count, kExtendType extend)
{
    p_stops = new kGradientStop[count];
    p_count = count;
    memcpy(p_stops, stops, count * sizeof(kGradientStop));
    p_extend = extend;
}


/*
 -------------------------------------------------------------------------------
 kPathImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kPathImplGDIPlus::kPathImplGDIPlus() :
    p_path(new GraphicsPath()),
    p_cp()
{}

kPathImplGDIPlus::~kPathImplGDIPlus()
{
    delete p_path;
}

void kPathImplGDIPlus::MoveTo(const kPoint &p)
{
    p_cp = p;
    p_path->StartFigure();
}

void kPathImplGDIPlus::LineTo(const kPoint &p)
{
    p_path->AddLine(p2pGDIP(p_cp), p2pGDIP(p));
    p_cp = p;
}

void kPathImplGDIPlus::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{
    p_path->AddBezier(p2pGDIP(p_cp), p2pGDIP(p1), p2pGDIP(p2), p2pGDIP(p3));
    p_cp = p3;
}

void kPathImplGDIPlus::PolyLineTo(const kPoint *points, size_t count)
{
    PointF pts[2] = {
        p2pGDIP(p_cp),
        p2pGDIP(points[0])
    };
    p_path->AddLines(pts, 2);
    p_path->AddLines(reinterpret_cast<const PointF*>(points + 1), INT(count - 1));
    p_cp = points[count - 1];
}

void kPathImplGDIPlus::PolyBezierTo(const kPoint *points, size_t count)
{
    PointF pts[4] = {
        p2pGDIP(p_cp),
        p2pGDIP(points[0]),
        p2pGDIP(points[1]),
        p2pGDIP(points[2])
    };
    p_path->AddBeziers(pts, 4);
    p_path->AddLines(reinterpret_cast<const PointF*>(points + 3), INT(count - 3));
    p_cp = points[count - 1];
}

void kPathImplGDIPlus::Text(const char *text, int count, const kFontBase *font, kTextOrigin origin)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = count == -1 ?
        convert.from_bytes(text) :
        convert.from_bytes(text, text + count);

    Font *fnt = _font;

    FontFamily ff;
    fnt->GetFamily(&ff);

    RectF rect(p_cp.x, p_cp.y, REAL(MAXINT), REAL(MAXINT));

    p_path->AddString(
        t.c_str(), INT(t.length()),
        &ff, fnt->GetStyle(), fnt->GetSize() * 96.0f / 72.0f,
        rect, StringFormat::GenericTypographic()
   );
}

void kPathImplGDIPlus::Close()
{
    p_path->CloseFigure();
}

void kPathImplGDIPlus::Clear()
{
    p_path->Reset();
}

void kPathImplGDIPlus::Commit()
{
    // nothing to do?
}

void kPathImplGDIPlus::FromPath(const kPathImpl *source, const kTransform &transform)
{
    delete p_path;
    p_path = static_cast<const kPathImplGDIPlus*>(source)->p_path->Clone();

    // Microsoft so cool! This is always be copypasta since Matrix doesn't have copy constructor
    Matrix tfm(
        transform.m00, transform.m01,
        transform.m10, transform.m11,
        transform.m20, transform.m21
    );
    p_path->Transform(&tfm);
}


/*
 -------------------------------------------------------------------------------
 kBitmapImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kBitmapImplGDIPlus::kBitmapImplGDIPlus() :
    p_bitmap(nullptr)
{}

kBitmapImplGDIPlus::~kBitmapImplGDIPlus()
{
    delete p_bitmap;
}

void kBitmapImplGDIPlus::Initialize(size_t width, size_t height, kBitmapFormat format)
{
    p_bitmap = new Bitmap(INT(width), INT(height), PixelFormat32bppPARGB);
}

void kBitmapImplGDIPlus::Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, const void *data)
{
    kRectInt bitmaprect(0, 0, p_bitmap->GetWidth(), p_bitmap->GetHeight());
    kRectInt update = updaterect ? bitmaprect.intersectionwith(*updaterect) : bitmaprect;

    Rect rect(update.left, update.top, update.width(), update.height());
    BitmapData lockdata;
    p_bitmap->LockBits(&rect, ImageLockModeWrite, p_bitmap->GetPixelFormat(), &lockdata);

    switch (sourceformat) {
        case kBitmapFormat::Mask8Bit: {
            const char *src = reinterpret_cast<const char*>(data);
            char *dst = reinterpret_cast<char*>(lockdata.Scan0);

            for (size_t y = 0; y < lockdata.Height; ++y) {
                for (size_t x = 0; x < lockdata.Width; ++x) {
                    dst[0] = dst[1] = dst[2] = dst[3] = *src++;
                    dst += 4;
                }

                dst += lockdata.Stride - lockdata.Width * 4;
                src += sourcepitch - lockdata.Width;
            }

            break;
        }

        case kBitmapFormat::Color32BitAlphaPremultiplied: {
            const char *src = reinterpret_cast<const char*>(data);
            char *dst = reinterpret_cast<char*>(lockdata.Scan0);

            for (size_t y = 0; y < lockdata.Height; ++y) {
                memcpy(dst, src, lockdata.Width * 4);
                dst += lockdata.Stride;
                src += sourcepitch;
            }

            break;
        }
    }

    p_bitmap->UnlockBits(&lockdata);
}


/*
 -------------------------------------------------------------------------------
 kCanvasImplGDIPlus object implementation
 -------------------------------------------------------------------------------
*/

kCanvasImplGDIPlus::kCanvasImplGDIPlus(const CanvasFactory *factory) :
    boundDC(0),
    cacheDC(0),
    prevbitmap(0),
    g(nullptr),
    sf(StringFormat::GenericTypographic()),
    transform()
{}

kCanvasImplGDIPlus::~kCanvasImplGDIPlus()
{
    Unbind();
}

bool kCanvasImplGDIPlus::BindToBitmap(const kBitmapImpl *target, const kRectInt *rect)
{
    if (boundDC) {
        return false;
    }

    boundDC = HDC(1);

    const kBitmapImplGDIPlus *impl = static_cast<const kBitmapImplGDIPlus*>(target);
    boundrect.setLeftTop(0);
    boundrect.right = impl->p_bitmap->GetWidth();
    boundrect.bottom = impl->p_bitmap->GetHeight();
    g = new Graphics(impl->p_bitmap);

    transform.identity();

    SetDefaults();
    UpdateTransform();

    return true;
}

bool kCanvasImplGDIPlus::BindToPrinter(kPrinter printer)
{
    return false;
}

bool kCanvasImplGDIPlus::BindToContext(kContext context, const kRectInt *rect)
{
    if (boundDC) {
        return false;
    }

    boundDC = HDC(context);
    cacheDC = CreateCompatibleDC(boundDC);

    int cx, cy;
    if (rect) {
        boundrect = *rect;
        cx = rect->width();
        cy = rect->height();
    } else {
        _CrtDbgBreak();
    }

    prevbitmap = SelectObject(cacheDC, HGDIOBJ(CreateCompatibleBitmap(boundDC, cx, cy)));

    g = new Graphics(cacheDC);

    transform.identity();

    SetDefaults();
    UpdateTransform();

    return true;
}

bool kCanvasImplGDIPlus::Unbind()
{
    if (boundDC) {
        while (clipStack.size()) {
            EndClippedDrawing();
        }

        if (prevbitmap) {
            BitBlt(
                boundDC, boundrect.left, boundrect.top, boundrect.width(), boundrect.height(),
                cacheDC, 0, 0, SRCCOPY
            );

            DeleteObject(SelectObject(cacheDC, prevbitmap));
            DeleteDC(cacheDC);
            cacheDC = 0;
            prevbitmap = 0;
        }

        delete g;
        boundDC = 0;
        return true;
    }
    return false;
}

void kCanvasImplGDIPlus::Line(const kPoint &a, const kPoint &b, const kPenBase *pen)
{
    if (pen_not_empty) {
        g->DrawLine(_pen, p2pGDIP(a), p2pGDIP(b));
    }
}

void kCanvasImplGDIPlus::Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen)
{
    if (pen_not_empty) {
        g->DrawBezier(_pen, p2pGDIP(p1), p2pGDIP(p2), p2pGDIP(p3), p2pGDIP(p4));
    }
}

void kCanvasImplGDIPlus::PolyLine(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        g->DrawLines(_pen, reinterpret_cast<const PointF*>(points), INT(count));
    }
}

void kCanvasImplGDIPlus::PolyBezier(const kPoint *points, size_t count, const kPenBase *pen)
{
    if (pen_not_empty) {
        g->DrawBeziers(_pen, reinterpret_cast<const PointF*>(points), INT(count));
    }
}

void kCanvasImplGDIPlus::Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        if (Brush *sec = _brushsec) {
            PushClipState(true);
            g->SetClip(_brushclip, CombineModeExclude);
            g->FillRectangle(sec, r2rGDIP(rect));
            PopClipState();
        }
        g->FillRectangle(_brush, r2rGDIP(rect));
    }

    if (pen_not_empty) {
        g->DrawRectangle(_pen, r2rGDIP(rect));
    }
}

void kCanvasImplGDIPlus::RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush)
{
    RectF corner(rect.left, rect.top, round.width * 2.0f, round.height * 2.0f);

    GraphicsPath path;
    path.AddArc(corner, 180, 90);
    corner.X += rect.width() - corner.Width;
    path.AddArc(corner, 270, 90);
    corner.Y += rect.height() - corner.Height;
    path.AddArc(corner, 0, 90);
    corner.X -= rect.width() - corner.Width;
    path.AddArc(corner, 90, 90);
    path.CloseFigure();

    if (brush_not_empty) {
        if (Brush *sec = _brushsec) {
            PushClipState(true);
            g->SetClip(_brushclip, CombineModeExclude);
            g->FillPath(sec, &path);
            PopClipState();
        }
        g->FillPath(_brush, &path);
    }

    if (pen_not_empty) {
        g->DrawPath(_pen, &path);
    }
}

void kCanvasImplGDIPlus::Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        if (Brush *sec = _brushsec) {
            PushClipState(true);
            g->SetClip(_brushclip, CombineModeExclude);
            g->FillEllipse(sec, r2rGDIP(rect));
            PopClipState();
        }
        g->FillEllipse(_brush, r2rGDIP(rect));
    }

    if (pen_not_empty) {
        g->DrawEllipse(_pen, r2rGDIP(rect));
    }
}

void kCanvasImplGDIPlus::Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        if (Brush *sec = _brushsec) {
            PushClipState(true);
            g->SetClip(_brushclip, CombineModeExclude);
            g->FillPolygon(sec, reinterpret_cast<const PointF*>(points), INT(count));
            PopClipState();
        }
        g->FillPolygon(_brush, reinterpret_cast<const PointF*>(points), INT(count));
    }

    if (pen_not_empty) {
        g->DrawPolygon(_pen, reinterpret_cast<const PointF*>(points), INT(count));
    }
}

void kCanvasImplGDIPlus::PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        if (Brush *sec = _brushsec) {
            PushClipState(true);
            g->SetClip(_brushclip, CombineModeExclude);
            g->FillClosedCurve(sec, reinterpret_cast<const PointF*>(points), INT(count));
            PopClipState();
        }
        g->FillClosedCurve(_brush, reinterpret_cast<const PointF*>(points), INT(count));
    }

    if (pen_not_empty) {
        // TODO: this seems not very optimal, can't find the way to properly close
        //       bezier curve without making a path

        GraphicsPath path;
        path.AddBeziers(reinterpret_cast<const PointF*>(points), INT(count) - 2);

        path.AddBezier(
            p2pGDIP(points[count - 3]),
            p2pGDIP(points[count - 2]),
            p2pGDIP(points[count - 1]),
            p2pGDIP(points[0])
        );

        path.CloseFigure();

        g->DrawPath(_pen, &path);
    }
}

void kCanvasImplGDIPlus::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush)
{
    const kPathImplGDIPlus *p = reinterpret_cast<const kPathImplGDIPlus*>(path);
    DrawPathImpl(p->p_path, pen, brush);
}

void kCanvasImplGDIPlus::DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform)
{
    const kPathImplGDIPlus *p = reinterpret_cast<const kPathImplGDIPlus*>(path);

    GraphicsPath *copy = p->p_path->Clone();

    Matrix tfm(
        transform.m00, transform.m01,
        transform.m10, transform.m11,
        transform.m20, transform.m21
    );
    copy->Transform(&tfm);

    DrawPathImpl(copy, pen, brush);

    delete copy;
}

void kCanvasImplGDIPlus::DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, float sourcealpha)
{
    const kBitmapImplGDIPlus *p = reinterpret_cast<const kBitmapImplGDIPlus*>(bitmap);
    if (sourcealpha == 1.0) {
        g->DrawImage(
            p->p_bitmap,
            r2rGDIP(kRect(origin + 0.5f, destsize)),
            source.x, source.y, sourcesize.width, sourcesize.height,
            UnitPixel, nullptr
        );
    } else {
        ImageAttributes attr;
        ColorMatrix matrix = {{ 
            { 1, 0, 0, 0,           0 },
            { 0, 1, 0, 0,           0 },
            { 0, 0, 1, 0,           0 },
            { 0, 0, 0, sourcealpha, 0 },
            { 0, 0, 0, 0,           1 }
        }};
        attr.SetColorMatrix(&matrix);
        g->DrawImage(
            p->p_bitmap,
            r2rGDIP(kRect(origin + 0.5f, destsize)),
            source.x, source.y, sourcesize.width, sourcesize.height,
            UnitPixel, &attr
        );
    }
}

void kCanvasImplGDIPlus::DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize)
{
    INT width = roundup(destsize.width);
    INT height = roundup(destsize.height);

    const kBitmapImplGDIPlus *maskimpl = static_cast<const kBitmapImplGDIPlus*>(mask);

    Bitmap resizedmask(width, height, PixelFormat32bppPARGB);
    {
        Graphics rmg(&resizedmask);
        rmg.SetSmoothingMode(SmoothingModeAntiAlias);
        rmg.SetInterpolationMode(InterpolationModeBilinear);
        rmg.DrawImage(
            maskimpl->p_bitmap,
            r2rGDIP(kRect(kPoint(), destsize)),
            source.x, source.y, sourcesize.width, sourcesize.height,
            UnitPixel, nullptr
         );
    }

    Bitmap maskedimage(width, height, PixelFormat32bppARGB);
    {
        Graphics mig(&maskedimage);
        mig.SetSmoothingMode(SmoothingModeAntiAlias);
        Matrix m;
        m.Translate(-origin.x, -origin.y);
        mig.SetTransform(&m);

        if (Brush *sec = _brushsec) {
            PushClipState(true);
            g->SetClip(_brushclip, CombineModeExclude);
            g->FillRectangle(sec, r2rGDIP(kRect(origin, destsize)));
            PopClipState();
        }
        mig.FillRectangle(_brush, r2rGDIP(kRect(origin, destsize)));
    }

    CopyMask(width, height, &maskedimage, &resizedmask);

    g->DrawImage(&maskedimage, p2pGDIP(origin + 0.5f));
}

void kCanvasImplGDIPlus::GetFontMetrics(const kFontBase *font, kFontMetrics &metrics)
{
    HFONT gdifont = _fontgdi;
    HDC dc = CreateCompatibleDC(0);
    HGDIOBJ pf = SelectObject(dc, gdifont);

    OUTLINETEXTMETRICA tm;
    GetOutlineTextMetricsA(dc, sizeof(tm), &tm);

    // TODO: check for correct units

    metrics.ascent = kScalar(tm.otmTextMetrics.tmAscent);
    metrics.descent = kScalar(tm.otmTextMetrics.tmDescent);
    metrics.height = kScalar(tm.otmTextMetrics.tmHeight);
    metrics.linegap = kScalar(tm.otmTextMetrics.tmExternalLeading);
    // TODO: this two metrics are not supported under GDI/GDI+
    metrics.capheight = 0;
    metrics.xheight = 0;
    metrics.underlinepos = kScalar(tm.otmsUnderscorePosition);
    metrics.underlinewidth = kScalar(tm.otmsUnderscorePosition);
    metrics.strikethroughpos = kScalar(tm.otmsStrikeoutPosition);
    metrics.strikethroughwidth = kScalar(tm.otmsStrikeoutSize);

    SelectObject(dc, pf);
    DeleteDC(dc);
}

void kCanvasImplGDIPlus::GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics)
{
    const size_t BUFFER_LEN = 256;
    ABCFLOAT abc[BUFFER_LEN];

    HFONT gdifont = _fontgdi;
    HDC dc = CreateCompatibleDC(0);
    HGDIOBJ pf = SelectObject(dc, gdifont);

    while (first <= last) {
        size_t count = umin(BUFFER_LEN, last - first + 1);
        GetCharABCWidthsFloatA(dc, UINT(first), UINT(first + count), abc);

        for (size_t n = 0; n < count; ++n) {
            metrics->leftbearing = abc[n].abcfA;
            metrics->advance = abc[n].abcfA + abc[n].abcfB + abc[n].abcfC;
            metrics->rightbearing = -abc[n].abcfC;
            ++metrics;
        }

        first += count;
    }

    SelectObject(dc, pf);
    DeleteDC(dc);
}

kSize kCanvasImplGDIPlus::TextSize(const char *text, size_t count, const kFontBase *font)
{
    bool wasbound = g != nullptr;

    if (!wasbound) {
        boundDC = GetDC(0);
        g = new Graphics(boundDC);
    }

    // GDI+ can't measure single space character, why anybody want to do that, yeah?

    // this is dirty hack, need more robust solution here
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = L"!" + convert.from_bytes(text, text + count) + L"!";
    replace(t.begin(), t.end(), '\n', ' ');

    count += 2;

    RectF rect;
    g->MeasureString(t.c_str(), INT(count), _font, PointF(), sf, &rect);

    RectF rectpad;
    g->MeasureString(L"!!", 2, _font, PointF(), sf, &rectpad);

    if (!wasbound) {
        delete g;
        g = nullptr;
        ReleaseDC(0, boundDC);
        boundDC = 0;
    }

    return kSize(rect.Width - rectpad.Width, rect.Height);
}

void kCanvasImplGDIPlus::Text(const kPoint &p, const char *text, size_t count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = convert.from_bytes(text);
    replace(t.begin(), t.end(), '\n', ' ');

    if (Brush *sec = _brushsec) {
        PushClipState(true);
        g->SetClip(_brushclip, CombineModeExclude);
        g->DrawString(t.c_str(), INT(count), _font, PointF(p.x + 0.5f, p.y + 0.5f), sf, sec);
        PopClipState();
    }
    g->DrawString(t.c_str(), INT(count), _font, PointF(p.x + 0.5f, p.y + 0.5f), sf, _brush);
}

void kCanvasImplGDIPlus::BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend)
{
    Clip &clipstate = PushClipState(false);

    INT width = boundrect.width();
    INT height = boundrect.height();
    clipstate.mask = new Bitmap(width, height, PixelFormat32bppPARGB);
    clipstate.maskedimage = new Bitmap(width, height, PixelFormat32bppARGB);

    // TODO: optimize mask filling and merging back on EndClippedDrawing

    // render mask
    Graphics pmg(clipstate.mask);
    pmg.SetSmoothingMode(SmoothingModeAntiAlias);
    TextureBrush brush(static_cast<const kBitmapImplGDIPlus*>(mask)->p_bitmap);
    Matrix m(
        transform.m00, transform.m01,
        transform.m10, transform.m11,
        transform.m20, transform.m21
    );
    m.Translate(REAL(-boundrect.left), REAL(-boundrect.top), MatrixOrderAppend);
    brush.SetTransform(&m);
    pmg.FillRectangle(&brush, -0.5f, -0.5f, REAL(width), REAL(height));

    // replace current graphics object with masked image graphics
    clipstate.graphics = g;
    clipstate.boundrect = boundrect;
    clipstate.origin.X = REAL(boundrect.left);
    clipstate.origin.Y = REAL(boundrect.top);

    g = new Graphics(clipstate.maskedimage);
    SetDefaults();
    UpdateTransform();
}

void kCanvasImplGDIPlus::BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform)
{
    Clip &clipstate = PushClipState(false);

    const kPathImplGDIPlus *path = static_cast<const kPathImplGDIPlus*>(clip);
    GraphicsPath *copy = path->p_path->Clone();

    kTransform t = this->transform * transform;

    Matrix m(
        t.m00, t.m01,
        t.m10, t.m11,
        t.m20, t.m21
    );
    copy->Transform(&m);

    RectF bounds;
    copy->GetBounds(&bounds);

    bounds.GetLocation(&clipstate.origin);
    clipstate.origin.X -= this->transform.m20;
    clipstate.origin.Y -= this->transform.m21;

    INT width = roundup(bounds.Width);
    INT height = roundup(bounds.Height);
    clipstate.mask = new Bitmap(width, height, PixelFormat32bppPARGB);
    clipstate.maskedimage = new Bitmap(width, height, PixelFormat32bppARGB);

    // render path mask
    Graphics pmg(clipstate.mask);
    pmg.SetSmoothingMode(SmoothingModeAntiAlias);
    Matrix tfm;
    tfm.Translate(-0.5f - bounds.X, -0.5f - bounds.Y);
    pmg.SetTransform(&tfm);
    SolidBrush brush(Color::White);
    pmg.FillPath(&brush, copy);

    // replace current graphics object with masked image graphics
    clipstate.graphics = g;
    clipstate.boundrect = boundrect;

    g = new Graphics(clipstate.maskedimage);
    boundrect = kRectInt(pointT<int>(int(bounds.X), int(bounds.Y)), sizeT<int>(width, height));
    SetDefaults();
    UpdateTransform();

    delete copy;
}

void kCanvasImplGDIPlus::BeginClippedDrawingByRect(const kRect &clip)
{
    PushClipState(true);
    g->SetClip(r2rGDIP(clip), CombineModeIntersect);
}

void kCanvasImplGDIPlus::EndClippedDrawing()
{
    PopClipState();
}

void kCanvasImplGDIPlus::SetTransform(const kTransform &transform)
{
    this->transform = transform;
    UpdateTransform();
}

void kCanvasImplGDIPlus::SetDefaults()
{
    g->SetSmoothingMode(SmoothingModeAntiAlias);
    g->SetTextRenderingHint(TextRenderingHintAntiAlias);
}

void kCanvasImplGDIPlus::UpdateTransform()
{
    Matrix m(
        transform.m00, transform.m01,
        transform.m10, transform.m11,
        transform.m20, transform.m21
    );
    m.Translate(-0.5f - boundrect.left, -0.5f - boundrect.top, MatrixOrderAppend);
    g->SetTransform(&m);
}

kCanvasImplGDIPlus::Clip& kCanvasImplGDIPlus::PushClipState(bool createcontainer)
{
    Clip clip = {};

    if (createcontainer) {
        clip.container = g->BeginContainer();
        SetDefaults();
    }

    clipStack.push_back(clip);

    return clipStack.back();
}

void kCanvasImplGDIPlus::PopClipState()
{
    Clip &clip = clipStack.back();

    if (clip.container) {
        g->EndContainer(clip.container);
    } else if (clip.graphics) {
        delete g;

        CopyMask(boundrect.width(), boundrect.height(), clip.maskedimage, clip.mask);

        g = clip.graphics;
        boundrect = clip.boundrect;
        UpdateTransform();

        g->DrawImage(clip.maskedimage, clip.origin.X + 0.5f, clip.origin.Y + 0.5f);

        delete clip.mask;
        delete clip.maskedimage;
    }

    clipStack.pop_back();
}

void kCanvasImplGDIPlus::CopyMask(int width, int height, Bitmap *maskedimage, Bitmap *mask)
{
    Rect rect(0, 0, width, height);

    BitmapData masklock;
    mask->LockBits(&rect, ImageLockModeRead, PixelFormat32bppPARGB, &masklock);

    BitmapData imagelock;
    maskedimage->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &imagelock);

    const char *src = reinterpret_cast<const char*>(masklock.Scan0);
    char *dst = reinterpret_cast<char*>(imagelock.Scan0);

    for (size_t y = 0; y < masklock.Height; ++y) {
        for (size_t x = 0; x < masklock.Width; ++x) {
            dst[3] = char(uint8_t(dst[3]) * uint8_t(src[3]) * 258 >> 16);
            src += 4;
            dst += 4;
        }

        dst += imagelock.Stride - imagelock.Width * 4;
        src += masklock.Stride - masklock.Width * 4;
    }

    mask->UnlockBits(&masklock);
    maskedimage->UnlockBits(&imagelock);
}

void kCanvasImplGDIPlus::DrawPathImpl(const GraphicsPath *path, const kPenBase *pen, const kBrushBase *brush)
{
    if (brush_not_empty) {
        if (Brush *sec = _brushsec) {
            PushClipState(true);
            g->SetClip(_brushclip, CombineModeExclude);
            g->FillPath(sec, path);
            PopClipState();
        }
        g->FillPath(_brush, path);
    }

    if (pen_not_empty) {
        g->DrawPath(_pen, path);
    }
}


/*
 -------------------------------------------------------------------------------
 kGDIPlusStroke object implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusStroke::kGDIPlusStroke(const StrokeData &stroke) :
    p_stroke(stroke)
{}

kGDIPlusStroke::~kGDIPlusStroke()
{}


/*
 -------------------------------------------------------------------------------
 kGDIPlusPen object implementation
 -------------------------------------------------------------------------------
*/

static const REAL dashpattern[] = { 2.0f, 2.0f };
static const REAL dashdotpattern[] = { 2.0f, 1.0f, 1.0f, 1.0f };
static const REAL dashdotdotpattern[] = { 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

kGDIPlusPen::kGDIPlusPen(const PenData &pen) :
    p_pen(nullptr)
{
    // TODO: fix pen for radial gradient brush

    Brush *brush;
    if (pen.p_brush) {
        void *native[3];
        pen.p_brush->setupNativeResources(native);
        brush = reinterpret_cast<Brush*>(native[0]);
    } else {
        SolidBrush b(Color(0, 0, 0));
        brush = &b;
    }

    p_pen = new Pen(brush, pen.p_width);

    if (pen.p_stroke) {
        kGDIPlusStroke *stroke = static_cast<kGDIPlusStroke*>(pen.p_stroke);
        const StrokeData &data = stroke->getStrokeData();

        // TODO: fix styles & caps

        switch (data.p_style) {
            case kStrokeStyle::Dot:
                p_pen->SetDashStyle(DashStyleDot);
                break;

            case kStrokeStyle::Dash: {
                p_pen->SetDashStyle(DashStyleCustom);
                p_pen->SetDashPattern(dashpattern, 2);
                break;
            }

            case kStrokeStyle::DashDot: {
                p_pen->SetDashStyle(DashStyleCustom);
                p_pen->SetDashPattern(dashdotpattern, 4);
                break;
            }

            case kStrokeStyle::DashDotDot: {
                p_pen->SetDashStyle(DashStyleCustom);
                p_pen->SetDashPattern(dashdotdotpattern, 6);
                break;
            }

            case kStrokeStyle::Custom:
                p_pen->SetDashStyle(DashStyleCustom);
                p_pen->SetDashPattern(data.p_stroke, INT(data.p_count));
                break;
        }

        p_pen->SetLineJoin(gpjoinstyles[size_t(data.p_join)]);
        p_pen->SetLineCap(
            linecapstyles[size_t(data.p_startcap)],
            linecapstyles[size_t(data.p_endcap)],
            dashcapstyles[size_t(data.p_dashcap)]
        );
    }
}

kGDIPlusPen::~kGDIPlusPen()
{
    delete p_pen;
}


/*
 -------------------------------------------------------------------------------
 kGDIPlusBrush object implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusBrush::kGDIPlusBrush(const BrushData &brush) :
    p_brush(nullptr),
    p_secondarybrush(nullptr),
    p_secondaryclip(nullptr)
{
    if (brush.p_style == kBrushStyle::Clear) {
        return;
    }

    // GDI+ doesn't clamp gradients properly, this is gradient extention factor
    const float ext = 4000.0f;

    switch (brush.p_style) {
        case kBrushStyle::Solid:
            p_brush = new SolidBrush(c2cGDIP(brush.p_color));
            break;

        case kBrushStyle::LinearGradient: {
            kGradientImplGDIPlus *g = static_cast<kGradientImplGDIPlus*>(brush.p_gradient);

            // Since authors of GDI+ decided not to clamp gradients to its border colors
            // this behaviour will be emulated with large enough gradient with additional
            // points before first one and after last. Also, GDI+ doesn't accept non 0 value at
            // first gradient stop and non 1 value at last geadient stop.

            bool extend = g->p_extend == kExtendType::Clamp;
            bool extendFirst = g->p_stops[0].position > 0.0f;
            bool extendLast = g->p_stops[g->p_count - 1].position < 1.0f;

            INT count = INT(g->p_count);

            if (extend || extendFirst) {
                ++count;
            }

            if (extend || extendLast) {
                ++count;
            }

            // TODO: change with static cache allocation
            Color *colors = new Color[count];
            REAL *positions = new REAL[count];

            INT index = 0;
            if (extend || extendFirst) {
                colors[index] = c2cGDIP(g->p_stops[0].color);
                positions[index] = 0.0f;
                ++index;
            }

            for (size_t n = 0; n < g->p_count; ++n) {
                colors[index] = c2cGDIP(g->p_stops[n].color);
                positions[index] = g->p_stops[n].position;
                ++index;
            }

            if (extend || extendLast) {
                colors[index] = colors[index - 1];
                positions[index] = 1.0f;
                ++index;
            }

            kPoint start = brush.p_start;
            kPoint end = brush.p_end;

            if (extend || extendFirst || extendLast) {
                kPoint dir = brush.p_end - brush.p_start;
                float len = brush.p_start.distance(brush.p_end);
                float fulllen;
                float posoffset;

                if (extend) {
                    // gradient needs to be extended to emulate clamp to border color
                    // behaviour. Two edge points added to gradient and it's length
                    // is extended, so original stop locations should be remapped
                    fulllen = len + ext;
                    posoffset = ext * 0.5f / fulllen;

                    dir = dir / len;
                    start = start - dir * (ext * 0.5f);
                    end = end + dir * (ext * 0.5f);
                } else {



                }

                // rescale original stop factors
                float posscale = len / fulllen;
                INT first = 0;
                if (extend || extendFirst) {
                    ++first;
                }

                for (size_t n = 0; n < g->p_count; ++n) {
                    positions[first] = positions[first] * posscale + posoffset;
                    ++first;
                }
            }

            LinearGradientBrush *b = new LinearGradientBrush(
                p2pGDIP(start),
                p2pGDIP(end),
                colors[0],
                colors[index - 1]
            );
            b->SetInterpolationColors(colors, positions, count);

            delete[] colors;
            delete[] positions;

            p_brush = b;

            break;
        }

        case kBrushStyle::RadialGradient: {
            kGradientImplGDIPlus *g = static_cast<kGradientImplGDIPlus*>(brush.p_gradient);

            bool needsecondarybrush = brush.p_end != 0.0f;

            kSize exts = needsecondarybrush ?
                brush.p_radius :
                kSize(ext * 0.5f, ext * (brush.p_radius.width / brush.p_radius.height) * 0.5f);

            // TODO: check for requested ellipse size is larger than extended boundary
            GraphicsPath path;
            RectF e(
                brush.p_start.x - exts.width, brush.p_start.y - exts.height,
                exts.width * 2.0f, exts.height * 2.0f
            );
            path.AddEllipse(e);

            INT count = INT(g->p_count) + 1;
            // TODO: change with static cache allocation
            Color *colors = new Color[count];
            REAL *positions = new REAL[count];

            INT index = 0;

            colors[index] = c2cGDIP(g->p_stops[g->p_count - 1].color);
            positions[index] = 0.0f;
            ++index;

            float len = brush.p_radius.width;
            float fulllen = exts.width;
            float posscale = len / fulllen;

            for (size_t n = 0; n < g->p_count; ++n) {
                colors[index] = c2cGDIP(g->p_stops[g->p_count - n - 1].color);
                positions[index] = 1.0f - g->p_stops[g->p_count - n - 1].position * posscale;
                ++index;
            }

            PathGradientBrush *b = new PathGradientBrush(&path);
            b->SetCenterPoint(p2pGDIP(brush.p_start + brush.p_end));
            b->SetInterpolationColors(colors, positions, count);

            p_brush = b;

            if (needsecondarybrush) {
                p_secondarybrush = new SolidBrush(colors[0]);
                p_secondaryclip = new GraphicsPath();
                p_secondaryclip->AddEllipse(RectF(
                    brush.p_start.x - brush.p_radius.width,
                    brush.p_start.y - brush.p_radius.height,
                    brush.p_radius.width * 2.0f,
                    brush.p_radius.height * 2.0f
                ));
            }

            delete[] colors;
            delete[] positions;

            break;
        }

        case kBrushStyle::Bitmap: {
            TextureBrush *b = new TextureBrush(
                reinterpret_cast<kBitmapImplGDIPlus*>(brush.p_bitmap)->p_bitmap
                // TODO: wrap mode, offsets
            );
            Matrix m;
            m.Translate(0.5f, 0.5f);
            b->SetTransform(&m);

            p_brush = b;

            break;
        }
    }
}

kGDIPlusBrush::~kGDIPlusBrush()
{
    delete p_brush;
    delete p_secondarybrush;
    delete p_secondaryclip;
}


/*
 -------------------------------------------------------------------------------
 kGDIPlusFont object implementation
 -------------------------------------------------------------------------------
*/

kGDIPlusFont::kGDIPlusFont(const FontData &font) :
    p_font(nullptr),
    p_gdifont(0)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    wstring t = convert.from_bytes(font.p_facename);

    UINT style = 0;
    style += font.p_style & kFontStyle::Bold ? FontStyleBold : 0;
    style += font.p_style & kFontStyle::Italic ? FontStyleItalic : 0;
    style += font.p_style & kFontStyle::Underline ? FontStyleUnderline : 0;
    style += font.p_style & kFontStyle::Strikethrough ? FontStyleStrikeout : 0;
    p_font = new Font(t.c_str(), font.p_size, style);

    Bitmap b(1, 1);
    Graphics g(&b);

    LOGFONTA logfont;
    p_font->GetLogFontA(&g, &logfont);
    p_gdifont = CreateFontIndirectA(&logfont);
}

kGDIPlusFont::~kGDIPlusFont()
{
    delete p_font;
    DeleteObject(p_gdifont);
}


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
{
    gdiplusToken = 0;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

CanvasFactoryGDIPlus::~CanvasFactoryGDIPlus()
{
    GdiplusShutdown(gdiplusToken);
}

bool CanvasFactoryGDIPlus::initialized()
{
    return gdiplusToken != 0;
}
