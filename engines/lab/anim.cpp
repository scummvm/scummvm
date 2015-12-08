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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/utils.h"

namespace Lab {

Anim::Anim(LabEngine *vm) : _vm(vm) {
	_header = 0;
	_curBit = 0;
	_numChunks = 1;
	_headerdata._width = 0;
	_headerdata._height = 0;
	_headerdata._fps = 0;
	_headerdata._flags = 0;
	_waitSec = 0;
	_waitMicros = 0;
	_delayMicros = 0;
	_continuous = false;
	_isPlaying = false;
	_isAnim = false;
	_isPal = false;
	_noPalChange = false;
	_donePal = false;
	_frameNum = 0;
	_playOnce = false;
	_buffer = nullptr;
	_diffFile = nullptr;
	_size = 0;
	_rawDiffBM._bytesPerRow = 0;
	_rawDiffBM._flags = 0;
	for (int i = 0; i < 16; i++)
		_rawDiffBM._planes[i] = nullptr;
	_rawDiffBM._rows = 0;
	_waitForEffect = false;
	_stopPlayingEnd = false;
	_sampleSpeed = 0;
	_doBlack = false;
	_diffWidth = 0;
	_diffHeight = 0;
	DrawBitMap = _vm->_graphics->_dispBitMap;

	for (int i = 0; i < 3 * 256; i++)
		_diffPalette[i] = 0;
}

void Anim::readBlock(void *Buffer, uint32 Size, byte **File) {
	memcpy(Buffer, *File, (size_t)Size);
	(*File) += Size;
}

void Anim::diffNextFrame(bool onlyDiffData) {
	if (_header == 65535)
		// Already done.
		return;

	if (_vm->_graphics->_dispBitMap->_flags & BITMAPF_VIDEO) {
		_vm->_graphics->_dispBitMap->_planes[0] = _vm->_graphics->getCurrentDrawingBuffer();
		_vm->_graphics->_dispBitMap->_planes[1] = _vm->_graphics->_dispBitMap->_planes[0] + 0x10000;
		_vm->_graphics->_dispBitMap->_planes[2] = _vm->_graphics->_dispBitMap->_planes[1] + 0x10000;
		_vm->_graphics->_dispBitMap->_planes[3] = _vm->_graphics->_dispBitMap->_planes[2] + 0x10000;
		_vm->_graphics->_dispBitMap->_planes[4] = _vm->_graphics->_dispBitMap->_planes[3] + 0x10000;
	}

	_vm->_event->mouseHide();

	while (1) {
		if (_curBit >= _numChunks) {
			_vm->_event->mouseShow();

			if (!onlyDiffData) {
				if (_headerdata._fps) {
					_vm->waitForTime(_waitSec, _waitMicros);
					_vm->addCurTime(0L, _delayMicros, &_waitSec, &_waitMicros);
				}

				if (_isPal && !_noPalChange) {
					_vm->_graphics->setPalette(_diffPalette, 256);
					_isPal = false;
				}

				_donePal = true;
			}

			if (_isPal && !_noPalChange && !onlyDiffData && !_donePal) {
				_vm->_graphics->setPalette(_diffPalette, 256);
				_isPal = false;
			}

			_donePal = false;

			_frameNum++;

			if ((_frameNum == 1) && (_continuous || (!_playOnce)))
				_buffer = _diffFile;

			_isAnim = (_frameNum >= 3) && (!_playOnce);
			_curBit = 0;

			if (_vm->_graphics->_dispBitMap->_flags & BITMAPF_VIDEO)
				_vm->_graphics->screenUpdate();

			// done with the next frame.
			return;
		}

		_vm->_music->updateMusic();
		_header = READ_LE_UINT32(_diffFile);
		_diffFile += 4;

		_size = READ_LE_UINT32(_diffFile);
		_diffFile += 4;

		switch (_header) {
		case 8L:
			readBlock(_diffPalette, _size, &_diffFile);
			_isPal = true;
			break;

		case 10L:
			_rawDiffBM._planes[_curBit] = _diffFile;

			if (onlyDiffData)
				_diffFile += _size;
			else {
				readBlock(DrawBitMap->_planes[_curBit], _size, &_diffFile);
			}

			_curBit++;
			break;

		case 11L:
			_diffFile += 4;
			_vm->_utils->runLengthDecode(DrawBitMap->_planes[_curBit], _diffFile);
			_curBit++;
			_diffFile += _size - 4;
			break;

		case 12L:
			_diffFile += 4;
			_vm->_utils->VRunLengthDecode(DrawBitMap->_planes[_curBit], _diffFile, DrawBitMap->_bytesPerRow);
			_curBit++;
			_diffFile += _size - 4;
			break;

		case 20L:
			_vm->_utils->unDiff(DrawBitMap->_planes[_curBit], _vm->_graphics->_dispBitMap->_planes[_curBit], _diffFile, DrawBitMap->_bytesPerRow, false);
			_curBit++;
			_diffFile += _size;
			break;

		case 21L:
			_vm->_utils->unDiff(DrawBitMap->_planes[_curBit], _vm->_graphics->_dispBitMap->_planes[_curBit], _diffFile, DrawBitMap->_bytesPerRow, true);
			_curBit++;
			_diffFile += _size;
			break;

		case 25L:
			_curBit++;
			break;

		case 26L:
			_curBit++;
			break;

		case 30L:
		case 31L:
			if (_waitForEffect) {
				while (_vm->_music->isSoundEffectActive()) {
					_vm->_music->updateMusic();
					_vm->waitTOF();
				}
			}

			_size -= 8L;


			_diffFile += 4;
			_sampleSpeed = READ_LE_UINT16(_diffFile);
			_diffFile += 4;

			_vm->_music->playSoundEffect(_sampleSpeed, _size, _diffFile);
			_diffFile += _size;
			break;
		case 65535L:
			if ((_frameNum == 1) || _playOnce || _stopPlayingEnd) {
				bool didTOF = false;

				if (_waitForEffect) {
					while (_vm->_music->isSoundEffectActive()) {
						_vm->_music->updateMusic();
						_vm->waitTOF();

						if (_vm->_graphics->_dispBitMap->_flags & BITMAPF_VIDEO)
							didTOF = true;
					}
				}

				_isPlaying = false;
				_vm->_event->mouseShow();

				if (!didTOF)
					_vm->_graphics->screenUpdate();

				return;
			}

			// Random frame number so it never gets back to 2
			_frameNum = 4;
			_diffFile = _buffer;
			break;

		default:
			_diffFile += _size;
			break;
		}
	}
}

/**
 * A separate task launched by readDiff.  Plays the DIFF.
 */
void Anim::playDiff(byte *buffer, bool onlyDiffData) {
	_waitSec = 0L;
	_waitMicros = 0L;
	_delayMicros = 0L;
	_header = 0;
	_curBit = 0;
	_frameNum = 0;
	_numChunks = 1;
	_donePal = false;
	_stopPlayingEnd = false;
	_isPlaying = true;

	if (_doBlack) {
		_doBlack = false;
		_vm->_graphics->blackScreen();
	}

	_diffFile = buffer;

	_continuous = false;
	uint32 signature = READ_BE_UINT32(_diffFile);
	_diffFile += 4;

	_header = READ_LE_UINT32(_diffFile);
	_diffFile += 4;

	if ((signature != MKTAG('D', 'I', 'F', 'F')) || (_header != 1219009121L)) {
		_isPlaying = false;
		return;
	}

	_header = READ_LE_UINT32(_diffFile);
	_diffFile += 4;

	_size = READ_LE_UINT32(_diffFile);
	_diffFile += 4;

	if (_header == 0) {
		// sizeof(headerdata) != 18, but the padding might be at the end
		_headerdata._version = READ_LE_UINT16(_diffFile);
		_diffFile += 2;
		_headerdata._width = READ_LE_UINT16(_diffFile);
		_diffFile += 2;
		_headerdata._height = READ_LE_UINT16(_diffFile);
		_diffFile += 2;
		_headerdata._depth = _diffFile[0];
		_diffFile++;
		_headerdata._fps = _diffFile[0];
		_diffFile++;
		_headerdata._bufferSize = READ_LE_UINT32(_diffFile);
		_diffFile += 4;
		_headerdata._machine = READ_LE_UINT16(_diffFile);
		_diffFile += 2;
		_headerdata._flags = READ_LE_UINT32(_diffFile);
		_diffFile += 4;

		_diffFile += _size - 18;

		_continuous = CONTINUOUS & _headerdata._flags;
		_diffWidth = _headerdata._width;
		_diffHeight = _headerdata._height;
		_vm->_utils->setBytesPerRow(_diffWidth);

		_numChunks = (((int32) _diffWidth) * _diffHeight) / 0x10000;

		if ((uint32)(_numChunks * 0x10000) < (uint32)(((int32) _diffWidth) * _diffHeight))
			_numChunks++;
	} else {
		return;
	}

	for (_header = 0; _header < 8; _header++)
		_rawDiffBM._planes[_header] = NULL;

	if (_headerdata._fps)
		_delayMicros = ONESECOND / _headerdata._fps;

	if (_playOnce) {
		while (_header != 65535)
			diffNextFrame(onlyDiffData);
	} else
		diffNextFrame(onlyDiffData);
}

/**
 * Stops an animation from running.
 */
void Anim::stopDiff() {
	if (_isPlaying && _isAnim)
		_vm->_graphics->blackScreen();
}

/**
 * Stops an animation from running.
 */
void Anim::stopDiffEnd() {
	if (_isPlaying) {
		_stopPlayingEnd = true;
		while (_isPlaying) {
			_vm->_music->updateMusic();
			diffNextFrame();
		}
	}
}

/**
 * Reads in a DIFF file.
 */
bool Anim::readDiff(byte *buffer, bool playOnce, bool onlyDiffData) {
	_playOnce = playOnce;
	playDiff(buffer, onlyDiffData);
	return true;
}

} // End of namespace Lab
