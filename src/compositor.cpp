#include "compositor.h"
#include "bmp.h"

namespace Compositor {

namespace {

constexpr int kCheckerSize = 8;
constexpr uint8_t kCheckerLight = 0xFF;
constexpr uint8_t kCheckerDark  = 0xCC;

struct Bg { uint8_t b, g, r; };

Bg BackgroundAt(int x, int y, const Config::Settings& cfg) {
    if (cfg.background == Config::Background::Transparent) {
        bool light = ((x / kCheckerSize) + (y / kCheckerSize)) % 2 == 0;
        uint8_t v = light ? kCheckerLight : kCheckerDark;
        return {v, v, v};
    }
    // White (solid color).
    return {
        static_cast<uint8_t>( cfg.background_color        & 0xFF),
        static_cast<uint8_t>((cfg.background_color >> 8)  & 0xFF),
        static_cast<uint8_t>((cfg.background_color >> 16) & 0xFF),
    };
}

// final = src_premul + round(bg * (255 - alpha) / 255)  [+127 is the rounding term]
inline uint8_t Composite(uint8_t src_premul, uint8_t bg, uint8_t alpha) {
    return static_cast<uint8_t>(src_premul + (bg * (255 - alpha) + 127) / 255);
}

}  // namespace

std::vector<uint8_t> ToDibBgr24(const SvgRender::Pixmap& src,
                                const Config::Settings& cfg) {
    if (src.width <= 0 || src.height <= 0) return {};

    const int stride_out = Bmp::CalculateLineBytes(src.width, 24);
    std::vector<uint8_t> out(static_cast<size_t>(stride_out) * src.height, 0);

    for (int y_top = 0; y_top < src.height; ++y_top) {
        const int y_bottom = src.height - 1 - y_top;
        const uint8_t* in_row  = src.rgba.data() + static_cast<size_t>(y_top)   * src.width * 4;
        uint8_t*       out_row = out.data()      + static_cast<size_t>(y_bottom) * stride_out;
        for (int x = 0; x < src.width; ++x) {
            const uint8_t r = in_row[x * 4 + 0];
            const uint8_t g = in_row[x * 4 + 1];
            const uint8_t b = in_row[x * 4 + 2];
            const uint8_t a = in_row[x * 4 + 3];
            const Bg bg = BackgroundAt(x, y_top, cfg);
            out_row[x * 3 + 0] = Composite(b, bg.b, a);
            out_row[x * 3 + 1] = Composite(g, bg.g, a);
            out_row[x * 3 + 2] = Composite(r, bg.r, a);
        }
    }
    return out;
}

}  // namespace Compositor
