/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    win/canvasimpld2d.h
        canvas API Direct2D implementation header
*/

#pragma once
#include "../canvasimpl.h"
#include <d2d1.h>
#include <dwrite.h>


namespace k_canvas
{
    namespace impl
    {
        /*
         -------------------------------------------------------------------------------
         kGradientImplD2D
         -------------------------------------------------------------------------------
            gradient resource object Direct2D implementation
        */
        class kGradientImplD2D : public kGradientImpl
        {
            friend class kD2DBrush;

        public:
            kGradientImplD2D();
            ~kGradientImplD2D() override;

            void Initialize(const kGradientStop *stops, size_t count, kExtendType extend) override;

        protected:
            ID2D1GradientStopCollection *p_gradient;
        };


        /*
         -------------------------------------------------------------------------------
         kPathImplD2D
         -------------------------------------------------------------------------------
            path resource object Direct2D implementation
        */
        class kPathImplD2D : public kPathImpl
        {
            friend class kCanvasImplD2D;

        public:
            kPathImplD2D();
            ~kPathImplD2D() override;

            void MoveTo(const kPoint &p) override;
            void LineTo(const kPoint &p) override;
            void BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3) override;
            void PolyLineTo(const kPoint *points, size_t count) override;
            void PolyBezierTo(const kPoint *points, size_t count) override;
            void Text(const char *text, int count, const kFontBase *font, kTextOrigin origin) override;
            void Close() override;
            
            void Clear() override;
            void Commit() override;

            void FromPath(const kPathImpl *source, const kTransform &transform) override;

            ID2D1Geometry* MakeTransformedPath(const D2D1_MATRIX_3X2_F &transform) const;

        private:
            void OpenSink();
            void CloseSink();
            void OpenFigure();
            void CloseFigure(bool opened);

        private:
            D2D1_POINT_2F      p_cp;
            ID2D1Geometry     *p_path;
            ID2D1GeometrySink *p_sink;
            bool               p_opened;
        };


        /*
         -------------------------------------------------------------------------------
         kBitmapImplD2D
         -------------------------------------------------------------------------------
            bitmap resource object Direct2D implementation
        */
        class kBitmapImplD2D : public kBitmapImpl
        {
            friend class kCanvasImplD2D;
            friend class kD2DBrush;

        public:
            kBitmapImplD2D();
            ~kBitmapImplD2D() override;

            void Initialize(size_t width, size_t height, kBitmapFormat format) override;
            void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourceputch, void *data) override;

        private:
            ID2D1Bitmap *p_bitmap;
        };


        /*
         -------------------------------------------------------------------------------
         kCanvasImplD2D
         -------------------------------------------------------------------------------
            canvas Direct2D implementation
        */
        class kCanvasImplD2D : public kCanvasImpl
        {
            friend class kPathImplD2D;
            friend class kD2DPenAllocator;
            friend class kD2DBrushAllocator;
            friend class kD2DFontAllocator;

        public:
            kCanvasImplD2D(const CanvasFactory *factory);
            ~kCanvasImplD2D() override;

            bool BindToBitmap(const kBitmapImpl *target) override;
            bool BindToContext(kContext context) override;
            bool BindToPrinter(kPrinter printer) override;
            bool Unbind() override;

            void Line(const kPoint &a, const kPoint &b, const kPenBase *pen) override;
            void Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen) override;
            void PolyLine(const kPoint *points, size_t count, const kPenBase *pen) override;
            void PolyBezier(const kPoint *points, size_t count, const kPenBase *pen) override;

            void Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush) override;
            void RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush) override;
            void Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush) override;
            void Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush) override;
            void PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush) override;

            void DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush) override;
            void DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform) override;
            void DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha) override;
            void DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize) override;

            void GetFontMetrics(const kFontBase *font, kFontMetrics *metrics) override;
            void GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics) override;
            kSize TextSize(const char *text, int count, const kFontBase *font, kSize *bounds) override;
            void Text(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin) override;

            void BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend) override;
            void BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform) override;
            void BeginClippedDrawingByRect(const kRect &clip) override;
            void EndClippedDrawing() override;

        private:
            ID2D1PathGeometry* GeometryFromPoints(const kPoint *points, size_t count, bool closed);
            ID2D1PathGeometry* GeometryFromPointsBezier(const kPoint *points, size_t count, bool closed);

        private:
            struct Clip
            {
                ID2D1Layer       *layer;
                ID2D1BitmapBrush *brush;
            };

            HDC               boundDC;
            std::vector<Clip> clipStack;
        };


        /*
         -------------------------------------------------------------------------------
         kD2DStroke
         -------------------------------------------------------------------------------
            stroke resource object Direct2D implementation
        */
        class kD2DStroke : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_STYLE
            };

        public:
            kD2DStroke(const StrokeData &stroke);
            ~kD2DStroke() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_STYLE] = p_strokestyle;
            }

        private:
            ID2D1StrokeStyle *p_strokestyle;
        };


        /*
         -------------------------------------------------------------------------------
         kD2DPen
         -------------------------------------------------------------------------------
            pen resource object Direct2D implementation
        */
        class kD2DPen : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_BRUSH,
                RESOURCE_STYLE
            };

        public:
            kD2DPen(const PenData &pen);
            ~kD2DPen() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_BRUSH] = p_brush;
                native[RESOURCE_STYLE] = p_strokestyle;
            }

        private:
            ID2D1Brush       *p_brush;
            ID2D1StrokeStyle *p_strokestyle;
            float             p_width;
        };


        /*
         -------------------------------------------------------------------------------
         kD2DBrush
         -------------------------------------------------------------------------------
            brush resource object Direct2D implementation
        */
        class kD2DBrush : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_BRUSH
            };

        public:
            kD2DBrush(const BrushData &brush);
            ~kD2DBrush() override;

            ID2D1Brush* getBrush() const { return p_brush; }

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_BRUSH] = p_brush;
            }

        private:
            ID2D1Brush *p_brush;
        };


        /*
         -------------------------------------------------------------------------------
         kD2DFont
         -------------------------------------------------------------------------------
            font resource object Direct2D implementation
        */
        class kD2DFont : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_FONT,
                RESOURCE_FONTFACE
            };

        public:
            kD2DFont(const FontData &font);
            ~kD2DFont() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_FONT] = p_font;
                native[RESOURCE_FONTFACE] = p_face;
            }

        private:
            IDWriteFont     *p_font;
            IDWriteFontFace *p_face;
        };


        /*
         -------------------------------------------------------------------------------
         kD2DStrokeAllocator
         -------------------------------------------------------------------------------
            allocator for stroke Direct2D resource objects
        */
        class kD2DStrokeAllocator
        {
        public:
            static kD2DStroke* createResource(const StrokeData &stroke);
            static void deleteResource(kD2DStroke *stroke);
        };


        /*
         -------------------------------------------------------------------------------
         kD2DPenAllocator
         -------------------------------------------------------------------------------
            allocator for pen Direct2D resource objects
        */
        class kD2DPenAllocator
        {
        public:
            static kD2DPen* createResource(const PenData &pen);
            static void deleteResource(kD2DPen *pen);
        };


        /*
         -------------------------------------------------------------------------------
         kD2DBrushAllocator
         -------------------------------------------------------------------------------
            allocator for brush Direct2D resource objects
        */
        class kD2DBrushAllocator
        {
        public:
            static kD2DBrush* createResource(const BrushData &brush);
            static void deleteResource(kD2DBrush *brush);
        };


        /*
         -------------------------------------------------------------------------------
         kD2DFontAllocator
         -------------------------------------------------------------------------------
            allocator for font Direct2D resource objects
        */
        class kD2DFontAllocator
        {
        public:
            static kD2DFont* createResource(const FontData &font);
            static void deleteResource(kD2DFont *font);
        };


        /*
         -------------------------------------------------------------------------------
         CanvasFactoryD2D
         -------------------------------------------------------------------------------
            Direct2D factory implementation
        */
        class CanvasFactoryD2D : public CanvasFactoryImpl<
            kGradientImplD2D,
            kPathImplD2D,
            kBitmapImplD2D,
            kCanvasImplD2D,
            kD2DStrokeAllocator,
            kD2DPenAllocator,
            kD2DBrushAllocator,
            kD2DFontAllocator,
            kD2DStroke*,
            kD2DPen*,
            kD2DBrush*,
            kD2DFont*
        >
        {
            friend class CanvasFactory;
            friend class kGradientImplD2D;
            friend class kPathImplD2D;
            friend class kBitmapImplD2D;
            friend class kCanvasImplD2D;
            friend class kD2DStroke;
            friend class kD2DPen;
            friend class kD2DBrush;
            friend class kD2DBrushAllocator;
            friend class kD2DFont;

        public:
            bool initialized() override;

        public:
            CanvasFactoryD2D();
            ~CanvasFactoryD2D() override;

        private:
            HMODULE              p_d2d1_dll;
            HMODULE              p_dwrite_dll;

            ID2D1Factory        *p_factory;
            IDWriteFactory      *p_dwrite_factory;
            ID2D1DCRenderTarget *p_rt;
        };

    } // namespace impl
} // namespace k_canvas
