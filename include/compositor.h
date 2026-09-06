#ifndef IFMSVG_COMPOSITOR_H
#define IFMSVG_COMPOSITOR_H

#include <cstdint>
#include <vector>
#include "svg_render.h"
#include "config.h"

namespace Compositor {

// RGBA premultiplied (top-down) -> 24bit BGR, 4-byte aligned rows, bottom-up
// (Windows DIB convention). The chosen background (colored solid or transparent
// checkerboard) is composited under transparent pixels.
std::vector<uint8_t> ToDibBgr24(const SvgRender::Pixmap& src,
                                const Config::Settings& cfg);

}  // namespace Compositor

#endif
