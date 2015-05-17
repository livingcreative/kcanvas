/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    canvastypes.cpp
        canvas types implementation
*/

#include "canvastypes.h"

using namespace k_canvas;


const kColor kColor::Pink                 = kColor(255, 192, 203);
const kColor kColor::LightPink            = kColor(255, 182, 193);
const kColor kColor::HotPink              = kColor(255, 105, 180);
const kColor kColor::DeepPink             = kColor(255,  20, 147);
const kColor kColor::PaleVioletRed        = kColor(219, 112, 147);
const kColor kColor::MediumVioletRed      = kColor(199,  21, 133);
const kColor kColor::LightSalmon          = kColor(255, 160, 122);
const kColor kColor::Salmon               = kColor(250, 128, 114);
const kColor kColor::DarkSalmon           = kColor(233, 150, 122);
const kColor kColor::LightCoral           = kColor(240, 128, 128);
const kColor kColor::IndianRed            = kColor(205,  92,  92);
const kColor kColor::Crimson              = kColor(220,  20,  60);
const kColor kColor::FireBrick            = kColor(178,  34,  34);
const kColor kColor::DarkRed              = kColor(139,   0,   0);
const kColor kColor::Red                  = kColor(255,   0,   0);
const kColor kColor::OrangeRed            = kColor(255,  69,   0);
const kColor kColor::Tomato               = kColor(255,  99,  71);
const kColor kColor::Coral                = kColor(255, 127,  80);
const kColor kColor::DarkOrange           = kColor(255, 140,   0);
const kColor kColor::Orange               = kColor(255, 165,   0);
const kColor kColor::Yellow               = kColor(255, 255,   0);
const kColor kColor::LightYellow          = kColor(255, 255, 224);
const kColor kColor::LemonChiffon         = kColor(255, 250, 205);
const kColor kColor::LightGoldenrodYellow = kColor(250, 250, 210);
const kColor kColor::PapayaWhip           = kColor(255, 239, 213);
const kColor kColor::Moccasin             = kColor(255, 228, 181);
const kColor kColor::PeachPuff            = kColor(255, 218, 185);
const kColor kColor::PaleGoldenrod        = kColor(238, 232, 170);
const kColor kColor::Khaki                = kColor(240, 230, 140);
const kColor kColor::DarkKhaki            = kColor(189, 183, 107);
const kColor kColor::Gold                 = kColor(255, 215,   0);
const kColor kColor::Cornsilk             = kColor(255, 248, 220);
const kColor kColor::BlanchedAlmond       = kColor(255, 235, 205);
const kColor kColor::Bisque               = kColor(255, 228, 196);
const kColor kColor::NavajoWhite          = kColor(255, 222, 173);
const kColor kColor::Wheat                = kColor(245, 222, 179);
const kColor kColor::BurlyWood            = kColor(222, 184, 135);
const kColor kColor::Tan                  = kColor(210, 180, 140);
const kColor kColor::RosyBrown            = kColor(188, 143, 143);
const kColor kColor::SandyBrown           = kColor(244, 164,  96);
const kColor kColor::Goldenrod            = kColor(218, 165,  32);
const kColor kColor::DarkGoldenrod        = kColor(184, 134,  11);
const kColor kColor::Peru                 = kColor(205, 133,  63);
const kColor kColor::Chocolate            = kColor(210, 105,  30);
const kColor kColor::SaddleBrown          = kColor(139,  69,  19);
const kColor kColor::Sienna               = kColor(160,  82,  45);
const kColor kColor::Brown                = kColor(165,  42,  42);
const kColor kColor::Maroon               = kColor(128,   0,   0);
const kColor kColor::DarkOliveGreen       = kColor( 85, 107,  47);
const kColor kColor::Olive                = kColor(128, 128,   0);
const kColor kColor::OliveDrab            = kColor(107, 142,  35);
const kColor kColor::YellowGreen          = kColor(154, 205,  50);
const kColor kColor::LimeGreen            = kColor( 50, 205,  50);
const kColor kColor::Lime                 = kColor(  0, 255,   0);
const kColor kColor::LawnGreen            = kColor(124, 252,   0);
const kColor kColor::Chartreuse           = kColor(127, 255,   0);
const kColor kColor::GreenYellow          = kColor(173, 255,  47);
const kColor kColor::SpringGreen          = kColor(  0, 255, 127);
const kColor kColor::MediumSpringGreen    = kColor(  0, 250, 154);
const kColor kColor::LightGreen           = kColor(144, 238, 144);
const kColor kColor::PaleGreen            = kColor(152, 251, 152);
const kColor kColor::DarkSeaGreen         = kColor(143, 188, 143);
const kColor kColor::MediumSeaGreen       = kColor( 60, 179, 113);
const kColor kColor::SeaGreen             = kColor( 46, 139,  87);
const kColor kColor::ForestGreen          = kColor( 34, 139,  34);
const kColor kColor::Green                = kColor(  0, 128,   0);
const kColor kColor::DarkGreen            = kColor(  0, 100,   0);
const kColor kColor::MediumAquamarine     = kColor(102, 205, 170);
const kColor kColor::Aqua                 = kColor(  0, 255, 255);
const kColor kColor::Cyan                 = kColor(  0, 255, 255);
const kColor kColor::LightCyan            = kColor(224, 255, 255);
const kColor kColor::PaleTurquoise        = kColor(175, 238, 238);
const kColor kColor::Aquamarine           = kColor(127, 255, 212);
const kColor kColor::Turquoise            = kColor( 64, 224, 208);
const kColor kColor::MediumTurquoise      = kColor( 72, 209, 204);
const kColor kColor::DarkTurquoise        = kColor(  0, 206, 209);
const kColor kColor::LightSeaGreen        = kColor( 32, 178, 170);
const kColor kColor::CadetBlue            = kColor( 95, 158, 160);
const kColor kColor::DarkCyan             = kColor(  0, 139, 139);
const kColor kColor::Teal                 = kColor(  0, 128, 128);
const kColor kColor::LightSteelBlue       = kColor(176, 196, 222);
const kColor kColor::PowderBlue           = kColor(176, 224, 230);
const kColor kColor::LightBlue            = kColor(173, 216, 230);
const kColor kColor::SkyBlue              = kColor(135, 206, 235);
const kColor kColor::LightSkyBlue         = kColor(135, 206, 250);
const kColor kColor::DeepSkyBlue          = kColor(  0, 191, 255);
const kColor kColor::DodgerBlue           = kColor( 30, 144, 255);
const kColor kColor::CornflowerBlue       = kColor(100, 149, 237);
const kColor kColor::SteelBlue            = kColor( 70, 130, 180);
const kColor kColor::RoyalBlue            = kColor( 65, 105, 225);
const kColor kColor::Blue                 = kColor(  0,   0, 255);
const kColor kColor::MediumBlue           = kColor(  0,   0, 205);
const kColor kColor::DarkBlue             = kColor(  0,   0, 139);
const kColor kColor::Navy                 = kColor(  0,   0, 128);
const kColor kColor::MidnightBlue         = kColor( 25,  25, 112);
const kColor kColor::Lavender             = kColor(230, 230, 250);
const kColor kColor::Thistle              = kColor(216, 191, 216);
const kColor kColor::Plum                 = kColor(221, 160, 221);
const kColor kColor::Violet               = kColor(238, 130, 238);
const kColor kColor::Orchid               = kColor(218, 112, 214);
const kColor kColor::Fuchsia              = kColor(255,   0, 255);
const kColor kColor::Magenta              = kColor(255,   0, 255);
const kColor kColor::MediumOrchid         = kColor(186,  85, 211);
const kColor kColor::MediumPurple         = kColor(147, 112, 219);
const kColor kColor::BlueViolet           = kColor(138,  43, 226);
const kColor kColor::DarkViolet           = kColor(148,   0, 211);
const kColor kColor::DarkOrchid           = kColor(153,  50, 204);
const kColor kColor::DarkMagenta          = kColor(139,   0, 139);
const kColor kColor::Purple               = kColor(128,   0, 128);
const kColor kColor::Indigo               = kColor( 75,   0, 130);
const kColor kColor::DarkSlateBlue        = kColor( 72,  61, 139);
const kColor kColor::RebeccaPurple        = kColor(102,  51, 153);
const kColor kColor::SlateBlue            = kColor(106,  90, 205);
const kColor kColor::MediumSlateBlue      = kColor(123, 104, 238);
const kColor kColor::White                = kColor(255, 255, 255);
const kColor kColor::Snow                 = kColor(255, 250, 250);
const kColor kColor::Honeydew             = kColor(240, 255, 240);
const kColor kColor::MintCream            = kColor(245, 255, 250);
const kColor kColor::Azure                = kColor(240, 255, 255);
const kColor kColor::AliceBlue            = kColor(240, 248, 255);
const kColor kColor::GhostWhite           = kColor(248, 248, 255);
const kColor kColor::WhiteSmoke           = kColor(245, 245, 245);
const kColor kColor::Seashell             = kColor(255, 245, 238);
const kColor kColor::Beige                = kColor(245, 245, 220);
const kColor kColor::OldLace              = kColor(253, 245, 230);
const kColor kColor::FloralWhite          = kColor(255, 250, 240);
const kColor kColor::Ivory                = kColor(255, 255, 240);
const kColor kColor::AntiqueWhite         = kColor(250, 235, 215);
const kColor kColor::Linen                = kColor(250, 240, 230);
const kColor kColor::LavenderBlush        = kColor(255, 240, 245);
const kColor kColor::MistyRose            = kColor(255, 228, 225);
const kColor kColor::Gainsboro            = kColor(220, 220, 220);
const kColor kColor::LightGrey            = kColor(211, 211, 211);
const kColor kColor::Silver               = kColor(192, 192, 192);
const kColor kColor::DarkGray             = kColor(169, 169, 169);
const kColor kColor::Gray                 = kColor(128, 128, 128);
const kColor kColor::DimGray              = kColor(105, 105, 105);
const kColor kColor::LightSlateGray       = kColor(119, 136, 153);
const kColor kColor::SlateGray            = kColor(112, 128, 144);
const kColor kColor::DarkSlateGray        = kColor( 47,  79,  79);
const kColor kColor::Black                = kColor(  0,   0,   0);


kColor kColor::fromHSL(kScalar hue, kScalar saturation, kScalar lightness)
{
    const kScalar one_third = kScalar(1.0 / 3.0);
    const kScalar two_thirds = kScalar(2.0 / 3.0);
    const kScalar one_sixth = kScalar(1.0 / 6.0);

    kScalar q = lightness < kScalar(0.5) ?
                lightness * (kScalar(1.0) + saturation) :
                lightness + saturation - lightness * saturation;
    kScalar p = kScalar(2.0) * lightness - q;

    float t[3] = {hue + one_third, hue, hue - one_third};
    for (int n = 0; n < 3; n++) {
        if (t[n] < kScalar(0)) {
            t[n] += kScalar(1);
        }
        if (t[n] > kScalar(1)) {
            t[n] -= kScalar(1);
        }
    }

    float rgb[3];
    for (int n = 0; n < 3; n++) {
        if (t[n] < one_sixth) {
            rgb[n] = p + (q - p) * kScalar(6.0) * t[n];
        } else if (t[n] < kScalar(0.5)) {
            rgb[n] = q;
        } else if (t[n] < two_thirds) {
            rgb[n] = p + (q - p) * (two_thirds - t[n]) * kScalar(6.0);
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
