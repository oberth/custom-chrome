// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#pragma once
#include <string>
#include <Windows.h>
#include <dwmapi.h>

#include <utility/measure.hpp>
#include <graphics/renderer.hpp>
#include <graphics/image.hpp>

namespace chrome::gui {

    struct window {

        friend auto CALLBACK process_message(HWND, UINT, WPARAM, LPARAM) -> LRESULT;

        window(std::string title, measure::rectangle<float> const& frame);

        window(window const&) = delete;
        window(window&&) = default;

        window& operator=(window const&) = delete;
        window& operator=(window&&) = default;

        ~window() = default;

        auto show_window() -> void;
        auto hide_window() -> void;

    private:

        auto paint() -> void;
        auto paint_mock_tabs(measure::rectangle<float> const& client_rectangle) -> void;
        auto paint_mock_toolbar(measure::rectangle<float> const& client_rectangle) -> void;
        auto paint_mock_sidebar(measure::rectangle<float> const& client_rectangle) -> void;

        auto handle_resize() -> void;

        HWND _system_window_handle = nullptr;
        MARGINS _margins {};
        std::string _title;
        float _user_scaling = 96.0f;
        float _client_area_offset_dip = 0.0f;
        
        std::unique_ptr<graphics::renderer> _renderer;

        std::unique_ptr<resource::image> _tab_raster;
        std::unique_ptr<resource::image> _new_tab_symbol;

    };

}