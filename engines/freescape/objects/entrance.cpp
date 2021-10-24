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
	uint16 objectID,
	const Math::Vector3d &origin,
	const Math::Vector3d &rotation) {
	_objectID = objectID;
	_origin = origin;
	_rotation = rotation;
}

Entrance::~Entrance() {}

bool Entrance::isDrawable() { return false; }
bool Entrance::isPlanar() { return true; }