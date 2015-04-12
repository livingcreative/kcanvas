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
#include "kcommon/c_util.h"
#include "canvasplatform.h"
#include <cstdint>


namespace k_canvas
{
    // base scalar type (float is default)
    typedef float Scalar;

    // basic types derived from c_util types
    typedef c_util::pointT<Scalar> kPoint;
    typedef c_util::sizeT<Scalar> kSize;
    typedef c_util::rectT<Scalar> kRect;
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

    enum class kCapStyle
    {
        Flat   = 0,
        Square = 1,
        Round  = 2
    };

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
            Bold      = 0x01,
            Italic    = 0x02,
            Underline = 0x04
        };

        kFontStyle() : p_value() {}
        kFontStyle(Style value) : p_value(value) {}
        operator uint32_t() const { return p_value; }
        kFontStyle operator=(const Style value) { p_value = value; return *this; }
        kFontStyle operator=(const uint32_t value) { p_value = value; return *this; }

    private:
        uint32_t p_value;
    };

    // kBitmap formats
    enum class kBitmapFormat
    {
        Color32BitAlphaPremultiplied = 1,
        Mask8Bit                     = 2
    };

    // kColor
    struct kColor
    {
        uint8_t r, g, b, a;

        inline kColor();
        inline kColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255);

        inline kColor inverse() const;

        inline bool operator==(const kColor color) const;
        inline bool operator!=(const kColor color) const;
        inline bool operator<(const kColor color) const;

        static inline kColor fromHSL(int hue, int saturation, int lightness);
        static inline kColor fromHSL(Scalar hue, Scalar saturation, Scalar lightness);

        static const kColor Black;
        static const kColor Gray;
        static const kColor LtGray;
        static const kColor White;
    };

    // kColorReal
    //     Primary type for passing colors to canvas API is kColor
    //     if you need to use floating point color kColorReal is a handy way to
    //     implicitly convert between fp and fixed color
    struct kColorReal
    {
        Scalar r, g, b, a;

        inline kColorReal();
        inline kColorReal(const kColor color);
        inline kColorReal(float _r, float _g, float _b, float _a = 1.0f);

        inline operator kColor() const;
    };

    struct kGradientStop
    {
        kColor color;
        Scalar position;

        kGradientStop(const kColor _color, Scalar _position) :
            color(_color),
            position(_position)
        {}
    };

    // simple text rendering structs
    struct kTextSizeProperties
    {
    };

    struct kTextOutProperties
    {
    };



    // IMPLEMENTATION

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
            Scalar(hue) / Scalar(360),
            Scalar(saturation) / Scalar(100),
            Scalar(lightness) / Scalar(100)
        );
    }

    kColor kColor::fromHSL(Scalar hue, Scalar saturation, Scalar lightness)
    {
        const Scalar one_third = Scalar(1.0 / 3.0);
        const Scalar two_thirds = Scalar(2.0 / 3.0);
        const Scalar one_sixth = Scalar(1.0 / 6.0);

        Scalar q = lightness < Scalar(0.5) ?
                   lightness * (Scalar(1.0) + saturation) :
                   lightness + saturation - lightness * saturation;
        Scalar p = Scalar(2.0) * lightness - q;

        float t[3] = {hue + one_third, hue, hue - one_third};
        for (int n = 0; n < 3; n++) {
            if (t[n] < Scalar(0)) {
                t[n] += Scalar(1);
            }
            if (t[n] > Scalar(1)) {
                t[n] -= Scalar(1);
            }
        }

        float rgb[3];
        for (int n = 0; n < 3; n++) {
            if (t[n] < one_sixth) {
                rgb[n] = p + (q - p) * Scalar(6.0) * t[n];
            } else if (t[n] < Scalar(0.5)) {
                rgb[n] = q;
            } else if (t[n] < two_thirds) {
                rgb[n] = p + (q - p) * (two_thirds - t[n]) * Scalar(6.0);
            } else {
                rgb[n] = p;
            }
        }

        return kColor(
            c_util::roundint(rgb[0] * 255.0f),
            c_util::roundint(rgb[1] * 255.0f),
            c_util::roundint(rgb[2] * 255.0f)
        );
    }



    #define FCS Scalar(1.0 / 255.0)

    kColorReal::kColorReal() :
        r(0), g(0), b(0), a(1)
    {}

    kColorReal::kColorReal(const kColor color) :
        r(color.r * FCS),
        g(color.g * FCS),
        b(color.b * FCS),
        a(color.a * FCS)
    {}

    kColorReal::kColorReal(Scalar _r, Scalar _g, Scalar _b, Scalar _a) :
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
