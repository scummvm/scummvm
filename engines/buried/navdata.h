/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BURIED_NAVDATA_H
#define BURIED_NAVDATA_H

#include "common/scummsys.h"

namespace Buried {

struct Location {
	uint16 timeZone;
	uint16 environment;
	uint16 node;
	uint16 facing;
	uint16 orientaton;
	uint16 depth;
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
	uint16 transitionType;

	// Data specific to each type of transition:
	// TRANSITION_VIDEO: video clip ID
	// TRANSITION_PUSH: identifies direction
	uint16 transitionData;

	uint32 transitionStartFrame; // Unused for video
	uint32 transitionLength;     // Unused for video
};

struct LocationStaticData {
	Location location;
	DestinationScene destUp;
	DestinationScene destLeft;
	DestinationScene destRight;
	DestinationScene destDown;
	DestinationScene destForward;
	uint16 classID;
	uint32 navFrameIndex;
	uint32 miscFrameIndex;
	uint32 miscFrameCount;
	uint32 cycleStartFrame;
	uint32 cycleFrameCount;
};

} // End of namespace Buried

#endif
