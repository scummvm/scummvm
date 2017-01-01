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

#include "common/file.h"

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/utils.h"

namespace Lab {

Anim::Anim(LabEngine *vm) : _vm(vm) {
	_lastBlockHeader = 0;
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
	_scrollScreenBuffer = nullptr;
	_waitForEffect = false;
	_stopPlayingEnd = false;
	_sampleSpeed = 0;
	_doBlack = false;

	for (int i = 0; i < 3 * 256; i++)
		_diffPalette[i] = 0;

	_outputBuffer = nullptr; // output to screen
}

Anim::~Anim() {
	delete[] _vm->_anim->_scrollScreenBuffer;
	_vm->_anim->_scrollScreenBuffer = nullptr;
}

void Anim::setOutputBuffer(byte *memoryBuffer) {
	_outputBuffer = memoryBuffer;
}

uint16 Anim::getDIFFHeight() {
	return _headerdata._height;
}

void Anim::diffNextFrame(bool onlyDiffData) {
	if (_lastBlockHeader == 65535)
		// Already done.
		return;

	bool drawOnScreen = false;
	byte *startOfBuf = _outputBuffer;
	int bufPitch = _vm->_graphics->_screenWidth;

	if (!startOfBuf) {
		startOfBuf = _vm->_graphics->getCurrentDrawingBuffer();
		drawOnScreen = true;
	}
	byte *endOfBuf = startOfBuf + (int)_headerdata._width * _headerdata._height;

	int curBit = 0;

	while (1) {
		byte *buf = startOfBuf + 0x10000 * curBit;

		if (buf >= endOfBuf) {
			if (!onlyDiffData) {
				if (_headerdata._fps) {
					uint32 targetMillis = _vm->_system->getMillis() + _delayMicros;
					while (_vm->_system->getMillis() < targetMillis)
						_vm->_system->delayMillis(10);
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

			if (drawOnScreen)
				_vm->_graphics->screenUpdate();

			// done with the next frame.
			return;
		}

		_vm->updateEvents();
		_lastBlockHeader = _diffFile->readUint32LE();
		_size = _diffFile->readUint32LE();

		uint32 curPos = 0;

		switch (_lastBlockHeader) {
		case 8:
			_diffFile->read(_diffPalette, _size);
			_isPal = true;
			break;

		case 10:
			if (onlyDiffData) {
				if (curBit > 0)
					error("diffNextFrame: attempt to read screen to non-zero plane (%d)", curBit);
				delete[] _scrollScreenBuffer;
				_scrollScreenBuffer = new byte[_headerdata._width * _headerdata._height];
				_diffFile->read(_scrollScreenBuffer, _size);
			} else {
				_diffFile->read(buf, _size);
			}
			curBit++;
			break;

		case 11:
			curPos = _diffFile->pos();
			_diffFile->skip(4);
			_vm->_utils->runLengthDecode(buf, _diffFile);
			curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 12:
			curPos = _diffFile->pos();
			_diffFile->skip(4);
			_vm->_utils->verticalRunLengthDecode(buf, _diffFile, bufPitch);
			curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 20:
			curPos = _diffFile->pos();
			_vm->_utils->unDiff(buf, buf, _diffFile, bufPitch, false);
			curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 21:
			curPos = _diffFile->pos();
			_vm->_utils->unDiff(buf, buf, _diffFile, bufPitch, true);
			curBit++;
			_diffFile->seek(curPos + _size, SEEK_SET);
			break;

		case 25:
		case 26:
			curBit++;
			break;

		case 30:
		case 31:
			if (_waitForEffect) {
				while (_vm->_music->isSoundEffectActive()) {
					_vm->updateEvents();
					_vm->waitTOF();
				}

				_waitForEffect = false;
			}

			_size -= 8;

			_diffFile->skip(4);
			_sampleSpeed = _diffFile->readUint16LE();
			_diffFile->skip(2);

			// Sound effects embedded in animations are started here. These are
			// usually animation-specific, like door opening sounds, and are not looped.
			// The engine should wait for all such sounds to end.
			_waitForEffect = true;
			_vm->_music->playSoundEffect(_sampleSpeed, _size, false, _diffFile);
			break;

		case 65535:
			if ((_frameNum == 1) || _playOnce || _stopPlayingEnd) {
				bool didTOF = false;

				if (_waitForEffect) {
					while (_vm->_music->isSoundEffectActive()) {
						_vm->updateEvents();
						_vm->waitTOF();

						if (drawOnScreen)
							didTOF = true;
					}

					_waitForEffect = false;
				}

				_isPlaying = false;

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
		_vm->updateEvents();
		diffNextFrame();
	}
}

void Anim::readDiff(Common::File *diffFile, bool playOnce, bool onlyDiffData) {
	_playOnce = playOnce;
	_delayMicros = 0;
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

	if (!_diffFile)
		return;

	uint32 magicBytes = _diffFile->readUint32LE();
	if (magicBytes != 1219009121) {
		_isPlaying = false;
		return;
	}

	uint32 signature3 = _diffFile->readUint32LE();
	_size = _diffFile->readUint32LE();

	if (signature3 != 0)
		return;

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
	_vm->_system->delayMillis(150);

	if (_headerdata._fps == 1)
		_headerdata._fps = 0;

	// 4 + 2 bytes, buffer size and machine, unused
	_diffFile->skip(6);
	_headerdata._flags = _diffFile->readUint32LE();

	_diffFile->skip(_size - 18);

	_continuous = CONTINUOUS & _headerdata._flags;
	_vm->_utils->setBytesPerRow(_headerdata._width);

	delete[] _scrollScreenBuffer;
	_scrollScreenBuffer = nullptr;

	if (_headerdata._fps)
		_delayMicros = 1000 / _headerdata._fps;

	_lastBlockHeader = signature3;
	if (_playOnce) {
		while (_lastBlockHeader != 65535)
			diffNextFrame(onlyDiffData);
	} else
		diffNextFrame(onlyDiffData);
}

} // End of namespace Lab
