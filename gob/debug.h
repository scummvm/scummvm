/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __MY_DEBUG_H
#define __MY_DEBUG_H

#define LOG_NAME	"log.txt"

#include "scenery.h"

namespace Gob {

void log_write(const char *format, ...);

void dbg_dumpMem(char *ptr, int16 size);

void dbg_dumpAnimation(Scen_Animation *anim);
void dbg_dumpFramePiece(Scen_AnimFramePiece *piece, int16 j,
    Scen_AnimLayer *layer);

}				// End of namespace Gob

#endif
