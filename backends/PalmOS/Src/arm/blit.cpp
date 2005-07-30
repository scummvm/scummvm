#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	BlitType
#include "macros.h"

#define memcpy MemMove

UInt32 Display_blit(void *userData68KP) {
// import variables
	SETPTR	(uint8 *,		dstBuf		)
	SETPTR	(const uint8 *,	srcBuf		)
	SET16	(uint16,		dstPitch	)
	SET16	(uint16,		srcPitch	)
	SET16	(uint16,	w		)
	SET16	(uint16,	h		)
	SET8	(bool,		xflip	)
	SET8	(bool,		masked	)
// end of import

	if (!masked) { // Unmasked always unflipped
		while (h--) {
			memcpy(dstBuf, srcBuf, w);
			srcBuf += srcPitch;
			dstBuf += dstPitch;
		}
	} else if (!xflip) { // Masked bitmap unflipped
		while (h--) {
			for(int i = 0; i < w; ++i) {
				uint8 b = *(srcBuf + i);
				if(b != 0) {
					*(dstBuf + i) = b;
				}
			}
			srcBuf += srcPitch;
			dstBuf += dstPitch;
		}
	} else { // Masked bitmap flipped
		while (h--) {
			for(int i = 0; i < w; ++i) {
				uint8 b = *(srcBuf + i);
				if(b != 0) {
					*(dstBuf - i) = b;
				}
			}
			srcBuf += srcPitch;
			dstBuf += dstPitch;
		}
	}

	return 0;
}
