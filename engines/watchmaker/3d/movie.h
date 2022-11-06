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

#include "common/memstream.h"
#include "watchmaker/3d/dds_header.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"

namespace Watchmaker {

struct gMovie {
private:
	Common::MemoryReadStream *_frameStream = nullptr;
public:
	Common::String _name;
	Common::SharedPtr<Common::SeekableReadStream> _stream;
	uint16 _numFrames;
	uint16 _curFrame;
	uint16 _width;
	uint16 _height;
	uint16 _numBlocks;
	uint32 _startTime;
	uint8 _keyFrame;
	uint8 _frameRate;

	uint32 *_frameOffsets;
	uint8 *_buffer;
	uint8 *_surfaceBuffer;

	DDSHeader _header;

	Texture *_texture = nullptr;

	bool _paused = false;

	gMovie(Common::SharedPtr<Common::SeekableReadStream> stream, Texture *texture, const Common::String &name);

	bool setFrame(uint32 newFrame);
	void loadThisFrameData(uint32 frame);
	void buildNewFrame(byte *surf, uint32 frame);
	bool updateMovie();

	~gMovie() {
		//warning("TODO: Clean up gMovie properly");
	}
private:
	int frameSize(int index);
	uint32 bufferSize() const;
};

class WorkDirs;
Common::SharedPtr<gMovie> gLoadMovie(WorkDirs &workDirs, const char *TextName, Texture *texture);

} // End of namespace Watchmaker

#endif // WATCHMAKER_MOVIE_H
