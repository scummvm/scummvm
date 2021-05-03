//
//  Entrance.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "freescape/objects/entrance.h"

#pragma mark -
#pragma mark Construction/Destruction

Entrance::Entrance(
	uint16 _objectID,
	const Vector3d &_origin,
	const Vector3d &_rotation) {
	objectID = _objectID;
	origin = _origin;
	rotation = _rotation;
}

Entrance::~Entrance() {}

bool Entrance::isDrawable() { return false; }
bool Entrance::isPlanar() { return true; }