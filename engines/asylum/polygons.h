/*
 * Polygons.h
 *
 *  Created on: Sep 18, 2009
 *      Author: alex
 */

#ifndef ASYLUM_POLYGONS_H_
#define ASYLUM_POLYGONS_H_

#include "common/array.h"
#include "common/rect.h"

#define MAX_POLYGONS 200

namespace Asylum {

typedef struct PolyDefinitions {
	uint32		  numPoints;
	Common::Point *points;
	Common::Rect  boundingRect;

} PolyDefinitions;

class Polygons {
public:
	Polygons();
	virtual ~Polygons();

	uint32 size;
	uint32 numEntries;

	Common::Array<PolyDefinitions> entries;

}; // end of class Polygons

} // end of namespace Asylum

#endif
