#include "native.h"
#include "endianutils.h"
#include "../shared.h"

#define MAIN_TYPE	WideType
#include "macros.h"

UInt32 OSystem_updateScreen_widePortrait(void *userData68KP) {
// import variables
	SETPTR(UInt8 *	,dst)
	SETPTR(UInt8 *	,src)
// end of import

	Coord x, y;
	UInt8 *src2	= src;

	for (x = 0; x < WIDE_HALF_WIDTH; x++) {
		for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
			*dst++ = *src;
			src += WIDE_PITCH;
			*dst++ = *src;
			*dst++ = *src;
			src += WIDE_PITCH;
		}
		src = --src2;
		dst += 20; // we draw 200pix scaled to 1.5 = 300, screen width=320, so next is 20

		for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
			*dst++ = *src;
			src += WIDE_PITCH;
			*dst++ = *src;
			*dst++ = *src;
			src += WIDE_PITCH;
		}
		src = --src2;
		dst += 20;

		MemMove(dst, dst - WIDE_PITCH, 300);	// 300 = 200 x 1.5
		dst += WIDE_PITCH;
	}

	return 0;
}
