#ifndef DECOMPRESSZ80_H
#define DECOMPRESSZ80_H

#include "common/scummsys.h"

namespace Glk {
namespace Scott {

// Will return nullptr on error or 0xc000 (49152) bytes of uncompressed raw data on success
uint8_t *decompressZ80(uint8_t *rawData, size_t length);

} // End of namespace Scott
} // End of namespace Glk

#endif
