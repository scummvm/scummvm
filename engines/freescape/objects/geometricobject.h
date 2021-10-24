//
//  GeometricObject.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__GeometricObject__
#define __Phantasma__GeometricObject__

#include "common/array.h"
#include "freescape/language/instruction.h"
#include "freescape/objects/object.h"

class GeometricObject : public Object {
public:
	static int numberOfColoursForObjectOfType(Type type);
	static int numberOfOrdinatesForType(Type type);

	GeometricObject(
		Type type,
		uint16 objectID,
		uint16 flags,
		const Math::Vector3d &origin,
		const Math::Vector3d &size,
		Common::Array<uint8> *colours,
		Common::Array<uint16> *ordinates,
		FCLInstructionVector condition);
	virtual ~GeometricObject();
	void draw(Freescape::Renderer *gfx) override;
	bool isDrawable();
	bool isPlanar();

private:

	FCLInstructionVector condition;
	Common::Array<uint8> *colours;
	Common::Array<uint16> *ordinates;

	size_t drawElementsStartIndex;
};

#endif /* defined(__Phantasma__GeometricObject__) */
