/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __TIMER_H_
#define __TIMER_H_

namespace Gob {

typedef void (* TickHandler) (void);

void timer_enableTimer(void);
void timer_disableTimer(void);
void timer_setHandler(void);
void timer_restoreHandler(void);
void timer_addTicks(int16 ticks);
void timer_setTickHandler(TickHandler handler);
int32 timer_getTicks(void);

}				// End of namespace Gob

#endif
