/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __RESOURCE_H
#define __RESOURCE_H

namespace Gob {

void res_Free(void);
void res_Init(void);
void res_Search(char resid);
void res_Get(char *buf);

}				// End of namespace Gob

#endif
