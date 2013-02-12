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
#include "hopkins/objects.h"
#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/sound.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

ScriptManager::ScriptManager() {
	TRAVAILOBJET = false;
}

void ScriptManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

int ScriptManager::handleOpcode(byte *dataP) {
	if (READ_BE_UINT16(dataP) != MKTAG16('F', 'C'))
		return 0;

	int opcodeType = 0;
	int vbobFrameIndex = 0;

	uint32 signature24 = READ_BE_UINT24(&dataP[2]);
	switch (signature24) {
	case MKTAG24('T', 'X', 'T'): {
		vbobFrameIndex = dataP[6];
		int mesgId = (int16)READ_LE_UINT16(dataP + 13);
		opcodeType = 1;
		if (!TRAVAILOBJET) {
			if (_vm->_globals._saveData->_data[svField356] == 1) {
				if (mesgId == 53)
					mesgId = 644;
				if (mesgId == 624)
					mesgId = 639;
				if (mesgId == 627)
					mesgId = 630;
				if (mesgId == 625)
					mesgId = 639;
				if (mesgId == 8)
					mesgId = 637;
				if (mesgId == 53)
					mesgId = 644;
				if (mesgId == 557)
					mesgId = 636;
				if (mesgId == 51)
					mesgId = 644;
				if (mesgId == 287)
					mesgId = 636;
				if (mesgId == 619)
					mesgId = 633;
				if (mesgId == 620)
					mesgId = 634;
				if (mesgId == 622)
					mesgId = 644;
				if (mesgId == 297)
					mesgId = 636;
				if (mesgId == 612 || mesgId == 613 || mesgId == 614 || mesgId == 134)
					mesgId = 636;
				if (mesgId == 615)
					mesgId = 635;
				if (mesgId == 618)
					mesgId = 632;
				if (mesgId == 611)
					mesgId = 642;
				if (mesgId == 610)
					mesgId = 641;
				if (mesgId == 18)
					mesgId = 643;
				if (mesgId == 602)
					mesgId = 645;
				if (mesgId == 603)
					mesgId = 646;
				if (mesgId == 604)
					mesgId = 647;
				if (mesgId == 51)
					mesgId = 644;
				if (mesgId == 607)
					mesgId = 650;
				if (mesgId == 605)
					mesgId = 648;
				if (mesgId == 606)
					mesgId = 649;
				if (mesgId == 601)
					mesgId = 652;
				if (mesgId == 37)
					mesgId = 636;
				if (mesgId == 595)
					mesgId = 633;
				if (mesgId == 596)
					mesgId = 634;
				if (mesgId == 532)
					mesgId = 636;
				if (mesgId == 599)
					mesgId = 636;
				if (mesgId == 363)
					mesgId = 636;
			}
			if (!_vm->_soundManager._soundOffFl && _vm->_soundManager._soundFl) {
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game
					_vm->_eventsManager.VBL();
				} while (_vm->_soundManager._soundFl);
			}
			if (!_vm->_soundManager._textOffFl) {
				int textPosX = (int16)READ_LE_UINT16(dataP + 9);
				int textPosY = (int16)READ_LE_UINT16(dataP + 11);
				_vm->_fontManager.initTextBuffers(9, mesgId, _vm->_globals.FICH_TEXTE, 2 * textPosX, 2 * textPosY + 40, 6, dataP[7], 253);
				if (!_vm->_soundManager._textOffFl)
					_vm->_fontManager.showText(9);
			}
			if (!_vm->_soundManager._voiceOffFl)
				_vm->_soundManager.mixVoice(mesgId, 4);
		} else { // if (TRAVAILOBJET)
			if (_vm->_globals._saveData->_data[svField356]) {
				_vm->_fontManager.initTextBuffers(9, 635, _vm->_globals.FICH_TEXTE, 55, 20, dataP[8], 35, 253);
				if (!_vm->_soundManager._textOffFl)
					_vm->_fontManager.showText(9);
				if (!_vm->_soundManager._voiceOffFl)
					_vm->_soundManager.mixVoice(635, 4);
			} else {
				int textPosX = (int16)READ_LE_UINT16(dataP + 9);
				if (_vm->_globals._language == LANG_FR && !_vm->_soundManager._textOffFl)
					_vm->_fontManager.initTextBuffers(9, mesgId, "OBJET1.TXT", 2 * textPosX, 60, 6, dataP[7], 253);
				else if (_vm->_globals._language == LANG_EN && !_vm->_soundManager._textOffFl)
					_vm->_fontManager.initTextBuffers(9, mesgId, "OBJETAN.TXT", 2 * textPosX, 60, 6, dataP[7], 253);
				else if (_vm->_globals._language == LANG_SP && !_vm->_soundManager._textOffFl) {
					_vm->_fontManager.initTextBuffers(9, mesgId, "OBJETES.TXT", 2 * textPosX, 60, 6, dataP[7], 253);
				}

				if (!_vm->_soundManager._textOffFl)
					_vm->_fontManager.showText(9);

				if (!_vm->_soundManager._voiceOffFl)
					_vm->_soundManager.mixVoice(mesgId, 5);
			}
		}
		break;
		}
	case MKTAG24('B', 'O', 'B'):
		if (!_vm->_objectsManager._disableFl) {
			int vbobIdx = dataP[5];
			vbobFrameIndex = dataP[6];
			int v4 = dataP[7];
			int vbobPosX = (int16)READ_LE_UINT16(dataP + 8);
			int vbobPosY = (int16)READ_LE_UINT16(dataP + 10);
			if (vbobIdx == 52) {
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, vbobPosX, (int16)READ_LE_UINT16(dataP + 10), vbobFrameIndex);
			} else if (vbobIdx == 51) {
				_vm->_objectsManager.BOB_VIVANT(vbobFrameIndex);
			} else if (vbobIdx != 50) {
				_vm->_objectsManager.VBOB(_vm->_globals.SPRITE_ECRAN, vbobIdx, vbobPosX, vbobPosY, vbobFrameIndex);
				if (v4)
					v4 /= _vm->_globals._speed;
				if (v4 > 1) {
					do {
						if (_vm->shouldQuit())
							return -1; // Exiting game

						--v4;
						_vm->_eventsManager.VBL();
					} while (v4);
				}
			}
		}
		opcodeType = 1;
		break;
	case MKTAG24('S', 'T', 'P'):
			if (!_vm->_objectsManager._disableFl) {
				_vm->_objectsManager._twoCharactersFl = false;
				_vm->_objectsManager._characterPos.x = (int16)READ_LE_UINT16(dataP + 6);
				_vm->_objectsManager._characterPos.y = (int16)READ_LE_UINT16(dataP + 8);
				_vm->_objectsManager._startSpriteIndex = dataP[5];
				if (_vm->_objectsManager._changeHeadFl) {
					if (_vm->_globals._saveData->_data[svField354] == 1
							&& _vm->_globals._saveData->_cloneHopkins._pos.x && _vm->_globals._saveData->_cloneHopkins._pos.y
							&& _vm->_globals._saveData->_cloneHopkins._startSpriteIndex && _vm->_globals._saveData->_cloneHopkins._location) {

						_vm->_objectsManager._characterPos = _vm->_globals._saveData->_cloneHopkins._pos;
						_vm->_objectsManager._startSpriteIndex = _vm->_globals._saveData->_cloneHopkins._startSpriteIndex;
					}
					if (_vm->_globals._saveData->_data[svField356] == 1
							&& _vm->_globals._saveData->_samantha._pos.x && _vm->_globals._saveData->_samantha._pos.y
							&& _vm->_globals._saveData->_samantha._startSpriteIndex && _vm->_globals._saveData->_samantha._location) {
						_vm->_objectsManager._characterPos = _vm->_globals._saveData->_samantha._pos;
						_vm->_objectsManager._startSpriteIndex = _vm->_globals._saveData->_samantha._startSpriteIndex;
					}
					if (_vm->_globals._saveData->_data[svField357] == 1
							&& _vm->_globals._saveData->_realHopkins._pos.x && _vm->_globals._saveData->_realHopkins._pos.y
							&& _vm->_globals._saveData->_realHopkins._startSpriteIndex && _vm->_globals._saveData->_realHopkins._location) {
						_vm->_objectsManager._characterPos = _vm->_globals._saveData->_realHopkins._pos;
						_vm->_objectsManager._startSpriteIndex = _vm->_globals._saveData->_realHopkins._startSpriteIndex;
					}
				}
				if (_vm->_globals._saveData->_data[svField356] == 1
						&& _vm->_globals._saveData->_realHopkins._location == _vm->_globals._screenId) {
					_vm->_objectsManager.addStaticSprite(
					    _vm->_globals.TETE,
					    _vm->_globals._saveData->_realHopkins._pos,
					    1,
					    2,
					    _vm->_globals._saveData->_realHopkins._zoomFactor,
					    false,
					    34,
					    190);
					_vm->_objectsManager.animateSprite(1);
					_vm->_objectsManager._twoCharactersFl = true;
				}
				if (_vm->_globals._saveData->_data[svField357] == 1
				        && _vm->_globals._saveData->_data[svField355] == 1
				        && _vm->_globals._saveData->_samantha._location == _vm->_globals._screenId) {
					_vm->_objectsManager.addStaticSprite(
					    _vm->_globals.TETE,
					    _vm->_globals._saveData->_samantha._pos,
					    1,
					    3,
					    _vm->_globals._saveData->_samantha._zoomFactor,
					    false,
					    20,
					    127);
					_vm->_objectsManager.animateSprite(1);
					_vm->_objectsManager._twoCharactersFl = true;
				}
			}
			opcodeType = 1;
			_vm->_objectsManager._changeHeadFl = false;
		break;
	case MKTAG24('S', 'T', 'E'):
		if (!_vm->_objectsManager._disableFl) {
			_vm->_globals._prevScreenId = _vm->_globals._screenId;
			_vm->_globals._saveData->_data[svField6] = _vm->_globals._screenId;
			_vm->_globals._screenId = _vm->_globals._saveData->_data[svField5] = dataP[5];
			vbobFrameIndex = dataP[6];
		}
		opcodeType = 1;
		break;
	case MKTAG24('B', 'O', 'F'):
		if (!_vm->_objectsManager._disableFl)
			_vm->_objectsManager.VBOB_OFF((int16)READ_LE_UINT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('P', 'E', 'R'): {
		int specialOpcode = (int16)READ_LE_UINT16(dataP + 5);
		if (!_vm->_globals._saveData->_data[svField122] && !_vm->_globals._saveData->_data[svField356]) {
			vbobFrameIndex = 0;

			switch (specialOpcode) {
			case 1:
			case 14:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(4);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(4);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(4);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(4);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(4);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(4);
				break;
			case 2:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(7);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(7);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(7);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(7);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(7);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(7);
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(8);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(8);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(8);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(8);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(8);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(8);
				break;
			case 19:
			case 4:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(1);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(1);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(1);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(1);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(1);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(1);
				break;
			case 5:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(5);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(5);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(5);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(5);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(5);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(5);
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(6);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(6);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(6);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(6);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(6);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(6);
				break;
			case 17:
			case 7:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(2);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(2);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(2);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(2);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(2);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(2);
				break;
			case 18:
			case 8:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(3);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(3);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(3);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(3);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(3);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(3);
				break;
			case 9:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(5);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(5);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(5);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(5);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(5);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(5);
				break;
			case 10:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(6);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(6);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(6);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(6);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(6);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(6);
				break;
			case 15:
			case 11:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(7);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(7);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(7);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(7);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(7);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(7);
				break;
			case 16:
			case 12:
				if (_vm->_globals._actionDirection == 1)
					_vm->_objectsManager.ACTION_DOS(8);
				if (_vm->_globals._actionDirection == 3)
					_vm->_objectsManager.ACTION_DROITE(8);
				if (_vm->_globals._actionDirection == 2)
					_vm->_objectsManager.Q_DROITE(8);
				if (_vm->_globals._actionDirection == 5)
					_vm->_objectsManager.ACTION_FACE(8);
				if (_vm->_globals._actionDirection == 8)
					_vm->_objectsManager.Q_GAUCHE(8);
				if (_vm->_globals._actionDirection == 7)
					_vm->_objectsManager.ACTION_GAUCHE(8);
				break;
			}
		}
		opcodeType = 1;
		break;
		}
	case MKTAG24('M', 'U', 'S'):
		opcodeType = 1;
		break;
	case MKTAG24('W', 'A', 'I'): {
		uint frameNumb = READ_LE_UINT16(dataP + 5) / _vm->_globals._speed;
		if (!frameNumb)
			frameNumb = 1;
		for (uint i = 0; i < frameNumb + 1; i++) {
			if (_vm->shouldQuit())
				return -1; // Exiting game

			_vm->_eventsManager.VBL();
		}
		opcodeType = 1;
		break;
		}
	case MKTAG24('O', 'B', 'P'):
		opcodeType = 1;
		_vm->_objectsManager.addObject((int16)READ_LE_UINT16(dataP + 5));
		break;
	case MKTAG24('O', 'B', 'M'):
		opcodeType = 1;
		_vm->_objectsManager.removeObject((int16)READ_LE_UINT16(dataP + 5));
		break;
	case MKTAG24('G', 'O', 'T'):
		opcodeType = 2;
		break;
	case MKTAG24('Z', 'O', 'N'):
		_vm->_linesManager.enableZone((int16)READ_LE_UINT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('Z', 'O', 'F'):
		_vm->_linesManager.disableZone((int16)READ_LE_UINT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('E', 'X', 'I'):
		opcodeType = 5;
		break;
	case MKTAG24('S', 'O', 'R'):
		_vm->_globals._exitId = (int16)READ_LE_UINT16(dataP + 5);
		opcodeType = 5;
		break;
	case MKTAG24('B', 'C', 'A'):
		_vm->_globals.B_CACHE_OFF((int16)READ_LE_UINT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('A', 'N', 'I'): {
		int animId = (int16)READ_LE_UINT16(dataP + 5);
		if (animId <= 100)
			_vm->_objectsManager.setBobAnimation(animId);
		else
			_vm->_objectsManager.stopBobAnimation(animId - 100);
		opcodeType = 1;
		break;
		}
	case MKTAG24('S', 'P', 'E'):
		switch ((int16)READ_LE_UINT16(dataP + 5)) {
		case 6:
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.OPTI_ONE(20, 0, 14, 4);
			break;

		case 7:
			_vm->_talkManager.PARLER_PERSO("rueh1.pe2");
			break;

		case 8:
			_vm->_talkManager.PARLER_PERSO("ruef1.pe2");
			break;

		case 10:
			_vm->_talkManager.PARLER_PERSO("bqeflic1.pe2");
			break;

		case 11:
			_vm->_talkManager.PARLER_PERSO("bqeflic2.pe2");
			break;

		case 12:
			_vm->_fontManager.hideText(9);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_talkManager.PARLER_PERSO("bqetueur.pe2");
			break;

		case 13:
			_vm->_eventsManager._mouseButton = _vm->_eventsManager._curMouseButton;
			_vm->_globals._disableInventFl = true;
			_vm->_graphicsManager.fadeOutLong();
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.removeSprite(0);
			_vm->_fontManager.hideText(5);
			_vm->_fontManager.hideText(9);
			_vm->_graphicsManager.FIN_VISU();
			_vm->_objectsManager.clearScreen();

			if ((_vm->getPlatform() == Common::kPlatformWindows) && _vm->getIsDemo()) {
				_vm->_graphicsManager.fadeOutLong();
			} else {
				_vm->_soundManager.playSound("SOUND17.WAV");
				_vm->_graphicsManager.FADE_LINUX = 2;
				_vm->_animationManager.playSequence2("HELICO.SEQ", 10, 4, 10);
			}

			_vm->_animationManager.loadAnim("otage");
			_vm->_graphicsManager.loadImage("IM05");
			_vm->_graphicsManager.VISU_ALL();

			for (int i = 0; i <= 4; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}

			_vm->_eventsManager.mouseOff();
			_vm->_graphicsManager.fadeInDefaultLength(_vm->_graphicsManager._vesaBuffer);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(3) != 100);
			_vm->_graphicsManager.fadeOutDefaultLength(_vm->_graphicsManager._vesaBuffer);
			_vm->_graphicsManager.FIN_VISU();

			// If uncensored, rip the throat of the hostage
			if (!_vm->_globals._censorshipFl) {
				_vm->_soundManager._specialSoundNum = 16;
				_vm->_graphicsManager.FADE_LINUX = 2;
				_vm->_animationManager.playAnim("EGORGE.ANM", 50, 28, 500);
				_vm->_soundManager._specialSoundNum = 0;
			}
			_vm->_animationManager.loadAnim("ASCEN");
			_vm->_eventsManager.mouseOff();
			_vm->_graphicsManager.loadImage("ASCEN");
			_vm->_graphicsManager.VISU_ALL();

			for (int i = 0; i <= 4; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}

			_vm->_eventsManager.mouseOff();
			_vm->_graphicsManager.fadeInDefaultLength(_vm->_graphicsManager._vesaBuffer);
			_vm->_objectsManager.SCI_OPTI_ONE(1, 0, 17, 3);
			_vm->_graphicsManager.fadeOutDefaultLength(_vm->_graphicsManager._vesaBuffer);
			_vm->_graphicsManager.FIN_VISU();

			if ((_vm->getPlatform() == Common::kPlatformWindows) && _vm->getIsDemo())
				_vm->_soundManager.playSound("SOUND17.WAV");

			_vm->_soundManager._specialSoundNum = 14;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.playSequence2("ASSOM.SEQ", 10, 4, 500);
			_vm->_soundManager._specialSoundNum = 0;

			if ((_vm->getPlatform() == Common::kPlatformWindows) && _vm->getIsDemo())
				_vm->_graphicsManager.fadeOutLong();

			_vm->_globals._disableInventFl = false;
			_vm->_objectsManager._helicopterFl = true;
			break;

		case 16:
			_vm->_talkManager.PARLER_PERSO("ftoubib.pe2");
			break;

		case 17:
			_vm->_talkManager.PARLER_PERSO("flic2b.pe2");
			break;

		case 18:
			_vm->_talkManager.PARLER_PERSO("fjour.pe2");
			break;

		case 20:
			_vm->_talkManager.PARLER_PERSO("PUNK.pe2");
			break;

		case 21:
			_vm->_talkManager.PARLER_PERSO("MEDLEG.pe2");
			break;

		case 22:
			_vm->_talkManager.OBJET_VIVANT("CADAVRE1.pe2");
			break;

		case 23:
			_vm->_talkManager.PARLER_PERSO2("CHERCHE1.pe2");
			break;

		case 25:
			_vm->_talkManager.PARLER_PERSO("AGENT1.pe2");
			break;

		case 26:
			_vm->_talkManager.PARLER_PERSO("AGENT2.pe2");
			break;

		case 27:
			if (_vm->_globals._saveData->_data[svField94] != 1 || _vm->_globals._saveData->_data[svField95] != 1)
				_vm->_talkManager.PARLER_PERSO("STANDAR.pe2");
			else
				_vm->_talkManager.PARLER_PERSO("STANDAR1.pe2");
			break;

		case 29:
			_vm->_globals._disableInventFl = true;
			_vm->_talkManager.OBJET_VIVANT("TELEP.pe2");
			_vm->_globals._disableInventFl = false;
			break;

		case 32:
			_vm->_talkManager.PARLER_PERSO("SAMAN.pe2");
			break;

		case 35:
			if (!_vm->_soundManager._soundOffFl) {
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					_vm->_eventsManager.VBL();
				} while (_vm->_soundManager._soundFl);
			}
			_vm->_talkManager.PARLER_PERSO("PTLAB.pe2");
			break;

		case 36:
			if (_vm->_globals._saveData->_data[svField270] == 2 && _vm->_globals._saveData->_data[svField94] == 1 && _vm->_globals._saveData->_data[svField95] == 1)
				_vm->_globals._saveData->_data[svField270] = 3;
			if (!_vm->_globals._saveData->_data[svField270])
				_vm->_talkManager.PARLER_PERSO2("PATRON0.pe2");
			if (_vm->_globals._saveData->_data[svField270] == 1)
				_vm->_talkManager.PARLER_PERSO2("PATRON1.pe2");
			if (_vm->_globals._saveData->_data[svField270] == 2)
				_vm->_talkManager.PARLER_PERSO2("PATRON2.pe2");
			if (_vm->_globals._saveData->_data[svField270] == 3)
				_vm->_talkManager.PARLER_PERSO2("PATRON3.pe2");
			if (_vm->_globals._saveData->_data[svField270] > 3) {
				_vm->_talkManager.PARLER_PERSO2("PATRON4.pe2");
				_vm->_globals._saveData->_data[svField270] = 5;
			}
			break;

		case 37:
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.playSequence2("corde.SEQ", 32, 32, 100);
			_vm->_graphicsManager._noFadingFl = true;
			break;

		case 38:
			_vm->_soundManager.loadSample(1, "SOUND44.WAV");
			_vm->_soundManager.loadSample(2, "SOUND42.WAV");
			_vm->_soundManager.loadSample(3, "SOUND41.WAV");
			_vm->_soundManager._specialSoundNum = 17;
			_vm->_animationManager.playSequence("grenade.SEQ", 1, 32, 100);
			_vm->_soundManager._specialSoundNum = 0;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.playAnim("CREVE17.ANM", 24, 24, 200);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_soundManager.DEL_SAMPLE(2);
			_vm->_soundManager.DEL_SAMPLE(3);
			_vm->_graphicsManager._noFadingFl = true;
			break;

		case 40:
			_vm->_talkManager.PARLER_PERSO("MAGE.pe2");
			break;

		case 41:
			_vm->_talkManager.PARLER_PERSO("MORT3.pe2");
			break;

		case 42:
			_vm->_talkManager.PARLER_PERSO("MORT2.pe2");
			break;

		case 43:
			_vm->_talkManager.PARLER_PERSO("MORT1.pe2");
			break;

		case 44:
			_vm->_talkManager.PARLER_PERSO("MORT3A.pe2");
			break;

		case 45:
			_vm->_talkManager.PARLER_PERSO("FEM3.pe2");
			break;

		case 46: {
			_vm->_globals.NOT_VERIF = true;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 564, 420);
			_vm->_objectsManager._zoneNum = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_objectsManager.removeSprite(0);
			_vm->_globals.NOT_VERIF = true;
			_vm->_soundManager.loadSample(1, "SOUND44.WAV");
			_vm->_soundManager.loadSample(2, "SOUND45.WAV");
			_vm->_objectsManager.OPTI_BOBON(9, 10, -1, 0, 0, 0);
			bool v15 = false;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 4 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 5)
					v15 = false;
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 16 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 17)
					v15 = false;
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 28 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 29)
					v15 = false;
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 10 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 11)
					v15 = false;
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 22 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 23)
					v15 = false;
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 33 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 34)
					v15 = false;
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 12)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 249, 1);
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 23)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 249, 2);
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 34)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 249, 3);
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(9) != 36);
			_vm->_objectsManager.animateSprite(0);
			_vm->_objectsManager.stopBobAnimation(9);
			_vm->_objectsManager.stopBobAnimation(10);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_soundManager.DEL_SAMPLE(2);
			break;
			}

		case 47:
			_vm->_talkManager.PARLER_PERSO("BARMAN.pe2");
			break;

		case 48:
			_vm->_talkManager.PARLER_PERSO("SAMAN2.pe2");
			break;

		case 49: {
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.OPTI_BOBON(9, 10, -1, 0, 0, 0);
			int v19 = 12;
			if (_vm->_globals._saveData->_data[svField133] == 1)
				v19 = 41;
			int v20 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 4 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND44.WAV");
					v20 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 5)
					v20 = 0;
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 18 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND46.WAV");
					v20 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(9) == 19)
					v20 = 0;
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 11 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND45.WAV");
					v20 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(10) == 12)
					v20 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(9) != v19);
			if (v19 == 12) {
				_vm->_objectsManager.animateSprite(0);
				_vm->_objectsManager.stopBobAnimation(9);
			}
			_vm->_globals.CACHE_ON();
			break;
			}

		case 50:
			_vm->_soundManager.playSound("SOUND46.WAv");
			_vm->_objectsManager.OPTI_ONE(11, 0, 23, 0);
			break;

		case 51: {
			_vm->_graphicsManager.fadeOutLong();
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.removeSprite(0);
			_vm->_fontManager.hideText(5);
			_vm->_fontManager.hideText(9);
			_vm->_graphicsManager.FIN_VISU();
			_vm->_graphicsManager.loadImage("IM20f");
			_vm->_animationManager.loadAnim("ANIM20f");
			_vm->_graphicsManager.VISU_ALL();
			_vm->_eventsManager.mouseOff();
			_vm->_graphicsManager.fadeInLong();
			bool v52 = false;
			_vm->_soundManager.loadWav("SOUND46.WAV", 1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(12) == 5 && !v52) {
					_vm->_soundManager.playWav(1);
					v52 = true;
				}
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(12) != 34);
			_vm->_objectsManager.stopBobAnimation(2);
			_vm->_graphicsManager.fadeOutLong();
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_globals._exitId = 20;
			break;
			}

		case 52:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("GARDE.PE2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 53:
			_vm->_talkManager.PARLER_PERSO("GARDE1.pe2");
			break;

		case 54:
			_vm->_talkManager.PARLER_PERSO("GARDE2.pe2");
			break;

		case 55:
			_vm->_objectsManager.stopBobAnimation(1);
			_vm->_objectsManager.OPTI_ONE(15, 0, 12, 0);
			_vm->_objectsManager.stopBobAnimation(15);
			_vm->_objectsManager.OBSSEUL = true;
			_vm->_objectsManager.INILINK("IM19a");
			_vm->_objectsManager.OBSSEUL = false;
			break;

		case 56:
			_vm->_globals.PERSO = _vm->_fileManager.loadFile("HOPFEM.SPR");
			_vm->_globals.PERSO_TYPE = 1;
			_vm->_globals._saveData->_data[svField122] = 1;
			_vm->_globals.loadCharacterData();
			_vm->_objectsManager._sprite[0].field12 = 28;
			_vm->_objectsManager._sprite[0].field14 = 155;
			_vm->_objectsManager.computeAndSetSpriteSize();
			break;

		case 57:
			_vm->_globals.PERSO = _vm->_fileManager.loadFile("PERSO.SPR");
			_vm->_globals.PERSO_TYPE = 0;
			_vm->_globals._saveData->_data[svField122] = 0;
			_vm->_globals.loadCharacterData();
			_vm->_objectsManager._sprite[0].field12 = 34;
			_vm->_objectsManager._sprite[0].field14 = 190;
			_vm->_objectsManager.computeAndSetSpriteSize();
			break;

		case 58:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("Gm1.PE2");
			_vm->_globals._saveData->_data[svField176] = 1;
			_vm->_globals._saveData->_data[svField270] = 2;
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 59: {
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._oldCharacterPosX = _vm->_objectsManager.getSpriteX(0);
			_vm->_globals._oldDirection = -1;
			_vm->_globals.Compteur = 0;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 445, 332);
			_vm->_globals.NOT_VERIF = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(7);
			_vm->_objectsManager.setBobAnimDataIdx(7, 0);
			int v18 = 0;
			_vm->_soundManager.loadSample(1, "SOUND40.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(7) == 10 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(7) == 11)
					v18 = 0;
				if (_vm->_objectsManager.getBobAnimDataIdx(7) == 18 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(7) == 19)
					v18 = 0;
				if (_vm->_objectsManager.getBobAnimDataIdx(7) == 19)
					_vm->_objectsManager.setBobAnimation(3);
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(3) != 48);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_objectsManager.setSpriteIndex(0, 62);
			_vm->_objectsManager.animateSprite(0);
			_vm->_objectsManager.setBobAnimation(6);
			_vm->_objectsManager.stopBobAnimation(7);
			_vm->_objectsManager.stopBobAnimation(3);
			break;
			}

		case 62:
			_vm->_talkManager.OBJET_VIVANT("SBCADA.pe2");
			break;

		case 65:
			_vm->_talkManager.OBJET_VIVANT("ScCADA.pe2");
			break;

		case 80: {
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(12);
			_vm->_objectsManager.setBobAnimation(13);
			_vm->_objectsManager.setBobAnimDataIdx(12, 0);
			_vm->_objectsManager.setBobAnimDataIdx(13, 0);
			int v21 = 0;
			_vm->_soundManager.loadWav("SOUND44.WAV", 1);
			_vm->_soundManager.loadWav("SOUND71.WAV", 2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(12) == 4 && !v21) {
					_vm->_soundManager.playWav(1);
					v21 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(12) == 5)
					v21 = 0;
				if (_vm->_objectsManager.getBobAnimDataIdx(4) == 5 && !v21) {
					_vm->_soundManager.playWav(2);
					v21 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(4) == 6)
					v21 = 0;
				if (_vm->_objectsManager.getBobAnimDataIdx(13) == 8) {
					_vm->_objectsManager.stopBobAnimation(13);
					_vm->_objectsManager.stopBobAnimation(3);
					_vm->_objectsManager.setBobAnimation(4);
					_vm->_objectsManager.setBobAnimDataIdx(4, 0);
					_vm->_objectsManager.setBobAnimDataIdx(13, 0);
				}
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 16);
			_vm->_objectsManager.stopBobAnimation(12);
			_vm->_objectsManager.stopBobAnimation(4);
			_vm->_objectsManager.animateSprite(0);
			_vm->_objectsManager.OBSSEUL = true;
			_vm->_objectsManager.INILINK("IM27a");
			_vm->_objectsManager.OBSSEUL = false;
			break;
			}

		case 81: {
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._oldCharacterPosX = _vm->_objectsManager.getSpriteX(0);
			_vm->_globals._oldDirection = -1;
			_vm->_globals.Compteur = 0;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 119, 268);
			_vm->_globals.NOT_VERIF = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(11);
			_vm->_objectsManager.setBobAnimation(8);
			_vm->_objectsManager.setBobAnimDataIdx(11, 0);
			_vm->_objectsManager.setBobAnimDataIdx(8, 0);
			_vm->_soundManager.loadWav("SOUND44.WAV", 1);
			_vm->_soundManager.loadWav("SOUND48.WAV", 2);
			_vm->_soundManager.loadWav("SOUND49.WAV", 3);
			int v24 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(11) == 4 && !v24) {
					_vm->_soundManager.playWav(1);
					v24 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(11) == 5)
					v24 = 0;
				if (_vm->_objectsManager.getBobAnimDataIdx(8) == 11 && !v24) {
					_vm->_soundManager.playWav(2);
					v24 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(8) == 12)
					v24 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(8) != 32);
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 201, 14, 1);
			_vm->_objectsManager.animateSprite(0);
			_vm->_objectsManager.stopBobAnimation(11);
			_vm->_objectsManager.stopBobAnimation(8);
			_vm->_objectsManager.setBobAnimation(5);
			_vm->_objectsManager.setBobAnimation(6);
			_vm->_objectsManager.setBobAnimDataIdx(5, 0);
			_vm->_objectsManager.setBobAnimDataIdx(6, 0);
			_vm->_soundManager.playWav(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(5) != 74);
			_vm->_objectsManager.stopBobAnimation(5);
			_vm->_objectsManager.stopBobAnimation(6);
			_vm->_objectsManager.setBobAnimation(9);
			_vm->_objectsManager.setBobAnimation(7);
			break;
			}

		case 83:
			_vm->_talkManager.PARLER_PERSO("CVIGIL.pe2");
			break;

		case 84:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("CVIGIL1.PE2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 85:
			_vm->_objectsManager.stopBobAnimation(3);
			_vm->_objectsManager.setBobAnimation(5);
			_vm->_objectsManager.setBobAnimDataIdx(5, 0);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(5) != 6);
			_vm->_objectsManager.stopBobAnimation(5);
			_vm->_objectsManager.setBobAnimation(6);
			_vm->_objectsManager.OBSSEUL = true;
			_vm->_objectsManager.INILINK("IM24a");
			_vm->_objectsManager.OBSSEUL = false;
			break;

		case 86:
			if (_vm->_globals._saveData->_data[svField231] == 1) {
				_vm->_talkManager.PARLER_PERSO("chotess1.pe2");
			} else {
				_vm->_globals._introSpeechOffFl = true;
				_vm->_talkManager.PARLER_PERSO("chotesse.pe2");
				_vm->_globals._introSpeechOffFl = false;
			}
			break;

		case 87:
			if (_vm->_globals._saveData->_data[svField188])
				_vm->_talkManager.PARLER_PERSO("stand2.pe2");
			else
				_vm->_talkManager.PARLER_PERSO("stand1.pe2");
			break;

		case 88:
			if (_vm->_globals._saveData->_data[svField183] == 1) {
				_vm->_objectsManager.setBobAnimDataIdx(1, 0);
				_vm->_objectsManager.setBobAnimDataIdx(2, 0);
				_vm->_objectsManager.setBobAnimation(1);
				_vm->_objectsManager.setBobAnimation(2);
				_vm->_soundManager.loadSample(1, "SOUND40.WAV");
				int v25 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 1 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 2)
						v25 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 3 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 4)
						v25 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 5 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 6)
						v25 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 7 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 8)
						v25 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.getBobAnimDataIdx(1) != 9);
				_vm->_objectsManager.stopBobAnimation(1);
				_vm->_objectsManager.stopBobAnimation(2);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
			if (_vm->_globals._saveData->_data[svField183] == 2) {
				_vm->_objectsManager.setBobAnimDataIdx(1, 0);
				_vm->_objectsManager.setBobAnimDataIdx(3, 0);
				_vm->_objectsManager.setBobAnimation(1);
				_vm->_objectsManager.setBobAnimation(3);
				_vm->_soundManager.loadSample(1, "SOUND40.WAV");
				int v26 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 1 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 2)
						v26 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 3 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 4)
						v26 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 5 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 6)
						v26 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 7 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(1) == 8)
						v26 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.getBobAnimDataIdx(1) != 9);
				_vm->_objectsManager.stopBobAnimation(1);
				_vm->_objectsManager.stopBobAnimation(3);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
			break;

		case 90:
			_vm->_soundManager.playSound("SOUND52.WAV");
			if (!_vm->_globals._saveData->_data[svField186]) {
				_vm->_animationManager.playSequence("CIB5A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals._saveData->_data[svField186] == 1) {
				_vm->_animationManager.playSequence("CIB5C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 329, 87, 2);
			}
			break;

		case 91:
			_vm->_soundManager.playSound("SOUND52.WAV");
			if (!_vm->_globals._saveData->_data[svField186]) {
				_vm->_animationManager.playSequence("CIB5B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals._saveData->_data[svField186] == 1) {
				_vm->_animationManager.playSequence("CIB5D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
			}
			break;

		case 92:
			_vm->_soundManager.playSound("SOUND52.WAV");
			if (!_vm->_globals._saveData->_data[svField184]) {
				_vm->_animationManager.playSequence("CIB6A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals._saveData->_data[svField184] == 1) {
				_vm->_animationManager.playSequence("CIB6C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 293, 139, 3);
			}
			break;

		case 93:
			_vm->_soundManager.playSound("SOUND52.WAV");
			if (!_vm->_globals._saveData->_data[svField184]) {
				_vm->_animationManager.playSequence("CIB6B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals._saveData->_data[svField184] == 1) {
				_vm->_animationManager.playSequence("CIB6D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
			}
			break;

		case 94:
			if (!_vm->_globals._saveData->_data[svField228])
				_vm->_talkManager.PARLER_PERSO("flicn.pe2");
			if (_vm->_globals._saveData->_data[svField228] == 1)
				_vm->_talkManager.PARLER_PERSO("flicn1.pe2");
			break;

		case 95:
			_vm->_objectsManager.setBobAnimation(9);
			_vm->_objectsManager.setBobAnimation(10);
			_vm->_objectsManager.setBobAnimation(12);
			_vm->_objectsManager.setBobAnimDataIdx(9, 0);
			_vm->_objectsManager.setBobAnimDataIdx(10, 0);
			_vm->_objectsManager.setBobAnimDataIdx(12, 0);
			_vm->_objectsManager.removeSprite(0);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(9) != 15);
			_vm->_objectsManager.stopBobAnimation(9);
			_vm->_objectsManager.animateSprite(0);
			_vm->_soundManager.playSound("SOUND50.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(12) != 117);
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 830, 122, 0);
			_vm->_objectsManager.stopBobAnimation(12);
			_vm->_objectsManager.stopBobAnimation(10);
			_vm->_objectsManager.setBobAnimation(11);
			break;

		case 98:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("CVIGIL2.PE2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 100:
			_vm->_talkManager.PARLER_PERSO("tourist.pe2");
			break;

		case 101:
			_vm->_talkManager.PARLER_PERSO("tahi1.pe2");
			break;

		case 103:
			// Dice game
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("tourist1.pe2");
			_vm->_globals._introSpeechOffFl = false;
			_vm->_animationManager.playAnim2("T421.ANM", 100, 14, 500);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("tourist2.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 104:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("tourist3.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 105:
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._oldCharacterPosX = _vm->_objectsManager.getSpriteX(0);
			_vm->_globals._oldDirection = -1;
			_vm->_globals.Compteur = 0;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			if (_vm->_globals._saveData->_data[svField253] == 1) {
				_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 201, 294);
			}
			if (_vm->_globals._saveData->_data[svField253] == 2) {
				_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 158, 338);
			}
			if (_vm->_globals._saveData->_data[svField253] > 2) {
				_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 211, 393);
			}
			_vm->_globals.NOT_VERIF = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setSpriteIndex(0, 60);
			_vm->_soundManager.loadSample(1, "SOUND63.WAV");
			if (_vm->_globals._saveData->_data[svField253] > 2) {
				_vm->_objectsManager.setBobAnimation(4);
				int v33 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.getBobAnimDataIdx(4) == 9 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(4) == 10)
						v33 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(4) == 32 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(4) == 33)
						v33 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(4) == 55 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(4) == 56)
						v33 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 72);
				_vm->_objectsManager.stopBobAnimation(4);
			}
			if (_vm->_globals._saveData->_data[svField253] == 1) {
				_vm->_objectsManager.setBobAnimation(6);
				int v34 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.getBobAnimDataIdx(6) == 9 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(6) == 10)
						v34 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(6) == 32 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(6) == 33)
						v34 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(6) == 55 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(6) == 56)
						v34 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.getBobAnimDataIdx(6) != 72);
				_vm->_objectsManager.stopBobAnimation(6);
			}
			if (_vm->_globals._saveData->_data[svField253] == 2) {
				_vm->_objectsManager.setBobAnimation(5);
				int v35 = 0;
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					if (_vm->_objectsManager.getBobAnimDataIdx(5) == 9 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(5) == 10)
						v35 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(5) == 32 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(5) == 33)
						v35 = 0;
					if (_vm->_objectsManager.getBobAnimDataIdx(5) == 55 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (_vm->_objectsManager.getBobAnimDataIdx(5) == 56)
						v35 = 0;
					_vm->_eventsManager.VBL();
				} while (_vm->_objectsManager.getBobAnimDataIdx(5) != 72);
				_vm->_objectsManager.stopBobAnimation(5);
			}
			_vm->_objectsManager.animateSprite(0);
			_vm->_objectsManager.ACTION_DOS(1);
			_vm->_soundManager.DEL_SAMPLE(1);
			break;

		case 106:
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(4);
			_vm->_objectsManager.setBobAnimDataIdx(4, 0);
			_vm->_soundManager.loadWav("SOUND61.WAV", 1);
			_vm->_soundManager.loadWav("SOUND62.WAV", 2);
			_vm->_soundManager.loadWav("SOUND61.WAV", 3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 10);
			_vm->_soundManager.playWav(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 18);
			_vm->_soundManager.playWav(2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 62);
			_vm->_soundManager.playWav(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 77);
			_vm->_objectsManager.stopBobAnimation(4);
			_vm->_objectsManager.animateSprite(0);
			break;

		case 107:
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(5);
			_vm->_objectsManager.setBobAnimDataIdx(5, 0);
			_vm->_soundManager.loadWav("SOUND61.WAV", 1);
			_vm->_soundManager.loadWav("SOUND62.WAV", 2);
			_vm->_soundManager.loadWav("SOUND61.WAV", 3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(5) != 10);
			_vm->_soundManager.playWav(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(5) != 18);
			_vm->_soundManager.playWav(2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(5) != 38);
			_vm->_soundManager.playWav(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(5) != 53);
			_vm->_objectsManager.stopBobAnimation(5);
			_vm->_objectsManager.animateSprite(0);
			break;

		case 108:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("peche1.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 109:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("peche2.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 110:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("peche3.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 111:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("peche4.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 112:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("teint1.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 113:
			_vm->_talkManager.PARLER_PERSO("teint.pe2");
			break;

		case 114:
			_vm->_talkManager.PARLER_PERSO("tahibar.pe2");
			break;

		case 115:
			_vm->_talkManager.PARLER_PERSO("ilebar.pe2");
			break;

		case 116:
			_vm->_talkManager.PARLER_PERSO("Profred.pe2");
			break;

		case 170:
			_vm->_talkManager.PARLER_PERSO("GRED.pe2");
			break;

		case 171: {
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("gred1.pe2");
			_vm->_globals._introSpeechOffFl = false;
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._oldCharacterPosX = _vm->_objectsManager.getSpriteX(0);
			_vm->_globals._oldDirection = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.NOT_VERIF = true;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 361, 325);
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._zoneNum = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_globals._exitId = 59;
			break;
			}

		case 172:
			_vm->_talkManager.PARLER_PERSO("GBLEU.pe2");
			break;

		case 173: {
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("gbleu1.pe2");
			_vm->_globals._introSpeechOffFl = false;
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._oldCharacterPosX = _vm->_objectsManager.getSpriteX(0);
			_vm->_globals._oldDirection = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.NOT_VERIF = true;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 361, 325);
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._zoneNum = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_globals._exitId = 59;
			break;
			}

		case 174:
			_vm->_talkManager.PARLER_PERSO("Profbl.pe2");
			break;

		case 175:
			_vm->_objectsManager.setSpriteIndex(0, 55);
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(9);
			_vm->_objectsManager.setBobAnimation(10);
			_vm->_objectsManager.BOB_OFFSET(10, 300);
			_vm->_soundManager.playSound("SOUND44.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(10) != 7);
			_vm->_objectsManager.setBobAnimation(6);
			_vm->_objectsManager.stopBobAnimation(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(6) != 10);
			_vm->_soundManager.playSound("SOUND71.WAV");
			_vm->_objectsManager.setBobAnimation(7);
			_vm->_objectsManager.stopBobAnimation(4);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(7) != 15);
			_vm->_objectsManager.stopBobAnimation(5);
			_vm->_objectsManager.setBobAnimation(8);
			_vm->_soundManager.playSound("SOUND70.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(8) != 76);
			_vm->_objectsManager.stopBobAnimation(6);
			_vm->_objectsManager.stopBobAnimation(7);
			_vm->_objectsManager.stopBobAnimation(8);
			_vm->_objectsManager.stopBobAnimation(9);
			_vm->_objectsManager.stopBobAnimation(10);
			_vm->_objectsManager.animateSprite(0);
			break;

		case 176:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("gred2.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 177:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("gbleu2.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 200:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("Gm2.PE2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 201:
			_vm->_objectsManager.setBobAnimation(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(3) != 18);
			_vm->_objectsManager.stopBobAnimation(3);
			_vm->_objectsManager.setBobAnimation(4);
			break;

		case 202:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("SVGARD2.PE2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 203:
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(4);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
				if (_vm->_objectsManager.getBobAnimDataIdx(4) == 18)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 18, 334, 0, false);
			} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 26);
			_vm->_objectsManager.stopBobAnimation(4);
			_vm->_objectsManager.animateSprite(0);
			break;

		case 204: {
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(3);
			_vm->_soundManager.loadWav("SOUND67.WAV", 1);
			int v41 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(3) == 10 && !v41) {
					_vm->_soundManager.playWav(1);
					v41 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(3) == 11)
					v41 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(3) != 50);
			_vm->_objectsManager.stopBobAnimation(3);
			_vm->_objectsManager.animateSprite(0);
			break;
			}

		case 205: {
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(4);
			_vm->_soundManager.loadWav("SOUND69.WAV", 1);
			int v42 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(4) == 10 && !v42) {
					_vm->_soundManager.playWav(1);
					v42 = 1;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(4) == 11)
					v42 = 0;
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(4) != 24);
			_vm->_objectsManager.stopBobAnimation(4);
			_vm->_objectsManager.animateSprite(0);
			break;
			}

		case 207:
			_vm->_talkManager.OBJET_VIVANT("PANNEAU.PE2");
			break;

		case 208: {
			_vm->_globals._disableInventFl = true;
			if (_vm->_globals._saveData->_data[svField6] != _vm->_globals._saveData->_data[svField401]) {
				_vm->_soundManager._specialSoundNum = 208;
				_vm->_animationManager.playSequence("SORT.SEQ", 10, 4, 10, true);
				_vm->_soundManager._specialSoundNum = 0;
			}
			_vm->_globals.NOT_VERIF = true;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 330, 418);
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._zoneNum = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_objectsManager.setSpriteIndex(0, 64);
			_vm->_globals._exitId = _vm->_globals._saveData->_data[svField401];
			_vm->_globals._disableInventFl = false;
			break;
			}

		case 209: {
			_vm->_objectsManager.setBobAnimDataIdx(1, 0);
			_vm->_objectsManager.setBobAnimDataIdx(2, 0);
			_vm->_objectsManager.setSpriteIndex(0, 60);
			_vm->_objectsManager.stopBobAnimation(4);
			_vm->_objectsManager.setBobAnimation(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(1) != 9);
			_vm->_objectsManager.stopBobAnimation(1);
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_globals.NOT_VERIF = true;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 330, 314);
			_vm->_objectsManager._zoneNum = 0;
			_vm->_globals.NOT_VERIF = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_objectsManager.setSpriteIndex(0, 64);
			_vm->_objectsManager.setBobAnimation(2);
			_vm->_soundManager.playSound("SOUND66.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(2) != 10);
			_vm->_objectsManager.stopBobAnimation(2);
			_vm->_objectsManager.setBobAnimation(4);
			break;
			}

		case 210:
			_vm->_animationManager.NO_SEQ = true;
			_vm->_soundManager._specialSoundNum = 210;
			_vm->_animationManager.playSequence2("SECRET1.SEQ", 1, 12, 1);
			_vm->_soundManager._specialSoundNum = 0;
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 192, 152, 0);
			_vm->_objectsManager.setBobAnimation(9);
			_vm->_objectsManager.OBSSEUL = true;
			_vm->_objectsManager.INILINK("IM73a");
			_vm->_objectsManager.OBSSEUL = false;
			_vm->_globals.CACHE_ON();
			_vm->_animationManager.NO_SEQ = false;
			_vm->_globals.CACHE_ADD(0);
			_vm->_globals.CACHE_ADD(1);
			_vm->_graphicsManager.SETCOLOR4(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(254, 0, 0, 0);
			break;

		case 211:
			_vm->_objectsManager.removeSprite(0);
			_vm->_globals.CACHE_OFF();
			_vm->_animationManager.NO_SEQ = true;
			_vm->_soundManager._specialSoundNum = 211;
			_vm->_animationManager.playSequence("SECRET2.SEQ", 1, 12, 100);
			_vm->_soundManager._specialSoundNum = 0;
			_vm->_animationManager.NO_SEQ = false;
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_graphicsManager.fadeOutLong();

			for (int i = 1; i <= 39; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}

			_vm->_graphicsManager.SETCOLOR4(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(254, 0, 0, 0);
			break;

		case 215:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("aviat.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 216:
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("aviat1.pe2");
			_vm->_globals._introSpeechOffFl = false;
			break;

		case 229:
			_vm->_soundManager._specialSoundNum = 229;
			_vm->_animationManager.playSequence("MUR.SEQ", 1, 12, 1);
			_vm->_soundManager._specialSoundNum = 0;
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 340, 157, 2);
			break;

		case 230: {
			_vm->_objectsManager.OBSSEUL = true;
			_vm->_objectsManager.INILINK("IM93a");
			_vm->_objectsManager.OBSSEUL = false;
			_vm->_globals.CACHE_ON();
			_vm->_globals.NOT_VERIF = true;
			_vm->_objectsManager._oldCharacterPosX = _vm->_objectsManager.getSpriteX(0);
			_vm->_globals._oldDirection = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.NOT_VERIF = true;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(_vm->_objectsManager.getSpriteX(0), _vm->_objectsManager.getSpriteY(0), 488, 280);
			_vm->_globals.NOT_VERIF = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsManager.GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
			_vm->_objectsManager.removeSprite(0);
			int v45 = 0;
			_vm->_objectsManager.setBobAnimation(7);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsManager.getBobAnimDataIdx(7) == 9 && !v45) {
					v45 = 1;
					_vm->_soundManager.playSound("SOUND81.WAV");
				}
				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(7) != 15);
			_vm->_objectsManager.stopBobAnimation(7);
			_vm->_objectsManager.setSpriteX(0, 476);
			_vm->_objectsManager.setSpriteY(0, 278);
			_vm->_objectsManager.animateSprite(0);
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 337, 154, 3);
			_vm->_objectsManager.OBSSEUL = true;
			_vm->_objectsManager.INILINK("IM93c");
			_vm->_objectsManager.OBSSEUL = false;
			_vm->_globals.CACHE_ON();
			break;
			}

		case 231:
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(12);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(12) != 6);
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("PRMORT.pe2");
			_vm->_globals._introSpeechOffFl = false;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(12) != 12);
			_vm->_objectsManager.animateSprite(0);
			_vm->_objectsManager.stopBobAnimation(12);
			_vm->_globals.CACHE_ON();
			break;

		case 233: {
			_vm->_globals.CACHE_OFF();
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.setBobAnimation(11);
			int v46 = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
				if (_vm->_objectsManager.getBobAnimDataIdx(11) == 10 && !v46)
					v46 = 1;
			} while (_vm->_objectsManager.getBobAnimDataIdx(11) != 13);
			_vm->_objectsManager.stopBobAnimation(11);
			_vm->_globals.CACHE_ON();
			_vm->_objectsManager.setBobAnimation(13);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			} while (_vm->_objectsManager.getBobAnimDataIdx(13) != 48);
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("HRADIO.PE2");
			_vm->_globals._introSpeechOffFl = false;
			_vm->_graphicsManager.fadeOutLong();
			_vm->_objectsManager.stopBobAnimation(13);
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_globals._exitId = 94;
			break;
			}

		case 236: {
			char v47 = _vm->_globals._saveData->_data[svField341];
			if (v47) {
				if (v47 == 2)
					vbobFrameIndex = 5;
				else if (v47 == 3)
					vbobFrameIndex = 4;
				else if (v47 == 1)
					vbobFrameIndex = 6;
				_vm->_soundManager.playSound("SOUND83.WAV");
				_vm->_objectsManager.OPTI_ONE(vbobFrameIndex, 26, 50, 0);
				if (_vm->_globals._saveData->_data[svField341] == 1)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals._saveData->_data[svField341] == 2)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals._saveData->_data[svField341] == 3)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals._saveData->_data[svField341] == 1)
					_vm->_globals._saveData->_data[svField338] = 0;
				if (_vm->_globals._saveData->_data[svField341] == 2)
					_vm->_globals._saveData->_data[svField339] = 0;
				if (_vm->_globals._saveData->_data[svField341] == 3)
					_vm->_globals._saveData->_data[svField340] = 0;
			}
			_vm->_soundManager.playSound("SOUND83.WAV");
			_vm->_objectsManager.OPTI_ONE(6, 0, 23, 0);
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 27, 117, 1);
			break;
			}

		case 237: {
			char v48 = _vm->_globals._saveData->_data[svField341];
			if (v48) {
				if (v48 == 2)
					vbobFrameIndex = 5;
				if (v48 == 3)
					vbobFrameIndex = 4;
				if (v48 == 1)
					vbobFrameIndex = 6;
				_vm->_soundManager.playSound("SOUND83.WAV");
				_vm->_objectsManager.OPTI_ONE(vbobFrameIndex, 26, 50, 0);
				if (_vm->_globals._saveData->_data[svField341] == 1)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals._saveData->_data[svField341] == 2)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals._saveData->_data[svField341] == 3)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals._saveData->_data[svField341] == 1)
					_vm->_globals._saveData->_data[svField338] = 0;
				if (_vm->_globals._saveData->_data[svField341] == 2)
					_vm->_globals._saveData->_data[svField339] = 0;
				if (_vm->_globals._saveData->_data[svField341] == 3)
					_vm->_globals._saveData->_data[svField340] = 0;
			}
			_vm->_soundManager.playSound("SOUND83.WAV");
			_vm->_objectsManager.OPTI_ONE(5, 0, 23, 0);
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 145, 166, 3);
			break;
			}

		case 238: {
			char v49 = _vm->_globals._saveData->_data[svField341];
			if (v49) {
				if (v49 == 2)
					vbobFrameIndex = 5;
				else if (v49 == 3)
					vbobFrameIndex = 4;
				else if (v49 == 1)
					vbobFrameIndex = 6;
				_vm->_soundManager.playSound("SOUND83.WAV");
				_vm->_objectsManager.OPTI_ONE(vbobFrameIndex, 26, 50, 0);
				if (_vm->_globals._saveData->_data[svField341] == 1)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals._saveData->_data[svField341] == 2)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals._saveData->_data[svField341] == 3)
					_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals._saveData->_data[svField341] == 1)
					_vm->_globals._saveData->_data[svField338] = 0;
				if (_vm->_globals._saveData->_data[svField341] == 2)
					_vm->_globals._saveData->_data[svField339] = 0;
				if (_vm->_globals._saveData->_data[svField341] == 3)
					_vm->_globals._saveData->_data[svField340] = 0;
			}
			_vm->_soundManager.playSound("SOUND83.WAV");
			_vm->_objectsManager.OPTI_ONE(4, 0, 23, 0);
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 296, 212, 5);
			break;
			}

		case 239:
			_vm->_objectsManager.removeSprite(0);
			_vm->_soundManager.playSound("SOUND84.WAV");
			_vm->_objectsManager.OPTI_ONE(16, 0, 10, 0);
			break;

		case 240: {
			_vm->_objectsManager.setBobAnimation(1);
			bool soundFlag = false;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
				if (_vm->_objectsManager.getBobAnimDataIdx(1) == 12 && !soundFlag) {
					_vm->_soundManager.playSound("SOUND86.WAV");
					soundFlag = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(1) == 13)
					soundFlag = false;
				if (_vm->_objectsManager.getBobAnimDataIdx(1) == 25 && !soundFlag) {
					_vm->_soundManager.playSound("SOUND85.WAV");
					soundFlag = true;
				}
				if (_vm->_objectsManager.getBobAnimDataIdx(1) == 25)
					soundFlag = false;
			} while (_vm->_objectsManager.getBobAnimDataIdx(1) != 32);
			_vm->_objectsManager.stopBobAnimation(1);
			_vm->_objectsManager.setBobAnimation(2);
			_vm->_fontManager.hideText(9);
			if (!_vm->_soundManager._textOffFl) {
				_vm->_fontManager.initTextBuffers(9, 617, _vm->_globals.FICH_TEXTE, 91, 41, 3, 30, 253);
				_vm->_fontManager.showText(9);
			}
			if (!_vm->_soundManager._voiceOffFl)
				_vm->_soundManager.mixVoice(617, 4);
			for (int i = 0; i <= 29; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_eventsManager.VBL();
			}
			CharacterLocation *v51 = &_vm->_globals._saveData->_realHopkins;
			v51->_pos.x = _vm->_objectsManager.getSpriteX(0);
			v51->_pos.y = _vm->_objectsManager.getSpriteY(0);
			v51->_startSpriteIndex = 57;
			v51->_location = 97;
			_vm->_globals._saveData->_data[svField121] = 1;
			_vm->_globals._saveData->_data[svField352] = 1;
			_vm->_globals._saveData->_data[svField353] = 1;
			_vm->_globals._saveData->_data[svField354] = 1;
			break;
			}

		case 241:
			_vm->_talkManager.PARLER_PERSO("RECEP.PE2");
			break;

		// Resurrect Samantha's clone
		case 242: {
			_vm->_soundManager.playSound("SOUND87.WAV");
			_vm->_animationManager.NO_SEQ = true;
			_vm->_animationManager.playSequence("RESUF.SEQ", 1, 24, 1);
			_vm->_animationManager.NO_SEQ = false;

			CharacterLocation *samantha = &_vm->_globals._saveData->_samantha;
			samantha->_pos.x = 404;
			samantha->_pos.y = 395;
			samantha->_startSpriteIndex = 64;
			samantha->_location = _vm->_globals._screenId;
			samantha->_zoomFactor = -(100 * (67 - (100 - abs(_vm->_globals._spriteSize[790 / 2]))) / 67);

			_vm->_globals._saveData->_data[svField357] = 1;
			_vm->_globals._saveData->_data[svField354] = 0;
			_vm->_globals._saveData->_data[svField356] = 0;
			_vm->_globals._saveData->_data[svField355] = 1;
			_vm->_objectsManager._twoCharactersFl = true;
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 373, 191, 3);
			_vm->_objectsManager.addStaticSprite(_vm->_globals.TETE, samantha->_pos, 1, 3, samantha->_zoomFactor, false, 20, 127);
			_vm->_objectsManager.animateSprite(1);
			break;
			}

		case 243:
			_vm->_soundManager.playSound("SOUND88.WAV");
			if (_vm->_globals._saveData->_data[svField341] == 2) {
				_vm->_animationManager.NO_SEQ = true;
				_vm->_animationManager.playSequence("RESU.SEQ", 2, 24, 2);
				_vm->_animationManager.NO_SEQ = false;
			} else {
				_vm->_objectsManager.OPTI_ONE(7, 0, 14, 0);
			}
			break;

		case 245:
			_vm->_soundManager.playSound("SOUND89.WAV");
			_vm->_objectsManager.OPTI_ONE(5, 0, 6, 0);
			_vm->_linesManager.ZONEP[4]._destX = 276;
			_vm->_objectsManager.enableVerb(4, 19);
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 285, 379, 0);
			_vm->_globals._saveData->_data[svField399] = 1;
			break;

		case 246:
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.OPTI_ONE(6, 0, 15, 0);
			_vm->_objectsManager.PERSO_ON = true;
			_vm->_graphicsManager.NB_SCREEN(true);
			_vm->_animationManager.NO_SEQ = true;
			_vm->_animationManager.playSequence2("TUNNEL.SEQ", 1, 18, 20);
			_vm->_animationManager.NO_SEQ = false;
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_graphicsManager.fadeOutLong();
			_vm->_objectsManager.PERSO_ON = false;
			_vm->_globals._exitId = 100;
			break;

		case 600:
			if (!_vm->getIsDemo()) {
				_vm->_graphicsManager.FADE_LINUX = 2;
				_vm->_graphicsManager._fadeDefaultSpeed = 1;
				_vm->_animationManager.playAnim("BOMBE1A.ANM", 100, 18, 100);
			}
			_vm->_graphicsManager.loadImage("BOMBEB");
			_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
			_vm->_graphicsManager.OPTI_INI("BOMBE", 2, true);
			_vm->_graphicsManager.fadeInShort();
			break;

		case 601:
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 163, 7, false);
			_vm->_objectsManager.OPTI_ONE(2, 0, 16, 4);
			break;

		case 602:
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 163, 7, false);
			_vm->_objectsManager.OPTI_ONE(4, 0, 16, 4);
			break;

		case 603:
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 163, 7, false);
			_vm->_objectsManager.OPTI_ONE(3, 0, 16, 4);
			_vm->_soundManager._specialSoundNum = 199;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.playAnim("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager._specialSoundNum = 0;
			memset(_vm->_graphicsManager._vesaBuffer, 0, 614400);
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_globals._exitId = 151;
			break;

		case 604:
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 163, 7, false);
			_vm->_objectsManager.OPTI_ONE(1, 0, 16, 4);
			_vm->_soundManager._specialSoundNum = 199;
			_vm->_animationManager.playAnim("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager._specialSoundNum = 0;
			_vm->_graphicsManager._noFadingFl = true;
			memset(_vm->_graphicsManager._vesaBuffer, 0, 614400);
			_vm->_globals._exitId = 151;
			break;

		case 605:
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 163, 7, false);
			_vm->_objectsManager.OPTI_ONE(5, 0, 16, 4);
			_vm->_graphicsManager.fadeOutShort();
			_vm->_soundManager._specialSoundNum = 199;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.playAnim("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager._specialSoundNum = 0;
			_vm->_graphicsManager._noFadingFl = true;
			memset(_vm->_graphicsManager._vesaBuffer, 0, 614400);
			_vm->_globals._exitId = 151;
			break;

		case 606:
			_vm->_graphicsManager.fastDisplay(_vm->_globals.SPRITE_ECRAN, 513, 163, 7, false);
			_vm->_objectsManager.OPTI_ONE(6, 0, 16, 4);
			if ((_vm->getPlatform() != Common::kPlatformWindows) || !_vm->getIsDemo()) {
				_vm->_animationManager.playAnim("BOMBE3A.ANM", 50, 14, 500);
				memset(_vm->_graphicsManager._vesaBuffer, 0, 614400);
			}
			_vm->_globals._exitId = 6;
			break;

		case 607:
			// Display bomb plan
			if (!_vm->getIsDemo()) {
				memcpy(_vm->_graphicsManager._oldPalette, _vm->_graphicsManager._palette, 769);
				_vm->_animationManager.playAnim2("PLAN.ANM", 50, 10, 800);
			}
			_vm->_graphicsManager.NBBLOC = 0;
			break;

		case 608:
			_vm->_objectsManager.stopBobAnimation(2);
			_vm->_objectsManager.stopBobAnimation(3);
			_vm->_objectsManager.stopBobAnimation(4);
			_vm->_objectsManager.stopBobAnimation(6);
			_vm->_objectsManager.stopBobAnimation(11);
			_vm->_objectsManager.stopBobAnimation(10);
			break;

		case 609:
			_vm->_objectsManager.setBobAnimation(2);
			_vm->_objectsManager.setBobAnimation(3);
			_vm->_objectsManager.setBobAnimation(4);
			_vm->_objectsManager.setBobAnimation(6);
			_vm->_objectsManager.setBobAnimation(11);
			_vm->_objectsManager.setBobAnimation(10);
			break;

		case 610:
			_vm->_objectsManager.stopBobAnimation(5);
			_vm->_objectsManager.stopBobAnimation(7);
			_vm->_objectsManager.stopBobAnimation(8);
			_vm->_objectsManager.stopBobAnimation(9);
			_vm->_objectsManager.stopBobAnimation(12);
			_vm->_objectsManager.stopBobAnimation(13);
			break;

		case 611:
			_vm->_objectsManager.setBobAnimation(5);
			_vm->_objectsManager.setBobAnimation(7);
			_vm->_objectsManager.setBobAnimation(8);
			_vm->_objectsManager.setBobAnimation(9);
			_vm->_objectsManager.setBobAnimation(12);
			_vm->_objectsManager.setBobAnimation(13);
			break;
		}
		opcodeType = 1;
		break;
	case MKTAG24('E', 'I', 'F'):
		opcodeType = 4;
		break;
	case MKTAG24('V', 'A', 'L'): {
		opcodeType = 1;
		int idx = (int16)READ_LE_UINT16(dataP + 5);
		assert(idx >= 0 && idx < 2050);
		_vm->_globals._saveData->_data[idx] = dataP[7];
		break;
		}
	case MKTAG24('A', 'D', 'D'):
		opcodeType = 1;
		_vm->_globals._saveData->_data[(int16)READ_LE_UINT16(dataP + 5)] += dataP[7];
		break;
	case MKTAG24('B', 'O', 'S'):
		opcodeType = 1;
		_vm->_objectsManager.BOB_OFFSET((int16)READ_LE_UINT16(dataP + 5), (int16)READ_LE_UINT16(dataP + 7));
		break;
	case MKTAG24('V', 'O', 'N'):
		_vm->_objectsManager.enableVerb((int16)READ_LE_UINT16(dataP + 5), (int16)READ_LE_UINT16(dataP + 7));
		opcodeType = 1;
		break;
	case MKTAG24('Z', 'C', 'H'):
		_vm->_linesManager.ZONEP[(int16)READ_LE_UINT16(dataP + 5)].field12 = (int16)READ_LE_UINT16(dataP + 7);
		opcodeType = 1;
		break;
	case MKTAG24('J', 'U', 'M'):
		_vm->_objectsManager._jumpZone = (int16)READ_LE_UINT16(dataP + 5);
		_vm->_objectsManager._jumpVerb = (int16)READ_LE_UINT16(dataP + 7);
		opcodeType = 6;
		break;
	case MKTAG24('S', 'O', 'U'): {
		int soundNum = (int16)READ_LE_UINT16(dataP + 5);

		Common::String file = Common::String::format("SOUND%d.WAV", soundNum);
		_vm->_soundManager.playSound(file);
		opcodeType = 1;
		break;
		}
	case MKTAG24('V', 'O', 'F'):
		_vm->_objectsManager.disableVerb((int16)READ_LE_UINT16(dataP + 5), (int16)READ_LE_UINT16(dataP + 7));
		opcodeType = 1;
		break;
	case MKTAG24('I', 'I', 'F'):
		opcodeType = 3;
		break;
	default:
		warning("Unhandled opcode %c%c%c", dataP[2], dataP[3], dataP[4]);
		break;
	}

	return opcodeType;
}


int ScriptManager::handleGoto(const byte *dataP) {
	return (int16)READ_LE_UINT16(dataP + 5);
}

int ScriptManager::handleIf(const byte *dataP, int a2) {
	int v20;
	int v2 = a2;
	bool loopFl;
	do {
		loopFl = false;
		int v3 = v2;
		int opcodeType;
		do {
			if (_vm->shouldQuit())
				return 0; // Exiting game

			++v3;
			opcodeType = checkOpcode(dataP + 20 * v3);
			if (v3 > 400)
				error("Control if failed");
		} while (opcodeType != 4); // EIF
		v20 = v3;
		int v6 = v2;
		bool v7 = false;
		do {
			if (_vm->shouldQuit())
				return 0; // Exiting game

			++v6;
			if (checkOpcode(dataP + 20 * v6) == 3) // IIF
				v7 = true;
			if (v6 > 400)
				error("Control if failed ");
			if (v7) {
				v2 = v20;
				loopFl = true;
				break;
			}
		} while (v20 != v6);
	} while (loopFl);

	const byte *buf = dataP + 20 * a2;
	byte oper = buf[13];
	byte oper2 = buf[14];
	byte operType = buf[15];
	int saveDataIdx1 = (int16)READ_LE_UINT16(buf + 5);
	int compVal1 = (int16)READ_LE_UINT16(buf + 7);
	bool check1Fl = false;
	if ((oper == 1 && _vm->_globals._saveData->_data[saveDataIdx1] == compVal1) ||
	    (oper == 2 && _vm->_globals._saveData->_data[saveDataIdx1] != compVal1) ||
	    (oper == 3 && _vm->_globals._saveData->_data[saveDataIdx1] <= compVal1) ||
	    (oper == 4 && _vm->_globals._saveData->_data[saveDataIdx1] >= compVal1) ||
	    (oper == 5 && _vm->_globals._saveData->_data[saveDataIdx1] > compVal1) ||
	    (oper == 6 && _vm->_globals._saveData->_data[saveDataIdx1] < compVal1))
		check1Fl = true;

	bool check2Fl = false;
	if (operType != 3) {
		int saveDataIdx2 = (int16)READ_LE_UINT16(buf + 9);
		int compVal2 = (int16)READ_LE_UINT16(buf + 11);
		if ((oper2 == 1 && compVal2 == _vm->_globals._saveData->_data[saveDataIdx2]) ||
		    (oper2 == 2 && compVal2 != _vm->_globals._saveData->_data[saveDataIdx2]) ||
		    (oper2 == 3 && compVal2 >= _vm->_globals._saveData->_data[saveDataIdx2]) ||
		    (oper2 == 4 && compVal2 <= _vm->_globals._saveData->_data[saveDataIdx2]) ||
		    (oper2 == 5 && compVal2 < _vm->_globals._saveData->_data[saveDataIdx2]) ||
		    (oper2 == 6 && compVal2 > _vm->_globals._saveData->_data[saveDataIdx2]))
			check2Fl = true;
	}
	
	if ((operType == 3) && check1Fl) {
		return (a2 + 1);
	} else if ((operType == 1) && check1Fl && check2Fl) {
		return (a2 + 1);
	} else if ((operType == 2) && (check1Fl || check2Fl)) {
		return (a2 + 1);
	}

	return (v20 + 1);
}

int ScriptManager::checkOpcode(const byte *dataP) {
	int result = 0;
	if (READ_BE_UINT16(dataP) != MKTAG16('F', 'C'))
		return result;

	uint32 signature24 = READ_BE_UINT24(&dataP[2]);
	switch (signature24) {
	case MKTAG24('A', 'N', 'I'):
	case MKTAG24('B', 'C', 'A'):
	case MKTAG24('B', 'O', 'B'):
	case MKTAG24('B', 'O', 'F'):
	case MKTAG24('B', 'O', 'S'):
	case MKTAG24('M', 'U', 'S'):
	case MKTAG24('O', 'B', 'M'):
	case MKTAG24('O', 'B', 'P'):
	case MKTAG24('P', 'E', 'R'):
	case MKTAG24('S', 'O', 'U'):
	case MKTAG24('S', 'P', 'E'):
	case MKTAG24('T', 'X', 'T'):
	case MKTAG24('V', 'A', 'L'):
	case MKTAG24('V', 'O', 'F'):
	case MKTAG24('V', 'O', 'N'):
	case MKTAG24('Z', 'C', 'H'):
	case MKTAG24('Z', 'O', 'F'):
	case MKTAG24('Z', 'O', 'N'):
		result = 1;
		break;
	case MKTAG24('G', 'O', 'T'):
		result = 2;
		break;
	case MKTAG24('I', 'I', 'F'):
		result = 3;
		break;
	case MKTAG24('E', 'I', 'F'):
		result = 4;
		break;
	case MKTAG24('E', 'X', 'I'):
	case MKTAG24('S', 'O', 'R'):
		result = 5;
		break;
	case MKTAG24('J', 'U', 'M'):
		result = 6;
		break;
//	default:
//		warning("Unhandled opcode %c%c%c", dataP[2], dataP[3], dataP[4]);
	}
	return result;
}

} // End of namespace Hopkins
