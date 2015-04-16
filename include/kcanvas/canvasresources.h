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
         kSharedResourceBase
         -------------------------------------------------------------------------------
            base class template for all shareable resource objects
        */
        template <typename Tdata>
        class kSharedResourceBase
        {
            friend class k_canvas::kPen;
            friend class k_canvas::kCanvas;
            friend class kCanvasImpl;

        protected:
            kSharedResourceBase() :
                p_resource(nullptr)
            {}

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
            kResourceObject* createResource() const;

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

            const Tdata& data() const { return p_data; }
            void** native() const { return p_native; }

        protected:
            static const size_t MAX_NATIVE_RESOURCES = 4;

        protected:
            Tdata                    p_data;                         // resource data (properties)
            mutable kResourceObject *p_resource;                     // resource refcount object
            mutable void            *p_native[MAX_NATIVE_RESOURCES]; // native resource pointer copy
        };


        /*
         -------------------------------------------------------------------------------
         StrokeData
         -------------------------------------------------------------------------------
            stroke resource object properties data structure
        */
        struct StrokeData
        {
            kStrokeStyle p_style;
            kCapStyle    p_startcap;
            kCapStyle    p_endcap;
            kCapStyle    p_dashcap;
            kScalar      p_dashoffset;
            kLineJoin    p_join;
            kScalar      p_stroke[16];
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

            bool operator<(const PenData &other) const
            {
                if (p_brush == other.p_brush) {
                    if (p_stroke == other.p_stroke) {
                        return p_width < other.p_width;
                    } else {
                        return p_stroke < other.p_stroke;
                    }
                } else {
                    return p_brush < other.p_brush;
                }
            }

            bool operator==(const PenData &other) const
            {
                return p_brush == other.p_brush && p_stroke == other.p_stroke &&
                       p_width == other.p_width;
            }
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

            bool operator<(const BrushData &other) const
            {
                if (p_style == other.p_style) {
                    return p_color < other.p_color;
                } else {
                    return p_style < other.p_style;
                }
            }

            bool operator==(const BrushData &other) const
            {
                return p_style == other.p_style && p_color == other.p_color;
            }
        };


        /*
         -------------------------------------------------------------------------------
         FontData
         -------------------------------------------------------------------------------
            font resource object properties data structure
        */
        struct FontData
        {
            char       p_facename[32]; // == LF_FACESIZE
            kFontStyle p_style;
            kScalar    p_size;

            bool operator<(const FontData &other) const
            {
                if (p_style == other.p_style) {
                    if (p_size == other.p_size) {
                        return strcmp(p_facename, other.p_facename) < 0;
                    } else {
                        return p_size < other.p_size;
                    }
                } else {
                    return p_style < other.p_style;
                }
            }

            bool operator==(const FontData &other) const
            {
                return p_style == other.p_style && p_size == other.p_size && strcmp(p_facename, other.p_facename) == 0;
            }
        };


        // typedefs for base resource object classes
        typedef kSharedResourceBase<StrokeData> kStrokeBase;
        typedef kSharedResourceBase<PenData> kPenBase;
        typedef kSharedResourceBase<BrushData> kBrushBase;
        typedef kSharedResourceBase<FontData> kFontBase;

    } // namespace impl
} // namespace k_canvas
