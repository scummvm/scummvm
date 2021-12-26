/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_NAVDATA_H
#define BURIED_NAVDATA_H

#include "common/scummsys.h"

namespace Buried {

struct Location {
	Location() : timeZone(-1), environment(-1), node(-1), facing(-1), orientation(-1), depth(-1) {}
	Location(int16 tz, int16 e, int16 n, int16 f, int16 o, int16 d) : timeZone(tz), environment(e), node(n), facing(f), orientation(o), depth(d) {}

	int16 timeZone;
	int16 environment;
	int16 node;
	int16 facing;
	int16 orientation;
	int16 depth;
};

enum {
	TRANSITION_NONE = 0,
	TRANSITION_PUSH = 1,
	TRANSITION_WALK = 2,
	TRANSITION_VIDEO = 3,
	TRANSITION_FADE = 4
};

enum {
	TF_PUSH_UP = 0,
	TF_PUSH_LEFT = 1,
	TF_PUSH_RIGHT = 2,
	TF_PUSH_DOWN = 3
};

struct DestinationScene {
	Location destinationScene;
	int16 transitionType;

	// Data specific to each type of transition:
	// TRANSITION_VIDEO: video clip ID
	// TRANSITION_PUSH: identifies direction
	int16 transitionData;

	int32 transitionStartFrame; // Unused for video
	int32 transitionLength;     // Unused for video
};

struct LocationStaticData {
	Location location;
	DestinationScene destUp;
	DestinationScene destLeft;
	DestinationScene destRight;
	DestinationScene destDown;
	DestinationScene destForward;
	int16 classID;
	int32 navFrameIndex;
	int32 miscFrameIndex;
	int32 miscFrameCount;
	int32 cycleStartFrame;
	int32 cycleFrameCount;
};

} // End of namespace Buried

#endif
