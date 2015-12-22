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
	_delayMicros = 0;
	_continuous = false;
	_isPlaying = false;
	_isAnim = false;
	_isPal = false;
	_noPalChange = false;
	_donePal = false;
	_frameNum = 0;
	_playOnce = false;
	_diffFile = nullptr;
	_diffFileStart = 0;
	_size = 0;
	_rawDiffBM._bytesPerRow = 0;
	_rawDiffBM._drawOnScreen = false;
	for (int i = 0; i < 16; i++)
		_rawDiffBM._planes[i] = nullptr;
	_waitForEffect = false;
	_stopPlayingEnd = false;
	_sampleSpeed = 0;
	_doBlack = false;
	_diffWidth = 0;
	_diffHeight = 0;

	for (int i = 0; i < 3 * 256; i++)
		_diffPalette[i] = 0;
}

void Anim::diffNextFrame(bool onlyDiffData) {
	if (_header == 65535)
		// Already done.
		return;

	BitMap *disp = _vm->_graphics->_dispBitMap;
	if (disp->_drawOnScreen)
		disp->_planes[0] = _vm->_graphics->getCurrentDrawingBuffer();

	disp->_planes[1] = disp->_planes[0] + 0x10000;
	disp->_planes[2] = disp->_planes[1] + 0x10000;
	disp->_planes[3] = disp->_planes[2] + 0x10000;
	disp->_planes[4] = disp->_planes[3] + 0x10000;

	_vm->_event->mouseHide();

	while (1) {
		if (_curBit >= _numChunks) {
			_vm->_event->mouseShow();

			if (!onlyDiffData) {
				if (_headerdata._fps) {
					uint32 targetMillis = g_system->getMillis() + _delayMicros;
					while (g_system->getMillis() < targetMillis)
						g_system->delayMillis(10);
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

			if ((_frameNum == 1) && (_continuous || !_playOnce))
				_diffFileStart = _diffFile->pos();

			_isAnim = (_frameNum >= 3) && (!_playOnce);
			_curBit = 0;

			if (disp->_drawOnScreen)
				_vm->_graphics->screenUpdate();

			// done with the next frame.
			return;
		}

		_vm->updateMusicAndEvents();
		_header = _diffFile->readUint32LE();
		_size = _diffFile->readUint32LE();

		uint32 curPos = 0;

		switch (_header) {
		case 8:
			_diffFile->read(_diffPalette, _size);
			_isPal = true;
			break;

		case 10:
			if (onlyDiffData)
				warning("Boom");
			_diffFile->read(disp->_planes[_curBit], _size);
			_curBit++;
			break;

		case 11:
			curPos = _diffFile->pos();
			_diffFile->skip(4);
			_vm->_utils->runLengthDecode(disp->_planes[_curBit], _diffFile);
			_curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 12:
			curPos = _diffFile->pos();
			_diffFile->skip(4);
			_vm->_utils->verticalRunLengthDecode(disp->_planes[_curBit], _diffFile, disp->_bytesPerRow);
			_curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 20:
			curPos = _diffFile->pos();
			_vm->_utils->unDiff(disp->_planes[_curBit], disp->_planes[_curBit], _diffFile, disp->_bytesPerRow, false);
			_curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 21:
			curPos = _diffFile->pos();
			_vm->_utils->unDiff(disp->_planes[_curBit], disp->_planes[_curBit], _diffFile, disp->_bytesPerRow, true);
			_curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 25:
			_curBit++;
			break;

		case 26:
			_curBit++;
			break;

		case 30:
		case 31:
			if (_waitForEffect) {
				while (_vm->_music->isSoundEffectActive()) {
					_vm->updateMusicAndEvents();
					_vm->waitTOF();
				}
			}

			_size -= 8;

			_diffFile->skip(4);
			_sampleSpeed = _diffFile->readUint16LE();
			_diffFile->skip(2);

			_vm->_music->playSoundEffect(_sampleSpeed, _size, _diffFile);
			break;

		case 65535:
			if ((_frameNum == 1) || _playOnce || _stopPlayingEnd) {
				bool didTOF = false;

				if (_waitForEffect) {
					while (_vm->_music->isSoundEffectActive()) {
						_vm->updateMusicAndEvents();
						_vm->waitTOF();

						if (disp->_drawOnScreen)
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
			_diffFile->seek(_diffFileStart, SEEK_SET);
			break;

		default:
			_diffFile->skip(_size);
			break;
		}
	}
}

void Anim::stopDiff() {
	if (_isPlaying && _isAnim)
		_vm->_graphics->blackScreen();
}

void Anim::stopDiffEnd() {
	if (!_isPlaying)
		return;

	_stopPlayingEnd = true;
	while (_isPlaying) {
		_vm->updateMusicAndEvents();
		diffNextFrame();
	}
}

void Anim::readDiff(Common::File *diffFile, bool playOnce, bool onlyDiffData) {
	_playOnce = playOnce;
	_delayMicros = 0;
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

	_diffFile = diffFile;

	_continuous = false;
	uint32 signature = _diffFile->readUint32BE();
	_header = _diffFile->readUint32LE();

	if ((signature != MKTAG('D', 'I', 'F', 'F')) || (_header != 1219009121L)) {
		_isPlaying = false;
		return;
	}

	_header = _diffFile->readUint32LE();
	_size = _diffFile->readUint32LE();

	if (_header == 0) {
		// sizeof(headerdata) != 18, but the padding might be at the end
		// 2 bytes, version, unused.
		_diffFile->skip(2);
		_headerdata._width = _diffFile->readUint16LE();
		_headerdata._height = _diffFile->readUint16LE();
		// 1 byte, depth, unused
		_diffFile->skip(1);
		_headerdata._fps = _diffFile->readByte();

		// HACK: The original game defines a 1 second delay when changing screens, which is
		// very annoying. We first removed the delay, but it looked wrong when changing screens
		// as it was possible to see that something was displayed, without being able to tell
		// what it was. A shorter delay (150ms) makes it acceptable during gameplay and
		// readable. The big question is: do we need that message?
		g_system->delayMillis(150);

		if (_headerdata._fps == 1)
			_headerdata._fps = 0;

		// 4 + 2 bytes, buffer size and machine, unused
		_diffFile->skip(6);
		_headerdata._flags = _diffFile->readUint32LE();

		_diffFile->skip(_size - 18);

		_continuous = CONTINUOUS & _headerdata._flags;
		_diffWidth = _headerdata._width;
		_diffHeight = _headerdata._height;
		_vm->_utils->setBytesPerRow(_diffWidth);

		_numChunks = (((int32)_diffWidth) * _diffHeight) / 0x10000;

		if ((uint32)(_numChunks * 0x10000) < (uint32)(((int32)_diffWidth) * _diffHeight))
			_numChunks++;
	} else
		return;

	for (_header = 0; _header < 8; _header++)
		_rawDiffBM._planes[_header] = nullptr;

	if (_headerdata._fps)
		_delayMicros = 1000 / _headerdata._fps;

	if (_playOnce) {
		while (_header != 65535)
			diffNextFrame(onlyDiffData);
	} else
		diffNextFrame(onlyDiffData);
}

} // End of namespace Lab
