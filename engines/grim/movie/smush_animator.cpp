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
#include "common/stream.h"

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

void SmushAnimator::decodeBlocks() {
	_decoder->decodeFrameBlock(_block, _blockSize);
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
					decodeBlocks();

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

} // End of namespace Grim
