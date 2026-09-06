#ifndef IFMSVG_PATH_UTILS_H
#define IFMSVG_PATH_UTILS_H

#include <windows.h>
#include <string>

namespace Path {

std::string AnsiToUtf8(const char* ansi);
std::string WideToUtf8(LPCWSTR wide);
std::wstring Utf8ToWide(const char* utf8);
std::wstring Utf8ToWide(const std::string& utf8);

// Returns the directory containing the given module (DLL/EXE), with a
// trailing backslash. Pass nullptr to get the current module's directory.
// Returns L"" on failure.
std::wstring GetModuleDirectoryW(HMODULE hModule);

}  // namespace Path

#endif
