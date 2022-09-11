/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef WATCHMAKER_MOVIE_H
#define WATCHMAKER_MOVIE_H

#include "watchmaker/types.h"
#include "watchmaker/utils.h"

namespace Watchmaker {

struct gMovie {
	Common::SharedPtr<Common::SeekableReadStream> stream;
	uint16 numFrames;
	uint16 curFrame;
	uint16 width;
	uint16 height;
	uint16 numBlocks;
	uint32 startTime;
	uint8 keyFrame;
	uint8 frameRate;

	uint32 *frameOffsets;
	uint8 *buffer;
	// TODO

	~gMovie() {
		//warning("TODO: Clean up gMovie properly");
	}
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_MOVIE_H
