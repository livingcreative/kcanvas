/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/kcanvas

    common/bmp.h
        Declarations for very simple BMP file loader

        This loader is specific ONLY for this project
        and sample bitmaps provided with this project, DO NOT use it as
        general bitmap loader anywhere else
*/


class BitmapData
{
public:
    BitmapData(const char *filename);
    ~BitmapData();

    const char* data() const { return p_data; }
    int width() const { return p_width; }
    int height() const { return p_height; }
    int pitch() const { return p_pitch; }
    int bpp()  const { return p_bpp; }

private:
    char *p_data;
    int   p_width;
    int   p_height;
    int   p_pitch;
    int   p_bpp;
};
