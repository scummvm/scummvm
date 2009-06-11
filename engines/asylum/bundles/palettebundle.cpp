/*
 * PaletteBundle.cpp
 *
 *  Created on: 9-Jun-2009
 *      Author: alex
 */

#include "asylum/bundles/palettebundle.h"

namespace Asylum {

PaletteBundle::PaletteBundle(uint8 fileNum, uint32 index, uint32 length) {
	loadRawRecord(parseFilename(fileNum), index, length);
}

void PaletteBundle::update() {

}

uint8* PaletteBundle::getPalette(uint8 brightness) {
	// palettes always start from offset 32
	memcpy(palette, data+32, sizeof(uint8)*256*3);

	for (int i = 0; i < 256; i++) {
		*p++ = *palette++ * brightness;
		*p++ = *palette++ * brightness;
		*p++ = *palette++ * brightness;
		*p++ = 0;
	}

	return palette;
}

} // end of namespace Asylum
