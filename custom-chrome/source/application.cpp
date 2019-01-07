#include <array>
#include <sstream>
#include <dwmapi.h>

#include <application.hpp>

namespace chrome {

    application::application(char**, int) {

        auto frame = measure::rectangle{ 100.0f, 100.0f, 1280.f, 720.f };
        _window = std::make_unique<gui::window>("Chrome management", frame);
        _window->show_window(); 
        
    }

    auto application::execute() -> int {

        MSG message_structure{};

        while (message_structure.message != WM_QUIT)
        if (PeekMessageW(&message_structure, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message_structure);
            DispatchMessageW(&message_structure);
        }

        return static_cast<int>(message_structure.wParam);

    }

}