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

#undef TextOut


namespace k_canvas
{
    namespace impl
    {

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
            void Text(const char *text, const kFontBase *font) override;
            void Close() override;
            
            void Clear() override;
            void Commit() override;

        private:
            void OpenSink();
            void CloseSink() const;
            void OpenFigure(const D2D1_POINT_2F &p);
            void CloseFigure(bool opened) const;

        private:
            ID2D1PathGeometry         *p_path;
            mutable ID2D1GeometrySink *p_sink;
            mutable bool               p_opened;
        };


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


        class kCanvasImplD2D : public kCanvasImpl
        {
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
            void DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha) override;

            kSize TextSize(const char *text, int count, const kFontBase *font) override;
            void TextOut(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush) override;

        private:
            ID2D1PathGeometry* GeomteryFromPoints(const kPoint *points, size_t count, bool closed);
            ID2D1PathGeometry* GeomteryFromPointsBezier(const kPoint *points, size_t count, bool closed);

        private:
            HDC boundDC;
        };


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

            ID2D1StrokeStyle* getStrokeStyle() const { return p_strokestyle; }

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_STYLE] = p_strokestyle;
            }

        private:
            ID2D1StrokeStyle *p_strokestyle;
        };


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

            ID2D1Brush* getBrush() const { return p_brush; }
            ID2D1StrokeStyle* getStrokeStyle() const { return p_strokestyle; }
            float getWidth() const { return p_width; }

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


        class kD2DBrush : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_BRUSH,
                RESOURCE_GRADIENT
            };

        public:
            kD2DBrush(const BrushData &brush);
            ~kD2DBrush() override;

            ID2D1Brush* getBrush() const { return p_brush; }

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_BRUSH] = p_brush;
                native[RESOURCE_GRADIENT] = p_gradient;
            }

        private:
            ID2D1Brush                  *p_brush;
            ID2D1GradientStopCollection *p_gradient;
            ID2D1Bitmap                 *p_bitmap;
        };


        class kD2DFont : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_TEXTFORMAT,
                RESOURCE_FONT
            };

        public:
            kD2DFont(const FontData &font);
            ~kD2DFont() override;

            IDWriteTextFormat* getTextFormat() const { return p_textformat; }
            IDWriteFont* getFont() const { return p_font; }

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_TEXTFORMAT] = p_textformat;
                native[RESOURCE_FONT] = p_font;
            }

        private:
            IDWriteTextFormat *p_textformat;
            IDWriteFont       *p_font;
        };


        class kD2DStrokeAllocator
        {
        public:
            static kD2DStroke* createResource(const StrokeData &stroke);
            static void deleteResource(kD2DStroke *stroke);
            static void adjustResource(kD2DStroke *resource, const StrokeData &data) {}
        };

        class kD2DPenAllocator
        {
        public:
            static kD2DPen* createResource(const PenData &pen);
            static void deleteResource(kD2DPen *pen);
            static void adjustResource(kD2DPen *resource, const PenData &data) {}
        };

        class kD2DBrushAllocator
        {
        public:
            static kD2DBrush* createResource(const BrushData &brush);
            static void deleteResource(kD2DBrush *brush);
            static void adjustResource(kD2DBrush *resource, const BrushData &data);
        };

        class kD2DFontAllocator
        {
        public:
            static kD2DFont* createResource(const FontData &font);
            static void deleteResource(kD2DFont *font);
            static void adjustResource(kD2DFont *resource, const FontData &data) {}
        };

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
