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

#ifndef BURIED_INNDATA_H
#define BURIED_INNDATA_H

#include "common/scummsys.h"

namespace Buried {

struct INNHotspotData {
	int16 left;
	int16 top;
	int16 right;
	int16 bottom;
	int32 stillFrameOffset;
};

struct INNFrame {
	int16 topicID;
	int16 pageType;
	int32 stillFrameOffset;
	INNHotspotData hotspots[8];
};

enum {
	MEDIA_TYPE_VIDEO_FULL = 1,
	MEDIA_TYPE_VIDEO_SMALL_A = 2,
	MEDIA_TYPE_VIDEO_SMALL_B = 3,
	MEDIA_TYPE_AUDIO = 4
};

struct INNMediaElement {
	int32 frameIndex;
	int16 mediaType;
	int16 fileIDOffset;
};

} // End of namespace Buried

#endif
