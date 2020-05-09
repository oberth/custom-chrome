#pragma once
#include <stdexcept>
#include <string>
#include <Windows.h>

namespace com {
    inline auto validate_result(HRESULT result, std::string const& error_message) {
        if (FAILED(result)) throw std::runtime_error{ error_message };
    }
}