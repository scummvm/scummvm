#include "PACEInterfaceLib.h"
#include "ArmNative.h"
#include "endianutils.h"
#include "../shared.h"

unsigned long OSystem_PALMOS_copy_rect(void *userData68KP) {
	UInt8* dataP = (UInt8 *)userData68KP;

	UInt8 *dst = (UInt8 *)ReadUnaligned32(dataP + 2);		// ->dst
	UInt8 *buf = (UInt8 *)ReadUnaligned32(dataP + 6);		// ->buf
	UInt32 pitch = ReadUnaligned32(dataP + 10);				// ->pitch
	UInt32 _offScreenPitch = ReadUnaligned32(dataP + 14);	// ->_offScreenPitch
	UInt32 w = ReadUnaligned32(dataP + 18);					// ->w
	UInt32 h = ReadUnaligned32(dataP + 22);					// ->h

	if (_offScreenPitch == pitch && pitch == w) {
		MemMove(dst, buf, h * w);
	} else {
		do {
			MemMove(dst, buf, w);
			dst += _offScreenPitch;
			buf += pitch;
		} while (--h);
	}

	return 0;
}