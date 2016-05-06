/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    win/canvasimplgdiplus.h
        canvas API GDI+ implementation header
*/

#pragma once
#include "../canvasimpl.h"
#include <Windows.h>
#include <gdiplus.h>


namespace k_canvas
{
    namespace impl
    {
        /*
         -------------------------------------------------------------------------------
         kGradientImplGDIPlus
         -------------------------------------------------------------------------------
            gradient resource object GDI+ implementation
        */
        class kGradientImplGDIPlus : public kGradientImpl
        {
            friend class kGDIPlusBrush;

        public:
            kGradientImplGDIPlus();
            ~kGradientImplGDIPlus() override;

            void Initialize(const kGradientStop *stops, size_t count, kExtendType extend) override;

        protected:
            kGradientStop *p_stops;
            size_t         p_count;
            kExtendType    p_extend;
        };


        /*
         -------------------------------------------------------------------------------
         kPathImplGDIPlus
         -------------------------------------------------------------------------------
            path resource object GDI+ implementation
        */
        class kPathImplGDIPlus : public kPathImpl
        {
            friend class kCanvasImplGDIPlus;

        public:
            kPathImplGDIPlus();
            ~kPathImplGDIPlus() override;

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

        private:
            Gdiplus::GraphicsPath *p_path;
            kPoint                 p_cp;
        };


        /*
         -------------------------------------------------------------------------------
         kBitmapImplGDIPlus
         -------------------------------------------------------------------------------
            bitmap resource object GDI+ implementation
        */
        class kBitmapImplGDIPlus : public kBitmapImpl
        {
            friend class kCanvasImplGDIPlus;
            friend class kGDIPlusBrush;

        public:
            kBitmapImplGDIPlus();
            ~kBitmapImplGDIPlus() override;

            void Initialize(size_t width, size_t height, kBitmapFormat format) override;
            void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourceputch, const void *data) override;

        private:
            Gdiplus::Bitmap *p_bitmap;
        };


        /*
         -------------------------------------------------------------------------------
         kCanvasImplGDIPlus
         -------------------------------------------------------------------------------
            canvas GDI+ implementation
        */
        class kCanvasImplGDIPlus : public kCanvasImpl
        {
            friend class kPathImplGDIPlus;
            friend class kGDIPlusPenAllocator;
            friend class kGDIPlusBrushAllocator;
            friend class kGDIPlusFontAllocator;

        public:
            kCanvasImplGDIPlus(const CanvasFactory *factory);
            ~kCanvasImplGDIPlus() override;

            bool BindToBitmap(const kBitmapImpl *target, const kRectInt *rect) override;
            bool BindToContext(kContext context, const kRectInt *rect) override;
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

            void GetFontMetrics(const kFontBase *font, kFontMetrics &metrics) override;
            void GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics) override;
            kSize TextSize(const char *text, size_t count, const kFontBase *font) override;
            void Text(const kPoint &p, const char *text, size_t count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin) override;

            void BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend) override;
            void BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform) override;
            void BeginClippedDrawingByRect(const kRect &clip) override;
            void EndClippedDrawing() override;

            void SetTransform(const kTransform &transform) override;

        private:
            struct Clip
            {
                Gdiplus::GraphicsContainer  container;
                Gdiplus::Bitmap            *maskedimage;
                Gdiplus::Bitmap            *mask;
                Gdiplus::PointF             origin;
                Gdiplus::Graphics          *graphics;
                kRectInt                    boundrect;
            };

            void SetDefaults();
            void UpdateTransform();
            Clip& PushClipState(bool createcontainer);
            void PopClipState();

            void CopyMask(int width, int height, Gdiplus::Bitmap *maskedimage, Gdiplus::Bitmap *mask);

            void DrawPathImpl(const Gdiplus::GraphicsPath *path, const kPenBase *pen, const kBrushBase *brush);

        private:
            HDC                          boundDC;
            HDC                          cacheDC;
            HGDIOBJ                      prevbitmap;
            kRectInt                     boundrect;
            Gdiplus::Graphics           *g;
            const Gdiplus::StringFormat *sf;
            kTransform                   transform;
            std::vector<Clip>            clipStack;
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusStroke
         -------------------------------------------------------------------------------
            stroke resource object GDI+ implementation
        */
        class kGDIPlusStroke : public kResourceObject
        {
        public:
            kGDIPlusStroke(const StrokeData &stroke);
            ~kGDIPlusStroke() override;

            void setupNativeResources(void **native) override
            {}

            const StrokeData& getStrokeData() const { return p_stroke; }

        private:
            StrokeData p_stroke;
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusPen
         -------------------------------------------------------------------------------
            pen resource object GDI+ implementation
        */
        class kGDIPlusPen : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_PEN
            };

        public:
            kGDIPlusPen(const PenData &pen);
            ~kGDIPlusPen() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_PEN] = p_pen;
            }

        private:
            Gdiplus::Pen *p_pen;
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusBrush
         -------------------------------------------------------------------------------
            brush resource object GDI+ implementation
        */
        class kGDIPlusBrush : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_BRUSH,
                RESUURCE_SECONDARYBRUSH,
                RESUURCE_SECONDARYBRUSHCLIP
            };

        public:
            kGDIPlusBrush(const BrushData &brush);
            ~kGDIPlusBrush() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_BRUSH] = p_brush;
                native[RESUURCE_SECONDARYBRUSH] = p_secondarybrush;
                native[RESUURCE_SECONDARYBRUSHCLIP] = p_secondaryclip;
            }

        private:
            Gdiplus::Brush        *p_brush;
            Gdiplus::Brush        *p_secondarybrush;
            Gdiplus::GraphicsPath *p_secondaryclip;
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusFont
         -------------------------------------------------------------------------------
            font resource object GDI+ implementation
        */
        class kGDIPlusFont : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_FONT,
                RESOURCE_GDIFONT
            };

        public:
            kGDIPlusFont(const FontData &font);
            ~kGDIPlusFont() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_FONT] = p_font;
                native[RESOURCE_GDIFONT] = p_gdifont;
            }

        private:
            Gdiplus::Font *p_font;
            HFONT          p_gdifont;
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusStrokeAllocator
         -------------------------------------------------------------------------------
            allocator for stroke GDI+ resource objects
        */
        class kGDIPlusStrokeAllocator
        {
        public:
            static kGDIPlusStroke* createResource(const StrokeData &stroke);
            static void deleteResource(kGDIPlusStroke *stroke);
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusPenAllocator
         -------------------------------------------------------------------------------
            allocator for pen GDI+ resource objects
        */
        class kGDIPlusPenAllocator
        {
        public:
            static kGDIPlusPen* createResource(const PenData &pen);
            static void deleteResource(kGDIPlusPen *pen);
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusBrushAllocator
         -------------------------------------------------------------------------------
            allocator for brush GDI+ resource objects
        */
        class kGDIPlusBrushAllocator
        {
        public:
            static kGDIPlusBrush* createResource(const BrushData &brush);
            static void deleteResource(kGDIPlusBrush *brush);
        };


        /*
         -------------------------------------------------------------------------------
         kGDIPlusFontAllocator
         -------------------------------------------------------------------------------
            allocator for font GDI+ resource objects
        */
        class kGDIPlusFontAllocator
        {
        public:
            static kGDIPlusFont* createResource(const FontData &font);
            static void deleteResource(kGDIPlusFont *font);
        };


        /*
         -------------------------------------------------------------------------------
         CanvasFactoryGDIPlus
         -------------------------------------------------------------------------------
            GDI+ factory implementation
        */
        class CanvasFactoryGDIPlus : public CanvasFactoryImpl<
            kGradientImplGDIPlus,
            kPathImplGDIPlus,
            kBitmapImplGDIPlus,
            kCanvasImplGDIPlus,
            kGDIPlusStrokeAllocator,
            kGDIPlusPenAllocator,
            kGDIPlusBrushAllocator,
            kGDIPlusFontAllocator,
            kGDIPlusStroke*,
            kGDIPlusPen*,
            kGDIPlusBrush*,
            kGDIPlusFont*
        >
        {
            friend class CanvasFactory;
            friend class kGradientImplGDIPlus;
            friend class kPathImplGDIPlus;
            friend class kBitmapImplGDIPlus;
            friend class kCanvasImplGDIPlus;
            friend class kGDIPlusStroke;
            friend class kGDIPlusPen;
            friend class kGDIPlusBrush;
            friend class kGDIPlusBrushAllocator;
            friend class kGDIPlusFont;

        public:
            bool initialized() override;

        public:
            CanvasFactoryGDIPlus();
            ~CanvasFactoryGDIPlus() override;

        private:
            Gdiplus::GdiplusStartupInput gdiplusStartupInput;
            ULONG_PTR                    gdiplusToken;
        };

    } // namespace impl
} // namespace k_canvas
