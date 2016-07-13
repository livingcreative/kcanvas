/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    canvasimpl.cpp
        basic API objects implementation interfaces implementation
*/

#include "canvasimpl.h"
#include "canvasimplplatform.h"


using namespace k_canvas;
using namespace impl;


/*
 -------------------------------------------------------------------------------
 kPathImplDefault implementation
 -------------------------------------------------------------------------------
*/

kPathImplDefault::kPathImplDefault() :
    p_commands(),
    p_curr_command(0),
    p_points(),
    p_curr_point(0),
    p_text(),
    p_curr_text(0)
{}

kPathImplDefault::~kPathImplDefault()
{}

kPathImplDefault::Command::Command() :
    font(nullptr)
{}

kPathImplDefault::Command::Command(CommandType _command, int _start_index, size_t _element_count, kResourceObject *_font) :
    command(_command),
    start_index(_start_index),
    element_count(_element_count),
    font(_font)
{}

kPathImplDefault::Command::~Command()
{
    if (font) {
        font->release();
    }
}

kPathImplDefault::Command& kPathImplDefault::Command::operator=(const Command &source)
{
    command = source.command;
    start_index = source.start_index;
    element_count = source.element_count;
    font = source.font;
    if (font) {
        font->addref();
    }
    return *this;
}

void kPathImplDefault::MoveTo(const kPoint &p)
{
    AddCommand(PC_MOVETO, 1);
    AddPoint(p);
}

void kPathImplDefault::LineTo(const kPoint &p)
{
    AddCommand(PC_LINETO, 1);
    AddPoint(p);
}

void kPathImplDefault::BezierTo(const kPoint &p1, const kPoint &p2, const kPoint &p3)
{
    AddCommand(PC_BEZIERTO, 3);
    AddPoint(p1);
    AddPoint(p2);
    AddPoint(p3);
}

void kPathImplDefault::PolyLineTo(const kPoint *points, size_t count)
{
    AddCommand(PC_POLYLINETO, count);
    while (count--) {
        AddPoint(*points++);
    }
}

void kPathImplDefault::PolyBezierTo(const kPoint *points, size_t count)
{
    AddCommand(PC_POLYBEZIERTO, count);
    while (count--) {
        AddPoint(*points++);
    }
}

void kPathImplDefault::Text(const char *text, int count, const kFontBase *font, kTextOrigin origin)
{
    if (p_curr_command + 1 > p_commands.size()) {
        p_commands.resize(p_commands.size() + 16);
    }

    p_commands[p_curr_command++] = Command(PC_TEXT, int(p_curr_text), 0, font->getResource());

    if (p_curr_text + 1 > p_text.size()) {
        p_text.resize(p_text.size() + 16);
    }

    p_text[p_curr_text++] = std::string(text);
}

void kPathImplDefault::Close()
{
    AddCommand(PC_CLOSE, 0);
}

void kPathImplDefault::Clear()
{
    p_curr_command = 0;
    p_curr_point = 0;
}

void kPathImplDefault::Commit()
{
}

void kPathImplDefault::AddCommand(CommandType command, size_t point_count)
{
    if (p_curr_command + 1 > p_commands.size()) {
        p_commands.resize(p_commands.size() + 16);
    }

    p_commands[p_curr_command++] = Command(command, int(p_curr_point), point_count);
}

void kPathImplDefault::AddPoint(const kPoint &point)
{
    if (p_curr_point + 1 > p_points.size()) {
        p_points.resize(p_points.size() + 16);
    }

    p_points[p_curr_point++] = point;
}



/*
 -------------------------------------------------------------------------------
 kCanvasImpl implementation
 -------------------------------------------------------------------------------
*/

kCanvasImpl::~kCanvasImpl()
{}



/*
 -------------------------------------------------------------------------------
 CanvasFactory implementation
 -------------------------------------------------------------------------------
*/

CanvasFactory *CanvasFactory::factory = nullptr;
Impl           CanvasFactory::current_impl = IMPL_NONE;

CanvasFactory::CanvasFactory()
{
}

CanvasFactory::~CanvasFactory()
{
}

void CanvasFactory::setImpl(Impl impl)
{
    if (factory && impl != current_impl) {
        // TODO
    }

    current_impl = impl;
}

Impl CanvasFactory::getImpl()
{
    return current_impl;
}

kGradientImpl* CanvasFactory::CreateGradient()
{
    return getFactory()->CreateGradientImpl();
}

kPathImpl* CanvasFactory::CreatePath()
{
    return getFactory()->CreatePathImpl();
}

kBitmapImpl* CanvasFactory::CreateBitmap()
{
    return getFactory()->CreateBitmapImpl();
}

kCanvasImpl* CanvasFactory::CreateCanvas()
{
    return getFactory()->CreateCanvasImpl();
}

CanvasFactory* CanvasFactory::getFactory()
{
    const CanvasImplDesc *desc = factory_descriptors;
    while (!factory) {
        if (desc->implementation == IMPL_NONE) {
            break;
        }

        factory = desc->createproc();
        if (factory->initialized()) {
            current_impl = desc->implementation;
        } else {
            delete factory;
            factory = nullptr;
        }

        ++desc;
    }

    return factory;
}

void CanvasFactory::destroyFactory()
{
    if (factory) {
        factory->destroyResources();
        delete factory;
        factory = nullptr;
    }
}
