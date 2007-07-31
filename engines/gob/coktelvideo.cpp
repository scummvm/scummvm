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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/system.h"

#include "gob/coktelvideo.h"

namespace Gob {

Imd::Imd() {
	clear(false);
}

Imd::~Imd() {
	clear();
}

bool Imd::load(Common::SeekableReadStream &stream) {
	unload();

	_stream = &stream;

	// Version
	uint16 handle = _stream->readUint16LE();
	_version = _stream->readByte();

	// Version checking
	if ((handle != 0) || (_version < 2)) {
		warning("IMD Version incorrect (%d,%X)", handle, _version);
		unload();
		return false;
	}

	// Rest header
	_features = _stream->readByte();
	_framesCount = _stream->readUint16LE();
	_x = _stream->readSint16LE();
	_y = _stream->readSint16LE();
	_width = _stream->readSint16LE();
	_height = _stream->readSint16LE();
	_flags = _stream->readUint16LE();
	_firstFramePos = _stream->readUint16LE();

	// IMDs always have video
	_features |= kFeaturesVideo;

	// Palette
	_stream->read((byte *) _palette, 768);

	// Standard coordinates
	if (_version >= 3) {
		_stdX = _stream->readUint16LE();
		if (_stdX > 1) {
			warning("IMD: More than one standard coordinate quad found (%d)", _stdX);
			unload();
			return false;
		}
		if (_stdX != 0) {
			_stdX = _stream->readSint16LE();
			_stdY = _stream->readSint16LE();
			_stdWidth = _stream->readSint16LE();
			_stdHeight = _stream->readSint16LE();
			_features |= kFeaturesStdCoords;
		} else
			_stdX = -1;
	} else
		_stdX = -1;

	// Offset to frame positions table
	uint32 framesPosPos = 0;
	if (_version >= 4) {
		framesPosPos = _stream->readUint32LE();
		if (framesPosPos != 0) {
			_framesPos = new uint32[_framesCount];
			assert(_framesPos);
			_features |= kFeaturesFramesPos;
		}
	}

	// Offset to frame coordinates
	uint32 framesCoordsPos = 0;
	if (_features & kFeaturesFrameCoords)
		framesCoordsPos = _stream->readUint32LE();

	// Sound
	if (_features & kFeaturesSound) {
		_soundFreq = _stream->readSint16LE();
		_soundSliceSize = _stream->readUint16LE();
		_soundSlicesCount = _stream->readSint16LE();

		if (_soundFreq < 0)
			_soundFreq = -_soundFreq;

		if (_soundSlicesCount < 0)
			_soundSlicesCount = -_soundSlicesCount - 1;

		if (_soundSlicesCount > 40) {
			warning("IMD: More than 40 sound slices found (%d)", _soundSlicesCount);
			unload();
			return false;
		}

		_soundSliceLength = 1000 / (_soundFreq / _soundSliceSize);
		_frameLength = _soundSliceLength;

		_soundStage = 1;
		_hasSound = true;

		_audioStream = Audio::makeAppendableAudioStream(_soundFreq, 0);
	} else
		_frameLength = 1000 / 12; // 12 FPS for a video without sound

	// Sizes of the frame data and extra video buffer
	if (_features & kFeaturesDataSize) {
		_frameDataSize = _stream->readUint16LE();
		if (_frameDataSize == 0) {
			_frameDataSize = _stream->readUint32LE();
			_vidBufferSize = _stream->readUint32LE();
		} else
			_vidBufferSize = _stream->readUint16LE();
	} else {
		_frameDataSize = _width * _height + 500;
		if (!(_flags & 0x100) || (_flags & 0x1000))
			_vidBufferSize = _frameDataSize;
	}

	// Frame positions table
	if (_framesPos) {
		_stream->seek(framesPosPos, SEEK_SET);
		for (int i = 0; i < _framesCount; i++)
			_framesPos[i] = _stream->readUint32LE();
	}

	// Frame coordinates table
	if (_features & kFeaturesFrameCoords) {
		_stream->seek(framesCoordsPos, SEEK_SET);
		_frameCoords = new Coord[_framesCount];
		assert(_frameCoords);
		for (int i = 0; i < _framesCount; i++) {
			_frameCoords[i].left = _stream->readSint16LE();
			_frameCoords[i].top = _stream->readSint16LE();
			_frameCoords[i].right = _stream->readSint16LE();
			_frameCoords[i].bottom = _stream->readSint16LE();
		}
	}

	// Seek to the first frame
	_stream->seek(_firstFramePos, SEEK_SET);

	// Allocating working memory
	_frameData = new byte[_frameDataSize + 500];
	assert(_frameData);
	memset(_frameData, 0, _frameDataSize + 500);
	_vidBuffer = new byte[_vidBufferSize + 500];
	assert(_vidBuffer);
	memset(_vidBuffer, 0, _vidBufferSize + 500);

	return true;
}

void Imd::unload() {
	clear();
}

void Imd::setXY(int16 x, int16 y) {
	// Adjusting the standard coordinates
	if (_stdX != -1) {
		if (x >= 0)
			_stdX = _stdX - _x + x;
		if (y >= 0)
			_stdY = _stdY - _y + y;
	}
	
	// Going through the coordinate table as well
	if (_frameCoords) {
		for (int i = 0; i < _framesCount; i++) {
			if (_frameCoords[i].left != -1) {
				if (x >= 0) {
					_frameCoords[i].left = _frameCoords[i].left - _x + x;
					_frameCoords[i].right = _frameCoords[i].right - _x + x;
				}
				if (y >= 0) {
					_frameCoords[i].top = _frameCoords[i].top - _y + y;
					_frameCoords[i].bottom = _frameCoords[i].bottom - _y + y;
				}
			}
		}
	}

	if (x >= 0)
		_x = x;
	if (y >= 0)
		_y = y;
}

void Imd::setVideoMemory(byte *vidMem, uint16 width, uint16 height) {
	deleteVidMem();

	_hasOwnVidMem = false;
	_vidMem = vidMem;
	_vidMemWidth = width;
	_vidMemHeight = height;
}

void Imd::setVideoMemory() {
	deleteVidMem();

	if ((_width > 0) && (_height > 0)) {
		setXY(0, 0);
		_hasOwnVidMem = true;
		_vidMem = new byte[_width * _height];
		_vidMemWidth = _width;
		_vidMemHeight = _height;
	}
}

void Imd::enableSound(Audio::Mixer &mixer) {
	// Only possible on the first frame
	if (_curFrame > 0)
		return;

	_mixer = &mixer;
	_soundEnabled = true;
}

void Imd::disableSound() {
	if (_audioStream) {

		if (_soundStage == 2) {
			_audioStream->finish();
			_mixer->stopHandle(_audioHandle);
		} else
			delete _audioStream;

		_audioStream = 0;
	}
	_soundEnabled = false;
	_mixer = 0;
}

void Imd::seekFrame(int32 frame, int16 whence, bool restart) {
	if (!_stream)
		// Nothing to do
		return;

	// Find the frame to which to seek
	if (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _framesCount - frame - 1;
	else if (whence != SEEK_SET)
		return;

	if ((frame < 0) || (frame >= _framesCount) || (frame == _curFrame))
		// Nothing to do
		return;

	// Try every possible way to find a file offset to that frame
	uint32 framePos = 0;
	if (frame == 0) {
		framePos = _firstFramePos;
	} else if (frame == 1) {
		framePos = _firstFramePos;
		_stream->seek(framePos, SEEK_SET);
		framePos += _stream->readUint16LE() + 4;
	} else if (_framesPos) {
		framePos = _framesPos[frame];
	} else if (restart && (_soundStage == 0)) {
		for (int i = ((frame > _curFrame) ? _curFrame : 0); i <= frame; i++)
			processFrame(i);
	} else
		error("Frame %d is not directly accessible", frame);

	// Seek
	_stream->seek(framePos);
	_curFrame = frame;
}

CoktelVideo::State Imd::nextFrame() {
	return processFrame(_curFrame);
}

void Imd::waitEndFrame() {
	if (_soundEnabled && _hasSound) {
		if (_soundStage != 2)
			return;

		if (_skipFrames == 0) {
			int32 waitTime = (_curFrame * _soundSliceLength) -
				(g_system->getMillis() - _soundStartTime);

			if (waitTime < 0) {
				_skipFrames = -waitTime / _soundSliceLength;
				warning("IMD A/V sync broken, skipping %d frame(s)", _skipFrames + 1);
			} else if (waitTime > 0)
				g_system->delayMillis(waitTime);

		} else
			_skipFrames--;
	} else
		g_system->delayMillis(_frameLength);
}

void Imd::copyCurrentFrame(byte *dest, uint16 x, uint16 y, uint16 width, int16 transp) {
	if (!_vidMem)
		return;

	dest += width * y;

	uint16 copyWidth = MIN<int16>(width - x, _width);
	uint16 destPitch = width - x;
	byte *vidMem = _vidMem;

	if (transp < 0) {
		// No transparency
		if ((x > 0) || (_width != width)) {
			// Copy row-by-row
			for (int i = 0; i < _height; i++) {
				dest += x;
				memcpy(dest, vidMem, copyWidth);
				dest += destPitch;
				vidMem += _width;
			}

		} else
			// Dimensions fit, copy everything at once
			memcpy(dest, _vidMem, _width * _height);

		return;
	}

	// Transparency, copy per pixel
	for (int i = 0; i < _height; i++) {
		byte *s = vidMem;
		byte *d = dest;

		d += x;
		for (int j = 0; j < _width; j++) {
			if (*s != transp)
				*d = *s;

			s++;
			d++;
		}

		dest += width;;
		vidMem += _width;
	}
}

void Imd::deleteVidMem(bool del) {
	if (del) {
		if (_hasOwnVidMem)
			delete[] _vidMem;
	}

	_hasOwnVidMem = false;
	_vidMem = 0;
	_vidMemWidth = _vidMemHeight = 0;
}

void Imd::clear(bool del) {
	if (del) {
		delete[] _framesPos;
		delete[] _frameCoords;
		delete[] _frameData;
		delete[] _vidBuffer;

		disableSound();
	}

	_stream = 0;

	_version = 0;
	_features = 0;
	_flags = 0;
	_x = _y = _width = _height = 0;
	_stdX = _stdY = _stdWidth = _stdHeight = 0;
	_framesCount = _curFrame = 0;
	_framesPos = 0;
	_firstFramePos = 0;
	_frameCoords = 0;

	_frameDataSize = _vidBufferSize = 0;
	_frameData = _vidBuffer = 0;

	memset(_palette, 0, 768);

	deleteVidMem(del);

	_hasSound = false;
	_soundEnabled = false;
	_soundStage = 0;
	_soundStartTime = 0;
	_skipFrames = 0;

	_soundFreq = 0;
	_soundSliceSize = 0;
	_soundSlicesCount = 0;
	_soundSliceLength = 0;

	_audioStream = 0;

	_frameLength = 0;
	_lastFrameTime = 0;
}

CoktelVideo::State Imd::processFrame(uint16 frame) {
	State state;
	uint32 cmd = 0;
	int16 xBak, yBak, heightBak, widthBak;
	bool hasNextCmd = false;
	bool startSound = false;

	if (!_stream || (frame >= _framesCount)) {
		state.flags = kStateBreak;
		return state;
	}

	if (frame != _curFrame) {
		state.flags |= kStateSeeked;
		seekFrame(frame);
	}

	state.left = xBak = _x;
	state.top = yBak = _y;
	state.bottom = heightBak = _height;
	state.right = widthBak = _width;
	state.right += state.left - 1;
	state.bottom += state.top - 1;

	do {
		if (frame != 0) {
			if (_stdX != -1) {
				state.left = _x = _stdX;
				state.top = _y = _stdY;
				state.right = _width = _stdWidth;
				state.bottom = _height = _stdHeight;
				state.right += state.left - 1;
				state.bottom += state.top - 1;
				state.flags |= kStateStdCoords;
			}
			if (_frameCoords &&
					(_frameCoords[frame].left != -1)) {
				state.left = _x = _frameCoords[frame].left;
				state.top = _y = _frameCoords[frame].top;
				state.right = _width =
					_frameCoords[frame].right - _x + 1;
				state.bottom = _height =
					_frameCoords[frame].bottom - _y + 1;
				state.right += state.left - 1;
				state.bottom += state.top - 1;
				state.flags |= kStateFrameCoords;
			}
		}

		cmd = _stream->readUint16LE();

		if ((cmd & 0xFFF8) == 0xFFF0) {
			if (cmd == 0xFFF0) {
				_stream->seek(2, SEEK_CUR);
				cmd = _stream->readUint16LE();
			}

			if (cmd == 0xFFF1) {
				state.flags = kStateBreak;
				continue;
			} else if (cmd == 0xFFF2) { // Skip (16 bit)
				cmd = _stream->readUint16LE();
				_stream->seek(cmd, SEEK_CUR);
				state.flags = kStateBreak;
				continue;
			} else if (cmd == 0xFFF3) { // Skip (32 bit)
				cmd = _stream->readUint32LE();
				_stream->seek(cmd, SEEK_CUR);
				state.flags = kStateBreak;
				continue;
			}
		}

		if (_soundStage != 0) {
			byte *soundBuf;

			// Next sound slice data
			if (cmd == 0xFF00) {

				if (!hasNextCmd && _soundEnabled) {
					soundBuf = new byte[_soundSliceSize];
					assert(soundBuf);

					_stream->read(soundBuf, _soundSliceSize);
					unsignedToSigned(soundBuf, _soundSliceSize);
					_audioStream->queueBuffer(soundBuf, _soundSliceSize);
				} else
					_stream->seek(_soundSliceSize, SEEK_CUR);

				cmd = _stream->readUint16LE();

			// Initial sound data (all slices)
			} else if (cmd == 0xFF01) {
				int dataLength = _soundSliceSize * _soundSlicesCount;

				if (!hasNextCmd && _soundEnabled) {
					soundBuf = new byte[dataLength];
					assert(soundBuf);

					_stream->read(soundBuf, dataLength);
					unsignedToSigned(soundBuf, dataLength);

					_soundStage = 1;
					startSound = true;
					_audioStream->queueBuffer(soundBuf, dataLength);
				} else
					_stream->seek(dataLength, SEEK_CUR);

				cmd = _stream->readUint16LE();

			// Empty sound slice
			} else if (!hasNextCmd && (_soundEnabled)) {
				soundBuf = new byte[_soundSliceSize];
				assert(soundBuf);

				memset(soundBuf, 0, _soundSliceSize);
				_audioStream->queueBuffer(soundBuf, _soundSliceSize);
			}
		}

		// Set palette
		if (cmd == 0xFFF4) {
			_stream->seek(2, SEEK_CUR);
			state.flags |= kStatePalette;
			_stream->read(_palette, 768);
			cmd = _stream->readUint16LE();
		}

		hasNextCmd = false;

		// Jump to frame
		if (cmd == 0xFFFD) {

			frame = _stream->readSint16LE();
			if (_framesPos) {
				_curFrame = frame;
				_stream->seek(_framesPos[frame], SEEK_SET);

				hasNextCmd = true;
				state.flags |= kStateJump;
			}

		} else if (cmd == 0xFFFC) {

			state.flags |= 1;
			cmd = _stream->readUint32LE();
			_stream->read(_frameData, cmd + 2);

			int16 left = _x;
			int16 top = _y;
			int16 right = _width + left;
			int16 bottom = _height + top;

			if (!_vidMem)
				setVideoMemory();

			if (_vidMemWidth < right) {
				left = 0;
				right = _width;
			}
			if (_vidMemWidth < right)
				right = _vidMemWidth;
			if (_vidMemHeight < bottom) {
				top = 0;
				bottom = _height;
			}
			if (_vidMemHeight < bottom)
				bottom = _vidMemHeight;

			_x = left;
			_y = top;
			_height = bottom - top;
			_width = right - left;

			state.flags |= renderFrame();
			state.flags |= _frameData[0];

		// Frame video data
		} else if (cmd != 0) {

			_stream->read(_frameData, cmd + 2);

			state.flags |= renderFrame();
			state.flags |= _frameData[0];

		} else
			state.flags |= kStateNoVideoData;

	} while (hasNextCmd);

	if (startSound && _soundEnabled) {
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_audioHandle, _audioStream);
		_soundStartTime = g_system->getMillis();
		_skipFrames = 0;
		_soundStage = 2;
	}

	_x = xBak;
	_y = yBak;
	_width = widthBak;
	_height = heightBak;

	_curFrame++;
	if ((_curFrame == _framesCount) && (_soundStage == 2)) {
		_audioStream->finish();
		_mixer->stopHandle(_audioHandle);
		_audioStream = 0;
		_soundStage = 0;
	}

	_lastFrameTime = g_system->getMillis();
	return state;
}

uint32 Imd::renderFrame() {
	if (!_frameData || (_width <= 0) || (_height <= 0))
		return 0;

	if (!_vidMem)
		setVideoMemory();

	byte *dataPtr = _frameData;
	int16 imdX = _x;
	int16 imdY = _y;
	int16 imdW = _width;
	int16 imdH = _height;
	int16 sW = _vidMemWidth;
	byte *imdVidMem = _vidMem + sW * imdY + imdX;
	uint8 type = *dataPtr++;
	byte *srcPtr = dataPtr;
	uint32 retVal = 0;

	if (type & 0x10) { // Palette data
		// One byte index
		int index = *dataPtr++;
		// 16 entries with each 3 bytes (RGB)
		memcpy(_palette + index * 3, dataPtr, MIN((255 - index) * 3, 48));

		retVal = kStatePalette;
		dataPtr += 48;
		type ^= 0x10;
	}

	srcPtr = dataPtr;
	if (type & 0x80) { // Frame data is compressed
		srcPtr = _vidBuffer;
		type &= 0x7F;
		if ((type == 2) && (imdW == sW)) {
			deLZ77(imdVidMem, dataPtr);
			return retVal;
		} else
			deLZ77(srcPtr, dataPtr);
	}

	uint16 pixCount, pixWritten;
	byte *imdVidMemBak;

	if (type == 2) { // Whole block
		for (int i = 0; i < imdH; i++) {
			memcpy(imdVidMem, srcPtr, imdW);
			srcPtr += imdW;
			imdVidMem += sW;
		}
	} else if (type == 1) { // Sparse block
		imdVidMemBak = imdVidMem;
		for (int i = 0; i < imdH; i++) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) { // data
					pixCount = MIN((pixCount & 0x7F) + 1, imdW - pixWritten);
					memcpy(imdVidMem, srcPtr, pixCount);

					pixWritten += pixCount;
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else { // "hole"
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			imdVidMemBak += sW;
			imdVidMem = imdVidMemBak;
		}
	} else if (type == 0x42) { // Whole quarter-wide block
		for (int i = 0; i < imdH; i++) {
			imdVidMemBak = imdVidMem;

			for (int j = 0; j < imdW; j += 4, imdVidMem += 4, srcPtr++)
				memset(imdVidMem, *srcPtr, 4);

			imdVidMemBak += sW;
			imdVidMem = imdVidMemBak;
		}
	} else if ((type & 0xF) == 2) { // Whole half-high block
		for (; imdH > 1; imdH -= 2, imdVidMem += sW + sW, srcPtr += imdW) {
			memcpy(imdVidMem, srcPtr, imdW);
			memcpy(imdVidMem + sW, srcPtr, imdW);
		}
		if (imdH == -1)
			memcpy(imdVidMem, srcPtr, imdW);
	} else { // Sparse half-high block
		imdVidMemBak = imdVidMem;
		for (int i = 0; i < imdH; i += 2) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) { // data
					pixCount = MIN((pixCount & 0x7F) + 1, imdW - pixWritten);
					memcpy(imdVidMem, srcPtr, pixCount);
					memcpy(imdVidMem + sW, srcPtr, pixCount);

					pixWritten += pixCount;
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else { // "hole"
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			imdVidMemBak += sW + sW;
			imdVidMem = imdVidMemBak;
		}
	}

	return retVal;
}

void Imd::deLZ77(byte *dest, byte *src) {
	int i;
	byte buf[4370];
	uint16 chunkLength;
	uint16 frameLength;
	uint16 bufPos1;
	uint16 bufPos2;
	uint16 tmp;
	uint8 chunkBitField;
	uint8 chunkCount;
	bool mode;

	frameLength = READ_LE_UINT32(src);
	src += 4;

	if ((READ_LE_UINT16(src) == 0x1234) && (READ_LE_UINT16(src + 2) == 0x5678)) {
		src += 4;
		bufPos1 = 273;
		mode = 1; // 123Ch (cmp al, 12h)
	} else {
		bufPos1 = 4078;
		mode = 0; // 275h (jnz +2)
	}

	memset(buf, 32, bufPos1);
	chunkCount = 1;
	chunkBitField = 0;

	while (frameLength > 0) {
		chunkCount--;
		if (chunkCount == 0) {
			tmp = *src++;
			chunkCount = 8;
			chunkBitField = tmp;
		}
		if (chunkBitField % 2) {
			chunkBitField >>= 1;
			buf[bufPos1] = *src;
			*dest++ = *src++;
			bufPos1 = (bufPos1 + 1) % 4096;
			frameLength--;
			continue;
		}
		chunkBitField >>= 1;

		tmp = READ_LE_UINT16(src);
		src += 2;
		chunkLength = ((tmp & 0xF00) >> 8) + 3;

		if ((mode && ((chunkLength & 0xFF) == 0x12)) ||
				(!mode && (chunkLength == 0)))
			chunkLength = *src++ + 0x12;

		bufPos2 = (tmp & 0xFF) + ((tmp >> 4) & 0x0F00);
		if (((tmp + chunkLength) >= 4096) ||
				((chunkLength + bufPos1) >= 4096)) {

			for (i = 0; i < chunkLength; i++, dest++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
				bufPos1 = (bufPos1 + 1) % 4096;
				bufPos2 = (bufPos2 + 1) % 4096;
			}

		} else if (((tmp + chunkLength) < bufPos1) ||
				((chunkLength + bufPos1) < bufPos2)) {

			memcpy(dest, buf + bufPos2, chunkLength);
			memmove(buf + bufPos1, buf + bufPos2, chunkLength);

			dest += chunkLength;
			bufPos1 += chunkLength;
			bufPos2 += chunkLength;

		} else {

			for (i = 0; i < chunkLength; i++, dest++, bufPos1++, bufPos2++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
			}

		}
		frameLength -= chunkLength;

	}
}

Vmd::Vmd() {
	clear(false);
}

Vmd::~Vmd() {
	clear();
}

bool Vmd::load(Common::SeekableReadStream &stream) {
	unload();

	_stream = &stream;

	uint16 headerLength = _stream->readUint16LE();
	uint16 handle = _stream->readUint16LE();
	_version = _stream->readUint16LE();

	// Version checking
	if ((headerLength != 814) || (handle != 0) || (_version != 1)) {
		warning("IMD Version incorrect (%d, %d, %d)", headerLength, handle, _version);
		unload();
		return false;
	}

	_framesCount = _stream->readUint16LE();

	warning("# of frames: %d", _framesCount);

	_x = _stream->readSint16LE();
	_y = _stream->readSint16LE();
	_width = _stream->readSint16LE();
	_height = _stream->readSint16LE();
	if ((_width != 0) && (_height != 0)) {
		_hasVideo = true;
		_features |= kFeaturesVideo;

		warning("%dx%d+%d+%d", _width, _height, _x, _y);

	} else
		_hasVideo = false;

	_flags = _stream->readUint16LE();
	_partsPerFrame = _stream->readUint16LE();
	_firstFramePos = _stream->readUint32LE();
	uint32 unknown1 = _stream->readUint32LE();

	warning("flags: %d (0x%X), #parts: %d, firstFramePos: %d, U1: %d (0x%X)",
			_flags, _flags, _partsPerFrame, _firstFramePos, unknown1, unknown1);

	_stream->read((byte *) _palette, 768);

	_frameDataSize = _stream->readUint32LE();
	_vidBufferSize = _stream->readUint32LE();

	if (_hasVideo) {
		if (_frameDataSize == 0)
			_frameDataSize = _width * _height + 500;
		if (_vidBufferSize)
			_vidBufferSize = _frameDataSize;

		_frameData = new byte[_frameDataSize];
		assert(_frameData);
		memset(_frameData, 0, _frameDataSize);
		_vidBuffer = new byte[_vidBufferSize];
		assert(_vidBuffer);
		memset(_vidBuffer, 0, _vidBufferSize);
		warning("Sizes: frameData: %d, vidBuffer: %d", _frameDataSize, _vidBufferSize);
	}

	_soundFreq = _stream->readSint16LE();
	_soundSliceSize = _stream->readUint16LE();
	_soundSlicesCount = _stream->readSint16LE();
	uint16 soundFlags = _stream->readUint16LE();
	_hasSound = (_soundFreq != 0);

	if (_hasSound) {
		_features |= kFeaturesSound;

		_soundSliceLength = 1000 / (_soundFreq / _soundSliceSize);
		_frameLength = _soundSliceLength;

		_soundStage = 1;

		_audioStream = Audio::makeAppendableAudioStream(_soundFreq, 0);

		warning("Sound: Freq: %d, # slices %d, slideSize: %d, flags: %d (0x%X), sliceLen = %d",
				_soundFreq, _soundSlicesCount, _soundSliceSize, soundFlags, soundFlags, _soundSliceLength);

	} else
		_frameLength = 1000 / 12; // 12 FPS for a video without sound

	uint32 frameInfoOffset = _stream->readUint32LE();

	warning("frameInfoOffset: %d", frameInfoOffset);

	_stream->seek(frameInfoOffset);
	_frames = new Frame[_framesCount];
	for (uint16 i = 0; i < _framesCount; i++) {
		_frames[i].parts = new Part[_partsPerFrame];
		_stream->skip(2);
		_frames[i].offset = _stream->readUint32LE();
	}
	for (uint16 i = 0; i < _framesCount; i++) {
		for (uint16 j = 0; j < _partsPerFrame; j++) {
			_frames[i].parts[j].type = (PartType) _stream->readByte();
			uint16 Unknown3 = _stream->readByte();
			_frames[i].parts[j].size = _stream->readUint32LE();
			if (_frames[i].parts[j].type == kPartTypeAudio) {
				_frames[i].parts[j].flags = _stream->readByte();
				_stream->skip(9);
				warning("%d.%d (%d): Audio: %d (0x%X), %d (0x%X)",
						i, j, _frames[i].parts[j].size,
						Unknown3, Unknown3, _frames[i].parts[j].flags, _frames[i].parts[j].flags);

			} else if (_frames[i].parts[j].type == kPartTypeVideo) {
				_frames[i].parts[j].left = _stream->readUint16LE();
				_frames[i].parts[j].top = _stream->readUint16LE();
				_frames[i].parts[j].right = _stream->readUint16LE();
				_frames[i].parts[j].bottom = _stream->readUint16LE();
				uint16 Unknown4 = _stream->readByte();
				_frames[i].parts[j].flags = _stream->readByte();
				warning("%d.%d (%d): Video: %d (0x%X), %d+%d+%d+%d, %d (0x%X), %d (0x%X)",
						i, j, _frames[i].parts[j].size,
						Unknown3, Unknown3, _frames[i].parts[j].left,
						_frames[i].parts[j].top, _frames[i].parts[j].right,
						_frames[i].parts[j].bottom, Unknown4, Unknown4,
						_frames[i].parts[j].flags, _frames[i].parts[j].flags);

			} else {
				warning("VMD: Unknown frame part type found (%d.%d: %d, %d)",
						i, j, _frames[i].parts[j].type, _frames[i].parts[j].size);
				_stream->skip(10);
//				unload();
//				return false;
			}
		}
	}

	return true;
}

void Vmd::unload() {
	clear();
}

void Vmd::setXY(int16 x, int16 y) {

	for (int i = 0; i < _framesCount; i++) {
		for (int j = 0; j < _partsPerFrame; j++) {

			if (_frames[i].parts[j].type == kPartTypeVideo) {
				if (x >= 0) {
					_frames[i].parts[j].left = _frames[i].parts[j].left - _x + x;
					_frames[i].parts[j].right = _frames[i].parts[j].right - _x + x;
				}
				if (y >= 0) {
					_frames[i].parts[j].top = _frames[i].parts[j].top - _y + y;
					_frames[i].parts[j].bottom = _frames[i].parts[j].bottom - _y + y;
				}
			}

		}
	}

	if (x >= 0)
		_x = x;
	if (y >= 0)
		_y = y;
}

void Vmd::seekFrame(int32 frame, int16 whence, bool restart) {
	if (!_stream)
		// Nothing to do
		return;

	// Find the frame to which to seek
	if (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _framesCount - frame - 1;
	else if (whence != SEEK_SET)
		return;

	if ((frame < 0) || (frame >= _framesCount))
		// Nothing to do
		return;

	// Seek
	_stream->seek(_frames[frame].offset);
	_curFrame = frame;
}

CoktelVideo::State Vmd::nextFrame() {
	State state;

	state = processFrame(_curFrame);
	_curFrame++;
	return state;
}

void Vmd::clear(bool del) {
	Imd::clear(del);

	if (del) {
		delete[] _frames;
	}

	_hasVideo = true;
	_partsPerFrame = 0;
	_frames = 0;
}

CoktelVideo::State Vmd::processFrame(uint16 frame) {
	State state;
	int16 xBak = 0, yBak = 0, heightBak = 0, widthBak = 0;
	bool startSound = false;

	seekFrame(frame);

	state.flags |= kStateNoVideoData;
	state.left = -1;

	for (uint16 i = 0; i < _partsPerFrame; i++) {
		Part &part = _frames[frame].parts[i];

		if (part.type == kPartTypeAudio) {
			byte *soundBuf;

			// Next sound slice data
			if (part.flags == 1) {

				if (_soundEnabled) {
					soundBuf = new byte[part.size];
					assert(soundBuf);

					_stream->read(soundBuf, part.size);
					unsignedToSigned(soundBuf, part.size);

					_audioStream->queueBuffer(soundBuf, part.size);
				} else
					_stream->skip(part.size);

			// Initial sound data (all slices)
			} else if (part.flags == 2) {

				if (_soundEnabled) {
					uint32 U = _stream->readUint32LE();

					warning("Mask? %d (0x%X)", U, U);

					soundBuf = new byte[part.size - 4];
					assert(soundBuf);

					_stream->read(soundBuf, part.size - 4);
					unsignedToSigned(soundBuf, part.size - 4);

					_audioStream->queueBuffer(soundBuf, part.size - 4);

					if (_soundStage == 1) {
						startSound = true;
					}

				} else
					_stream->skip(part.size);

			// Empty sound slice
			} else if (part.flags == 3) {

				if (_soundEnabled && (part.size > 0)) {
					soundBuf = new byte[part.size];
					assert(soundBuf);

					memset(soundBuf, 0, part.size);

					_audioStream->queueBuffer(soundBuf, part.size);
				} else
					_stream->skip(part.size);
			}

		} else if (part.type == kPartTypeVideo) {
			state.flags &= ~kStateNoVideoData;

			if (state.left == -1) {
				state.left = _x = part.left;
				state.top = _y = part.top;
				state.right = _width = part.right;
				state.bottom = _height = part.bottom;
				_width -= _x - 1;
				_height -= _y - 1;
			} else {
				_x = part.left;
				_y = part.top;
				_width = part.right - (_x - 1);
				_height = part.bottom - (_y - 1);
				state.left = MIN(state.left, part.left);
				state.top = MIN(state.top, part.top);
				state.right = MAX(state.right, part.right);
				state.bottom = MAX(state.bottom, part.bottom);
			}

			if (part.flags & 2) {
				uint8 index = _stream->readByte();
				uint8 count = _stream->readByte();

				_stream->read(_palette + index * 3, count + 1);
				_stream->skip((255 - count) * 3);
				
				state.flags |= kStatePalette;
			}

			_stream->read(_frameData, part.size);
			state.flags |= renderFrame();
		} else {
			warning("Unknown frame part type %d, size %d (%d of %d)", part.type, part.size, i + 1, _partsPerFrame);
			_stream->skip(part.size);
		}
	}

	if (startSound && _soundEnabled) {
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_audioHandle, _audioStream);
		_soundStartTime = g_system->getMillis();
		_skipFrames = 0;
		_soundStage = 2;
	}

	_x = xBak;
	_y = yBak;
	_width = widthBak;
	_height = heightBak;

	if ((_curFrame == (_framesCount - 1)) && (_soundStage == 2)) {
		_audioStream->finish();
		_mixer->stopHandle(_audioHandle);
		_audioStream = 0;
		_soundStage = 0;
	}

	_lastFrameTime = g_system->getMillis();
	return state;
}

} // End of namespace Gob
