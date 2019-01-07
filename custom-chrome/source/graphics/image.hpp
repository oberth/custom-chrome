// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#pragma once
#include <string>

// Extremely simple image resource, does nothing but formulate a handle essentially.
// In the mockup application scenario, we just make every resource resident.
// Again, this is just serves as a visualization bonus on top of the explanation how to draw custom chromes.

namespace chrome::resource {

    struct image {

        image() = delete;
        image(std::string file_path) : _file_path(std::move(file_path)) {}

        image(image const&) = default;
        image(image&&) = default;
        image& operator=(image const&) = default;
        image& operator=(image&&) = default;

        ~image() = default;

        auto& get_file_path() const { return _file_path; }

    private:

        std::string _file_path;

    };

}
