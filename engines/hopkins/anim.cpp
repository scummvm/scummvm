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

#include "hopkins/anim.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"

namespace Hopkins {

AnimationManager::AnimationManager(HopkinsEngine *vm) {
	_vm = vm;
	_clearAnimationFl = false;
	for (int i = 0; i < 8; ++i)
		Common::fill((byte *)&Bank[i], (byte *)&Bank[i] + sizeof(BankItem), 0);
	for (int i = 0; i < 35; ++i)
		Common::fill((byte *)&_animBqe[i], (byte *)&_animBqe[i] + sizeof(BqeAnimItem), 0);
}

void AnimationManager::clearAll() {
	initAnimBqe();
}

/**
 * Play Animation
 * @param filename		Filename of animation to play
 * @param rate1			Delay amount before starting animation
 * @param rate2			Delay amount between animation frames
 * @param rate3			Delay amount after animation finishes
 */
void AnimationManager::playAnim(const Common::Path &hiresName, const Common::Path &lowresName, uint32 rate1, uint32 rate2, uint32 rate3, bool skipSeqFl) {
	Common::File f;

	if (_vm->shouldQuit())
		return;

	_vm->_events->mouseOff();

	byte *screenP = _vm->_graphicsMan->_backBuffer;

	if (!f.open(hiresName)) {
		if (!f.open(lowresName))
			error("Files not found: %s - %s", hiresName.toString().c_str(), lowresName.toString().c_str());
	}

	f.skip(6);
	f.read(_vm->_graphicsMan->_palette, 800);
	f.skip(4);
	size_t nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	if (_clearAnimationFl)
		_vm->_graphicsMan->clearScreen();

	if (skipSeqFl) {
		_vm->_graphicsMan->setPaletteVGA256(_vm->_graphicsMan->_palette);
	} else {
		_vm->_graphicsMan->setPaletteVGA256(_vm->_graphicsMan->_palette);
		_vm->_graphicsMan->display8BitRect(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_vm->_graphicsMan->updateScreen();
	}
	_vm->_events->_rateCounter = 0;
	_vm->_events->_escKeyFl = false;
	_vm->_soundMan->loadAnimSound();

	if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
		// Do pre-animation delay
		do {
			if (_vm->_events->_escKeyFl)
				break;

			_vm->_events->refreshEvents();
		} while (!_vm->shouldQuit() && _vm->_events->_rateCounter < rate1);
	}

	if (!_vm->_events->_escKeyFl) {
		_vm->_events->_rateCounter = 0;
		int frameNumber = 0;
		while (!_vm->shouldQuit()) {
			++frameNumber;
			_vm->_soundMan->playAnimSound(frameNumber);

			byte imageStr[17];
			// Read frame header
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;
			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));

			if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
				do {
					if (_vm->_events->_escKeyFl)
						break;

					_vm->_events->refreshEvents();
					_vm->_soundMan->checkSoundEnd();
				} while (!_vm->shouldQuit() && _vm->_events->_rateCounter < rate2);
			}

			if (!_vm->_events->_escKeyFl) {
				_vm->_events->_rateCounter = 0;
				if (*screenP != kByteStop)
					_vm->_graphicsMan->copyVideoVbe16(screenP);

				_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				_vm->_graphicsMan->updateScreen();
				_vm->_soundMan->checkSoundEnd();
			}
		}
	}

	if (_vm->_globals->_eventMode == EVENTMODE_IGNORE && !_vm->_events->_escKeyFl) {
		// Do post-animation delay
		do {
			if (_vm->_events->_escKeyFl)
				break;

			_vm->_events->refreshEvents();
			_vm->_soundMan->checkSoundEnd();
		} while (_vm->_events->_rateCounter < rate3);
	}

	if (!_vm->_events->_escKeyFl) {
		_vm->_events->_rateCounter = 0;
		_vm->_soundMan->checkSoundEnd();
	}

	if (_vm->_graphicsMan->_fadingFl) {
		byte *screenCopy = _vm->_globals->allocMemory(307200);

		f.seek(6);
		f.read(_vm->_graphicsMan->_palette, 800);
		f.skip(4);
		nbytes = f.readUint32LE();
		f.skip(14);
		f.read(screenP, nbytes);

		memcpy(screenCopy, screenP, 307200);

		for (;;) {
			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (*screenP != kByteStop)
				_vm->_graphicsMan->copyWinscanVbe3(screenP, screenCopy);
		}
		_vm->_graphicsMan->fadeOutDefaultLength(screenCopy);
		_vm->_globals->freeMemory(screenCopy);
	}

	_vm->_graphicsMan->_fadingFl = false;
	f.close();
	_vm->_graphicsMan->_skipVideoLockFl = false;

	_vm->_events->mouseOn();
}

/**
 * Play Animation, type 2
 */
void AnimationManager::playAnim2(const Common::Path &hiresName, const Common::Path &lowresName, uint32 rate1, uint32 rate2, uint32 rate3) {
	int oldScrollPosX = 0;
	byte *screenP = nullptr;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	_vm->_events->mouseOff();

	while (!_vm->shouldQuit()) {
		memcpy(_vm->_graphicsMan->_oldPalette, _vm->_graphicsMan->_palette, 769);

		_vm->_graphicsMan->backupScreen();

		if (!_vm->_graphicsMan->_lineNbr)
			_vm->_graphicsMan->_scrollOffset = 0;

		screenP = _vm->_graphicsMan->_backBuffer;
		if (!f.open(hiresName)) {
			if (!f.open(lowresName))
				error("Error opening files: %s - %s", hiresName.toString().c_str(), lowresName.toString().c_str());
		}

		f.skip(6);
		f.read(_vm->_graphicsMan->_palette, 800);
		f.skip(4);
		size_t nbytes = f.readUint32LE();
		f.skip(14);

		f.read(screenP, nbytes);

		_vm->_graphicsMan->clearPalette();
		oldScrollPosX = _vm->_graphicsMan->_scrollPosX;
		_vm->_graphicsMan->setScreenWidth(SCREEN_WIDTH);
		_vm->_graphicsMan->scrollScreen(0);
		_vm->_graphicsMan->clearScreen();
		_vm->_graphicsMan->_maxX = SCREEN_WIDTH;

		_vm->_graphicsMan->setPaletteVGA256(_vm->_graphicsMan->_palette);
		_vm->_graphicsMan->display8BitRect(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_vm->_graphicsMan->updateScreen();

		_vm->_events->_rateCounter = 0;
		_vm->_events->_escKeyFl = false;
		_vm->_soundMan->loadAnimSound();
		if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
			while (!_vm->_events->_escKeyFl && _vm->_events->_rateCounter < rate1) {
				_vm->_events->refreshEvents();
			}
		}
		break;
	}

	if (!_vm->_events->_escKeyFl) {
		_vm->_events->_rateCounter = 0;
		int frameNumber = 0;
		for (;;) {
			if (_vm->_events->_escKeyFl)
				break;
			++frameNumber;
			_vm->_soundMan->playAnimSound(frameNumber);
			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
				while (!_vm->_events->_escKeyFl && _vm->_events->_rateCounter < rate2) {
					_vm->_events->refreshEvents();
					_vm->_soundMan->checkSoundEnd();
				}
			}

			_vm->_events->_rateCounter = 0;
			if (*screenP != kByteStop)
				_vm->_graphicsMan->copyVideoVbe16(screenP);

			_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_vm->_graphicsMan->updateScreen();
			_vm->_soundMan->checkSoundEnd();
		}

		if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
			while (!_vm->_events->_escKeyFl && _vm->_events->_rateCounter < rate3) {
				_vm->_events->refreshEvents();
				_vm->_soundMan->checkSoundEnd();
			}
		}
	}

	_vm->_graphicsMan->_skipVideoLockFl = false;
	f.close();

	if (_vm->_graphicsMan->_fadingFl) {
		f.seek(6);
		f.read(_vm->_graphicsMan->_palette, 800);
		f.skip(4);
		size_t nbytes = f.readUint32LE();
		f.skip(14);
		f.read(screenP, nbytes);
		byte *ptra = _vm->_globals->allocMemory(307200);
		memcpy(ptra, screenP, 307200);

		for (;;) {
			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (*screenP != kByteStop)
				_vm->_graphicsMan->copyWinscanVbe3(screenP, ptra);
		}
		_vm->_graphicsMan->fadeOutDefaultLength(ptra);
		ptra = _vm->_globals->freeMemory(ptra);
	}
	_vm->_graphicsMan->_fadingFl = false;

	_vm->_graphicsMan->restoreScreen();

	memcpy(_vm->_graphicsMan->_palette, _vm->_graphicsMan->_oldPalette, 769);
	_vm->_graphicsMan->clearPalette();
	_vm->_graphicsMan->clearScreen();

	_vm->_graphicsMan->_scrollPosX = oldScrollPosX;
	_vm->_graphicsMan->scrollScreen(oldScrollPosX);
	if (_vm->_graphicsMan->_largeScreenFl) {
		_vm->_graphicsMan->setScreenWidth(2 * SCREEN_WIDTH);
		_vm->_graphicsMan->_maxX = 2 * SCREEN_WIDTH;
		_vm->_graphicsMan->display8BitRect(_vm->_graphicsMan->_frontBuffer, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	} else {
		_vm->_graphicsMan->setScreenWidth(SCREEN_WIDTH);
		_vm->_graphicsMan->_maxX = SCREEN_WIDTH;
		_vm->_graphicsMan->clearScreen();
		_vm->_graphicsMan->display8BitRect(_vm->_graphicsMan->_frontBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	}
	_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	_vm->_graphicsMan->fadeInShort();
	_vm->_graphicsMan->updateScreen();

	_vm->_events->mouseOn();
}

/**
 * Load Animation
 */
void AnimationManager::loadAnim(const Common::Path &animName) {
	clearAnim();

	Common::Path filename(animName);
	filename.appendInPlace(".ANI");
	Common::File f;
	if (!f.open(filename))
		error("Failed to open %s", filename.toString().c_str());

	int filesize = f.size();
	int nbytes = filesize - 115;

	char header[10];
	char dummyBuf[15];
	char filename1[15];
	char filename2[15];
	char filename3[15];
	char filename4[15];
	char filename5[15];
	char filename6[15];

	f.read(header, 10);
	f.read(dummyBuf, 15);
	f.read(filename1, 15);
	f.read(filename2, 15);
	f.read(filename3, 15);
	f.read(filename4, 15);
	f.read(filename5, 15);
	f.read(filename6, 15);

	if (READ_BE_UINT32(header) != MKTAG('A', 'N', 'I', 'S'))
		error("Invalid animation File: %s", filename.toString().c_str());

	const char *files[6] = { &filename1[0], &filename2[0], &filename3[0], &filename4[0],
			&filename5[0], &filename6[0] };

	for (int idx = 0; idx <= 5; ++idx) {
		if (files[idx][0]) {
			if (!f.exists(files[idx]))
				error("Missing file %s in animation File: %s", files[idx], filename.toString().c_str());
			if (loadSpriteBank(idx + 1, files[idx]))
				error("Invalid sprite bank in animation File: %s", filename.toString().c_str());
		}
	}

	byte *data = _vm->_globals->allocMemory(nbytes + 1);
	f.read(data, nbytes);
	f.close();

	for (int idx = 1; idx <= 20; ++idx)
		searchAnim(data, idx, nbytes);

	_vm->_globals->freeMemory(data);
}

/**
 * Clear animation
 */
void AnimationManager::clearAnim() {
	for (int idx = 0; idx < 35; ++idx) {
		_animBqe[idx]._data = _vm->_globals->freeMemory(_animBqe[idx]._data);
		_animBqe[idx]._enabledFl = false;
	}

	for (int idx = 0; idx < 8; ++idx) {
		Bank[idx]._data = _vm->_globals->freeMemory(Bank[idx]._data);
		Bank[idx]._loadedFl = false;
		Bank[idx]._filename = "";
		Bank[idx]._fileHeader = 0;
	}
}

/**
 * Load Sprite Bank
 */
int AnimationManager::loadSpriteBank(int idx, const Common::Path &filename) {
	int result = 0;
	Bank[idx]._loadedFl = true;
	Bank[idx]._filename = filename;

	byte *fileDataPtr = _vm->_fileIO->loadFile(filename);

	Bank[idx]._fileHeader = 0;
	if (fileDataPtr[1] == 'L' && fileDataPtr[2] == 'E')
		Bank[idx]._fileHeader = 1;
	else if (fileDataPtr[1] == 'O' && fileDataPtr[2] == 'R')
		Bank[idx]._fileHeader = 2;

	if (!Bank[idx]._fileHeader) {
		_vm->_globals->freeMemory(fileDataPtr);
		Bank[idx]._loadedFl = false;
		result = -1;
	}

	Bank[idx]._data = fileDataPtr;

	int objectDataIdx = 0;
	for(objectDataIdx = 0; objectDataIdx <= 249; objectDataIdx++) {
		int width = _vm->_objectsMan->getWidth(fileDataPtr, objectDataIdx);
		int height = _vm->_objectsMan->getHeight(fileDataPtr, objectDataIdx);
		if (!width && !height)
			break;
	}

	if (objectDataIdx > 249) {
		_vm->_globals->freeMemory(fileDataPtr);
		Bank[idx]._loadedFl = false;
		result = -2;
	}
	Bank[idx]._objDataIdx = objectDataIdx;

	Common::String ofsFilename = Bank[idx]._filename.baseName();
	char ch;
	do {
		ch = ofsFilename.lastChar();
		ofsFilename.deleteLastChar();
	} while (ch != '.');
	ofsFilename += ".OFS";

	Common::Path ofsPathname(Bank[idx]._filename.getParent().appendComponent(ofsFilename));
	Common::File f;
	if (f.exists(ofsPathname)) {
		byte *ofsData = _vm->_fileIO->loadFile(ofsPathname);
		byte *curOfsData = ofsData;
		for (int objIdx = 0; objIdx < Bank[idx]._objDataIdx; ++objIdx, curOfsData += 8) {
			int x1 = READ_LE_INT16(curOfsData);
			int y1 = READ_LE_INT16(curOfsData + 2);
			int x2 = READ_LE_INT16(curOfsData + 4);
			int y2 = READ_LE_INT16(curOfsData + 6);

			_vm->_objectsMan->setOffsetXY(Bank[idx]._data, objIdx, x1, y1, 0);
			if (Bank[idx]._fileHeader == 2)
				_vm->_objectsMan->setOffsetXY(Bank[idx]._data, objIdx, x2, y2, 1);
		}

		_vm->_globals->freeMemory(ofsData);
		result = 0;
	}

	return result;
}

/**
 * Search Animation
 */
void AnimationManager::searchAnim(const byte *data, int animIndex, int bufSize) {
	for (int dataIdx = 0; dataIdx <= bufSize; dataIdx++) {
		if (READ_BE_UINT32(&data[dataIdx]) == MKTAG('A', 'N', 'I', 'M')) {
			int entryIndex = data[dataIdx + 4];
			if (animIndex == entryIndex) {
				int curBufferPos = dataIdx + 5;
				int count = 0;
				bool innerLoopCond = false;
				do {
					if (READ_BE_UINT32(&data[curBufferPos]) == MKTAG('A', 'N', 'I', 'M') || READ_BE_UINT24(&data[curBufferPos]) == MKTAG24('F', 'I', 'N'))
						innerLoopCond = true;
					if (bufSize < curBufferPos) {
						_animBqe[animIndex]._enabledFl = false;
						_animBqe[animIndex]._data = nullptr;
						return;
					}
					++curBufferPos;
					++count;
				} while (!innerLoopCond);
				_animBqe[animIndex]._data = _vm->_globals->allocMemory(count + 50);
				_animBqe[animIndex]._enabledFl = true;
				memcpy(_animBqe[animIndex]._data, data + dataIdx + 5, 20);

				byte *dataP = _animBqe[animIndex]._data;
				int curDestDataIndx = 20;
				int curSrcDataIndx = dataIdx + 25;

				for (int i = 0; i <= 4999; i++) {
					memcpy(dataP + curDestDataIndx, data + curSrcDataIndx, 10);
					if (!READ_LE_UINT16(data + curSrcDataIndx + 4))
						break;
					curDestDataIndx += 10;
					curSrcDataIndx += 10;
				}
				break;
			}
		}
		if (READ_BE_UINT24(&data[dataIdx]) == MKTAG24('F', 'I', 'N'))
			break;
	}
}

/**
 * Play sequence
 */
void AnimationManager::playSequence(const Common::Path &file, uint32 rate1, uint32 rate2, uint32 rate3, bool skipEscFl, bool skipSeqFl, bool noColFl) {
	if (_vm->shouldQuit())
		return;

	_vm->_events->_mouseFl = false;
	if (!noColFl) {
		_vm->_events->refreshScreenAndEvents();

		_vm->_graphicsMan->backupScreen();

		if (!_vm->_graphicsMan->_lineNbr)
			_vm->_graphicsMan->_scrollOffset = 0;
	}
	byte *screenP = _vm->_graphicsMan->_backBuffer;
	Common::File f;
	if (!f.open(file))
		error("Error opening file - %s", file.toString().c_str());

	f.skip(6);
	f.read(_vm->_graphicsMan->_palette, 800);
	f.skip(4);
	size_t nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	if (skipSeqFl) {
		if (!_vm->getIsDemo()) {
			_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);
		}
		_vm->_graphicsMan->setPaletteVGA256(_vm->_graphicsMan->_palette);
	} else {
		_vm->_graphicsMan->display8BitRect(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_vm->_graphicsMan->updateScreen();
	}
	bool skipFl = false;
	if (noColFl)
		_vm->_graphicsMan->fadeInDefaultLength(screenP);
	_vm->_events->_rateCounter = 0;
	_vm->_events->_escKeyFl = false;
	_vm->_soundMan->loadAnimSound();
	if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
		do {
			if (_vm->shouldQuit() || (_vm->_events->_escKeyFl && !skipEscFl)) {
				skipFl = true;
				break;
			}

			_vm->_events->_escKeyFl = false;
			_vm->_events->refreshEvents();
			_vm->_soundMan->checkSoundEnd();
		} while (_vm->_events->_rateCounter < rate1);
	}
	_vm->_events->_rateCounter = 0;
	if (!skipFl) {
		int soundNumber = 0;
		for (;;) {
			++soundNumber;
			_vm->_soundMan->playAnimSound(soundNumber);
			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
				do {
					if (_vm->shouldQuit() || (_vm->_events->_escKeyFl && !skipEscFl)) {
						skipFl = true;
						break;
					}

					_vm->_events->_escKeyFl = false;
					_vm->_events->refreshEvents();
					_vm->_soundMan->checkSoundEnd();
				} while (_vm->_events->_rateCounter < rate2);
			}

			if (skipFl)
				break;

			_vm->_events->_rateCounter = 0;
			if (*screenP != kByteStop)
				_vm->_graphicsMan->copyVideoVbe16a(screenP);

			_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_vm->_graphicsMan->updateScreen();
			_vm->_soundMan->checkSoundEnd();
		}
	}

	if (_vm->_globals->_eventMode == EVENTMODE_IGNORE && !skipFl) {
		do {
			if (_vm->shouldQuit() || (_vm->_events->_escKeyFl && !skipEscFl)) {
				skipFl = true;
				break;
			}

			_vm->_events->_escKeyFl = false;
			_vm->_events->refreshEvents();
			_vm->_soundMan->checkSoundEnd();
		} while (_vm->_events->_rateCounter < rate3);
	}

	if (!skipFl)
		_vm->_events->_rateCounter = 0;

	_vm->_graphicsMan->_skipVideoLockFl = false;
	f.close();

	if (!noColFl) {
		_vm->_graphicsMan->restoreScreen();

		_vm->_events->_mouseFl = true;
	}
}

/**
 * Play Sequence type 2
 */
void AnimationManager::playSequence2(const Common::Path &file, uint32 rate1, uint32 rate2, uint32 rate3, bool skipSeqFl) {
	byte *screenP;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	_vm->_events->_mouseFl = false;
	screenP = _vm->_graphicsMan->_backBuffer;

	if (!f.open(file))
		error("File not found - %s", file.toString().c_str());

	f.skip(6);
	f.read(_vm->_graphicsMan->_palette, 800);
	f.skip(4);
	size_t nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	if (skipSeqFl) {
		_vm->_graphicsMan->setPaletteVGA256(_vm->_graphicsMan->_palette);
	} else {
		_vm->_graphicsMan->setPaletteVGA256(_vm->_graphicsMan->_palette);
		_vm->_graphicsMan->display8BitRect(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);

		_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_vm->_graphicsMan->updateScreen();
	}
	_vm->_events->_rateCounter = 0;
	_vm->_events->_escKeyFl = false;
	_vm->_soundMan->loadAnimSound();
	if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
		do {
			_vm->_events->refreshEvents();
			_vm->_soundMan->checkSoundEnd();
		} while (!_vm->shouldQuit() && !_vm->_events->_escKeyFl && _vm->_events->_rateCounter < rate1);
	}

	if (!_vm->_events->_escKeyFl) {
		_vm->_events->_rateCounter = 0;
		int frameNumber = 0;
		while (!_vm->shouldQuit()) {
			_vm->_soundMan->playAnimSound(frameNumber++);

			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
				do {
					_vm->_events->refreshEvents();
				} while (!_vm->shouldQuit() && !_vm->_events->_escKeyFl && _vm->_events->_rateCounter < rate2);
			}

			_vm->_events->_rateCounter = 0;
			if (*screenP != kByteStop)
				_vm->_graphicsMan->copyVideoVbe16a(screenP);

			_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_vm->_graphicsMan->updateScreen();
			_vm->_soundMan->checkSoundEnd();
		}
	}

	if (_vm->_globals->_eventMode == EVENTMODE_IGNORE) {
		// Wait for third rate delay
		do {
			_vm->_events->refreshEvents();
			_vm->_soundMan->checkSoundEnd();
		} while (!_vm->shouldQuit() && !_vm->_events->_escKeyFl && _vm->_events->_rateCounter < rate3);
	}

	_vm->_events->_rateCounter = 0;

	if (_vm->_graphicsMan->_fadingFl) {
		byte *ptra = _vm->_globals->allocMemory(307200);

		f.seek(6);
		f.read(_vm->_graphicsMan->_palette, 800);
		f.skip(4);
		nbytes = f.readUint32LE();
		f.skip(14);
		f.read(screenP, nbytes);

		memcpy(ptra, screenP, 307200);
		for (;;) {
			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (*screenP != kByteStop)
				_vm->_graphicsMan->copyWinscanVbe(screenP, ptra);
		}
		_vm->_graphicsMan->fadeOutDefaultLength(ptra);
		ptra = _vm->_globals->freeMemory(ptra);
	}
	_vm->_graphicsMan->_fadingFl = false;

	f.close();
	_vm->_events->_mouseFl = true;
}

void AnimationManager::initAnimBqe() {
	for (int idx = 0; idx < 35; ++idx) {
		_animBqe[idx]._data = nullptr;
		_animBqe[idx]._enabledFl = false;
	}

	for (int idx = 0; idx < 8; ++idx) {
		Bank[idx]._data = nullptr;
		Bank[idx]._loadedFl = false;
		Bank[idx]._filename = "";
		Bank[idx]._fileHeader = 0;
	}
}

} // End of namespace Hopkins
