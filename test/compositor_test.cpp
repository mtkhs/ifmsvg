#include <catch2/catch_test_macros.hpp>
#include "compositor.h"
#include "config.h"
#include "bmp.h"

using SvgRender::Pixmap;

TEST_CASE("opaque red 1x1 over white -> red BGR", "[compositor]") {
    Pixmap p;
    p.width = 1; p.height = 1;
    p.rgba = {0xFF, 0x00, 0x00, 0xFF};  // R G B A, premul (FF*1.0 = FF)

    Config::Settings cfg;
    cfg.background = Config::Background::Colored;
    cfg.background_color = 0xFFFFFFu;

    auto out = Compositor::ToDibBgr24(p, cfg);
    // line bytes for 1px BGR = 3 padded to 4
    REQUIRE(out.size() == 4);
    // BGR order, bottom-up but height 1 so order same
    REQUIRE(out[0] == 0x00);  // B
    REQUIRE(out[1] == 0x00);  // G
    REQUIRE(out[2] == 0xFF);  // R
    REQUIRE(out[3] == 0x00);  // padding
}

TEST_CASE("fully transparent over white -> white", "[compositor]") {
    Pixmap p;
    p.width = 1; p.height = 1;
    p.rgba = {0x00, 0x00, 0x00, 0x00};

    Config::Settings cfg;
    cfg.background = Config::Background::Colored;
    cfg.background_color = 0xFFFFFFu;

    auto out = Compositor::ToDibBgr24(p, cfg);
    REQUIRE(out[0] == 0xFF);
    REQUIRE(out[1] == 0xFF);
    REQUIRE(out[2] == 0xFF);
}

TEST_CASE("fully transparent over transparent background alternates", "[compositor]") {
    // 2x2 fully transparent. Checker squares are 8px so a 2x2 image is all
    // in the same cell -> all light gray (0xC0).
    Pixmap p;
    p.width = 2; p.height = 2;
    p.rgba.assign(2 * 2 * 4, 0);

    Config::Settings cfg;
    cfg.background = Config::Background::Transparent;

    auto out = Compositor::ToDibBgr24(p, cfg);
    // line_bytes = 6 padded to 8; total = 16
    REQUIRE(out.size() == 16);
    // all four pixels are the same (light cell: 0xFFFFFF)
    for (int row = 0; row < 2; ++row) {
        for (int x = 0; x < 2; ++x) {
            int off = row * 8 + x * 3;
            REQUIRE(out[off + 0] == 0xFF);
            REQUIRE(out[off + 1] == 0xFF);
            REQUIRE(out[off + 2] == 0xFF);
        }
    }
}

TEST_CASE("rows are bottom-up", "[compositor]") {
    // 1x2 image: top row red, bottom row green. After ToDibBgr24, the FIRST
    // row in memory should be the BOTTOM (green) per DIB convention.
    Pixmap p;
    p.width = 1; p.height = 2;
    p.rgba = {
        0xFF, 0x00, 0x00, 0xFF,   // row 0 (top): red
        0x00, 0xFF, 0x00, 0xFF,   // row 1 (bottom): green
    };

    Config::Settings cfg;
    cfg.background = Config::Background::Colored;

    auto out = Compositor::ToDibBgr24(p, cfg);
    // line_bytes = 4. row 0 of output = bottom of input = green
    REQUIRE(out[0] == 0x00);  // B
    REQUIRE(out[1] == 0xFF);  // G
    REQUIRE(out[2] == 0x00);  // R
    // row 1 of output = top of input = red
    REQUIRE(out[4] == 0x00);  // B
    REQUIRE(out[5] == 0x00);  // G
    REQUIRE(out[6] == 0xFF);  // R
}

TEST_CASE("half-transparent red over white blends", "[compositor]") {
    Pixmap p;
    p.width = 1; p.height = 1;
    // premultiplied: R=0x80 (=255*0.5), G=0, B=0, A=0x80
    p.rgba = {0x80, 0x00, 0x00, 0x80};

    Config::Settings cfg;
    cfg.background = Config::Background::Colored;
    cfg.background_color = 0xFFFFFFu;

    auto out = Compositor::ToDibBgr24(p, cfg);
    // final = src + bg*(1 - A/255) = 0x80 + 0xFF*(1 - 0x80/0xFF) ~ 0x80 + 0x7F = 0xFF for R
    // For G,B: 0 + 0xFF*0.5 ~ 0x7F or 0x80
    REQUIRE(out[2] == 0xFF);                   // R
    REQUIRE((out[1] == 0x7F || out[1] == 0x80));  // G
    REQUIRE((out[0] == 0x7F || out[0] == 0x80));  // B
}
