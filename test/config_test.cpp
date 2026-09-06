#include <catch2/catch_test_macros.hpp>
#include <windows.h>
#include <fstream>
#include <filesystem>
#include "config.h"
#include "path_utils.h"

namespace {
std::wstring WriteTempIni(const std::string& body) {
    auto tmp = std::filesystem::temp_directory_path() /
               ("ifmsvg_test_" + std::to_string(GetTickCount64()));
    std::filesystem::create_directories(tmp);
    std::ofstream(tmp / "ifmsvg.ini") << body;
    return tmp.wstring() + L"\\";
}
}  // namespace

TEST_CASE("missing INI -> all defaults", "[config]") {
    auto cfg = Config::Load(L"C:\\definitely\\not\\there\\");
    REQUIRE(cfg.dpi == 150);
    REQUIRE(cfg.background == Config::Background::Colored);
    REQUIRE(cfg.background_color == 0xFFFFFFu);
    REQUIRE(cfg.max_size == 8192);
    REQUIRE(cfg.load_system_fonts == true);
    REQUIRE(cfg.default_family == "Arial");
    REQUIRE(cfg.extra_font_dirs.empty());
}

TEST_CASE("dpi within range honored", "[config]") {
    auto dir = WriteTempIni("[render]\ndpi=300\n");
    auto cfg = Config::Load(dir);
    REQUIRE(cfg.dpi == 300);
}

TEST_CASE("dpi out of range falls back to default", "[config]") {
    auto dir = WriteTempIni("[render]\ndpi=10000\n");
    auto cfg = Config::Load(dir);
    REQUIRE(cfg.dpi == 150);
}

TEST_CASE("background=transparent parsed", "[config]") {
    auto dir = WriteTempIni("[render]\nbackground=transparent\n");
    auto cfg = Config::Load(dir);
    REQUIRE(cfg.background == Config::Background::Transparent);
}

TEST_CASE("background_color parsed as hex RRGGBB", "[config]") {
    auto dir = WriteTempIni("[render]\nbackground_color=336699\n");
    auto cfg = Config::Load(dir);
    REQUIRE(cfg.background_color == 0x336699u);
}

TEST_CASE("font.extra_dirs split on semicolons", "[config]") {
    auto dir = WriteTempIni("[font]\nextra_dirs=C:\\fonts;D:\\more\\fonts\n");
    auto cfg = Config::Load(dir);
    REQUIRE(cfg.extra_font_dirs.size() == 2);
    REQUIRE(cfg.extra_font_dirs[0] == L"C:\\fonts");
    REQUIRE(cfg.extra_font_dirs[1] == L"D:\\more\\fonts");
}

TEST_CASE("load_system_fonts=false parsed", "[config]") {
    auto dir = WriteTempIni("[font]\nload_system_fonts=false\n");
    auto cfg = Config::Load(dir);
    REQUIRE(cfg.load_system_fonts == false);
}
