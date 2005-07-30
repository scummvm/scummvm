#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	CopyRectangleType
#include "macros.h"

UInt32 OSystem_CopyRectToScreen(void *userData68KP) {
// import variables
	SETPTR	(UInt8 *,	dst				)
	SETPTR	(UInt8 *,	buf				)
	SET32	(UInt32,	pitch			)
	SET32	(UInt32,	_offScreenPitch	)
	SET32	(UInt32,	w	)
	SET32	(UInt32,	h	)
// end of import

	if (w == pitch && w == _offScreenPitch) {
		MemMove(dst, buf, w*h);
	} else {
		do {
			MemMove(dst, buf, w);
			dst += _offScreenPitch;
			buf += pitch;
		} while (--h);
	}

	return 0;
}
