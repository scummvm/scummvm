/*
 * PaletteBundle.h
 *
 *  Created on: 9-Jun-2009
 *      Author: alex
 */

#ifndef PALETTEBUNDLE_H_
#define PALETTEBUNDLE_H_

#include "asylum/bundles/bundle.h"

namespace Asylum {

class PaletteBundle: public Bundle {
public:
	PaletteBundle() {}
	PaletteBundle(uint8 fileNum, uint32 index, uint32 length);

	~PaletteBundle() {}

	uint8* getPalette(uint8 brightness);

protected:
	uint8 palette[256*3];
	void update();
};

}

#endif /* PALETTEBUNDLE_H_ */
