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

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"
#include "hopkins/anim.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

AnimationManager::AnimationManager() {
	_clearAnimationFl = false;
	NO_SEQ = false;
	NO_COUL = false;
}

/**
 * Play Animation
 * @param filename		Filename of animation to play
 * @param rate1			Delay amount before starting animation
 * @param rate2			Delay amount between animation frames
 * @param rate3			Delay amount after animation finishes
 */
void AnimationManager::playAnim(const Common::String &filename, uint32 rate1, uint32 rate2, uint32 rate3) {
	byte *screenCopy = NULL;
	int frameNumber;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	bool hasScreenCopy = false;
	byte *screenP = _vm->_graphicsManager._vesaScreen;
	byte *ptr = _vm->_globals.allocMemory(20);

	// The Windows 95 demo only contains the interlaced version of the BOMBE1 and BOMBE2 videos
	if (_vm->getPlatform() == Common::kPlatformWindows && _vm->getIsDemo() && filename == "BOMBE1A.ANM")
		_vm->_fileManager.constructFilename("ANM", "BOMBE1.ANM");
	else if (_vm->getPlatform() == Common::kPlatformWindows && _vm->getIsDemo() && filename == "BOMBE2A.ANM")
		_vm->_fileManager.constructFilename("ANM", "BOMBE2.ANM");
	else
		_vm->_fileManager.constructFilename("ANM", filename);
	if (!f.open(_vm->_globals._curFilename))
		error("File not found - %s", _vm->_globals._curFilename.c_str());

	f.skip(6);
	f.read(_vm->_graphicsManager._palette, 800);
	f.skip(4);
	size_t nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	if (_clearAnimationFl) {
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.clearScreen();
		_vm->_graphicsManager.unlockScreen();
	}
	if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
		hasScreenCopy = true;
		screenCopy = _vm->_globals.allocMemory(307200);
		memcpy(screenCopy, screenP, 307200);
	}
	if (NO_SEQ) {
		if (hasScreenCopy)
			memcpy(screenCopy, _vm->_graphicsManager._vesaBuffer, 307200);
		_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	} else {
		_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
		_vm->_graphicsManager.lockScreen();
		if (hasScreenCopy)
			_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.DD_VBL();
	}
	_vm->_eventsManager._rateCounter = 0;
	_vm->_eventsManager._escKeyFl = false;
	_vm->_soundManager.LOAD_ANM_SOUND();

	if (_vm->_globals.iRegul == 1) {
		// Do pre-animation delay
		do {
			if (_vm->_eventsManager._escKeyFl)
				goto EXIT;

			_vm->_eventsManager.refreshEvents();
		} while (!_vm->shouldQuit() && _vm->_eventsManager._rateCounter < rate1);
	}

	_vm->_eventsManager._rateCounter = 0;
	frameNumber = 0;
	while (!_vm->shouldQuit()) {
		++frameNumber;
		_vm->_soundManager.playAnim_SOUND(frameNumber);

		// Read frame header
		if (f.read(ptr, 16) != 16)
			break;

		if (strncmp((char *)ptr, "IMAGE=", 6))
			break;

		f.read(screenP, READ_LE_UINT32(ptr + 8));

		if (_vm->_globals.iRegul == 1) {
			do {
				if (_vm->_eventsManager._escKeyFl)
					goto EXIT;

				_vm->_eventsManager.refreshEvents();
				_vm->_soundManager.VERIF_SOUND();
			} while (!_vm->shouldQuit() && _vm->_eventsManager._rateCounter < rate2);
		}

		_vm->_eventsManager._rateCounter = 0;
		_vm->_graphicsManager.lockScreen();
		if (hasScreenCopy) {
			if (*screenP != kByteStop) {
				_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, screenCopy);
				_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
		} else if (*screenP != kByteStop) {
			_vm->_graphicsManager.Copy_Video_Vbe16(screenP);
		}
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.DD_VBL();
		_vm->_soundManager.VERIF_SOUND();
	}

	if (_vm->_globals.iRegul == 1) {
		// Do post-animation delay
		do {
			if (_vm->_eventsManager._escKeyFl)
				break;

			_vm->_eventsManager.refreshEvents();
			_vm->_soundManager.VERIF_SOUND();
		} while (_vm->_eventsManager._rateCounter < rate3);
	}

	_vm->_eventsManager._rateCounter = 0;
	_vm->_soundManager.VERIF_SOUND();
EXIT:
	if (_vm->_graphicsManager.FADE_LINUX == 2 && !hasScreenCopy) {
		screenCopy = _vm->_globals.allocMemory(307200);

		f.seek(0);
		f.skip(6);
		f.read(_vm->_graphicsManager._palette, 800);
		f.skip(4);
		nbytes = f.readUint32LE();
		f.skip(14);
		f.read(screenP, nbytes);

		memcpy(screenCopy, screenP, 307200);

		for (;;) {
			memset(ptr, 0, 20);

			if (f.read(ptr, 16) != 16)
				break;
			if (strncmp((char *)ptr, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(ptr + 8));
			if (*screenP != kByteStop)
				_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, screenCopy);
		}
		_vm->_graphicsManager.fadeOutDefaultLength(screenCopy);
		screenCopy = _vm->_globals.freeMemory(screenCopy);
	}
	if (hasScreenCopy) {
		if (_vm->_graphicsManager.FADE_LINUX == 2)
			_vm->_graphicsManager.fadeOutDefaultLength(screenCopy);
		screenCopy = _vm->_globals.freeMemory(screenCopy);
	}

	_vm->_graphicsManager.FADE_LINUX = 0;
	f.close();
	ptr = _vm->_globals.freeMemory(ptr);
	_vm->_graphicsManager._skipVideoLockFl = false;
}

/**
 * Play Animation, type 2
 */
void AnimationManager::playAnim2(const Common::String &filename, uint32 a2, uint32 a3, uint32 a4) {
	int v5;
	byte *screenCopy = NULL;
	int oldScrollVal = 0;
	byte *screenP = NULL;
	byte *ptr = NULL;
	int frameNumber;
	size_t nbytes;
	byte buf[6];
	Common::File f;

	if (_vm->shouldQuit())
		return;

	bool hasScreenCopy = false;
	while (!_vm->shouldQuit()) {
		memcpy(_vm->_graphicsManager._oldPalette, _vm->_graphicsManager._palette, 769);

		_vm->_fileManager.constructLinuxFilename("TEMP.SCR");

		if (_vm->_graphicsManager._lineNbr == SCREEN_WIDTH)
			_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 307200);
		else if (_vm->_graphicsManager._lineNbr == (SCREEN_WIDTH * 2))
			_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 614400);
		if (!_vm->_graphicsManager._lineNbr)
			_vm->_graphicsManager._scrollOffset = 0;

		screenP = _vm->_graphicsManager._vesaScreen;
		ptr = _vm->_globals.allocMemory(20);
		_vm->_fileManager.constructFilename("ANM", filename);

		if (!f.open(_vm->_globals._curFilename))
			error("Error opening file - %s", _vm->_globals._curFilename.c_str());

		f.read(&buf, 6);
		f.read(_vm->_graphicsManager._palette, 800);
		f.read(&buf, 4);
		nbytes = f.readUint32LE();
		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.read(screenP, nbytes);

		_vm->_graphicsManager.clearPalette();
		oldScrollVal = _vm->_graphicsManager.SCROLL;
		_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH);
		_vm->_graphicsManager.scrollScreen(0);
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.clearScreen();
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.max_x = SCREEN_WIDTH;
		if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
			hasScreenCopy = true;
			screenCopy = _vm->_globals.allocMemory(307200);
			memcpy(screenCopy, screenP, 307200);
		}
		if (NO_SEQ) {
			if (hasScreenCopy)
				memcpy(screenCopy, _vm->_graphicsManager._vesaBuffer, 307200);
			_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
		} else {
			_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
			_vm->_graphicsManager.lockScreen();
			if (hasScreenCopy)
				_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				_vm->_graphicsManager.m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			_vm->_graphicsManager.unlockScreen();
			_vm->_graphicsManager.DD_VBL();
		}
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul != 1)
			break;
		for (;;) {
			if (_vm->_eventsManager._escKeyFl)
				goto LABEL_114;
			_vm->_eventsManager.refreshEvents();
			if (_vm->_eventsManager._rateCounter >= a2)
				goto LABEL_48;
		}
		if (_vm->_graphicsManager._skipVideoLockFl)
			goto LABEL_114;
		if (hasScreenCopy)
			screenCopy = _vm->_globals.freeMemory(screenCopy);
		_vm->_globals.freeMemory(ptr);
		f.close();

		_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
		g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

		memcpy(_vm->_graphicsManager._palette, _vm->_graphicsManager._oldPalette, 769);
		_vm->_graphicsManager.clearPalette();
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.clearScreen();
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.SCROLL = oldScrollVal;
		_vm->_graphicsManager.scrollScreen(oldScrollVal);
		if (_vm->_graphicsManager._largeScreenFl) {
			_vm->_graphicsManager.SCANLINE(1280);
			_vm->_graphicsManager.max_x = 1280;
			_vm->_graphicsManager.lockScreen();
			_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaBuffer, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		} else {
			_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH * 2);
			_vm->_graphicsManager.max_x = SCREEN_WIDTH;
			_vm->_graphicsManager.lockScreen();
			_vm->_graphicsManager.clearScreen();
			_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		}
		_vm->_graphicsManager.unlockScreen();
		_vm->_eventsManager.VBL();
		_vm->_graphicsManager.fadeInShort();
	}
LABEL_48:
	_vm->_eventsManager._rateCounter = 0;
	v5 = 0;
	frameNumber = 0;
	for (;;) {
		++frameNumber;
		_vm->_soundManager.playAnim_SOUND(frameNumber);
		memset(&buf, 0, 6);
		memset(ptr, 0, 19);

		if (f.read(ptr, 16) != 16)
			v5 = -1;

		if (strncmp((const char *)ptr, "IMAGE=", 6))
			v5 = -1;

		if (v5)
			goto LABEL_88;
		f.read(screenP, READ_LE_UINT32(ptr + 8));
		if (_vm->_globals.iRegul == 1)
			break;
LABEL_77:
		_vm->_eventsManager._rateCounter = 0;
		_vm->_graphicsManager.lockScreen();
		if (hasScreenCopy) {
			if (*screenP != kByteStop) {
				_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, screenCopy);
				_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
		} else if (*screenP != kByteStop) {
			_vm->_graphicsManager.Copy_Video_Vbe16(screenP);
		}
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.DD_VBL();
		_vm->_soundManager.VERIF_SOUND();
LABEL_88:
		if (v5 == -1) {
			if (_vm->_globals.iRegul == 1) {
				while (!_vm->_eventsManager._escKeyFl) {
					_vm->_eventsManager.refreshEvents();
					_vm->_soundManager.VERIF_SOUND();
					if (_vm->_eventsManager._rateCounter >= a4)
						goto LABEL_114;
				}
			}
			goto LABEL_114;
		}
	}
	while (!_vm->_eventsManager._escKeyFl) {
		_vm->_eventsManager.refreshEvents();
		_vm->_soundManager.VERIF_SOUND();
		if (_vm->_eventsManager._rateCounter >= a3)
			goto LABEL_77;
	}
LABEL_114:
	_vm->_graphicsManager._skipVideoLockFl = false;
	f.close();

	if (_vm->_graphicsManager.FADE_LINUX == 2 && !hasScreenCopy) {
		byte *ptra;
		ptra = _vm->_globals.allocMemory(307200);

		f.seek(0);
		f.read(&buf, 6);
		f.read(_vm->_graphicsManager._palette, 800);
		f.read(&buf, 4);
		nbytes = f.readUint32LE();

		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.read(screenP, nbytes);
		memcpy(ptra, screenP, 307200);

		int v6 = 0;
		do {
			memset(&buf, 0, 6);
			memset(ptr, 0, 19);
			if (f.read(ptr, 16) != 16)
				v6 = -1;
			if (strncmp((const char *)ptr, "IMAGE=", 6))
				v6 = -1;

			if (!v6) {
				f.read(screenP, READ_LE_UINT32(ptr + 8));
				if (*screenP != kByteStop)
					_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, ptra);
			}
		} while (v6 != -1);
		_vm->_graphicsManager.fadeOutDefaultLength(ptra);
		ptra = _vm->_globals.freeMemory(ptra);
	}
	if (hasScreenCopy) {
		if (_vm->_graphicsManager.FADE_LINUX == 2)
			_vm->_graphicsManager.fadeOutDefaultLength(screenCopy);
		_vm->_globals.freeMemory(screenCopy);
	}
	_vm->_graphicsManager.FADE_LINUX = 0;
	_vm->_globals.freeMemory(ptr);

	_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	memcpy(_vm->_graphicsManager._palette, _vm->_graphicsManager._oldPalette, 769);
	_vm->_graphicsManager.clearPalette();
	_vm->_graphicsManager.lockScreen();
	_vm->_graphicsManager.clearScreen();
	_vm->_graphicsManager.unlockScreen();
	_vm->_graphicsManager.SCROLL = oldScrollVal;
	_vm->_graphicsManager.scrollScreen(oldScrollVal);
	if (_vm->_graphicsManager._largeScreenFl) {
		_vm->_graphicsManager.SCANLINE(1280);
		_vm->_graphicsManager.max_x = 1280;
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaBuffer, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	} else {
		_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH);
		_vm->_graphicsManager.max_x = SCREEN_WIDTH;
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.clearScreen();
		_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	}
	_vm->_graphicsManager.unlockScreen();
	_vm->_graphicsManager.fadeInShort();
	_vm->_graphicsManager.DD_VBL();
}

/**
 * Load Animation
 */
void AnimationManager::loadAnim(const Common::String &animName) {
	byte v20[15];
	char header[10];
	char filename1[15];
	char filename2[15];
	char filename3[15];
	char filename4[15];
	char filename5[15];
	char filename6[15];

	clearAnim();

	Common::String filename = animName + ".ANI";
	_vm->_fileManager.constructFilename("ANIM", filename);

	Common::File f;
	if (!f.open(_vm->_globals._curFilename))
		error("Failed to open %s", _vm->_globals._curFilename.c_str());

	int filesize = f.size();
	int nbytes = filesize - 115;
	f.read(header, 10);
	f.read(v20, 15);
	f.read(filename1, 15);
	f.read(filename2, 15);
	f.read(filename3, 15);
	f.read(filename4, 15);
	f.read(filename5, 15);
	f.read(filename6, 15);

	if (header[0] != 'A' || header[1] != 'N' || header[2] != 'I' || header[3] != 'S')
		error("File incompatible with this soft.");

	const char *files[6] = { &filename1[0], &filename2[0], &filename3[0], &filename4[0],
			&filename5[0], &filename6[0] };

	for (int idx = 0; idx <= 5; ++idx) {
		if (files[idx][0]) {
			_vm->_fileManager.constructFilename("ANIM", files[idx]);

			if (!f.exists(_vm->_globals._curFilename))
				error("File not found");
			if (loadSpriteBank(idx + 1, files[idx]))
				error("File not compatible with this soft.");
		}
	}

	byte *data = _vm->_globals.allocMemory(nbytes + 1);
	f.read(data, nbytes);
	f.close();

	for (int idx = 1; idx <= 20; ++idx)
		searchAnim(data, idx, nbytes);

	_vm->_globals.freeMemory(data);
}

/**
 * Clear animation
 */
void AnimationManager::clearAnim() {
	for (int idx = 0; idx < 35; ++idx) {
		_vm->_globals.Bqe_Anim[idx]._data = _vm->_globals.freeMemory(_vm->_globals.Bqe_Anim[idx]._data);
		_vm->_globals.Bqe_Anim[idx]._enabledFl = false;
	}

	for (int idx = 0; idx < 8; ++idx) {
		_vm->_globals.Bank[idx]._data = _vm->_globals.freeMemory(_vm->_globals.Bank[idx]._data);
		_vm->_globals.Bank[idx]._loadedFl = false;
		_vm->_globals.Bank[idx]._filename = "";
		_vm->_globals.Bank[idx]._fileHeader = 0;
		_vm->_globals.Bank[idx].field1C = 0;
	}
}

/**
 * Load Sprite Bank
 */
int AnimationManager::loadSpriteBank(int idx, const Common::String &filename) {
	byte *v13;
	byte *v19;
	int result = 0;
	_vm->_fileManager.constructFilename("ANIM", filename);
	_vm->_globals.Bank[idx].field1C = _vm->_fileManager.fileSize(_vm->_globals._curFilename);
	_vm->_globals.Bank[idx]._loadedFl = true;
	_vm->_globals.Bank[idx]._filename = filename;

	byte *fileDataPtr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);

	_vm->_globals.Bank[idx]._fileHeader = 0;
	if (fileDataPtr[1] == 'L' && fileDataPtr[2] == 'E')
	    _vm->_globals.Bank[idx]._fileHeader = 1;
	else if (fileDataPtr[1] == 'O' && fileDataPtr[2] == 'R')
		_vm->_globals.Bank[idx]._fileHeader = 2;

	if (_vm->_globals.Bank[idx]._fileHeader) {
		_vm->_globals.Bank[idx]._data = fileDataPtr;

		bool loopCond = false;
		int v8 = 0;
		int width;
		int height;
		do {
			width = _vm->_objectsManager.getWidth(fileDataPtr, v8);
			height = _vm->_objectsManager.getHeight(fileDataPtr, v8);
			if (!width && !height)
				loopCond = true;
			else
				++v8;
			if (v8 > 249)
				loopCond = true;
		} while (!loopCond);

		if (v8 <= 249) {
			_vm->_globals.Bank[idx].field1A = v8;

			Common::String ofsFilename = _vm->_globals.Bank[idx]._filename;
			char ch;
			do {
				ch = ofsFilename.lastChar();
				ofsFilename.deleteLastChar();
			} while (ch != '.');
			ofsFilename += ".OFS";

			_vm->_fileManager.constructFilename("ANIM", ofsFilename);
			Common::File f;
			if (f.exists(_vm->_globals._curFilename)) {
				v19 = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
				v13 = v19;
				for (int objIdx = 0; objIdx < _vm->_globals.Bank[idx].field1A; ++objIdx, v13 += 8) {
					int x1 = (int16)READ_LE_UINT16(v13);
					int y1 = (int16)READ_LE_UINT16(v13 + 2);
					int x2 = (int16)READ_LE_UINT16(v13 + 4);
					int y2 = (int16)READ_LE_UINT16(v13 + 6);

					_vm->_objectsManager.setOffsetXY(_vm->_globals.Bank[idx]._data, objIdx, x1, y1, 0);
					if (_vm->_globals.Bank[idx]._fileHeader == 2)
						_vm->_objectsManager.setOffsetXY(_vm->_globals.Bank[idx]._data, objIdx, x2, y2, 1);
				}

				_vm->_globals.freeMemory(v19);
			}

			result = 0;
		} else {
			_vm->_globals.freeMemory(fileDataPtr);
			_vm->_globals.Bank[idx]._loadedFl = false;
			result = -2;
		}
	} else {
		_vm->_globals.freeMemory(fileDataPtr);
		_vm->_globals.Bank[idx]._loadedFl = false;
		result = -1;
	}

	return result;
}

/**
 * Search Animation
 */
void AnimationManager::searchAnim(const byte *data, int animIndex, int count) {
	int v6;
	int v7;
	byte *v9;
	int v10;
	int v11;
	int v12;
	int v15;
	int v16;
	int v21;
	int v22;
	const byte *v23;
	int v;

	v21 = 0;
	bool loopCond = false;
	do {
		if (data[v21] == 'A' && data[v21 + 1] == 'N' && data[v21 + 2] == 'I' && data[v21 + 3] == 'M') {
			int entryIndex = data[v21 + 4];
			if (animIndex == entryIndex) {
				v6 = v21 + 5;
				v7 = 0;
				bool innerLoopCond = false;
				do {
					if ((data[v6] == 'A' && data[v6 + 1] == 'N' && data[v6 + 2] == 'I' && data[v6 + 3] == 'M') ||
					    (data[v6] == 'F' && data[v6 + 1] == 'I' && data[v6 + 2] == 'N'))
						innerLoopCond = true;
					if (count < v6) {
						_vm->_globals.Bqe_Anim[animIndex]._enabledFl = false;
						_vm->_globals.Bqe_Anim[animIndex]._data = g_PTRNUL;
						return;
					}
					++v6;
					++v7;
				} while (!innerLoopCond);
				_vm->_globals.Bqe_Anim[animIndex]._data = _vm->_globals.allocMemory(v7 + 50);
				_vm->_globals.Bqe_Anim[animIndex]._enabledFl = true;
				memcpy(_vm->_globals.Bqe_Anim[animIndex]._data, v21 + data + 5, 20);

				byte *dataP = _vm->_globals.Bqe_Anim[animIndex]._data;
				v9 = dataP + 20;
				v23 = v21 + data + 25;
				v10 = READ_LE_UINT16(v21 + data + 25);
				v11 = READ_LE_UINT16(v21 + data + 27);
				v22 = READ_LE_UINT16(v21 + data + 29);
				v12 = READ_LE_UINT16(v21 + data + 31);
				WRITE_LE_UINT16(dataP + 20, v10);
				WRITE_LE_UINT16(dataP + 22, v11);
				WRITE_LE_UINT16(dataP + 24, v22);
				WRITE_LE_UINT16(dataP + 26, v12);
				dataP[28] = data[v21 + 33];
				dataP[29] = data[v21 + 34];

				for (int v14 = 1; v14 <= 4999; v14++) {
					v9 += 10;
					v23 += 10;
					if (!v22)
						break;

					v = READ_LE_UINT16(v23);
					v15 = READ_LE_UINT16(v23 + 2);
					v22 = READ_LE_UINT16(v23 + 4);
					v16 = READ_LE_UINT16(v23 + 6);
					WRITE_LE_UINT16(v9, v);
					WRITE_LE_UINT16(v9 + 2, v15);
					WRITE_LE_UINT16(v9 + 4, v22);
					WRITE_LE_UINT16(v9 + 6, v16);
					v9[8] = v23[8];
					v9[9] = v23[9];
				}
				loopCond = true;
			}
		}
		if (data[v21] == 'F' && data[v21 + 1] == 'I' && data[v21 + 2] == 'N')
			loopCond = true;
		++v21;
	} while (v21 <= count && !loopCond);
}

/**
 * Play sequence
 */
void AnimationManager::playSequence(const Common::String &file, uint32 rate1, uint32 rate2, uint32 rate3) {
	bool readError;
	byte *screenCopy = NULL;
	byte *screenP;
	byte *v10;
	int soundNumber;
	size_t nbytes;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	bool hasScreenCopy = false;
	_vm->_eventsManager._mouseFl = false;
	if (!NO_COUL) {
		_vm->_eventsManager.VBL();

		_vm->_fileManager.constructLinuxFilename("TEMP.SCR");
		if (_vm->_graphicsManager._lineNbr == SCREEN_WIDTH)
			_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 307200);
		else if (_vm->_graphicsManager._lineNbr == (SCREEN_WIDTH * 2))
			_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 614400);
		if (!_vm->_graphicsManager._lineNbr)
			_vm->_graphicsManager._scrollOffset = 0;
	}
	screenP = _vm->_graphicsManager._vesaScreen;
	v10 = _vm->_globals.allocMemory(22);
	_vm->_fileManager.constructFilename("SEQ", file);
	if (!f.open(_vm->_globals._curFilename))
		error("Error opening file - %s", _vm->_globals._curFilename.c_str());

	f.skip(6);
	f.read(_vm->_graphicsManager._palette, 800);
	f.skip(4);
	nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
		hasScreenCopy = true;
		screenCopy = _vm->_globals.allocMemory(307200);
		memcpy(screenCopy, screenP, 307200);
	}
	if (NO_SEQ) {
		if (hasScreenCopy)
			memcpy(screenCopy, _vm->_graphicsManager._vesaBuffer, 307200);
		if (!_vm->getIsDemo()) {
			_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
		}
		_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	} else {
		_vm->_graphicsManager.lockScreen();
		if (hasScreenCopy)
			_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.DD_VBL();
	}
	if (_vm->getIsDemo()) {
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul == 1) {
			do {
				if (_vm->_eventsManager._escKeyFl) {
					if (!_vm->_eventsManager._disableEscKeyFl)
						goto LABEL_59;
					_vm->_eventsManager._escKeyFl = false;
				}
				_vm->_eventsManager.refreshEvents();
				_vm->_soundManager.VERIF_SOUND();
			} while (_vm->_eventsManager._rateCounter < rate1);
		}
	} else {
		if (NO_COUL)
			_vm->_graphicsManager.fadeInDefaultLength(screenP);
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul == 1) {
			do {
				if (_vm->_eventsManager._escKeyFl) {
					if (!_vm->_eventsManager._disableEscKeyFl)
						goto LABEL_59;
					_vm->_eventsManager._escKeyFl = false;
				}
				_vm->_eventsManager.refreshEvents();
				_vm->_soundManager.VERIF_SOUND();
			} while (_vm->_eventsManager._rateCounter < rate1);
		}
	}
	_vm->_eventsManager._rateCounter = 0;
	readError = false;
	soundNumber = 0;
	do {
		++soundNumber;
		_vm->_soundManager.playAnim_SOUND(soundNumber);
		memset(v10, 0, 19);
		if (f.read(v10, 16) != 16)
			readError = true;

		if (strncmp((const char *)v10, "IMAGE=", 6))
			readError = true;
		if (!readError) {
			f.read(screenP, READ_LE_UINT32(v10 + 8));
			if (_vm->_globals.iRegul == 1) {
				do {
					if (_vm->_eventsManager._escKeyFl) {
						if (!_vm->_eventsManager._disableEscKeyFl)
							goto LABEL_59;
						_vm->_eventsManager._escKeyFl = false;
					}
					_vm->_eventsManager.refreshEvents();
					_vm->_soundManager.VERIF_SOUND();
				} while (_vm->_eventsManager._rateCounter < rate2);
			}
			_vm->_eventsManager._rateCounter = 0;
			_vm->_graphicsManager.lockScreen();
			if (hasScreenCopy) {
				if (*screenP != kByteStop) {
					_vm->_graphicsManager.Copy_WinScan_Vbe(screenP, screenCopy);
					_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				}
			} else if (*screenP != kByteStop) {
				_vm->_graphicsManager.Copy_Video_Vbe16a(screenP);
			}
			_vm->_graphicsManager.unlockScreen();
			_vm->_graphicsManager.DD_VBL();
			_vm->_soundManager.VERIF_SOUND();
		}
	} while (!readError);

	if (_vm->_globals.iRegul == 1) {
		do {
			if (_vm->_eventsManager._escKeyFl) {
				if (!_vm->_eventsManager._disableEscKeyFl)
					goto LABEL_59;
				_vm->_eventsManager._escKeyFl = false;
			}
			_vm->_eventsManager.refreshEvents();
			_vm->_soundManager.VERIF_SOUND();
		} while (_vm->_eventsManager._rateCounter < rate3);
	}
	_vm->_eventsManager._rateCounter = 0;
LABEL_59:
	_vm->_graphicsManager._skipVideoLockFl = false;
	f.close();

	if (!NO_COUL) {
		_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
		g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

		_vm->_eventsManager._mouseFl = true;
	}
	if (hasScreenCopy)
		_vm->_globals.freeMemory(screenCopy);
	_vm->_globals.freeMemory(v10);
}

/**
 * Play Sequence type 2
 */
void AnimationManager::playSequence2(const Common::String &file, uint32 rate1, uint32 rate2, uint32 rate3) {
	bool v4;
	byte *screenCopy = NULL;
	byte *screenP;
	byte *v11 = NULL;
	int v13;
	size_t nbytes;
	Common::File f;

	bool multiScreenFl = false;
	for (;;) {
		if (_vm->shouldQuit())
			return;

		_vm->_eventsManager._mouseFl = false;
		screenP = _vm->_graphicsManager._vesaScreen;
		v11 = _vm->_globals.allocMemory(22);
		_vm->_fileManager.constructFilename("SEQ", file);

		if (!f.open(_vm->_globals._curFilename))
			error("File not found - %s", _vm->_globals._curFilename.c_str());

		f.skip(6);
		f.read(_vm->_graphicsManager._palette, 800);
		f.skip(4);
		nbytes = f.readUint32LE();
		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.read(screenP, nbytes);

		if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
			multiScreenFl = true;
			screenCopy = _vm->_globals.allocMemory(307200);
			memcpy((void *)screenCopy, screenP, 307200);
		}
		if (NO_SEQ) {
			if (multiScreenFl) {
				assert(screenCopy != NULL);
				memcpy((void *)screenCopy, _vm->_graphicsManager._vesaBuffer, 307200);
			}
			_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
		} else {
			_vm->_graphicsManager.lockScreen();
			_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
			if (multiScreenFl)
				_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				_vm->_graphicsManager.m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			_vm->_graphicsManager.unlockScreen();
			_vm->_graphicsManager.DD_VBL();
		}
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul != 1)
			break;
		while (!_vm->shouldQuit()) {
			if (_vm->_eventsManager._escKeyFl)
				goto LABEL_54;
			_vm->_eventsManager.refreshEvents();
			_vm->_soundManager.VERIF_SOUND();
			if (_vm->_eventsManager._rateCounter >= rate1)
				goto LABEL_23;
		}
		if (_vm->_graphicsManager._skipVideoLockFl)
			goto LABEL_54;
		if (multiScreenFl)
			screenCopy = _vm->_globals.freeMemory(screenCopy);
		_vm->_globals.freeMemory(v11);
		f.close();
	}
LABEL_23:
	_vm->_eventsManager._rateCounter = 0;
	v4 = false;
	v13 = 0;
	while (!_vm->shouldQuit()) {
		_vm->_soundManager.playAnim_SOUND(v13++);

		memset(v11, 0, 19);
		if (f.read(v11, 16) != 16)
			v4 = true;

		if (strncmp((const char *)v11, "IMAGE=", 6))
			v4 = true;
		if (v4)
			goto LABEL_44;
		f.read(screenP, READ_LE_UINT32(v11 + 8));
		if (_vm->_globals.iRegul == 1)
			break;
LABEL_33:
		_vm->_eventsManager._rateCounter = 0;
		_vm->_graphicsManager.lockScreen();
		if (multiScreenFl) {
			if (*screenP != kByteStop) {
				_vm->_graphicsManager.Copy_WinScan_Vbe(screenP, screenCopy);
				_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
		} else if (*screenP != kByteStop) {
			_vm->_graphicsManager.Copy_Video_Vbe16a(screenP);
		}
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.DD_VBL();
		_vm->_soundManager.VERIF_SOUND();
LABEL_44:
		if (v4) {
			if (_vm->_globals.iRegul == 1) {
				while (!_vm->_eventsManager._escKeyFl) {
					_vm->_eventsManager.refreshEvents();
					_vm->_soundManager.VERIF_SOUND();
					if (_vm->_eventsManager._rateCounter >= rate3) {
						_vm->_eventsManager._rateCounter = 0;
						break;
					}
				}
			} else {
				_vm->_eventsManager._rateCounter = 0;
			}
			goto LABEL_54;
		}
	}
	while (!_vm->_eventsManager._escKeyFl) {
		_vm->_eventsManager.refreshEvents();
		if (_vm->_eventsManager._rateCounter >= rate2)
			goto LABEL_33;
	}
LABEL_54:
	if (_vm->_graphicsManager.FADE_LINUX == 2 && !multiScreenFl) {
		byte *ptra = _vm->_globals.allocMemory(307200);

		f.seek(0);
		f.skip(6);
		f.read(_vm->_graphicsManager._palette, 800);
		f.skip(4);
		nbytes = f.readUint32LE();

		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.read(screenP, nbytes);

		memcpy(ptra, screenP, 307200);
		for (;;) {
			memset(v11, 0, 19);
			if (f.read(v11, 16) != 16)
				break;

			if (strncmp((const char *)v11, "IMAGE=", 6))
				break;

			f.read(screenP, READ_LE_UINT32(v11 + 8));
			if (*screenP != kByteStop)
				_vm->_graphicsManager.Copy_WinScan_Vbe(screenP, ptra);
		}
		_vm->_graphicsManager.fadeOutDefaultLength(ptra);
		ptra = _vm->_globals.freeMemory(ptra);
	}
	if (multiScreenFl) {
		if (_vm->_graphicsManager.FADE_LINUX == 2)
			_vm->_graphicsManager.fadeOutDefaultLength(screenCopy);
		_vm->_globals.freeMemory(screenCopy);
	}
	_vm->_graphicsManager.FADE_LINUX = 0;

	f.close();
	_vm->_globals.freeMemory(v11);
	_vm->_eventsManager._mouseFl = true;
}

} // End of namespace Hopkins
