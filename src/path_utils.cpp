#include "path_utils.h"

namespace Path {

std::string WideToUtf8(LPCWSTR wide) {
    if (!wide) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
    if (n <= 1) return "";
    std::string out(n - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &out[0], n, NULL, NULL);
    return out;
}

std::wstring Utf8ToWide(const char* utf8) {
    if (!utf8 || !*utf8) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    if (n <= 1) return L"";
    std::wstring out(n - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &out[0], n);
    return out;
}

std::wstring Utf8ToWide(const std::string& utf8) {
    return Utf8ToWide(utf8.c_str());
}

std::string AnsiToUtf8(const char* ansi) {
    if (!ansi) return "";
    int wn = MultiByteToWideChar(CP_ACP, 0, ansi, -1, NULL, 0);
    if (wn <= 1) return "";
    std::wstring w(wn - 1, 0);
    MultiByteToWideChar(CP_ACP, 0, ansi, -1, &w[0], wn);
    return WideToUtf8(w.c_str());
}

std::wstring GetModuleDirectoryW(HMODULE hModule) {
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(hModule, buf, MAX_PATH);
    if (n == 0 || n == MAX_PATH) return L"";
    std::wstring path(buf, n);
    size_t slash = path.find_last_of(L"\\/");
    if (slash == std::wstring::npos) return L"";
    return path.substr(0, slash + 1);
}

}  // namespace Path
