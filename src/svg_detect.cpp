#include "svg_detect.h"
#include <cctype>
#include <cstring>
#include <string>

namespace SvgDetect {

namespace {

bool ExtensionEquals(const char* filename, const char* ext_lower) {
    if (!filename) return false;
    const char* dot = strrchr(filename, '.');
    if (!dot) return false;
    size_t flen = strlen(dot + 1);
    if (flen != strlen(ext_lower)) return false;
    for (size_t i = 0; i < flen; ++i) {
        if (std::tolower(static_cast<unsigned char>(dot[1 + i])) != ext_lower[i])
            return false;
    }
    return true;
}

bool LooksLikeSvgText(const uint8_t* head, size_t len) {
    if (!head || len < 4) return false;
    // Skip optional UTF-8 BOM.
    size_t off = 0;
    if (len >= 3 && head[0] == 0xEF && head[1] == 0xBB && head[2] == 0xBF) off = 3;
    // Skip leading whitespace.
    while (off < len && (head[off] == ' ' || head[off] == '\t' ||
                         head[off] == '\r' || head[off] == '\n')) ++off;
    if (off >= len) return false;
    // Accept "<?xml" or "<svg" (case-insensitive on the tag name).
    if (head[off] != '<') return false;
    if (off + 4 < len && head[off+1] == '?' &&
        std::tolower(head[off+2]) == 'x' &&
        std::tolower(head[off+3]) == 'm' &&
        std::tolower(head[off+4]) == 'l') return true;
    if (off + 3 < len &&
        std::tolower(head[off+1]) == 's' &&
        std::tolower(head[off+2]) == 'v' &&
        std::tolower(head[off+3]) == 'g') return true;
    return false;
}

bool LooksLikeGzip(const uint8_t* head, size_t len) {
    return head && len >= 2 && head[0] == 0x1F && head[1] == 0x8B;
}

}  // namespace

Format Detect(const char* filename, const uint8_t* head, size_t head_len) noexcept {
    const bool is_svg_ext  = ExtensionEquals(filename, "svg");
    const bool is_svgz_ext = ExtensionEquals(filename, "svgz");
    if (!is_svg_ext && !is_svgz_ext) return Format::Unknown;

    const bool no_head = !head || head_len == 0;

    if (is_svgz_ext) {
        // .svgz must be gzip-wrapped to be valid.
        if (LooksLikeGzip(head, head_len)) return Format::Svgz;
        if (no_head) return Format::Svgz;  // trust the extension
        return Format::Unknown;            // .svgz with non-gzip content
    }

    // .svg extension
    if (LooksLikeGzip(head, head_len)) return Format::Svgz;  // misnamed but ok
    if (no_head) return Format::Svg;                          // trust extension
    if (LooksLikeSvgText(head, head_len)) return Format::Svg;
    return Format::Unknown;
}

}  // namespace SvgDetect
