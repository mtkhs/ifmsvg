#include <catch2/catch_test_macros.hpp>
#include "path_utils.h"

TEST_CASE("Utf8ToWide round-trips ASCII", "[path_utils]") {
    auto w = Path::Utf8ToWide("hello");
    REQUIRE(w == L"hello");
}

TEST_CASE("Utf8ToWide round-trips Japanese", "[path_utils]") {
    // "テスト" in UTF-8: E3 83 86 E3 82 B9 E3 83 88
    const char* utf8 = "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88";
    auto w = Path::Utf8ToWide(utf8);
    REQUIRE(w == L"テスト");
}

TEST_CASE("WideToUtf8 round-trips Japanese", "[path_utils]") {
    auto u = Path::WideToUtf8(L"テスト");
    REQUIRE(u == "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88");
}

TEST_CASE("Utf8ToWide on empty/null returns empty", "[path_utils]") {
    REQUIRE(Path::Utf8ToWide(static_cast<const char*>(nullptr)) == L"");
    REQUIRE(Path::Utf8ToWide("") == L"");
}

TEST_CASE("WideToUtf8 on null returns empty", "[path_utils]") {
    REQUIRE(Path::WideToUtf8(nullptr) == "");
}

TEST_CASE("GetPluginDirectory returns a non-empty path ending with separator", "[path_utils]") {
    auto dir = Path::GetModuleDirectoryW(nullptr);
    REQUIRE_FALSE(dir.empty());
    REQUIRE((dir.back() == L'\\' || dir.back() == L'/'));
}
