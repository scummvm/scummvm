#include "PACEInterfaceLib.h"
#include "ArmNative.h"
#include "endianutils.h"
#include "../shared.h"

unsigned long OSystem_PALMOS_update_screen__wide_landscape(void *userData68KP) {
	DataOSysWidePtr dataP = (DataOSysWideType *)userData68KP;

	Coord x, y;
	UInt8 *dst = (UInt8 *)ReadUnaligned32(&(dataP->dst));
	UInt8 *src = (UInt8 *)ReadUnaligned32(&(dataP->src));

	for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
		for (x = 0; x < WIDE_HALF_WIDTH; x++) {
			*dst++ = *src++;
			*dst++ = *src;
			*dst++ = *src++;
		}
		for (x = 0; x < WIDE_HALF_WIDTH; x++) {
			*dst++ = *src++;
			*dst++ = *src;
			*dst++ = *src++;
		}

		MemMove(dst, dst - 480, 480);
		dst += 480;
	}

	return 0;
}
