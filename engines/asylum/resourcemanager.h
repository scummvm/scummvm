/*
 * resourcemanager.h
 *
 *  Created on: 7-Jun-2009
 *      Author: alex
 */

#ifndef ASYLUM_RESOURCEMANAGER_H_
#define ASYLUM_RESOURCEMANAGER_H_

#include "common/str.h"
#include "common/array.h"

#include "asylum/bundles/bundle.h"
#include "asylum/bundles/graphicbundle.h"
#include "asylum/resources/palette.h"

namespace Asylum {

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	int addBundle(Common::String filename);
	GraphicBundle getGraphic(Common::String file, uint32 offset);
	PaletteResource getPalette(Common::String file, uint32 offset);

private:
	uint32 getNextValidOffset(Bundle *bun, uint8 pos);

}; // end of class ResourceManager


} // end of namespace Asylum

#endif
