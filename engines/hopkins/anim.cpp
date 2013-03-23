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

#include "hopkins/anim.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"

namespace Hopkins {

AnimationManager::AnimationManager(HopkinsEngine *vm) {
	_vm = vm;
	_clearAnimationFl = false;
}

/**
 * Play Animation
 * @param filename		Filename of animation to play
 * @param rate1			Delay amount before starting animation
 * @param rate2			Delay amount between animation frames
 * @param rate3			Delay amount after animation finishes
 */
void AnimationManager::playAnim(const Common::String &filename, uint32 rate1, uint32 rate2, uint32 rate3, bool skipSeqFl) {
	byte *screenCopy = NULL;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	_vm->_eventsManager->mouseOff();

	bool hasScreenCopy = false;
	byte *screenP = _vm->_graphicsManager->_vesaScreen;

	Common::String tmpStr;
	// The Windows 95 demo only contains the interlaced version of the BOMBE1 and BOMBE2 videos
	if (_vm->getPlatform() == Common::kPlatformWindows && _vm->getIsDemo() && filename == "BOMBE1A.ANM")
		tmpStr = "BOMBE1.ANM";
	else if (_vm->getPlatform() == Common::kPlatformWindows && _vm->getIsDemo() && filename == "BOMBE2A.ANM")
		tmpStr = "BOMBE2.ANM";
	else
		tmpStr = filename;
	if (!f.open(tmpStr))
		error("File not found - %s", tmpStr.c_str());

	f.skip(6);
	f.read(_vm->_graphicsManager->_palette, 800);
	f.skip(4);
	size_t nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	if (_clearAnimationFl) {
		_vm->_graphicsManager->lockScreen();
		_vm->_graphicsManager->clearScreen();
		_vm->_graphicsManager->unlockScreen();
	}
	if (_vm->_graphicsManager->WinScan / 2 > SCREEN_WIDTH) {
		hasScreenCopy = true;
		screenCopy = _vm->_globals->allocMemory(307200);
		memcpy(screenCopy, screenP, 307200);
	}
	if (skipSeqFl) {
		if (hasScreenCopy)
			memcpy(screenCopy, _vm->_graphicsManager->_vesaBuffer, 307200);
		_vm->_graphicsManager->setPaletteVGA256(_vm->_graphicsManager->_palette);
	} else {
		_vm->_graphicsManager->setPaletteVGA256(_vm->_graphicsManager->_palette);
		_vm->_graphicsManager->lockScreen();
		if (hasScreenCopy)
			_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager->m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager->unlockScreen();

		_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_vm->_graphicsManager->updateScreen();
	}
	_vm->_eventsManager->_rateCounter = 0;
	_vm->_eventsManager->_escKeyFl = false;
	_vm->_soundManager->loadAnimSound();

	if (_vm->_globals->iRegul == 1) {
		// Do pre-animation delay
		do {
			if (_vm->_eventsManager->_escKeyFl)
				break;

			_vm->_eventsManager->refreshEvents();
		} while (!_vm->shouldQuit() && _vm->_eventsManager->_rateCounter < rate1);
	}

	if (!_vm->_eventsManager->_escKeyFl) {
		_vm->_eventsManager->_rateCounter = 0;
		int frameNumber = 0;
		while (!_vm->shouldQuit()) {
			++frameNumber;
			_vm->_soundManager->playAnimSound(frameNumber);

			byte imageStr[17];
			// Read frame header
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;
			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));

			if (_vm->_globals->iRegul == 1) {
				do {
					if (_vm->_eventsManager->_escKeyFl)
						break;

					_vm->_eventsManager->refreshEvents();
					_vm->_soundManager->checkSoundEnd();
				} while (!_vm->shouldQuit() && _vm->_eventsManager->_rateCounter < rate2);
			}

			if (!_vm->_eventsManager->_escKeyFl) {
				_vm->_eventsManager->_rateCounter = 0;
				_vm->_graphicsManager->lockScreen();
				if (hasScreenCopy) {
					if (*screenP != kByteStop) {
						_vm->_graphicsManager->copyWinscanVbe3(screenP, screenCopy);
						_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
					}
				} else if (*screenP != kByteStop) {
					_vm->_graphicsManager->copyVideoVbe16(screenP);
				}
				_vm->_graphicsManager->unlockScreen();

				_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				_vm->_graphicsManager->updateScreen();
				_vm->_soundManager->checkSoundEnd();
			}
		}
	}

	if (_vm->_globals->iRegul == 1 && !_vm->_eventsManager->_escKeyFl) {
		// Do post-animation delay
		do {
			if (_vm->_eventsManager->_escKeyFl)
				break;

			_vm->_eventsManager->refreshEvents();
			_vm->_soundManager->checkSoundEnd();
		} while (_vm->_eventsManager->_rateCounter < rate3);
	}

	if (!_vm->_eventsManager->_escKeyFl) {
		_vm->_eventsManager->_rateCounter = 0;
		_vm->_soundManager->checkSoundEnd();
	}

	if (_vm->_graphicsManager->FADE_LINUX == 2 && !hasScreenCopy) {
		screenCopy = _vm->_globals->allocMemory(307200);

		f.seek(6);
		f.read(_vm->_graphicsManager->_palette, 800);
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
				_vm->_graphicsManager->copyWinscanVbe3(screenP, screenCopy);
		}
		_vm->_graphicsManager->fadeOutDefaultLength(screenCopy);
		screenCopy = _vm->_globals->freeMemory(screenCopy);
	}
	if (hasScreenCopy) {
		if (_vm->_graphicsManager->FADE_LINUX == 2)
			_vm->_graphicsManager->fadeOutDefaultLength(screenCopy);
		screenCopy = _vm->_globals->freeMemory(screenCopy);
	}

	_vm->_graphicsManager->FADE_LINUX = 0;
	f.close();
	_vm->_graphicsManager->_skipVideoLockFl = false;

	_vm->_eventsManager->mouseOn();
}

/**
 * Play Animation, type 2
 */
void AnimationManager::playAnim2(const Common::String &filename, uint32 rate1, uint32 rate2, uint32 rate3) {
	byte *screenCopy = NULL;
	int oldScrollPosX = 0;
	byte *screenP = NULL;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	_vm->_eventsManager->mouseOff();

	bool hasScreenCopy = false;
	while (!_vm->shouldQuit()) {
		memcpy(_vm->_graphicsManager->_oldPalette, _vm->_graphicsManager->_palette, 769);

		if (_vm->_graphicsManager->_lineNbr == SCREEN_WIDTH)
			_vm->_saveLoadManager->saveFile("TEMP.SCR", _vm->_graphicsManager->_vesaScreen, 307200);
		else if (_vm->_graphicsManager->_lineNbr == (SCREEN_WIDTH * 2))
			_vm->_saveLoadManager->saveFile("TEMP.SCR", _vm->_graphicsManager->_vesaScreen, 614400);

		if (!_vm->_graphicsManager->_lineNbr)
			_vm->_graphicsManager->_scrollOffset = 0;

		screenP = _vm->_graphicsManager->_vesaScreen;
		if (!f.open(filename))
			error("Error opening file - %s", filename.c_str());

		f.skip(6);
		f.read(_vm->_graphicsManager->_palette, 800);
		f.skip(4);
		size_t nbytes = f.readUint32LE();
		f.skip(14);

		f.read(screenP, nbytes);

		_vm->_graphicsManager->clearPalette();
		oldScrollPosX = _vm->_graphicsManager->_scrollPosX;
		_vm->_graphicsManager->setScreenWidth(SCREEN_WIDTH);
		_vm->_graphicsManager->scrollScreen(0);
		_vm->_graphicsManager->lockScreen();
		_vm->_graphicsManager->clearScreen();
		_vm->_graphicsManager->unlockScreen();

		_vm->_graphicsManager->_maxX = SCREEN_WIDTH;
		if (_vm->_graphicsManager->WinScan / 2 > SCREEN_WIDTH) {
			hasScreenCopy = true;
			screenCopy = _vm->_globals->allocMemory(307200);
			memcpy(screenCopy, screenP, 307200);
		}

		_vm->_graphicsManager->setPaletteVGA256(_vm->_graphicsManager->_palette);
		_vm->_graphicsManager->lockScreen();
		if (hasScreenCopy)
			_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager->m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);

		_vm->_graphicsManager->unlockScreen();
		_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_vm->_graphicsManager->updateScreen();

		_vm->_eventsManager->_rateCounter = 0;
		_vm->_eventsManager->_escKeyFl = false;
		_vm->_soundManager->loadAnimSound();
		if (_vm->_globals->iRegul == 1) {
			while (!_vm->_eventsManager->_escKeyFl && _vm->_eventsManager->_rateCounter < rate1) {
				_vm->_eventsManager->refreshEvents();
			}
		}
		break;
	}

	if (!_vm->_eventsManager->_escKeyFl) {
		_vm->_eventsManager->_rateCounter = 0;
		int frameNumber = 0;
		for (;;) {
			if (_vm->_eventsManager->_escKeyFl)
				break;
			++frameNumber;
			_vm->_soundManager->playAnimSound(frameNumber);
			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (_vm->_globals->iRegul == 1) {
				while (!_vm->_eventsManager->_escKeyFl && _vm->_eventsManager->_rateCounter < rate2) {
					_vm->_eventsManager->refreshEvents();
					_vm->_soundManager->checkSoundEnd();
				}
			}

			_vm->_eventsManager->_rateCounter = 0;
			_vm->_graphicsManager->lockScreen();
			if (*screenP != kByteStop) {
				if (hasScreenCopy) {
					_vm->_graphicsManager->copyWinscanVbe3(screenP, screenCopy);
					_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				} else {
					_vm->_graphicsManager->copyVideoVbe16(screenP);
				}
			}
			_vm->_graphicsManager->unlockScreen();

			_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_vm->_graphicsManager->updateScreen();
			_vm->_soundManager->checkSoundEnd();
		}

		if (_vm->_globals->iRegul == 1) {
			while (!_vm->_eventsManager->_escKeyFl && _vm->_eventsManager->_rateCounter < rate3) {
				_vm->_eventsManager->refreshEvents();
				_vm->_soundManager->checkSoundEnd();
			}
		}
	}

	_vm->_graphicsManager->_skipVideoLockFl = false;
	f.close();

	if (_vm->_graphicsManager->FADE_LINUX == 2 && !hasScreenCopy) {
		f.seek(6);
		f.read(_vm->_graphicsManager->_palette, 800);
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
				_vm->_graphicsManager->copyWinscanVbe3(screenP, ptra);
		}
		_vm->_graphicsManager->fadeOutDefaultLength(ptra);
		ptra = _vm->_globals->freeMemory(ptra);
	}
	if (hasScreenCopy) {
		if (_vm->_graphicsManager->FADE_LINUX == 2)
			_vm->_graphicsManager->fadeOutDefaultLength(screenCopy);
		_vm->_globals->freeMemory(screenCopy);
	}
	_vm->_graphicsManager->FADE_LINUX = 0;

	_vm->_saveLoadManager->load("TEMP.SCR", _vm->_graphicsManager->_vesaScreen);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	memcpy(_vm->_graphicsManager->_palette, _vm->_graphicsManager->_oldPalette, 769);
	_vm->_graphicsManager->clearPalette();
	_vm->_graphicsManager->lockScreen();
	_vm->_graphicsManager->clearScreen();
	_vm->_graphicsManager->unlockScreen();

	_vm->_graphicsManager->_scrollPosX = oldScrollPosX;
	_vm->_graphicsManager->scrollScreen(oldScrollPosX);
	if (_vm->_graphicsManager->_largeScreenFl) {
		_vm->_graphicsManager->setScreenWidth(2 * SCREEN_WIDTH);
		_vm->_graphicsManager->_maxX = 2 * SCREEN_WIDTH;
		_vm->_graphicsManager->lockScreen();
		_vm->_graphicsManager->m_scroll16(_vm->_graphicsManager->_vesaBuffer, _vm->_eventsManager->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	} else {
		_vm->_graphicsManager->setScreenWidth(SCREEN_WIDTH);
		_vm->_graphicsManager->_maxX = SCREEN_WIDTH;
		_vm->_graphicsManager->lockScreen();
		_vm->_graphicsManager->clearScreen();
		_vm->_graphicsManager->m_scroll16(_vm->_graphicsManager->_vesaBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	}
	_vm->_graphicsManager->unlockScreen();
	_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	_vm->_graphicsManager->fadeInShort();
	_vm->_graphicsManager->updateScreen();

	_vm->_eventsManager->mouseOn();
}

/**
 * Load Animation
 */
void AnimationManager::loadAnim(const Common::String &animName) {
	clearAnim();

	Common::String filename = animName + ".ANI";
	Common::File f;
	if (!f.open(filename))
		error("Failed to open %s", filename.c_str());

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
		error("Invalid animation File: %s", filename.c_str());

	const char *files[6] = { &filename1[0], &filename2[0], &filename3[0], &filename4[0],
			&filename5[0], &filename6[0] };

	for (int idx = 0; idx <= 5; ++idx) {
		if (files[idx][0]) {
			if (!f.exists(files[idx]))
				error("Missing file %s in animation File: %s", files[idx], filename.c_str());
			if (loadSpriteBank(idx + 1, files[idx]))
				error("Invalid sprite bank in animation File: %s", filename.c_str());
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
		_vm->_globals->_animBqe[idx]._data = _vm->_globals->freeMemory(_vm->_globals->_animBqe[idx]._data);
		_vm->_globals->_animBqe[idx]._enabledFl = false;
	}

	for (int idx = 0; idx < 8; ++idx) {
		_vm->_globals->Bank[idx]._data = _vm->_globals->freeMemory(_vm->_globals->Bank[idx]._data);
		_vm->_globals->Bank[idx]._loadedFl = false;
		_vm->_globals->Bank[idx]._filename = "";
		_vm->_globals->Bank[idx]._fileHeader = 0;
	}
}

/**
 * Load Sprite Bank
 */
int AnimationManager::loadSpriteBank(int idx, const Common::String &filename) {
	int result = 0;
	_vm->_globals->Bank[idx]._loadedFl = true;
	_vm->_globals->Bank[idx]._filename = filename;

	byte *fileDataPtr = _vm->_fileManager->loadFile(filename);

	_vm->_globals->Bank[idx]._fileHeader = 0;
	if (fileDataPtr[1] == 'L' && fileDataPtr[2] == 'E')
		_vm->_globals->Bank[idx]._fileHeader = 1;
	else if (fileDataPtr[1] == 'O' && fileDataPtr[2] == 'R')
		_vm->_globals->Bank[idx]._fileHeader = 2;

	if (!_vm->_globals->Bank[idx]._fileHeader) {
		_vm->_globals->freeMemory(fileDataPtr);
		_vm->_globals->Bank[idx]._loadedFl = false;
		result = -1;
	}
	
	_vm->_globals->Bank[idx]._data = fileDataPtr;

	int objectDataIdx = 0;
	for(objectDataIdx = 0; objectDataIdx <= 249; objectDataIdx++) {
		int width = _vm->_objectsManager->getWidth(fileDataPtr, objectDataIdx);
		int height = _vm->_objectsManager->getHeight(fileDataPtr, objectDataIdx);
		if (!width && !height)
			break;
	}

	if (objectDataIdx > 249) {
		_vm->_globals->freeMemory(fileDataPtr);
		_vm->_globals->Bank[idx]._loadedFl = false;
		result = -2;
	}
	_vm->_globals->Bank[idx]._objDataIdx = objectDataIdx;

	Common::String ofsFilename = _vm->_globals->Bank[idx]._filename;
	char ch;
	do {
		ch = ofsFilename.lastChar();
		ofsFilename.deleteLastChar();
	} while (ch != '.');
	ofsFilename += ".OFS";

	Common::File f;
	if (f.exists(ofsFilename)) {
		byte *ofsData = _vm->_fileManager->loadFile(ofsFilename);
		byte *curOfsData = ofsData;
		for (int objIdx = 0; objIdx < _vm->_globals->Bank[idx]._objDataIdx; ++objIdx, curOfsData += 8) {
			int x1 = READ_LE_INT16(curOfsData);
			int y1 = READ_LE_INT16(curOfsData + 2);
			int x2 = READ_LE_INT16(curOfsData + 4);
			int y2 = READ_LE_INT16(curOfsData + 6);

			_vm->_objectsManager->setOffsetXY(_vm->_globals->Bank[idx]._data, objIdx, x1, y1, 0);
			if (_vm->_globals->Bank[idx]._fileHeader == 2)
				_vm->_objectsManager->setOffsetXY(_vm->_globals->Bank[idx]._data, objIdx, x2, y2, 1);
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
						_vm->_globals->_animBqe[animIndex]._enabledFl = false;
						_vm->_globals->_animBqe[animIndex]._data = g_PTRNUL;
						return;
					}
					++curBufferPos;
					++count;
				} while (!innerLoopCond);
				_vm->_globals->_animBqe[animIndex]._data = _vm->_globals->allocMemory(count + 50);
				_vm->_globals->_animBqe[animIndex]._enabledFl = true;
				memcpy(_vm->_globals->_animBqe[animIndex]._data, data + dataIdx + 5, 20);

				byte *dataP = _vm->_globals->_animBqe[animIndex]._data;
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
void AnimationManager::playSequence(const Common::String &file, uint32 rate1, uint32 rate2, uint32 rate3, bool skipEscFl, bool skipSeqFl, bool noColFl) {
	if (_vm->shouldQuit())
		return;

	bool hasScreenCopy = false;
	_vm->_eventsManager->_mouseFl = false;
	if (!noColFl) {
		_vm->_eventsManager->refreshScreenAndEvents();

		if (_vm->_graphicsManager->_lineNbr == SCREEN_WIDTH)
			_vm->_saveLoadManager->saveFile("TEMP.SCR", _vm->_graphicsManager->_vesaScreen, 307200);
		else if (_vm->_graphicsManager->_lineNbr == (SCREEN_WIDTH * 2))
			_vm->_saveLoadManager->saveFile("TEMP.SCR", _vm->_graphicsManager->_vesaScreen, 614400);
		if (!_vm->_graphicsManager->_lineNbr)
			_vm->_graphicsManager->_scrollOffset = 0;
	}
	byte *screenP = _vm->_graphicsManager->_vesaScreen;
	Common::File f;
	if (!f.open(file))
		error("Error opening file - %s", file.c_str());

	f.skip(6);
	f.read(_vm->_graphicsManager->_palette, 800);
	f.skip(4);
	size_t nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	byte *screenCopy = NULL;
	if (_vm->_graphicsManager->WinScan / 2 > SCREEN_WIDTH) {
		hasScreenCopy = true;
		screenCopy = _vm->_globals->allocMemory(307200);
		memcpy(screenCopy, screenP, 307200);
	}
	if (skipSeqFl) {
		if (hasScreenCopy)
			memcpy(screenCopy, _vm->_graphicsManager->_vesaBuffer, 307200);
		if (!_vm->getIsDemo()) {
			_vm->_graphicsManager->SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager->SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager->SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager->SETCOLOR3(254, 0, 0, 0);
		}
		_vm->_graphicsManager->setPaletteVGA256(_vm->_graphicsManager->_palette);
	} else {
		_vm->_graphicsManager->lockScreen();
		if (hasScreenCopy)
			_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager->m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager->unlockScreen();

		_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_vm->_graphicsManager->updateScreen();
	}
	bool skipFl = false;
	if (noColFl)
		_vm->_graphicsManager->fadeInDefaultLength(screenP);
	_vm->_eventsManager->_rateCounter = 0;
	_vm->_eventsManager->_escKeyFl = false;
	_vm->_soundManager->loadAnimSound();
	if (_vm->_globals->iRegul == 1) {
		do {
			if (_vm->shouldQuit() || (_vm->_eventsManager->_escKeyFl && !skipEscFl)) {
				skipFl = true;
				break;
			}

			_vm->_eventsManager->_escKeyFl = false;
			_vm->_eventsManager->refreshEvents();
			_vm->_soundManager->checkSoundEnd();
		} while (_vm->_eventsManager->_rateCounter < rate1);
	}
	_vm->_eventsManager->_rateCounter = 0;
	if (!skipFl) {
		int soundNumber = 0;
		for (;;) {
			++soundNumber;
			_vm->_soundManager->playAnimSound(soundNumber);
			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (_vm->_globals->iRegul == 1) {
				do {
					if (_vm->shouldQuit() || (_vm->_eventsManager->_escKeyFl && !skipEscFl)) {
						skipFl = true;
						break;
					}

					_vm->_eventsManager->_escKeyFl = false;
					_vm->_eventsManager->refreshEvents();
					_vm->_soundManager->checkSoundEnd();
				} while (_vm->_eventsManager->_rateCounter < rate2);
			}

			if (skipFl)
				break;

			_vm->_eventsManager->_rateCounter = 0;
			_vm->_graphicsManager->lockScreen();
			if (hasScreenCopy) {
				if (*screenP != kByteStop) {
					_vm->_graphicsManager->copyWinscanVbe(screenP, screenCopy);
					_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				}
			} else if (*screenP != kByteStop) {
				_vm->_graphicsManager->copyVideoVbe16a(screenP);
			}
			_vm->_graphicsManager->unlockScreen();

			_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_vm->_graphicsManager->updateScreen();
			_vm->_soundManager->checkSoundEnd();
		}
	}

	if (_vm->_globals->iRegul == 1 && !skipFl) {
		do {
			if (_vm->shouldQuit() || (_vm->_eventsManager->_escKeyFl && !skipEscFl)) {
				skipFl = true;
				break;
			}

			_vm->_eventsManager->_escKeyFl = false;
			_vm->_eventsManager->refreshEvents();
			_vm->_soundManager->checkSoundEnd();
		} while (_vm->_eventsManager->_rateCounter < rate3);
	}

	if (!skipFl)
		_vm->_eventsManager->_rateCounter = 0;

	_vm->_graphicsManager->_skipVideoLockFl = false;
	f.close();

	if (!noColFl) {
		_vm->_saveLoadManager->load("TEMP.SCR", _vm->_graphicsManager->_vesaScreen);
		g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

		_vm->_eventsManager->_mouseFl = true;
	}
	if (hasScreenCopy)
		_vm->_globals->freeMemory(screenCopy);
}

/**
 * Play Sequence type 2
 */
void AnimationManager::playSequence2(const Common::String &file, uint32 rate1, uint32 rate2, uint32 rate3, bool skipSeqFl) {
	byte *screenCopy = NULL;
	byte *screenP;
	int frameNumber;
	Common::File f;

	bool multiScreenFl = false;
	for (;;) {
		if (_vm->shouldQuit())
			return;

		_vm->_eventsManager->_mouseFl = false;
		screenP = _vm->_graphicsManager->_vesaScreen;

		if (!f.open(file))
			error("File not found - %s", file.c_str());

		f.skip(6);
		f.read(_vm->_graphicsManager->_palette, 800);
		f.skip(4);
		size_t nbytes = f.readUint32LE();
		f.skip(14);
		f.read(screenP, nbytes);

		if (_vm->_graphicsManager->WinScan / 2 > SCREEN_WIDTH) {
			multiScreenFl = true;
			screenCopy = _vm->_globals->allocMemory(307200);
			memcpy((void *)screenCopy, screenP, 307200);
		}
		if (skipSeqFl) {
			if (multiScreenFl) {
				assert(screenCopy != NULL);
				memcpy((void *)screenCopy, _vm->_graphicsManager->_vesaBuffer, 307200);
			}
			_vm->_graphicsManager->setPaletteVGA256(_vm->_graphicsManager->_palette);
		} else {
			_vm->_graphicsManager->lockScreen();
			_vm->_graphicsManager->setPaletteVGA256(_vm->_graphicsManager->_palette);
			if (multiScreenFl)
				_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				_vm->_graphicsManager->m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			_vm->_graphicsManager->unlockScreen();

			_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_vm->_graphicsManager->updateScreen();
		}
		_vm->_eventsManager->_rateCounter = 0;
		_vm->_eventsManager->_escKeyFl = false;
		_vm->_soundManager->loadAnimSound();
		if (_vm->_globals->iRegul == 1) {
			do {
				_vm->_eventsManager->refreshEvents();
				_vm->_soundManager->checkSoundEnd();
			} while (!_vm->shouldQuit() && !_vm->_eventsManager->_escKeyFl && _vm->_eventsManager->_rateCounter < rate1);
		}
		break;
	}

	if (!_vm->_eventsManager->_escKeyFl) {
		_vm->_eventsManager->_rateCounter = 0;
		frameNumber = 0;
		while (!_vm->shouldQuit()) {
			_vm->_soundManager->playAnimSound(frameNumber++);

			byte imageStr[17];
			if (f.read(imageStr, 16) != 16)
				break;
			imageStr[16] = 0;

			if (strncmp((const char *)imageStr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(imageStr + 8));
			if (_vm->_globals->iRegul == 1) {
				do {
					_vm->_eventsManager->refreshEvents();
				} while (!_vm->shouldQuit() && !_vm->_eventsManager->_escKeyFl && _vm->_eventsManager->_rateCounter < rate2);
			}

			_vm->_eventsManager->_rateCounter = 0;
			_vm->_graphicsManager->lockScreen();
			if (multiScreenFl) {
				if (*screenP != kByteStop) {
					_vm->_graphicsManager->copyWinscanVbe(screenP, screenCopy);
					_vm->_graphicsManager->m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				}
			} else if (*screenP != kByteStop) {
				_vm->_graphicsManager->copyVideoVbe16a(screenP);
			}
			_vm->_graphicsManager->unlockScreen();

			_vm->_graphicsManager->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_vm->_graphicsManager->updateScreen();
			_vm->_soundManager->checkSoundEnd();
		}
	}

	if (_vm->_globals->iRegul == 1) {
		// Wait for third rate delay
		do {
			_vm->_eventsManager->refreshEvents();
			_vm->_soundManager->checkSoundEnd();
		} while (!_vm->shouldQuit() && !_vm->_eventsManager->_escKeyFl && _vm->_eventsManager->_rateCounter < rate3);
	}

	_vm->_eventsManager->_rateCounter = 0;

	if (_vm->_graphicsManager->FADE_LINUX == 2 && !multiScreenFl) {
		byte *ptra = _vm->_globals->allocMemory(307200);

		f.seek(6);
		f.read(_vm->_graphicsManager->_palette, 800);
		f.skip(4);
		size_t nbytes = f.readUint32LE();
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
				_vm->_graphicsManager->copyWinscanVbe(screenP, ptra);
		}
		_vm->_graphicsManager->fadeOutDefaultLength(ptra);
		ptra = _vm->_globals->freeMemory(ptra);
	}
	if (multiScreenFl) {
		if (_vm->_graphicsManager->FADE_LINUX == 2)
			_vm->_graphicsManager->fadeOutDefaultLength(screenCopy);
		_vm->_globals->freeMemory(screenCopy);
	}
	_vm->_graphicsManager->FADE_LINUX = 0;

	f.close();
	_vm->_eventsManager->_mouseFl = true;
}

} // End of namespace Hopkins
