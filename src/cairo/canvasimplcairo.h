/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    cairo/canvasimplcairo.h
        canvas API Cairo implementation header
*/

#pragma once
#include "../canvasimpl.h"
#include "cairo/cairo.h"
#include <cstring>


namespace k_canvas
{
    namespace impl
    {
        // forwards
        class kCairoPen;
        class kCairoBrush;
        class kCairoFont;

        /*
         -------------------------------------------------------------------------------
         kGradientImplCairo
         -------------------------------------------------------------------------------
            gradient resource object Cairo implementation
        */
        class kGradientImplCairo : public kGradientImpl
        {
            friend class kCairoBrush;

        public:
            kGradientImplCairo();
            ~kGradientImplCairo() override;

            void Initialize(const kGradientStop *stops, size_t count, kExtendType extend) override;

        protected:
            kGradientStop *p_stops;
            size_t         p_count;
            kExtendType    p_extend;
        };


        /*
         -------------------------------------------------------------------------------
         kPathImplCairo
         -------------------------------------------------------------------------------
            path resource object Cairo implementation
        */
        class kPathImplCairo : public kPathImplDefault
        {
            friend class kCanvasImplCairo;

        public:
            kPathImplCairo();
            ~kPathImplCairo() override;

            // TODO: move this to default implementation?
            void FromPath(const kPathImpl *source, const kTransform &transform) override;
        };


        /*
         -------------------------------------------------------------------------------
         kBitmapImplCairo
         -------------------------------------------------------------------------------
            bitmap resource object Cairo implementation
        */
        class kBitmapImplCairo : public kBitmapImpl
        {
            friend class kCanvasImplCairo;
            friend class kCairoBrush;

        public:
            kBitmapImplCairo();
            ~kBitmapImplCairo() override;

            void Initialize(size_t width, size_t height, kBitmapFormat format) override;
            void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourceputch, const void *data) override;

        private:
            cairo_surface_t *p_bitmap;
            unsigned char   *p_data;
            size_t           p_width;
            size_t           p_height;
            size_t           p_pitch;
        };


        /*
         -------------------------------------------------------------------------------
         kCanvasImplCairo
         -------------------------------------------------------------------------------
            canvas Cairo implementation
        */
        class kCanvasImplCairo : public kCanvasImpl
        {
            friend class kPathImplCairo;
            friend class kCairoPenAllocator;
            friend class kCairoBrushAllocator;
            friend class kCairoFontAllocator;

        public:
            kCanvasImplCairo(const CanvasFactory *factory);
            ~kCanvasImplCairo() override;

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
                cairo_surface_t *surface;
                cairo_t         *cairo;
                cairo_pattern_t *pattern;
            };

            void PathToCairoPath(const kPathImpl *path, const kTransform &transform);
            void ApplyPen(const kPenBase *pen);
            void ApplyBrush(const kBrushBase *brush);
            void ApplyFont(const kFontBase *font);
            void FillAndStroke(const kPenBase *pen, const kBrushBase *brush);
            Clip& PushClip(bool save);
            void PopClip();

        private:
            cairo_t           *boundContext;
            bool               releaseContext;
            kRectInt           bounds;
            std::vector<Clip>  clipStack;
        };


        /*
         -------------------------------------------------------------------------------
         kCairoStroke
         -------------------------------------------------------------------------------
            stroke resource object Cairo implementation
        */
        class kCairoStroke : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_STROKE
            };

        public:
            kCairoStroke(const StrokeData &stroke);
            ~kCairoStroke() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_STROKE] = this;
            }

            void ApplyToContext(cairo_t *context, float width) const;

        private:
            StrokeData p_data;
        };


        /*
         -------------------------------------------------------------------------------
         kCairoPen
         -------------------------------------------------------------------------------
            pen resource object Cairo implementation
        */
        class kCairoPen : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_PEN
            };

        public:
            kCairoPen(const PenData &pen);
            ~kCairoPen() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_PEN] = this;
            }

            void ApplyToContext(cairo_t *context) const;

        private:
            kCairoBrush  *p_brush;
            kCairoStroke *p_stroke;
            float         p_width;
        };


        /*
         -------------------------------------------------------------------------------
         kCairoBrush
         -------------------------------------------------------------------------------
            brush resource object Cairo implementation
        */
        class kCairoBrush : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_BRUSH
            };

        public:
            kCairoBrush(const BrushData &brush);
            ~kCairoBrush() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_BRUSH] = this;
            }

            void ApplyToContext(cairo_t *context) const;

        private:
            kBrushStyle      p_style;
            kColorReal       p_color;
            cairo_pattern_t *p_pattern;
        };


        /*
         -------------------------------------------------------------------------------
         kCairoFont
         -------------------------------------------------------------------------------
            font resource object Cairo implementation
        */
        class kCairoFont : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_FONT
            };

        public:
            kCairoFont(const FontData &font);
            ~kCairoFont() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_FONT] = this;
            }

            void ApplyToContext(cairo_t *context) const;

        private:
            char                p_face[64];
            cairo_font_slant_t  p_slant;
            cairo_font_weight_t p_weight;
            float               p_size;
        };


        /*
         -------------------------------------------------------------------------------
         kCairoStrokeAllocator
         -------------------------------------------------------------------------------
            allocator for stroke Cairo resource objects
        */
        class kCairoStrokeAllocator
        {
        public:
            static kCairoStroke* createResource(const StrokeData &stroke);
            static void deleteResource(kCairoStroke *stroke);
        };


        /*
         -------------------------------------------------------------------------------
         kCairoPenAllocator
         -------------------------------------------------------------------------------
            allocator for pen Cairo resource objects
        */
        class kCairoPenAllocator
        {
        public:
            static kCairoPen* createResource(const PenData &pen);
            static void deleteResource(kCairoPen *pen);
        };


        /*
         -------------------------------------------------------------------------------
         kCairoBrushAllocator
         -------------------------------------------------------------------------------
            allocator for brush Cairo resource objects
        */
        class kCairoBrushAllocator
        {
        public:
            static kCairoBrush* createResource(const BrushData &brush);
            static void deleteResource(kCairoBrush *brush);
        };


        /*
         -------------------------------------------------------------------------------
         kCairoFontAllocator
         -------------------------------------------------------------------------------
            allocator for font Cairo resource objects
        */
        class kCairoFontAllocator
        {
        public:
            static kCairoFont* createResource(const FontData &font);
            static void deleteResource(kCairoFont *font);
        };


        /*
         -------------------------------------------------------------------------------
         CanvasFactoryCairo
         -------------------------------------------------------------------------------
            Cairo factory implementation
        */
        class CanvasFactoryCairo : public CanvasFactoryImpl<
            kGradientImplCairo,
            kPathImplCairo,
            kBitmapImplCairo,
            kCanvasImplCairo,
            kCairoStrokeAllocator,
            kCairoPenAllocator,
            kCairoBrushAllocator,
            kCairoFontAllocator,
            kCairoStroke*,
            kCairoPen*,
            kCairoBrush*,
            kCairoFont*
        >
        {
            friend class CanvasFactory;
            friend class kGradientImplCairo;
            friend class kPathImplCairo;
            friend class kBitmapImplCairo;
            friend class kCanvasImplCairo;
            friend class kCairoStroke;
            friend class kCairoPen;
            friend class kCairoBrush;
            friend class kCairoBrushAllocator;
            friend class kCairoFont;

        public:
            bool initialized() override;

        public:
            CanvasFactoryCairo();
            ~CanvasFactoryCairo() override;
        };

    } // namespace impl
} // namespace k_canvas
