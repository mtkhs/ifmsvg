#include <catch2/catch_test_macros.hpp>
#include <windows.h>
#include "bmp.h"

TEST_CASE("CalculateLineBytes pads to 4-byte multiple", "[bmp]") {
    REQUIRE(Bmp::CalculateLineBytes(1, 24) == 4);    // 3 -> 4
    REQUIRE(Bmp::CalculateLineBytes(2, 24) == 8);    // 6 -> 8
    REQUIRE(Bmp::CalculateLineBytes(4, 24) == 12);   // already aligned
    REQUIRE(Bmp::CalculateLineBytes(100, 24) == 300);
}

TEST_CASE("BuildDib writes header + bits with correct dimensions", "[bmp]") {
    // 2x2 image, BGR rows, already padded (line bytes = 8 for 24bpp 2px).
    std::vector<uint8_t> bgr_rows(8 * 2, 0xAB);
    auto h = Bmp::BuildDib(bgr_rows, 2, 2, 96);

    REQUIRE(h.info != nullptr);
    REQUIRE(h.bits != nullptr);

    auto* info = static_cast<BITMAPINFOHEADER*>(LocalLock(h.info));
    REQUIRE(info != nullptr);
    REQUIRE(info->biSize == sizeof(BITMAPINFOHEADER));
    REQUIRE(info->biWidth == 2);
    REQUIRE(info->biHeight == 2);
    REQUIRE(info->biPlanes == 1);
    REQUIRE(info->biBitCount == 24);
    REQUIRE(info->biCompression == BI_RGB);
    LocalUnlock(h.info);

    auto* bits = static_cast<uint8_t*>(LocalLock(h.bits));
    REQUIRE(bits != nullptr);
    REQUIRE(bits[0] == 0xAB);
    REQUIRE(bits[15] == 0xAB);
    LocalUnlock(h.bits);

    LocalFree(h.info);
    LocalFree(h.bits);
}

TEST_CASE("BuildDib returns nulls for zero-sized input", "[bmp]") {
    std::vector<uint8_t> empty;
    auto h = Bmp::BuildDib(empty, 0, 0, 96);
    REQUIRE(h.info == nullptr);
    REQUIRE(h.bits == nullptr);
}
