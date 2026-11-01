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

#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/debug.h"

#include "engines/grim/movie/codecs/blocky8.h"
#include "engines/grim/movie/codecs/codec48.h"
#include "engines/grim/movie/codecs/smush_decoder.h"
#include "engines/grim/movie/smush_animator.h"

namespace Grim {

#define SMUSH_TAG_NULL MKTAG('n', 'u', 'l', 'l')
#define SMUSH_TAG_END MKTAG('e', 'n', 'd', ' ')
#define SMUSH_TAG_PAUSE MKTAG('p', 'a', 'u', 's')

SmushAnimator::SmushAnimator(SmushDecoder *decoder) {
	_decoder = decoder;
	_buffer = nullptr;
	_bufferSize = 0;
	_block = nullptr;
	_blockStatus = 0;

	animatorInitialize();
}

SmushAnimator::~SmushAnimator() {
	free(_buffer);
}

void SmushAnimator::animatorInitialize() {
	_frame = 0;
	_blockSize = 0;
	_tag = SMUSH_TAG_NULL;
	_frameCount = 1;
	_pause = 0;
}

void SmushAnimator::animatorNewAnim() {
	animatorInitialize();

	_frameCount = _decoder->getFrameCount();
}

void SmushAnimator::animatorPause(bool pause) {
	_pause = pause;
}

void SmushAnimator::readBlock(int size) {
	Common::SeekableReadStream *file = _decoder->getFile();

	if (!file || file->pos() >= file->size()) {
		_block = nullptr;
		_blockStatus = 0;
		_frame = -1;
		return;
	}

	if (file->size() - file->pos() < size) {
		_block = nullptr;
		_blockStatus = -2;
		return;
	}

	if (_bufferSize < (uint32)size) {
		_buffer = (byte *)realloc(_buffer, size);
		_bufferSize = size;
	}

	file->read(_buffer, size);
	_block = _buffer;
	_blockStatus = 1;
}

void SmushAnimator::decodeBlocks(int flags) {
	// Only the "late" bit (0x8000) gets to the decoder, as flag 4
	int decodeFlags = (flags >> 13) & 4;
	_decoder->decodeFrameBlock(_block, _blockSize, decodeFlags);
}

bool SmushAnimator::moreData() {
	Common::SeekableReadStream *file = _decoder->getFile();
	return file && file->pos() < file->size();
}

int SmushAnimator::animatorRender(int flags) {
	while (true) {
		switch (_tag) {
		case MKTAG('A', 'N', 'N', 'O'):
			readBlock(_blockSize);
			if (_blockStatus == -2)
				return -2;
			if (!_block)
				return -2;

			_tag = SMUSH_TAG_NULL;
			break;
		case MKTAG('F', 'R', 'M', 'E'):
			readBlock(_blockSize);
			if (_blockStatus == -2)
				return -2;

			_tag = SMUSH_TAG_NULL;
			if (!_block)
				return -2;

			_frame++;
			decodeBlocks(flags);

			if (_frame < _frameCount - 1) {
				if (_pause)
					_tag = SMUSH_TAG_PAUSE;
			} else if (!moreData()) {
				_tag = SMUSH_TAG_END;
			}

			// The disasm never renders the last frame
			if (_frame >= 10)
				return _frame < _frameCount - 1 ? 0 : 4;

			return 1;
		case SMUSH_TAG_NULL:
			readBlock(8);
			if (_blockStatus == -2 || !_block)
				return -2;

			_tag = READ_BE_UINT32(_block);
			_blockSize = (READ_BE_UINT32(_block + 4) + 1) & ~1;
			break;
		case SMUSH_TAG_END:
			_frame++;

			if (moreData())
				_tag = SMUSH_TAG_NULL;

			return 4;
		case SMUSH_TAG_PAUSE:
			if (!_pause)
				_tag = SMUSH_TAG_NULL;

			return 3;
		default:
			return -2;
		}
	}
}

SmushDemoStream::SmushDemoStream() {
	_ring = nullptr;
	_ringSize = 0;
	_maxRead = 0;
	_writePos = 0;
	_readPos = 0;
	_held = 0;
	_rewindPos = 0;
	_alignSkip = 0;
	_file = nullptr;
	_fileOffset = 0;
	_fileSize = 0;
	_restart = 0;
	_hasData = 0;
	_released = 0;
	_refCount = 0;
	_openPending = false;
	_openOffset = 0;
}

SmushDemoStream::~SmushDemoStream() {
	delete _file;
	delete[] _ring;
}

int SmushDemoStream::streamInitialize(int ringSize, int maxRead) {
	if (_refCount++ != 0)
		return 0;

	_ring = new byte[maxRead + ringSize]();
	_maxRead = maxRead;
	_ringSize = ringSize & ~(SMUSH_DEMO_SECTOR_SIZE - 1);
	_file = nullptr;
	memset(_ring, 0, _ringSize);
	return 0;
}

void SmushDemoStream::streamTerminate() {
	if (--_refCount)
		return;

	if (_file) {
		delete _file;
		_file = nullptr;
	}

	delete[] _ring;
	_ring = nullptr;
}

void SmushDemoStream::streamReset() {
	_writePos = 0;
	_readPos = 0;
	_restart = 0;
	_hasData = 0;
	_held = 0;
	_alignSkip = 0;
	if (_file) {
		delete _file;
		_file = nullptr;
	}
}

void SmushDemoStream::streamOpen(const Common::String &name, int offset) {
	_openName = name;
	_openPending = true;
	_openOffset = offset;
}

int SmushDemoStream::streamDoOpen() {
	if (!_openPending)
		return -1;

	Common::File *file = new Common::File();
	if (!file->open(Common::Path(_openName))) {
		delete file;
		_openPending = false;
		return -2;
	}

	delete _file;
	_file = file;
	_fileSize = _file->size();

	_file->seek(_openOffset & ~(SMUSH_DEMO_SECTOR_SIZE - 1));
	_alignSkip = _openOffset & (SMUSH_DEMO_SECTOR_SIZE - 1);
	_fileOffset = _openOffset & ~(SMUSH_DEMO_SECTOR_SIZE - 1);
	_rewindPos = _writePos;
	_restart = 1;
	_hasData = 0;
	_openPending = false;
	return 0;
}

bool SmushDemoStream::streamIsReady() {
	return _hasData && _restart;
}

bool SmushDemoStream::streamHasSpace(int size) {
	int end = _writePos;
	if (_readPos > _writePos)
		end = _writePos + _ringSize;

	return end >= _readPos + size;
}

bool SmushDemoStream::streamFill(int limit) {
	bool opened = (streamDoOpen() == 0);
	int size;

	if (!_file || _fileOffset >= _fileSize)
		return false;

	if (_readPos > _writePos) {
		if (_readPos != 0) {
			size = (_readPos - _writePos - SMUSH_DEMO_SECTOR_SIZE) & ~(SMUSH_DEMO_SECTOR_SIZE - 1);
			if (size < SMUSH_DEMO_SECTOR_SIZE)
				return false;
		} else if (_ringSize - _writePos > SMUSH_DEMO_SECTOR_SIZE) {
			size = _ringSize - _writePos - SMUSH_DEMO_SECTOR_SIZE;
		} else {
			size = (_readPos - _writePos - SMUSH_DEMO_SECTOR_SIZE) & ~(SMUSH_DEMO_SECTOR_SIZE - 1);
			if (size < SMUSH_DEMO_SECTOR_SIZE)
				return false;
		}
	} else if (_readPos != 0) {
		size = _ringSize - _writePos;
	} else if (_ringSize - _writePos > SMUSH_DEMO_SECTOR_SIZE) {
		size = _ringSize - _writePos - SMUSH_DEMO_SECTOR_SIZE;
	} else {
		size = (_readPos - _writePos - SMUSH_DEMO_SECTOR_SIZE) & ~(SMUSH_DEMO_SECTOR_SIZE - 1);
		if (size < SMUSH_DEMO_SECTOR_SIZE)
			return false;
	}

	if (opened) {
		if (size > _maxRead)
			size = _maxRead;
	} else if (size > limit) {
		size = limit;
	}

	int toRead = _fileSize - _fileOffset;
	if (toRead >= size)
		toRead = size;

	if ((int)_file->read(_ring + _writePos, toRead) == toRead) {
		int next = _writePos + size;
		_fileOffset += size;
		if (next >= _ringSize)
			next -= _ringSize;

		_writePos = next;
		if (_restart)
			_hasData = 1;
	} else {
		_file->seek(_fileOffset);
	}

	return true;
}

int SmushDemoStream::streamRead(int size, const byte **data) {
	*data = nullptr;

	if (!_file)
		return -1;

	int wanted = size;
	if (size > _maxRead) {
		warning("File Streamer:  Invalid frame size %d", size);
		return -2;
	}

	if (!_released) {
		int next = _readPos + _held;
		bool wraps = next >= _ringSize;
		_readPos += _held;
		_held = 0;
		if (wraps)
			_readPos = next - _ringSize;

		_released = 1;
	}

	if (_restart) {
		streamRewind();
		_released = 0;
		return 0;
	}

	if (_alignSkip)
		wanted = _alignSkip + size;

	if (!streamHasSpace(wanted))
		return -1;

	int position = _readPos;
	_held = wanted;
	_released = 0;
	int overflow = _readPos + wanted - _ringSize;
	if (overflow > 0)
		memcpy(_ring + _ringSize, _ring, overflow);

	if (!_restart && _alignSkip) {
		position += _alignSkip;
		_alignSkip = 0;
	}

	*data = _ring + position;
	return 1;
}

int SmushDemoStream::streamRewind() {
	if (!_restart)
		return -1;

	_restart = 0;
	_hasData = 0;
	_held = 0;
	_readPos = _rewindPos;
	return 0;
}

SmushDemoAnim::SmushDemoAnim(SmushDemoStream *stream, SmushDemoClient *client) {
	_stream = stream;
	_client = client;
	_blocky8 = nullptr;
	_codec48 = nullptr;
	_refCount = 0;
	_bitmap = nullptr;
	_rect.x = 0;
	_rect.y = 0;
	_rect.width = 0;
	_rect.height = 0;
	_block = nullptr;
	_blockStatus = 0;
	_frame = 0;
	_frameCount = 1;
	_blockSize = 0;
	_tag = 0;
	_quit = 0;
	memset(_palette, 0, sizeof(_palette));
	memset(_paletteAccum, 0, sizeof(_paletteAccum));
	memset(_paletteDelta, 0, sizeof(_paletteDelta));
	_paletteState = 0;
	_noDisplay = 0;
	_displayPending = 1;
}

SmushDemoAnim::~SmushDemoAnim() {
	anmUnregisterCodecs();
	if (_bitmap) {
		delete[] _bitmap->pixels;
		delete _bitmap;
	}
}

int SmushDemoAnim::anmInitialize(int width, int height) {
	if (_refCount++ != 0)
		return 0;

	animatorInitialize();

	_bitmap = new SmushDemoBitmap();
	_bitmap->pixels = new byte[width * height]();
	_bitmap->width = width;
	_bitmap->height = height;
	_bitmap->size = width * height;
	return 0;
}

int SmushDemoAnim::anmTerminate() {
	if (--_refCount)
		return 0;

	if (_bitmap) {
		delete[] _bitmap->pixels;
		delete _bitmap;
		_bitmap = nullptr;
	}

	return 0;
}

void SmushDemoAnim::animatorInitialize() {
	_stream->streamReset();
	_frame = 0;
	_frameCount = 1;
	_tag = 0;
	_quit = 0;
	animatorResetView();
}

void SmushDemoAnim::animatorResetView() {
	_rect.x = 0;
	_rect.y = 0;
	_rect.width = _bitmap ? _bitmap->width : 320;
	_rect.height = _bitmap ? _bitmap->height : 200;
}

void SmushDemoAnim::animatorNewAnim(const Common::String &name, int offset) {
	_stream->streamOpen(name, offset + 8);
}

int SmushDemoAnim::animatorReadBlock(int size) {
	_blockStatus = _stream->streamRead(size, &_block);
	if (_blockStatus == 0)
		_frame = -1;

	return _blockStatus;
}

void SmushDemoAnim::animatorDecodeBlocks(int flags) {
	_client->preFrame(_frame, _frameCount);

	if (anmDecodeBody(_block, _blockSize, flags))
		_quit = 1;

	_client->postFrame(_frame, _frameCount);

	anmDisplay(flags);
}

void SmushDemoAnim::animatorHandleHeader(const byte *header) {
	anmSetPalette(header + 6, 1);

	_frameCount = READ_LE_UINT16(header + 2);
	if (header[0] > 1) {
		int fps = READ_LE_UINT16(header + 774);
		if (fps)
			_client->setFrameRate(fps);
	}

	animatorResetView();
}

int SmushDemoAnim::animatorRender(int flags) {
	if (_quit) {
		_quit = 0;
		return -1;
	}

	while (true) {
		switch (_tag) {
		case 0:
			animatorReadBlock(8);
			if (_blockStatus == -1)
				return 11;
			if (_blockStatus == -2)
				return -3;

			if (_block) {
				_tag = _block[0];
				_blockSize = READ_BE_UINT32(_block + 4);
			}

			break;
		case 'A':
			animatorReadBlock(_blockSize);
			if (_blockStatus == -1)
				return 11;
			if (_blockStatus == -2)
				return -3;

			_tag = 0;
			if (_block)
				animatorHandleHeader(_block);

			break;
		case 'F':
			animatorReadBlock(_blockSize);
			if (_blockStatus == -1)
				return 11;
			if (_blockStatus == -2)
				return -3;

			_tag = 0;
			if (!_block)
				break;

			_frame++;
			animatorDecodeBlocks(flags);
			if (_frame >= _frameCount - 1 && !_stream->streamIsReady())
				_tag = 'e';

			return _frame >= 10 ? 0 : 2;
		case 'e':
			_frame++;
			animatorDecodeBlocks(flags);

			if (_stream->streamIsReady())
				_tag = 0;

			return 1;
		default:
			warning("Read invalid frame header");
			return -3;
		}

		if (_tag > 0x77) {
			warning("Read invalid frame header");
			return -3;
		}
	}
}

void SmushDemoAnim::anmDisplay(int flags) {
	if (!_displayPending)
		return;

	_displayPending = 0;
	_client->frameCallback(flags, _paletteState >= 2, !_noDisplay, _paletteState);
	_paletteState = 0;
}

void SmushDemoAnim::anmSetPalette(const byte *palette, int mode) {
	if (palette != _palette)
		memcpy(_palette, palette, 768);

	_paletteState = (mode != 0) + 1;
}

int SmushDemoAnim::anmFrameObject(int flags, const byte *chunk) {
	int drawX = (int16)READ_LE_UINT16(chunk + 2);
	int drawY = (int16)READ_LE_UINT16(chunk + 4);
	int width = (int16)READ_LE_UINT16(chunk + 6);
	int height = (int16)READ_LE_UINT16(chunk + 8);

	if (chunk[0] == 48 && _codec48)
		return _codec48->decode(&_rect, drawX, drawY, chunk, _client);

	if (chunk[0] != 47 || !_blocky8)
		return -1;

	_blocky8->init(width, height);
	if (!_blocky8->decode(_bitmap->pixels, chunk + 14, flags))
		return 1;

	int clipX = drawX;
	int clipY = drawY;
	int sourceX = 0;
	int sourceY = 0;
	int clipWidth = width;
	int clipHeight = height;

	if (_rect.x > drawX) {
		clipX = _rect.x;
		sourceX = _rect.x - drawX;
		clipWidth = width + drawX - _rect.x;
	}

	if (clipWidth + clipX > _rect.width + _rect.x)
		clipWidth = _rect.x + _rect.width - clipX;

	if (_rect.y > drawY) {
		clipY = _rect.y;
		sourceY = _rect.y - drawY;
		clipHeight = drawY + height - _rect.y;
	}

	if (clipHeight + clipY > _rect.height + _rect.y)
		clipHeight = _rect.y + _rect.height - clipY;

	_client->setFrameBuffer(_bitmap->pixels, clipX, clipY, sourceX, sourceY, clipWidth, clipHeight);
	return 0;
}

int SmushDemoAnim::anmRegisterCodecs(int width, int height) {
	_codec48 = new Codec48Decoder();
	_codec48->init(width, height);
	_blocky8 = new Blocky8();
	_blocky8->init(width, height);
	return 0;
}

void SmushDemoAnim::anmUnregisterCodecs() {
	delete _codec48;
	_codec48 = nullptr;
	delete _blocky8;
	_blocky8 = nullptr;
}

int SmushDemoAnim::anmDecodeBody(const byte *block, int size, int flags) {
	_displayPending = 1;
	const byte *start = block;
	_noDisplay = 0;

	while (size > 4) {
		if (((block - start) & 1) != 0 && !*block) {
			block++;
			size--;
		}

		uint32 tag = READ_BE_UINT32(block);
		uint32 chunkSize = READ_BE_UINT32(block + 4);
		const byte *payload = block + 8;

		switch (tag) {
		case MKTAG('F', 'O', 'B', 'J'):
			if ((int16)anmFrameObject(flags, payload) > 0)
				_noDisplay = 1;

			break;
		case MKTAG('I', 'A', 'C', 'T'):
			_client->chunkCallback(payload, chunkSize, flags);
			break;
		case MKTAG('X', 'P', 'A', 'L'): {
			int mode = READ_LE_UINT16(payload + 2);
			if (mode == 0 || mode == 512) {
				if (mode == 512)
					memcpy(_palette, payload + 1540, 768);

				for (int i = 0; i < 768; i++) {
					_paletteAccum[i] = _palette[i] << 7;
					_paletteDelta[i] = (int16)READ_LE_UINT16(payload + 4 + i * 2);
				}
			} else {
				for (int i = 0; i < 768; i++) {
					_paletteAccum[i] += _paletteDelta[i];
					_palette[i] = _paletteAccum[i] >> 7;
				}

				_paletteState = 1;
			}

			break;
		}
		default: {
			bool printable = true;
			for (int i = 0; i < 4; i++) {
				byte c = (tag >> (i * 8)) & 0xFF;
				if (c < 0x30 || c > 0x5A)
					printable = false;
			}

			if (!printable) {
				warning("Bad IFF Header found %x at %x", tag, (uint32)(block - start));
				return -1;
			}

			break;
		}
		}

		block = payload + chunkSize;
		size -= 8 + chunkSize;
	}

	return 0;
}

} // End of namespace Grim
