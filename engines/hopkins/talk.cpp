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
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/talk.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

namespace Hopkins {

TalkManager::TalkManager() {
	_characterBuffer = NULL;
	_characterPalette = NULL;
	_characterSprite = NULL;
	_characterAnim = NULL;
	_characterSize = 0;
	STATI = 0;
	PLIGNE1 = PLIGNE2 = 0;
	PLIGNE3 = PLIGNE4 = 0;
	_paletteBufferIdx = 0;
}

void TalkManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void TalkManager::PARLER_PERSO(const Common::String &filename) {
	Common::String spriteFilename;

	int answer = 0;
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(9);
	_vm->_eventsManager.VBL();
	_vm->_graphicsManager.no_scroll = 1;
	bool oldDisableInventFl = _vm->_globals._disableInventFl;
	_vm->_globals._disableInventFl = true;
	_characterBuffer = _vm->_fileManager.searchCat(filename, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, filename);
		_characterBuffer = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		_characterSize = _vm->_fileManager.fileSize(_vm->_globals._curFilename);
	}
	_vm->_globals._saveData->data[svField4] = 0;
	getStringFromBuffer(40, spriteFilename, (const char *)_characterBuffer);
	getStringFromBuffer(0, _questionsFilename, (const char *)_characterBuffer);
	getStringFromBuffer(20, _answersFilename, (const char *)_characterBuffer);
	if (_vm->_globals._language == LANG_FR) {
		_answersFilename = _questionsFilename = "RUE.TXT";
	} else if (_vm->_globals._language == LANG_EN) {
		_answersFilename = _questionsFilename = "RUEAN.TXT";
	} else if (_vm->_globals._language == LANG_SP) {
		_answersFilename = _questionsFilename = "RUEES.TXT";
	}
	PLIGNE1 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 40);
	_paletteBufferIdx = 20 * (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) + 110;
	_characterSprite = _vm->_fileManager.searchCat(spriteFilename, 7);
	if (_characterSprite) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, spriteFilename);
	} else {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, "RES_SAN.RES");
	}
	_characterSprite = _vm->_objectsManager.loadSprite(_vm->_globals._curFilename);

	_vm->_fileManager.constructLinuxFilename("TEMP.SCR");
	if (_vm->_graphicsManager._lineNbr == SCREEN_WIDTH)
		_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 307200);
	else if (_vm->_graphicsManager._lineNbr == (SCREEN_WIDTH * 2))
		_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 614400);

	if (!_vm->_graphicsManager._lineNbr)
		_vm->_graphicsManager._scrollOffset = 0;
	_vm->_graphicsManager.NB_SCREEN(true);
	_vm->_objectsManager.PERSO_ON = true;
	searchCharacterPalette(_paletteBufferIdx, false);
	startCharacterAnim0(_paletteBufferIdx, false);
	initCharacterAnim();
	PLIGNE2 = PLIGNE1 + 1;
	PLIGNE3 = PLIGNE1 + 2;
	PLIGNE4 = PLIGNE1 + 3;
	int v14 = _vm->_eventsManager._mouseCursorId;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);
	if (!_vm->_globals.NOPARLE) {
		int v5;
		do {
			v5 = DIALOGUE();
			if (v5 != PLIGNE4)
				answer = DIALOGUE_REP(v5);
			if (answer == -1)
				v5 = PLIGNE4;
			_vm->_eventsManager.VBL();
		} while (v5 != PLIGNE4);
	}
	if (_vm->_globals.NOPARLE) {
		int v6 = 1;
		int v7;
		do
			v7 = DIALOGUE_REP(v6++);
		while (v7 != -1);
	}
	clearCharacterAnim();
	_vm->_globals.NOPARLE = false;
	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_characterSprite = _vm->_globals.freeMemory(_characterSprite);
	_vm->_graphicsManager.NB_SCREEN(false);

	_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	_vm->_objectsManager.PERSO_ON = false;
	_vm->_eventsManager._mouseCursorId = v14;

	_vm->_eventsManager.changeMouseCursor(v14);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);

	if (_vm->getIsDemo() == false)
		_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	_vm->_graphicsManager.initColorTable(145, 150, _vm->_graphicsManager._palette);
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	_vm->_graphicsManager.lockScreen();
	_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	_vm->_graphicsManager.unlockScreen();
	memcpy(_vm->_graphicsManager._vesaBuffer, _vm->_graphicsManager._vesaScreen, 614399);
	_vm->_globals._disableInventFl = oldDisableInventFl;
	_vm->_graphicsManager.DD_VBL();
	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager.VBL();
	_vm->_graphicsManager.no_scroll = 0;
}

void TalkManager::PARLER_PERSO2(const Common::String &filename) {
	_vm->_objectsManager._disableCursorFl = true;
	STATI = 1;
	bool v7 = _vm->_globals._disableInventFl;
	_vm->_globals._disableInventFl = true;
	_characterBuffer = _vm->_fileManager.searchCat(filename, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, filename);
		_characterBuffer = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		_characterSize = _vm->_fileManager.fileSize(_vm->_globals._curFilename);
	}

	_vm->_globals._saveData->data[svField4] = 0;
	getStringFromBuffer(0, _questionsFilename, (const char *)_characterBuffer);
	getStringFromBuffer(20, _answersFilename, (const char *)_characterBuffer);

	switch (_vm->_globals._language) {
	case LANG_EN:
		_questionsFilename = "RUEAN.TXT";
		_answersFilename = "RUEAN.TXT";
		break;
	case LANG_FR:
		_questionsFilename = "RUE.TXT";
		_answersFilename = "RUE.TXT";
		break;
	case LANG_SP:
		_questionsFilename = "RUEES.TXT";
		_answersFilename = "RUEES.TXT";
		break;
	}

	PLIGNE1 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 40);
	_paletteBufferIdx = 20 * (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) + 110;
	searchCharacterPalette(_paletteBufferIdx, false);
	PLIGNE2 = PLIGNE1 + 1;
	PLIGNE3 = PLIGNE1 + 2;
	PLIGNE4 = PLIGNE1 + 3;
	int v8 = _vm->_eventsManager._mouseCursorId;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);

	if (!_vm->_globals.NOPARLE) {
		int v3;
		do {
			v3 = DIALOGUE();
			if (v3 != PLIGNE4) {
				if (DIALOGUE_REP(v3) == -1)
					v3 = PLIGNE4;
			}
		} while (v3 != PLIGNE4);
	}

	if (_vm->_globals.NOPARLE) {
		int v4 = 1;
		int v5;
	    do
			v5 = DIALOGUE_REP(v4++);
		while (v5 != -1);
	}

	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_vm->_eventsManager._mouseCursorId = v8;

	_vm->_eventsManager.changeMouseCursor(v8);
	_vm->_graphicsManager.initColorTable(145, 150, _vm->_graphicsManager._palette);
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	_vm->_objectsManager._disableCursorFl = false;
	_vm->_globals._disableInventFl = v7;
	STATI = 0;
}

void TalkManager::getStringFromBuffer(int srcStart, Common::String &dest, const char *srcData) {
	dest = Common::String(srcData + srcStart);
}

int TalkManager::DIALOGUE() {
	byte *v0;
	int v1;
	int v2;
	int v3;
	int v4;
	int v5;
	int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	byte *v11;
	int v12;
	int v13;
	int v14;
	int v15;
	int v16;
	int v18;
	int v19;
	int v20;
	int v21;

	if (STATI) {
		v0 = _characterBuffer;
		v1 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 48);
		if (v1)
			_vm->_objectsManager.setBobAnimation(v1);
		if ((int16)READ_LE_UINT16((uint16 *)v0 + 48) != 1)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16((uint16 *)v0 + 49));
		if ((int16)READ_LE_UINT16((uint16 *)v0 + 48) != 2)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16((uint16 *)v0 + 50));
		if ( (int16)READ_LE_UINT16((uint16 *)v0 + 48) != 3)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16((uint16 *)v0 + 51));
		if ((int16)READ_LE_UINT16((uint16 *)v0 + 48) != 4)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16((uint16 *)v0 + 52));
	} else {
		dialogWait();
	}

	v19 = VERIF_BOITE(PLIGNE1, _questionsFilename, 65);
	v2 = VERIF_BOITE(PLIGNE2, _questionsFilename, 65);
	v3 = VERIF_BOITE(PLIGNE3, _questionsFilename, 65);
	v20 = 420 - 20 * VERIF_BOITE(PLIGNE4, _questionsFilename, 65);
	v21 = v20 - 20 * v3;
	v18 = v20 - 20 * v3 - 1;
	v4 = v20 - 20 * v3 - 20 * v2;
	_vm->_fontManager.initTextBuffers(5, PLIGNE1, _questionsFilename, 5, v4 - 20 * v19, 0, 65, 255);
	_vm->_fontManager.initTextBuffers(6, PLIGNE2, _questionsFilename, 5, v4, 0, 65, 255);
	_vm->_fontManager.initTextBuffers(7, PLIGNE3, _questionsFilename, 5, v21, 0, 65, 255);
	_vm->_fontManager.initTextBuffers(8, PLIGNE4, _questionsFilename, 5, v20, 0, 65, 255);
	_vm->_fontManager.showText(5);
	_vm->_fontManager.showText(6);
	_vm->_fontManager.showText(7);
	_vm->_fontManager.showText(8);

	v5 = -1;
	v6 = 0;
  	do {
		v7 = _vm->_eventsManager.getMouseY();
		if ((v4 - 20 * v19) < v7 && (v4 - 1) > v7) {
			v8 = v7;
			_vm->_fontManager.setOptimalColor(6, 7, 8, 5);
			v5 = PLIGNE1;
			v7 = v8;
		}
		if (v7 > v4 && v18 > v7) {
			v9 = v7;
			_vm->_fontManager.setOptimalColor(5, 7, 8, 6);
			v5 = PLIGNE2;
			v7 = v9;
		}
		if (v21 < v7 && (v20 - 1) > v7) {
			v10 = v7;
			_vm->_fontManager.setOptimalColor(5, 6, 8, 7);
			v5 = PLIGNE3;
			v7 = v10;
		}
		if (v20 < v7 && v7 < 419) {
			_vm->_fontManager.setOptimalColor(5, 6, 7, 8);
			v5 = PLIGNE4;
		}

		_vm->_eventsManager.VBL();
		if (_vm->_eventsManager.getMouseButton())
			v6 = 1;
		if (v5 == -1)
			v6 = 0;
	} while (!_vm->shouldQuit() && v6 != 1);

	_vm->_soundManager.mixVoice(v5, 1);
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(6);
	_vm->_fontManager.hideText(7);
	_vm->_fontManager.hideText(8);

	if (STATI) {
		v11 = _characterBuffer;
		v12 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 48);
		if (v12)
			_vm->_objectsManager.stopBobAnimation(v12);
		v13 = (int16)READ_LE_UINT16((uint16 *)v11 + 49);
		if (v13 != 1)
			_vm->_objectsManager.stopBobAnimation(v13);
		v14 = (int16)READ_LE_UINT16((uint16 *)v11 + 50);
		if (v14 != 2)
			_vm->_objectsManager.stopBobAnimation(v14);
		v15 = (int16)READ_LE_UINT16((uint16 *)v11 + 51);
		if (v15 != 3)
			_vm->_objectsManager.stopBobAnimation(v15);
		v16 = (int16)READ_LE_UINT16((uint16 *)v11 + 52);
		if (v16 != 4)
			_vm->_objectsManager.stopBobAnimation(v16);
	} else {
		dialogTalk();
	}

	_vm->_eventsManager.VBL();
  return v5;
}

int TalkManager::DIALOGUE_REP(int idx) {
	int v1;
	int v2;
	byte *v3;
	int v6;
	int v7;
	byte *v8;
	int v9;
	int v10;
	int v11;
	int v12;
	int v13;
	int v14;
	void *v15;
	int v16;
	int v17;
	int v18;
	int v19;
	int v20;
	int v21;
	int v22;
	int v23;
	int v24;
	int v25;

	v1 = 0;
	v2 = 0;
	v3 = _characterBuffer + 110;
	for (; (int16)READ_LE_UINT16(v3) != idx; v3 = _characterBuffer + 20 * v1 + 110) {
		++v1;
		if ((int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) < v1)
			v2 = 1;
		if (v2 == 1)
			return -1;
	}
	if (v2 == 1)
		return -1;

	v22 = (int16)READ_LE_UINT16((uint16 *)v3 + 1);
	v25 = (int16)READ_LE_UINT16((uint16 *)v3 + 2);
	v24 = (int16)READ_LE_UINT16((uint16 *)v3 + 3);
	v23 = (int16)READ_LE_UINT16((uint16 *)v3 + 4);
	PLIGNE1 = (int16)READ_LE_UINT16((uint16 *)v3 + 5);
	PLIGNE2 = (int16)READ_LE_UINT16((uint16 *)v3 + 6);
	PLIGNE3 = (int16)READ_LE_UINT16((uint16 *)v3 + 7);
	v6 = (int16)READ_LE_UINT16((uint16 *)v3 + 8);
	v7 = (int16)READ_LE_UINT16((uint16 *)v3 + 9);

	if (v7)
		_vm->_globals._saveData->data[svField4] = v7;

	if (!v6)
		v6 = 10;
	if (STATI) {
		v8 = _characterBuffer;
		v9 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 43);
		if (v9)
			_vm->_objectsManager.setBobAnimation(v9);
		v10 = (int16)READ_LE_UINT16((uint16 *)v8 + 44);
		if (v10)
			_vm->_objectsManager.setBobAnimation(v10);
		v11 = (int16)READ_LE_UINT16((uint16 *)v8 + 45);
		if (v11)
			_vm->_objectsManager.setBobAnimation(v11);
		v12 = (int16)READ_LE_UINT16((uint16 *)v8 + 46);
		if (v12)
			_vm->_objectsManager.setBobAnimation(v12);
		v13 = (int16)READ_LE_UINT16((uint16 *)v8 + 47);
		if (v13)
			_vm->_objectsManager.setBobAnimation(v13);
	} else {
		VISU_PARLE();
	}

	if (!_vm->_soundManager._textOffFl) {
		_vm->_fontManager.initTextBuffers(9, v22, _answersFilename, v25, v24, 5, v23, 252);
		_vm->_fontManager.showText(9);
	}
	if (!_vm->_soundManager.mixVoice(v22, 1)) {
		v14 = 0;
		_vm->_eventsManager._curMouseButton = 0;
		_vm->_eventsManager._mouseButton = 0;

		if (_vm->getIsDemo()) {
			do {
				_vm->_eventsManager.VBL();
				++v14;
			} while (v14 != v6);
		} else {
			int tmpVal = 0;
			do {
				_vm->_eventsManager.VBL();
				++v14;
				if (_vm->_eventsManager._mouseButton || _vm->_eventsManager._curMouseButton)
					v14 = v6;
				if (_vm->_eventsManager.getMouseButton()) {
					tmpVal = v6 / 5;
				if (tmpVal < 0)
					tmpVal = -tmpVal;
				if (v14 > tmpVal)
					v14 = v6;
				}
			} while (v14 != v6);
		}
	}

	if (!_vm->_soundManager._textOffFl)
		_vm->_fontManager.hideText(9);
	if (STATI) {
		v15 = _characterBuffer;
		v16 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 43);
		if (v16)
			_vm->_objectsManager.stopBobAnimation(v16);
		v17 = (int16)READ_LE_UINT16((uint16 *)v15 + 44);
		if (v17)
			_vm->_objectsManager.stopBobAnimation(v17);
		v18 = (int16)READ_LE_UINT16((uint16 *)v15 + 45);
		if (v18)
			_vm->_objectsManager.stopBobAnimation(v18);
		v19 = (int16)READ_LE_UINT16((uint16 *)v15 + 46);
		if (v19)
			_vm->_objectsManager.stopBobAnimation(v19);
		v20 = (int16)READ_LE_UINT16((uint16 *)v15 + 47);
		if (v20)
			_vm->_objectsManager.stopBobAnimation(v20);
	} else {
		dialogEndTalk();
	}
	v21 = 0;
	if (!PLIGNE1)
		v21 = -1;

	return v21;
}

void TalkManager::searchCharacterPalette(int startIdx, bool dark) {
	int palettePos = 0;
	size_t curIdx = startIdx;
	for (;;) {
		if (_characterBuffer[curIdx] == 'P' && _characterBuffer[curIdx + 1] == 'A' && _characterBuffer[curIdx + 2] == 'L') {
			palettePos = curIdx;
			break;
		}
		++curIdx;
		if (_characterSize == curIdx)
			return;
	}

	_characterPalette = _characterBuffer + palettePos + 5;
	_characterPalette[0] = 0;
	_characterPalette[1] = 0;
	_characterPalette[2] = 0;
	_characterPalette[759] = 255;
	_characterPalette[760] = 255;
	_characterPalette[762] = 0;
	_characterPalette[763] = 0;
	_characterPalette[764] = 0;
	_characterPalette[765] = 224;
	_characterPalette[766] = 224;
	_characterPalette[767] = 255;

	if (!dark)
		_characterPalette[761] = 86;
	else
		_characterPalette[761] = 255;

	_vm->_graphicsManager.setPaletteVGA256(_characterPalette);
	_vm->_graphicsManager.initColorTable(145, 150, _characterPalette);
}

void TalkManager::dialogWait() {
	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::dialogTalk() {
	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_OFF(idx);
	}

	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_ZERO(idx);
	}
}

void TalkManager::dialogEndTalk() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_OFF(idx);
	}

	_vm->_eventsManager.VBL();
	_vm->_eventsManager.VBL();

	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_ZERO(idx);
	}
}

int TalkManager::VERIF_BOITE(int idx, const Common::String &file, int a3) {
	char v8;
	int v9;
	int v10;
	char v11;
	char v13;
	char v14;
	int v15;
	byte *ptr;
	int v17;
	int v18;
	byte *v19;
	uint32 indexData[4047];
	Common::String filename;
	Common::String dest;
	Common::File f;
	int filesize;

	v18 = 0;
	_vm->_globals.police_l = 11;
	_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, file);

	// Build up the filename
	filename = dest = _vm->_globals._curFilename;
	while (filename.lastChar() != '.')
		filename.deleteLastChar();
	filename += "IND";

	if (!f.open(filename))
		error("Could not open file - %s", filename.c_str());
	filesize = f.size();
	assert(filesize < 16188);

	for (int i = 0; i < (filesize / 4); ++i)
		indexData[i] = f.readUint32LE();
	f.close();

	if (!f.open(dest))
		error("Error opening file - %s", dest.c_str());

	f.seek(indexData[idx]);
	ptr = _vm->_globals.allocMemory(2058);
	if (ptr == g_PTRNUL)
		error("temporary TEXT");
	f.read(ptr, 2048);
	f.close();

	v19 = ptr;
	for (int i = 0; i < 2048; i++) {
		v13 = *v19;
		if ((byte)(*v19 + 46) > 27) {
			if ((byte)(v13 + 80) > 27) {
				if ((byte)(v13 - 65) <= 25 || (byte)(v13 - 97) <= 25)
					v13 = 32;
			} else {
				v13 -= 79;
			}
		} else {
			v13 = *v19 + 111;
		}
		*v19 = v13;
		v19 = v19 + 1;
	}

	for (int i = 0; i < 2048; i++) {
		v8 = ptr[i];
		if ( v8 == 10 || v8 == 13 )
			ptr[i] = 0;
	}

	v9 = 0;
	v15 = (11 * a3) - 4;
	do {
		v10 = 0;
		for (;;) {
			v17 = v10;
			do {
				v11 = ptr[v9 + v10];
				v14 = v11;
				++v10;
			} while (v11 != ' ' && v11 != '%');

			if (v10 >= v15 / _vm->_globals.police_l)
				break;
			if (v11 == '%') {
				if (v10 < v15 / _vm->_globals.police_l)
					goto LABEL_31;
				break;
			}
		}
		if (v11 != '%')
			goto LABEL_33;
		v14 = ' ';
LABEL_31:
		if (v14 == '%')
			v17 = v10;
LABEL_33:
		++v18;
		v9 += v17;
	} while (v14 != 37);
	free(ptr);
	return v18;
}

void TalkManager::VISU_PARLE() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::BOB_VISU_PARLE(int idx) {
	int v4;
	byte *v5;

	_vm->_objectsManager._priorityFl = true;
	if (!_vm->_globals._bob[idx].field0) {
		_vm->_objectsManager.BOB_ZERO(idx);
		v5 = _vm->_globals.Bqe_Anim[idx]._data;
		v4 = (int16)READ_LE_UINT16(v5 + 2);
		if (!v4)
			v4 = 1;
		if ((int16)READ_LE_UINT16(v5 + 24)) {
			_vm->_globals._bob[idx]._isSpriteFl = true;
			_vm->_globals._bob[idx].field36 = 0;
			_vm->_globals._bob[idx]._modeFlag = 0;
			_vm->_globals._bob[idx]._animData = _vm->_globals.Bqe_Anim[idx]._data;
			_vm->_globals._bob[idx].field0 = 10;
			v5 = _characterSprite;
			_vm->_globals._bob[idx]._spriteData = _characterSprite;
			_vm->_globals._bob[idx].field1E = v4;
			_vm->_globals._bob[idx].field20 = -1;
			_vm->_globals._bob[idx].field22 = 0;
			_vm->_globals._bob[idx]._offsetY = 0;
		}
	}
}

void TalkManager::startCharacterAnim0(int startIdx, bool readOnlyFl) {
	int animIdx = 0;
	size_t curIdx = startIdx;
	for (;;) {
		if (_characterBuffer[curIdx] == 'A' && _characterBuffer[curIdx + 1] == 'N' && _characterBuffer[curIdx + 2] == 'I' && _characterBuffer[curIdx + 3] == 'M' && _characterBuffer[curIdx + 4] == 1) {
			animIdx = curIdx;
			break;
		}
		++curIdx;
		if (_characterSize == curIdx)
			return;
	}
	_characterAnim = _characterBuffer + animIdx + 25;
	if (!readOnlyFl) {
		int idx = 0;
		int v7;
		do {
			v7 = (int16)READ_LE_UINT16(&_characterAnim[2 * idx + 4]);
			if (v7 && _vm->_globals._speed != 501)
				_vm->_graphicsManager.fastDisplay(_characterSprite, _vm->_eventsManager._startPos.x + (int16)READ_LE_UINT16(&_characterAnim[2 * idx]),
				    (int16)READ_LE_UINT16(&_characterAnim[2 * idx + 2]), _characterAnim[2 * idx + 8]);
			idx += 5;
		} while (_vm->_globals._speed != 501 && v7);
	}
}

/**
 * Initialize character animation
 */
void TalkManager::initCharacterAnim() {
	byte *v0;
	byte *v1;
	int v2;
	int v4;
	int v5;
	int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	int v11;
	int v12;

	v0 = _characterBuffer;
	v1 = _characterBuffer + 110;
	v2 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 43);
	if (v2)
		searchCharacterAnim(21, (_characterBuffer + 110), v2, _characterSize);
	v4 = (int16)READ_LE_UINT16((uint16 *)v0 + 44);
	if (v4)
		searchCharacterAnim(22, v1, v4, _characterSize);
	v5 = (int16)READ_LE_UINT16((uint16 *)v0 + 45);
	if (v5)
		searchCharacterAnim(23, v1, v5, _characterSize);
	v6 = (int16)READ_LE_UINT16((uint16 *)v0 + 46);
	if (v6)
		searchCharacterAnim(24, v1, v6, _characterSize);
	v7 = (int16)READ_LE_UINT16((uint16 *)v0 + 47);
	if (v7)
		searchCharacterAnim(25, v1, v7, _characterSize);
	v8 = (int16)READ_LE_UINT16((uint16 *)v0 + 48);
	if (v8)
		searchCharacterAnim(26, v1, v8, _characterSize);
	v9 = (int16)READ_LE_UINT16((uint16 *)v0 + 49);
	if (v9)
		searchCharacterAnim(27, v1, v9, _characterSize);
	v10 = (int16)READ_LE_UINT16((uint16 *)v0 + 50);
	if (v10)
		searchCharacterAnim(28, v1, v10, _characterSize);
	v11 = (int16)READ_LE_UINT16((uint16 *)v0 + 51);
	if (v11)
		searchCharacterAnim(29, v1, v11, _characterSize);
	v12 = (int16)READ_LE_UINT16((uint16 *)v0 + 52);
	if (v12)
		searchCharacterAnim(30, v1, v12, _characterSize);
}

void TalkManager::clearCharacterAnim() {
	for (int idx = 21; idx <= 34; ++idx) {
		_vm->_globals.Bqe_Anim[idx]._data = _vm->_globals.freeMemory(_vm->_globals.Bqe_Anim[idx]._data);
		_vm->_globals.Bqe_Anim[idx]._enabledFl = false;
	}
}

bool TalkManager::searchCharacterAnim(int idx, const byte *bufPerso, int a3, int a4) {
	bool result;
	const byte *v5;
	int v6;
	int v7;
	byte *v8;
	byte *v9;
	int v10;
	int v11;
	int v12;
	int v14;
	int v15;
	int v16;
	int v18;
	int v22;
	int v23;
	const byte *v24;

	v22 = 0;
	result = false;

	do {
		if (bufPerso[v22] == 'A' && bufPerso[v22 + 1] == 'N' && bufPerso[v22 + 2] == 'I' && bufPerso[v22 + 3] == 'M' && bufPerso[v22 + 4] == a3) {
			v5 = v22 + bufPerso + 5;
			v18 = v22 + 5;
			v6 = 0;
			v7 = 0;
			do {
				if ((v5[0] == 'A' && v5[1] == 'N' && v5[2] == 'I' && v5[3] == 'M') ||
				    (v5[0] == 'F' && v5[1] == 'I' && v5[2] == 'N'))
					v7 = 1;
				if (v18 > a4) {
					_vm->_globals.Bqe_Anim[idx]._enabledFl = false;
					result = false;
					_vm->_globals.Bqe_Anim[idx]._data = g_PTRNUL;
				}
				++v18;
				++v6;
				++v5;
			} while (v7 != 1);
			_vm->_globals.Bqe_Anim[idx]._data = _vm->_globals.allocMemory(v6 + 50);
			_vm->_globals.Bqe_Anim[idx]._enabledFl = true;
			memcpy(_vm->_globals.Bqe_Anim[idx]._data, (const byte *)(v22 + bufPerso + 5), 20);
			v8 = _vm->_globals.Bqe_Anim[idx]._data;

			v9 = v8 + 20;
			v24 = v22 + bufPerso + 25;
			v10 = (int16)READ_LE_UINT16(v22 + bufPerso + 25);
			v11 = (int16)READ_LE_UINT16(v22 + bufPerso + 27);
			v23 = (int16)READ_LE_UINT16(v22 + bufPerso + 29);
			v12 = (int16)READ_LE_UINT16(v22 + bufPerso + 31);
			v8[28] = bufPerso[v22 + 33];
			v8[29] = bufPerso[v22 + 34];
			WRITE_LE_UINT16(v8 + 20, v10);
			WRITE_LE_UINT16(v8 + 22, v11);
			WRITE_LE_UINT16(v8 + 24, v23);
			WRITE_LE_UINT16(v8 + 26, v12);
			for (int i = 1; i < 5000; i++) {
				v9 += 10;
				v24 += 10;
				if (!v23)
					break;
				v14 = (int16)READ_LE_UINT16(v24);
				v15 = (int16)READ_LE_UINT16(v24 + 2);
				v23 = (int16)READ_LE_UINT16(v24 + 4);
				v16 = (int16)READ_LE_UINT16(v24 + 6);
				v9[8] = v24[8];
				v9[9] = v24[9];
				WRITE_LE_UINT16(v9, v14);
				WRITE_LE_UINT16(v9 + 2, v15);
				WRITE_LE_UINT16(v9 + 4, v23);
				WRITE_LE_UINT16(v9 + 6, v16);
			}
			result = true;
		}
		if (bufPerso[v22] == 'F' && bufPerso[v22 + 1] == 'I' && bufPerso[v22 + 2] == 'N')
			result = true;
		++v22;
	} while (v22 <= a4 && !result);

	return result;
}

void TalkManager::REPONSE(int a1, int a2) {
	byte v2;
	byte v3;
	byte *v5;
	byte *v6;
	uint16 v7;
	byte *v8;
	int v10;
	uint16 v11;
	int v12;
	int v13;
	int v15;
	int v16;
	bool loopCond;
	byte *ptr;

	v2 = a1;
	v3 = a2;
LABEL_2:
	v15 = 0;
	if (_vm->_globals.COUCOU != g_PTRNUL) {
		v5 = _vm->_globals.COUCOU;
		for (;;) {
			if (v5[0] == 'C' && v5[1] == 'O' && v5[2] == 'D') {
				if (v5[3] == v2 && v5[4] == v3)
					v15 = 1;
			}
			if (v5[0] == 'F' && v5[1] == 'I' && v5[2] == 'N')
				break;
			if (!v15)
				v5 = v5 + 1;
			if (v15 == 1) {
				v6 = v5 + 5;
				ptr = _vm->_globals.allocMemory(620);
				if (g_PTRNUL == ptr)
					error("TRADUC");
				memset(ptr, 0, 620);
				v7 = 0;
				v12 = 0;
				loopCond = false;
				do {
					v16 = 0;
					if (v6[v7] == 'F' && v6[v7 + 1] == 'C') {
						++v12;
						assert(v12 < (620 / 20));

						v8 = (ptr + 20 * v12);
						v11 = 0;
						do {
							assert(v11 < 20);
							v8[v11++] = v6[v7++];
							if (v6[v7] == 'F' && v6[v7 + 1] == 'F') {
								v16 = 1;
								v8[v11] = 'F';
								v8[v11 + 1] = 'F';
								++v7;
							}
						} while (v16 != 1);
					}
					if (v16 != 1) {
						if ((v6[v7] == 'C' && v6[v7 + 1] == 'O' && v6[v7 + 2] == 'D') || (v6[v7] == 'F' && v6[v7 + 1] == 'I' && v6[v7 + 2] == 'N'))
							loopCond = true;
					}
					v6 += v7 + 1;
					v7 = 0;
				} while (!loopCond);
				loopCond = false;
				v13 = 1;
				do {
					v10 = _vm->_scriptManager.handleOpcode(ptr + 20 * v13);
					if (_vm->shouldQuit())
						return;

					if (v10 == 2)
						v13 =  _vm->_scriptManager.handleGoto(ptr + 20 * v13);
					else if (v10 == 3)
						v13 =  _vm->_scriptManager.handleIf(ptr, v13);

					if (v13 == -1)
						error("Invalid IFF function");

					if (v10 == 1 || v10 == 4)
						++v13;
					else if (!v10 || v10 == 5)
						loopCond = true;
					else if (v10 == 6) {
						_vm->_globals.freeMemory(ptr);
						v2 = _vm->_objectsManager.NVZONE;
						v3 = _vm->_objectsManager.NVVERBE;
						goto LABEL_2;
					}
				} while (!loopCond);
				_vm->_globals.freeMemory(ptr);
				_vm->_globals._saveData->data[svField2] = 0;
				return;
			}
		}
	}
}

void TalkManager::REPONSE2(int a1, int a2) {
	int indx = 0;
	if (a2 == 5 && _vm->_globals._saveData->data[svField3] == 4) {
		if (a1 == 22 || a1 == 23) {
			_vm->_objectsManager.setFlipSprite(0, false);
			_vm->_objectsManager.setSpriteIndex(0, 62);
			_vm->_objectsManager.SPACTION(_vm->_globals._forestSprite, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, 0);
			if (a1 == 22) {
				_vm->_objectsManager.lockAnimX(6, _vm->_objectsManager.getBobPosX(3));
				_vm->_objectsManager.lockAnimX(8, _vm->_objectsManager.getBobPosX(3));
			} else { // a1 == 23
				_vm->_objectsManager.lockAnimX(6, _vm->_objectsManager.getBobPosX(4));
				_vm->_objectsManager.lockAnimX(8, _vm->_objectsManager.getBobPosX(4));
			}
			_vm->_objectsManager.stopBobAnimation(3);
			_vm->_objectsManager.stopBobAnimation(4);
			_vm->_objectsManager.setBobAnimation(6);
			_vm->_soundManager.PLAY_SAMPLE2(1);
			_vm->_objectsManager.SPACTION1(_vm->_globals._forestSprite, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
			do
				_vm->_eventsManager.VBL();
			while (_vm->_objectsManager.BOBPOSI(6) < 12);
			_vm->_objectsManager.stopBobAnimation(6);
			_vm->_objectsManager.setBobAnimation(8);
			
			switch (_vm->_globals._screenId) {
			case 35:
				indx = 201;
				break;
			case 36:
				indx = 203;
				break;
			case 37:
				indx = 205;
				break;
			case 38:
				indx = 207;
				break;
			case 39:
				indx = 209;
				break;
			case 40:
				indx = 211;
				break;
			case 41:
				indx = 213;
				break;
			}
			_vm->_globals._saveData->data[indx] = 2;
			_vm->_objectsManager.disableZone(22);
			_vm->_objectsManager.disableZone(23);
		} else if (a1 == 20 || a1 == 21) {
			_vm->_objectsManager.setFlipSprite(0, true);
			_vm->_objectsManager.setSpriteIndex(0, 62);
			_vm->_objectsManager.SPACTION(_vm->_globals._forestSprite, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, 1);
			if (a1 == 20) {
				_vm->_objectsManager.lockAnimX(5, _vm->_objectsManager.getBobPosX(1));
				_vm->_objectsManager.lockAnimX(7, _vm->_objectsManager.getBobPosX(1));
			} else { // a1 == 21
				_vm->_objectsManager.lockAnimX(5, _vm->_objectsManager.getBobPosX(2));
				_vm->_objectsManager.lockAnimX(7, _vm->_objectsManager.getBobPosX(2));
			}
			_vm->_objectsManager.stopBobAnimation(1);
			_vm->_objectsManager.stopBobAnimation(2);
			_vm->_objectsManager.setBobAnimation(5);
			_vm->_soundManager.PLAY_SAMPLE2(1);
			_vm->_objectsManager.SPACTION1(_vm->_globals._forestSprite, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
			do
				_vm->_eventsManager.VBL();
			while (_vm->_objectsManager.BOBPOSI(5) < 12);
			_vm->_objectsManager.stopBobAnimation(5);
			_vm->_objectsManager.setBobAnimation(7);
			switch (_vm->_globals._screenId) {
			case 35:
				indx = 200;
				break;
			case 36:
				indx = 202;
				break;
			case 37:
				indx = 204;
				break;
			case 38:
				indx = 206;
				break;
			case 39:
				indx = 208;
				break;
			case 40:
				indx = 210;
				break;
			case 41:
				indx = 212;
				break;
			}
			_vm->_globals._saveData->data[indx] = 2;
			_vm->_objectsManager.disableZone(21);
			_vm->_objectsManager.disableZone(20);
		}
	}
}

void TalkManager::OBJET_VIVANT(const Common::String &a2) {
	byte *v11;
	Common::String s;
	Common::String v20;
	Common::String v22;
	Common::String v23;

	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(9);
	_vm->_eventsManager.VBL();
	_vm->_graphicsManager.no_scroll = 1;
	_vm->_linesManager.CLEAR_ZONE();
	_vm->_linesManager.RESET_OBSTACLE();
	_vm->_globals.resetCache();
	
	for (int i = 0; i <= 44; i++)
		_vm->_globals.BOBZONE[i] = 0;

	_vm->_objectsManager.NUMZONE = -1;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);
	_characterBuffer = _vm->_fileManager.searchCat(a2, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, a2);
		_characterBuffer = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		_characterSize = _vm->_fileManager.fileSize(_vm->_globals._curFilename);
	}
	getStringFromBuffer(40, v23, (const char *)_characterBuffer);
	getStringFromBuffer(0, v22, (const char *)_characterBuffer);
	getStringFromBuffer(20, v20, (const char *)_characterBuffer);

	if (v20 == "NULL")
		v20 = Common::String::format("IM%d", _vm->_globals._screenId);

	_characterSprite = _vm->_fileManager.searchCat(v23, 7);
	if (_characterSprite)
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, v23);
	else
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, "RES_SAN.RES");

	_characterSprite = _vm->_objectsManager.loadSprite(_vm->_globals._curFilename);

	_vm->_fileManager.constructLinuxFilename("TEMP.SCR");
	if (_vm->_graphicsManager._lineNbr == SCREEN_WIDTH)
		_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 307200);
	else if (_vm->_graphicsManager._lineNbr == (SCREEN_WIDTH * 2))
		_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, _vm->_graphicsManager._vesaScreen, 614400);

	if (!_vm->_graphicsManager._lineNbr)
		_vm->_graphicsManager._scrollOffset = 0;
	_vm->_graphicsManager.NB_SCREEN(true);
	_paletteBufferIdx = 20 * (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) + 110;
	_vm->_graphicsManager.NB_SCREEN(true);
	_vm->_objectsManager.PERSO_ON = true;
	searchCharacterPalette(_paletteBufferIdx, true);
	startCharacterAnim0(_paletteBufferIdx, false);
	v11 = _vm->_globals.COUCOU;
	_vm->_globals.COUCOU = g_PTRNUL;
	_vm->_globals.NOMARCHE = true;
	_vm->_objectsManager.INILINK(v22);
	_vm->_objectsManager.PERSO_ON = true;
	_vm->_globals.GOACTION = false;
	_vm->_objectsManager.NUMZONE = -1;
	initCharacterAnim();
	VISU_PARLE();
	dialogWait();
	_vm->_graphicsManager.INI_ECRAN2(v22);
	_vm->_globals.NOMARCHE = true;
	_vm->_objectsManager._forceZoneFl = true;
	_vm->_objectsManager.NUMZONE = -1;
	do {
		int mouseButton = _vm->_eventsManager.getMouseButton();
		if (mouseButton == 1)
			_vm->_objectsManager.handleLeftButton();
		else if (mouseButton == 2)
			_vm->_objectsManager.handleRightButton();

		_vm->_objectsManager.checkZone();
		if (_vm->_globals.GOACTION)
			_vm->_objectsManager.PARADISE();
		_vm->_eventsManager.VBL();
	} while (!_vm->_globals._exitId);
	dialogEndTalk();
	dialogTalk();
	clearCharacterAnim();
	clearCharacterAnim();
	_vm->_globals.NOPARLE = false;
	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_characterSprite = _vm->_globals.freeMemory(_characterSprite);
	_vm->_graphicsManager.NB_SCREEN(false);
	_vm->_linesManager.CLEAR_ZONE();
	_vm->_linesManager.RESET_OBSTACLE();
	_vm->_globals.resetCache();
	for (int i = 0; i <= 44; i++)
		_vm->_globals.BOBZONE[i] = 0;

	_vm->_globals.freeMemory(_vm->_globals.COUCOU);
	_vm->_globals.COUCOU = v11;
	_vm->_objectsManager._disableFl = true;
	_vm->_objectsManager.INILINK(v20);
	_vm->_graphicsManager.INI_ECRAN2(v20);
	_vm->_objectsManager._disableFl = false;
	_vm->_globals.NOMARCHE = false;
	if (_vm->_globals._exitId == 101)
		_vm->_globals._exitId = 0;

	_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	_vm->_objectsManager.PERSO_ON = false;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(4);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);

	if (!_vm->getIsDemo())
		_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	_vm->_graphicsManager.initColorTable(145, 150, _vm->_graphicsManager._palette);
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	_vm->_graphicsManager.lockScreen();
	_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	_vm->_graphicsManager.unlockScreen();
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	memcpy(_vm->_graphicsManager._vesaBuffer, _vm->_graphicsManager._vesaScreen, 614399);
	_vm->_globals._disableInventFl = false;
	_vm->_graphicsManager.DD_VBL();
	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager.VBL();
	_vm->_graphicsManager.no_scroll = 0;
}

} // End of namespace Hopkins
