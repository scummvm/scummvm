#ifndef DETECTGAME_H
#define DETECTGAME_H

#include "definitions.h"
#include "common/stream.h"

namespace Glk {
namespace Scott {

void readHeader(uint8_t *ptr);
int parseHeader(int *h, HeaderType type, int *ni, int *na, int *nw, int *nr, int *mc, int *pr, int *tr, int *wl, int *lt, int *mn, int *trm);
GameIDType detectGame(Common::SeekableReadStream *f);
uint8_t *seekToPos(uint8_t *buf, int offset);
int seekIfNeeded(int expectedStart, int *offset, uint8_t **ptr);
int tryLoading(GameInfo info, int dictStart, int loud);
DictionaryType getId(size_t *offset);
int findCode(const char *x, int base);

} // End of namespace Scott
} // End of namespace Glk

#endif
