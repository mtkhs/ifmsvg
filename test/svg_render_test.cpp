#include <catch2/catch_test_macros.hpp>
#include "svg_render.h"
#include "config.h"

namespace {
const char kRedSquare[] =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" "
    "width=\"10\" height=\"10\">"
    "<rect width=\"10\" height=\"10\" fill=\"red\"/>"
    "</svg>";
}

TEST_CASE("Render produces 10x10 image at DPI 96", "[svg_render]") {
    Config::Settings cfg;
    cfg.dpi = 96;
    cfg.load_system_fonts = false;

    auto pix = SvgRender::Render(reinterpret_cast<const uint8_t*>(kRedSquare),
                                 sizeof(kRedSquare) - 1, cfg);
    REQUIRE(pix.has_value());
    REQUIRE(pix->width == 10);
    REQUIRE(pix->height == 10);
    REQUIRE(pix->rgba.size() == 10 * 10 * 4);

    // Center pixel should be red (premul, fully opaque).
    int center_off = (5 * 10 + 5) * 4;
    REQUIRE(pix->rgba[center_off + 0] == 0xFF);  // R
    REQUIRE(pix->rgba[center_off + 1] == 0x00);  // G
    REQUIRE(pix->rgba[center_off + 2] == 0x00);  // B
    REQUIRE(pix->rgba[center_off + 3] == 0xFF);  // A
}

TEST_CASE("Render at DPI 192 doubles pixel dimensions", "[svg_render]") {
    Config::Settings cfg;
    cfg.dpi = 192;
    cfg.load_system_fonts = false;

    auto pix = SvgRender::Render(reinterpret_cast<const uint8_t*>(kRedSquare),
                                 sizeof(kRedSquare) - 1, cfg);
    REQUIRE(pix.has_value());
    REQUIRE(pix->width == 20);
    REQUIRE(pix->height == 20);
}

TEST_CASE("Render of garbage returns nullopt", "[svg_render]") {
    Config::Settings cfg;
    cfg.load_system_fonts = false;
    const uint8_t junk[] = {0x00, 0x01, 0x02, 0x03};
    REQUIRE_FALSE(SvgRender::Render(junk, sizeof(junk), cfg).has_value());
}

TEST_CASE("max_size clamps oversized output", "[svg_render]") {
    Config::Settings cfg;
    cfg.dpi = 9600;     // would produce 1000x1000
    cfg.max_size = 200;
    cfg.load_system_fonts = false;

    auto pix = SvgRender::Render(reinterpret_cast<const uint8_t*>(kRedSquare),
                                 sizeof(kRedSquare) - 1, cfg);
    REQUIRE(pix.has_value());
    REQUIRE(pix->width <= 200);
    REQUIRE(pix->height <= 200);
}

TEST_CASE("GetDimensions matches Render dimensions", "[svg_render]") {
    Config::Settings cfg;
    cfg.dpi = 150;
    cfg.load_system_fonts = false;

    auto dims = SvgRender::GetDimensions(reinterpret_cast<const uint8_t*>(kRedSquare),
                                         sizeof(kRedSquare) - 1, cfg);
    auto pix  = SvgRender::Render(reinterpret_cast<const uint8_t*>(kRedSquare),
                                  sizeof(kRedSquare) - 1, cfg);
    REQUIRE(dims.has_value());
    REQUIRE(pix.has_value());
    REQUIRE(dims->width  == pix->width);
    REQUIRE(dims->height == pix->height);
    REQUIRE(dims->x_density == 150);
    REQUIRE(dims->y_density == 150);
}
