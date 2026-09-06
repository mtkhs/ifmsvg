#ifndef IFMSVG_SVG_DETECT_H
#define IFMSVG_SVG_DETECT_H

#include <cstdint>
#include <cstddef>

namespace SvgDetect {

enum class Format { Unknown, Svg, Svgz };

// Detect SVG format from filename extension + leading bytes (head/head_len may
// be NULL/0 to skip content sniffing — extension alone then determines result).
// Extension matching is case-insensitive.
//   - gzip magic (1F 8B) -> Svgz (overrides extension)
//   - .svg ext + no garbage -> Svg
//   - .svgz ext -> Svgz
//   - otherwise Unknown
Format Detect(const char* filename_utf8, const uint8_t* head, size_t head_len) noexcept;

}  // namespace SvgDetect

#endif
