#ifndef IFMSVG_SVGZ_H
#define IFMSVG_SVGZ_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace Svgz {

// Decompresses a gzip blob (RFC 1952) into 'out'. Returns false on any error
// (NULL input, malformed gzip, allocation failure). 'out' may be modified
// even on failure — caller should treat it as undefined when false is returned.
bool Decompress(const uint8_t* gz, size_t gz_len, std::vector<uint8_t>& out);

}  // namespace Svgz

#endif
