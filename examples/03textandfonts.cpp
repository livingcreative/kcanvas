/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    03textandfonts.cpp
        Text and fonts example
*/

#include "kcanvas/canvas.h"


using namespace k_canvas;


const char *TITLE = "kcanvas - Text and fonts example";


void Initialize()
{}

void Example(kCanvas &canvas)
{
    const char *text1 = "Text example";
    const char *text2 = "Test\nline\nbreak";
    const char *text3 = "Text example with  line breaks\n\nand word wrapping";

    kFont bigfont("Tahoma", 30, kFontStyle::Italic);
    kFont normal("Times New Roman", 12);
    kFont bold("Times New Roman", 12, kFontStyle::Bold);
    kFont italic("Times New Roman", 12, kFontStyle::Italic);
    kFont underline("Times New Roman", 12, kFontStyle::Underline);
    kFont strike("Times New Roman", 12, kFontStyle::Strikethrough);

    kGradient gradient(kColor(130, 218, 255), kColor(63, 72, 204));
    kBrush brush(kPoint(10, 10), kPoint(210, 190), gradient);
    canvas.Text(kPoint(10, 10), text1, -1, bigfont, brush);

    kRect bounds;
    kSize sz = canvas.TextSize(text1, -1, bigfont, nullptr, &bounds);

    kPen pen1(kColor(130, 218, 255), 1, kStrokeStyle::Dot);
    kPen pen2(kColor(255, 218, 130), 1, kStrokeStyle::Dot);
    canvas.Rectangle(kRect(kPoint(10, 10) + 0.5f, sz), &pen1, nullptr);
    canvas.Rectangle(bounds + (kPoint(10, 10) + 0.5f), &pen2, nullptr);

    canvas.Text(kPoint(320, 130), text2, -1, normal, brush);

    kFontMetrics m;
    canvas.GetFontMetrics(bigfont, m);
    canvas.Line(
        kPoint(10, 10 + m.ascent),
        kPoint(10 + sz.width, 10 + m.ascent), pen1
    );
    canvas.Line(
        kPoint(10, 10 + m.ascent - m.xheight),
        kPoint(10 + sz.width, 10 + m.ascent - m.xheight), pen1
    );
    canvas.Line(
        kPoint(10, 10 + m.ascent - m.capheight),
        kPoint(10 + sz.width, 10 + m.ascent - m.capheight), pen1
    );

    canvas.Text(kPoint(320, 10), "Normal", -1, normal, brush);
    canvas.Text(kPoint(320, 30), "Bold", -1, bold, brush);
    canvas.Text(kPoint(320, 50), "Italic", -1, italic, brush);
    canvas.Text(kPoint(320, 70), "Underline", -1, underline, brush);
    canvas.Text(kPoint(320, 90), "Strikethrough", -1, strike, brush);


    kRect textrect(10, 80, 300, 260);
    canvas.Rectangle(textrect, &pen1, nullptr);

    kTextOutProperties op = kTextOutProperties::construct(
        kTextFlags::IgnoreLineBreaks |
        kTextFlags::Multiline        |
        kTextFlags::MergeSpaces      |
        kTextFlags::Ellipses
    );
    canvas.Text(textrect, text3, -1, bigfont, brush, &op);
}

void Shutdown()
{}
