/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    canvasimpl.h
        basic API objects implementation interfaces
        defines interfaces for platform independend API functionality
*/

#pragma once
#include "canvastypes.h"
#include "canvasresources.h"
#include <vector>
#include <string>


namespace k_canvas
{
    namespace impl
    {
        /*
         -------------------------------------------------------------------------------
         kGradientImpl
         -------------------------------------------------------------------------------
            interface for kGradient implementation
        */
        class kGradientImpl : public kRefcounted
        {
        public:
            virtual void Initialize(const kGradientStop *stops, size_t count, kExtendType extend) = 0;
        };


        /*
         -------------------------------------------------------------------------------
         kPathImpl
         -------------------------------------------------------------------------------
            interface for kPath implementation
        */
        class kPathImpl : public kRefcounted
        {
        public:
            virtual void MoveTo(const kPoint &point) = 0;
            virtual void LineTo(const kPoint &point) = 0;
            virtual void BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3) = 0;
            virtual void PolyLineTo(const kPoint *points, size_t count) = 0;
            virtual void PolyBezierTo(const kPoint *points, size_t count) = 0;
            virtual void Text(const char *text, int count, const kFontBase *font, kTextOrigin origin) = 0;
            virtual void Close() = 0;

            virtual void Clear() = 0;
            virtual void Commit() = 0;

            virtual void FromPath(const kPathImpl *source, const kTransform &transform) = 0;
        };


        /*
         -------------------------------------------------------------------------------
         kPathImplDefault
         -------------------------------------------------------------------------------
            default implementation for kPathImpl interface
                used for back-ends without own path object
        */
        class kPathImplDefault : public kPathImpl
        {
        public:
            kPathImplDefault();
            ~kPathImplDefault() override;

            void MoveTo(const kPoint &p) override;
            void LineTo(const kPoint &p) override;
            void BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3) override;
            void PolyLineTo(const kPoint *points, size_t count) override;
            void PolyBezierTo(const kPoint *points, size_t count) override;
            void Text(const char *text, int count, const kFontBase *font, kTextOrigin origin) override;
            void Close() override;

            void Clear() override;
            void Commit() override;

        protected:
            enum CommandType
            {
                PC_MOVETO,
                PC_LINETO,
                PC_BEZIERTO,
                PC_POLYLINETO,
                PC_POLYBEZIERTO,
                PC_TEXT,
                PC_CLOSE
            };

            struct Command
            {
                Command();
                Command(CommandType _command, int _start_index = -1, size_t _element_count = 0, kResourceObject *_font = nullptr);
                ~Command();

                Command& operator=(const Command &source);

                CommandType      command;
                int              start_index;
                size_t           element_count;
                kResourceObject *font;
            };

            void AddCommand(CommandType command, size_t point_count);
            void AddCommand(const char *text, const kFontBase *font);
            void AddPoint(const kPoint &point);

        protected:
            std::vector<Command>     p_commands;
            size_t                   p_curr_command;
            std::vector<kPoint>      p_points;
            size_t                   p_curr_point;
            std::vector<std::string> p_text;
            size_t                   p_curr_text;
        };


        /*
         -------------------------------------------------------------------------------
         kBitmapImpl
         -------------------------------------------------------------------------------
            interface for kBitmap implementation
        */
        class kBitmapImpl : public kRefcounted
        {
        public:
            virtual void Initialize(size_t width, size_t height, kBitmapFormat format) = 0;
            virtual void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourceputch, const void *data) = 0;
        };


        /*
         -------------------------------------------------------------------------------
         kCanvasImpl
         -------------------------------------------------------------------------------
            interface for kCanvas implementation
        */
        class kCanvasImpl
        {
        public:
            virtual ~kCanvasImpl();

            virtual bool BindToBitmap(const kBitmapImpl *target, const kRectInt *rect) = 0;
            virtual bool BindToContext(kContext context, const kRectInt *rect) = 0;
            virtual bool BindToPrinter(kPrinter printer) = 0;
            virtual bool Unbind() = 0;

            virtual void Line(const kPoint &a, const kPoint &b, const kPenBase *pen) = 0;
            virtual void Bezier(const kPoint &p1, const kPoint &p2, const kPoint &p3, const kPoint &p4, const kPenBase *pen) = 0;
            virtual void PolyLine(const kPoint *points, size_t count, const kPenBase *pen) = 0;
            virtual void PolyBezier(const kPoint *points, size_t count, const kPenBase *pen) = 0;

            virtual void Rectangle(const kRect &rect, const kPenBase *pen, const kBrushBase *brush) = 0;
            virtual void RoundedRectangle(const kRect &rect, const kSize &round, const kPenBase *pen, const kBrushBase *brush) = 0;
            virtual void Ellipse(const kRect &rect, const kPenBase *pen, const kBrushBase *brush) = 0;
            virtual void Polygon(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush) = 0;
            virtual void PolygonBezier(const kPoint *points, size_t count, const kPenBase *pen, const kBrushBase *brush) = 0;

            virtual void DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush) = 0;
            virtual void DrawPath(const kPathImpl *path, const kPenBase *pen, const kBrushBase *brush, const kTransform &transform) = 0;
            virtual void DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha) = 0;
            virtual void DrawMask(const kBitmapImpl *mask, kBrushBase *brush, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize) = 0;

            virtual void GetFontMetrics(const kFontBase *font, kFontMetrics &metrics) = 0;
            virtual void GetGlyphMetrics(const kFontBase *font, size_t first, size_t last, kGlyphMetrics *metrics) = 0;
            virtual kSize TextSize(const char *text, size_t count, const kFontBase *font) = 0;
            virtual void Text(const kPoint &p, const char *text, size_t count, const kFontBase *font, const kBrushBase *brush, kTextOrigin origin) = 0;

            virtual void BeginClippedDrawingByMask(const kBitmapImpl *mask, const kTransform &transform, kExtendType xextend, kExtendType yextend) = 0;
            virtual void BeginClippedDrawingByPath(const kPathImpl *clip, const kTransform &transform) = 0;
            virtual void BeginClippedDrawingByRect(const kRect &clip) = 0;
            virtual void EndClippedDrawing() = 0;

            virtual void SetTransform(const kTransform &transform) = 0;

        protected:
            // access to resource data
            template <typename T, typename R>
            static inline const T& resourceData(const R *resource)
            {
                return resource->data();
            }

            // access to resource native pointers
            template <typename T>
            static inline void** native(const T *resource)
            {
                return resource->native();
            }
        };


        /*
         -------------------------------------------------------------------------------
         CanvasFactory
         -------------------------------------------------------------------------------
            abstract implementation factory
        */
        class CanvasFactory
        {
        public:
            static void setImpl(Impl impl);
            static Impl getImpl();

            static kGradientImpl* CreateGradient();
            static kPathImpl* CreatePath();
            static kBitmapImpl* CreateBitmap();
            static kCanvasImpl* CreateCanvas();

            template <typename T>
            static kResourceObject* GetResource(const T &data)
            {
                return getFactory()->getResource(data);
            }

            static void destroyFactory();

        protected:
            virtual bool initialized() = 0;

            virtual kGradientImpl* CreateGradientImpl() = 0;
            virtual kPathImpl* CreatePathImpl() = 0;
            virtual kBitmapImpl* CreateBitmapImpl() = 0;
            virtual kCanvasImpl* CreateCanvasImpl() = 0;

            virtual kResourceObject* getResource(const StrokeData &data) = 0;
            virtual kResourceObject* getResource(const PenData &data) = 0;
            virtual kResourceObject* getResource(const BrushData &data) = 0;
            virtual kResourceObject* getResource(const FontData &data) = 0;

            virtual void destroyResources() = 0;

        protected:
            CanvasFactory();
            virtual ~CanvasFactory();

            static CanvasFactory* getFactory();

        private:
            static CanvasFactory *factory;
            static Impl           current_impl;
        };


        /*
         -------------------------------------------------------------------------------
         CanvasFactoryImpl
         -------------------------------------------------------------------------------
            factory implementation template
        */

        #define FACTORY_RESOURCE_MANAGER(name, allocator, res) \
        public:\
            kResourceObject* getResource(const name##Data &data) override\
            {\
                return allocator::createResource(data);\
            }\
        private:\


        template <
            typename Tgradientimpl,
            typename Tpathimpl,
            typename Tbitmapimpl,
            typename Tcanvasimpl,
            typename Tstrokeallocator,
            typename Tpenallocator,
            typename Tbrushallocator,
            typename Tfontallocator,
            typename Tstroke,
            typename Tpen,
            typename Tbrush,
            typename Tfont
        >
        class CanvasFactoryImpl : public CanvasFactory {
        protected:
            kGradientImpl* CreateGradientImpl() override { return new Tgradientimpl(); }
            kPathImpl* CreatePathImpl() override { return new Tpathimpl(); }
            kBitmapImpl* CreateBitmapImpl() override { return new Tbitmapimpl(); }
            kCanvasImpl* CreateCanvasImpl() override { return new Tcanvasimpl(this); }

            void destroyResources() override
            {
                // TODO: implement resource management
            }

            FACTORY_RESOURCE_MANAGER(Stroke, Tstrokeallocator, Tstroke)
            FACTORY_RESOURCE_MANAGER(Pen, Tpenallocator, Tpen)
            FACTORY_RESOURCE_MANAGER(Brush, Tbrushallocator, Tbrush)
            FACTORY_RESOURCE_MANAGER(Font, Tfontallocator, Tfont)
        };

        #undef FACTORY_RESOURCE_MANAGER



        typedef CanvasFactory* (*CanvasFactoryCreateProc)();
        struct CanvasImplDesc
        {
            CanvasImplDesc(Impl impl_, CanvasFactoryCreateProc proc_) : 
                implementation(impl_),
                createproc(proc_)
            {}

            Impl                    implementation;
            CanvasFactoryCreateProc createproc;
        };

        extern const CanvasImplDesc factory_descriptors[];

        #define FACTORY_DESCRIPTORS_BEGIN() const CanvasImplDesc factory_descriptors[] = {
        #define FACTORY_DESCRIPTOR(impl_, proc_) CanvasImplDesc(impl_, proc_),
        #define FACTORY_DESCRIPTORS_END() CanvasImplDesc(IMPL_NONE, nullptr) };

    } // namespace impl
} // namespace k_canvas
