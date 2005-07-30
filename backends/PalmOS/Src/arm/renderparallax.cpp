#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	ParallaxType
#include "macros.h"

#define READ_LE_UINT32(ptr)	*(const uint32 *)(ptr)

UInt32 Screen_renderParallax(void *userData68KP) {
// import variables
	SETPTR	(UInt8 *	,data		);
	SETPTR	(UInt32 *	,lineIndexes);
	SETPTR	(UInt8 *	,_screenBuf	);
	SET16	(UInt16,	_scrnSizeX	);
	SET16	(UInt16,	scrnScrlX	);
	SET16	(UInt16,	scrnScrlY	);
	SET16	(UInt16,	paraScrlX	);
	SET16	(UInt16,	paraScrlY	);
	SET16	(UInt16,	scrnWidth	);
	SET16	(UInt16,	scrnHeight	);
// end of import

	for (uint16 cnty = 0; cnty < scrnHeight; cnty++) {
		uint8 *src = data + READ_LE_UINT32(lineIndexes + cnty + paraScrlY);
		uint8 *dest = _screenBuf + scrnScrlX + (cnty + scrnScrlY) * _scrnSizeX;
		uint16 remain = paraScrlX;
		uint16 xPos = 0;
		bool copyFirst = false;
		while (remain) { // skip past the first part of the parallax to get to the right scrolling position
			uint8 doSkip = *src++;
			if (doSkip <= remain)
				remain -= doSkip;
			else {
				xPos = doSkip - remain;
				dest += xPos;
				remain = 0;
			}
			if (remain) {
				uint8 doCopy = *src++;
				if (doCopy <= remain) {
					remain -= doCopy;
					src += doCopy;
				} else {
					uint16 remCopy = doCopy - remain;
					MemMove(dest, src + remain, remCopy);
					dest += remCopy;
					src += doCopy;
					xPos = remCopy;
					remain = 0;
				}
			} else
				copyFirst = true;
		}
		while (xPos < scrnWidth) {
			if (!copyFirst) {
				if (uint8 skip = *src++) {
					dest += skip;
					xPos += skip;
				}
			} else
				copyFirst = false;
			if (xPos < scrnWidth) {
				if (uint8 doCopy = *src++) {
					if (xPos + doCopy > scrnWidth)
						doCopy = scrnWidth - xPos;
					MemMove(dest, src, doCopy);
					dest += doCopy;
					xPos += doCopy;
					src += doCopy;
				}
			}
		}
	}

	return 0;
}
