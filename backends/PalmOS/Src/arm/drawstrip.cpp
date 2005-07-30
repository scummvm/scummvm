#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	DrawStripType
#include "macros.h"

#define CHARSET_MASK_TRANSPARENCY	253

UInt32 Gdi_drawStripToScreen(void *userData68KP) {
// import variables
	SET32	(int			,width				)
	SET32	(int			,height				)
	SETPTR	(const byte *	,src				)
	SETPTR	(byte *			,dst				)
	SETPTR	(const byte *	,text				)
	SET32	(int			,_vm_screenWidth	)
	SET16	(uint16			,vs_pitch			)
	SET16	(uint16			,_textSurface_pitch	)
// end of import

	// Compose the text over the game graphics
	for (int h = 0; h < height; ++h) {
		for (int w = 0; w < width; ++w) {
			if (text[w] == CHARSET_MASK_TRANSPARENCY)
				dst[w] = src[w];
			else
				dst[w] = text[w];
		}
		src += vs_pitch;
		dst += _vm_screenWidth;
		text += _textSurface_pitch;
	}

	return 0;
}
