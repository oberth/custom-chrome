// Copyright ©2019 Domagoj "oberth" Pandža
// MIT license | Read LICENSE.txt for details.

#pragma once

#include <cstdint>

namespace measure {

    template<typename T>
    struct point {
        T x, y;

        point() = default;
        point(T const x, T const y)
        : x(x), y(y) {}

        auto& operator*=(T rhs) {
            this->x *= rhs;
            this->y *= rhs;
            return *this;
        }

        friend auto operator*(point<T> lhs, T rhs) {
            return lhs *= rhs;
        }

    };

    template<typename T>
    struct size {
        T width, height;

        size() = default;
        size(T const width, T const height)
        : width(width), height(height) {}

        auto& operator*=(T rhs) {
            this->width *= rhs;
            this->height *= rhs;
            return *this;
        }

        friend auto operator*(size<T> lhs, T rhs) {
            return lhs *= rhs;
        }

    };

    template<typename T>
    struct rectangle {
        point<T> origin;
        size<T> dimension;

        rectangle() = default;

        rectangle(T const x, T const y, T const w, T const h)
        : origin{ x, y }, dimension{ w, h } {}

        rectangle(point<T> const& origin, size<T> const& dimension)
        : origin{ origin }, dimension{ dimension } {}

        auto& operator*=(T rhs) {
            this->origin *= rhs;
            this->dimension *= rhs;
            return *this;
        }

        friend auto operator*(rectangle<T> lhs, T rhs) {
            lhs *= rhs;
            return lhs;
        }

    };

    struct color {
        float r, g, b, a;

        color(std::uint8_t const r, std::uint8_t const g, std::uint8_t const b, std::uint8_t const a = 255)
            : r(static_cast<float>(r) / 255.0f), g(static_cast<float>(g) / 255.0f),
            b(static_cast<float>(b) / 255.0f), a(static_cast<float>(a) / 255.0f) {}

        color(float const r, float const g, float const b, float const a = 1.0f)
        : r(r), g(g), b(b), a(a) {}

    };
}