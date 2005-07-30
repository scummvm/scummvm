#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	CompressType
#include "macros.h"

UInt32 Screen_decompressRLE7(void *userData68KP) {
// import variables
	SETPTR	(UInt8 *	,src	);
	SET32	(UInt32,	compSize);
	SETPTR	(UInt8 *	,dest	);
// end of import

	uint8 *compBufEnd = src + compSize;
	while (src < compBufEnd) {
		uint8 code = *src++;
		if ((code > 127) || (code == 0))
			*dest++ = code;
		else {
			code++;
			MemSet(dest, code, *src++);
			dest += code;
		}
	}

	return 0;
}
