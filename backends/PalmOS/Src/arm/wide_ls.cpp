#include "ArmNative.h"
#include "endianutils.h"
#include "../shared.h"

void O_WideLandscape(void *userData68KP) {
	OSysWidePtr dataP = (OSysWideType *)userData68KP;

	Coord x, y;
	UInt8 *dst = (UInt8 *)ReadUnaligned32(&(dataP->dst));
	UInt8 *src = (UInt8 *)ReadUnaligned32(&(dataP->src));

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
}
