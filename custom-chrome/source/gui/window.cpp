#include <gui/window.hpp>
#include <cmath>

#include <utility/string_conversion.hpp>
#include <gui/window_helper.hpp>

namespace chrome::gui {

    auto CALLBACK process_message(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT {

        LRESULT result;
        // Ask whether DWM would like to process the incoming message (to handle the caption parts)
        auto dwm_has_processed = DwmDefWindowProc(window_handle, message, wparam, lparam, &result);
        if (dwm_has_processed) return result;

        auto window = reinterpret_cast<chrome::gui::window*>(GetWindowLongPtr(window_handle, GWLP_USERDATA));

        if (message == WM_CREATE) {

            auto create_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
            SetWindowLongPtrW(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));

            // We need to trigger recompute of the window and client area.
            SetWindowPos(window_handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);

        }

        // Extends the client area all around (returning 0 when wparam is TRUE)
        else if (message == WM_NCCALCSIZE) {

            auto client_area_needs_calculating = static_cast<bool>(wparam);

            if (client_area_needs_calculating) {
                auto parameters = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);

                auto& requested_client_area = parameters->rgrc[0];
                requested_client_area.right -= GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                requested_client_area.left += GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                requested_client_area.bottom -= GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);

                return 0;
            }

        }

        // Determine whether the cursor is near interactive points of the window
        else if ((message == WM_NCHITTEST)) {

            result = helper::compute_sector_of_window(window_handle, wparam, lparam, window->_margins.cyTopHeight);
            if (result != HTNOWHERE) return result;

        }

        else if (message == WM_PAINT) window->paint();
        else if (message == WM_CLOSE) PostQuitMessage(0);
        else if (message == WM_SIZE) window->handle_resize();

        return DefWindowProcW(window_handle, message, wparam, lparam);

    }

    window::window(std::string title, measure::rectangle<float> const& frame) {

        _title = std::move(title);

        WNDCLASSEX window_class {

            sizeof(WNDCLASSEX), CS_VREDRAW | CS_HREDRAW, process_message, 0, sizeof(this),
            GetModuleHandleW(nullptr), nullptr, LoadCursor(nullptr, IDC_ARROW),
            (HBRUSH)GetStockObject(BLACK_BRUSH), nullptr, L"BasicWindow", nullptr

        };

        RegisterClassEx(&window_class);

        auto wide_title = utility::convert_utf8_to_utf16(_title);

        _system_window_handle = CreateWindowExW(
            WS_EX_NOREDIRECTIONBITMAP, window_class.lpszClassName, wide_title.c_str(), 
            WS_OVERLAPPEDWINDOW, 10, 10, 100, 100, nullptr, nullptr, window_class.hInstance, this
        );

        if (_system_window_handle == nullptr) throw std::runtime_error{ "Failed to create a window." };

        auto dpi = GetDpiForWindow(_system_window_handle);
        _user_scaling = static_cast<float>(dpi) / 96.0f;

        auto [origin, dimension] = frame;
        dimension *= _user_scaling;

        SetWindowPos(
            _system_window_handle, nullptr, static_cast<int>(origin.x), static_cast<int>(origin.y),
            static_cast<int>(dimension.width), static_cast<int>(dimension.height), SWP_FRAMECHANGED
        );

        auto offset_for_tabs = 10u; // Arbitrary
        auto caption_height = helper::compute_standard_caption_height_for_window(_system_window_handle);
        _margins = { 0, 0, static_cast<int>(caption_height + offset_for_tabs), 0 };
        auto hr = DwmExtendFrameIntoClientArea(_system_window_handle, &_margins);
        if (FAILED(hr)) throw std::runtime_error { "DWM failed to extend frame into the client area." };
        _client_area_offset_dip = static_cast<float>(_margins.cyTopHeight) / _user_scaling;

        _tab_raster         = std::make_unique<resource::image>("media/tab_raster.png");
        _new_tab_symbol     = std::make_unique<resource::image>("media/new_tab_symbol.png");

        _renderer = std::make_unique<graphics::renderer>();
        _renderer->attach_to_window(_system_window_handle);

    }

    auto window::show_window() -> void {
        ShowWindow(_system_window_handle, SW_SHOW);
    }

    auto window::hide_window() -> void {
        ShowWindow(_system_window_handle, SW_HIDE);
    }

    auto window::paint() -> void {

        _renderer->begin_draw();

        RECT client_rectangle;
        GetClientRect(_system_window_handle, &client_rectangle);

        measure::rectangle<float> client_area {
            0.0f, 0.0f,
            static_cast<float>(client_rectangle.right),
            static_cast<float>(client_rectangle.bottom - _margins.cyTopHeight)
        };

        client_area *= 1.0f / _user_scaling;

        _renderer->push_transform(D2D1::Matrix3x2F::Translation(0.0f, _client_area_offset_dip));

        // Paint the whole client area into our baseline color.
        _renderer->fill_rectangle(client_area, measure::color{ 0.96f, 0.96f, 0.96f });

        paint_mock_tabs(client_area);
        paint_mock_toolbar(client_area);
        paint_mock_sidebar(client_area);

        _renderer->end_draw();

    }

    auto window::paint_mock_tabs(measure::rectangle<float> const& window_rectangle) -> void {

        _renderer->draw_image(_tab_raster.get(), 0.5f, measure::point<float> {229.f, -28.0f}, 0.6f);
        _renderer->draw_image(_new_tab_symbol.get(), 0.5f, measure::point<float> {460.f, -22.0f}, 1.0f);

        _renderer->draw_line(
            measure::point<float> { window_rectangle.origin.x, -1.0f + 0.5f},
            measure::point<float> { window_rectangle.dimension.width, -1.0f + 0.5f},
            1.0f, measure::color{ 0.77f, 0.77f, 0.77f, 1.0f }
        );

        _renderer->draw_image(_tab_raster.get(), 0.5f, measure::point<float> {10.0f, -28.0f}, 1.0f);

        _renderer->draw_text(
            "Expand the frame into t...", measure::point<float>{ 48.f, -24.f },
            "Segoe UI", 14.0f, measure::color{ 0.4f, 0.4f, 0.4f, 1.0f }
        );

        _renderer->draw_text(
            "Recompute the window...", measure::point<float>{ 221.f + 48.f, -24.f },
            "Segoe UI", 14.0f, measure::color{ 0.4f, 0.4f, 0.4f, 0.6f }
        );

    }

    auto window::paint_mock_toolbar(measure::rectangle<float> const& window_rectangle) -> void {

        _renderer->draw_line(
            measure::point<float> { window_rectangle.origin.x, 50.5f},
            measure::point<float> { window_rectangle.dimension.width, 50.5f},
            1.0f, measure::color{ 0.82f, 0.82f, 0.82f, 1.0f }
        );

        _renderer->draw_line(
            measure::point<float> { window_rectangle.origin.x, 54.5f},
            measure::point<float> { window_rectangle.dimension.width, 54.5f},
            1.0f, measure::color{ 0.82f, 0.82f, 0.82f, 1.0f }
        );

        _renderer->fill_rectangle(
            measure::rectangle<float> {
                window_rectangle.origin.x, 55.0f,
                window_rectangle.dimension.width - window_rectangle.origin.x, 
                window_rectangle.dimension.height - 55.0f
            }, measure::color{ 1.0f, 1.0f, 1.0f }
        );

        _renderer->fill_rectangle(
            measure::rectangle<float> {
                14.0f, 9.0f, window_rectangle.dimension.width - 28.0f, 32.0f
            }, measure::color{ 0.9f, 0.9f, 0.9f }
        );

        _renderer->fill_rectangle(
            measure::rectangle<float> {
                15.0f,  10.0f, window_rectangle.dimension.width - 30.0f, 30.0f
            }, measure::color{ 1.0f, 1.0f, 1.0f }
        );

    }


    auto window::paint_mock_sidebar(measure::rectangle<float> const& window_rectangle) -> void {

        _renderer->fill_rectangle(
            measure::rectangle<float> {
                window_rectangle.origin.x, 55.0f,
                window_rectangle.origin.x + 400.0f, 
                window_rectangle.dimension.height
            }, measure::color{ 0.92f, 0.92f, 0.92f }
        );

    }

    auto window::handle_resize() -> void {
        if(_renderer) _renderer->resize_buffers();
    }
}
