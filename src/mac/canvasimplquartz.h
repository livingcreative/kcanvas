/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    mac/canvasimplquartz.h
        canvas API Quartz implementation header
*/

#pragma once
#include "../canvasimpl.h"
#include <QuartzCore/QuartzCore.h>


namespace k_canvas
{
    namespace impl
    {
        /*
         -------------------------------------------------------------------------------
         kGradientImplQuartz
         -------------------------------------------------------------------------------
            gradient resource object Quartz implementation
        */
        class kGradientImplQuartz : public kGradientImpl
        {
            friend class kQuartzBrush;

        public:
            kGradientImplQuartz();
            ~kGradientImplQuartz() override;

            void Initialize(const kGradientStop *stops, size_t count, kExtendType extend) override;

        protected:
        };


        /*
         -------------------------------------------------------------------------------
         kPathImplQuartz
         -------------------------------------------------------------------------------
            path resource object Quartz implementation
        */
        class kPathImplQuartz : public kPathImplDefault
        {
            friend class kCanvasImplQuartz;

        public:
            kPathImplQuartz();
            ~kPathImplQuartz() override;

            // TODO: move this to default implementation?
            void FromPath(const kPathImpl *source, const kTransform &transform) override;
        };


        /*
         -------------------------------------------------------------------------------
         kBitmapImplQuartz
         -------------------------------------------------------------------------------
            bitmap resource object Quartz implementation
        */
        class kBitmapImplQuartz : public kBitmapImpl
        {
            friend class kCanvasImplQuartz;
            friend class kQuartzBrush;

        public:
            kBitmapImplQuartz();
            ~kBitmapImplQuartz() override;

            void Initialize(size_t width, size_t height, kBitmapFormat format) override;
            void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourceputch, void *data) override;

        private:
            size_t p_width;
            size_t p_height;
            size_t p_pitch;
        };


        /*
         -------------------------------------------------------------------------------
         kCanvasImplQuartz
         -------------------------------------------------------------------------------
            canvas Quartz implementation
        */
        class kCanvasImplQuartz : public kCanvasImpl
        {
            friend class kPathImplQuartz;
            friend class kQuartzPenAllocator;
            friend class kQuartzBrushAllocator;
            friend class kQuartzFontAllocator;

        public:
            kCanvasImplQuartz(const CanvasFactory *factory);
            ~kCanvasImplQuartz() override;

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

            void GetFontMetrics(const kFontBase *font, kFontMetrics *metrics) override;
            void GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics) override;
            kSize TextSize(const char *text, int count, const kFontBase *font, kSize *bounds) override;
            void Text(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin) override;

            void BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend) override;
            void BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform) override;
            void BeginClippedDrawingByRect(const kRect &clip) override;
            void EndClippedDrawing() override;

            void SetTransform(const kTransform &transform) override;

        private:
            void PathToQuartzPath(const kPathImpl *path);
            void ApplyPen(const kPenBase *pen);
            void ApplyBrush(const kBrushBase *brush);
            void ApplyFont(const kFontBase *font);

        private:
            CGContextRef boundContext;
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzStroke
         -------------------------------------------------------------------------------
            stroke resource object Quartz implementation
        */
        class kQuartzStroke : public kResourceObject
        {
        public:
            kQuartzStroke(const StrokeData &stroke);
            ~kQuartzStroke() override;

            void setupNativeResources(void **native) override
            {}
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzPen
         -------------------------------------------------------------------------------
            pen resource object Quartz implementation
        */
        class kQuartzPen : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_PEN
            };

        public:
            kQuartzPen(const PenData &pen);
            ~kQuartzPen() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_PEN] = this;
            }

            void ApplyToContext(CGContextRef context) const;

        private:
            CGColorSpaceRef p_colorspace; // TODO: move to context
            CGColorRef      p_color;
            float           p_width;
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzBrush
         -------------------------------------------------------------------------------
            brush resource object Quartz implementation
        */
        class kQuartzBrush : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_BRUSH
            };

        public:
            kQuartzBrush(const BrushData &brush);
            ~kQuartzBrush() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_BRUSH] = this;
            }

            void ApplyToContext(CGContextRef context) const;

        private:
            CGColorSpaceRef p_colorspace; // TODO: move to context
            CGColorRef      p_color;
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzFont
         -------------------------------------------------------------------------------
            font resource object Quartz implementation
        */
        class kQuartzFont : public kResourceObject
        {
        public:
            enum ResourceIndex
            {
                RESOURCE_FONT
            };

        public:
            kQuartzFont(const FontData &font);
            ~kQuartzFont() override;

            void setupNativeResources(void **native) override
            {
                native[RESOURCE_FONT] = this;
            }

            void ApplyToContext(CGContextRef context) const;

        private:
            CTFontDescriptorRef p_fontdesc;
            CTFontRef           p_font;
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzStrokeAllocator
         -------------------------------------------------------------------------------
            allocator for stroke Quartz resource objects
        */
        class kQuartzStrokeAllocator
        {
        public:
            static kQuartzStroke* createResource(const StrokeData &stroke);
            static void deleteResource(kQuartzStroke *stroke);
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzPenAllocator
         -------------------------------------------------------------------------------
            allocator for pen Quartz resource objects
        */
        class kQuartzPenAllocator
        {
        public:
            static kQuartzPen* createResource(const PenData &pen);
            static void deleteResource(kQuartzPen *pen);
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzBrushAllocator
         -------------------------------------------------------------------------------
            allocator for brush Quartz resource objects
        */
        class kQuartzBrushAllocator
        {
        public:
            static kQuartzBrush* createResource(const BrushData &brush);
            static void deleteResource(kQuartzBrush *brush);
        };


        /*
         -------------------------------------------------------------------------------
         kQuartzFontAllocator
         -------------------------------------------------------------------------------
            allocator for font Quartz resource objects
        */
        class kQuartzFontAllocator
        {
        public:
            static kQuartzFont* createResource(const FontData &font);
            static void deleteResource(kQuartzFont *font);
        };


        /*
         -------------------------------------------------------------------------------
         CanvasFactoryQuartz
         -------------------------------------------------------------------------------
            Quartz factory implementation
        */
        class CanvasFactoryQuartz : public CanvasFactoryImpl<
            kGradientImplQuartz,
            kPathImplQuartz,
            kBitmapImplQuartz,
            kCanvasImplQuartz,
            kQuartzStrokeAllocator,
            kQuartzPenAllocator,
            kQuartzBrushAllocator,
            kQuartzFontAllocator,
            kQuartzStroke*,
            kQuartzPen*,
            kQuartzBrush*,
            kQuartzFont*
        >
        {
            friend class CanvasFactory;
            friend class kGradientImplQuartz;
            friend class kPathImplQuartz;
            friend class kBitmapImplQuartz;
            friend class kCanvasImplQuartz;
            friend class kQuartzStroke;
            friend class kQuartzPen;
            friend class kQuartzBrush;
            friend class kQuartzBrushAllocator;
            friend class kQuartzFont;

        public:
            bool initialized() override;

        public:
            CanvasFactoryQuartz();
            ~CanvasFactoryQuartz() override;
        };

    } // namespace impl
} // namespace k_canvas
