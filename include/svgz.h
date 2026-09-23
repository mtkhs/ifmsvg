#ifndef IFMSVG_SVGZ_H
#define IFMSVG_SVGZ_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace Svgz {

// Upper bound on decompressed size: a few KB of gzip can expand to gigabytes,
// and no real SVG comes near this.
constexpr size_t kMaxDecompressed = 64u << 20;

// Decompresses a gzip blob (RFC 1952) into 'out'. Returns false on any error
// (NULL input, malformed gzip, allocation failure, output beyond max_out).
// 'out' may be modified even on failure — caller should treat it as undefined
// when false is returned.
bool Decompress(const uint8_t* gz, size_t gz_len, std::vector<uint8_t>& out,
                size_t max_out = kMaxDecompressed);

}  // namespace Svgz

#endif
