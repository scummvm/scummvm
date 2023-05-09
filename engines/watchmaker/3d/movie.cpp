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

#include "watchmaker/3d/movie.h"
#include "watchmaker/3d/dds_header.h"
#include "watchmaker/file_utils.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/work_dirs.h"

namespace Watchmaker {

gMovie::gMovie(Common::SharedPtr<Common::SeekableReadStream> stream, Texture *texture, const Common::String &name) : _name(name), _stream(stream), _texture(texture) {
	_numFrames = stream->readUint16LE();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_keyFrame = stream->readByte();
	_frameRate = stream->readByte();

	_header = DDSHeader(*stream);
	_numBlocks = _width * _height / 16;
	_curFrame = 0xFFFF;

	_frameOffsets = new uint32[_numFrames] {};
	if (!_frameOffsets) {
		error("gLoadMovie FAILED: Can't alloc Movie->frameOffsets struct");
	}

	_buffer = new uint8[bufferSize()] {};
	_surfaceBuffer = new uint8[_header.dataSize()] {};
	_frameStream = new Common::MemoryReadStream(_surfaceBuffer, _header.dataSize(), DisposeAfterUse::NO);
	if (!_buffer) {
		error("gLoadMovie FAILED: Can't alloc Movie->buffer struct");
	}

	//read frame offsets
	for (int i = 0; i < _numFrames; i++) {
		_frameOffsets[i] = _stream->readUint32LE();
	}
}

gMovie::~gMovie() {
	delete[] _frameOffsets;
	delete[] _buffer;
	delete[] _surfaceBuffer;
	delete _frameStream;
}

Common::SharedPtr<gMovie> gLoadMovie(WorkDirs &workDirs, const char *TextName, Texture *texture) {
	//convert .avi name in .wmm
	Common::String finalName = replaceExtension(TextName, "wmm");

	auto stream = workDirs.resolveFile(finalName);
	if (!stream) {
		DebugLogFile("gLoadMovie FAILED: Can't find movie file\n");
		return nullptr;
	}
	auto Movie = Common::SharedPtr<gMovie>(new gMovie(stream, texture, TextName));
	Movie->_name = TextName;
	if (!Movie) {
		DebugLogFile("gLoadMovie FAILED: Can't alloc Movie struct");
		return nullptr;
	}

	//  Movie->frameRate=240;
	return Movie;
}

void gMovie::loadThisFrameData(uint16 frame) {
	_stream->seek(_frameOffsets[frame], SEEK_SET);
	//read frame data
	int32 size = 0;
	if ((frame + 1) == _numFrames) {
		size = _stream->size() - _frameOffsets[frame];
	} else {
		size = _frameOffsets[frame + 1] - _frameOffsets[frame];
	}
	assert(size <= (int32)bufferSize());
	_stream->read(_buffer, size);
}

//build a new frame by difference from previous
void gMovie::buildNewFrame(byte *surf, uint16 frame) {
	loadThisFrameData(frame);

	DWORD bitArraySize = _numBlocks >> 3;
	byte *buf = &_buffer[bitArraySize];
	WORD curBlock = 0;

	for (int i = 0; i < bitArraySize; i++) {
		byte block = _buffer[i];
		if (!block) {
			curBlock += 8;
			continue;   //everything is equal
		}

		for (int j = 0; j < 8; j++, curBlock++) {
			if (block & (1 << j)) {
				memcpy(&surf[curBlock << 3], buf, 8);
				buf += 8;
			}
		}
	}
}

bool gMovie::setFrame(uint16 newFrame) {
	warning("Set Frame: %d\t%s", newFrame, _name.c_str());
	if (_curFrame == newFrame)
		return true;

	//do we have to replace the whole frame or do we have to built it?
	bool rebuildFrame = true;
	if (_curFrame == 0xFFFF) rebuildFrame = false;
	else if (!(newFrame % _keyFrame)) rebuildFrame = false;        //it's a keyframe

	_header.dataSize();
#if 0
	DDSURFACEDESC2 ddsd2;
	ddsd2.dwSize = sizeof(DDSURFACEDESC2);

	if ((mv->surf->Lock(NULL, &ddsd2, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL))) {                  // Lock and fill with the dds
		DebugLogFile("gMovie_SetFrame: Can't lock surface DDS");
		return NULL;
	}
#endif
	if (!rebuildFrame) {
		loadThisFrameData(newFrame);
		memcpy(_surfaceBuffer, _buffer, _header.dataSize());
	} else {
		if ((_curFrame + 1) != newFrame) { //we can't directly build this frame because the current frame is not his previous
			uint16 startFrame;
			uint16 prevKey = (newFrame / _keyFrame) * _keyFrame;

			if ((_curFrame > newFrame) || (_curFrame < prevKey)) {
				loadThisFrameData(prevKey);
				memcpy(_surfaceBuffer, _buffer, _header.dataSize());
				startFrame = prevKey + 1;
			} else startFrame = _curFrame + 1;
			for (uint16 i = startFrame; i < newFrame; i++) {
				buildNewFrame(_surfaceBuffer, i);
			}
		}
		buildNewFrame(_surfaceBuffer, newFrame);
	}

	_frameStream->seek(0, SEEK_SET);
	auto tex = loadDdsTexture(*_frameStream, _header);
	_texture->assignData(*tex);
#if 0
	if (mat->Texture->lpDDSurface->Blt(NULL, mv->surf, NULL, DDBLT_WAIT, NULL) != DD_OK) {
		DebugLogFile("gMovie_SetFrame: Can't Blit DDS texture");
		return NULL;
	}
#endif
	_curFrame = newFrame;

	return true;
}

//*********************************************************************************************
bool gMovie::updateMovie() {
	int16 newFrame = 0;

	if (_paused)
		return TRUE;

	warning("Update Movie: %s", _name.c_str());

	if ((_curFrame == 0xFFFF) || (!_startTime)) {
		_startTime = timeGetTime();
		newFrame = 0;
	} else {
		// Use the time to find which frame we should be drawing
		uint32 curTime = timeGetTime();
		DWORD elapsedTime = curTime - _startTime;
		newFrame = (WORD)((float)elapsedTime / (1000.f / (float)_frameRate));

		if (newFrame >= _numFrames) {
			_startTime = curTime;
			newFrame = 0;
		}
	}

	return setFrame(newFrame);
}

int gMovie::frameSize(int index) {
	if ((index + 1) < _numFrames) {
		return _frameOffsets[index + 1] - _frameOffsets[index];
	} else {
		return _stream->size() - _frameOffsets[index];
	}
}

uint32 gMovie::bufferSize() const {
	return (_numBlocks / 8) + 8 * _numBlocks; //bit array + max different blocks
}


} // End of namespace Watchmaker
