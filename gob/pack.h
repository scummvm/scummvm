/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __UNPACKER_H
#define __UNPACKER_H

namespace Gob {

int32 asm_unpackData(char *source, char *dest, char *temp);
int32 unpackData(char *source, char *dest);

}				// End of namespace Gob

#endif
