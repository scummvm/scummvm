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
#define SMUSH_TAG_WAIT MKTAG('w', 'a', 'i', 't')

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

int SmushAnimator::animatorInitialize() {
	_flags = 0;
	_frame = 0;
	_blockSize = 0;
	_tag = SMUSH_TAG_NULL;
	_frameCount = 1;
	_fileMode = 0;
	_quit = 0;
	_pause = 0;
	_seeking = 0;
	_skipping = 0;
	_skipStart = -1;
	_skipBase = 0;
	_skipEnd = -1;
	return 0;
}

int SmushAnimator::animatorNewAnim(int flags, int skipLength, int skipUntil) {
	animatorInitialize();

	_frameCount = _decoder->getFrameCount();
	_seeking = 0;
	_flags = flags;
	_fileMode = 1;

	_skipStart = skipUntil;
	_skipBase = _frame;
	_skipEnd = skipLength;
	return 0;
}

int SmushAnimator::animatorQuery() {
	return _fileMode;
}

void SmushAnimator::animatorQuit() {
	_quit = 1;
}

int SmushAnimator::animatorPause(int mode) {
	if (mode == 2) {
		_pause = (_pause == 0);
		return _pause;
	}

	if (mode == 4) {
		if (_pause) {
			_pause = 3;
			return _pause;
		}
	} else if (mode != 5) {
		_pause = mode;
	}

	return _pause;
}

void SmushAnimator::readBlock(int size) {
	Common::SeekableReadStream *file = _decoder->getFile();

	if (!file || file->pos() >= file->size()) {
		_block = nullptr;
		_blockStatus = 0;
		_frame = -1;
		_fileMode = 0;
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

int SmushAnimator::checkSkip() {
	if (!_skipping) {
		if (_fileMode) {
			if (_skipEnd >= 0 && _frame > _skipEnd + _skipBase)
				return 1;
		} else if (_skipStart >= 0) {
			if (_frame < _skipStart)
				return 1;

			_skipStart = -1;
		}
	}

	return 0;
}

void SmushAnimator::decodeBlocks(int flags) {
	int allFlags = _flags | flags;
	int decodeFlags = ((allFlags >> 13) & 4) | (allFlags & 0x30);
	_decoder->decodeFrameBlock(_block, _blockSize, decodeFlags);
}

bool SmushAnimator::moreData() {
	Common::SeekableReadStream *file = _decoder->getFile();
	return file && file->pos() < file->size();
}

int SmushAnimator::animatorRender(int flags) {
	if (_quit) {
		_quit = 0;
		return -1;
	}

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
			if (_block) {
				_frame++;
				if (_skipping || !checkSkip()) {
					decodeBlocks(flags);

					if (_frame < _frameCount - 1 || _skipping) {
						if (_pause == 1)
							_tag = SMUSH_TAG_PAUSE;
						else if (_pause == 3)
							_pause = 1;
					} else if (moreData() || _seeking) {
						_tag = SMUSH_TAG_NULL;
					} else {
						_tag = SMUSH_TAG_END;
					}

					if (_skipping)
						return 2;

					// The disasm never renders the last frame
					if (_frame >= 10)
						return _frame < _frameCount - 1 ? 0 : 4;

					return 1;
				}

				if (_frame >= _frameCount - 1) {
					if (moreData() || _seeking)
						_tag = SMUSH_TAG_NULL;
					else
						_tag = SMUSH_TAG_END;
				}
			} else {
				return -2;
			}
			break;
		case SMUSH_TAG_NULL:
			if (_seeking) {
				_seeking = 0;
				_skipping = 1;
			}

			readBlock(8);
			if (_blockStatus == -2 || !_block)
				return -2;

			_tag = READ_BE_UINT32(_block);
			_blockSize = (READ_BE_UINT32(_block + 4) + 1) & ~1;
			break;
		case SMUSH_TAG_END:
			_frame++;
			checkSkip();

			if (moreData() || _seeking)
				_tag = SMUSH_TAG_NULL;

			return 4;
		case SMUSH_TAG_PAUSE:
			if (_pause != 1) {
				if (_pause == 3)
					_pause = 1;

				_tag = SMUSH_TAG_NULL;
			}

			return 3;
		case SMUSH_TAG_WAIT:
			if (moreData() || _seeking)
				_tag = SMUSH_TAG_NULL;

			return 5;
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
	_fileIsMemory = false;
	_fileOffset = 0;
	_fileSize = 0;
	_busy = 0;
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
	_ringSize = ringSize & 0xFFFFF800;
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

void SmushDemoStream::streamInitMemory() {
	_restart = 1;
	_hasData = 1;
	_busy = 0;
	_rewindPos = 0;
	_alignSkip = 0;
	_writePos = _ringSize - 1;
	_readPos = 0;
	_held = 0;
	_fileIsMemory = true;
	_fileOffset = 0;
	_fileSize = _ringSize - 1;
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

	_fileIsMemory = false;
	_busy = 0;
}

void SmushDemoStream::streamOpen(const Common::String &name, int offset) {
	_openName = name;
	_openPending = true;
	_openOffset = offset;
}

int SmushDemoStream::streamDoOpen() {
	if (!_openPending)
		return -1;

	if (!_openName.empty()) {
		Common::File *file = new Common::File();
		if (!file->open(Common::Path(_openName))) {
			delete file;
			_openPending = false;
			return -2;
		}

		delete _file;
		_file = file;
		_fileIsMemory = false;
		_fileSize = _file->size();
	}

	_file->seek(_openOffset & 0xFFFFF800);
	_alignSkip = _openOffset & 0x7FF;
	_fileOffset = _openOffset & 0xFFFFF800;
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
			size = (_readPos - _writePos - 2048) & 0xFFFFF800;
			if (size < 2048)
				return false;
		} else if (_ringSize - _writePos > 2048) {
			size = _ringSize - _writePos - 2048;
		} else {
			size = (_readPos - _writePos - 2048) & 0xFFFFF800;
			if (size < 2048)
				return false;
		}
	} else if (_readPos != 0) {
		size = _ringSize - _writePos;
	} else if (_ringSize - _writePos > 2048) {
		size = _ringSize - _writePos - 2048;
	} else {
		size = (_readPos - _writePos - 2048) & 0xFFFFF800;
		if (size < 2048)
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

	if (_busy || (!_file && !_fileIsMemory))
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

	if (size < 0) {
		if (_restart) {
			_held = 0;
			_readPos = _rewindPos;
		}

		_released = 0;
		return -1;
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
	_originX = 0;
	_originY = 0;
	_newAnim = 0;
	_options = 0;
	_seeking = 0;
	_skipping = 0;
	_quit = 0;
	_pause = 0;
	_dirty = 0;
	_skipUntil = 0;
	_skipBase = 0;
	_skipLength = 0;
	_flags = 0;
	_overlay = nullptr;
	_preload = nullptr;
	_overlaySize = 0;
	_preloadSize = 0;
	_preloadUsed = 0;
	_preloadCursor = 8;
	_preloadLast = 0;
	_preloadFirst = 0;
	memset(_updateFlags, 0, sizeof(_updateFlags));
	memset(_palette, 0, sizeof(_palette));
	memset(_paletteAccum, 0, sizeof(_paletteAccum));
	memset(_paletteDelta, 0, sizeof(_paletteDelta));
	_paletteState = 0;
	_paletteRefresh = 0;
	_noDisplay = 0;
	_displayPending = 1;
}

SmushDemoAnim::~SmushDemoAnim() {
	anmUnregisterCodecs();
	if (_bitmap) {
		delete[] _bitmap->pixels;
		delete _bitmap;
	}

	delete[] _overlay;
	delete[] _preload;
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
	anmClearUpdateFlags(0);
	_stream->streamReset();
	_options = 0;
	_frame = 0;
	_frameCount = 1;
	_tag = 0;
	_originX = 0;
	_originY = 0;
	_newAnim = 0;
	_dirty = 0;
	_quit = 0;
	_pause = 0;
	_seeking = 0;
	_skipping = 0;
	animatorResetView();
}

void SmushDemoAnim::animatorSetOrigin(int x, int y) {
	_originX = x;
	_originY = y;
}

void SmushDemoAnim::animatorResetView() {
	_rect.x = 0;
	_rect.y = 0;
	_rect.width = _bitmap ? _bitmap->width : 320;
	_rect.height = _bitmap ? _bitmap->height : 200;
	animatorSetOrigin(0, 0);
	anmClearUpdateFlags(0);
	_dirty = 0;
}

void SmushDemoAnim::animatorNewAnim(const Common::String &name, int options, int skipLength, int skipUntil, int seeking, int offset) {
	_seeking = seeking;
	_flags = options & 0xFFFF;
	_stream->streamOpen(name, offset + 8);
	_newAnim = 1;
	animatorSetSkip(skipLength, skipUntil);
}

void SmushDemoAnim::animatorSetSkip(int skipLength, int skipUntil) {
	_skipBase = _frame;
	_skipUntil = skipUntil;
	_skipLength = skipLength;
}

int SmushDemoAnim::animatorPause(int mode) {
	if (mode == 2) {
		_pause = (_pause == 0);
		return _pause;
	}

	if (mode == 4) {
		if (_pause) {
			_pause = 3;
			return 3;
		}
	} else if (mode != 5) {
		_pause = mode;
	}

	return _pause;
}

int SmushDemoAnim::animatorReadBlock(int size) {
	if (_skipping) {
		_block = anmPreloadGet(size);
		const byte *discard;
		_stream->streamRead(-1, &discard);
		_blockStatus = _block ? 1 : 0;
		if (!_block) {
			_skipping = 0;
			_skipLength = 0;
			_skipBase = 0;
			_tag = 'w';
		}
	} else {
		_blockStatus = _stream->streamRead(size, &_block);
		if (_blockStatus == 0) {
			_frame = -1;
			_newAnim = 0;
		}
	}

	return _blockStatus;
}

int SmushDemoAnim::animatorCheckSkip() {
	if (!_skipping) {
		if (_newAnim) {
			if (_skipLength >= 0 && _frame > _skipBase + _skipLength)
				return 1;
		} else if (_skipUntil >= 0) {
			if (_frame < _skipUntil)
				return 1;

			_skipUntil = -1;
		}
	}

	return 0;
}

int SmushDemoAnim::animatorDecodeBlocks(int x, int y, int flags) {
	if (_client)
		_client->preFrame(_frame, _frameCount);

	if (anmDecodeBody(x, y, _block, _blockSize, _options | flags))
		_quit = 1;

	if (_client)
		_client->postFrame(_frame, _frameCount);
	else if (_frame == _frameCount)
		_quit = 1;

	anmDisplay(_options | flags);
	if (_dirty)
		animatorResetView();

	return 0;
}

void SmushDemoAnim::animatorHandleHeader(const byte *header) {
	_options = _flags & 0xFFFF;
	if (!(_flags & 0x400))
		anmSetPalette(header + 6, 1);

	_frameCount = READ_LE_UINT16(header + 2);
	if (header[0] > 1) {
		int fps = READ_LE_UINT16(header + 774);
		if (fps && !(_options & 8))
			_client->setFrameRate(fps);
	}

	if (!(_options & 0x40))
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
			if (_seeking) {
				_seeking = 0;
				_skipping = 1;
			}

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
			if (_skipping || !animatorCheckSkip()) {
				animatorDecodeBlocks(_originX, _originY, flags);
				if (_frame < _frameCount - 1 || _skipping) {
					if (_pause == 1)
						_tag = 'p';
					else if (_pause == 3)
						_pause = 1;
				} else if (_stream->streamIsReady() || _seeking) {
					_tag = 0;
				} else {
					_tag = 'e';
				}

				if (_skipping)
					return 1;

				return _frame >= 10 ? 0 : 2;
			}

			if (_frame >= _frameCount - 1) {
				if (_stream->streamIsReady() || _seeking)
					_tag = 0;
				else
					_tag = 'e';
			}

			break;
		case 'e':
			_frame++;
			if (!animatorCheckSkip())
				animatorDecodeBlocks(_originX, _originY, flags);

			if (_stream->streamIsReady() || _seeking)
				_tag = 0;

			return 1;
		case 'p':
			animatorDecodeBlocks(_originX, _originY, flags);
			if (_pause == 1)
				return 1;

			if (_pause == 3)
				_pause = 1;

			_tag = 0;
			return 1;
		case 'w':
			if (_stream->streamIsReady() || _seeking)
				_tag = 0;

			return 1;
		default:
			warning("Read invalid frame header -- CD may be dirty or damaged.");
			return -3;
		}

		if (_tag > 0x77) {
			warning("Read invalid frame header -- CD may be dirty or damaged.");
			return -3;
		}
	}
}

int SmushDemoAnim::anmBuffersInitialize(int overlaySize, int preloadSize) {
	_overlaySize = overlaySize;
	_preloadSize = preloadSize;
	anmClearUpdateFlags(0);

	if (overlaySize)
		_overlay = new byte[overlaySize]();

	if (preloadSize)
		_preload = new byte[preloadSize]();

	return 0;
}

void SmushDemoAnim::anmBuffersTerminate() {
	if (_overlaySize) {
		delete[] _overlay;
		_overlay = nullptr;
	}

	if (_preloadSize) {
		delete[] _preload;
		_preload = nullptr;
	}
}

void SmushDemoAnim::anmClearUpdateFlags(int index) {
	if (index > 0) {
		int bit = index - 1;
		if ((bit & 0xF8) < 1024)
			_updateFlags[bit >> 3] &= ~(128 >> (bit & 7));
	} else {
		memset(_updateFlags, 0, sizeof(_updateFlags));
	}
}

bool SmushDemoAnim::anmTestUpdateFlag(int index) {
	int bit = index - 1;
	return (bit & 0xF8) < 1024 && (_updateFlags[bit >> 3] & (128 >> (bit & 7))) != 0;
}

void SmushDemoAnim::anmPreloadPut(const byte *chunk, int size) {
	int part = READ_LE_UINT16(chunk + 2);
	int first = READ_LE_UINT16(chunk);
	int used;

	if (part) {
		used = _preloadUsed;
	} else {
		used = 0;
		_preloadCursor = 8;
		_preloadUsed = 0;
		_preloadLast = -1;
		_preloadFirst = first;
	}

	if (part == _preloadLast + 1) {
		if (size + used < _preloadSize) {
			memcpy(_preload + used, chunk + 10, size - 10);
			_preloadUsed += size - 10;
		}

		_preloadLast = part;
	}
}

const byte *SmushDemoAnim::anmPreloadGet(int size) {
	int next = size + _preloadCursor;
	if (next > _preloadUsed) {
		_preloadCursor = 8;
		return nullptr;
	}

	_preloadCursor += size;
	return _preload + next - size;
}

void SmushDemoAnim::anmDisplay(int flags) {
	if (!_displayPending)
		return;

	_displayPending = 0;
	bool display = !_noDisplay && (flags & 0x8000) == 0;
	_client->frameCallback(flags, _paletteState >= 2, display, _paletteState);
	if (_paletteState) {
		_paletteRefresh = 1;
		_paletteState = 0;
	}
}

void SmushDemoAnim::anmSetPalette(const byte *palette, int mode) {
	if (palette != _palette)
		memcpy(_palette, palette, 768);

	_paletteState = (mode != 0) + 1;
}

void SmushDemoAnim::anmStoreObject(byte *destination, const byte *chunk) {
	int size = READ_BE_UINT32(chunk + 4) + 8;
	if (size < _overlaySize)
		memcpy(destination, chunk, size);
}

void SmushDemoAnim::anmDrawObject(int x, int y, const byte *object, int flags) {
	anmFrameObject(x, y, flags, object + 8, READ_BE_UINT32(object + 4));
}

int SmushDemoAnim::anmFrameObject(int x, int y, int flags, const byte *chunk, int size) {
	int drawX;
	int drawY;

	if (flags & 1) {
		drawX = x;
		drawY = y;
	} else {
		drawX = (int16)(READ_LE_UINT16(chunk + 2) + x);
		drawY = (int16)(READ_LE_UINT16(chunk + 4) + y);
	}

	int width = (int16)READ_LE_UINT16(chunk + 6);
	int height = (int16)READ_LE_UINT16(chunk + 8);
	if (flags & 2) {
		drawX -= width >> 1;
		drawY -= height >> 1;
	}

	if (chunk[0] == 48 && _codec48)
		return _codec48->decode(_bitmap, &_rect, drawX, drawY, chunk, flags, _client);

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

int SmushDemoAnim::anmDecodeBody(int x, int y, const byte *block, int size, int flags) {
	_displayPending = 1;
	int handled = 0;
	bool skipNext = false;
	bool storePending = false;
	uint32 previousTag = 0;
	const byte *lastObject = nullptr;
	const byte *start = block;
	_noDisplay = 0;

	while (size > 4) {
		handled = 0;

		if (((block - start) & 1) != 0 && !*block) {
			block++;
			size--;
		}

		if (storePending && _overlay) {
			if (READ_BE_UINT32(block) != MKTAG('I', 'A', 'C', 'T')) {
				anmStoreObject(_overlay, block);
				anmDrawObject(x, y, _overlay, flags);
				storePending = false;
				handled = 1;
			}
		}

		uint32 tag = READ_BE_UINT32(block);
		uint32 chunkSize = READ_BE_UINT32(block + 4);
		const byte *payload = block + 8;

		if (skipNext) {
			skipNext = false;
		} else {
			switch (tag) {
			case MKTAG('F', 'T', 'C', 'H'):
				if (_overlay) {
					int offsetX;
					int offsetY;
					if (chunkSize == 12) {
						offsetX = READ_BE_UINT16(payload + 6);
						offsetY = READ_BE_UINT16(payload + 10);
					} else {
						offsetX = (int16)READ_LE_UINT16(payload + 2);
						offsetY = (int16)READ_LE_UINT16(payload + 4);
					}

					anmDrawObject(x + offsetX, y + offsetY, _overlay, flags);
				}

				break;
			case MKTAG('F', 'O', 'B', 'J'):
				lastObject = payload;
				if (handled)
					break;

				if (previousTag == MKTAG('S', 'K', 'I', 'P')) {
					if ((uint16)anmFrameObject(x, y, flags, payload, chunkSize))
						_noDisplay = 1;
				} else if ((int16)anmFrameObject(x, y, flags, payload, chunkSize) > 0) {
					_noDisplay = 1;
				}

				break;
			case MKTAG('G', 'O', 'S', 'T'): {
				int mode = 0;
				if (READ_LE_UINT16(payload) == 0)
					mode = 0x2000;
				else if (READ_LE_UINT16(payload) == 1)
					mode = 0x4000;
				else if (READ_LE_UINT16(payload) == 2)
					mode = 0x6000;

				int offsetX = (int16)READ_LE_UINT16(payload + 2);
				int offsetY = (int16)READ_LE_UINT16(payload + 4);
				anmFrameObject(x + offsetX, y + offsetY, flags | mode, lastObject, chunkSize);
				break;
			}
			case MKTAG('I', 'A', 'C', 'T'):
				_client->chunkCallback(payload, chunkSize, flags);
				break;
			case MKTAG('N', 'P', 'A', 'L'):
				if (animatorPause(5) != 1)
					anmSetPalette(payload, 1);

				break;
			case MKTAG('L', 'O', 'A', 'D'):
				anmPreloadPut(payload, chunkSize);
				break;
			case MKTAG('S', 'A', 'U', 'D'):
			case MKTAG('P', 'S', 'A', 'D'):
				break;
			case MKTAG('S', 'T', 'O', 'R'):
				storePending = true;
				break;
			case MKTAG('S', 'K', 'I', 'P'): {
				int second = READ_LE_UINT16(payload + 2);
				int first = READ_LE_UINT16(payload);
				if (second)
					skipNext = anmTestUpdateFlag(first) != anmTestUpdateFlag(second);
				else
					skipNext = anmTestUpdateFlag(first) != 0;

				break;
			}
			case MKTAG('V', 'M', 'S', 'K'):
				if (animatorPause(5) != 1)
					_noDisplay = 1;

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
				} else if (animatorPause(5) != 1) {
					for (int i = 0; i < 768; i++) {
						_paletteAccum[i] += _paletteDelta[i];
						_palette[i] = _paletteAccum[i] >> 7;
					}

					_paletteRefresh = 1;
					_paletteState = 1;
				}

				break;
			}
			case MKTAG('T', 'R', 'E', 'S'):
			case MKTAG('T', 'E', 'X', 'T'):
				if (flags & 0x200) {
					const byte pseudo[2] = { 9, 0 };
					_client->chunkCallback(pseudo, chunkSize, flags);
				} else if (tag == MKTAG('T', 'R', 'E', 'S')) {
					_client->setAnnotation((int16)READ_LE_UINT16(payload + 16));
				}

				break;
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
		}

		previousTag = tag;
		block = payload + chunkSize;
		size -= 8 + chunkSize;
	}

	return 0;
}

} // End of namespace Grim
