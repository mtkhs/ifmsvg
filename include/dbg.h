#pragma once

#ifdef _DEBUG

#include <windows.h>
#include <cstdio>
#include <cstdarg>

inline void DbgLog(const char* fmt, ...) {
    char path[MAX_PATH];
    GetTempPathA(MAX_PATH, path);
    strcat_s(path, "ifmsvg_debug.txt");
    FILE* f = nullptr;
    fopen_s(&f, path, "a");
    if (!f) return;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f);
    fclose(f);
}

#else
#define DbgLog(...) ((void)0)
#endif
