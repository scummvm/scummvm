//
//  Entrance.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Entrance__
#define __Phantasma__Entrance__

#include "freescape/objects/object.h"

class Entrance : public Object {
public:

	Entrance(
		uint16 objectID,
		const Vector3d &origin,
		const Vector3d &rotation);
	virtual ~Entrance();

	bool isDrawable();
	bool isPlanar();
};


#endif /* defined(__Phantasma__Entrance__) */
