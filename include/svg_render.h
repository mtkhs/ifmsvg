#ifndef IFMSVG_SVG_RENDER_H
#define IFMSVG_SVG_RENDER_H

#include <cstdint>
#include <optional>
#include <vector>

namespace Config { struct Settings; }

namespace SvgRender {

struct Pixmap {
    std::vector<uint8_t> rgba;  // premultiplied, row-major top-down, stride = width*4
    int width = 0;
    int height = 0;
};

struct Dimensions {
    int width;
    int height;
    int x_density;
    int y_density;
};

std::optional<Pixmap> Render(const uint8_t* svg_bytes, size_t len,
                             const Config::Settings& cfg);

std::optional<Dimensions> GetDimensions(const uint8_t* svg_bytes, size_t len,
                                        const Config::Settings& cfg);

}  // namespace SvgRender

#endif
