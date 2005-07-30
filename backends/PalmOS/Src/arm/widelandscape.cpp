#include "native.h"
#include "endianutils.h"
#include "../shared.h"

#define MAIN_TYPE	WideType
#include "macros.h"

UInt32 OSystem_updateScreen_wideLandscape(void *userData68KP) {
// import variables
	SETPTR(UInt8 *	,dst)
	SETPTR(UInt8 *	,src)
// end of import

	Coord x, y;

	for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
		// draw 2 lines
		for (x = 0; x < WIDE_FULL_WIDTH; x++) {
			*dst++ = *src++;
			*dst++ = *src;
			*dst++ = *src++;
		}
		// copy the second to the next line
		MemMove(dst, dst - 480, 480);
		dst += 480;
	}

	return 0;
}

