#include "bmp.h"

namespace Bmp {

int CalculateLineBytes(int width, int bitsPerPixel) {
    int bytesPerPixel = (bitsPerPixel + 7) / 8;
    int lineBytes = width * bytesPerPixel;
    return (lineBytes + 3) & ~3;
}

DibHandles BuildDib(const std::vector<uint8_t>& bgr_rows,
                    int width, int height, int dpi) {
    DibHandles out{nullptr, nullptr};
    if (width <= 0 || height <= 0 || bgr_rows.empty()) return out;

    // LHND (= LMEM_MOVEABLE | LMEM_ZEROINIT): returns an opaque handle that
    // hosts must pass to LocalLock before dereferencing. Both ifmgs and ifqoi
    // use this pattern; あふw and Susie-compatible hosts expect moveable handles.
    HLOCAL hInfo = LocalAlloc(LHND, sizeof(BITMAPINFOHEADER));
    if (!hInfo) return out;

    HLOCAL hBits = LocalAlloc(LHND, bgr_rows.size());
    if (!hBits) { LocalFree(hInfo); return out; }

    auto* info = static_cast<BITMAPINFOHEADER*>(LocalLock(hInfo));
    if (!info) { LocalFree(hBits); LocalFree(hInfo); return out; }

    info->biSize          = sizeof(BITMAPINFOHEADER);
    info->biWidth         = width;
    info->biHeight        = height;   // positive = bottom-up
    info->biPlanes        = 1;
    info->biBitCount      = 24;
    info->biCompression   = BI_RGB;
    info->biSizeImage     = static_cast<DWORD>(bgr_rows.size());
    info->biXPelsPerMeter = static_cast<LONG>(dpi / 0.0254 + 0.5);
    info->biYPelsPerMeter = info->biXPelsPerMeter;
    info->biClrUsed       = 0;
    info->biClrImportant  = 0;
    LocalUnlock(hInfo);

    auto* bits = static_cast<uint8_t*>(LocalLock(hBits));
    if (!bits) { LocalFree(hBits); LocalFree(hInfo); return out; }
    memcpy(bits, bgr_rows.data(), bgr_rows.size());
    LocalUnlock(hBits);

    out.info = hInfo;
    out.bits = hBits;
    return out;
}

}  // namespace Bmp
