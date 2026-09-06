#ifndef IFMSVG_CONFIG_H
#define IFMSVG_CONFIG_H

#include <cstdint>
#include <string>
#include <vector>

namespace Config {

enum class Background { Colored, Transparent };

struct Settings {
    int dpi = 150;
    Background background = Background::Colored;
    uint32_t background_color = 0xFFFFFFu;  // 0xRRGGBB
    int max_size = 8192;
    bool load_system_fonts = true;
    std::string default_family = "Arial";
    std::vector<std::wstring> extra_font_dirs;
};

// Load <dir>\ifmsvg.ini if it exists. Missing file or unparseable values
// fall back to per-field defaults; the call always returns a usable struct.
Settings Load(const std::wstring& dll_dir);

}  // namespace Config

#endif
