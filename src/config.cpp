#include "config.h"
#include "path_utils.h"
#include <windows.h>
#include <cwchar>
#include <cstdlib>
#include <algorithm>

namespace Config {

namespace {

constexpr int kDpiMin = 30;
constexpr int kDpiMax = 1200;

std::wstring ReadString(LPCWSTR section, LPCWSTR key, LPCWSTR def, LPCWSTR file) {
    wchar_t buf[1024];
    GetPrivateProfileStringW(section, key, def, buf, 1024, file);
    return std::wstring(buf);
}

int ReadInt(LPCWSTR section, LPCWSTR key, int def, LPCWSTR file) {
    return GetPrivateProfileIntW(section, key, def, file);
}

std::vector<std::wstring> Split(const std::wstring& s, wchar_t sep) {
    std::vector<std::wstring> out;
    size_t i = 0;
    while (i < s.size()) {
        size_t j = s.find(sep, i);
        if (j == std::wstring::npos) j = s.size();
        if (j > i) out.emplace_back(s.substr(i, j - i));
        i = j + 1;
    }
    return out;
}

uint32_t ParseHexRgb(const std::wstring& hex, uint32_t fallback) {
    if (hex.size() != 6) return fallback;
    wchar_t* end = nullptr;
    unsigned long v = wcstoul(hex.c_str(), &end, 16);
    if (!end || *end != L'\0') return fallback;
    return static_cast<uint32_t>(v & 0xFFFFFF);
}

}  // namespace

Settings Load(const std::wstring& dll_dir) {
    Settings cfg;  // defaults

    std::wstring ini = dll_dir + L"ifmsvg.ini";
    if (GetFileAttributesW(ini.c_str()) == INVALID_FILE_ATTRIBUTES) return cfg;
    LPCWSTR f = ini.c_str();

    // [render]
    int dpi = ReadInt(L"render", L"dpi", cfg.dpi, f);
    if (dpi >= kDpiMin && dpi <= kDpiMax) cfg.dpi = dpi;

    auto bg = ReadString(L"render", L"background", L"colored", f);
    if (bg == L"transparent") cfg.background = Background::Transparent;
    else                      cfg.background = Background::Colored;

    auto color = ReadString(L"render", L"background_color", L"FFFFFF", f);
    cfg.background_color = ParseHexRgb(color, cfg.background_color);

    int max_size = ReadInt(L"render", L"max_size", cfg.max_size, f);
    if (max_size >= 0) cfg.max_size = max_size;

    // [font]
    auto load_fonts = ReadString(L"font", L"load_system_fonts", L"true", f);
    cfg.load_system_fonts = !(load_fonts == L"false" || load_fonts == L"0");

    auto fam = ReadString(L"font", L"default_family", L"Arial", f);
    cfg.default_family = Path::WideToUtf8(fam.c_str());
    if (cfg.default_family.empty()) cfg.default_family = "Arial";

    auto dirs = ReadString(L"font", L"extra_dirs", L"", f);
    if (!dirs.empty()) cfg.extra_font_dirs = Split(dirs, L';');

    return cfg;
}

}  // namespace Config
