#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	FastShrinkType
#include "macros.h"

UInt32 Screen_fastShrink(void *userData68KP) {
// import variables
	SETPTR	(UInt8 *	,src		);
	SET32	(UInt32,	width		);
	SET32	(UInt32,	height		);
	SET32	(uint32,	scale		);
	SETPTR	(UInt8 *	,dest		);
// end of import

	uint32 resHeight = (height * scale) >> 8;
	uint32 resWidth = (width * scale) >> 8;
	uint32 step = 0x10000 / scale;
	uint8 columnTab[160];
	uint32 res = step >> 1;
	for (uint16 cnt = 0; cnt < resWidth; cnt++) {
		columnTab[cnt] = (uint8)(res >> 8);
		res += step;
	}

	uint32 newRow = step >> 1;
	uint32 oldRow = 0;

	uint8 *destPos = dest;
	uint16 lnCnt;
	for (lnCnt = 0; lnCnt < resHeight; lnCnt++) {
		while (oldRow < (newRow >> 8)) {
			oldRow++;
			src += width;
		}
		for (uint16 colCnt = 0; colCnt < resWidth; colCnt++) {
			*destPos++ = src[columnTab[colCnt]];
		}
		newRow += step;
	}
	// scaled, now stipple shadows if there are any
	for (lnCnt = 0; lnCnt < resHeight; lnCnt++) {
		uint16 xCnt = lnCnt & 1;
		destPos = dest + lnCnt * resWidth + (lnCnt & 1);
		while (xCnt < resWidth) {
			if (*destPos == 200)
				*destPos = 0;
			destPos += 2;
			xCnt += 2;
		}
	}

	return 0;
}
