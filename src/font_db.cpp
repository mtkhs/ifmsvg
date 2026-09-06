#define NOMINMAX
#include "font_db.h"
#include "path_utils.h"
#include "dbg.h"
#include <resvg.h>
#include <atomic>
#include <filesystem>

namespace FontDb {

namespace {

// Use atomic flag instead of std::call_once to avoid call_once/SEH interaction.
// If a Rust panic (SEH exception) unwinds through call_once's internal lock on
// MSVC, the lock state becomes corrupted and subsequent calls deadlock or throw.
// Atomic compare-exchange is immune to this: it commits before any resvg call.
std::atomic<int>            g_state{0};  // 0=not started, 1=done
std::atomic<resvg_options*> g_opts{nullptr};

void LoadFontsFromDir(resvg_options* opts, const std::wstring& dir_w) noexcept {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dir_w)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().wstring();
            for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
            if (ext != L".ttf" && ext != L".otf" && ext != L".ttc") continue;
            std::string path_u8 = Path::WideToUtf8(entry.path().wstring().c_str());
            if (!path_u8.empty()) {
                try { resvg_options_load_font_file(opts, path_u8.c_str()); } catch (...) {}
            }
        }
    } catch (...) {}
}

// noexcept: must NEVER propagate any exception (C++ or SEH).
// This ensures call-site atomic compare-exchange works correctly and
// the once-flag can be set without corrupting internal lock state.
void Init(const Config::Settings& cfg) noexcept {
    DbgLog("FontDb::Init dpi=%d lsf=%d", cfg.dpi, cfg.load_system_fonts);
    resvg_options* opts = nullptr;
    try {
        opts = resvg_options_create();
        if (!opts) return;

        try { resvg_options_set_dpi(opts, static_cast<float>(cfg.dpi)); } catch (...) {}

        if (cfg.load_system_fonts) {
            try { resvg_options_load_system_fonts(opts); } catch (...) {}
        }

        for (const auto& dir_w : cfg.extra_font_dirs) {
            LoadFontsFromDir(opts, dir_w);
        }

        if (!cfg.default_family.empty()) {
            try { resvg_options_set_font_family(opts, cfg.default_family.c_str()); } catch (...) {}
        }

        g_opts.store(opts, std::memory_order_release);
        DbgLog("FontDb::Init done");
    } catch (...) {
        DbgLog("FontDb::Init EXCEPTION");
        if (opts) resvg_options_destroy(opts);
    }
}

}  // namespace

resvg_options* Get(const Config::Settings& cfg) {
    // Fast path: already initialized.
    if (g_state.load(std::memory_order_acquire) != 0)
        return g_opts.load(std::memory_order_acquire);

    // Race to be the initializer (compare_exchange only succeeds for one caller).
    int expected = 0;
    if (g_state.compare_exchange_strong(expected, 1,
                                        std::memory_order_acq_rel,
                                        std::memory_order_acquire)) {
        // We won the race: initialize, then mark done.
        // g_state is already 1 (done) from compare_exchange — Init stores g_opts.
        Init(cfg);
    }
    // Either we just ran Init, or another caller already did.
    return g_opts.load(std::memory_order_acquire);
}

void Destroy() {
    auto* p = g_opts.exchange(nullptr, std::memory_order_acq_rel);
    if (p) resvg_options_destroy(p);
}

}  // namespace FontDb
