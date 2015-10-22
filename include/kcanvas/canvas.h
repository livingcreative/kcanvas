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
#include "canvastypes.h"     // all basic data types used by canvas and its objects
#include "canvasresources.h" // internal resource object definitions


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
        similar parameters. If you create two instances of kPen object with
        fully identical properties, both objects get same single implementation
        object.

        Unique objects are always hold a unique copy of their resurce object.
        The resource object only shared between other objects which use it
        as a source of data or properties.

        All objects are immutable. Properties of the object can't be changed
        after object has been created. This rule doesn't apply to object
        data (e.g. kBitmap image data can be changed, but size and format can not).

        Shareable objects
            kStroke
            kPen
            kBrush
            kFont

        Unique objects
            kGradient
            kPath
            kBitmap
    */


    /*
     -------------------------------------------------------------------------------
     kStroke
     -------------------------------------------------------------------------------
        stroke properties object

        stroke properties object defines properties which affect
        how stroke painting operation should be done

        these properties are:
            kStroke   style      - line style
            kLineJoin join       - how multiples lines are connected
            kCapStyle startcap   - style of start line cap
            kCapStyle endcap     - style of end line cap
            kCapStyle dashcap    - style of caps between line dashes (for non solid styles)
            kScalar   dashoffset - offset of dash pattern along line
            kScalar   strokes[]  - array for custom dash pattern definition

        Strokes array passed to constructor is copied inside object and can be
        safely deleted after object construction
    */
    class kStroke : public impl::kStrokeBase
    {
    public:
        // create simple stroke with predefined or custom stroke style
        // and other parameters set to defaults
        kStroke(
            kStrokeStyle style,
            kScalar dashoffset = 0,
            const kScalar *strokes = nullptr, size_t count = 0
        );
        // create stroke with full parameters
        kStroke(
            kStrokeStyle style,
            kLineJoin join,
            kCapStyle startcap = kCapStyle::Flat,
            kCapStyle endcap = kCapStyle::Flat,
            kCapStyle dashcap = kCapStyle::Flat,
            kScalar dashoffset = 0,
            const kScalar *strokes = nullptr, size_t count = 0
        );
        ~kStroke();
    };


    /*
     -------------------------------------------------------------------------------
     kGradient
     -------------------------------------------------------------------------------
        gradient definition object

        gradient object defines colors for gradient and other properties

        properties of gradient object:
            kExtendType   extend  - defines how gradient is extended outside of its bounds
            kGradientStop stops[] - array of gradient stops

        kGradientStop structure defines single gradient color at certain position
        positions should be in range from 0 to 1

        Gradient stops array passed to constructor is copied inside object and can be
        safely deleted after object construction
    */
    class kGradient
    {
        // kBrush object can reference kGradient object for gradient brush
        // definitions and needs access to internal gradient implementation object
        friend class kBrush;

    public:
        // create simple two color gradient with start color at 0 and end color at 1
        kGradient(const kColor &start, const kColor &end, kExtendType extend = kExtendType::Clamp);
        // create gradient based on gradient stops array
        kGradient(const kGradientStop *stops, size_t count, kExtendType extend = kExtendType::Clamp);
        ~kGradient();

    private:
        // copy constructor (gradient can't be copied)
        kGradient(const kGradient &source) {}

    protected:
        impl::kGradientImpl *p_impl; // gradient object implementation
    };


    /*
     -------------------------------------------------------------------------------
     kPen
     -------------------------------------------------------------------------------
        pen object

        pen object completley defines stroke painting operation
        it incapsulates kStroke and kBrush objects for stroke painting

        these objects can be constructed independently or implicitly by kPen
        constructor. Referenced objects can be safely deleted after pen
        construction.

        pen properties are:
            kScalar width  - defines line width for stroke operation
            kStroke stroke - reference to kStroke object which defines stroke style
            kBrush  brush  - reference to kBrush object which defines stroke fill
    */
    class kPen : public impl::kPenBase
    {
    public:
        // quick Clear pen creation
        kPen();
        // quick pen creation based on color, width and stroke style
        //     implicitly creates solid style brush object with color provided
        //     other stroke parameters set to default values (see kStroke default values)
        kPen(const kColor &color, kScalar width = 1, kStrokeStyle style = kStrokeStyle::Solid, const kScalar *strokes = nullptr, size_t count = 0);
        // quick pen creation based on color and kStroke properties object
        //     implicitly creates solid style brush object with color provided
        kPen(const kColor &color, kScalar width, const kStroke *stroke);
        // general pen creation based on kBrush object and kStroke properties object
        kPen(const kBrush &brush, kScalar width, const kStroke *stroke);
        // copy constructor
        kPen(const kPen &source);
        ~kPen();
    };


    /*
     -------------------------------------------------------------------------------
     kBrush
     -------------------------------------------------------------------------------
        brush object

        brush object defines how fill painting operation is performed

        kBrushStyle type defines kind of fill operation
            Solid          - single solid color fill
            LinearGradient - fill with linear gradient sourced from kGradient object
            RadialGradient - fill with radial gradient source from kGradient object
            Bitmap         - fill with bitmap pattern (texture) sourced from kBitmap object

        properties for solid fill
            kColor color - single color for solid fill

        properties for linear gradient fill
            kPoint start - gradient line starting point (where color from 0 stop position sampled)
            kPoint end   - gradient line ending point (where color from 1 stop position sampled)

        properties for radial gradient fill
            kPoint center - radial gradient center point (where color from 1 stop position sampled)
                this point defines geometric center of ellipse which contains gradient
            kPoint offset - offset for actual gradient center
            kSize  radius - defines radius of ellipse which contains gradient

        properties for bitmap fill
            kExtendType xextend - defines how pattern extended along x axis
            kExtendType yextend - defines how pattern extended along y axis
            kBitmap     bitmap  - source of bitmap data which is used for filling

        all referenced objects can be safely deleted after pen
        construction.
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
        // copy constructor
        kBrush(const kBrush &source);
        ~kBrush();
    };


    /*
     -------------------------------------------------------------------------------
     kFont
     -------------------------------------------------------------------------------
        font object

        font object defines font properties for text rendering

        these properties are:
            char     facename[] - name of the font
                implementation might use system's font mapper
            kScalar  size       - font size int points (1/72 inch)
            uint32_t style      - combination of kFontStyle flags
    */
    class kFont : public impl::kFontBase
    {
    public:
        // create system's default font
        kFont();
        // create font with desired properties
        kFont(const char *facename, kScalar size, uint32_t style = 0);
        // copy constructor
        //kFont(const kFont &source);
        ~kFont();
    };


    /*
     -------------------------------------------------------------------------------
     kPath
     -------------------------------------------------------------------------------
        path object

        path object contains predefined set of geometric primitives which form
        geometric shape of any complexity and could be stroked or filled
        several times

        after creation with default constructor path object remains in
        construction state until Commit() command is called. In construction
        state path construction commands can be issued for path definition. When
        desired path is constructed one should issue Commit() command. After
        commitment path object is transfered to ready state and can't be modified.

        Path construction commands
            MoveTo(kPoint point)
                move current point to specified point
                closes currently open figure and starts new one with specified point
                default current point value is (0, 0)

            LineTo(kPoint point)
                add straight line to current figure from current path point to
                specified point

            BezierTo(kPoint p1, kPoint p2, kPoint p3)
                add qubic bezier line to current figure
                p1 is first control point
                p2 is second control point
                p3 is line end point

            ArcTo(kRect rect, kScalar start, kScalar end)
                add elliptic arc to current path
                rect is ellipse bounding rectangle
                start is start arc angle
                end is end arc angle
                angles are counted clockwise with 0 matching to Y-up axis

            PolyLineTo(kPoint points[], size_t count)
                add multiple connected line segments to current path
                points - array of points
                count - point count (matches total line segments being added)

            PolyBezierTo(kPoint points[], size_t count);
                add multiple connected bezier segments
                points - array of points
                count - point count, should be a multiple of 3

            Text(char text[], kFont font)
                add straight line of text as set of glyph contours
                current open figure is closed before adding glyph contours

            Close
                close current open figure

        Path object commands
            Clear
                reset all path data and transfers path object to construction state

            Commit
                finishe path construction and transfers path object to ready state
    */
    class kPath
    {
        friend class kCanvas;

    public:
        // Empty path constructor, created path should be constructed
        // with path construction commands and commited before use
        kPath();
        // Create transformed copy of a source path. Path created in
        // commited state
        kPath(const kPath &source, const kTransform &transform);
        ~kPath();

        // current path construction interface
        // these calls are used to build current path
        void MoveTo(const kPoint &point);
        void LineTo(const kPoint &point);
        void BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3);
        void ArcTo(const kRect &rect, kScalar start, kScalar end);
        void PolyLineTo(const kPoint *points, size_t count);
        void PolyBezierTo(const kPoint *points, size_t count);
        void Text(const char *text, int count, const kFont *font, kTextOrigin origin = kTextOrigin::Top);
        void Close();

        // path clear & commit
        void Clear();
        void Commit();

    private:
        // copy constructor (path can not be copied)
        kPath(const kPath &source) {}

    protected:
        impl::kPathImpl *p_impl; // path object implementation
    };


    /*
     -------------------------------------------------------------------------------
     kBitmap
     -------------------------------------------------------------------------------
        bitmap object

        holds rectangular pixel data

        bitmap properties:
            size_t        width  - width of the bitmap image
            size_t        height - height of the bitmap image
            kBitmapFormat format - pixel format

        Methods
            Update(optional kRect updaterect, kBitmapFormat source format, size_t sourcepitch, data)
                updae whole or part of bitmap's pixel data
                updateerect - optional destination rectangle
                    if not provided the whole bitmap is updated
                    always clipped to bitmap's dimensions
                format      - format of source pixel data (now should match bitmap format)
                sourcepitch - byte size of source data row
                void        - source data pointer

                source pixels are not stretched and copied as is into desired place
    */
    class kBitmap
    {
        friend class kCanvas;
        friend class kBitmapCanvas;
        friend class kBrush;

    public:
        kBitmap(size_t width, size_t height, kBitmapFormat format);
        ~kBitmap();

        size_t width() const { return p_width; }
        size_t height() const { return p_height; }
        kSize size() const { return kSize(kScalar(p_width), kScalar(p_height)); }
        kBitmapFormat format() const { return p_format; }

        void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, void *data);

    private:
        // copy constructor (bitmap can not be copied)
        kBitmap(const kBitmap &source) {}

    protected:
        impl::kBitmapImpl *p_impl;
        size_t             p_width;
        size_t             p_height;
        kBitmapFormat      p_format;
    };


    /*
     -------------------------------------------------------------------------------
     kTextService
     -------------------------------------------------------------------------------
        text service object
        
        defines interface for retrieving font metrics and text measuring

        text measurement

        GetFontMetrics       - retrieve basic font metrics
            GetGlyphMetrics      - retrieve array of glyph metrics for glyph range
            TextSize             - single or multiline text measurement
    */
    class kTextService
    {
    public:
        kTextService();
        ~kTextService();

        // simple text measuring and drawing 
        void GetFontMetrics(const kFont *font, kFontMetrics &metrics);
        void GetGlyphMetrics(const kFont *font, size_t first, size_t last, kGlyphMetrics *metrics);
        kSize TextSize(const char *text, int count, const kFont *font, const kTextSizeProperties *properties = nullptr, kRect *bounds = nullptr);

    protected:
        impl::kCanvasImpl *p_impl;
    };


    /*
     -------------------------------------------------------------------------------
     kCanvas
     -------------------------------------------------------------------------------
        canvas object

        defines basic canvas interface

        canvas doesn't have notion of current position point
        all primitive drawing functions take all the points they need

        there are only few basic primitve drawing functions
        complex shapes can be defined by path object

        non-closed outlines
            Line       - draw straight line from a to b
            Bezier     - draw qubic bezier line
                p1 - starting point, p2 - first control point,
                p3 - second control point, p4 - ending point
            Arc        - draw arc inscribed in rectangle
            PolyLine   - draw multiple lines connected with each other
            PolyBezier - draw multiple bezier lines connected with each other

            for Poly* commands - first point is a starting point
            for PolyLine each point (except first one) defines line segment from
            previous point
            for PolyBezier every three points define two control points and
            ending point for segment (total point count should be multiple of 3 + 1)

        closed outlines
            Rectangle        - draw rectangle
            RoundedRectangle - draw rounded rectangle
                round parameter represents radius for corner rounding
            Ellipse          - draw ellipse
            Polygon          - draw polygon
                last point is always connected with the first one
            PolygonBezier    - draw polygon with bezier segments
                similar to PolyLine every bezier segment is defined by three points
                except the last one - it should contain only two control points and
                the last point is automatically equals to first point

        object drawing
            DrawPath command draws kPath object
            DrawBitmap commands are used to draw kBitmap objects

        text drawing
            canvas provides only basic text drawing capabilities
            to draw complex formatted text one could use own implementation above
            basic text drawing functions

            Text(kPoint p ...)   - render single line of text with specified font and brush
            Text(kRect rect ...) - render bounded multiline text
                with optional alignment and clipping
    */
    class kCanvas : public kTextService
    {
    public:
        // quick draw calls of certain primitive types for non-closed outlines
        void Line(const kPoint &a, const kPoint &b, const kPen *pen);
        void Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPen *pen);
        void Arc(const kRect &rect, kScalar start, kScalar end, const kPen *pen);
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
        void DrawPath(const kPath *path, const kPen *pen, const kBrush *brush, const kPoint &offset);
        void DrawPath(const kPath *path, const kPen *pen, const kBrush *brush, const kTransform &transform);
        void DrawBitmap(const kBitmap *bitmap, const kPoint &origin, kScalar sourcealpha = 1.0f);
        void DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kPoint &source, const kSize &size, kScalar sourcealpha = 1.0f);
        void DrawBitmap(const kBitmap *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha = 1.0f);
        void DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin);
        void DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin, const kPoint &source, const kSize &size);
        void DrawMask(const kBitmap *mask, kBrush *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize);

        // simple text drawing 
        void Text(const kPoint &p, const char *text, int count, const kFont *font, const kBrush *brush, kTextOrigin origin = kTextOrigin::Top);
        void Text(const kRect &rect, const char *text, int count, const kFont *font, const kBrush *brush, const kTextOutProperties *properties = nullptr);

        // masking & clipping
        void BeginClippedDrawing(const kBitmap *mask, const kTransform &transform = kTransform(), kExtendType xextend = kExtendType::Clamp, kExtendType yextend = kExtendType::Clamp);
        void BeginClippedDrawing(const kPath *clip, const kTransform &transform = kTransform());
        void BeginClippedDrawing(const kRect &clip);
        void EndClippedDrawing();

        // transform
        void SetTransform(const kTransform &transform);
        void PushTransform(const kTransform &transform);
        void PopTransform();

        // global initialization & finalization
        static bool Initialize(Impl implementation = IMPL_NONE);
        static bool Shutdown();

    protected:
        // Default canvas instantiation is not allowed
        kCanvas() {}
        ~kCanvas() {}

        static inline void needResources(const kPen *pen, const kBrush *brush);

    protected:
        std::vector<kTransform> p_transform_stack;
    };


    /*
     -------------------------------------------------------------------------------
     kBitmapCanvas
     -------------------------------------------------------------------------------
        canvas object for painting to kBitmap object
    */
    class kBitmapCanvas : public kCanvas
    {
    public:
        kBitmapCanvas(const kBitmap *target, const kRectInt *rect = nullptr);
        ~kBitmapCanvas();
    };


    /*
     -------------------------------------------------------------------------------
     kContextCanvas
     -------------------------------------------------------------------------------
        canvas object for painting to platform context (typically window)
    */
    class kContextCanvas : public kCanvas
    {
    public:
        kContextCanvas(kContext context, const kRectInt *rect = nullptr);
        ~kContextCanvas();
    };


    /*
     -------------------------------------------------------------------------------
     kPrinterCanvas
     -------------------------------------------------------------------------------
        canvas object for printing
    */
    class kPrinterCanvas : public kCanvas
    {
    public:
        kPrinterCanvas(kPrinter printer);
        ~kPrinterCanvas();
    };

} // namespace k_canvas
