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

#ifndef BURIED_FBCDATA_H
#define BURIED_FBCDATA_H

#include "common/scummsys.h"

namespace Buried {

struct FilesPageHotspot {
	int16 left;
	int16 top;
	int16 right;
	int16 bottom;
	int16 pageIndex;
};

struct FilesPage {
	int16 pageID;
	int16 returnPageIndex;
	int16 nextButtonPageIndex;
	int16 prevButtonPageIndex;
	FilesPageHotspot hotspots[6];
};

} // End of namespace Buried

#endif
