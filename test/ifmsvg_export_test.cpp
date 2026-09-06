#include <catch2/catch_test_macros.hpp>
#include <windows.h>
#include <filesystem>

namespace {
std::wstring SphPath() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::filesystem::path p(buf);
    return (p.parent_path() / "ifmsvg.sph").wstring();
}
}

TEST_CASE("plugin DLL exports GetPluginInfo with 00IN signature", "[plugin]") {
    HMODULE h = LoadLibraryW(SphPath().c_str());
    REQUIRE(h != nullptr);

    using Fn = int(__stdcall*)(int, LPSTR, int);
    auto get_info = reinterpret_cast<Fn>(GetProcAddress(h, "GetPluginInfo"));
    REQUIRE(get_info != nullptr);

    char buf[64] = {};
    int n = get_info(0, buf, sizeof(buf));
    REQUIRE(n == 4);
    REQUIRE(std::string(buf) == "00IN");

    n = get_info(2, buf, sizeof(buf));
    REQUIRE(std::string(buf) == "*.svg;*.svgz");

    FreeLibrary(h);
}

TEST_CASE("IsSupported accepts .svg and rejects .png", "[plugin]") {
    HMODULE h = LoadLibraryW(SphPath().c_str());
    REQUIRE(h != nullptr);

    using Fn = int(__stdcall*)(LPCSTR, const void*);
    auto is_sup = reinterpret_cast<Fn>(GetProcAddress(h, "IsSupported"));
    REQUIRE(is_sup != nullptr);

    const char head[] = "<?xml version=\"1.0\"?><svg/>";
    REQUIRE(is_sup("foo.svg", head) == 1);
    REQUIRE(is_sup("foo.png", head) == 0);

    FreeLibrary(h);
}
