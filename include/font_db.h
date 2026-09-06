#ifndef IFMSVG_FONT_DB_H
#define IFMSVG_FONT_DB_H

#include "config.h"

// Forward-declare resvg_options as an opaque pointer so this header
// stays cheap to include.
extern "C" struct resvg_options;

namespace FontDb {

// Returns a process-wide resvg_options* configured per cfg. The first call
// performs the (potentially expensive) system font scan; subsequent calls
// return the same pointer instantly. Thread-safe.
//
// Subsequent calls IGNORE cfg — the very first call wins. This is intentional:
// fonts are an expensive per-process resource and the plugin is single-config
// per process anyway. Callers must NOT destroy the returned pointer.
resvg_options* Get(const Config::Settings& cfg);

// Called from DllMain DLL_PROCESS_DETACH to release the singleton.
void Destroy();

}  // namespace FontDb

#endif
