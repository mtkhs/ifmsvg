#include <catch2/catch_test_macros.hpp>
#include "svg_detect.h"

using SvgDetect::Format;

TEST_CASE("plain SVG with .svg extension detected as Svg", "[svg_detect]") {
    const char* head = "<?xml version=\"1.0\"?><svg></svg>";
    REQUIRE(SvgDetect::Detect("foo.svg", reinterpret_cast<const uint8_t*>(head), 32)
            == Format::Svg);
}

TEST_CASE("plain SVG starting with <svg detected", "[svg_detect]") {
    const char* head = "<svg xmlns=\"http://www.w3.org/2000/svg\"></svg>";
    REQUIRE(SvgDetect::Detect("foo.svg", reinterpret_cast<const uint8_t*>(head), 32)
            == Format::Svg);
}

TEST_CASE("gzip magic detected as Svgz regardless of extension", "[svg_detect]") {
    const uint8_t head[] = {0x1F, 0x8B, 0x08, 0x00};
    REQUIRE(SvgDetect::Detect("foo.svgz", head, 4) == Format::Svgz);
    REQUIRE(SvgDetect::Detect("foo.svg",  head, 4) == Format::Svgz);
}

TEST_CASE("non-SVG extension rejected", "[svg_detect]") {
    const char* head = "<?xml version=\"1.0\"?>";
    REQUIRE(SvgDetect::Detect("foo.png", reinterpret_cast<const uint8_t*>(head), 21)
            == Format::Unknown);
}

TEST_CASE("SVG extension but garbage content rejected", "[svg_detect]") {
    const uint8_t head[] = {0xDE, 0xAD, 0xBE, 0xEF};
    REQUIRE(SvgDetect::Detect("foo.svg", head, 4) == Format::Unknown);
}

TEST_CASE("extension match alone (null head) accepted", "[svg_detect]") {
    REQUIRE(SvgDetect::Detect("foo.svg",  nullptr, 0) == Format::Svg);
    REQUIRE(SvgDetect::Detect("foo.svgz", nullptr, 0) == Format::Svgz);
}

TEST_CASE("extension match is case-insensitive", "[svg_detect]") {
    REQUIRE(SvgDetect::Detect("FOO.SVG",  nullptr, 0) == Format::Svg);
    REQUIRE(SvgDetect::Detect("Foo.SvgZ", nullptr, 0) == Format::Svgz);
}

TEST_CASE("BOM-prefixed SVG accepted", "[svg_detect]") {
    // UTF-8 BOM + <?xml
    const uint8_t head[] = {0xEF, 0xBB, 0xBF, '<', '?', 'x', 'm', 'l'};
    REQUIRE(SvgDetect::Detect("foo.svg", head, 8) == Format::Svg);
}
