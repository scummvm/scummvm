#ifndef OSTATE_H
#define OSTATE_H

#include "vector3d.h"
#include "resource.h"
#include <string>
#include <list>


class ObjectState {
	public:
	enum Position {
		OBJSTATE_UNDERLAY = 1,
		OBJSTATE_OVERLAY = 2,
		OBJSTATE_STATE = 3
	};
};


#endif
