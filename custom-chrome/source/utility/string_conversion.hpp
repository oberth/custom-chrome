// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#pragma once

#include <string>
#include <Windows.h>

namespace utility {

    inline auto convert_utf8_to_utf16(std::string const& string) {

        auto new_size = MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int>(string.size()), nullptr, 0);
        
        std::wstring utf16_string; utf16_string.resize(new_size);
        MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int>(string.size()), utf16_string.data(), new_size);

        return utf16_string;

    }

}