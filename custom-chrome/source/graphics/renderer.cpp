#include <array>

#include <graphics/renderer.hpp>
#include <com/runtime_validation.hpp>
#include <utility/string_conversion.hpp>

namespace chrome::graphics {

    renderer::renderer() {

        std::uint32_t flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        std::array requested_levels { 
            D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, 
            D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
        };

        D3D_FEATURE_LEVEL received_feature_level; // Any will do in this scenario.
        ID3D11Device* temporary_device; ID3D11DeviceContext* temporary_context;

        auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            flags, requested_levels.data(), static_cast<std::uint32_t>(requested_levels.size()),
            D3D11_SDK_VERSION, &temporary_device, &received_feature_level, &temporary_context
        );

        com::validate_result(hr, "Failed during creation of the D3D11 device.");
        _device_context_d3d11 = com::make_unique(temporary_context);
        _device_d3d11 = com::make_unique(temporary_device);

        ID2D1Factory* temporary_factory;
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &temporary_factory);
        com::validate_result(hr, "Failed during creation of the D2D1 factory.");
        _factory_d2d1 = com::make_unique(temporary_factory);
        
        auto pixel_format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

        _factory_d2d1->GetDesktopDpi(&_dpi_x, &_dpi_y);
        auto properties = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT, pixel_format, _dpi_x, _dpi_y, 
            D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT
        );

        IDWriteFactory* temporary_factory_dwrite;
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), 
            reinterpret_cast<IUnknown**>(&temporary_factory_dwrite)
        );

        com::validate_result(hr, "Failed during the creation of the DWrite factory.");
        _factory_dwrite = com::make_unique(temporary_factory_dwrite);

        com::validate_result(CoInitialize(nullptr), "Failed to CoInitialize.");

        IWICImagingFactory* temporary_factory_wic;
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&temporary_factory_wic));
        com::validate_result(hr, "Failed during the creation of the WIC factory.");
        _factory_wic = com::make_unique(temporary_factory_wic);

        IDXGIDevice* temporary_device_dxgi;
        _device_d3d11->QueryInterface<IDXGIDevice>(&temporary_device_dxgi);
        auto device_dxgi = com::make_unique<IDXGIDevice>(temporary_device_dxgi);

        auto creation_properties = D2D1::CreationProperties(
            D2D1_THREADING_MODE_SINGLE_THREADED, D2D1_DEBUG_LEVEL_INFORMATION, D2D1_DEVICE_CONTEXT_OPTIONS_NONE
        );

        ID2D1Device* temporary_device_d2d1;
        hr = D2D1CreateDevice(device_dxgi.get(), creation_properties, &temporary_device_d2d1);
        _device_d2d1 = com::make_unique(temporary_device_d2d1);
        com::validate_result(hr, "Failed during the creation of the D2D1 device.");

        ID2D1DeviceContext* temporary_device_context_d2d1;
        hr = _device_d2d1->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &temporary_device_context_d2d1);
        com::validate_result(hr, "Failed during the creation of the resource creating D2D1 device context.");
        _resource_device_context_d2d1 = com::make_unique(temporary_device_context_d2d1);
        _resource_device_context_d2d1->SetDpi(_dpi_x, _dpi_x);

        IDCompositionDesktopDevice* temporary_device_dcomp;
        hr = DCompositionCreateDevice2(_device_d2d1.get(), IID_PPV_ARGS(&temporary_device_dcomp));
        com::validate_result(hr, "Failed during the creation of the DComp device.");
        _device_dcomp = com::make_unique(temporary_device_dcomp);
        
        IDCompositionVisual2* temporary_visual;
        _device_dcomp->CreateVisual(&temporary_visual);
        _primary_visual_dcomp = com::make_unique(temporary_visual);

        _transforms.emplace(D2D1::Matrix3x2F::Identity());

        ID2D1SolidColorBrush* temporary_brush;
        _resource_device_context_d2d1->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), &temporary_brush);
        _brush = com::make_unique(temporary_brush);

    }

    auto renderer::attach_to_window(HWND window_handle) -> void {

        _associated_window = window_handle;

        IDCompositionTarget* temporary_target;
        auto hr = _device_dcomp->CreateTargetForHwnd(window_handle, true, &temporary_target);
        com::validate_result(hr, "Failed during creation of the DComp window target.");
        _window_target_dcomp = com::make_unique(temporary_target);
        _window_target_dcomp->SetRoot(_primary_visual_dcomp.get());

        RECT window_rectangle;
        GetClientRect(_associated_window, &window_rectangle);

        IDCompositionVirtualSurface* temporary_surface;
        _device_dcomp->CreateVirtualSurface(
            window_rectangle.right, window_rectangle.bottom,
            DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED,
            &temporary_surface
        );

        _window_surface_dcomp = com::make_unique(temporary_surface);
        _primary_visual_dcomp->SetContent(_window_surface_dcomp.get());

    }

    auto renderer::begin_draw() -> void {

        ID2D1DeviceContext* temporary_device_context_d2d1; POINT offset = {};
        _window_surface_dcomp->BeginDraw(nullptr, IID_PPV_ARGS(&temporary_device_context_d2d1), &offset);
        _device_context_d2d1 = com::make_unique(temporary_device_context_d2d1);
        _device_context_d2d1->SetDpi(_dpi_x, _dpi_y);
        auto offsetX = static_cast<float>(offset.x);
        auto offsetY = static_cast<float>(offset.y);

        push_transform(D2D1::Matrix3x2F::Translation(
            offsetX * 96.0f / _dpi_x, offsetY * 96.0f / _dpi_y
        ));

        _device_context_d2d1->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0, 0.0f));

    }

    auto renderer::end_draw() -> void {

        clear_transforms();

        _window_surface_dcomp->EndDraw();
        // _device_dcomp->WaitForCommitCompletion(); // Uncomment if you care about trailing while resizing
        _device_dcomp->Commit();

    }

    auto renderer::fill_rectangle(measure::rectangle<float> const& fill_area, measure::color const& fill_color) -> void {

        _brush->SetColor(D2D1::ColorF(fill_color.r, fill_color.g, fill_color.b, fill_color.a));

        // No reference because MSVC has issues. (Bug reported, fix expected in VS 2019)
        auto [origin, dimension] = fill_area;
        auto [x, y] = origin; auto [w, h] = dimension;

        _device_context_d2d1->FillRectangle(D2D1::RectF(x, y, x + w, y + h), _brush.get());

    }

    auto renderer::draw_line(
        measure::point<float> const& start_point, measure::point<float> const& end_point, 
        float const stroke_width, measure::color const& stroke_color
    ) -> void {

        auto& [r, g, b, a] = stroke_color;
        _brush->SetColor(D2D1::ColorF(r, g, b, a));

        auto& p = start_point; auto& q = end_point;

        _device_context_d2d1->DrawLine(
            D2D1::Point2F(p.x, p.y), D2D1::Point2F(q.x, q.y), 
            _brush.get(), stroke_width
        );

    }

    auto renderer::draw_image(resource::image const* image, float scale, measure::point<float> const& top_left, float const opacity) -> void {

        auto resident_texture = get_texture(image->get_file_path());
        auto dimensions = resident_texture->GetSize();

        auto& [x, y] = top_left;
        auto& [width, height] = dimensions;

        _device_context_d2d1->DrawBitmap(
            resident_texture, D2D1::RectF(x, y, x + width * scale, y + height * scale),
            opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );

    }

    auto renderer::draw_text(
        std::string const& text, measure::point<float> const& top_left, std::string const& font_family,
        float const font_size, measure::color const& text_color, DWRITE_FONT_WEIGHT const font_weight
    ) -> void {

        auto wtext = utility::convert_utf8_to_utf16(text);
        auto wfont_family = utility::convert_utf8_to_utf16(font_family);

        IDWriteTextFormat* temporary_text_format;
        _factory_dwrite->CreateTextFormat(
            wfont_family.c_str(), nullptr, font_weight, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, font_size, L"en_US", &temporary_text_format
        );

        auto text_format = com::make_unique(temporary_text_format);

        _brush->SetColor(D2D1::ColorF(text_color.r, text_color.g, text_color.b, text_color.a));
        _device_context_d2d1->DrawTextW(wtext.c_str(), static_cast<UINT32>(wtext.size()), text_format.get(),
            D2D1::RectF(top_left.x, top_left.y, 5000.f, 5000.f), _brush.get()
        );

    }

    auto renderer::push_transform(D2D1::Matrix3x2F const& transform) -> void {

        auto& previous_transform = _transforms.top();
        _transforms.emplace(previous_transform * transform);
        _device_context_d2d1->SetTransform(_transforms.top());

    }

    auto renderer::pop_transform() -> void {

        _transforms.pop();
        _device_context_d2d1->SetTransform(_transforms.top());

    }

    auto renderer::resize_buffers() -> void {

        RECT window_rectangle;
        GetClientRect(_associated_window, &window_rectangle);

        _window_surface_dcomp->Resize(window_rectangle.right, window_rectangle.bottom);

    }

    auto renderer::clear_transforms() -> void {

        _transforms = {}; _transforms.emplace(D2D1::Matrix3x2F::Identity());
        _device_context_d2d1->SetTransform(_transforms.top());

    }

    auto renderer::get_texture(std::string const& filename) -> ID2D1Bitmap* {

        auto lookup_iterator = _resident_textures_map.find(filename);
        return lookup_iterator != _resident_textures_map.end() ? lookup_iterator->second.get() : load_image_into_pool(filename);

    }

    auto renderer::load_image_into_pool(std::string const& filename) -> ID2D1Bitmap* {

        auto wfilename = utility::convert_utf8_to_utf16(filename);

        IWICBitmapDecoder* temporary_bitmap_decoder;
        _factory_wic->CreateDecoderFromFilename(
            wfilename.c_str(), nullptr, GENERIC_READ, 
            WICDecodeOptions::WICDecodeMetadataCacheOnLoad, 
            &temporary_bitmap_decoder
        );
        auto bitmap_decoder = com::make_unique<IWICBitmapDecoder>(temporary_bitmap_decoder);

        IWICBitmapFrameDecode* temporary_frame_decode;
        bitmap_decoder->GetFrame(0, &temporary_frame_decode);
        auto zero_frame = com::make_unique<IWICBitmapFrameDecode>(temporary_frame_decode);

        IWICFormatConverter* temporary_format_converter;
        _factory_wic->CreateFormatConverter(&temporary_format_converter);
        auto format_converter = com::make_unique<IWICFormatConverter>(temporary_format_converter);

        format_converter->Initialize(
            zero_frame.get(), GUID_WICPixelFormat32bppPBGRA, 
            WICBitmapDitherTypeNone, nullptr, 0.0f, 
            WICBitmapPaletteTypeMedianCut
        );

        ID2D1Bitmap* temporary_bitmap;
        _resource_device_context_d2d1->CreateBitmapFromWicBitmap(format_converter.get(), &temporary_bitmap);
        _resident_textures_map.emplace(filename, temporary_bitmap);

        return temporary_bitmap;

    }

}