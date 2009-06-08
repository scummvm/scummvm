/*
 * resource.h
 *
 *  Created on: 7-Jun-2009
 *      Author: alex
 */

#ifndef ASYLUM_RESOURCE_H_
#define ASYLUM_RESOURCE_H_

#include "common/str.h"

namespace Asylum {

class Resource {
public:
	Resource();
	virtual ~Resource(){}

	int save(Common::String filename);

	Common::String type;
	uint32         size;
	uint32         offset;
	uint8*         data;

}; // end of class Resource

} // end of namespace Asylum

#endif
