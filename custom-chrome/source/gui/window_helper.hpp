// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#pragma once

#include <cmath>
#include <cstdint>
#include <Windows.h>
#include <windowsx.h>

namespace chrome::gui::helper {

    // Simplest variant, just to get you started.
    auto compute_sector_of_window(HWND window_handle, WPARAM, LPARAM lparam, int caption_height) -> LRESULT {

        // Acquire the window rect
        RECT window_rectangle;
        GetWindowRect(window_handle, &window_rectangle);

        auto offset = 10;

        POINT cursor_position{
            GET_X_LPARAM(lparam),
            GET_Y_LPARAM(lparam)
        };

        if (cursor_position.y < window_rectangle.top + offset && cursor_position.x < window_rectangle.left + offset) return HTTOPLEFT;
        if (cursor_position.y < window_rectangle.top + offset && cursor_position.x > window_rectangle.right - offset) return HTTOPRIGHT;
        if (cursor_position.y > window_rectangle.bottom - offset && cursor_position.x > window_rectangle.right - offset) return HTBOTTOMRIGHT;
        if (cursor_position.y > window_rectangle.bottom - offset && cursor_position.x < window_rectangle.left + offset) return HTBOTTOMLEFT;

        if (cursor_position.x > window_rectangle.left && cursor_position.x < window_rectangle.right) {
            if (cursor_position.y < window_rectangle.top + offset) return HTTOP;
            else if (cursor_position.y > window_rectangle.bottom - offset) return HTBOTTOM;
        }
        if (cursor_position.y > window_rectangle.top && cursor_position.y < window_rectangle.bottom) {
            if (cursor_position.x < window_rectangle.left + offset) return HTLEFT;
            else if (cursor_position.x > window_rectangle.right - offset) return HTRIGHT;
        }

        if (cursor_position.x > window_rectangle.left && cursor_position.x < window_rectangle.right) {
            if (cursor_position.y < window_rectangle.top + caption_height) return HTCAPTION;
        }

        return HTNOWHERE;

    }

    auto compute_standard_caption_height_for_dpi(std::uint32_t dpi) {

        auto const user_scaling = static_cast<float>(dpi) / 96.0f;

        auto const base_caption = GetSystemMetricsForDpi(SM_CYCAPTION, 96);
        auto const base_frame_padding = GetSystemMetricsForDpi(SM_CXFIXEDFRAME, 96);
        auto const framesize = GetSystemMetricsForDpi(SM_CYSIZEFRAME, dpi);
        auto const border = GetSystemMetricsForDpi(SM_CYBORDER, dpi);

        auto const linear_part = user_scaling * (base_caption + base_frame_padding);
        return static_cast<std::uint32_t>(std::ceilf(linear_part + framesize + border));

    }

    auto compute_standard_caption_for_current_dpi() {
        return compute_standard_caption_height_for_dpi(GetDpiForSystem());
    }

}