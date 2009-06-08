/*
 * graphicbundle.h
 *
 *  Created on: 7-Jun-2009
 *      Author: alex
 */

#ifndef ASYLUM_GRAPHICBUNDLE_H_
#define ASYLUM_GRAPHICBUNDLE_H_

#include "asylum/bundles/bundle.h"
#include "asylum/resources/graphic.h"

namespace Asylum {

class GraphicBundle: public Bundle {
public:
	GraphicBundle() {}
	GraphicBundle(Common::String filename, uint32 index);
	~GraphicBundle() {}

	Common::Array<GraphicResource> entries;
	GraphicResource getEntry(uint32 index);

protected:
	void update();

private:
	uint32 _tagValue;
	uint32 _flag;
	uint32 _contentOffset;
	uint32 _unknown1;
	uint32 _unknown2;
	uint32 _unknown3;
	uint16 _maxWidth;
};

} // end of namespace Asylum

#endif
