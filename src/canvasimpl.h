/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvasimpl.h
        basic API objects implementation interfaces
        defines interfaces for platform independend API functionality
*/

#pragma once
#include "canvastypes.h"
#include "canvasresources.h"
#include <vector>


namespace k_canvas
{
    namespace impl
    {
        /*
         -------------------------------------------------------------------------------
         kRefcounted
         -------------------------------------------------------------------------------
            basic refcounted class implementation for unique resource objects
        */
        class kRefcounted
        {
        public:
            kRefcounted() :
                p_refcount(1)
            {}

            virtual ~kRefcounted()
            {}

            size_t addref()
            {
                return ++p_refcount;
            }

            size_t release()
            {
                if (--p_refcount == 0) {
                    // 
                    delete this;
                    return 0;
                }

                return p_refcount;
            }

        private:
            size_t p_refcount;
        };

        // safe resource release template func
        template <typename T>
        inline void ReleaseResource(T &resource)
        {
            if (resource) {
                resource->release();
                resource = nullptr;
            }
        }


        /*
         -------------------------------------------------------------------------------
         kResourceObject
         -------------------------------------------------------------------------------
            base class for shareable resource objects

            defines setupNativeResources(...) function which is used
            by shareable resource objects to fill in native resource handles
            for quick access inside implementation
        */
        class kResourceObject : public kRefcounted
        {
        public:
            virtual void setupNativeResources(void **native) = 0;
        };


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
            virtual void Text(const char *text, const kFontBase *font) = 0;
            virtual void Close() = 0;

            virtual void Clear() = 0;
            virtual void Commit() = 0;
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
            void Text(const char *text, const kFontBase *font) override;
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
                Command(CommandType _command, int _start_index = -1, size_t _element_count = 0, const kFontBase *_font = nullptr);

                CommandType      command;
                int              start_index;
                size_t           element_count;
                const kFontBase *font;
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
            virtual void Update(const kRectInt *updaterect, kBitmapFormat sourceformat, size_t sourceputch, void *data) = 0;
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

            virtual bool BindToBitmap(const kBitmapImpl *target) = 0;
            virtual bool BindToContext(kContext context) = 0;
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
            virtual void DrawBitmap(const kBitmapImpl *bitmap, const kPoint &origin, const kSize &destsize, const kPoint &source, const kSize &sourcesize, kScalar sourcealpha) = 0;

            virtual kSize TextSize(const char *text, int count, const kFontBase *font) = 0;
            virtual void TextOut(const kPoint &p, const char *text, int count, const kFontBase *font, const kBrushBase *brush) = 0;

        protected:
            static inline const PenData& penData(const kPenBase *pen) { return pen->data(); }
            static inline const BrushData& brushData(const kBrushBase *brush) { return brush->data(); } 
            static inline const FontData& fontData(const kFontBase *font) { return font->data(); }
            template<typename T>
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

            static kResourceObject* GetStrokeResource(const StrokeData &data);
            static kResourceObject* GetPenResource(const PenData &data);
            static kResourceObject* GetBrushResource(const BrushData &data);
            static kResourceObject* GetFontResource(const FontData &data);
            static void ReleaseStrokeResource(void *resource);
            static void ReleasePenResource(void *resource);
            static void ReleaseBrushResource(void *resource);
            static void ReleaseFontResource(void *resource);

            static void destroyFactory();

        protected:
            virtual bool initialized() = 0;

            virtual kGradientImpl* CreateGradientImpl() = 0;
            virtual kPathImpl* CreatePathImpl() = 0;
            virtual kBitmapImpl* CreateBitmapImpl() = 0;
            virtual kCanvasImpl* CreateCanvasImpl() = 0;

            virtual kResourceObject* getStrokeResource(const StrokeData &data) = 0;
            virtual kResourceObject* getPenResource(const PenData &data) = 0;
            virtual kResourceObject* getBrushResource(const BrushData &data) = 0;
            virtual kResourceObject* getFontResource(const FontData &data) = 0;
            virtual void releasePenResource(void *resource) = 0;
            virtual void releaseBrushResource(void *resource) = 0;
            virtual void releaseFontResource(void *resource) = 0;
            virtual void destroyResources() = 0;

        protected:
            CanvasFactory();
            virtual ~CanvasFactory();

            static CanvasFactory* getFactory();

        private:
            static CanvasFactory *factory;
            static Impl           current_impl;
        };


        template <typename Tdata, typename Tallocator, typename Tres>
        class ResourceManager
        {
        public:
            ResourceManager();
            ~ResourceManager();

            Tres getResource(const Tdata &data);
            void releaseResource(Tres resource);
            void Clear();

        private:
            static inline size_t hash_int(size_t value)
            {
                value ^= (value >> 20) ^ (value >> 12);
                return value ^ (value >> 7) ^ (value >> 4);
            }

            template <typename T> static inline size_t hash(T *data, size_t size)
            {
                size_t res = 0;
                const uint8_t *p = reinterpret_cast<const uint8_t*>(data);
                while (size > 4) {
                    res += hash_int(*reinterpret_cast<const size_t*>(p));
                    p += 4;
                    size -= 4;
                }
                if (size) {
                    size_t last = 0;
                    uint8_t *p_last = reinterpret_cast<uint8_t*>(&last);
                    while (size) {
                        *p_last = *(p++);
                        size--;
                    }
                    res += hash_int(last);
                }

                return res;
            }

            class h
            {
            public:
                size_t operator()(const Tdata &data) const
                {
                    return hash(&data, sizeof(Tdata));
                }

                bool operator()(const Tdata &left, const Tdata &right) const
                {
                    return left < right;
                }

                static const size_t bucket_size = 32;
            };

            struct Resource
            {
                Tres  resource;
                int   refcount;
            };

            typedef std::unordered_map<Tdata, Resource, h> Tcontainer;

            Tcontainer p_resources;
        };

        template <typename Tdata, typename Tallocator, typename Tres>
        ResourceManager<Tdata, Tallocator, Tres>::ResourceManager()
        {}

        template <typename Tdata, typename Tallocator, typename Tres>
        ResourceManager<Tdata, Tallocator, Tres>::~ResourceManager()
        {
            // ENSURE CLEAR
        }

        template <typename Tdata, typename Tallocator, typename Tres>
        Tres ResourceManager<Tdata, Tallocator, Tres>::getResource(const Tdata &data)
        {
            Tres result = nullptr;

            typename Tcontainer::iterator it = p_resources.find(data);
            if (it == p_resources.end()) {
#ifdef _DEBUG
                //dbg_resource_count++;
#endif
                result = Tallocator::createResource(data);
                Resource res;
                res.resource = result;
                res.refcount = 1;
                p_resources[data] = res;
            } else {
                result = it->second.resource;
                Tallocator::adjustResource(result, data);
                it->second.refcount++;
            }

            return result;
        }

        template <typename Tdata, typename Tallocator, typename Tres>
        void ResourceManager<Tdata, Tallocator, Tres>::releaseResource(Tres resource)
        {
            //if (--resource.refcount == 0) {
            //    //Tallocator::deleteResource(resource);
            //}
        }

        template <typename Tdata, typename Tallocator, typename Tres>
        void ResourceManager<Tdata, Tallocator, Tres>::Clear()
        {
            for (auto it = p_resources.begin(); it != p_resources.end(); it++) {
                Tallocator::deleteResource(it->second.resource);
#ifdef _DEBUG
                //dbg_resource_count--;
#endif
            }
            p_resources.clear();
        }

#ifdef _DEBUG
        //extern int dbg_resource_count;
#endif


        /*
         -------------------------------------------------------------------------------
         CanvasFactoryImpl
         -------------------------------------------------------------------------------
            factory implementation template
        */

        #define FACTORY_RESOURCE_MANAGER(name, allocator, res) \
        public:\
            virtual kResourceObject* get##name##Resource(const name##Data &data)\
            {\
                /*return (void*)name##_manager.getResource(data);*/\
                return allocator::createResource(data);\
            }\
            virtual void release##name##Resource(void* resource) {name##_manager.releaseResource(res(resource));}\
        private:\
            ResourceManager<name##Data, allocator, res> name##_manager;


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
                Pen_manager.Clear();
                Brush_manager.Clear();
                Font_manager.Clear();
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
