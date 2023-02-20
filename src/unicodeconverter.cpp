#include "unicodeconverter.h"

using namespace std;


static wstring g_buffer;
static u32string g_buffer32;


namespace k_canvas
{
    namespace impl
    {
        static inline char32_t utf8_codepoint(const char *&utf8)
        {
            // check single byte 7-bit ASCII code and return it as a codepoint
            if ((*utf8 & 0x80) == 0) {
                return char32_t(unsigned char(*utf8++));
            }

            // here should be at least two byte codepoint
            auto bytecount = 2u;

            // count bytes
            auto mask = 0x20u;
            while ((*utf8 & mask) != 0) {
                ++bytecount;
                mask = mask >> 1;
            }

            // set first byte code mask
            mask = 0xffffff80 >> bytecount;
            mask = mask ^ 0xffffffff;

            // calc code shift
            auto shift = (bytecount - 1) * 6;

            // read first byte code bits
            auto codepoint = (*utf8++ & mask) << shift;

            // read rest bytes code bits
            while (--bytecount > 0) {
                shift -= 6;
                codepoint |= ((*utf8++) & 0x7f) << shift;
            }

            return char32_t(codepoint);
        }

        unsigned utf8codepoint(const char *utf8char)
        {
            return utf8_codepoint(utf8char);
        }

        wstring utf8toutf16(const char *utf8)
        {
            return utf8toutf16(utf8, strlen(utf8));
        }

        wstring utf8toutf16(const char *utf8, size_t size)
        {
            g_buffer.resize(size);

            auto result = g_buffer.data();
            auto end = utf8 + size;
            while (utf8 < end) {
                auto codepoint = utf8_codepoint(utf8);

                if (codepoint < 0x10000) {
                    *result++ = wchar_t(codepoint);
                    continue;
                }

                codepoint -= 0x10000;
                *result++ = wchar_t(0xd800 | (codepoint >> 10));
                *result++ = wchar_t(0xdc00 | (codepoint & 0x03ff));
            }

            g_buffer.resize(result - g_buffer.data());

            return g_buffer;
        }

        u32string utf8toutf32(const char *utf8)
        {
            return utf8toutf32(utf8, strlen(utf8));
        }

        u32string utf8toutf32(const char *utf8, size_t size)
        {
            g_buffer32.resize(size);

            auto result = g_buffer32.data();
            auto end = utf8 + size;
            while (utf8 < end) {
                auto codepoint = utf8_codepoint(utf8);
                *result++ = codepoint;
            }

            g_buffer32.resize(result - g_buffer32.data());

            return g_buffer32;
        }
    }
}
