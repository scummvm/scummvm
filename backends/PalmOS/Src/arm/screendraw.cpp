#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	DrawType
#include "macros.h"

UInt32 Screen_draw(void *userData68KP) {
// import variables
	SET16	(UInt16,	_scrnSizeX	);
	SET16	(UInt16,	_scrnSizeY	);
	SETPTR	(UInt8 *	,src		);
	SETPTR	(UInt8 *	,dest		);
// end of import

	for (uint16 cnty = 0; cnty < _scrnSizeY; cnty++)
		for (uint16 cntx = 0; cntx < _scrnSizeX; cntx++) {
			if (*src)
				*dest = *src;
			dest++;
			src++;
		}

	return 0;
}
