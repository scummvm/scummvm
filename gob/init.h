/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __INIT_H
#define __INIT_H

namespace Gob {

void init_findBestCfg(void);
void init_soundVideo(int32 smallHeapSize, int16 flag);

void init_initGame(char *totFile);

}				// End of namespace Gob

#endif
