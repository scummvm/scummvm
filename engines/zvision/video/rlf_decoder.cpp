/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
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

#include "common/scummsys.h"

#include "zvision/video/rlf_decoder.h"

#include "common/str.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/endian.h"

#include "graphics/colormasks.h"

namespace ZVision {

RLFDecoder::RLFDecoder(const Common::String &fileName, bool stream)
	: _stream(stream),
	  _readStream(NULL),
	  _lastFrameRead(0),
	  _frameCount(0),
	  _width(0),
	  _height(0),
	  _frameTime(0),
	  _frames(0),
	  _nextFrame(0),
	  _frameBufferByteSize(0) {

	Common::File *_file = new Common::File;
	if (!_file->open(fileName)) {
		warning("RLF animation file %s could not be opened", fileName.c_str());
		return;
	}

	_readStream = _file;

	if (!readHeader()) {
		warning("%s is not a RLF animation file. Wrong magic number", fileName.c_str());
		return;
	}

	_currentFrameBuffer.create(_width, _height, Graphics::createPixelFormat<565>());
	_frameBufferByteSize = _width * _height * sizeof(uint16);

	if (!stream) {
		_frames = new Frame[_frameCount];

		// Read in each frame
		for (uint i = 0; i < _frameCount; ++i) {
			_frames[i] = readNextFrame();
		}
	}
}

RLFDecoder::RLFDecoder(Common::SeekableReadStream *rstream, bool stream)
	: _stream(stream),
	  _readStream(rstream),
	  _lastFrameRead(0),
	  _frameCount(0),
	  _width(0),
	  _height(0),
	  _frameTime(0),
	  _frames(0),
	  _nextFrame(0),
	  _frameBufferByteSize(0) {

	if (!readHeader()) {
		warning("Stream is not a RLF animation. Wrong magic number");
		return;
	}

	_currentFrameBuffer.create(_width, _height, Graphics::createPixelFormat<565>());
	_frameBufferByteSize = _width * _height * sizeof(uint16);

	if (!stream) {
		_frames = new Frame[_frameCount];

		// Read in each frame
		for (uint i = 0; i < _frameCount; ++i) {
			_frames[i] = readNextFrame();
		}
	}
}

RLFDecoder::~RLFDecoder() {
	for (uint i = 0; i < _frameCount; ++i) {
		delete[] _frames[i].encodedData;
	}
	delete[] _frames;
	delete _readStream;
	_currentFrameBuffer.free();
}

bool RLFDecoder::readHeader() {
	if (_readStream->readUint32BE() != MKTAG('F', 'E', 'L', 'R')) {
		return false;
	}

	// Read the header
	_readStream->readUint32LE();                // Size1
	_readStream->readUint32LE();                // Unknown1
	_readStream->readUint32LE();                // Unknown2
	_frameCount = _readStream->readUint32LE();  // Frame count

	// Since we don't need any of the data, we can just seek right to the
	// entries we need rather than read in all the individual entries.
	_readStream->seek(136, SEEK_CUR);

	//// Read CIN header
	//_readStream->readUint32BE();          // Magic number FNIC
	//_readStream->readUint32LE();          // Size2
	//_readStream->readUint32LE();          // Unknown3
	//_readStream->readUint32LE();          // Unknown4
	//_readStream->readUint32LE();          // Unknown5
	//_readStream->seek(0x18, SEEK_CUR);    // VRLE
	//_readStream->readUint32LE();          // LRVD
	//_readStream->readUint32LE();          // Unknown6
	//_readStream->seek(0x18, SEEK_CUR);    // HRLE
	//_readStream->readUint32LE();          // ELHD
	//_readStream->readUint32LE();          // Unknown7
	//_readStream->seek(0x18, SEEK_CUR);    // HKEY
	//_readStream->readUint32LE();          // ELRH

	//// Read MIN info header
	//_readStream->readUint32BE();          // Magic number FNIM
	//_readStream->readUint32LE();          // Size3
	//_readStream->readUint32LE();          // OEDV
	//_readStream->readUint32LE();          // Unknown8
	//_readStream->readUint32LE();          // Unknown9
	//_readStream->readUint32LE();          // Unknown10
	_width = _readStream->readUint32LE();   // Width
	_height = _readStream->readUint32LE();  // Height

	// Read time header
	_readStream->readUint32BE();                    // Magic number EMIT
	_readStream->readUint32LE();                    // Size4
	_readStream->readUint32LE();                    // Unknown11
	_frameTime = _readStream->readUint32LE() / 10;  // Frame time in microseconds

	return true;
}

RLFDecoder::Frame RLFDecoder::readNextFrame() {
	RLFDecoder::Frame frame;

	_readStream->readUint32BE();                        // Magic number MARF
	uint32 size = _readStream->readUint32LE();          // Size
	_readStream->readUint32LE();                        // Unknown1
	_readStream->readUint32LE();                        // Unknown2
	uint32 type = _readStream->readUint32BE();          // Either ELHD or ELRH
	uint32 headerSize = _readStream->readUint32LE();    // Offset from the beginning of this frame to the frame data. Should always be 28
	_readStream->readUint32LE();                        // Unknown3

	frame.encodedSize = size - headerSize;
	frame.encodedData = new int8[frame.encodedSize];
	_readStream->read(frame.encodedData, frame.encodedSize);

	if (type == MKTAG('E', 'L', 'H', 'D')) {
		frame.type = Masked;
	} else if (type == MKTAG('E', 'L', 'R', 'H')) {
		frame.type = Simple;
		_completeFrames.push_back(_lastFrameRead);
	} else {
		warning("Frame %u doesn't have type that can be decoded", _lastFrameRead);
	}

	_lastFrameRead++;
	return frame;
}

void RLFDecoder::seekToFrame(int frameNumber) {
	assert(!_stream);
	assert(frameNumber < (int)_frameCount || frameNumber >= -1);

	if (_nextFrame == frameNumber)
		return;

	if (frameNumber < 0) {
		_nextFrame = 0;
		return;
	}

	int closestFrame = _nextFrame;
	int distance = (int)frameNumber - _nextFrame;

	if (distance < 0) {
		for (uint i = 0; i < _completeFrames.size(); ++i) {
			if ((int)_completeFrames[i] > frameNumber)
				break;
			closestFrame = _completeFrames[i];
		}
	} else {
		for (uint i = 0; i < _completeFrames.size(); ++i) {
			int newDistance = (int)frameNumber - (int)(_completeFrames[i]);
			if (newDistance < 0)
				break;
			if (newDistance < distance) {
				closestFrame = _completeFrames[i];
				distance = newDistance;
			}
		}
	}

	for (int i = closestFrame; i < frameNumber; ++i) {
		applyFrameToCurrent(i);
	}

	_nextFrame = frameNumber;
}

const Graphics::Surface *RLFDecoder::getFrameData(uint frameNumber) {
	assert(!_stream);
	assert(frameNumber < _frameCount);

	// Since this method is so expensive, first check to see if we can use
	// decodeNextFrame() it's cheap.
	if ((int)frameNumber == _nextFrame - 1) {
		return &_currentFrameBuffer;
	} else if (_nextFrame == (int)frameNumber) {
		return decodeNextFrame();
	}

	seekToFrame(frameNumber);
	return decodeNextFrame();
}

const Graphics::Surface *RLFDecoder::decodeNextFrame() {
	assert(_nextFrame < (int)_frameCount);

	if (_stream) {
		applyFrameToCurrent(readNextFrame());
	} else {
		applyFrameToCurrent(_nextFrame);
	}

	_nextFrame++;
	return &_currentFrameBuffer;
}

void RLFDecoder::applyFrameToCurrent(uint frameNumber) {
	if (_frames[frameNumber].type == Masked) {
		decodeMaskedRunLengthEncoding(_frames[frameNumber].encodedData, (int8 *)_currentFrameBuffer.getPixels(), _frames[frameNumber].encodedSize, _frameBufferByteSize);
	} else if (_frames[frameNumber].type == Simple) {
		decodeSimpleRunLengthEncoding(_frames[frameNumber].encodedData, (int8 *)_currentFrameBuffer.getPixels(), _frames[frameNumber].encodedSize, _frameBufferByteSize);
	}
}

void RLFDecoder::applyFrameToCurrent(const RLFDecoder::Frame &frame) {
	if (frame.type == Masked) {
		decodeMaskedRunLengthEncoding(frame.encodedData, (int8 *)_currentFrameBuffer.getPixels(), frame.encodedSize, _frameBufferByteSize);
	} else if (frame.type == Simple) {
		decodeSimpleRunLengthEncoding(frame.encodedData, (int8 *)_currentFrameBuffer.getPixels(), frame.encodedSize, _frameBufferByteSize);
	}
}

void RLFDecoder::decodeMaskedRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const {
	uint32 sourceOffset = 0;
	uint32 destOffset = 0;
	int16 numberOfCopy = 0;

	while (sourceOffset < sourceSize) {
		int8 numberOfSamples = source[sourceOffset];
		sourceOffset++;

		// If numberOfSamples is negative, the next abs(numberOfSamples) samples should
		// be copied directly from source to dest
		if (numberOfSamples < 0) {
			numberOfCopy = -numberOfSamples;

			while (numberOfCopy > 0) {
				if (sourceOffset + 1 >= sourceSize) {
					return;
				} else if (destOffset + 1 >= destSize) {
					debug(2, "Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				byte r, g, b;
				Graphics::colorToRGB<Graphics::ColorMasks<555> >(READ_LE_UINT16(source + sourceOffset), r, g, b);
				uint16 destColor = Graphics::RGBToColor<Graphics::ColorMasks<565> >(r, g, b);
				WRITE_UINT16(dest + destOffset, destColor);

				sourceOffset += 2;
				destOffset += 2;
				numberOfCopy--;
			}

			// If numberOfSamples is >= 0, move destOffset forward ((numberOfSamples * 2) + 2)
			// This function assumes the dest buffer has been memset with 0's.
		} else {
			if (sourceOffset + 1 >= sourceSize) {
				return;
			} else if (destOffset + 1 >= destSize) {
				debug(2, "Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
				return;
			}

			destOffset += (numberOfSamples * 2) + 2;
		}
	}
}

void RLFDecoder::decodeSimpleRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const {
	uint32 sourceOffset = 0;
	uint32 destOffset = 0;
	int16 numberOfCopy = 0;

	while (sourceOffset < sourceSize) {
		int8 numberOfSamples = source[sourceOffset];
		sourceOffset++;

		// If numberOfSamples is negative, the next abs(numberOfSamples) samples should
		// be copied directly from source to dest
		if (numberOfSamples < 0) {
			numberOfCopy = -numberOfSamples;

			while (numberOfCopy > 0) {
				if (sourceOffset + 1 >= sourceSize) {
					return;
				} else if (destOffset + 1 >= destSize) {
					debug(2, "Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				byte r, g, b;
				Graphics::colorToRGB<Graphics::ColorMasks<555> >(READ_LE_UINT16(source + sourceOffset), r, g, b);
				uint16 destColor = Graphics::RGBToColor<Graphics::ColorMasks<565> >(r, g, b);
				WRITE_UINT16(dest + destOffset, destColor);

				sourceOffset += 2;
				destOffset += 2;
				numberOfCopy--;
			}

			// If numberOfSamples is >= 0, copy one sample from source to the
			// next (numberOfSamples + 2) dest spots
		} else {
			if (sourceOffset + 1 >= sourceSize) {
				return;
			}

			byte r, g, b;
			Graphics::colorToRGB<Graphics::ColorMasks<555> >(READ_LE_UINT16(source + sourceOffset), r, g, b);
			uint16 sampleColor = Graphics::RGBToColor<Graphics::ColorMasks<565> >(r, g, b);
			sourceOffset += 2;

			numberOfCopy = numberOfSamples + 2;
			while (numberOfCopy > 0) {
				if (destOffset + 1 >= destSize) {
					debug(2, "Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				WRITE_UINT16(dest + destOffset, sampleColor);
				destOffset += 2;
				numberOfCopy--;
			}
		}
	}
}

} // End of namespace ZVision
