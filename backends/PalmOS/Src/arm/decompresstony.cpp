#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	CompressType
#include "macros.h"

UInt32 Screen_decompressTony(void *userData68KP) {
// import variables
	SETPTR	(UInt8 *	,src	);
	SET32	(UInt32,	compSize);
	SETPTR	(UInt8 *	,dest	);
// end of import

	uint8 *endOfData = src + compSize;
	while (src < endOfData) {
		uint8 numFlat = *src++;
		if (numFlat) {
			MemSet(dest, numFlat, *src);
			src++;
			dest += numFlat;
		}
		if (src < endOfData) {
			uint8 numNoFlat = *src++;
			MemMove(dest, src, numNoFlat);
			src += numNoFlat;
			dest += numNoFlat;
		}
	}

	return 0;
}
