#include <catch2/catch_test_macros.hpp>
#include <zlib.h>
#include <cstring>
#include <vector>
#include "svgz.h"

namespace {
std::vector<uint8_t> GzipCompress(const std::string& input) {
    z_stream zs{};
    // 31 = 15 (default windowBits) + 16 (gzip wrapper)
    REQUIRE(deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                         31, 8, Z_DEFAULT_STRATEGY) == Z_OK);
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
    zs.avail_in = static_cast<uInt>(input.size());

    std::vector<uint8_t> out(input.size() + 64);
    zs.next_out = out.data();
    zs.avail_out = static_cast<uInt>(out.size());

    REQUIRE(deflate(&zs, Z_FINISH) == Z_STREAM_END);
    out.resize(zs.total_out);
    deflateEnd(&zs);
    return out;
}
}  // namespace

TEST_CASE("Svgz::Decompress round-trips a real SVG", "[svgz]") {
    std::string svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                      "width=\"10\" height=\"10\"><rect width=\"10\" "
                      "height=\"10\" fill=\"red\"/></svg>";
    auto gz = GzipCompress(svg);

    std::vector<uint8_t> out;
    REQUIRE(Svgz::Decompress(gz.data(), gz.size(), out));
    std::string result(out.begin(), out.end());
    REQUIRE(result == svg);
}

TEST_CASE("Svgz::Decompress fails on garbage input", "[svgz]") {
    const uint8_t garbage[] = {0x1F, 0x8B, 0xFF, 0xFF, 0xFF};
    std::vector<uint8_t> out;
    REQUIRE_FALSE(Svgz::Decompress(garbage, sizeof(garbage), out));
}

TEST_CASE("Svgz::Decompress fails on empty input", "[svgz]") {
    std::vector<uint8_t> out;
    REQUIRE_FALSE(Svgz::Decompress(nullptr, 0, out));
}
