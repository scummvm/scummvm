#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	CompressType
#include "macros.h"

UInt32 Screen_decompressRLE0(void *userData68KP) {
// import variables
	SETPTR	(uint8 *	,src	);
	SET32	(uint32		,compSize);
	SETPTR	(uint8 *	,dest	);
// end of import

	uint8 *srcBufEnd = src + compSize;
	while (src < srcBufEnd) {
		uint8 color = *src++;
		if (color) {
			*dest++ = color;
		} else {
			uint8 skip = *src++;
			MemSet(dest, skip, 0);
			dest += skip;
		}
	}

	return 0;
}
