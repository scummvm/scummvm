#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	DrawSpriteType
#include "macros.h"

UInt32 Screen_drawSprite(void *userData68KP) {
// import variables
	SETPTR	(UInt8 *	,sprData	);
	SET16	(UInt16,	sprHeight	);
	SET16	(UInt16,	sprWidth	);
	SET16	(UInt16,	sprPitch	);
	SETPTR	(UInt8 *	,dest		);
	SET16	(UInt16,	_scrnSizeX	);
// end of import

	for (uint16 cnty = 0; cnty < sprHeight; cnty++) {
		for (uint16 cntx = 0; cntx < sprWidth; cntx++)
			if (sprData[cntx])
				dest[cntx] = sprData[cntx];
		sprData += sprPitch;
		dest += _scrnSizeX;
	}

	return 0;
}
