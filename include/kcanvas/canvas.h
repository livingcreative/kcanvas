/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    canvas.h
        main include file
        exposes mostly all the API types and classes

        general notes on code
            mandatory object parameters passed as is or by reference (&)
            optional object parameters passed by pointer (*) and can be null
            kColor values passed by value (it's only 4 bytes)
*/

#pragma once
#include <vector>
#include "canvastypes.h"     // all basic data types used by canvas and its objects
#include "canvasresources.h" // internal resource object definitions


// this empty defines are only for convenient hints to function parameters
//      (they are undefined at the end of this header)
#define in
#define out


namespace k_canvas
{
    // Basic forward declarations, see what you have to work with
    class kStroke;        // stroke properties, used with pen
    class kGradient;      // gradient object, holds gradient definitions
    class kPen;           // pen object, holds all the data stroke operation needs
    class kBrush;         // brush object, holds all the data fill operation needs
    class kFont;          // font object, holds all font properties
    class kPath;          // path object, holds shape definition
    class kBitmap;        // bitmap object, holds pixel data
    class kTextService;   // text service, provides font info/text measurement interface
    class kCanvas;        // canvas, provides drawing interface
    class kBitmapCanvas;  // canvas for painting into kBitmap
    class kContextCanvas; // canvas for painting into implementation specific context

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

        Unique objects are always hold a unique copy of their resource object.
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
            kStrokeStyle style      - line stroke style
            kLineJoin    join       - defines how multiple lines are connected together
            kCapStyle    startcap   - style of start line cap
            kCapStyle    endcap     - style of end line cap
            kCapStyle    dashcap    - style of caps between line dashes (for non solid styles)
            kScalar      dashoffset - offset of dash pattern along line
            kScalar      strokes[]  - array for custom style pattern definition

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

        // this type of object can be copied and reassigned to other
        kStroke(const kStroke &source);
        kStroke& operator=(const kStroke &source);
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
        kGradient(const kColor start, const kColor end, kExtendType extend = kExtendType::Clamp);
        // create gradient based on gradient stops array
        kGradient(const kGradientStop *stops, size_t count, kExtendType extend = kExtendType::Clamp);
        ~kGradient();

    private:
        // this type of object can NOT be copied and reassigned to other
        kGradient(const kGradient &source);
        kGradient& operator=(const kGradient &source);

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
        kPen(const kColor color, kScalar width = 1, kStrokeStyle style = kStrokeStyle::Solid, const kScalar *strokes = nullptr, size_t count = 0);
        // quick pen creation based on color and kStroke properties object
        //     implicitly creates solid style brush object with color provided
        kPen(const kColor color, kScalar width, const kStroke &stroke);
        // general pen creation based on kBrush object and kStroke properties object
        kPen(const kBrush &brush, kScalar width, const kStroke &stroke);
        ~kPen();

        // this type of object can be copied and reassigned to other
        kPen(const kPen &source);
        kPen& operator=(const kPen &source);
    };


    /*
     -------------------------------------------------------------------------------
     kBrush
     -------------------------------------------------------------------------------
        brush object

        brush object defines how fill painting operation is performed

        kBrushStyle type defines kind of fill operation (set implicitly by constructor)
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
        kBrush(const kColor color);
        // linear gradient brush creation based on kGradient object
        kBrush(const kPoint &start, const kPoint &end, const kGradient &gradient);
        // radial gradient brush creation based on kGradient object
        kBrush(const kPoint &center, const kPoint &offset, const kSize &radius, const kGradient &gradient);
        // bitmap brush creation based on kBitmap object
        kBrush(kExtendType xextend, kExtendType yextend, const kBitmap &bitmap);
        ~kBrush();

        // this type of object can be copied and reassigned to other
        kBrush(const kBrush &source);
        kBrush& operator=(const kBrush &source);
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
        ~kFont();

        // this type of object can be copied and reassigned to other
        kFont(const kFont &source);
        kFont& operator=(const kFont &source);
    };


    /*
     -------------------------------------------------------------------------------
     kPath
     -------------------------------------------------------------------------------
        path object

        path object contains predefined set of geometric primitives which form
        geometric shape of any complexity and could be stroked or filled
        several times

        because path itself is immutable object it can not be instantiated
        directly by constructing it with default constructor, instead special
        helper Constructor class is used to build path. To construct new path
        object Constructor helper object should be created first via Create() call.
        Then path construction commands can be chained together to form a path and
        upon finished Build() command should be called to return new path object

        Example:
            kPath path = kPath::Create()
                .MoveTo(0, 0)
                .LineTo(50, 50)
                .Build();

        Path construction commands
            MoveTo(kPoint point)
                move current point to specified point
                closes currently open figure and starts new one with specified point
                default current point value is (0, 0)

            LineTo(kPoint point)
                add straight line to current figure from current path point to
                specified point

            BezierTo(kPoint p1, kPoint p2, kPoint p3)
                add cubic bezier line to current figure
                p1 is first control point
                p2 is second control point
                p3 is line end point

            ArcTo(kRect rect, kScalar start, kScalar end)
            NOTE: parameters to this command are subject to change
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

            Build
                finishe path construction and returns intermediate object to be
                passed into kPath constructor (or assigned to declared kPath variable)

        Path object commands
            NOTE: this command is subject for removal
            Clear
                reset all path data and transfers path object to construction state

    */
    class kPath
    {
        friend class kCanvas;

    private:
        // Path constructor helper class
        // in order for kPath object to remain immutable this helper object helps
        // to build new path object
        class Constructor
        {
        public:
            Constructor();
            Constructor(Constructor &source);
            ~Constructor();

            // current path construction interface
            // these calls are used to build current path
            Constructor& MoveTo(const kPoint &point);
            Constructor& LineTo(const kPoint &point);
            Constructor& BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3);
            Constructor& ArcTo(const kRect &rect, kScalar start, kScalar end);
            Constructor& PolyLineTo(const kPoint *points, size_t count);
            Constructor& PolyBezierTo(const kPoint *points, size_t count);
            Constructor& Text(const char *text, int count, const kFont &font, kTextOrigin origin = kTextOrigin::Top);
            Constructor& Close();

            // path final construction
            impl::kPathImpl* Build();

        private:
            impl::kPathImpl *p_impl; // path object implementation
        };


    public:
        // Create newly constructed path
        kPath(impl::kPathImpl *result);
        // Create transformed copy of a source path
        kPath(const kPath &source, const kTransform &transform);
        ~kPath();

        // Return new path constructor helper
        static Constructor Create();

    private:
        // there's no sense to construct empty path
        kPath();
        // this type of object can NOT be copied and reassigned to other
        kPath(const kPath &source);
        kPath& operator=(const kPath &source);

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

        void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourcepitch, const void *data);

    private:
        // this type of object can NOT be copied and reassigned to other
        kBitmap(const kBitmap &source);
        kBitmap& operator=(const kBitmap &source);

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
        void GetFontMetrics(const kFont &font, out kFontMetrics &metrics);
        void GetGlyphMetrics(const kFont &font, size_t first, size_t last, out kGlyphMetrics *metrics);
        kSize TextSize(const char *text, int count, const kFont &font, const kTextSizeProperties *properties = nullptr, out kRect *bounds = nullptr);

    private:
        // this type of object can NOT be copied and reassigned to other
        kTextService(const kTextService &source);
        kTextService& operator=(const kTextService &source);

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

        clipping
            drawing can be clipped by any arbitrary shape or mask
            BeginClippedDrawing command marks beginning of clipped drawing
            EndClippedDrawing   command reverts back to previous clipping
            clipping can be nested

        transform
            canvas transform organized as a stack
            SetTransform command changes transform at the top of the stack (it will be last in a hierarchy)
            PushTransform pushes new transform on a stack
            PopTransform pops transform from a stack and reverts to previous one
    */
    class kCanvas : public kTextService
    {
    public:
        // quick draw calls of certain primitive types for non-closed outlines
        void Line(const kPoint &a, const kPoint &b, const kPen &pen);
        void Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPen &pen);
        void Arc(const kRect &rect, kScalar start, kScalar end, const kPen &pen);
        void PolyLine(const kPoint *points, size_t count, const kPen &pen);
        void PolyBezier(const kPoint *points, size_t count, const kPen &pen);

        // quick draw calls of certain primitive types for closed outlines and fills
        //      both kPen and kBrush objects are optional, if both are null - nothing is painted
        void Rectangle(const kRect &rect, const kPen *pen, const kBrush *brush);
        void RoundedRectangle(const kRect &rect, const kSize &round, const kPen *pen, const kBrush *brush);
        void Ellipse(const kRect &rect, const kPen *pen, const kBrush *brush);
        void Polygon(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush);
        void PolygonBezier(const kPoint *points, size_t count, const kPen *pen, const kBrush *brush);

        // object drawing
        void DrawPath(const kPath &path, const kPen *pen, const kBrush *brush);
        void DrawPath(const kPath &path, const kPen *pen, const kBrush *brush, const kPoint &offset);
        void DrawPath(const kPath &path, const kPen *pen, const kBrush *brush, const kTransform &transform);
        void DrawBitmap(const kBitmap &bitmap, const kPoint &origin, kScalar sourcealpha = 1.0f);
        void DrawBitmap(const kBitmap &bitmap, const kPoint &origin, const kPoint &source, const kSize &size, kScalar sourcealpha = 1.0f);
        void DrawBitmap(const kBitmap &bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha = 1.0f);
        void DrawMask(const kBitmap &mask, kBrush &brush, const kPoint &origin);
        void DrawMask(const kBitmap &mask, kBrush &brush, const kPoint &origin, const kPoint &source, const kSize &size);
        void DrawMask(const kBitmap &mask, kBrush &brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize);

        // simple text drawing 
        void Text(const kPoint &p, const char *text, int count, const kFont &font, const kBrush &brush, kTextOrigin origin = kTextOrigin::Top);
        void Text(const kRect &rect, const char *text, int count, const kFont &font, const kBrush &brush, const kTextOutProperties *properties = nullptr);

        // masking & clipping
        void BeginClippedDrawing(const kBitmap &mask, const kTransform &transform = kTransform(), kExtendType xextend = kExtendType::Clamp, kExtendType yextend = kExtendType::Clamp);
        void BeginClippedDrawing(const kPath &clip, const kTransform &transform = kTransform());
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
        kTransform              p_transform;
    };


    /*
     -------------------------------------------------------------------------------
     kCanvasClipper
     -------------------------------------------------------------------------------
        helper object for "safe" clipped drawing within {} block
        automatically calls EndClippedDrawing when object goes out of scope
    */
    class kCanvasClipper
    {
    public:
        kCanvasClipper(kCanvas &canvas, const kBitmap &mask, const kTransform &transform = kTransform(), kExtendType xextend = kExtendType::Clamp, kExtendType yextend = kExtendType::Clamp) :
            p_canvas(canvas)
        {
            p_canvas.BeginClippedDrawing(mask, transform, xextend, yextend);
        }

        kCanvasClipper(kCanvas &canvas, const kPath &clip, const kTransform &transform = kTransform()) :
            p_canvas(canvas)
        {
            p_canvas.BeginClippedDrawing(clip, transform);
        }

        kCanvasClipper(kCanvas &canvas, const kRect &clip) :
            p_canvas(canvas)
        {
            p_canvas.BeginClippedDrawing(clip);
        }

        ~kCanvasClipper()
        {
            p_canvas.EndClippedDrawing();
        }

    private:
        // this type of object can NOT be copied and reassigned to other
        kCanvasClipper(const kCanvasClipper &source);
        kCanvasClipper& operator=(const kCanvasClipper &source);

    private:
        kCanvas &p_canvas;
    };


    /*
     -------------------------------------------------------------------------------
     kCanvasTransformer
     -------------------------------------------------------------------------------
        helper object for "safe" tranformed drawing within {} block
        automatically calls PopTransform when object goes out of scope
    */
    class kCanvasTransformer
    {
    public:
        kCanvasTransformer(kCanvas &canvas, const kTransform &transform) :
            p_canvas(canvas)
        {
            p_canvas.PushTransform(transform);
        }

        ~kCanvasTransformer()
        {
            p_canvas.PopTransform();
        }

    private:
        // this type of object can NOT be copied and reassigned to other
        kCanvasTransformer(const kCanvasTransformer &source);
        kCanvasTransformer& operator=(const kCanvasTransformer &source);

    private:
        kCanvas &p_canvas;
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
        kBitmapCanvas(const kBitmap &target, const kRectInt *rect = nullptr);
        ~kBitmapCanvas();

    private:
        // this type of object can NOT be copied and reassigned to other
        kBitmapCanvas(const kBitmapCanvas &source);
        kBitmapCanvas& operator=(const kBitmapCanvas &source);
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

    private:
        // this type of object can NOT be copied and reassigned to other
        kContextCanvas(const kContextCanvas &source);
        kContextCanvas& operator=(const kContextCanvas &source);
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

    private:
        // this type of object can NOT be copied and reassigned to other
        kPrinterCanvas(const kPrinterCanvas &source);
        kPrinterCanvas& operator=(const kPrinterCanvas &source);
    };

} // namespace k_canvas

#undef in
#undef out
