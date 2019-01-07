// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#pragma once

#include <d2d1.h>
#include <dwrite.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dcomp.h>
#include <wincodec.h>
#include <unordered_map>
#include <string>
#include <cmath>
#include <stack>

#include <utility/measure.hpp>
#include <graphics/image.hpp>
#include <com/memory.hpp>

namespace chrome::graphics {

    // Just a simple renderer.
    struct renderer {

        renderer();

        auto attach_to_window(HWND window_handle) -> void;
        auto begin_draw() -> void;
        auto end_draw() -> void;

        auto fill_rectangle(measure::rectangle<float> const& fill_area, measure::color const& fill_color) -> void;
        auto draw_line(measure::point<float> const& start, measure::point<float> const& end, float const stroke_width, measure::color const& stroke_color) -> void;
        auto draw_image(resource::image const* image, float scale, measure::point<float> const& top_left, float const opacity) -> void;

        auto draw_text(
            std::string const& text, measure::point<float> const& top_left, std::string const& font_family, float const font_size,
            measure::color const& text_color, DWRITE_FONT_WEIGHT const font_weight = DWRITE_FONT_WEIGHT_NORMAL
        ) -> void;

        auto push_transform(D2D1::Matrix3x2F const& transform) -> void;
        auto pop_transform() -> void;

        auto resize_buffers() -> void;

    private:

        auto clear_transforms() -> void;

        auto get_texture(std::string const& filename)-> ID2D1Bitmap*;
        auto load_image_into_pool(std::string const& filename) -> ID2D1Bitmap*;

        com::unique_ptr<ID3D11Device> _device_d3d11;
        com::unique_ptr<ID3D11DeviceContext> _device_context_d3d11;
        com::unique_ptr<ID2D1Factory> _factory_d2d1;
        com::unique_ptr<ID2D1RenderTarget> _dxgi_render_target_d2d1;
        com::unique_ptr<IDWriteFactory> _factory_dwrite;
        com::unique_ptr<IWICImagingFactory> _factory_wic;

        com::unique_ptr<ID2D1Device> _device_d2d1;
        com::unique_ptr<ID2D1DeviceContext> _device_context_d2d1;
        com::unique_ptr<ID2D1DeviceContext> _resource_device_context_d2d1;

        com::unique_ptr<ID2D1SolidColorBrush> _brush;
        com::unique_ptr<IDWriteTextFormat> _standard_text_format;

        com::unique_ptr<IDCompositionDesktopDevice> _device_dcomp;
        com::unique_ptr<IDCompositionTarget> _window_target_dcomp;
        com::unique_ptr<IDCompositionVisual2> _primary_visual_dcomp;
        com::unique_ptr<IDCompositionVirtualSurface> _window_surface_dcomp;

        std::unordered_map<std::string, com::unique_ptr<ID2D1Bitmap>> _resident_textures_map;

        HWND _associated_window = nullptr;

        float _dpi_x = 96.0f, _dpi_y = 96.0f;

        std::stack<D2D1::Matrix3x2F> _transforms;

    };

}