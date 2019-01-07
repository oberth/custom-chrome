// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#pragma once

#include <string>

#include <gui/window.hpp>

namespace chrome {

    struct application {

        application() = delete;
        application(char** args, int argument_count);

        application(application const&) = delete;
        application(application&&) = delete;

        application& operator=(application const&) = delete;
        application& operator=(application&&) = delete;

        ~application() = default;

        auto execute() -> int;

    private:

        std::unique_ptr<gui::window> _window;

    };

}

