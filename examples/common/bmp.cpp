/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/kcanvas

    common/bmp.cpp
        Very simple BMP file loader implementation

        This loader is specific ONLY for this project
        and sample bitmaps provided with this project, DO NOT use it as
        general bitmap loader anywhere else
*/

#include "bmp.h"
#include "kcommon/c_util.h"
#include <cstdio>
#include <cstdint>


#pragma pack(push, 2)

struct BITMAPFILEHEADER
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BITMAPINFOHEADER
{
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

#pragma pack(pop)


struct BGRA
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};


BitmapData::BitmapData(const char *filename) :
    p_data(nullptr),
    p_width(0),
    p_height(0),
    p_pitch(0),
    p_bpp(0)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return;
    }

    for (;;) {
        BITMAPFILEHEADER fh = {};
        fread(&fh, sizeof(fh), 1, file);

        if (fh.bfType != 19778) {
            break;
        }

        BITMAPINFOHEADER ih = {};
        fread(&ih, sizeof(ih), 1, file);

        if (ih.biBitCount != 8 && ih.biBitCount != 24 || ih.biCompression != 0 || ih.biPlanes != 1) {
            break;
        }

        p_width = ih.biWidth;
        p_height = ih.biHeight;
        p_bpp = ih.biBitCount;
        p_pitch = ih.biWidth * (ih.biBitCount == 8 ? 1 : 4);

        fseek(file, fh.bfOffBits, SEEK_SET);

        p_data = new char[p_pitch * p_height];
        char *buffer = new char[c_util::align(p_pitch, 4)];

        switch (p_bpp) {
            case 8:
                for (unsigned int y = 0; y < ih.biHeight; ++y) {
                    uint8_t *row = reinterpret_cast<uint8_t*>(p_data + (ih.biHeight - y - 1) * ih.biWidth);

                    int aligned = c_util::align(ih.biWidth, 4u);
                    fread(buffer, aligned, 1, file);

                    for (unsigned int x = 0; x < ih.biWidth; ++x) {
                        *row++ = buffer[x];
                    }
                }
                break;

            case 24:
                for (unsigned int y = 0; y < ih.biHeight; ++y) {
                    BGRA *row = reinterpret_cast<BGRA*>(p_data + (ih.biHeight - y - 1) * ih.biWidth * 4);

                    int aligned = c_util::align(ih.biWidth * 3, 4u);
                    fread(buffer, aligned, 1, file);

                    for (unsigned int x = 0; x < ih.biWidth; ++x) {
                        row->r = buffer[x * 3 + 2];
                        row->g = buffer[x * 3 + 1];
                        row->b = buffer[x * 3 + 0];
                        row->a = 255;
                        ++row;
                    }
                }
                break;
        }

        delete[] buffer;

        break;
    }

    fclose(file);
}

BitmapData::~BitmapData()
{
    delete[] p_data;
}
