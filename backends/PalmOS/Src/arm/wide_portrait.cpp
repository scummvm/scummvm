#include "PACEInterfaceLib.h"
#include "ArmNative.h"
#include "endianutils.h"
#include "../shared.h"

unsigned long OSystem_PALMOS_update_screen__wide_portrait(void *userData68KP) {
	DataOSysWidePtr dataP = (DataOSysWideType *)userData68KP;

	Coord x, y;
	UInt8 *dst = (UInt8 *)ReadUnaligned32(&(dataP->dst));
	UInt8 *src1 = (UInt8 *)ReadUnaligned32(&(dataP->src));
	UInt8 *src2 = src1;

	for (x = 0; x < WIDE_HALF_WIDTH; x++) 
	{
		for (y = 0; y < WIDE_HALF_HEIGHT; y++) 
		{
			*dst++ = *src1;
			src1 += WIDE_PITCH;
			*dst++ = *src1;
			*dst++ = *src1;
			src1 += WIDE_PITCH;
		}
		src1 = --src2;
		dst += 20; // we draw 200pix scaled to 1.5 = 300, screen width=320, so next is 20

		for (y = 0; y < WIDE_HALF_HEIGHT; y++) 
		{
			*dst++ = *src1;
			src1 += WIDE_PITCH;
			*dst++ = *src1;
			*dst++ = *src1;
			src1 += WIDE_PITCH;
		}
		src1 = --src2;
		dst += 20;

		MemMove(dst, dst - WIDE_PITCH, 300);	// 300 = 200 x 1.5
		dst += WIDE_PITCH;
	}	

	return 0;
}