/*
 * paletteresource.h
 *
 *  Created on: 8-Jun-2009
 *      Author: alex
 */

#ifndef ASYLUM_PALETTERESOURCE_H_
#define ASYLUM_PALETTERESOURCE_H_

#include "asylum/bundles/bundle.h"

namespace Asylum {

class PaletteResource: public Resource {
public:
	PaletteResource() {}
	PaletteResource(uint8* raw);
	~PaletteResource() {}

	// Override base data variable since
	// the final size is known
	uint8 data[256*3];

}; // end of class PaletteResource

} // end of namespace Asylum

#endif /* PALETTERESOURCE_H_ */
