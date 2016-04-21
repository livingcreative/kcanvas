/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvasresources.h
        canvas resource object types
        defines types and classes for resource objects
*/

#pragma once
#include "canvastypes.h"
#include <string>
#include <unordered_map>


namespace k_canvas
{
    // Forward kCanvas objects declarations
    class kPen;
    class kPath;
    class kTextService;
    class kCanvas;

    namespace impl
    {
        // Implementation forward declarations
        class kResourceObject;
        class kCanvasImpl;
        class kGradientImpl;
        class kBitmapImpl;


        /*
         -------------------------------------------------------------------------------
         StrokeData
         -------------------------------------------------------------------------------
            stroke resource object properties data structure
        */
        enum
        {
            MAX_STROKES = 16
        };

        struct StrokeData
        {
            kStrokeStyle p_style;
            kCapStyle    p_startcap;
            kCapStyle    p_endcap;
            kCapStyle    p_dashcap;
            kScalar      p_dashoffset;
            kLineJoin    p_join;
            kScalar      p_stroke[MAX_STROKES];
            size_t       p_count;
        };


        /*
         -------------------------------------------------------------------------------
         PenData
         -------------------------------------------------------------------------------
            pen resource object properties data structure
        */
        struct PenData
        {
            kScalar          p_width;  // pen width
            kResourceObject *p_brush;  // reference to brush resource object
            kResourceObject *p_stroke; // reference to stroke resource object
        };


        /*
         -------------------------------------------------------------------------------
         BrushData
         -------------------------------------------------------------------------------
            brush resource object properties data structure
        */
        struct BrushData
        {
            kBrushStyle    p_style;
            kColor         p_color;
            kPoint         p_start;
            kPoint         p_end;
            kSize          p_radius;
            kExtendType    p_xextend;
            kExtendType    p_yextend;
            kGradientImpl *p_gradient; // rference to gradient resource object
            kBitmapImpl   *p_bitmap;   // rference to bitmap resource object
        };


        /*
         -------------------------------------------------------------------------------
         FontData
         -------------------------------------------------------------------------------
            font resource object properties data structure
        */
        enum
        {
            MAX_FONT_FACE_LENGTH = 32
        };

        struct FontData
        {
            char       p_facename[MAX_FONT_FACE_LENGTH];
            kFontStyle p_style;
            kScalar    p_size;
        };


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
                    // in general every factory could define its own
                    // way for resource allocation and destruction, so
                    // simple delete operator isn't suitable and this
                    // behaviour could be changed in future
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
         kSharedResourceBase
         -------------------------------------------------------------------------------
            base class template for all shareable resource objects
        */
        enum
        {
            MAX_NATIVE_RESOURCES = 4
        };

        template <typename Tdata>
        class kSharedResourceBase
        {
            friend class k_canvas::kPen;
            friend class k_canvas::kPath;
            friend class k_canvas::kTextService;
            friend class k_canvas::kCanvas;
            friend class kCanvasImpl;

        protected:
            kSharedResourceBase() :
                p_resource(nullptr)
            {}

            kSharedResourceBase(const kSharedResourceBase<Tdata> &source) :
                p_data(source.p_data),
                p_resource(nullptr)
            {
                AssignResource(source);
            }

            ~kSharedResourceBase()
            {
                if (p_resource) {
                    p_resource->release();
                    p_resource = nullptr;
                }
            }

            // createResource implementation should fill in all needed
            // native resources, so underlying implementation could quickly
            // access them
            kResourceObject* createResource() const
            {
                kResourceObject *result = CanvasFactory::GetResource(p_data);
                result->setupNativeResources(p_native);
                return result;
            }

            // use need resource to ensure that implementation resource
            // object is created and ready to use
            void needResource() const
            {
                if (p_resource == nullptr) {
                    p_resource = createResource();
                }
            }

            // use getResource to get new reference for resource
            // returned resource MUST BE released
            kResourceObject* getResource() const
            {
                needResource();
                p_resource->addref();
                return p_resource;
            }

            void AssignResource(const kSharedResourceBase<Tdata> &source)
            {
                if (p_resource) {
                    p_resource->release();
                }

                p_resource = source.p_resource;

                if (p_resource) {
                    for (size_t n = 0; n < MAX_NATIVE_RESOURCES; ++n) {
                        p_native[n] = source.p_native[n];
                    }
                    p_resource->addref();
                }
            }

            template <typename T>
            void AssignReferencedResource(T source, T &dest)
            {
                if (source != dest) {
                    if (dest) {
                        dest->release();
                    }
                    dest = source;
                    if (dest) {
                        dest->addref();
                    }
                }
            }

            const Tdata& data() const { return p_data; }
            void** native() const { return p_native; }

        protected:
            Tdata                    p_data;                         // resource data (properties)
            mutable kResourceObject *p_resource;                     // resource refcount object
            mutable void            *p_native[MAX_NATIVE_RESOURCES]; // native resource pointer copy
        };


        // typedefs for base resource object classes
        typedef kSharedResourceBase<StrokeData> kStrokeBase;
        typedef kSharedResourceBase<PenData> kPenBase;
        typedef kSharedResourceBase<BrushData> kBrushBase;
        typedef kSharedResourceBase<FontData> kFontBase;

    } // namespace impl
} // namespace k_canvas
