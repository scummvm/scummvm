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

#include "hopkins/objects.h"
#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/sound.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"

namespace Hopkins {

ScriptManager::ScriptManager(HopkinsEngine *vm) {
	_vm = vm;
	_tempObjectFl = false;
}

int ScriptManager::handleOpcode(const byte *dataP) {
	if (READ_BE_UINT16(dataP) != MKTAG16('F', 'C'))
		return 0;

	int opcodeType = 0;
	int vbobFrameIndex = 0;

	uint32 signature24 = READ_BE_UINT24(&dataP[2]);
	switch (signature24) {
	case MKTAG24('T', 'X', 'T'): {
		vbobFrameIndex = dataP[6];
		int mesgId = READ_LE_INT16(dataP + 13);
		opcodeType = 1;
		if (!_tempObjectFl) {
			if (_vm->_globals->_saveData->_data[svField356] == 1) {
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
			if (!_vm->_soundMan->_soundOffFl && _vm->_soundMan->_soundFl) {
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game
					_vm->_events->refreshScreenAndEvents();
				} while (_vm->_soundMan->_soundFl);
			}
			bool displayedTxtFl = false;
			if (!_vm->_soundMan->_textOffFl) {
				int textPosX = READ_LE_INT16(dataP + 9);
				int textPosY = READ_LE_INT16(dataP + 11);
				_vm->_fontMan->initTextBuffers(9, mesgId, _vm->_globals->_textFilename, 2 * textPosX, 2 * textPosY + 40, 6, dataP[7], 253);
				_vm->_fontMan->showText(9);
				displayedTxtFl = true;
			}
			if (!_vm->_soundMan->_voiceOffFl)
				_vm->_soundMan->mixVoice(mesgId, 4, displayedTxtFl);
		} else { // if (_tempObjectFl)
			if (_vm->_globals->_saveData->_data[svField356]) {
				_vm->_fontMan->initTextBuffers(9, 635, _vm->_globals->_textFilename, 55, 20, dataP[8], 35, 253);
				bool displayedTxtFl = false;
				if (!_vm->_soundMan->_textOffFl) {
					_vm->_fontMan->showText(9);
					displayedTxtFl = true;
				}

				if (!_vm->_soundMan->_voiceOffFl)
					_vm->_soundMan->mixVoice(635, 4, displayedTxtFl);
			} else {
				int textPosX = READ_LE_INT16(dataP + 9);
				if (!_vm->_soundMan->_textOffFl) {
					switch (_vm->_globals->_language) {
					case LANG_FR:
						_vm->_fontMan->initTextBuffers(9, mesgId, "OBJET1.TXT", 2 * textPosX, 60, 6, dataP[7], 253);
						break;
					case LANG_EN:
						_vm->_fontMan->initTextBuffers(9, mesgId, "OBJETAN.TXT", 2 * textPosX, 60, 6, dataP[7], 253);
						break;
					case LANG_SP:
						_vm->_fontMan->initTextBuffers(9, mesgId, "OBJETES.TXT", 2 * textPosX, 60, 6, dataP[7], 253);
						break;
					default:
						break;
					}
				}

				bool displayedTxtFl = false;
				if (!_vm->_soundMan->_textOffFl) {
					_vm->_fontMan->showText(9);
					displayedTxtFl = true;
				}

				if (!_vm->_soundMan->_voiceOffFl)
					_vm->_soundMan->mixVoice(mesgId, 5, displayedTxtFl);
			}
		}
		break;
		}
	case MKTAG24('B', 'O', 'B'):
		if (!_vm->_objectsMan->_disableFl) {
			int vbobIdx = dataP[5];
			vbobFrameIndex = dataP[6];
			int moveChange = dataP[7];
			int vbobPosX = READ_LE_INT16(dataP + 8);
			int vbobPosY = READ_LE_INT16(dataP + 10);
			if (vbobIdx == 52) {
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, vbobPosX, READ_LE_INT16(dataP + 10), vbobFrameIndex);
			} else if (vbobIdx == 51) {
				_vm->_objectsMan->quickDisplayBobSprite(vbobFrameIndex);
			} else if (vbobIdx != 50) {
				_vm->_objectsMan->initVbob(_vm->_globals->_levelSpriteBuf, vbobIdx, vbobPosX, vbobPosY, vbobFrameIndex);
				if (moveChange)
					moveChange /= _vm->_globals->_speed;
				if (moveChange > 1) {
					do {
						if (_vm->shouldQuit())
							return -1; // Exiting game

						--moveChange;
						_vm->_events->refreshScreenAndEvents();
					} while (moveChange);
				}
			}
		}
		opcodeType = 1;
		break;
	case MKTAG24('S', 'T', 'P'):
		if (!_vm->_objectsMan->_disableFl) {
			// HACK: This piece of code is a replacement to the missing STE opcode when entering the FBI lab.
			if (_vm->_globals->_curRoomNum == 10) {
				_vm->_globals->_prevScreenId = _vm->_globals->_screenId;
				_vm->_globals->_saveData->_data[svLastPrevScreenId] = _vm->_globals->_screenId;
				_vm->_globals->_screenId = _vm->_globals->_saveData->_data[svLastScreenId] = 10;
			}
			_vm->_objectsMan->_twoCharactersFl = false;
			_vm->_objectsMan->_characterPos.x = READ_LE_INT16(dataP + 6);
			_vm->_objectsMan->_characterPos.y = READ_LE_INT16(dataP + 8);
			_vm->_objectsMan->_startSpriteIndex = dataP[5];
			if (_vm->_objectsMan->_changeHeadFl) {
				if (_vm->_globals->_saveData->_data[svField354] == 1
						&& _vm->_globals->_saveData->_cloneHopkins._pos.x && _vm->_globals->_saveData->_cloneHopkins._pos.y
						&& _vm->_globals->_saveData->_cloneHopkins._startSpriteIndex && _vm->_globals->_saveData->_cloneHopkins._location) {

					_vm->_objectsMan->_characterPos = _vm->_globals->_saveData->_cloneHopkins._pos;
					_vm->_objectsMan->_startSpriteIndex = _vm->_globals->_saveData->_cloneHopkins._startSpriteIndex;
				}
				if (_vm->_globals->_saveData->_data[svField356] == 1
						&& _vm->_globals->_saveData->_samantha._pos.x && _vm->_globals->_saveData->_samantha._pos.y
						&& _vm->_globals->_saveData->_samantha._startSpriteIndex && _vm->_globals->_saveData->_samantha._location) {
					_vm->_objectsMan->_characterPos = _vm->_globals->_saveData->_samantha._pos;
					_vm->_objectsMan->_startSpriteIndex = _vm->_globals->_saveData->_samantha._startSpriteIndex;
				}
				if (_vm->_globals->_saveData->_data[svField357] == 1
						&& _vm->_globals->_saveData->_realHopkins._pos.x && _vm->_globals->_saveData->_realHopkins._pos.y
						&& _vm->_globals->_saveData->_realHopkins._startSpriteIndex && _vm->_globals->_saveData->_realHopkins._location) {
					_vm->_objectsMan->_characterPos = _vm->_globals->_saveData->_realHopkins._pos;
					_vm->_objectsMan->_startSpriteIndex = _vm->_globals->_saveData->_realHopkins._startSpriteIndex;
				}
			}
			if (_vm->_globals->_saveData->_data[svField356] == 1
					&& _vm->_globals->_saveData->_realHopkins._location == _vm->_globals->_screenId) {
				_vm->_objectsMan->addStaticSprite(
				    _vm->_objectsMan->_headSprites,
				    _vm->_globals->_saveData->_realHopkins._pos,
				    1,
				    2,
				    _vm->_globals->_saveData->_realHopkins._zoomFactor,
				    false,
				    34,
				    190);
				_vm->_objectsMan->animateSprite(1);
				_vm->_objectsMan->_twoCharactersFl = true;
			}
			if (_vm->_globals->_saveData->_data[svField357] == 1
			        && _vm->_globals->_saveData->_data[svField355] == 1
			        && _vm->_globals->_saveData->_samantha._location == _vm->_globals->_screenId) {
				_vm->_objectsMan->addStaticSprite(
				    _vm->_objectsMan->_headSprites,
				    _vm->_globals->_saveData->_samantha._pos,
				    1,
				    3,
				    _vm->_globals->_saveData->_samantha._zoomFactor,
				    false,
				    20,
				    127);
				_vm->_objectsMan->animateSprite(1);
				_vm->_objectsMan->_twoCharactersFl = true;
			}
		}
		opcodeType = 1;
		_vm->_objectsMan->_changeHeadFl = false;
		break;
	case MKTAG24('S', 'T', 'E'):
		if (!_vm->_objectsMan->_disableFl) {
			_vm->_globals->_prevScreenId = _vm->_globals->_screenId;
			_vm->_globals->_saveData->_data[svLastPrevScreenId] = _vm->_globals->_screenId;
			_vm->_globals->_screenId = _vm->_globals->_saveData->_data[svLastScreenId] = dataP[5];
			vbobFrameIndex = dataP[6];
		}
		opcodeType = 1;
		break;
	case MKTAG24('B', 'O', 'F'):
		if (!_vm->_objectsMan->_disableFl)
			_vm->_objectsMan->disableVbob(READ_LE_INT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('P', 'E', 'R'): {
		int specialOpcode = READ_LE_INT16(dataP + 5);
		if (!_vm->_globals->_saveData->_data[svAlternateSpriteFl] && !_vm->_globals->_saveData->_data[svField356]) {
			vbobFrameIndex = 0;

			switch (specialOpcode) {
			case 1:
			case 14:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(4);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(4);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(4);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(4);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(4);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(4);
				break;
			case 2:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(7);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(7);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(7);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(7);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(7);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(7);
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(8);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(8);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(8);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(8);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(8);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(8);
				break;
			case 19:
			case 4:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(1);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(1);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(1);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(1);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(1);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(1);
				break;
			case 5:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(5);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(5);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(5);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(5);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(5);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(5);
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(6);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(6);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(6);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(6);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(6);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(6);
				break;
			case 17:
			case 7:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(2);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(2);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(2);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(2);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(2);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(2);
				break;
			case 18:
			case 8:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(3);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(3);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(3);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(3);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(3);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(3);
				break;
			case 9:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(5);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(5);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(5);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(5);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(5);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(5);
				break;
			case 10:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(6);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(6);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(6);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(6);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(6);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(6);
				break;
			case 15:
			case 11:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(7);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(7);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(7);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(7);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(7);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(7);
				break;
			case 16:
			case 12:
				if (_vm->_globals->_actionDirection == DIR_UP)
					_vm->_objectsMan->doActionBack(8);
				if (_vm->_globals->_actionDirection == DIR_RIGHT)
					_vm->_objectsMan->doActionRight(8);
				if (_vm->_globals->_actionDirection == DIR_UP_RIGHT)
					_vm->_objectsMan->doActionDiagRight(8);
				if (_vm->_globals->_actionDirection == DIR_DOWN)
					_vm->_objectsMan->doActionFront(8);
				if (_vm->_globals->_actionDirection == DIR_UP_LEFT)
					_vm->_objectsMan->doActionDiagLeft(8);
				if (_vm->_globals->_actionDirection == DIR_LEFT)
					_vm->_objectsMan->doActionLeft(8);
				break;
			default:
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
		uint frameNumb = READ_LE_UINT16(dataP + 5) / _vm->_globals->_speed;
		if (!frameNumb)
			frameNumb = 1;
		for (uint i = 0; i < frameNumb + 1; i++) {
			if (_vm->shouldQuit())
				return -1; // Exiting game

			_vm->_events->refreshScreenAndEvents();
		}
		opcodeType = 1;
		break;
		}
	case MKTAG24('O', 'B', 'P'):
		opcodeType = 1;
		_vm->_objectsMan->addObject(READ_LE_INT16(dataP + 5));
		break;
	case MKTAG24('O', 'B', 'M'):
		opcodeType = 1;
		_vm->_objectsMan->removeObject(READ_LE_INT16(dataP + 5));
		break;
	case MKTAG24('G', 'O', 'T'):
		opcodeType = 2;
		break;
	case MKTAG24('Z', 'O', 'N'):
		_vm->_linesMan->enableZone(READ_LE_INT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('Z', 'O', 'F'):
		_vm->_linesMan->disableZone(READ_LE_INT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('E', 'X', 'I'):
		opcodeType = 5;
		break;
	case MKTAG24('S', 'O', 'R'):
		_vm->_globals->_exitId = READ_LE_INT16(dataP + 5);
		opcodeType = 5;
		break;
	case MKTAG24('B', 'C', 'A'):
		_vm->_objectsMan->disableHidingItem(READ_LE_INT16(dataP + 5));
		opcodeType = 1;
		break;
	case MKTAG24('A', 'N', 'I'): {
		int animId = READ_LE_INT16(dataP + 5);
		if (animId <= 100)
			_vm->_objectsMan->setBobAnimation(animId);
		else
			_vm->_objectsMan->stopBobAnimation(animId - 100);
		opcodeType = 1;
		break;
		}
	case MKTAG24('S', 'P', 'E'):
		switch (READ_LE_INT16(dataP + 5)) {
		case 6:
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setAndPlayAnim(20, 0, 14, true);
			break;

		case 7:
			_vm->_talkMan->startAnimatedCharacterDialogue("rueh1.pe2");
			break;

		case 8:
			_vm->_talkMan->startAnimatedCharacterDialogue("ruef1.pe2");
			break;

		case 10:
			_vm->_talkMan->startAnimatedCharacterDialogue("bqeflic1.pe2");
			break;

		case 11:
			_vm->_talkMan->startAnimatedCharacterDialogue("bqeflic2.pe2");
			break;

		case 12:
			// Bank - negotiations between Hopkins and one of the killers
			_vm->_fontMan->hideText(9);
			_vm->_events->refreshScreenAndEvents();
			_vm->_events->refreshScreenAndEvents();
			_vm->_talkMan->startAnimatedCharacterDialogue("bqetueur.pe2");
			break;

		case 13:
			// Bank - after negotiations, Hopkins enters the bank
			_vm->_events->_mouseButton = _vm->_events->_curMouseButton;
			_vm->_globals->_disableInventFl = true;
			_vm->_graphicsMan->fadeOutLong();
			_vm->_objectsMan->disableHidingBehavior();
			_vm->_objectsMan->removeSprite(0);
			_vm->_fontMan->hideText(5);
			_vm->_fontMan->hideText(9);
			_vm->_graphicsMan->endDisplayBob();
			_vm->_objectsMan->clearScreen();

			if ((_vm->getPlatform() != Common::kPlatformWindows) || !_vm->getIsDemo()) {
				_vm->_soundMan->playSoundFile("SOUND17.WAV");
				_vm->_graphicsMan->_fadingFl = true;
				_vm->_animMan->playSequence2("HELICO.SEQ", 10, 4, 10);
			}

			_vm->_animMan->loadAnim("otage");
			_vm->_graphicsMan->loadImage("IM05");
			_vm->_graphicsMan->displayAllBob();

			for (int i = 0; i <= 4; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			}

			_vm->_events->mouseOff();
			_vm->_graphicsMan->fadeInDefaultLength(_vm->_graphicsMan->_frontBuffer);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(3) != 100);
			_vm->_graphicsMan->fadeOutDefaultLength(_vm->_graphicsMan->_frontBuffer);
			_vm->_graphicsMan->endDisplayBob();

			// If uncensored, rip the throat of the hostage
			if (!_vm->_globals->_censorshipFl) {
				_vm->_soundMan->_specialSoundNum = 16;
				_vm->_graphicsMan->_fadingFl = true;
				_vm->_animMan->playAnim("EGORGE.ANM", "EGORGE.ANM", 50, 28, 500);
				_vm->_soundMan->_specialSoundNum = 0;
			}
			_vm->_animMan->loadAnim("ASCEN");
			_vm->_events->mouseOff();
			_vm->_graphicsMan->loadImage("ASCEN");
			_vm->_graphicsMan->displayAllBob();

			for (int i = 0; i <= 4; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			}

			_vm->_events->mouseOff();
			_vm->_graphicsMan->fadeInDefaultLength(_vm->_graphicsMan->_frontBuffer);
			_vm->_objectsMan->checkEventBobAnim(1, 0, 17, 3);
			_vm->_graphicsMan->fadeOutDefaultLength(_vm->_graphicsMan->_frontBuffer);
			_vm->_graphicsMan->endDisplayBob();

			if ((_vm->getPlatform() == Common::kPlatformWindows) && _vm->getIsDemo())
				_vm->_soundMan->playSoundFile("SOUND17.WAV");

			_vm->_soundMan->_specialSoundNum = 14;
			_vm->_graphicsMan->_fadingFl = true;
			_vm->_animMan->playSequence2("ASSOM.SEQ", 10, 4, 500);
			_vm->_soundMan->_specialSoundNum = 0;
			_vm->_globals->_disableInventFl = false;
			_vm->_objectsMan->_helicopterFl = true;
			break;

		case 16:
			_vm->_talkMan->startAnimatedCharacterDialogue("ftoubib.pe2");
			break;

		case 17:
			_vm->_talkMan->startAnimatedCharacterDialogue("flic2b.pe2");
			break;

		case 18:
			_vm->_talkMan->startAnimatedCharacterDialogue("fjour.pe2");
			break;

		case 20:
			_vm->_talkMan->startAnimatedCharacterDialogue("PUNK.pe2");
			break;

		case 21:
			_vm->_talkMan->startAnimatedCharacterDialogue("MEDLEG.pe2");
			break;

		case 22:
			_vm->_talkMan->animateObject("CADAVRE1.pe2");
			break;

		case 23:
			_vm->_talkMan->startStaticCharacterDialogue("CHERCHE1.pe2");
			break;

		case 25:
			_vm->_talkMan->startAnimatedCharacterDialogue("AGENT1.pe2");
			break;

		case 26:
			_vm->_talkMan->startAnimatedCharacterDialogue("AGENT2.pe2");
			break;

		case 27:
			if (_vm->_globals->_saveData->_data[svField94] != 1 || _vm->_globals->_saveData->_data[svField95] != 1)
				_vm->_talkMan->startAnimatedCharacterDialogue("STANDAR.pe2");
			else
				_vm->_talkMan->startAnimatedCharacterDialogue("STANDAR1.pe2");
			break;

		case 29:
			_vm->_globals->_disableInventFl = true;
			_vm->_talkMan->animateObject("TELEP.pe2");
			_vm->_globals->_disableInventFl = false;
			break;

		case 32:
			_vm->_talkMan->startAnimatedCharacterDialogue("SAMAN.pe2");
			break;

		case 35:
			if (!_vm->_soundMan->_soundOffFl) {
				do {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					_vm->_events->refreshScreenAndEvents();
				} while (_vm->_soundMan->_soundFl);
			}
			_vm->_talkMan->startAnimatedCharacterDialogue("PTLAB.pe2");
			break;

		case 36:
			if (_vm->_globals->_saveData->_data[svField270] == 2 && _vm->_globals->_saveData->_data[svField94] == 1 && _vm->_globals->_saveData->_data[svField95] == 1)
				_vm->_globals->_saveData->_data[svField270] = 3;

			switch (_vm->_globals->_saveData->_data[svField270]) {
			case 0:
				_vm->_talkMan->startStaticCharacterDialogue("PATRON0.pe2");
				break;
			case 1:
				_vm->_talkMan->startStaticCharacterDialogue("PATRON1.pe2");
				break;
			case 2:
				_vm->_talkMan->startStaticCharacterDialogue("PATRON2.pe2");
				break;
			case 3:
				_vm->_talkMan->startStaticCharacterDialogue("PATRON3.pe2");
				break;
			default:
				if (_vm->_globals->_saveData->_data[svField270] > 3) {
					_vm->_talkMan->startStaticCharacterDialogue("PATRON4.pe2");
					_vm->_globals->_saveData->_data[svField270] = 5;
				}
				break;
			}
			break;

		case 37:
			_vm->_graphicsMan->_fadingFl = true;
			_vm->_animMan->playSequence2("corde.SEQ", 32, 32, 100);
			_vm->_graphicsMan->_noFadingFl = true;
			break;

		case 38:
			_vm->_soundMan->loadSample(1, "SOUND44.WAV");
			_vm->_soundMan->loadSample(2, "SOUND42.WAV");
			_vm->_soundMan->loadSample(3, "SOUND41.WAV");
			_vm->_soundMan->_specialSoundNum = 17;
			_vm->_animMan->playSequence("grenade.SEQ", 1, 32, 100, false, false);
			_vm->_soundMan->_specialSoundNum = 0;
			_vm->_graphicsMan->_fadingFl = true;
			_vm->_animMan->playAnim("CREVE17.ANM", "CREVE17.ANM", 24, 24, 200);
			_vm->_soundMan->removeSample(1);
			_vm->_soundMan->removeSample(2);
			_vm->_soundMan->removeSample(3);
			_vm->_graphicsMan->_noFadingFl = true;
			break;

		case 40:
			_vm->_talkMan->startAnimatedCharacterDialogue("MAGE.pe2");
			break;

		case 41:
			_vm->_talkMan->startAnimatedCharacterDialogue("MORT3.pe2");
			break;

		case 42:
			_vm->_talkMan->startAnimatedCharacterDialogue("MORT2.pe2");
			break;

		case 43:
			_vm->_talkMan->startAnimatedCharacterDialogue("MORT1.pe2");
			break;

		case 44:
			_vm->_talkMan->startAnimatedCharacterDialogue("MORT3A.pe2");
			break;

		case 45:
			_vm->_talkMan->startAnimatedCharacterDialogue("FEM3.pe2");
			break;

		case 46: {
			_vm->_globals->_checkDistanceFl = true;
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 564, 420);
			_vm->_objectsMan->_zoneNum = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_objectsMan->removeSprite(0);
			_vm->_globals->_checkDistanceFl = true;
			_vm->_soundMan->loadSample(1, "SOUND44.WAV");
			_vm->_soundMan->loadSample(2, "SOUND45.WAV");
			_vm->_objectsMan->setMultiBobAnim(9, 10, 0, 0);
			bool playFl = false;
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				switch (_vm->_objectsMan->getBobAnimDataIdx(9)) {
				case 4:
				case 16:
				case 28:
					if (!playFl) {
						_vm->_soundMan->playSample(1);
						playFl = true;
					}
					break;
				case 5:
				case 17:
				case 29:
					playFl = false;
					break;
				default:
					break;
				}

				switch (_vm->_objectsMan->getBobAnimDataIdx(10)) {
				case 10:
				case 22:
				case 33:
					if (!playFl) {
						_vm->_soundMan->playSample(2);
						playFl = true;
					}
					break;
				case 11:
					playFl = false;
					break;
				case 12:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 249, 1);
					break;
				case 23:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 249, 2);
					playFl = false;
					break;
				case 34:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 249, 3);
					playFl = false;
					break;
				default:
					break;
				}
				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(9) == 36)
					break;
			}
			_vm->_objectsMan->animateSprite(0);
			_vm->_objectsMan->stopBobAnimation(9);
			_vm->_objectsMan->stopBobAnimation(10);
			_vm->_soundMan->removeSample(1);
			_vm->_soundMan->removeSample(2);
			break;
			}

		case 47:
			_vm->_talkMan->startAnimatedCharacterDialogue("BARMAN.pe2");
			break;

		case 48:
			_vm->_talkMan->startAnimatedCharacterDialogue("SAMAN2.pe2");
			break;

		case 49: {
			_vm->_objectsMan->disableHidingBehavior();
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setMultiBobAnim(9, 10, 0, 0);

			int endIdx;
			if (_vm->_globals->_saveData->_data[svField133] == 1)
				endIdx = 41;
			else
				endIdx = 12;
			bool playFl = false;
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				switch (_vm->_objectsMan->getBobAnimDataIdx(9)) {
				case 4:
				case 18:
					if (!playFl) {
						_vm->_soundMan->directPlayWav("SOUND44.WAV");
						playFl = true;
					}
					break;
				case 5:
				case 9:
					playFl = false;
					break;
				default:
					break;
				}

				switch (_vm->_objectsMan->getBobAnimDataIdx(10)) {
				case 11:
					if (!playFl) {
						_vm->_soundMan->directPlayWav("SOUND45.WAV");
						playFl = true;
					}
					break;
				case 12:
					playFl = false;
					break;
				default:
					break;
				}
				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(9) == endIdx)
					break;
			}

			if (endIdx == 12) {
				_vm->_objectsMan->animateSprite(0);
				_vm->_objectsMan->stopBobAnimation(9);
			}
			_vm->_objectsMan->enableHidingBehavior();
			break;
			}

		case 50:
			_vm->_soundMan->playSoundFile("SOUND46.WAv");
			_vm->_objectsMan->setAndPlayAnim(11, 0, 23, false);
			break;

		case 51: {
			_vm->_graphicsMan->fadeOutLong();
			_vm->_objectsMan->disableHidingBehavior();
			_vm->_objectsMan->removeSprite(0);
			_vm->_fontMan->hideText(5);
			_vm->_fontMan->hideText(9);
			_vm->_graphicsMan->endDisplayBob();
			_vm->_graphicsMan->loadImage("IM20f");
			_vm->_animMan->loadAnim("ANIM20f");
			_vm->_graphicsMan->displayAllBob();
			_vm->_events->mouseOff();
			_vm->_graphicsMan->fadeInLong();
			_vm->_soundMan->loadWav("SOUND46.WAV", 1);
			bool playFl = false;
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsMan->getBobAnimDataIdx(12) == 5 && !playFl) {
					_vm->_soundMan->playWav(1);
					playFl = true;
				}
				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(12) == 34)
					break;
			}
			_vm->_objectsMan->stopBobAnimation(2);
			_vm->_graphicsMan->fadeOutLong();
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_globals->_exitId = 20;
			break;
			}

		case 52:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("GARDE.PE2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 53:
			_vm->_talkMan->startAnimatedCharacterDialogue("GARDE1.pe2");
			break;

		case 54:
			_vm->_talkMan->startAnimatedCharacterDialogue("GARDE2.pe2");
			break;

		case 55:
			_vm->_objectsMan->stopBobAnimation(1);
			_vm->_objectsMan->setAndPlayAnim(15, 0, 12, false);
			_vm->_objectsMan->stopBobAnimation(15);
			_vm->_objectsMan->loadLinkFile("IM19a", true);
			break;

		case 56:
			_vm->_globals->_characterSpriteBuf = _vm->_fileIO->loadFile("HOPFEM.SPR");
			_vm->_globals->_characterType = CHARACTER_HOPKINS_CLONE;
			_vm->_globals->_saveData->_data[svAlternateSpriteFl] = 1;
			_vm->_globals->loadCharacterData();
			_vm->_objectsMan->_sprite[0]._deltaX = 28;
			_vm->_objectsMan->_sprite[0]._deltaY = 155;
			_vm->_objectsMan->_sprite[0]._spriteData = _vm->_globals->_characterSpriteBuf;
			_vm->_objectsMan->computeAndSetSpriteSize();
			break;

		case 57:
			_vm->_globals->_characterSpriteBuf = _vm->_fileIO->loadFile("PERSO.SPR");
			_vm->_globals->_characterType = CHARACTER_HOPKINS;
			_vm->_globals->_saveData->_data[svAlternateSpriteFl] = 0;
			_vm->_globals->loadCharacterData();
			_vm->_objectsMan->_sprite[0]._deltaX = 34;
			_vm->_objectsMan->_sprite[0]._deltaY = 190;
			_vm->_objectsMan->_sprite[0]._spriteData = _vm->_globals->_characterSpriteBuf;
			_vm->_objectsMan->computeAndSetSpriteSize();
			break;

		case 58:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("Gm1.PE2");
			_vm->_globals->_saveData->_data[svField176] = 1;
			_vm->_globals->_saveData->_data[svField270] = 2;
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 59: {
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_oldCharacterPosX = _vm->_objectsMan->getSpriteX(0);
			_vm->_objectsMan->resetOldDirection();
			_vm->_objectsMan->resetHomeRateCounter();
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 445, 332);
			_vm->_globals->_checkDistanceFl = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(7);
			_vm->_objectsMan->setBobAnimDataIdx(7, 0);
			bool playFl = false;
			_vm->_soundMan->loadSample(1, "SOUND40.WAV");
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				switch (_vm->_objectsMan->getBobAnimDataIdx(7)) {
				case 10:
				case 18:
					if (!playFl) {
						_vm->_soundMan->playSample(1);
						playFl = true;
					}
					break;
				case 11:
					playFl = false;
					break;
				case 19:
					_vm->_objectsMan->setBobAnimation(3);
					playFl = false;
					break;
				default:
					break;
				}
				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(3) == 48)
					break;
			}
			_vm->_soundMan->removeSample(1);
			_vm->_objectsMan->setSpriteIndex(0, 62);
			_vm->_objectsMan->animateSprite(0);
			_vm->_objectsMan->setBobAnimation(6);
			_vm->_objectsMan->stopBobAnimation(7);
			_vm->_objectsMan->stopBobAnimation(3);
			break;
			}

		case 62:
			_vm->_talkMan->animateObject("SBCADA.pe2");
			break;

		case 65:
			_vm->_talkMan->animateObject("ScCADA.pe2");
			break;

		case 80: {
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(12);
			_vm->_objectsMan->setBobAnimation(13);
			_vm->_objectsMan->setBobAnimDataIdx(12, 0);
			_vm->_objectsMan->setBobAnimDataIdx(13, 0);
			_vm->_soundMan->loadWav("SOUND44.WAV", 1);
			_vm->_soundMan->loadWav("SOUND71.WAV", 2);
			bool playFl = false;
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				switch (_vm->_objectsMan->getBobAnimDataIdx(12)) {
				case 4:
					if (!playFl) {
						_vm->_soundMan->playWav(1);
						playFl = true;
					}
					break;
				case 5:
					playFl = false;
					break;
				default:
					break;
				}

				switch (_vm->_objectsMan->getBobAnimDataIdx(4)) {
				case 5:
					if (!playFl) {
						_vm->_soundMan->playWav(2);
						playFl = true;
					}
					break;
				case 6:
					playFl = false;
					break;
				default:
					break;
				}

				if (_vm->_objectsMan->getBobAnimDataIdx(13) == 8) {
					_vm->_objectsMan->stopBobAnimation(13);
					_vm->_objectsMan->stopBobAnimation(3);
					_vm->_objectsMan->setBobAnimation(4);
					_vm->_objectsMan->setBobAnimDataIdx(4, 0);
					_vm->_objectsMan->setBobAnimDataIdx(13, 0);
				}

				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(4) == 16)
					break;
			}
			_vm->_objectsMan->stopBobAnimation(12);
			_vm->_objectsMan->stopBobAnimation(4);
			_vm->_objectsMan->animateSprite(0);
			_vm->_objectsMan->loadLinkFile("IM27a", true);
			break;
			}

		case 81: {
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_oldCharacterPosX = _vm->_objectsMan->getSpriteX(0);
			_vm->_objectsMan->resetOldDirection();
			_vm->_objectsMan->resetHomeRateCounter();
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 119, 268);
			_vm->_globals->_checkDistanceFl = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(11);
			_vm->_objectsMan->setBobAnimation(8);
			_vm->_objectsMan->setBobAnimDataIdx(11, 0);
			_vm->_objectsMan->setBobAnimDataIdx(8, 0);
			_vm->_soundMan->loadWav("SOUND44.WAV", 1);
			_vm->_soundMan->loadWav("SOUND48.WAV", 2);
			_vm->_soundMan->loadWav("SOUND49.WAV", 3);
			bool playFl = false;
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				switch (_vm->_objectsMan->getBobAnimDataIdx(11)) {
				case 4:
					if (!playFl) {
						_vm->_soundMan->playWav(1);
						playFl = true;
					}
					break;
				case 5:
					playFl = false;
					break;
				default:
					break;
				}

				switch (_vm->_objectsMan->getBobAnimDataIdx(8)) {
				case 11:
					if (!playFl) {
						_vm->_soundMan->playWav(2);
						playFl = true;
					}
					break;
				case 12:
					playFl = false;
					break;
				default:
					break;
				}

				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(8) == 32)
					break;
			}
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 201, 14, 1);
			_vm->_objectsMan->animateSprite(0);
			_vm->_objectsMan->stopBobAnimation(11);
			_vm->_objectsMan->stopBobAnimation(8);
			_vm->_objectsMan->setBobAnimation(5);
			_vm->_objectsMan->setBobAnimation(6);
			_vm->_objectsMan->setBobAnimDataIdx(5, 0);
			_vm->_objectsMan->setBobAnimDataIdx(6, 0);
			_vm->_soundMan->playWav(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(5) != 74);
			_vm->_objectsMan->stopBobAnimation(5);
			_vm->_objectsMan->stopBobAnimation(6);
			_vm->_objectsMan->setBobAnimation(9);
			_vm->_objectsMan->setBobAnimation(7);
			break;
			}

		case 83:
			_vm->_talkMan->startAnimatedCharacterDialogue("CVIGIL.pe2");
			break;

		case 84:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("CVIGIL1.PE2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 85:
			_vm->_objectsMan->stopBobAnimation(3);
			_vm->_objectsMan->setBobAnimation(5);
			_vm->_objectsMan->setBobAnimDataIdx(5, 0);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(5) != 6);
			_vm->_objectsMan->stopBobAnimation(5);
			_vm->_objectsMan->setBobAnimation(6);
			_vm->_objectsMan->loadLinkFile("IM24a", true);
			break;

		case 86:
			if (_vm->_globals->_saveData->_data[svField231] == 1) {
				_vm->_talkMan->startAnimatedCharacterDialogue("chotess1.pe2");
			} else {
				_vm->_globals->_introSpeechOffFl = true;
				_vm->_talkMan->startAnimatedCharacterDialogue("chotesse.pe2");
				_vm->_globals->_introSpeechOffFl = false;
			}
			break;

		case 87:
			if (_vm->_globals->_saveData->_data[svField188])
				_vm->_talkMan->startAnimatedCharacterDialogue("stand2.pe2");
			else
				_vm->_talkMan->startAnimatedCharacterDialogue("stand1.pe2");
			break;

		case 88:
			// Shooting target - Shooting at target
			if (_vm->_globals->_saveData->_data[svField183] == 1) {
				_vm->_objectsMan->setBobAnimDataIdx(1, 0);
				_vm->_objectsMan->setBobAnimDataIdx(2, 0);
				_vm->_objectsMan->setBobAnimation(1);
				_vm->_objectsMan->setBobAnimation(2);
				_vm->_soundMan->loadSample(1, "SOUND40.WAV");
				bool playFl = false;
				for (;;) {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					switch (_vm->_objectsMan->getBobAnimDataIdx(1)) {
					case 1:
					case 3:
					case 5:
					case 7:
						if (!playFl) {
							_vm->_soundMan->playSample(1);
							playFl = true;
						}
						break;
					case 2:
					case 4:
					case 6:
					case 8:
						playFl = false;
						break;
					default:
						break;
					}
					_vm->_events->refreshScreenAndEvents();
					if (_vm->_objectsMan->getBobAnimDataIdx(1) == 9)
						break;
				}
				_vm->_objectsMan->stopBobAnimation(1);
				_vm->_objectsMan->stopBobAnimation(2);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 283, 160, 6);
				_vm->_soundMan->removeSample(1);
			}
			if (_vm->_globals->_saveData->_data[svField183] == 2) {
				_vm->_objectsMan->setBobAnimDataIdx(1, 0);
				_vm->_objectsMan->setBobAnimDataIdx(3, 0);
				_vm->_objectsMan->setBobAnimation(1);
				_vm->_objectsMan->setBobAnimation(3);
				_vm->_soundMan->loadSample(1, "SOUND40.WAV");
				bool playFl = false;
				for (;;) {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					switch (_vm->_objectsMan->getBobAnimDataIdx(1)) {
					case 1:
					case 3:
					case 5:
					case 7:
						if (!playFl) {
							_vm->_soundMan->playSample(1);
							playFl = true;
						}
						break;
					case 2:
					case 4:
					case 6:
					case 8:
						playFl = false;
						break;
					default:
						break;
					}
					_vm->_events->refreshScreenAndEvents();
					if (_vm->_objectsMan->getBobAnimDataIdx(1) == 9)
						break;
				}
				_vm->_objectsMan->stopBobAnimation(1);
				_vm->_objectsMan->stopBobAnimation(3);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 283, 161, 8);
				_vm->_soundMan->removeSample(1);
			}
			break;

		case 90:
			// Shooting target - Using the level
			_vm->_soundMan->playSoundFile("SOUND52.WAV");
			if (!_vm->_globals->_saveData->_data[svField186]) {
				_vm->_animMan->playSequence("CIB5A.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 0);
			} else if (_vm->_globals->_saveData->_data[svField186] == 1) {
				_vm->_animMan->playSequence("CIB5C.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 0);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 329, 87, 2);
			}
			break;

		case 91:
			_vm->_soundMan->playSoundFile("SOUND52.WAV");
			if (!_vm->_globals->_saveData->_data[svField186]) {
				_vm->_animMan->playSequence("CIB5B.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 5);
			} else if (_vm->_globals->_saveData->_data[svField186] == 1) {
				_vm->_animMan->playSequence("CIB5D.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 5);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 283, 160, 6);
			}
			break;

		case 92:
			_vm->_soundMan->playSoundFile("SOUND52.WAV");
			if (!_vm->_globals->_saveData->_data[svField184]) {
				_vm->_animMan->playSequence("CIB6A.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 0);
			} else if (_vm->_globals->_saveData->_data[svField184] == 1) {
				_vm->_animMan->playSequence("CIB6C.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 0);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 293, 139, 3);
			}
			break;

		case 93:
			_vm->_soundMan->playSoundFile("SOUND52.WAV");
			if (!_vm->_globals->_saveData->_data[svField184]) {
				_vm->_animMan->playSequence("CIB6B.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 5);
			} else if (_vm->_globals->_saveData->_data[svField184] == 1) {
				_vm->_animMan->playSequence("CIB6D.SEQ", 1, 12, 1, false, false);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 155, 29, 5);
				_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 283, 161, 8);
			}
			break;

		case 94:
			if (!_vm->_globals->_saveData->_data[svField228])
				_vm->_talkMan->startAnimatedCharacterDialogue("flicn.pe2");
			else if (_vm->_globals->_saveData->_data[svField228] == 1)
				_vm->_talkMan->startAnimatedCharacterDialogue("flicn1.pe2");
			break;

		case 95:
			_vm->_objectsMan->setBobAnimation(9);
			_vm->_objectsMan->setBobAnimation(10);
			_vm->_objectsMan->setBobAnimation(12);
			_vm->_objectsMan->setBobAnimDataIdx(9, 0);
			_vm->_objectsMan->setBobAnimDataIdx(10, 0);
			_vm->_objectsMan->setBobAnimDataIdx(12, 0);
			_vm->_objectsMan->removeSprite(0);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(9) != 15);
			_vm->_objectsMan->stopBobAnimation(9);
			_vm->_objectsMan->animateSprite(0);
			_vm->_soundMan->playSoundFile("SOUND50.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(12) != 117);
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 830, 122, 0);
			_vm->_objectsMan->stopBobAnimation(12);
			_vm->_objectsMan->stopBobAnimation(10);
			_vm->_objectsMan->setBobAnimation(11);
			break;

		case 98:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("CVIGIL2.PE2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 100:
			_vm->_talkMan->startAnimatedCharacterDialogue("tourist.pe2");
			break;

		case 101:
			_vm->_talkMan->startAnimatedCharacterDialogue("tahi1.pe2");
			break;

		case 103:
			// Dice game
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("tourist1.pe2");
			_vm->_globals->_introSpeechOffFl = false;
				_vm->_animMan->playAnim2("T421A.ANM", "T421.ANM", 100, 14, 500);
			_vm->_events->refreshScreenAndEvents();
			_vm->_events->refreshScreenAndEvents();
			_vm->_events->refreshScreenAndEvents();
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("tourist2.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 104:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("tourist3.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 105:
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_oldCharacterPosX = _vm->_objectsMan->getSpriteX(0);
			_vm->_objectsMan->resetOldDirection();
			_vm->_objectsMan->resetHomeRateCounter();
			_vm->_linesMan->_route = (RouteItem *)NULL;
			switch (_vm->_globals->_saveData->_data[svField253]) {
			case 1:
				_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 201, 294);
				break;
			case 2:
				_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 158, 338);
				break;
			default:
				if (_vm->_globals->_saveData->_data[svField253] > 2)
					_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 211, 393);
				break;
			}
			_vm->_globals->_checkDistanceFl = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setSpriteIndex(0, 60);
			_vm->_soundMan->loadSample(1, "SOUND63.WAV");
			if (_vm->_globals->_saveData->_data[svField253] > 2) {
				_vm->_objectsMan->setBobAnimation(4);
				bool playFl = false;
				for (;;) {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					switch (_vm->_objectsMan->getBobAnimDataIdx(4)) {
					case 9:
					case 32:
					case 55:
						if (!playFl) {
							_vm->_soundMan->playSample(1);
							playFl = true;
						}
						break;
					case 10:
					case 33:
					case 56:
						playFl = false;
						break;
					default:
						break;
					}
					_vm->_events->refreshScreenAndEvents();
					if (_vm->_objectsMan->getBobAnimDataIdx(4) == 72)
						break;
				}
				_vm->_objectsMan->stopBobAnimation(4);
			}
			if (_vm->_globals->_saveData->_data[svField253] == 1) {
				_vm->_objectsMan->setBobAnimation(6);
				bool playFl = false;
				for (;;) {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					switch (_vm->_objectsMan->getBobAnimDataIdx(6)) {
					case 9:
					case 32:
					case 55:
						if (!playFl) {
							_vm->_soundMan->playSample(1);
							playFl = true;
						}
						break;
					case 10:
					case 33:
					case 56:
						playFl = false;
						break;
					default:
						break;
					}
					_vm->_events->refreshScreenAndEvents();
					if (_vm->_objectsMan->getBobAnimDataIdx(6) == 72)
						break;
				}
				_vm->_objectsMan->stopBobAnimation(6);
			}
			if (_vm->_globals->_saveData->_data[svField253] == 2) {
				_vm->_objectsMan->setBobAnimation(5);
				bool playFl = false;
				for (;;) {
					if (_vm->shouldQuit())
						return -1; // Exiting game

					switch (_vm->_objectsMan->getBobAnimDataIdx(5)) {
					case 9:
					case 32:
					case 55:
						if (!playFl) {
							_vm->_soundMan->playSample(1);
							playFl = true;
						}
						break;
					case 10:
					case 33:
					case 56:
						playFl = false;
						break;
					default:
						break;
					}
					_vm->_events->refreshScreenAndEvents();
					if (_vm->_objectsMan->getBobAnimDataIdx(5) == 72)
						break;
				}
				_vm->_objectsMan->stopBobAnimation(5);
			}
			_vm->_objectsMan->animateSprite(0);
			_vm->_objectsMan->doActionBack(1);
			_vm->_soundMan->removeSample(1);
			break;

		case 106:
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(4);
			_vm->_objectsMan->setBobAnimDataIdx(4, 0);
			_vm->_soundMan->loadWav("SOUND61.WAV", 1);
			_vm->_soundMan->loadWav("SOUND62.WAV", 2);
			_vm->_soundMan->loadWav("SOUND61.WAV", 3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(4) != 10);
			_vm->_soundMan->playWav(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(4) != 18);
			_vm->_soundMan->playWav(2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(4) != 62);
			_vm->_soundMan->playWav(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(4) != 77);
			_vm->_objectsMan->stopBobAnimation(4);
			_vm->_objectsMan->animateSprite(0);
			break;

		case 107:
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(5);
			_vm->_objectsMan->setBobAnimDataIdx(5, 0);
			_vm->_soundMan->loadWav("SOUND61.WAV", 1);
			_vm->_soundMan->loadWav("SOUND62.WAV", 2);
			_vm->_soundMan->loadWav("SOUND61.WAV", 3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(5) != 10);
			_vm->_soundMan->playWav(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(5) != 18);
			_vm->_soundMan->playWav(2);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(5) != 38);
			_vm->_soundMan->playWav(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(5) != 53);
			_vm->_objectsMan->stopBobAnimation(5);
			_vm->_objectsMan->animateSprite(0);
			break;

		case 108:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("peche1.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 109:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("peche2.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 110:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("peche3.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 111:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("peche4.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 112:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("teint1.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 113:
			_vm->_talkMan->startAnimatedCharacterDialogue("teint.pe2");
			break;

		case 114:
			_vm->_talkMan->startAnimatedCharacterDialogue("tahibar.pe2");
			break;

		case 115:
			_vm->_talkMan->startAnimatedCharacterDialogue("ilebar.pe2");
			break;

		case 116:
			_vm->_talkMan->startAnimatedCharacterDialogue("Profred.pe2");
			break;

		case 170:
			_vm->_talkMan->startAnimatedCharacterDialogue("GRED.pe2");
			break;

		case 171: {
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("gred1.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_oldCharacterPosX = _vm->_objectsMan->getSpriteX(0);
			_vm->_objectsMan->resetOldDirection();
			_vm->_objectsMan->resetHomeRateCounter();
			_vm->_globals->_checkDistanceFl = true;
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 361, 325);
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_zoneNum = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_globals->_exitId = 59;
			break;
			}

		case 172:
			_vm->_talkMan->startAnimatedCharacterDialogue("GBLEU.pe2");
			break;

		case 173: {
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("gbleu1.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_oldCharacterPosX = _vm->_objectsMan->getSpriteX(0);
			_vm->_objectsMan->resetOldDirection();
			_vm->_objectsMan->resetHomeRateCounter();
			_vm->_globals->_checkDistanceFl = true;
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 361, 325);
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_zoneNum = -1;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_globals->_exitId = 59;
			break;
			}

		case 174:
			_vm->_talkMan->startAnimatedCharacterDialogue("Profbl.pe2");
			break;

		case 175:
			_vm->_objectsMan->setSpriteIndex(0, 55);
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(9);
			_vm->_objectsMan->setBobAnimation(10);
			_vm->_objectsMan->setBobOffset(10, 300);
			_vm->_soundMan->playSoundFile("SOUND44.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(10) != 7);
			_vm->_objectsMan->setBobAnimation(6);
			_vm->_objectsMan->stopBobAnimation(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(6) != 10);
			_vm->_soundMan->playSoundFile("SOUND71.WAV");
			_vm->_objectsMan->setBobAnimation(7);
			_vm->_objectsMan->stopBobAnimation(4);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(7) != 15);
			_vm->_objectsMan->stopBobAnimation(5);
			_vm->_objectsMan->setBobAnimation(8);
			_vm->_soundMan->playSoundFile("SOUND70.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(8) != 76);
			_vm->_objectsMan->stopBobAnimation(6);
			_vm->_objectsMan->stopBobAnimation(7);
			_vm->_objectsMan->stopBobAnimation(8);
			_vm->_objectsMan->stopBobAnimation(9);
			_vm->_objectsMan->stopBobAnimation(10);
			_vm->_objectsMan->animateSprite(0);
			break;

		case 176:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("gred2.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 177:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("gbleu2.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 200:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("Gm2.PE2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 201:
			_vm->_objectsMan->setBobAnimation(3);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(3) != 18);
			_vm->_objectsMan->stopBobAnimation(3);
			_vm->_objectsMan->setBobAnimation(4);
			break;

		case 202:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("SVGARD2.PE2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 203:
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(4);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(4) == 18)
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 18, 334, 0, false);
			} while (_vm->_objectsMan->getBobAnimDataIdx(4) != 26);
			_vm->_objectsMan->stopBobAnimation(4);
			_vm->_objectsMan->animateSprite(0);
			break;

		case 204: {
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(3);
			_vm->_soundMan->loadWav("SOUND67.WAV", 1);
			bool playFl = false;
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				switch (_vm->_objectsMan->getBobAnimDataIdx(3)) {
				case 10:
					if (!playFl) {
						_vm->_soundMan->playWav(1);
						playFl = true;
					}
					break;
				case 11:
					playFl = false;
					break;
				default:
					break;
				}
				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(3) == 50)
					break;
			}
			_vm->_objectsMan->stopBobAnimation(3);
			_vm->_objectsMan->animateSprite(0);
			break;
			}

		case 205: {
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(4);
			_vm->_soundMan->loadWav("SOUND69.WAV", 1);
			bool playFl = false;
			for (;;) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				switch (_vm->_objectsMan->getBobAnimDataIdx(4)) {
				case 10:
					if (!playFl) {
						_vm->_soundMan->playWav(1);
						playFl = true;
					}
					break;
				case 11:
					playFl = false;
					break;
				default:
					break;
				}
				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(4) == 24)
					break;
			}
			_vm->_objectsMan->stopBobAnimation(4);
			_vm->_objectsMan->animateSprite(0);
			break;
			}

		case 207:
			_vm->_talkMan->animateObject("PANNEAU.PE2");
			break;

		case 208: {
			_vm->_globals->_disableInventFl = true;
			if (_vm->_globals->_saveData->_data[svLastPrevScreenId] != _vm->_globals->_saveData->_data[svField401]) {
				_vm->_soundMan->_specialSoundNum = 208;
				_vm->_animMan->playSequence("SORT.SEQ", 10, 4, 10, true, false);
				_vm->_soundMan->_specialSoundNum = 0;
			}
			_vm->_globals->_checkDistanceFl = true;
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 330, 418);
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_zoneNum = 0;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_objectsMan->setSpriteIndex(0, 64);
			_vm->_globals->_exitId = _vm->_globals->_saveData->_data[svField401];
			_vm->_globals->_disableInventFl = false;
			break;
			}

		case 209: {
			_vm->_objectsMan->setBobAnimDataIdx(1, 0);
			_vm->_objectsMan->setBobAnimDataIdx(2, 0);
			_vm->_objectsMan->setSpriteIndex(0, 60);
			_vm->_objectsMan->stopBobAnimation(4);
			_vm->_objectsMan->setBobAnimation(1);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(1) != 9);
			_vm->_objectsMan->stopBobAnimation(1);
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_globals->_checkDistanceFl = true;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 330, 314);
			_vm->_objectsMan->_zoneNum = 0;
			_vm->_globals->_checkDistanceFl = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_objectsMan->setSpriteIndex(0, 64);
			_vm->_objectsMan->setBobAnimation(2);
			_vm->_soundMan->playSoundFile("SOUND66.WAV");
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(2) != 10);
			_vm->_objectsMan->stopBobAnimation(2);
			_vm->_objectsMan->setBobAnimation(4);
			break;
			}

		case 210:
			_vm->_soundMan->_specialSoundNum = 210;
			_vm->_animMan->playSequence2("SECRET1.SEQ", 1, 12, 1, true);
			_vm->_soundMan->_specialSoundNum = 0;
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 192, 152, 0);
			_vm->_objectsMan->setBobAnimation(9);
			_vm->_objectsMan->loadLinkFile("IM73a", true);
			_vm->_objectsMan->enableHidingBehavior();
			_vm->_objectsMan->setHidingUseCount(0);
			_vm->_objectsMan->setHidingUseCount(1);
			_vm->_graphicsMan->setColorPercentage2(252, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage2(253, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage2(251, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage2(254, 0, 0, 0);
			break;

		case 211:
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->disableHidingBehavior();
			_vm->_soundMan->_specialSoundNum = 211;
			_vm->_animMan->playSequence("SECRET2.SEQ", 1, 12, 100, false, true);
			_vm->_soundMan->_specialSoundNum = 0;
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_graphicsMan->fadeOutShort();

			for (int i = 1; i <= 39; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			}

			_vm->_graphicsMan->setColorPercentage2(252, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage2(253, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage2(251, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage2(254, 0, 0, 0);
			break;

		case 215:
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("aviat.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 216:
			// Discuss with pilot just before Flight cutscene
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("aviat1.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			break;

		case 229:
			_vm->_soundMan->_specialSoundNum = 229;
			_vm->_animMan->playSequence("MUR.SEQ", 1, 12, 1, false, false);
			_vm->_soundMan->_specialSoundNum = 0;
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 340, 157, 2);
			break;

		case 230: {
			_vm->_objectsMan->loadLinkFile("IM93a", true);
			_vm->_objectsMan->enableHidingBehavior();
			_vm->_globals->_checkDistanceFl = true;
			_vm->_objectsMan->_oldCharacterPosX = _vm->_objectsMan->getSpriteX(0);
			_vm->_objectsMan->resetOldDirection();
			_vm->_objectsMan->resetHomeRateCounter();
			_vm->_globals->_checkDistanceFl = true;
			_vm->_linesMan->_route = (RouteItem *)NULL;
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(_vm->_objectsMan->getSpriteX(0), _vm->_objectsMan->getSpriteY(0), 488, 280);
			_vm->_globals->_checkDistanceFl = true;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_objectsMan->goHome();
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_linesMan->_route != (RouteItem *)NULL);
			_vm->_objectsMan->removeSprite(0);
			bool playFl = false;
			_vm->_objectsMan->setBobAnimation(7);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				if (_vm->_objectsMan->getBobAnimDataIdx(7) == 9 && !playFl) {
					playFl = true;
					_vm->_soundMan->playSoundFile("SOUND81.WAV");
				}
				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(7) != 15);
			_vm->_objectsMan->stopBobAnimation(7);
			_vm->_objectsMan->setSpriteX(0, 476);
			_vm->_objectsMan->setSpriteY(0, 278);
			_vm->_objectsMan->animateSprite(0);
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 337, 154, 3);
			_vm->_objectsMan->loadLinkFile("IM93c", true);
			_vm->_objectsMan->enableHidingBehavior();
			break;
			}

		case 231:
			_vm->_objectsMan->disableHidingBehavior();
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(12);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(12) != 6);
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("PRMORT.pe2");
			_vm->_globals->_introSpeechOffFl = false;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(12) != 12);
			_vm->_objectsMan->animateSprite(0);
			_vm->_objectsMan->stopBobAnimation(12);
			_vm->_objectsMan->enableHidingBehavior();
			break;

		case 233: {
			_vm->_objectsMan->disableHidingBehavior();
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setBobAnimation(11);
			bool playFl = false;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
				if (_vm->_objectsMan->getBobAnimDataIdx(11) == 10 && !playFl)
					playFl = true;
			} while (_vm->_objectsMan->getBobAnimDataIdx(11) != 13);
			_vm->_objectsMan->stopBobAnimation(11);
			_vm->_objectsMan->enableHidingBehavior();
			_vm->_objectsMan->setBobAnimation(13);
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			} while (_vm->_objectsMan->getBobAnimDataIdx(13) != 48);
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("HRADIO.PE2");
			_vm->_globals->_introSpeechOffFl = false;
			_vm->_graphicsMan->fadeOutLong();
			_vm->_objectsMan->stopBobAnimation(13);
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_globals->_exitId = 94;
			break;
			}

		case 236: {
			if (_vm->_globals->_saveData->_data[svField341]) {
				switch (_vm->_globals->_saveData->_data[svField341]) {
				case 1:
					vbobFrameIndex = 6;
					break;
				case 2:
					vbobFrameIndex = 5;
					break;
				case 3:
					vbobFrameIndex = 4;
					break;
				default:
					break;
				}
				_vm->_soundMan->playSoundFile("SOUND83.WAV");
				_vm->_objectsMan->setAndPlayAnim(vbobFrameIndex, 26, 50, false);

				switch (_vm->_globals->_saveData->_data[svField341]) {
				case 1:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 27, 117, 0);
					_vm->_globals->_saveData->_data[svField338] = 0;
					break;
				case 2:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 145, 166, 2);
					_vm->_globals->_saveData->_data[svField339] = 0;
					break;
				case 3:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 296, 212, 4);
					_vm->_globals->_saveData->_data[svField340] = 0;
					break;
				default:
					break;
				}
			}
			_vm->_soundMan->playSoundFile("SOUND83.WAV");
			_vm->_objectsMan->setAndPlayAnim(6, 0, 23, false);
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 27, 117, 1);
			break;
			}

		case 237: {
			switch (_vm->_globals->_saveData->_data[svField341]) {
			case 1:
				vbobFrameIndex = 6;
				break;
			case 2:
				vbobFrameIndex = 5;
				break;
			case 3:
				vbobFrameIndex = 4;
				break;
			default:
				break;
			}

			if (_vm->_globals->_saveData->_data[svField341]) {
				_vm->_soundMan->playSoundFile("SOUND83.WAV");
				_vm->_objectsMan->setAndPlayAnim(vbobFrameIndex, 26, 50, false);

				switch (_vm->_globals->_saveData->_data[svField341]) {
				case 1:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 27, 117, 0);
					_vm->_globals->_saveData->_data[svField338] = 0;
					break;
				case 2:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 145, 166, 2);
					_vm->_globals->_saveData->_data[svField339] = 0;
					break;
				case 3:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 296, 212, 4);
					_vm->_globals->_saveData->_data[svField340] = 0;
					break;
				default:
					break;
				}
			}

			_vm->_soundMan->playSoundFile("SOUND83.WAV");
			_vm->_objectsMan->setAndPlayAnim(5, 0, 23, false);
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 145, 166, 3);
			break;
			}

		case 238: {
			switch (_vm->_globals->_saveData->_data[svField341]) {
			case 1:
				vbobFrameIndex = 6;
				break;
			case 2:
				vbobFrameIndex = 5;
				break;
			case 3:
				vbobFrameIndex = 4;
				break;
			default:
				break;
			}

			if (_vm->_globals->_saveData->_data[svField341]) {
				_vm->_soundMan->playSoundFile("SOUND83.WAV");
				_vm->_objectsMan->setAndPlayAnim(vbobFrameIndex, 26, 50, false);
				switch (_vm->_globals->_saveData->_data[svField341]) {
				case 1:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 27, 117, 0);
					_vm->_globals->_saveData->_data[svField338] = 0;
					break;
				case 2:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 145, 166, 2);
					_vm->_globals->_saveData->_data[svField339] = 0;
					break;
				case 3:
					_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 296, 212, 4);
					_vm->_globals->_saveData->_data[svField340] = 0;
					break;
				default:
					break;
				}
			}
			_vm->_soundMan->playSoundFile("SOUND83.WAV");
			_vm->_objectsMan->setAndPlayAnim(4, 0, 23, false);
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 296, 212, 5);
			break;
			}

		case 239:
			_vm->_objectsMan->removeSprite(0);
			_vm->_soundMan->playSoundFile("SOUND84.WAV");
			_vm->_objectsMan->setAndPlayAnim(16, 0, 10, false);
			break;

		case 240: {
			_vm->_objectsMan->setBobAnimation(1);
			bool soundFlag = false;
			do {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
				switch (_vm->_objectsMan->getBobAnimDataIdx(1)) {
				case 12:
					if (!soundFlag) {
						_vm->_soundMan->playSoundFile("SOUND86.WAV");
						soundFlag = true;
					}
					break;
				case 13:
				// The original was starting then stopping sound at 25
				// It looked wrong so the check was set on 26
				case 26:
					soundFlag = false;
					break;
				case 25:
					if (!soundFlag) {
						_vm->_soundMan->playSoundFile("SOUND85.WAV");
						soundFlag = true;
					}
					break;
				default:
					break;
				}
			} while (_vm->_objectsMan->getBobAnimDataIdx(1) != 32);
			_vm->_objectsMan->stopBobAnimation(1);
			_vm->_objectsMan->setBobAnimation(2);
			_vm->_fontMan->hideText(9);
			bool displayedTxtFl = false;
			if (!_vm->_soundMan->_textOffFl) {
				_vm->_fontMan->initTextBuffers(9, 617, _vm->_globals->_textFilename, 91, 41, 3, 30, 253);
				_vm->_fontMan->showText(9);
				displayedTxtFl = true;
			}
			if (!_vm->_soundMan->_voiceOffFl)
				_vm->_soundMan->mixVoice(617, 4, displayedTxtFl);
			for (int i = 0; i <= 29; i++) {
				if (_vm->shouldQuit())
					return -1; // Exiting game

				_vm->_events->refreshScreenAndEvents();
			}
			CharacterLocation *realHopkins = &_vm->_globals->_saveData->_realHopkins;
			realHopkins->_pos.x = _vm->_objectsMan->getSpriteX(0);
			realHopkins->_pos.y = _vm->_objectsMan->getSpriteY(0);
			realHopkins->_startSpriteIndex = 57;
			realHopkins->_location = 97;
			_vm->_globals->_saveData->_data[svHopkinsCloneFl] = 1;
			_vm->_globals->_saveData->_data[svField352] = 1;
			_vm->_globals->_saveData->_data[svField353] = 1;
			_vm->_globals->_saveData->_data[svField354] = 1;
			break;
			}

		case 241:
			_vm->_talkMan->startAnimatedCharacterDialogue("RECEP.PE2");
			break;

		// Resurrect Samantha's clone
		case 242: {
			_vm->_soundMan->playSoundFile("SOUND87.WAV");
			_vm->_animMan->playSequence("RESUF.SEQ", 1, 24, 1, false, true);

			CharacterLocation *samantha = &_vm->_globals->_saveData->_samantha;
			samantha->_pos.x = 404;
			samantha->_pos.y = 395;
			samantha->_startSpriteIndex = 64;
			samantha->_location = _vm->_globals->_screenId;
			samantha->_zoomFactor = -(100 * (67 - (100 - abs(_vm->_globals->_spriteSize[790 / 2]))) / 67);

			_vm->_globals->_saveData->_data[svField357] = 1;
			_vm->_globals->_saveData->_data[svField354] = 0;
			_vm->_globals->_saveData->_data[svField356] = 0;
			_vm->_globals->_saveData->_data[svField355] = 1;
			_vm->_objectsMan->_twoCharactersFl = true;
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 373, 191, 3);
			_vm->_objectsMan->addStaticSprite(_vm->_objectsMan->_headSprites, samantha->_pos, 1, 3, samantha->_zoomFactor, false, 20, 127);
			_vm->_objectsMan->animateSprite(1);
			break;
			}

		case 243:
			_vm->_soundMan->playSoundFile("SOUND88.WAV");
			if (_vm->_globals->_saveData->_data[svField341] == 2) {
				_vm->_animMan->playSequence("RESU.SEQ", 2, 24, 2, false, true);
			} else {
				_vm->_objectsMan->setAndPlayAnim(7, 0, 14, false);
			}
			break;

		case 245:
			_vm->_soundMan->playSoundFile("SOUND89.WAV");
			_vm->_objectsMan->setAndPlayAnim(5, 0, 6, false);
			_vm->_linesMan->_zone[4]._destX = 276;
			_vm->_objectsMan->enableVerb(4, 19);
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 285, 379, 0);
			_vm->_globals->_saveData->_data[svField399] = 1;
			break;

		case 246:
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->setAndPlayAnim(6, 0, 15, false);
			_vm->_objectsMan->_charactersEnabledFl = true;
			_vm->_graphicsMan->displayScreen(true);
			_vm->_animMan->playSequence2("TUNNEL.SEQ", 1, 18, 20, true);
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_graphicsMan->fadeOutLong();
			_vm->_objectsMan->_charactersEnabledFl = false;
			_vm->_globals->_exitId = 100;
			break;

		case 600:
			if (!_vm->getIsDemo()) {
				_vm->_graphicsMan->_fadingFl = true;
				_vm->_graphicsMan->_fadeDefaultSpeed = 1;
				_vm->_animMan->playAnim("BOMBE1A.ANM", "BOMBE1.ANM", 100, 18, 100);
			}
			_vm->_graphicsMan->loadImage("BOMBEB");
			_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);
			_vm->_graphicsMan->initScreen("BOMBE", 2, true);
			_vm->_graphicsMan->fadeInShort();
			break;

		case 601:
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 163, 7, false);
			_vm->_objectsMan->setAndPlayAnim(2, 0, 16, true);
			break;

		case 602:
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 163, 7, false);
			_vm->_objectsMan->setAndPlayAnim(4, 0, 16, true);
			break;

		case 603:
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 163, 7, false);
			_vm->_objectsMan->setAndPlayAnim(3, 0, 16, true);
			_vm->_soundMan->_specialSoundNum = 199;
			_vm->_graphicsMan->_fadingFl = true;
			_vm->_animMan->playAnim("BOMBE2A.ANM", "BOMBE2.ANM", 50, 14, 500);
			_vm->_soundMan->_specialSoundNum = 0;
			memset(_vm->_graphicsMan->_frontBuffer, 0, 614400);
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_globals->_exitId = 151;
			break;

		case 604:
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 163, 7, false);
			_vm->_objectsMan->setAndPlayAnim(1, 0, 16, true);
			_vm->_soundMan->_specialSoundNum = 199;
			_vm->_animMan->playAnim("BOMBE2A.ANM", "BOMBE2.ANM", 50, 14, 500);
			_vm->_soundMan->_specialSoundNum = 0;
			_vm->_graphicsMan->_noFadingFl = true;
			memset(_vm->_graphicsMan->_frontBuffer, 0, 614400);
			_vm->_globals->_exitId = 151;
			break;

		case 605:
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 163, 7, false);
			_vm->_objectsMan->setAndPlayAnim(5, 0, 16, true);
			_vm->_graphicsMan->fadeOutShort();
			_vm->_soundMan->_specialSoundNum = 199;
			_vm->_graphicsMan->_fadingFl = true;
			_vm->_animMan->playAnim("BOMBE2A.ANM", "BOMBE2.ANM", 50, 14, 500);
			_vm->_soundMan->_specialSoundNum = 0;
			_vm->_graphicsMan->_noFadingFl = true;
			memset(_vm->_graphicsMan->_frontBuffer, 0, 614400);
			_vm->_globals->_exitId = 151;
			break;

		case 606:
			_vm->_graphicsMan->fastDisplay(_vm->_globals->_levelSpriteBuf, 513, 163, 7, false);
			_vm->_objectsMan->setAndPlayAnim(6, 0, 16, true);
			if ((_vm->getPlatform() != Common::kPlatformWindows) || !_vm->getIsDemo()) {
				_vm->_animMan->playAnim("BOMBE3A.ANM", "BOMBE3.ANM", 50, 14, 500);
				memset(_vm->_graphicsMan->_frontBuffer, 0, 614400);
			}
			_vm->_globals->_exitId = 6;
			break;

		case 607:
			// Display bomb plan
			if (!_vm->getIsDemo()) {
				memcpy(_vm->_graphicsMan->_oldPalette, _vm->_graphicsMan->_palette, 769);
				_vm->_animMan->playAnim2("PLAN.ANM", "PLAN.ANM", 50, 10, 800);
			}
			_vm->_graphicsMan->resetDirtyRects();
			break;

		case 608:
			_vm->_objectsMan->stopBobAnimation(2);
			_vm->_objectsMan->stopBobAnimation(3);
			_vm->_objectsMan->stopBobAnimation(4);
			_vm->_objectsMan->stopBobAnimation(6);
			_vm->_objectsMan->stopBobAnimation(11);
			_vm->_objectsMan->stopBobAnimation(10);
			break;

		case 609:
			_vm->_objectsMan->setBobAnimation(2);
			_vm->_objectsMan->setBobAnimation(3);
			_vm->_objectsMan->setBobAnimation(4);
			_vm->_objectsMan->setBobAnimation(6);
			_vm->_objectsMan->setBobAnimation(11);
			_vm->_objectsMan->setBobAnimation(10);
			break;

		case 610:
			_vm->_objectsMan->stopBobAnimation(5);
			_vm->_objectsMan->stopBobAnimation(7);
			_vm->_objectsMan->stopBobAnimation(8);
			_vm->_objectsMan->stopBobAnimation(9);
			_vm->_objectsMan->stopBobAnimation(12);
			_vm->_objectsMan->stopBobAnimation(13);
			break;

		case 611:
			_vm->_objectsMan->setBobAnimation(5);
			_vm->_objectsMan->setBobAnimation(7);
			_vm->_objectsMan->setBobAnimation(8);
			_vm->_objectsMan->setBobAnimation(9);
			_vm->_objectsMan->setBobAnimation(12);
			_vm->_objectsMan->setBobAnimation(13);
			break;

		default:
			break;
		}
		opcodeType = 1;
		break;
	case MKTAG24('E', 'I', 'F'):
		opcodeType = 4;
		break;
	case MKTAG24('V', 'A', 'L'): {
		opcodeType = 1;
		int idx = READ_LE_INT16(dataP + 5);
		assert(idx >= 0 && idx < 2050);
		_vm->_globals->_saveData->_data[idx] = dataP[7];
		break;
		}
	case MKTAG24('A', 'D', 'D'):
		opcodeType = 1;
		_vm->_globals->_saveData->_data[READ_LE_INT16(dataP + 5)] += dataP[7];
		break;
	case MKTAG24('B', 'O', 'S'):
		opcodeType = 1;
		_vm->_objectsMan->setBobOffset(READ_LE_INT16(dataP + 5), READ_LE_INT16(dataP + 7));
		break;
	case MKTAG24('V', 'O', 'N'):
		_vm->_objectsMan->enableVerb(READ_LE_INT16(dataP + 5), READ_LE_INT16(dataP + 7));
		opcodeType = 1;
		break;
	case MKTAG24('Z', 'C', 'H'):
		_vm->_linesMan->_zone[READ_LE_INT16(dataP + 5)]._messageId = READ_LE_INT16(dataP + 7);
		opcodeType = 1;
		break;
	case MKTAG24('J', 'U', 'M'):
		_vm->_objectsMan->_jumpZone = READ_LE_INT16(dataP + 5);
		_vm->_objectsMan->_jumpVerb = READ_LE_INT16(dataP + 7);
		opcodeType = 6;
		break;
	case MKTAG24('S', 'O', 'U'): {
		int soundNum = READ_LE_INT16(dataP + 5);

		Common::String file = Common::String::format("SOUND%d.WAV", soundNum);
		_vm->_soundMan->playSoundFile(file);
		opcodeType = 1;
		break;
		}
	case MKTAG24('V', 'O', 'F'):
		_vm->_objectsMan->disableVerb(READ_LE_INT16(dataP + 5), READ_LE_INT16(dataP + 7));
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
	return READ_LE_INT16(dataP + 5);
}

int ScriptManager::handleIf(const byte *dataP, int offset) {
	int newOffset;
	int curOffset = offset;
	bool loopFl;
	do {
		loopFl = false;
		int tmpOffset = curOffset;
		int opcodeType;
		do {
			if (_vm->shouldQuit())
				return 0; // Exiting game

			++tmpOffset;
			if (tmpOffset > 400)
				error("Control if failed");
			opcodeType = checkOpcode(dataP + 20 * tmpOffset);
		} while (opcodeType != 4); // EIF
		newOffset = tmpOffset;
		tmpOffset = curOffset;
		do {
			if (_vm->shouldQuit())
				return 0; // Exiting game

			++tmpOffset;
			if (tmpOffset > 400)
				error("Control if failed ");
			if (checkOpcode(dataP + 20 * tmpOffset) == 3) { // IIF
				curOffset = newOffset;
				loopFl = true;
				break;
			}
		} while (newOffset != tmpOffset);
	} while (loopFl);

	const byte *buf = dataP + 20 * offset;
	byte oper = buf[13];
	byte oper2 = buf[14];
	byte operType = buf[15];
	int saveDataIdx1 = READ_LE_INT16(buf + 5);
	int compVal1 = READ_LE_INT16(buf + 7);
	bool check1Fl = false;
	if ((oper == 1 && _vm->_globals->_saveData->_data[saveDataIdx1] == compVal1) ||
	    (oper == 2 && _vm->_globals->_saveData->_data[saveDataIdx1] != compVal1) ||
	    (oper == 3 && _vm->_globals->_saveData->_data[saveDataIdx1] <= compVal1) ||
	    (oper == 4 && _vm->_globals->_saveData->_data[saveDataIdx1] >= compVal1) ||
	    (oper == 5 && _vm->_globals->_saveData->_data[saveDataIdx1] > compVal1) ||
	    (oper == 6 && _vm->_globals->_saveData->_data[saveDataIdx1] < compVal1))
		check1Fl = true;

	bool check2Fl = false;
	if (operType != 3) {
		int saveDataIdx2 = READ_LE_INT16(buf + 9);
		int compVal2 = READ_LE_INT16(buf + 11);
		if ((oper2 == 1 && compVal2 == _vm->_globals->_saveData->_data[saveDataIdx2]) ||
		    (oper2 == 2 && compVal2 != _vm->_globals->_saveData->_data[saveDataIdx2]) ||
		    (oper2 == 3 && compVal2 >= _vm->_globals->_saveData->_data[saveDataIdx2]) ||
		    (oper2 == 4 && compVal2 <= _vm->_globals->_saveData->_data[saveDataIdx2]) ||
		    (oper2 == 5 && compVal2 < _vm->_globals->_saveData->_data[saveDataIdx2]) ||
		    (oper2 == 6 && compVal2 > _vm->_globals->_saveData->_data[saveDataIdx2]))
			check2Fl = true;
	}

	if ((operType == 3) && check1Fl) {
		return (offset + 1);
	} else if ((operType == 1) && check1Fl && check2Fl) {
		return (offset + 1);
	} else if ((operType == 2) && (check1Fl || check2Fl)) {
		return (offset + 1);
	}

	return (newOffset + 1);
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
	default:
		warning("Unhandled opcode %c%c%c", dataP[2], dataP[3], dataP[4]);
		break;
	}
	return result;
}

} // End of namespace Hopkins
