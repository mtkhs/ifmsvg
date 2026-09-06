#define NOMINMAX
#include <windows.h>
#include "susie.h"
#include "path_utils.h"
#include "svg_detect.h"
#include "svgz.h"
#include "config.h"
#include "svg_render.h"
#include "compositor.h"
#include "bmp.h"
#include "font_db.h"
#include <algorithm>
#include <fstream>
#include <optional>
#include <vector>
#include "dbg.h"

static HMODULE g_self = nullptr;

namespace {

const Config::Settings& GetConfig() {
    static Config::Settings cfg = []() {
        return Config::Load(Path::GetModuleDirectoryW(g_self));
    }();
    return cfg;
}

std::optional<std::vector<uint8_t>> ReadAllBytes(const std::wstring& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return std::nullopt;
    auto sz = f.tellg();
    if (sz < 0) return std::nullopt;
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    f.seekg(0);
    f.read(reinterpret_cast<char*>(buf.data()), sz);
    if (!f) return std::nullopt;
    return buf;
}

std::optional<std::vector<uint8_t>> LoadInputBytes(
    const void* buf, LONG_PTR len, unsigned int flag,
    bool isW, std::string& out_filename_utf8)
{
    if (flag & SUSIE_SOURCE_MEM) {
        if (!buf || len <= 0) return std::nullopt;
        const uint8_t* p = static_cast<const uint8_t*>(buf);
        return std::vector<uint8_t>(p, p + static_cast<size_t>(len));
    }
    std::wstring wpath;
    if (isW) {
        wpath = static_cast<LPCWSTR>(buf);
        out_filename_utf8 = Path::WideToUtf8(wpath.c_str());
    } else {
        out_filename_utf8 = Path::AnsiToUtf8(static_cast<LPCSTR>(buf));
        wpath = Path::Utf8ToWide(out_filename_utf8);
    }
    return ReadAllBytes(wpath);
}

// For disk mode, detect format and decompress SVGZ.
// For memory mode, pass through (resvg handles SVG+SVGZ natively).
std::optional<std::vector<uint8_t>> PrepareSvgBytes(
    std::vector<uint8_t> raw, const std::string& filename_utf8, bool mem_mode)
{
    if (mem_mode) return raw;

    size_t head_len = std::min(raw.size(), static_cast<size_t>(SUSIE_CHECK_SIZE));
    auto fmt = SvgDetect::Detect(filename_utf8.c_str(), raw.data(), head_len);
    if (fmt == SvgDetect::Format::Unknown) return std::nullopt;

    if (fmt == SvgDetect::Format::Svgz) {
        std::vector<uint8_t> out;
        if (!Svgz::Decompress(raw.data(), raw.size(), out)) return std::nullopt;
        return out;
    }
    return raw;
}

int RenderToHandles(const void* buf, LONG_PTR len, unsigned int flag,
                    bool isW, HLOCAL* pHBInfo, HLOCAL* pHBm)
{
    *pHBInfo = nullptr;
    *pHBm    = nullptr;
    const bool mem_mode = (flag & SUSIE_SOURCE_MEM) != 0;
    std::string filename_utf8;

    DbgLog("RenderToHandles flag=0x%x isW=%d mem=%d", flag, isW, mem_mode);

    auto raw = LoadInputBytes(buf, len, flag, isW, filename_utf8);
    if (!raw) { DbgLog("  -> FAULTREAD"); return SUSIEERROR_FAULTREAD; }
    DbgLog("  file=%s raw=%zu bytes", filename_utf8.c_str(), raw->size());

    auto svg = PrepareSvgBytes(std::move(*raw), filename_utf8, mem_mode);
    if (!svg) { DbgLog("  -> UNKNOWNFORMAT"); return SUSIEERROR_UNKNOWNFORMAT; }
    DbgLog("  svg=%zu bytes", svg->size());

    const auto& cfg = GetConfig();
    DbgLog("  dpi=%d max_size=%d load_sys_fonts=%d", cfg.dpi, cfg.max_size, cfg.load_system_fonts);
    auto pix = SvgRender::Render(svg->data(), svg->size(), cfg);
    if (!pix) { DbgLog("  -> BROKENDATA (Render failed)"); return SUSIEERROR_BROKENDATA; }
    DbgLog("  rendered %dx%d", pix->width, pix->height);

    auto bgr = Compositor::ToDibBgr24(*pix, cfg);
    auto handles = Bmp::BuildDib(bgr, pix->width, pix->height, cfg.dpi);
    if (!handles.info || !handles.bits) { DbgLog("  -> FAULTMEMORY"); return SUSIEERROR_FAULTMEMORY; }

    *pHBInfo = handles.info;
    *pHBm    = handles.bits;
    DbgLog("  -> OK hInfo=%p hBm=%p", handles.info, handles.bits);
    return SUSIEERROR_NOERROR;
}

int GetPictureInfoImpl(const void* buf, LONG_PTR len, unsigned int flag,
                       bool isW, SUSIE_PICTUREINFO* lpInfo)
{
    const bool mem_mode = (flag & SUSIE_SOURCE_MEM) != 0;
    std::string filename_utf8;

    DbgLog("GetPictureInfoImpl flag=0x%x isW=%d mem=%d", flag, isW, mem_mode);

    auto raw = LoadInputBytes(buf, len, flag, isW, filename_utf8);
    if (!raw) { DbgLog("  -> FAULTREAD"); return SUSIEERROR_FAULTREAD; }
    DbgLog("  file=%s raw=%zu bytes", filename_utf8.c_str(), raw->size());

    auto svg = PrepareSvgBytes(std::move(*raw), filename_utf8, mem_mode);
    if (!svg) { DbgLog("  -> UNKNOWNFORMAT"); return SUSIEERROR_UNKNOWNFORMAT; }

    const auto& cfg = GetConfig();
    auto dims = SvgRender::GetDimensions(svg->data(), svg->size(), cfg);
    if (!dims) { DbgLog("  -> BROKENDATA"); return SUSIEERROR_BROKENDATA; }

    lpInfo->left       = 0;
    lpInfo->top        = 0;
    lpInfo->width      = dims->width;
    lpInfo->height     = dims->height;
    lpInfo->x_density  = 0;
    lpInfo->y_density  = 0;
    lpInfo->colorDepth = 24;
    lpInfo->hInfo      = nullptr;
    DbgLog("  -> OK %dx%d", dims->width, dims->height);
    return SUSIEERROR_NOERROR;
}

}  // namespace

static const char* kInfoA[] = { "00IN", "SVG/SVGZ Plug-in Version 0.1 (C) mtkhs", "*.svg;*.svgz", "SVG/SVGZ" };
static constexpr int kInfoCount = 4;

extern "C" {

// All exported functions wrap their body in try/catch(...).
// The TU is compiled with /EHa so catch(...) also catches SEH exceptions
// (e.g. Rust panics propagated as Windows structured exceptions).

int __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen) {
    try {
        if (!buf || buflen <= 0) return 0;
        if (infono < 0 || infono >= kInfoCount) { buf[0] = '\0'; return 0; }
        int n = static_cast<int>(strlen(kInfoA[infono]));
        if (n >= buflen) n = buflen - 1;
        memcpy(buf, kInfoA[infono], n);
        buf[n] = '\0';
        return n;
    } catch (...) { return 0; }
}

int __stdcall GetPluginInfoW(int infono, LPWSTR buf, int buflen) {
    try {
        if (!buf || buflen <= 0) return 0;
        char bufA[256];
        int n = GetPluginInfo(infono, bufA, sizeof(bufA));
        if (n == 0) { buf[0] = L'\0'; return 0; }
        MultiByteToWideChar(CP_ACP, 0, bufA, -1, buf, buflen);
        buf[buflen - 1] = L'\0';
        return static_cast<int>(wcslen(buf));
    } catch (...) { return 0; }
}

// In the Susie spec, IsSupported's dw is either a pointer to the first
// SUSIE_CHECK_SIZE bytes, or a Windows HANDLE (small integer) cast to void*.
// Guard: user-mode addresses are always above 64 KB on Windows.
static const uint8_t* ToHeadPtr(const void* dw) {
    return (reinterpret_cast<uintptr_t>(dw) > 0xFFFF)
        ? static_cast<const uint8_t*>(dw)
        : nullptr;
}

int __stdcall IsSupported(LPCSTR filename, const void* dw) {
    try {
        if (!filename) return 0;
        std::string utf8 = Path::AnsiToUtf8(filename);
        auto fmt = SvgDetect::Detect(utf8.c_str(), ToHeadPtr(dw), SUSIE_CHECK_SIZE);
        int r = (fmt != SvgDetect::Format::Unknown) ? 1 : 0;
        DbgLog("IsSupported(%s) dw=%p -> %d", utf8.c_str(), dw, r);
        return r;
    } catch (...) { DbgLog("IsSupported -> EXCEPTION"); return 0; }
}

int __stdcall IsSupportedW(LPCWSTR filename, const void* dw) {
    try {
        if (!filename) return 0;
        std::string utf8 = Path::WideToUtf8(filename);
        auto fmt = SvgDetect::Detect(utf8.c_str(), ToHeadPtr(dw), SUSIE_CHECK_SIZE);
        int r = (fmt != SvgDetect::Format::Unknown) ? 1 : 0;
        DbgLog("IsSupportedW(%s) dw=%p -> %d", utf8.c_str(), dw, r);
        return r;
    } catch (...) { DbgLog("IsSupportedW -> EXCEPTION"); return 0; }
}

int __stdcall GetPictureInfo(LPCSTR buf, LONG_PTR len, unsigned int flag,
                             SUSIE_PICTUREINFO* lpInfo) {
    try {
        DbgLog("GetPictureInfo buf=%s len=%Id flag=0x%x", buf ? buf : "(null)", len, flag);
        if (!buf || !lpInfo) return SUSIEERROR_INTERNAL;
        int r = GetPictureInfoImpl(buf, len, flag, false, lpInfo);
        DbgLog("GetPictureInfo -> %d", r);
        return r;
    } catch (...) { DbgLog("GetPictureInfo -> EXCEPTION"); return SUSIEERROR_INTERNAL; }
}

int __stdcall GetPictureInfoW(LPCWSTR buf, LONG_PTR len, unsigned int flag,
                              SUSIE_PICTUREINFO* lpInfo) {
    try {
        std::string u = buf ? Path::WideToUtf8(buf) : "(null)";
        DbgLog("GetPictureInfoW buf=%s len=%Id flag=0x%x", u.c_str(), len, flag);
        if (!buf || !lpInfo) return SUSIEERROR_INTERNAL;
        int r = GetPictureInfoImpl(buf, len, flag, true, lpInfo);
        DbgLog("GetPictureInfoW -> %d", r);
        return r;
    } catch (...) { DbgLog("GetPictureInfoW -> EXCEPTION"); return SUSIEERROR_INTERNAL; }
}

int __stdcall GetPicture(LPCSTR buf, LONG_PTR len, unsigned int flag,
                         HLOCAL* pHBInfo, HLOCAL* pHBm,
                         SUSIE_PROGRESS, LONG_PTR) {
    try {
        DbgLog("GetPicture buf=%s len=%Id flag=0x%x", buf ? buf : "(null)", len, flag);
        if (!buf || !pHBInfo || !pHBm) return SUSIEERROR_INTERNAL;
        int r = RenderToHandles(buf, len, flag, false, pHBInfo, pHBm);
        DbgLog("GetPicture -> %d", r);
        return r;
    } catch (...) { DbgLog("GetPicture -> EXCEPTION"); return SUSIEERROR_INTERNAL; }
}

int __stdcall GetPictureW(LPCWSTR buf, LONG_PTR len, unsigned int flag,
                          HLOCAL* pHBInfo, HLOCAL* pHBm,
                          SUSIE_PROGRESS, LONG_PTR) {
    try {
        std::string u = buf ? Path::WideToUtf8(buf) : "(null)";
        DbgLog("GetPictureW buf=%s len=%Id flag=0x%x", u.c_str(), len, flag);
        if (!buf || !pHBInfo || !pHBm) return SUSIEERROR_INTERNAL;
        int r = RenderToHandles(buf, len, flag, true, pHBInfo, pHBm);
        DbgLog("GetPictureW -> %d", r);
        return r;
    } catch (...) { DbgLog("GetPictureW -> EXCEPTION"); return SUSIEERROR_INTERNAL; }
}

int __stdcall GetPreview(LPCSTR, LONG_PTR, unsigned int, HLOCAL*, HLOCAL*,
                         SUSIE_PROGRESS, LONG_PTR) {
    return SUSIEERROR_NOTSUPPORT;
}

int __stdcall GetPreviewW(LPCWSTR, LONG_PTR, unsigned int, HLOCAL*, HLOCAL*,
                          SUSIE_PROGRESS, LONG_PTR) {
    return SUSIEERROR_NOTSUPPORT;
}

int __stdcall ConfigurationDlg(HWND, int) {
    return SUSIEERROR_NOTSUPPORT;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        g_self = hInst;
        DbgLog("DLL_PROCESS_ATTACH hInst=%p", hInst);
    } else if (reason == DLL_PROCESS_DETACH) {
        DbgLog("DLL_PROCESS_DETACH");
        FontDb::Destroy();
    }
    return TRUE;
}

}  // extern "C"
