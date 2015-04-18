/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvastypes.h
        basic canvas types used both in public include and internally in source
        by default it uses c_util.h basic types
*/

#pragma once
#include "kcommon/c_util.h" // common utility funcs and types
#include "canvasplatform.h" // implementation platform support
#include <cstdint>


namespace k_canvas
{
    // base scalar type (float is default)
    typedef float kScalar;

    // basic types derived from c_util types
    //      these types could be replaced with any compatible types
    //      you like (you might want to use your preffered lib for these types)
    //      see documentation for proper types replacement
    typedef c_util::pointT<kScalar> kPoint;
    typedef c_util::sizeT<kScalar> kSize;
    typedef c_util::rectT<kScalar> kRect;
    typedef c_util::rectT<int> kRectInt;

    // extend mode
    enum class kExtendType
    {
        Clamp = 0,
        Wrap  = 1
    };

    // kStroke styles
    enum class kStrokeStyle
    {
        Clear      = 0,
        Solid      = 1,
        Dot        = 2,
        Dash       = 3,
        DashDot    = 4,
        DashDotDot = 5,
        Custom     = 6
    };

    // line cap styles
    enum class kCapStyle
    {
        Flat   = 0,
        Square = 1,
        Round  = 2
    };

    // line join styles
    enum class kLineJoin
    {
        Miter = 0,
        Bevel = 1,
        Round = 2
    };

    // kBrush styles
    enum class kBrushStyle
    {
        Clear          = 0,
        Solid          = 1,
        LinearGradient = 2,
        RadialGradient = 3,
        Bitmap         = 4
    };

    // kFont style flags
    //     this can't be simple enum class because technically it's
    //     a bit flags set
    struct kFontStyle
    {
        enum Style {
            Bold          = 0x01,
            Italic        = 0x02,
            Underline     = 0x04,
            Strikethrough = 0x08
        };

        kFontStyle() : p_value() {}
        kFontStyle(Style value) : p_value(value) {}
        operator uint32_t() const { return p_value; }
        kFontStyle operator=(const Style value) { p_value = value; return *this; }
        kFontStyle operator=(const uint32_t value) { p_value = value; return *this; }

    private:
        uint32_t p_value;
    };

    // Text drawing origin
    enum class kTextOrigin
    {
        Top      = 0,
        BaseLine = 1
    };

    // kBitmap data formats
    enum class kBitmapFormat
    {
        Color32BitAlphaPremultiplied = 1,
        Mask8Bit                     = 2
    };

    // kColor
    //      basic color struct used in canvas API
    struct kColor
    {
        uint8_t r, g, b, a;

        inline kColor();
        inline kColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255);

        inline kColor inverse() const;

        inline bool operator==(const kColor color) const;
        inline bool operator!=(const kColor color) const;
        inline bool operator<(const kColor color) const;

        // make kColor object from HSL values
        //      in integer ranges (0 - 360 for hue and 0 - 100 for saturation and lightness )
        static inline kColor fromHSL(int hue, int saturation, int lightness);
        //      in float ranges (all values from 0 to 1)
        static kColor fromHSL(kScalar hue, kScalar saturation, kScalar lightness);

        // predefined colors constant values
        static const kColor Black;
        static const kColor Gray;
        static const kColor LtGray;
        static const kColor White;
    };

    // kColorReal
    //     Primary type for passing colors to canvas API is kColor
    //     if you need to use floating point color, kColorReal is a handy way to
    //     implicitly convert between fp and fixed color
    struct kColorReal
    {
        kScalar r, g, b, a;

        inline kColorReal();
        inline kColorReal(const kColor color);
        inline kColorReal(float _r, float _g, float _b, float _a = 1.0f);

        inline operator kColor() const;
    };

    // kGradientStop
    //      defines gradient color at certain stop point
    struct kGradientStop
    {
        kColor color;
        kScalar position;

        kGradientStop(const kColor _color, kScalar _position) :
            color(_color),
            position(_position)
        {}
    };

    // simple text rendering structs
    struct kFontMetrics
    {
        kScalar ascent;
        kScalar descent;
        kScalar height;
        kScalar linegap;
        kScalar underlinepos;
        kScalar underlinewidth;
        kScalar strikethroughpos;
        kScalar strikethroughwidth;
    };

    struct kGlyphMetrics
    {
        kScalar a;
        kScalar b;
        kScalar c;
    };

    struct kTextSizeProperties
    {
    };

    struct kTextOutProperties
    {
    };



    // Implementation for inline funcs

    kColor::kColor() :
        r(0), g(0), b(0), a(255)
    {}

    kColor::kColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) :
        r(_r), g(_g), b(_b), a(_a)
    {}

    kColor kColor::inverse() const
    {
        return kColor(~r, ~g, ~b, a);
    }

    bool kColor::operator==(const kColor color) const
    {
        return *reinterpret_cast<const uint32_t*>(this) ==
               *reinterpret_cast<const uint32_t*>(&color);
    }

    bool kColor::operator!=(const kColor color) const
    {
        return *reinterpret_cast<const uint32_t*>(this) !=
               *reinterpret_cast<const uint32_t*>(&color);
    }

    bool kColor::operator<(const kColor color) const
    {
        return *reinterpret_cast<const uint32_t*>(this) <
               *reinterpret_cast<const uint32_t*>(&color);
    }

    kColor kColor::fromHSL(int hue, int saturation, int lightness)
    {
        return fromHSL(
            kScalar(hue) / kScalar(360),
            kScalar(saturation) / kScalar(100),
            kScalar(lightness) / kScalar(100)
        );
    }


    #define FCS kScalar(1.0 / 255.0)

    kColorReal::kColorReal() :
        r(0), g(0), b(0), a(1)
    {}

    kColorReal::kColorReal(const kColor color) :
        r(color.r * FCS),
        g(color.g * FCS),
        b(color.b * FCS),
        a(color.a * FCS)
    {}

    kColorReal::kColorReal(kScalar _r, kScalar _g, kScalar _b, kScalar _a) :
        r(_r), g(_g), b(_b), a(_a)
    {}

    kColorReal::operator kColor() const
    {
        return kColor(
            c_util::roundint(r * 255.0f),
            c_util::roundint(g * 255.0f),
            c_util::roundint(b * 255.0f),
            c_util::roundint(a * 255.0f)
        );
    }

    #undef FCS

} // namespace k_canvas
