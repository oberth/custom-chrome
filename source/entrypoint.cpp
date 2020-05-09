// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#include <application.hpp>
#include <utility/string_conversion.hpp>

auto main(int argument_count, char* arguments[]) -> int try {

    chrome::application chrome_application { arguments, argument_count };
    return chrome_application.execute();

}

catch (std::exception const& exception) {

    auto what = utility::convert_utf8_to_utf16(exception.what());
    MessageBoxW(nullptr, what.c_str(), L"Fatal error!", MB_ICONERROR | MB_OK);
    
    return -1;

}