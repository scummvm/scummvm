/*
 * paletteresource.cpp
 *
 *  Created on: 8-Jun-2009
 *      Author: alex
 */

#include "asylum/resources/palette.h"

namespace Asylum {

PaletteResource::PaletteResource(uint8* raw) {
	memcpy(data, raw+32, sizeof(uint8)*256*3); // palettes always start from offset 32
}

} // end of namespace Asylum
