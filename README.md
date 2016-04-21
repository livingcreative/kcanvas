# kcanvas
Yet another 2D API abstraction

## What is it
kcanvas is easy to use C++ API for rendering 2D graphics. Its main purpose is to make 2D graphics programming easier and hide nasty details of platform specific graphics APIs. It's crossplatform and has native implementation for supported platforms (Direct2D on windows, Quartz on Mac OS X, Cairo on Linux). It's suitable for rendering 2D graphics such as user interfaces, data visualization, diagrams and other text and visual content. It's not intended to be used in interactive games, however simple 2D game could be rendered with kcanvas API.
> This project is under development now, so not all of its features are implemented and not all   
> platforms are fully supported

## What it does
kcanvas API is able to render 2D vector and raster graphics. You can stroke and fill different simple and complex shapes with many different styles, render text with different fonts available in your system. Here is sample image which shows what you can render with help of kcanvas API:   
![kcanvas demo image](https://raw.githubusercontent.com/livingcreative/kcanvas/master/demo.jpg)

## How it works
In kcanvas API all of painting functions tied to `kCanvas` class. `kCanvas` object can't be instantiated directly, you need to use specific implementation to direct painting to specific device or `kBitmap` object. You can use `kContextCanvas` to paint into window and `kBitmapCanvas` to paint into `kBitmap` object.

Most of the properties for painting operation are taken from special resource objects, in general kcanvas API is stateless with few exceptions. Resource objects are immutable – once created their properties can't be changed. Some of resource objects can't be copied (`kBitmap` and `kGradient`) and always remains unique.
* `kStroke` object holds properties for line style. It defines line pattern, line join style and line caps style.
* `kGradient` object holds color gradient definition.
* `kPen` object holds properties for stroke (outline painting) operations, it incapsulates `kStroke` and `kBrush` objects.
* `kBrush` object holds properties for fill operations.
* `kFont` object holds font properties for text painting (and measuring).
* `kPath` object holds geometric shape data.
* `kBitmap` object holds raster image data.

See [reference wiki page](https://github.com/livingcreative/kcanvas/wiki/Reference) for full kcanvas API types and classes reference.

Under the hood kcanvas API uses one of existing platform specific APIs, such as Direct2D on Windows platform.

## How to use
Having proper setup of your project for using kcanvas library (correct path to kcanvas include directory and kcanvas library linked into project) you only need to include `kcanvas/canvas.h` header in your source to get all kcanvas API stuff. To paint something you need to instantiate `kContextCanvas` object and issue painting commands.

Here is quick "Hello world" example:
```c++
// DeviceContext is platform dependent context descriptor,
// such as HDC or CGContextRef
kContextCanvas canvas(DeviceContext);

// create some resource objects for painting

// orange solid pen, 4 pixels wide
kPen pen(kColor(255, 127, 39), 4);
// yellow solid brush
kBrush brush(kColor(255, 201, 14));
// black solid brush
kBrush black(kColor::Black);
// font
kFont font("Tahoma", 50, kFontStyle::Bold);

// text to paint
const char *text = "Hello world!";

// measure text to find out rectangle which can include this text
kSize textsize = canvas.TextSize(text, -1, font);

// paint rectangle with outline
canvas.Rectangle(kRect(kPoint(10), textsize + 20), &pen, &brush);

// paint text
canvas.Text(kPoint(20, 20), text, -1, font, black);
```
Result:   
![kcanvas hello world](https://raw.githubusercontent.com/wiki/livingcreative/kcanvas/images/helloworld.jpg)

Please visit [examples wiki page](https://github.com/livingcreative/kcanvas/wiki/Examples) for additional information and usage examples.
