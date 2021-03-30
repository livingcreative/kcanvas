# kcanvas
Yet another 2D API abstraction  

[![Build status](https://ci.appveyor.com/api/projects/status/x8slen7egsu31ynm?svg=true)](https://ci.appveyor.com/project/livingcreative/kcanvas)
[![Build status](https://travis-ci.org/livingcreative/kcanvas.svg?branch=master)](https://travis-ci.org/livingcreative/kcanvas)

## What is it
kcanvas is easy to use C++ API for rendering 2D graphics. Its main purpose is to make 2D graphics
programming easier and hide nasty details of platform specific graphics APIs. It's crossplatform
and has native implementation for supported platforms (Direct2D on windows, Quartz on Mac OS X,
Cairo on Linux). It's suitable for rendering 2D graphics such as user interfaces, data
visualization, diagrams and other text and visual content. It's not intended to be used in
interactive games, however simple 2D game could be rendered with kcanvas API.
> This project is under development now, so not all of its features are implemented and   
>  not all platforms are fully supported

[Windows demo and examples binaries](https://raw.githubusercontent.com/wiki/livingcreative/kcanvas/data/kcanvasexamples_win.zip)

## What it does
kcanvas API is able to render 2D vector and raster graphics. You can stroke and fill different simple and complex shapes with many different styles, render text with different fonts available in your system. Here is sample image which shows what you can render with help of kcanvas API:   
![kcanvas demo image](https://raw.githubusercontent.com/livingcreative/kcanvas/master/demo.jpg)

**Features**
* Commonly used predefined shapes (line, bezier, arc, rectangle, rounded rectangle, ellipse, poly lines and polygons)
* Shapes of arbitrary complexity
* Many different line stroke styles, line join styles
* Several fill styles (solid, linear and radial gradients, bitmap)
* Full alpha transparency support
* Clipping to shapes and masking by grayscale images, mask fill with brush
* Bitmap rendering (with alpha channel support)
* Arbitrary transformation stack (via 3x2 matrix)
* Text measurement and rendering (with simple layouts)

**Ongoing development**   
The [Roadmap wiki page](https://github.com/livingcreative/kcanvas/wiki/Roadmap) contains some 
information about current project status and ongoing feature support and development plans.

## How it works
In kcanvas API all of painting functions tied to `kCanvas` class. `kCanvas` object can't be
instantiated directly, you need to use specific implementation to direct painting to specific
device or `kBitmap` object. You can use `kContextCanvas` to paint into window and
`kBitmapCanvas` to paint into `kBitmap` object.

Most of the properties for painting operation are taken from special resource objects, in general
kcanvas API is stateless with few exceptions. Resource objects are immutable – once created their
properties can't be changed. Some of resource objects can't be copied (`kBitmap` and
`kGradient`) and always remains unique.
* `kStroke` object holds properties for line style. It defines line pattern, line join style and line caps style.
* `kGradient` object holds color gradient definition.
* `kPen` object holds properties for stroke (outline painting) operations, it incapsulates `kStroke` and `kBrush` objects.
* `kBrush` object holds properties for fill operations.
* `kFont` object holds font properties for text painting (and measuring).
* `kPath` object holds geometric shape data.
* `kBitmap` object holds raster image data.

See reference wiki pages for full
kcanvas API [types](https://github.com/livingcreative/kcanvas/wiki/Basic-Data-Types) and [classes](https://github.com/livingcreative/kcanvas/wiki/Public-API-Classes) reference.   
See [guide wiki page](https://github.com/livingcreative/kcanvas/wiki/Guide) for general kcanvas
concepts and usage.

Under the hood kcanvas API uses one of existing platform specific APIs, such as Direct2D on
Windows platform.

## How to use
Having [proper setup](https://github.com/livingcreative/kcanvas/wiki/Guide#setting-up-project-to-use-kcanvas)
of your project for using kcanvas library (correct path to kcanvas include directory and
kcanvas library linked into project) you only need to include `kcanvas/canvas.h` header in
your source to get all kcanvas API stuff. To paint something you need to instantiate
`kContextCanvas` object and issue painting commands.

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

Please visit [examples wiki page](https://github.com/livingcreative/kcanvas/wiki/Examples) for
additional information and usage examples.

## How to build
Please visit [guide wiki page](https://github.com/livingcreative/kcanvas/wiki/Guide) for build
instructions.

## Copyright and licensing
kcanvas 2D Graphics Library

Copyright (C) 2015 – 2017, livingcreative (https://github.com/livingcreative)   
All rights reserved.

Redistribution and use in source and/or binary forms, with or without 
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* The name of the author may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"   
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF   
THE POSSIBILITY OF SUCH DAMAGE.

[License file](https://raw.githubusercontent.com/livingcreative/kcanvas/master/license.txt)
