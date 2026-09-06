#define NOMINMAX
#include <windows.h>
#include "svg_render.h"
#include "config.h"
#include "font_db.h"
#include "dbg.h"
#include <resvg.h>
#include <algorithm>
#include <cmath>
#include <float.h>

// SEH filter: log code and handle.
static int SehLogFilter([[maybe_unused]] DWORD code) noexcept {
    DbgLog("  resvg: SEH code=0x%08X", (unsigned)code);
    return EXCEPTION_EXECUTE_HANDLER;
}

// Mask all FP exceptions and return the previous control word.
//
// Some host applications (e.g. あふw) unmask FP exceptions via _controlfp_s.
// Rust code in resvg performs FP operations that then raise
// STATUS_FLOAT_INVALID_OPERATION (0xC0000090) and crash the call.
static unsigned int MaskFpExceptions() noexcept {
    unsigned int saved = 0;
    _controlfp_s(&saved, _CW_DEFAULT, _MCW_EM);
    return saved;
}

static void RestoreFpExceptions(unsigned int saved) noexcept {
    _controlfp_s(nullptr, saved, _MCW_EM);
}

// Plain wrapper around resvg_parse_tree_from_data.
// No C++ objects with destructors -- required for __try/__except on MSVC.
static bool DoParseTree(const char* data, size_t len,
                        resvg_options* opts,
                        resvg_render_tree** out_tree) noexcept {
    *out_tree = nullptr;
    unsigned int fp_save = MaskFpExceptions();

    bool ok = false;
    __try {
        int32_t rc = resvg_parse_tree_from_data(data, len, opts, out_tree);
        ok = rc == 0 && *out_tree != nullptr;
    } __except(SehLogFilter(GetExceptionCode())) {
        ok = false;
        *out_tree = nullptr;
    }

    RestoreFpExceptions(fp_save);
    return ok;
}

// Plain wrapper around resvg_render.
static void DoRender(resvg_render_tree* tree, resvg_transform xform,
                     uint32_t w, uint32_t h, char* pixbuf) noexcept {
    unsigned int fp_save = MaskFpExceptions();
    __try {
        resvg_render(tree, xform, w, h, pixbuf);
    } __except(SehLogFilter(GetExceptionCode())) {}
    RestoreFpExceptions(fp_save);
}

namespace SvgRender {

namespace {

struct TreeOwner {
    resvg_render_tree* t = nullptr;
    ~TreeOwner() { if (t) resvg_tree_destroy(t); }
};

// Compute pixel dimensions from the SVG's intrinsic size and DPI, clamping
// by cfg.max_size. CSS px = 1/96 inch.
void ComputeOutputSize(float svg_w, float svg_h, const Config::Settings& cfg,
                       int& out_w, int& out_h) {
    const double scale = static_cast<double>(cfg.dpi) / 96.0;
    double w = svg_w * scale;
    double h = svg_h * scale;
    if (cfg.max_size > 0) {
        const double m = static_cast<double>(cfg.max_size);
        if (w > m || h > m) {
            const double k = std::min(m / w, m / h);
            w *= k; h *= k;
        }
    }
    out_w = std::max(1, static_cast<int>(std::round(w)));
    out_h = std::max(1, static_cast<int>(std::round(h)));
}

bool ParseTree(const uint8_t* svg_bytes, size_t len,
               const Config::Settings& cfg, TreeOwner& tree) {
    resvg_options* opts = FontDb::Get(cfg);
    if (!opts) return false;
    return DoParseTree(reinterpret_cast<const char*>(svg_bytes), len, opts, &tree.t);
}

}  // namespace

std::optional<Pixmap> Render(const uint8_t* svg_bytes, size_t len,
                             const Config::Settings& cfg) {
    if (!svg_bytes || len == 0) return std::nullopt;
    TreeOwner tree;
    if (!ParseTree(svg_bytes, len, cfg, tree)) return std::nullopt;

    resvg_size sz = resvg_get_image_size(tree.t);
    int w, h;
    ComputeOutputSize(sz.width, sz.height, cfg, w, h);

    Pixmap pix;
    pix.width = w;
    pix.height = h;
    pix.rgba.assign(static_cast<size_t>(w) * h * 4, 0);

    // resvg_render draws at 1 CSS-px = 1 physical pixel with identity transform.
    // Apply explicit scale so SVG content fills the DPI-scaled output buffer.
    resvg_transform xform = resvg_transform_identity();
    if (sz.width > 0.0f && sz.height > 0.0f) {
        xform.a = static_cast<float>(w) / sz.width;
        xform.d = static_cast<float>(h) / sz.height;
    }
    DoRender(tree.t, xform,
             static_cast<uint32_t>(w), static_cast<uint32_t>(h),
             reinterpret_cast<char*>(pix.rgba.data()));
    return pix;
}

std::optional<Dimensions> GetDimensions(const uint8_t* svg_bytes, size_t len,
                                        const Config::Settings& cfg) {
    if (!svg_bytes || len == 0) return std::nullopt;
    TreeOwner tree;
    if (!ParseTree(svg_bytes, len, cfg, tree)) return std::nullopt;

    resvg_size sz = resvg_get_image_size(tree.t);
    int w, h;
    ComputeOutputSize(sz.width, sz.height, cfg, w, h);

    Dimensions d{w, h, cfg.dpi, cfg.dpi};
    return d;
}

}  // namespace SvgRender
