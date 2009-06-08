/*
 * graphicresource.h
 *
 *  Created on: 8-Jun-2009
 *      Author: alex
 */

#ifndef ASYLUM_GRAPHICRESOURCE_H_
#define ASYLUM_GRAPHICRESOURCE_H_

#include "resource.h"

namespace Asylum {

class GraphicResource: public Resource {
public:
	GraphicResource() {}
	~GraphicResource() {}

	uint32 size;
	uint32 flag;
	uint16 x;
	uint16 y;
	uint16 width;
	uint16 height;

};

} // end of namespace Asylum

#endif /* GRAPHICRESOURCE_H_ */
