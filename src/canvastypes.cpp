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


const kColor kColor::Black = kColor();
const kColor kColor::Gray = kColor(127, 127, 127);
const kColor kColor::LtGray = kColor(192, 192, 192);
const kColor kColor::White = kColor(255, 255, 255);


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
