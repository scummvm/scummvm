/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __PALANIM_H
#define __PALANIM_H

namespace Gob {

extern int16 pal_fadeValue;

char pal_fadeColor(char from, char to);
char pal_fadeStep(int16 oper);	// oper == 0 - fade all colors, 1, 2, 3 - red,green, blue
void pal_fade(PalDesc * palDesc, int16 fade, int16 all);

}				// End of namespace Gob

#endif	/* __PALANIM_H */
