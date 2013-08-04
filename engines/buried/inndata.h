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

#ifndef BURIED_INNDATA_H
#define BURIED_INNDATA_H

#include "common/scummsys.h"

namespace Buried {

struct INNHotspotData {
	uint32 left;
	uint32 top;
	uint32 right;
	uint32 bottom;
	uint32 stillFrameOffset;
};

struct INNFrame {
	uint16 topicID;
	uint16 pageType;
	uint32 stillFrameOffset;
	INNHotspotData hotspots[8];
};

enum {
	MEDIA_TYPE_VIDEO_FULL = 1,
	MEDIA_TYPE_VIDEO_SMALL_A = 2,
	MEDIA_TYPE_VIDEO_SMALL_B = 3,
	MEDIA_TYPE_AUDIO = 4
};

struct INNMediaElement {
	uint32 frameIndex;
	uint16 mediaType;
	uint16 fileIDOffset;
};

} // End of namespace Buried

#endif
