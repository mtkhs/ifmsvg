#ifndef IFMSVG_BMP_H
#define IFMSVG_BMP_H

#include <windows.h>
#include <cstdint>
#include <vector>

namespace Bmp {

// 4-byte aligned scan-line stride (Windows DIB convention).
int CalculateLineBytes(int width, int bitsPerPixel = 24);

// Build a Susie-compatible DIB:
//   info -> HLOCAL holding BITMAPINFOHEADER (no color table — BI_RGB 24bpp)
//   bits -> HLOCAL holding pixel data (bgr_rows passed straight through)
// dpi is converted to pixels-per-meter for biXPelsPerMeter / biYPelsPerMeter.
// On failure (allocation failure, zero size), both members are nullptr.
struct DibHandles { HLOCAL info; HLOCAL bits; };
DibHandles BuildDib(const std::vector<uint8_t>& bgr_rows,
                    int width, int height, int dpi);

}  // namespace Bmp

#endif
