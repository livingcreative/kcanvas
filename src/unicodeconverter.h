#pragma once

#include <string>

namespace k_canvas
{
    namespace impl
    {
        unsigned utf8codepoint(const char *utf8char);
        std::wstring utf8toutf16(const char *utf8);
        std::wstring utf8toutf16(const char *utf8, size_t size);
        std::u32string utf8toutf32(const char *utf8);
        std::u32string utf8toutf32(const char *utf8, size_t size);
    }
}
