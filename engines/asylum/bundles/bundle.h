/*
 * bundle.h
 *
 *  Created on: 7-Jun-2009
 *      Author: alex
 */

#ifndef ASYLUM_BUNDLE_H_
#define ASYLUM_BUNDLE_H_

#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"

#include "common/str.h"
#include "common/array.h"

#include "asylum/resources/resource.h"

namespace Asylum {

class Bundle {

public:
	Bundle();
	Bundle(Common::String filename, uint32 index);
	virtual ~Bundle() {}

	Common::String id;
	uint32 size;
	uint32 numEntries;
	Common::Array<Resource> entries;

	uint8* getData() { return data; }
protected:
	int loadRaw(Common::String filename, uint32 index);
	virtual void update(){}
	uint8 *data;

private:

}; // end of class Bundle


} // end of namespace Asylum

#endif
