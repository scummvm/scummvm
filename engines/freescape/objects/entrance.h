//
//  Entrance.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef FREESCAPE_ENTRANCE_H
#define FREESCAPE_ENTRANCE_H

#include "freescape/objects/object.h"

class Entrance : public Object {
public:

	Entrance(
		uint16 objectID,
		const Math::Vector3d &origin,
		const Math::Vector3d &rotation);
	virtual ~Entrance();

	bool isDrawable();
	bool isPlanar();
	Type getType() override { return Type::Entrance; };
	Math::Vector3d getRotation() { return _rotation; }
	
	void draw(Freescape::Renderer *gfx) override { error("cannot render Entrance"); };
};

#endif
