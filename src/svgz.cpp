#include "svgz.h"
#include <zlib.h>
#include <cstring>

namespace Svgz {

bool Decompress(const uint8_t* gz, size_t gz_len, std::vector<uint8_t>& out) {
    if (!gz || gz_len == 0) return false;

    z_stream zs{};
    // windowBits=31 -> raw deflate (15) + gzip wrapper (+16).
    if (inflateInit2(&zs, 31) != Z_OK) return false;

    zs.next_in  = const_cast<Bytef*>(gz);  // zlib C API; inflate does not modify input
    zs.avail_in = static_cast<uInt>(gz_len);  // zlib uInt is 32-bit; SVGZ > 4 GB unsupported

    out.clear();
    constexpr size_t CHUNK = 64 * 1024;
    std::vector<uint8_t> buf(CHUNK);

    int rc;
    do {
        zs.next_out  = buf.data();
        zs.avail_out = static_cast<uInt>(buf.size());
        rc = inflate(&zs, Z_NO_FLUSH);
        if (rc != Z_OK && rc != Z_STREAM_END) {
            inflateEnd(&zs);
            return false;
        }
        out.insert(out.end(), buf.data(), buf.data() + (buf.size() - zs.avail_out));
    } while (rc != Z_STREAM_END);

    inflateEnd(&zs);
    return true;
}

}  // namespace Svgz
