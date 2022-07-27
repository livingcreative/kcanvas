/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    canvastypes.h
        basic canvas types used both in public include and internally in source
        by default it uses c_util.h basic types
*/

#pragma once
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cfloat>
#include <limits>
#include "kcommon/c_util.h"     // common utility funcs and types
#include "kcommon/c_geometry.h" // common geometric functions and types
#include "canvasplatform.h"     // implementation platform support


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

    struct kRectInt : public c_util::rectT<int>
    {
        using c_util::rectT<int>::rectT;
    };

    // basic types derived from c_geometry types
    typedef c_geometry::mat3x2<kScalar> kTransform;

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
        enum Style
        {
            Normal        = 0,
            Bold          = 0x01,
            Italic        = 0x02,
            Underline     = 0x04,
            Strikethrough = 0x08
        };

        kFontStyle() : p_value() {}
        kFontStyle(Style value) : p_value(value) {}
        kFontStyle(uint32_t value) : p_value(value) {}
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
        inline kColor(const kColor source, uint8_t _a);
        inline kColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255);

        inline kColor inverse() const;

        inline bool operator==(const kColor color) const;
        inline bool operator!=(const kColor color) const;
        inline bool operator<(const kColor color) const;

        // make kColor object from HSL values
        //      in integer ranges (0 - 360 for hue and 0 - 100 for saturation and lightness)
        static inline kColor fromHSL(int hue, int saturation, int lightness);
        //      in float ranges (all values from 0 to 1)
        static kColor fromHSL(kScalar hue, kScalar saturation, kScalar lightness);

        // predefined colors constant values
        static const kColor Pink;                 // 255 192 203
        static const kColor LightPink;            // 255 182 193
        static const kColor HotPink;              // 255 105 180
        static const kColor DeepPink;             // 255  20 147
        static const kColor PaleVioletRed;        // 219 112 147
        static const kColor MediumVioletRed;      // 199  21 133
        static const kColor LightSalmon;          // 255 160 122
        static const kColor Salmon;               // 250 128 114
        static const kColor DarkSalmon;           // 233 150 122
        static const kColor LightCoral;           // 240 128 128
        static const kColor IndianRed;            // 205  92  92
        static const kColor Crimson;              // 220  20  60
        static const kColor FireBrick;            // 178  34  34
        static const kColor DarkRed;              // 139   0   0
        static const kColor Red;                  // 255   0   0
        static const kColor OrangeRed;            // 255  69   0
        static const kColor Tomato;               // 255  99  71
        static const kColor Coral;                // 255 127  80
        static const kColor DarkOrange;           // 255 140   0
        static const kColor Orange;               // 255 165   0
        static const kColor Yellow;               // 255 255   0
        static const kColor LightYellow;          // 255 255 224
        static const kColor LemonChiffon;         // 255 250 205
        static const kColor LightGoldenrodYellow; // 250 250 210
        static const kColor PapayaWhip;           // 255 239 213
        static const kColor Moccasin;             // 255 228 181
        static const kColor PeachPuff;            // 255 218 185
        static const kColor PaleGoldenrod;        // 238 232 170
        static const kColor Khaki;                // 240 230 140
        static const kColor DarkKhaki;            // 189 183 107
        static const kColor Gold;                 // 255 215   0
        static const kColor Cornsilk;             // 255 248 220
        static const kColor BlanchedAlmond;       // 255 235 205
        static const kColor Bisque;               // 255 228 196
        static const kColor NavajoWhite;          // 255 222 173
        static const kColor Wheat;                // 245 222 179
        static const kColor BurlyWood;            // 222 184 135
        static const kColor Tan;                  // 210 180 140
        static const kColor RosyBrown;            // 188 143 143
        static const kColor SandyBrown;           // 244 164  96
        static const kColor Goldenrod;            // 218 165  32
        static const kColor DarkGoldenrod;        // 184 134  11
        static const kColor Peru;                 // 205 133  63
        static const kColor Chocolate;            // 210 105  30
        static const kColor SaddleBrown;          // 139  69  19
        static const kColor Sienna;               // 160  82  45
        static const kColor Brown;                // 165  42  42
        static const kColor Maroon;               // 128   0   0
        static const kColor DarkOliveGreen;       //  85 107  47
        static const kColor Olive;                // 128 128   0
        static const kColor OliveDrab;            // 107 142  35
        static const kColor YellowGreen;          // 154 205  50
        static const kColor LimeGreen;            //  50 205  50
        static const kColor Lime;                 //   0 255   0
        static const kColor LawnGreen;            // 124 252   0
        static const kColor Chartreuse;           // 127 255   0
        static const kColor GreenYellow;          // 173 255  47
        static const kColor SpringGreen;          //   0 255 127
        static const kColor MediumSpringGreen;    //   0 250 154
        static const kColor LightGreen;           // 144 238 144
        static const kColor PaleGreen;            // 152 251 152
        static const kColor DarkSeaGreen;         // 143 188 143
        static const kColor MediumSeaGreen;       //  60 179 113
        static const kColor SeaGreen;             //  46 139  87
        static const kColor ForestGreen;          //  34 139  34
        static const kColor Green;                //   0 128   0
        static const kColor DarkGreen;            //   0 100   0
        static const kColor MediumAquamarine;     // 102 205 170
        static const kColor Aqua;                 //   0 255 255
        static const kColor Cyan;                 //   0 255 255
        static const kColor LightCyan;            // 224 255 255
        static const kColor PaleTurquoise;        // 175 238 238
        static const kColor Aquamarine;           // 127 255 212
        static const kColor Turquoise;            //  64 224 208
        static const kColor MediumTurquoise;      //  72 209 204
        static const kColor DarkTurquoise;        //   0 206 209
        static const kColor LightSeaGreen;        //  32 178 170
        static const kColor CadetBlue;            //  95 158 160
        static const kColor DarkCyan;             //   0 139 139
        static const kColor Teal;                 //   0 128 128
        static const kColor LightSteelBlue;       // 176 196 222
        static const kColor PowderBlue;           // 176 224 230
        static const kColor LightBlue;            // 173 216 230
        static const kColor SkyBlue;              // 135 206 235
        static const kColor LightSkyBlue;         // 135 206 250
        static const kColor DeepSkyBlue;          //   0 191 255
        static const kColor DodgerBlue;           //  30 144 255
        static const kColor CornflowerBlue;       // 100 149 237
        static const kColor SteelBlue;            //  70 130 180
        static const kColor RoyalBlue;            //  65 105 225
        static const kColor Blue;                 //   0   0 255
        static const kColor MediumBlue;           //   0   0 205
        static const kColor DarkBlue;             //   0   0 139
        static const kColor Navy;                 //   0   0 128
        static const kColor MidnightBlue;         //  25  25 112
        static const kColor Lavender;             // 230 230 250
        static const kColor Thistle;              // 216 191 216
        static const kColor Plum;                 // 221 160 221
        static const kColor Violet;               // 238 130 238
        static const kColor Orchid;               // 218 112 214
        static const kColor Fuchsia;              // 255   0 255
        static const kColor Magenta;              // 255   0 255
        static const kColor MediumOrchid;         // 186  85 211
        static const kColor MediumPurple;         // 147 112 219
        static const kColor BlueViolet;           // 138  43 226
        static const kColor DarkViolet;           // 148   0 211
        static const kColor DarkOrchid;           // 153  50 204
        static const kColor DarkMagenta;          // 139   0 139
        static const kColor Purple;               // 128   0 128
        static const kColor Indigo;               //  75   0 130
        static const kColor DarkSlateBlue;        //  72  61 139
        static const kColor RebeccaPurple;        // 102  51 153
        static const kColor SlateBlue;            // 106  90 205
        static const kColor MediumSlateBlue;      // 123 104 238
        static const kColor White;                // 255 255 255
        static const kColor Snow;                 // 255 250 250
        static const kColor Honeydew;             // 240 255 240
        static const kColor MintCream;            // 245 255 250
        static const kColor Azure;                // 240 255 255
        static const kColor AliceBlue;            // 240 248 255
        static const kColor GhostWhite;           // 248 248 255
        static const kColor WhiteSmoke;           // 245 245 245
        static const kColor Seashell;             // 255 245 238
        static const kColor Beige;                // 245 245 220
        static const kColor OldLace;              // 253 245 230
        static const kColor FloralWhite;          // 255 250 240
        static const kColor Ivory;                // 255 255 240
        static const kColor AntiqueWhite;         // 250 235 215
        static const kColor Linen;                // 250 240 230
        static const kColor LavenderBlush;        // 255 240 245
        static const kColor MistyRose;            // 255 228 225
        static const kColor Gainsboro;            // 220 220 220
        static const kColor LightGrey;            // 211 211 211
        static const kColor Silver;               // 192 192 192
        static const kColor DarkGray;             // 169 169 169
        static const kColor Gray;                 // 128 128 128
        static const kColor DimGray;              // 105 105 105
        static const kColor LightSlateGray;       // 119 136 153
        static const kColor SlateGray;            // 112 128 144
        static const kColor DarkSlateGray;        //  47  79  79
        static const kColor Black;                //   0   0   0
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

        kGradientStop() :
            color(),
            position(0)
        {}

        kGradientStop(const kColor _color, kScalar _position) :
            color(_color),
            position(_position)
        {}
    };

    // kFontMetrics
    //      whole font metrics
    struct kFontMetrics
    {
        kScalar ascent;             // distance between baseline and bounding box top
        kScalar descent;            // distance between baseline and bounding box bottom
        kScalar height;             // total font height (typically == ascent + descent)
        kScalar linegap;            // recommended interval between lines of text

        kScalar underlinepos;       // underline line position relative to baseline (TODO: think of its value sign)
        kScalar underlinewidth;     // underline line width
        kScalar strikethroughpos;   // strikethrough line position relative to baseline (TODO: think of its value sign)
        kScalar strikethroughwidth; // strikethrough line width

        kScalar capheight;          // OPTIONAL (can be not set, == 0) captial letter height (H or M blackbox height)
        kScalar xheight;            // OPTIONAL (can be not set, == 0) lowercase letter height (x letter height)
    };

    // kGlyphMetrics
    //      metrics of single font glyph
    struct kGlyphMetrics
    {
        kScalar leftbearing;
        kScalar advance;
        kScalar rightbearing;
    };

    // kTextFlags flags for text service measure & text output functions
    //     NOTE: there is actually some copy-paste from kFontStyle
    //           declaration, think about simplifying it
    struct kTextFlags
    {
        enum Flags
        {
            Multiline        = 0x01,
            IgnoreLineBreaks = 0x02,
            MergeSpaces      = 0x04,
            UseTabs          = 0x08,
            StrictBounds     = 0x10,
            ClipToBounds     = 0x20,
            Ellipses         = 0x40
        };

        kTextFlags() : p_value() {}
        kTextFlags(Flags value) : p_value(value) {}
        kTextFlags(uint32_t value) : p_value(value) {}
        operator uint32_t() const { return p_value; }
        kTextFlags operator=(const Flags value) { p_value = value; return *this; }
        kTextFlags operator=(const uint32_t value) { p_value = value; return *this; }

    private:
        uint32_t p_value;
    };

    // kTextPropertiesBase
    //      common properties for text measurement and output
    //      not used directly by API
    struct kTextPropertiesBase
    {
        kTextFlags flags;
        kScalar    interval;
        kScalar    indent;
        kScalar    defaulttabwidth;
    };

    // kTextSizeProperties
    //      properties for text measurement
    struct kTextSizeProperties : public kTextPropertiesBase
    {
        kSize bounds;

        static kTextSizeProperties construct(
            kTextFlags flags = 0,
            kScalar interval = 0, kScalar indent = 0, kScalar defaulttabwidth = 0,
            const kSize &bounds = kSize()
        )
        {
            kTextSizeProperties result;
            result.flags           = flags;
            result.interval        = interval;
            result.indent          = indent;
            result.defaulttabwidth = defaulttabwidth;
            result.bounds          = bounds;

            return result;
        }
    };

    // kTextHorizontalAlignment
    //      defines horizontal alignment for box text rendering
    enum class kTextHorizontalAlignment
    {
        Left    = 0,
        Center  = 1,
        Right   = 2,
        Justify = 3
    };

    // kTextVerticalAlignment
    //      defines vertical alignment for box text rendering
    enum class kTextVerticalAlignment
    {
        Top    = 0,
        Middle = 1,
        Bottom = 2
    };

    // kTextOutProperties
    //      properties for text rendering
    struct kTextOutProperties : public kTextPropertiesBase
    {
        kTextHorizontalAlignment horzalign;
        kTextVerticalAlignment   vertalign;

        static kTextOutProperties construct(
            kTextFlags flags = 0,
            kScalar interval = 0, kScalar indent = 0, kScalar defaulttabwidth = 0,
            kTextHorizontalAlignment horzalign = kTextHorizontalAlignment::Left,
            kTextVerticalAlignment vertalign = kTextVerticalAlignment::Top
        )
        {
            kTextOutProperties result;
            result.flags           = flags;
            result.interval        = interval;
            result.indent          = indent;
            result.defaulttabwidth = defaulttabwidth;
            result.horzalign       = horzalign;
            result.vertalign       = vertalign;

            return result;
        }
    };


    // forwards
    class kCanvas;
    class kBitmap;
    class kPath;


    /*
     -------------------------------------------------------------------------------
     kCanvasClipper
     -------------------------------------------------------------------------------
        helper object for "safe" clipped drawing within {} block
        clip state is valid while clipper object is alive,
        when clipper object destructed (or goes out of scope) clipping state gets
        restored back to the previous one
    */
    class kCanvasClipper
    {
    public:
        // begin bitmap masked painting
        kCanvasClipper(kCanvas &canvas, const kBitmap &mask, const kTransform &transform = kTransform(), kExtendType xextend = kExtendType::Clamp, kExtendType yextend = kExtendType::Clamp);

        // begin path clipped painting
        kCanvasClipper(kCanvas &canvas, const kPath &clip, const kTransform &transform = kTransform());

        // begin rectangle clipped painting
        kCanvasClipper(kCanvas &canvas, const kRect &clip);

        ~kCanvasClipper();

        // this type of object can NOT be copied and reassigned to other
        kCanvasClipper(const kCanvasClipper &source) = delete;
        kCanvasClipper &operator=(const kCanvasClipper &source) = delete;

        // these two are for support "operator-like" clipper usage
        operator bool() const { return p_dummy == 0; }
        kCanvasClipper& operator++() { ++p_dummy; return *this; }

    private:
        kCanvas &p_canvas;
        int      p_dummy;
    };

    // helper macro to write clipping in handy c# "using" operator style
    // Example:
    //      kclip(canvas, kRect(0, 0, 100, 100)) {
    //          * clipped painting here *
    //      }
    //      * back to unclipped state *
    // use KCANVAS_HELPER_MACRO definition to enable this macro
    #ifdef KCANVAS_HELPER_MACRO
    #define kclip(...) for (kCanvasClipper __clipper(__VA_ARGS__); __clipper; ++__clipper)
    #endif // !KCANVAS_NO_HELPER_MACRO


    /*
     -------------------------------------------------------------------------------
     kCanvasTransformer
     -------------------------------------------------------------------------------
        helper object for "safe" tranformed drawing within {} block
        automatically calls PopTransform when object goes out of scope
    */
    class kCanvasTransformer
    {
    public:
        kCanvasTransformer(kCanvas &canvas, const kTransform &transform);
        ~kCanvasTransformer();

        // this type of object can NOT be copied and reassigned to other
        kCanvasTransformer(const kCanvasTransformer &source) = delete;
        kCanvasTransformer &operator=(const kCanvasTransformer &source) = delete;

    private:
        kCanvas &p_canvas;
    };



    // Implementation for inline funcs

    kColor::kColor() :
        r(0), g(0), b(0), a(255)
    {}

    kColor::kColor(const kColor source, uint8_t _a) :
        r(source.r), g(source.g), b(source.b), a(_a)
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
