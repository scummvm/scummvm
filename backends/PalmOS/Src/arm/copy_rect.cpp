#include "ArmNative.h"
#include "endianutils.h"
#include "../shared.h"

void O_CopyRectToScreen(void *userData68KP) {
	OSysCopyPtr dataP = (OSysCopyType *)userData68KP;

	UInt8 *dst = (UInt8 *)ReadUnaligned32(&(dataP->dst));
	UInt8 *buf = (UInt8 *)ReadUnaligned32(&(dataP->buf));
	UInt32 pitch = ReadUnaligned32(&(dataP->pitch));
	UInt32 _offScreenPitch = ReadUnaligned32(&(dataP->_offScreenPitch));
	UInt32 w = ReadUnaligned32(&(dataP->w));
	UInt32 h = ReadUnaligned32(&(dataP->h));

	if (_offScreenPitch == pitch && pitch == w) {
		MemMove(dst, buf, h * w);
	} else {
		do {
			MemMove(dst, buf, w);
			dst += _offScreenPitch;
			buf += pitch;
		} while (--h);
	}
}
