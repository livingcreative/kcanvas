/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvas.h
        main include file
        exposes mostly all the API types and classes
*/

#pragma once
#include "canvastypes.h"
#include "canvasresources.h"


namespace k_canvas
{
    // Basic forward declarations, see what you have to work with
    class kStroke;   // stroke properties, used with pen
    class kGradient; // gradient object, holds gradient definitions
    class kPen;      // pen object, holds all the data stroke operation needs
    class kBrush;    // brush object, holds all the data fill operation needs
    class kFont;     // font object, holds all font properties
    class kPath;     // path object, holds shape definition
    class kBitmap;   // bitmap object, holds pixel data
    class kCanvas;   // canvas, provides drawing interface

    namespace impl
    {
        // Implementation forward declarations
        class kGradientImpl;
        class kPathImpl;
        class kBitmapImpl;
        class kCanvasImpl;
    }


    /*
        Resource objects

        All resource objects are reference holders for actual
        internal implementation object. Implementation objects are
        reference counted objects. This behaviour hides resource
        management and any API object could be safely deleted even if
        it was used as source for another object.

        There are two groups of resource objects - shareable and unique.

        Shareable objects can be shared across many API objects with
        similar parameters.

        Unique objects are always hold a unique copy of their resurce object.
        The resource object only shared between other objects which use it
        as a source of data or properties.
    */


    /*
     -------------------------------------------------------------------------------
     kStroke
     -------------------------------------------------------------------------------
        stroke properties object
    */
    class kStroke : public impl::kStrokeBase
    {
    public:
        // create simple stroke with predefined or custom stroke style
        // and ther parameters set to defaults
        kStroke(
            kStrokeStyle style,
            Scalar dashoffset = 0,
            const Scalar *strokes = nullptr, size_t count = 0
        );
        // create stroke with full parameters
        kStroke(
            kStrokeStyle style,
            kLineJoin join,
            kCapStyle startcap = kCapStyle::Flat,
            kCapStyle endcap = kCapStyle::Flat,
            kCapStyle dashcap = kCapStyle::Flat,
            Scalar dashoffset = 0,
            const Scalar *strokes = nullptr, size_t count = 0
        );
        ~kStroke();
    };


    /*
     -------------------------------------------------------------------------------
     kGradient
     -------------------------------------------------------------------------------
        gradient definition object
    */
    class kGradient
    {
        friend class kBrush;

    public:
        // create simple two color gradient
        kGradient(const kColor &start, const kColor &end, kExtendType extend = kExtendType::Clamp);
        // create gradient based on gradient stops array
        kGradient(const kGradientStop *stops, size_t count, kExtendType extend = kExtendType::Clamp);
        ~kGradient();

    protected:
        impl::kGradientImpl *p_impl;
    };


    /*
     -------------------------------------------------------------------------------
     kPen
     -------------------------------------------------------------------------------
        pen object
    */
    class kPen : public impl::kPenBase
    {
    public:
        // quick Clear pen creation
        kPen();
        // quick pen creation based on color, width and stroke style
        //     other stroke parameters set to default values (see kStroke default values)
        kPen(const kColor &color, Scalar width = 1, kStrokeStyle style = kStrokeStyle::Solid, const Scalar *strokes = nullptr, size_t count = 0);
        // quick pen creation based on color and kStroke properties object
        kPen(const kColor &color, Scalar width, const kStroke *stroke);
        // general pen creation based on kBrush object and kStroke properties object
        kPen(const kBrush &brush, Scalar width, const kStroke *stroke);
        ~kPen();
    };


    /*
     -------------------------------------------------------------------------------
     kBrush
     -------------------------------------------------------------------------------
        brush object
    */
    class kBrush : public impl::kBrushBase
    {
    public:
        // quick Clear brush creation
        kBrush();
        // solid brush creation based on color
        kBrush(const kColor &color);
        // linear gradient brush creation based on kGradient object
        kBrush(const kPoint &start, const kPoint &end, const kGradient &gradient);
        // radial gradient brush creation based on kGradient object
        kBrush(const kPoint &center, const kPoint &offset, const kSize &radius, const kGradient &gradient);
        // bitmap brush creation based on kBitmap object
        kBrush(kExtendType xextend, kExtendType yextend, const kBitmap *bitmap);
        ~kBrush();
    };


    /*
     -------------------------------------------------------------------------------
     kFont
     -------------------------------------------------------------------------------
        font object
    */
    class kFont : public impl::kFontBase
    {
    public:
        // create system's default font
        kFont();
        // create font with desired properties
        kFont(const char *facename, Scalar size, uint32_t style = 0);
        ~kFont();
    };


    /*
     -------------------------------------------------------------------------------
     kPath
     -------------------------------------------------------------------------------
        path object
    */
    class kPath
    {
        friend class kCanvas;

    public:
        kPath();
        ~kPath();

        // current path drawing interface
        // these calls are used to build current path
        void MoveTo(const kPoint &point);
        void LineTo(const kPoint &point);
        void BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3);
        void ArcTo(const kRect &rect, Scalar start, Scalar end);
        void PolyLineTo(const kPoint *points, size_t count);
        void PolyBezierTo(const kPoint *points, size_t count);
        void Text(const char *text, const kFont *font);
        void Close();

        // path clear & commit
        void Clear();
        void Commit();

    protected:
        impl::kPathImpl *p_impl;
    };


    /*
     -------------------------------------------------------------------------------
     kBitmap
     -------------------------------------------------------------------------------
        bitmap object
    */
    class kBitmap
    {
        friend class kCanvas;
        friend class kBrush;

    public:
        kBitmap(size_t width, size_t height, kBitmapFormat format);
        ~kBitmap();

        size_t width() const { return p_width; }
        size_t height() const { return p_height; }
        kBitmapFormat format() const { return p_format; }

        void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data);

    protected:
        impl::kBitmapImpl *p_impl;
        size_t             p_width;
        size_t             p_height;
        kBitmapFormat      p_format;
    };


    /*
     -------------------------------------------------------------------------------
     kCanvas
     -------------------------------------------------------------------------------
        canvas object
    */
    class kCanvas
    {
    public:
        // quick draw calls of certain primitive types for non-closed outlines
        void Line(const kPoint &a, const kPoint &b, const kPen *pen);
        void Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPen *pen);
        void Arc(const kRect &rect, Scalar start, Scalar end, const kPen *pen);
        void PolyLine(const kPoint *points, size_t count, const kPen *pen);
        void PolyBezier(const kPoint *points, size_t count, const kPen *pen);

        // quick draw calls of certain primitive types for closed outlines and fills
        void Rectangle(const kRect &rect, const kPen *pen, const kBrush *brush);
        void RoundedRectangle(const kRect &rect, const kSize &round, const kPen *pen, const kBrush *brush);
        void Ellipse(const kRect &rect, const kPen *pen, const kBrush *brush);
        void Polygon(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush);
        void PolygonBezier(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush);

        // object drawing
        void DrawPath(const kPath *path, const kPen *pen, const kBrush *brush);
        void DrawBitmap(const kBitmap *bitmap, const kPoint &origin, Scalar sourcealpha = 1.0f);
        void DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kPoint &source, const kSize &size, Scalar sourcealpha = 1.0f);
        void DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, Scalar sourcealpha = 1.0f);

        // simple text measuring and drawing 
        kSize TextSize(const char *text, int count, const kFont *font, const kSize *bounds = nullptr, const kTextSizeProperties *properties = nullptr);
        void TextOut(const kPoint &p, const char *text, int count, const kFont *font, const kBrush *brush);
        void TextOut(const kRect &rect, const char *text, int count, const kFont *font, const kBrush *brush, const kTextOutProperties *properties = nullptr);

        // masking

        // clipping

        // global initialization & finalization
        static bool Initialize(Impl implementation = IMPL_NONE);
        static bool Shutdown();

    protected:
        // Canvas interface by itself doesn't allow its instantiation
        // to create canvas object use more specific class
        // (kBitmapCanvas, kContextCanvas, kPrinterCanvas)
        kCanvas();
        ~kCanvas();

        static inline void needResources(const kPen *pen, const kBrush *brush);

    protected:
        impl::kCanvasImpl *p_impl;
    };



    class kBitmapCanvas : public kCanvas
    {
    public:
        kBitmapCanvas(const kBitmap *target);
        ~kBitmapCanvas();
    };


    class kContextCanvas : public kCanvas
    {
    public:
        kContextCanvas(kContext context);
        ~kContextCanvas();
    };


    class kPrinterCanvas : public kCanvas
    {
    public:
        kPrinterCanvas(kPrinter printer);
        ~kPrinterCanvas();
    };

} // namespace k_canvas
