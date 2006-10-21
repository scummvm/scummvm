#ifndef _I_ZODIAC_H_
#define _I_ZODIAC_H_

#define	_twGfxLib	ptrP[0]
#define _twSrc		ptrP[1]
#define _twDst		ptrP[2]
#define _twBmpV3	ptrP[3]

Err ZodiacInit(void **ptrP, Int32 w, Int32 h);
Err ZodiacRelease(void **ptrP);

#endif
