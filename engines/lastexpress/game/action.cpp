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

#include "lastexpress/lastexpress.h"

namespace LastExpress {

int LogicManager::findCursor(Link *link) {
	int result;

	if (link->cursor != kCursorProcess)
		return link->cursor;

	switch (link->action) {
	case 1:
		if (!_lastSavedNodeIndex && (_gameEvents[kEventKronosBringFirebird] || _gameProgress[kEventAugustBringEgg])) {
			result = kCursorNormal;
		} else {
			result = kCursorBackward;
		}

		break;
	case 5:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else {
			result = _gameObjects[link->param1].cursor;
		}

		break;
	case 6:
	case 31:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else if (_inventorySelectedItemIdx != kItemKey || (_gameObjects[link->param1].character) || _gameObjects[link->param1].door != 1 || !_gameObjects[link->param1].cursor2 || inComp(kCharacterCath) || preventEnterComp(link->param1)) {
			result = _gameObjects[link->param1].cursor2;
		} else {
			result = _gameInventory[kItemKey].cursor;
		}

		break;
	case 12:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else {
			if (_gameObjects[link->param1].character)
				result = _gameObjects[link->param1].cursor;
			else
				result = kCursorNormal;
		}

		break;
	case 13:
		if (link->param1 >= 32) {
			result = kCursorNormal;
		} else if ((!_inventorySelectedItemIdx || _gameInventory[_inventorySelectedItemIdx].manualSelect) && (link->param1 != 21 || _gameProgress[kProgressEventCorpseMovedFromFloor] == 1)) {
			result = kCursorHand;
		} else {
			result = kCursorNormal;
		}

		break;
	case 14:
		if (link->param1 >= 32) {
			result = kCursorNormal;
		} else if (link->param1 != _inventorySelectedItemIdx || link->param1 == 20 && !_gameProgress[kProgressIsTrainRunning] && link->param2 == 4 || link->param1 == 18 && link->param2 == 1 && _gameProgress[kProgressField5C]) {
			result = kCursorNormal;
		} else {
			result = _gameInventory[_inventorySelectedItemIdx].cursor;
		}

		break;
	case 15:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else if (_gameProgress[link->param1] == link->param2) {
			result = link->param3;
		} else {
			result = kCursorNormal;
		}

		break;
	case 16:
		if (_inventorySelectedItemIdx == kItemKey && !_gameObjects[kItemMatchBox].door || _gameObjects[kItemMatchBox].door == 1 && cathHasItem(kItemKey) && (_inventorySelectedItemIdx == kItemBriefcase || _inventorySelectedItemIdx == kItemFirebird)) {
			result = _gameInventory[kItemKey].cursor;
		} else {
			if (link->param1 >= 128) {
				result = 0;
			} else if (_inventorySelectedItemIdx != kItemKey ||
				_gameObjects[link->param1].character ||
				_gameObjects[link->param1].door != 1 ||
				!_gameObjects[link->param1].cursor2 ||
				inComp(kCharacterCath) || preventEnterComp(link->param1)) {
				result = _gameObjects[link->param1].cursor2;
			} else {
				result = _gameInventory[kItemKey].cursor;
			}
		}

		break;
	case 18:
		if (_gameProgress[kProgressJacket] == 2) {
			if ((_gameEvents[kEventCathLookOutsideWindowDay] || _gameEvents[kEventCathLookOutsideWindowNight] || getModel(1) == 1) &&
				_gameProgress[kProgressIsTrainRunning] &&
				(link->param1 != 45 || !inComp(kCharacterRebecca, kCarRedSleeping, 4840) && _gameObjects[kObjectOutsideBetweenCompartments].door == 2) &&
				_inventorySelectedItemIdx != kItemBriefcase && _inventorySelectedItemIdx != kItemFirebird) {
				result = kCursorForward;
			} else {
				result = getModel(1) == 1 ? kCursorNormal : kCursorMagnifier;
			}
		} else {
			result = kCursorNormal;
		}

		break;
	case 19:
		result = _gameProgress[kProgressFieldC8] == 0 ? kCursorNormal : kCursorLeft;
		break;
	case 21:
		if (_gameProgress[kProgressIsTrainRunning] && _inventorySelectedItemIdx != kItemBriefcase && _inventorySelectedItemIdx != kItemFirebird &&
			(_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3 || _gameProgress[kProgressChapter] == 5)) {
			result = kCursorUp;
		} else {
			result = kCursorNormal;
		}

		break;
	case 23:
		if (link->param1 == 1) {
			result = checkDoor(73) == 0 ? kCursorHand : kCursorNormal;
		} else {
			result = kCursorNormal;
		}

		break;
	case 24:
		if (link->param1 == 2) {
			if (!_gameEvents[kEventCathStruggleWithBonds2] || _gameEvents[kEventCathBurnRope])
				result = kCursorNormal;
			else
				result = kCursorHand;
		} else {
			result = kCursorNormal;
		}

		break;
	case 30:
		if (_engine->_beetle) {
			if (_engine->_beetle->onTable()) {
				if (_inventorySelectedItemIdx == kItemMatchBox && cathHasItem(kItemMatch))
					result = _gameInventory[kItemMatchBox].cursor;
				else
					result = kCursorHandPointer;
			} else {
				result = kCursorNormal;
			}
		} else {
			result = kCursorNormal;
		}

		break;
	case 33:
		if (link->param1 == 3) {
			if (_inventorySelectedItemIdx == kItemWhistle)
				result = _gameInventory[kItemWhistle].cursor;
			else
				result = kCursorNormal;
		} else {
			result = kCursorNormal;
		}

		break;
	case 35:
		result = _gameProgress[kProgressChapter] == 1 ? kCursorHand : kCursorNormal;
		break;
	case 37:
		result = getHintDialog(link->param1) == 0 ? kCursorNormal : kCursorHandPointer;
		break;
	case 40:
		if (_gameProgress[kProgressField18] == 2 && !_gameProgress[kProgressFieldE4] && (_gameTime > 1404000 || _gameProgress[kProgressEventMetAugust] && _gameProgress[kProgressFieldCC] && (!_gameProgress[kProgressField24] || _gameProgress[kProgressField3C]))) {
			result = kCursorSleep;
		} else {
			result = kCursorNormal;
		}

		break;
	default:
		result = kCursorNormal;
		break;
	}

	return result;
}

bool LogicManager::nodeHasItem(int item) {
	switch (_trainData[_trainNodeIndex].car) {
	case kCarKronos:
		if (_trainData[_trainNodeIndex].parameter1 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarGreenSleeping:
		if (_trainData[_trainNodeIndex].parameter1 != item && _trainData[_trainNodeIndex].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarRedSleeping:
		if (_trainData[_trainNodeIndex].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarRestaurant:
		if (_trainData[_trainNodeIndex].parameter1 != item && _trainData[_trainNodeIndex].parameter2 != item && _trainData[_trainNodeIndex].parameter3 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarLocomotive:
		if (_trainData[_trainNodeIndex].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	default:
		break;
	}

	return false;
}

void LogicManager::doPreFunction(int *sceneOut) {
	Link *link;
	Link *next;
	Link tmp;	
	uint16 scene;

	if (!*sceneOut || *sceneOut > 2500)
		*sceneOut = 1;

	switch (_trainData[*sceneOut].car) {
	case kCarBaggageRear:
		if (_trainData[*sceneOut].parameter1 < 128) {
			if (_gameObjects[_trainData[*sceneOut].parameter1].door) {
				link = _trainData[*sceneOut].link;
				for (bool found = false; link && !found; link = link->next) {
					if (_gameObjects[_trainData[*sceneOut].parameter1].door == link->location) {
						tmp.copyFrom(link);
						doAction(&tmp);

						if (tmp.scene) {
							*sceneOut = (int)link->scene;
							doPreFunction(sceneOut);
						}

						found = true;
					}
				}
			}
		}

		break;
	case kCarKronos:
		if (_trainData[*sceneOut].parameter1 < 32) {
			if (_gameInventory[_trainData[*sceneOut].parameter1].location) {
				link = _trainData[*sceneOut].link;
				for (bool found = false; link && !found; link = link->next) {
					if (_gameInventory[_trainData[*sceneOut].parameter1].location == link->location) {
						tmp.copyFrom(link);
						doAction(&tmp);

						if (tmp.scene) {
							*sceneOut = (int)link->scene;
							doPreFunction(sceneOut);
						}

						found = true;
					}
				}
			}
		}

		break;
	case kCarGreenSleeping:
		if (_trainData[*sceneOut].parameter1 < 32) {
			if (_trainData[*sceneOut].parameter2 < 32) {
				int locFlag = (_gameInventory[_trainData[*sceneOut].parameter1].location != 0) ? 1 : 0;

				if (_gameInventory[_trainData[*sceneOut].parameter2].location)
					locFlag |= 2;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_gameInventory[_trainData[*sceneOut].parameter1].location == link->param1 && _gameInventory[_trainData[*sceneOut].parameter2].location == link->param2) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	case kCarRedSleeping:
		if (_trainData[*sceneOut].parameter1 < 128) {
			if (_trainData[*sceneOut].parameter2 < 32) {
				int locFlag = (_gameObjects[_trainData[*sceneOut].parameter1].door == 2) ? 1 : 0;

				if (_gameInventory[_trainData[*sceneOut].parameter2].location)
					locFlag |= 2;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_gameObjects[_trainData[*sceneOut].parameter1].door == link->param1 && _gameInventory[_trainData[*sceneOut].parameter2].location == link->param2) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	case kCarRestaurant:
		if (_trainData[*sceneOut].parameter1 < 32 && _trainData[*sceneOut].parameter2 < 32) {
			if (_trainData[*sceneOut].parameter3 < 32) {
				int locFlag = (_gameInventory[_trainData[*sceneOut].parameter1].location != 0) ? 1 : 0;

				if (_gameInventory[_trainData[*sceneOut].parameter2].location)
					locFlag |= 2;

				if (_gameInventory[_trainData[*sceneOut].parameter3].location)
					locFlag |= 4;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_gameInventory[_trainData[*sceneOut].parameter1].location == link->param1 && _gameInventory[_trainData[*sceneOut].parameter2].location == link->param2 && _gameInventory[_trainData[*sceneOut].parameter3].location == link->param3) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	case kCarBaggage:
		if (_trainData[*sceneOut].parameter1 < 128) {
			link = _trainData[*sceneOut].link;
			bool found = false;
			if (link) {
				while (!found) {
					if (_gameObjects[_trainData[*sceneOut].parameter1].model == link->location) {
						tmp.copyFrom(link);
						doAction(&tmp);

						if (tmp.scene) {
							*sceneOut = (int)link->scene;
							doPreFunction(sceneOut);
						}

						found = true;
					}

					link = link->next;
					if (!link) {
						if (!found) {
							tmp.copyFrom(_trainData[*sceneOut].link);
							doAction(&tmp);

							if (tmp.scene) {
								*sceneOut = (int)tmp.scene;
								doPreFunction(sceneOut);
								break;
							}
						}
					}
				}
			} else {
				// This was a nullptr access in the original I think?
				// I'm shielding it for now, it might only happen during
				// non-ordinary situations (e.g. no conductors)
				if (!found && _trainData[*sceneOut].link) {
					tmp.copyFrom(_trainData[*sceneOut].link);
					doAction(&tmp);

					if (tmp.scene) {
						*sceneOut = (int)tmp.scene;
						doPreFunction(sceneOut);
					}
				}
			}
		}

		break;
	case kCarCoalTender:
		if (_trainData[*sceneOut].parameter1 < 16 &&
			(_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1] || _blockedEntitiesBits[_trainData[*sceneOut].parameter1])) {
			if ((!_engine->getOtisManager()->fDirection(_trainNodeIndex) || !_engine->getOtisManager()->fDirection(*sceneOut) || _trainData[_trainNodeIndex].nodePosition.position >= _trainData[*sceneOut].nodePosition.position) &&
				(!_engine->getOtisManager()->rDirection(_trainNodeIndex) || !_engine->getOtisManager()->rDirection(*sceneOut) || _trainData[_trainNodeIndex].nodePosition.position <= _trainData[*sceneOut].nodePosition.position)) {
				next = _trainData[*sceneOut].link->next;
				scene = next->scene;
				*sceneOut = (int)scene;
				doPreFunction(sceneOut);

				break;
			}

			if (whoseBit(_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30 &&
				whoseBit(_blockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30) {
				playDialog(kCharacterCath, "CAT1126A", -1, 0);
			}

			scene = _trainData[*sceneOut].link->scene;
			*sceneOut = (int)scene;
			doPreFunction(sceneOut);
		}

		break;
	case kCarLocomotive:
		if (_trainData[*sceneOut].parameter1 < (_engine->isDemo() ? 16 : 32)) {
			if (_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1] || _blockedEntitiesBits[_trainData[*sceneOut].parameter1]) {
				if (_engine->getOtisManager()->fDirection(_trainNodeIndex) &&
					_engine->getOtisManager()->fDirection(*sceneOut) &&
					_trainData[_trainNodeIndex].nodePosition.position < _trainData[*sceneOut].nodePosition.position ||
					_engine->getOtisManager()->rDirection(_trainNodeIndex) &&
					_engine->getOtisManager()->rDirection(*sceneOut) &&
					_trainData[_trainNodeIndex].nodePosition.position > _trainData[*sceneOut].nodePosition.position) {

					if (whoseBit(_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30 && whoseBit(_blockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30) {
						playDialog(kCharacterCath, "CAT1126A", -1, 0);
					}

					scene = _trainData[*sceneOut].link->scene;
				} else {
					next = _trainData[*sceneOut].link->next;
					scene = next->scene;
				}

				*sceneOut = (int)scene;
				doPreFunction(sceneOut);
			} else {
				if (_trainData[*sceneOut].parameter2 < 32) {
					if (_gameInventory[_trainData[*sceneOut].parameter2].location) {
						link = _trainData[*sceneOut].link;
						for (bool found = false; link && !found; link = link->next) {
							if (_gameInventory[_trainData[*sceneOut].parameter2].location == link->location) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	default:
		break;
	}

	if (whoRunningDialog(kCharacterTableE)) {
		if (_trainData[*sceneOut].car != 132 || _trainData[*sceneOut].parameter1)
			fadeDialog(kCharacterTableE);
	}

	if (!_engine->isDemo() && _engine->_beetle) {
		if (_trainData[*sceneOut].car != 130)
			_engine->endBeetle();
	}
}

void LogicManager::doPostFunction() {
	Link tmp;

	switch (_trainData[_trainNodeIndex].car) {
	case 128:
	{
		int32 delta = _trainData[_trainNodeIndex].parameter1 + 10;
		_gameTime += delta * _gameTimeTicksDelta;
		_currentGameSessionTicks += delta;
		int32 delayedTicks = _engine->getSoundFrameCounter() + 4 * _trainData[_trainNodeIndex].parameter1;

		if (!_engine->mouseHasRightClicked() && delayedTicks > _engine->getSoundFrameCounter()) {
			do {
				if (_engine->mouseHasRightClicked())
					break;
				_engine->getSoundManager()->soundThread();
				_engine->getSubtitleManager()->subThread();
			} while (delayedTicks > _engine->getSoundFrameCounter());
		}

		tmp.copyFrom(_trainData[_trainNodeIndex].link);
		doAction(&tmp);

		if (_engine->mouseHasRightClicked() && _trainData[tmp.scene].car == 128) {
			do {
				tmp.copyFrom(_trainData[tmp.scene].link);
				doAction(&tmp);
			} while (_trainData[tmp.scene].car == 128);
		}

		if (getCharacter(kCharacterCath).characterPosition.car == 9 &&
			(getCharacter(kCharacterCath).characterPosition.position == 4 ||
			 getCharacter(kCharacterCath).characterPosition.position == 3)) {

			int characterIdx = 0;
			int charactersRndArray[39];

			memset(charactersRndArray, 0, sizeof(charactersRndArray));

			for (int j = 1; j < 40; j++) {
				if (getCharacter(kCharacterCath).characterPosition.position == 4) {
					if (getCharacter(j).characterPosition.car == 4 && getCharacter(j).characterPosition.position > 9270 || getCharacter(j).characterPosition.car == 5 && getCharacter(j).characterPosition.position < 1540) {
						charactersRndArray[characterIdx] = j;
						characterIdx++;
					}
				} else if (getCharacter(j).characterPosition.car == 3 && getCharacter(j).characterPosition.position > 9270 || getCharacter(j).characterPosition.car == 4 && getCharacter(j).characterPosition.position < 850) {
					charactersRndArray[characterIdx] = j;
					characterIdx++;
				}
			}

			if (characterIdx) {
				if (characterIdx <= 1) {
					playChrExcuseMe(charactersRndArray[0], kCharacterCath, 16);
				} else {
					playChrExcuseMe(charactersRndArray[rnd(characterIdx)], kCharacterCath, 16);
				}
			}
		}

		if (tmp.scene)
			_engine->getGraphicsManager()->stepBG(tmp.scene);

		return;
	}
	case 129:
		if (_engine->isDemo())
			break;

		if (_gameProgress[kProgressField18] == 2)
			send(kCharacterCath, kCharacterMaster, 190346110, 0);

		return;
	case 130:
		if (_engine->isDemo())
			break;

		_engine->doBeetle();
		return;
	case 131:
	{
		if (_gameTime < 2418300 && _gameProgress[kProgressField18] != 4) {
			Slot *slot = _engine->getSoundManager()->_soundCache;
			if (slot) {
				do {
					if (slot->hasTag(kSoundTagLink))
						break;

					slot = slot->getNext();
				} while (slot);

				if (slot)
					slot->setFade(0);
			}

			playDialog(kCharacterClerk, "LIB050", 16, 0);

			if (_gameProgress[kProgressChapter] == 1) {
				endGame(0, 0, 62, true);
			} else if (_gameProgress[kProgressChapter] == 4) {
				endGame(0, 0, 64, true);
			} else {
				endGame(0, 0, 63, true);
			}
		}

		return;
	}
	case 132:
		if (!whoRunningDialog(kCharacterTableE)) {
			switch (_trainData[_trainNodeIndex].parameter1) {
			case 1:
				if (dialogRunning("TXT1001"))
					endDialog("TXT1001");

				playDialog(kCharacterTableE, "TXT1001", 16, 0);
				break;
			case 2:
				if (dialogRunning("TXT1001A"))
					endDialog("TXT1001A");

				playDialog(kCharacterTableE, "TXT1001A", 16, 0);
				break;
			case 3:
				if (dialogRunning("TXT1011"))
					endDialog("TXT1011");

				playDialog(kCharacterTableE, "TXT1011", 16, 0);
				break;
			case 4:
				if (dialogRunning("TXT1012"))
					endDialog("TXT1012");

				playDialog(kCharacterTableE, "TXT1012", 16, 0);
				break;
			case 5:
				if (dialogRunning("TXT1013"))
					endDialog("TXT1013");

				playDialog(kCharacterTableE, "TXT1013", 16, 0);
				break;
			case 6:
				if (dialogRunning("TXT1014"))
					endDialog("TXT1014");

				playDialog(kCharacterTableE, "TXT1014", 16, 0);
				break;
			case 7:
				if (dialogRunning("TXT1020"))
					endDialog("TXT1020");

				playDialog(kCharacterTableE, "TXT1020", 16, 0);
				break;
			case 8:
				if (dialogRunning("TXT1030"))
					endDialog("TXT1030");

				playDialog(kCharacterTableE, "TXT1030", 16, 0);
				break;
			case 50:
				playDialog(kCharacterTableE, "END1009B", 16, 0);
				break;
			case 51:
				playDialog(kCharacterTableE, "END1046", 16, 0);
				break;
			case 52:
				playDialog(kCharacterTableE, "END1047", 16, 0);
				break;
			case 53:
				playDialog(kCharacterTableE, "END1112", 16, 0);
				break;
			case 54:
				playDialog(kCharacterTableE, "END1112A", 16, 0);
				break;
			case 55:
				playDialog(kCharacterTableE, "END1503", 16, 0);
				break;
			case 56:
				playDialog(kCharacterTableE, "END1505A", 16, 0);
				break;
			case 57:
				playDialog(kCharacterTableE, "END1505B", 16, 0);
				break;
			case 58:
				playDialog(kCharacterTableE, "END1610", 16, 0);
				break;
			case 59:
				playDialog(kCharacterTableE, "END1612A", 16, 0);
				break;
			case 60:
				playDialog(kCharacterTableE, "END1612C", 16, 0);
				break;
			case 61:
				playDialog(kCharacterTableE, "END1612D", 16, 0);
				break;
			case 62:
				playDialog(kCharacterTableE, "ENDALRM1", 16, 0);
				break;
			case 63:
				playDialog(kCharacterTableE, "ENDALRM2", 16, 0);
				break;
			case 64:
				playDialog(kCharacterTableE, "ENDALRM3", 16, 0);
				break;
			default:
				break;
			}
		}

		return;
	case 133:
		if (_doubleClickFlag) {
			_doubleClickFlag = false;
			_engine->getGraphicsManager()->setMouseDrawable(true);
			mouseStatus();
		}

		break;
	default:
		break;
	}
}

void LogicManager::doAction(Link *link) {
	char filename[8];

	int musId = 0;
	int nisId = 0;
	
	switch (link->action) {
	case kActionInventory:
	{
		if (_useLastSavedNodeIndex) {
			int bumpScene = 0;
			if (_lastSavedNodeIndex) {
				bumpScene = _lastSavedNodeIndex;
				_lastSavedNodeIndex = 0;
				bumpCathNode(bumpScene);
			} else {
				_useLastSavedNodeIndex = 0;

				if (_positions[100 * _trainData[_lastNodeIndex].nodePosition.car + _trainData[_lastNodeIndex].cathDir]) {
					bumpCathNode(getSmartBumpNode(_lastNodeIndex));
				} else {
					bumpCathNode(_lastNodeIndex);
				}
			}

			if (_inventorySelectedItemIdx && (!_gameInventory[_inventorySelectedItemIdx].isSelectable || (bumpScene == 0 && findLargeItem()))) {
				_inventorySelectedItemIdx = findLargeItem();

				if (_inventorySelectedItemIdx) {
					_engine->getGraphicsManager()->drawItem(_gameInventory[_inventorySelectedItemIdx].cursor, 44, 0);
				} else if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}

				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
				return;
			}
		}

		break;
	}
	case kActionSendCathMessage:
		send(kCharacterCath, link->param1, link->param2, 0);
		break;
	case kActionPlaySound:
		Common::sprintf_s(filename, "LIB%03d", link->param1);
		if (link->param2 || !cathRunningDialog(filename))
			queueSFX(kCharacterCath, link->param1, link->param2);

		break;
	case kActionPlayMusic:
		Common::sprintf_s(filename, "MUS%03d", link->param1);
		if (!dialogRunning(filename) && (link->param1 != 50 || _gameProgress[kProgressChapter] == 5))
			playDialog(kCharacterCath, filename, 16, link->param2);

		break;
	case kActionKnock:
		if (link->param1 < 128) {
			if (_gameObjects[link->param1].character) {
				send(kCharacterCath, _gameObjects[link->param1].character, 8, link->param1);
				return;
			}

			if (!cathRunningDialog("LIB012"))
				queueSFX(kCharacterCath, 12, 0);
		}

		break;
	case kActionPlaySounds:
		queueSFX(kCharacterCath, link->param1, 0);
		queueSFX(kCharacterCath, link->param3, link->param2);

		break;
	case kActionPlayAnimation:
		if (!_gameEvents[link->param1]) {
			playNIS(link->param1);

			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionOpenCloseObject:
		if (link->param1 >= 128)
			return;

		setDoor(link->param1, _gameObjects[link->param1].character, link->param2, 255, 255);
		if ((link->param1 < 9 || link->param1 > 16) && (link->param1 < 40 || link->param1 > 47)) {
			if (link->param2) {
				if (link->param2 == 1) {
					queueSFX(kCharacterCath, 24, 0);
					return;
				}

				if (link->param2 != 2)
					return;
			}

			queueSFX(kCharacterCath, 36, 0);
			return;
		}

		if (link->param2) {
			if (link->param2 == 2)
				queueSFX(kCharacterCath, 20, 0);
		} else {
			queueSFX(kCharacterCath, 21, 0);
		}

		break;
	case kActionSetModel:
		if (link->param1 < 128) {
			setModel(link->param1, link->param2);
			if (link->param1 != 112 || dialogRunning("LIB096")) {
				if (link->param1 == 1)
					queueSFX(kCharacterCath, 73, 0);
			} else {
				queueSFX(kCharacterCath, 96, 0);
			}
		}

		break;
	case kActionSetItem:
		if (_engine->isDemo())
			break;

		if (link->param1 < 32) {
			if (!_gameInventory[link->param1].isPresent) {
				_gameInventory[link->param1].location = link->param2;

				if (link->param1 == kItemCorpse) {
					_gameProgress[kProgressEventCorpseMovedFromFloor] = (_gameInventory[kItemCorpse].location == 3 || _gameInventory[kItemCorpse].location == 4) ? 1 : 0;
				}
			}
		}

		break;
	case kActionPickItem:
		if (_engine->isDemo())
			break;

		if (link->param1 >= 32)
			return;

		if (!_gameInventory[link->param1].location)
			return;

		if (link->param1 == kItemCorpse) {
			takeTyler(link->scene == 0, link->param2);

			if (link->param2 != 4) {
				_gameInventory[kItemCorpse].isPresent = 1;
				_gameInventory[kItemCorpse].location = 0;
				_inventorySelectedItemIdx = kItemCorpse;
				_engine->getGraphicsManager()->drawItem(_gameInventory[kItemCorpse].cursor, 44, 0);
				_engine->getGraphicsManager()->burstBox(44u, 0, 32, 32);
			}
		} else {
			_gameInventory[link->param1].isPresent = 1;
			_gameInventory[link->param1].location = 0;

			if (link->param1 == kItemGreenJacket) {
				takeJacket(link->scene == 0);
			} else {
				if (link->param1 == kItemScarf) {
					takeScarf(link->scene == 0);
					return;
				}

				if (link->param1 == kItemParchemin && link->param2 == 2) {
					_gameInventory[kItemParchemin].isPresent = 1;
					_gameInventory[kItemParchemin].location = 0;
					_gameInventory[kItem11].location = 1;
					queueSFX(kCharacterCath, 9, 0);
				} else if (link->param1 == kItemBomb) {
					forceJump(kCharacterAbbot, &LogicManager::CONS_Abbot_CatchCath);
				} else if (link->param1 == kItemBriefcase) {
					queueSFX(kCharacterCath, 83, 0);
				}
			}

			if (_gameInventory[link->param1].scene) {
				if (!_useLastSavedNodeIndex) {
					if (!link->scene)
						link->scene = _trainNodeIndex;

					_useLastSavedNodeIndex = 1;
					_lastNodeIndex = link->scene;
				}

				bumpCathNode(_gameInventory[link->param1].scene);
				link->scene = kSceneNone;
			}

			if (_gameInventory[link->param1].isSelectable) {
				_inventorySelectedItemIdx = link->param1;
				_engine->getGraphicsManager()->drawItem(_gameInventory[link->param1].cursor, 44, 0);
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}
		}

		break;
	case kActionDropItem:
		if (_engine->isDemo())
			break;

		if (link->param1 >= 32 || !_gameInventory[link->param1].isPresent || !link->param2)
			return;

		if (link->param1 == kItemBriefcase) {
			queueSFX(kCharacterCath, 82, 0);

			if (link->param2 == 2) {
				if (!_gameProgress[kProgressField58]) {
					_engine->getVCR()->writeSavePoint(1, 0, 0);
					_gameProgress[kProgressField58] = 1;
				}

				if (_gameInventory[kItemParchemin].location == 2) {
					_gameInventory[kItemParchemin].isPresent = 1;
					_gameInventory[kItemParchemin].location = 0;
					_gameInventory[kItem11].location = 1;
					queueSFX(kCharacterCath, 9, 0);
				}
			}
		}

		_gameInventory[link->param1].isPresent = 0;
		_gameInventory[link->param1].location = link->param2;
		if (link->param1 == 20)
			dropTyler(link->scene == 0);

		_inventorySelectedItemIdx = 0;

		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
			_engine->getGraphicsManager()->unlockSurface();
		}

		_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
		break;
	
	case kActionLeanOutWindow:
		if (_engine->isDemo())
			break;

		if (!_gameEvents[kEventCathLookOutsideWindowDay] && !_gameEvents[kEventCathLookOutsideWindowNight] && getModel(1) != 1 || !_gameProgress[kProgressIsTrainRunning] || link->param1 == 45 && (inComp(kCharacterRebecca, kCarRedSleeping, 4840) || _gameObjects[kObjectOutsideBetweenCompartments].door != 2) || _inventorySelectedItemIdx == kItemBriefcase || _inventorySelectedItemIdx == kItemFirebird) {
			if (link->param1 == 9 || link->param1 >= 44 && link->param1 <= 45) {
				if (isNight()) {
					playNIS(kEventCathLookOutsideWindowNight);
				} else {
					playNIS(kEventCathLookOutsideWindowDay);
				}
				cleanNIS();
				link->scene = kSceneNone;
			}

			return;
		}

		switch (link->param1) {
		case 9:
			_gameEvents[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGoOutsideTylerCompartmentNight);
			} else {
				playNIS(kEventCathGoOutsideTylerCompartmentDay);
			}

			_gameProgress[kProgressFieldC8] = 1;
			break;
		case 44:
			_gameEvents[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGoOutsideNight);
			} else {
				playNIS(kEventCathGoOutsideDay);
			}

			_gameProgress[kProgressFieldC8] = 1;
			break;
		case 45:
			_gameEvents[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGetInsideNight);
			} else {
				playNIS(kEventCathGetInsideDay);
			}

			if (!link->scene)
				cleanNIS();

			break;
		}

		break;
	case kActionAlmostFall:
		if (_engine->isDemo())
			break;

		if (link->param1 == 9) {
			if (isNight()) {
				playNIS(kEventCathSlipTylerCompartmentNight);
			} else {
				playNIS(kEventCathSlipTylerCompartmentDay);
			}

			_gameProgress[kProgressFieldC8] = 0;

			if (link->scene)
				return;
		} else {
			if (link->param1 != 44)
				return;

			if (isNight()) {
				playNIS(kEventCathSlipNight);
			} else {
				playNIS(kEventCathSlipDay);
			}

			_gameProgress[kProgressFieldC8] = 0;

			if (link->scene)
				return;
		}

		cleanNIS();
		break;
	case kActionClimbInWindow:
		if (_engine->isDemo())
			break;

		switch (link->param1) {
		case 9:
			if (isNight()) {
				playNIS(kEventCathGetInsideTylerCompartmentNight);
			} else {
				playNIS(kEventCathGetInsideTylerCompartmentDay);
			}

			if (link->scene)
				return;

			break;
		case 44:
			if (isNight()) {
				playNIS(kEventCathGetInsideNight);
			} else {
				playNIS(kEventCathGetInsideDay);
			}

			if (link->scene)
				return;

			break;
		case 45:
			playNIS(kEventCathGettingInsideAnnaCompartment);

			if (link->scene)
				return;

			break;
		default:
			return;
		}

		cleanNIS();
		break;
	case kActionClimbLadder:
		if (_engine->isDemo())
			break;

		if (link->param1 == 1) {
			if (_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3) {
				playNIS(kEventCathTopTrainGreenJacket);
			} else if (_gameProgress[kProgressChapter] == 5) {
				playNIS(kEventCathTopTrainNoJacketDay - (_gameProgress[kProgressIsDayTime] == kProgressField0));
			}

			if (link->scene)
				return;
		} else {
			if (link->param1 != 2)
				return;

			if (_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3) {
				playNIS(kEventCathClimbUpTrainGreenJacket);
				playNIS(kEventCathTopTrainGreenJacket);
			} else if (_gameProgress[kProgressChapter] == 5) {
				playNIS(kEventCathClimbUpTrainNoJacketDay - (_gameProgress[kProgressIsDayTime] == 0));
				playNIS(kEventCathTopTrainNoJacketDay - (_gameProgress[kProgressIsDayTime] == 0));
			}

			if (link->scene)
				return;
		}

		cleanNIS();
		break;
	case kActionClimbDownTrain:
		if (_engine->isDemo())
			break;

		if (_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3) {
			nisId = kEventCathClimbDownTrainGreenJacket;
		} else if (_gameProgress[kProgressChapter] == 5) {
			if (_gameProgress[kProgressIsDayTime] == 0) {
				nisId = kEventCathClimbDownTrainNoJacketNight;
			} else {
				nisId = kEventCathClimbDownTrainNoJacketDay;
			}
		}

		if (nisId) {
			playNIS(nisId);

			if (nisId == kEventCathClimbDownTrainNoJacketDay)
				queueSFX(kCharacterCath, 37, 0);

			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionKronosSanctum:
		if (_engine->isDemo())
			break;

		switch (link->param1) {
		case 1:
			send(kCharacterCath, 32, 225056224, 0);
			break;
		case 2:
			send(kCharacterCath, 32, 338494260, 0);
			break;
		case 3:
			if (_inventorySelectedItemIdx == kItemBriefcase) {
				_gameInventory[kItemBriefcase].location = 3;
				_gameInventory[kItemBriefcase].isPresent = 0;
				queueSFX(kCharacterCath, 82, 0);
				_inventorySelectedItemIdx = 0;
				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}

			if (_gameInventory[kItemBriefcase].location == 3) {
				nisId = kEventCathJumpUpCeiling;
			} else {
				nisId = kEventCathJumpUpCeilingBriefcase;
			}

			break;
		case 4:
			if (_gameProgress[kProgressChapter] == 1)
				send(kCharacterCath, kCharacterKronos, 202621266, 0);
			break;
		default:
			break;
		}

		if (nisId) {
			playNIS(nisId);

			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionEscapeBaggage:
		if (_engine->isDemo())
			break;

		switch (link->param1) {
		case 1:
			nisId = kEventCathStruggleWithBonds;
			break;
		case 2:
			nisId = kEventCathBurnRope;
			break;
		case 3:
			if (_gameEvents[kEventCathBurnRope]) {
				playNIS(kEventCathRemoveBonds);
				_gameProgress[kProgressField84] = 0;
				bumpCath(kCarBaggageRear, 89, 255);
				link->scene = kSceneNone;
			}

			break;
		case 4:
			if (!_gameEvents[kEventCathStruggleWithBonds2]) {
				playNIS(kEventCathStruggleWithBonds2);
				queueSFX(kCharacterCath, 101, 0);
				dropItem(kItemMatch, 2);
				if (!link->scene)
					cleanNIS();
			}
			break;
		case 5:
			send(kCharacterCath, kCharacterIvo, 192637492, 0);
			break;
		default:
			break;
		}

		if (nisId) {
			playNIS(nisId);
			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionEnterBaggage:
		switch (link->param1) {
		case 1:
			send(kCharacterCath, kCharacterAnna, 272177921, 0);
			break;
		case 2:
			if (!dialogRunning("MUS021"))
				playDialog(kCharacterCath, "MUS021", 16, 0);

			break;
		case 3:
			queueSFX(kCharacterCath, 43, 0);

			if (_engine->isDemo()) {
				forceJump(kCharacterAnna, &LogicManager::CONS_DemoAnna_BaggageFight);
				link->scene = kSceneNone;
			} else {
				if (cathHasItem(kItemKey)) {
					if (!_gameEvents[kEventAnnaBaggageArgument]) {
						forceJump(kCharacterAnna, &LogicManager::CONS_Anna_BaggageFight);
						link->scene = kSceneNone;
					}
				}
			}

			break;
		}

		break;
	case kActionBombPuzzle:
		if (_engine->isDemo())
			break;

		switch (link->param1) {
		case 1:
			send(kCharacterCath, kCharacterMaster, 158610240, 0);
			break;
		case 2:
			send(kCharacterCath, kCharacterMaster, 225367984, 0);
			_inventorySelectedItemIdx = 0;

			if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
				_engine->getGraphicsManager()->unlockSurface();
			}

			_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			link->scene = kSceneNone;

			break;
		case 3:
			send(kCharacterCath, kCharacterMaster, 191001984, 0);
			link->scene = kSceneNone;
			break;
		case 4:
			send(kCharacterCath, kCharacterMaster, 201959744, 0);
			link->scene = kSceneNone;
			break;
		case 5:
			send(kCharacterCath, kCharacterMaster, 169300225, 0);
			break;
		default:
			break;
		}

		break;
	case kActionConductors:
		if (_engine->isDemo())
			break;

		if (!cathRunningDialog("LIB031"))
			queueSFX(kCharacterCath, 31, 0);

		if (getCharacter(kCharacterCath).characterPosition.car == 3) {
			send(kCharacterCath, kCharacterCond1, 225358684, link->param1);
		} else if (getCharacter(kCharacterCath).characterPosition.car == 4) {
			send(kCharacterCath, kCharacterCond2, 225358684, link->param1);
		}

		break;
	case kActionKronosConcert:
		if (_engine->isDemo())
			break;

		if (link->param1 == 1) {
			nisId = kEventConcertSit;
		} else if (link->param1 == 2) {
			nisId = kEventConcertCough;
		}

		if (nisId) {
			playNIS(nisId);
			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionPlayMusic2:
		if (_engine->isDemo())
			break;

		_gameProgress[kProgressFieldC] = 1;
		queueSFX(kCharacterCath, link->param1, link->param2);
		Common::sprintf_s(filename, "MUS%03d", link->param3);
		if (!dialogRunning(filename))
			playDialog(kCharacterCath, filename, 16, 0);

		break;
	case kActionCatchBeetle:
		if (_engine->isDemo())
			break;

		if (_engine->_beetle && _engine->_beetle->click()) {
			_engine->endBeetle();
			_gameInventory[kItemBeetle].location = 1;
			send(kCharacterCath, kCharacterClerk, 202613084, 0);
		}

		break;

	case kActionCompartment:
	case kActionExitCompartment:
	case kActionEnterCompartment:
	{
		bool skipFlag = false;

		if (link->action != kActionCompartment) {
			if (!_engine->isDemo()) {
				if (link->action == kActionExitCompartment) {
					if (!_gameProgress[kProgressField30] && _gameProgress[kProgressJacket]) {
						_engine->getVCR()->writeSavePoint(1, kCharacterCath, 0);
						_gameProgress[kProgressField30] = 1;
					}

					setModel(1, link->param2);
				}

				if (_gameObjects[kItemMatchBox].door != 1 && _gameObjects[kItemMatchBox].door != 3 && _inventorySelectedItemIdx != kItemKey) {
					if (!_gameProgress[kProgressEventFoundCorpse]) {
						_engine->getVCR()->writeSavePoint(1, kCharacterCath, 0);
						playDialog(kCharacterCath, "LIB014", -1, 0);
						playNIS(kEventCathFindCorpse);
						playDialog(kCharacterCath, "LIB015", -1, 0);
						_gameProgress[kProgressEventFoundCorpse] = 1;
						link->scene = kSceneCompartmentCorpse;

						return;
					}
				} else {
					skipFlag = true;
				}
			} else {
				link->action = kActionCompartment;
			}
		}

		if (skipFlag || link->action == kActionCompartment || (link->action != kActionEnterCompartment || _gameInventory[kItemBriefcase].location != 2)) {
			if (link->param1 >= 128)
				return;

			if (_gameObjects[link->param1].character) {
				send(kCharacterCath, _gameObjects[link->param1].character, 9, link->param1);
				link->scene = kSceneNone;
				return;
			}

			if (!_engine->isDemo()) {
				if (bumpCathTowardsCond(link->param1, 1, 1)) {
					link->scene = kSceneNone;
					return;
				}
			}

			if (_gameObjects[link->param1].door == 1 || _gameObjects[link->param1].door == 3 || preventEnterComp(link->param1)) {
				if (_gameObjects[link->param1].door != 1 || preventEnterComp(link->param1) || _inventorySelectedItemIdx != 15 && (link->param1 != 1 || !cathHasItem(kItemKey) || _inventorySelectedItemIdx != kItemBriefcase && _inventorySelectedItemIdx != kItemFirebird)) {
					if (!cathRunningDialog("LIB013"))
						queueSFX(kCharacterCath, 13, 0);

					link->scene = kSceneNone;
					return;
				}

				queueSFX(kCharacterCath, 32, 0);

				if (link->param1 != 0 && link->param1 <= 3 || link->param1 >= 32 && link->param1 <= 37)
					setDoor(link->param1, kCharacterCath, 0, 10, 9);

				queueSFX(kCharacterCath, 15, 22);
				_inventorySelectedItemIdx = 0;
				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
			} else {
				if (link->action != 16 || _inventorySelectedItemIdx != kItemKey) {
					if (link->param1 == 109) {
						queueSFX(kCharacterCath, 26, 0);
					} else {
						queueSFX(kCharacterCath, 14, 0);
						queueSFX(kCharacterCath, 15, 22);
					}

					return;
				}

				setDoor(1, kCharacterCath, 1, 10, 9);
				queueSFX(kCharacterCath, 16, 0);
				_inventorySelectedItemIdx = 0;
				link->scene = kSceneNone;

				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
			}

			_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			return;
		}

		if (!_engine->isDemo()) {
			queueSFX(kCharacterCath, 14, 0);
			queueSFX(kCharacterCath, 15, 22);

			if (_gameProgress[kProgressField78] && !dialogRunning("MUS003")) {
				playDialog(kCharacterCath, "MUS003", 16, 0);
				_gameProgress[kProgressField78] = 0;
			}

			bumpCath(kCarGreenSleeping, 77, 255);
			link->scene = kSceneNone;
		}

		break;
	}

	case kActionOutsideTrain:
		if (_engine->isDemo())
			break;

		switch (link->param1) {
		case 1:
			send(kCharacterCath, kCharacterSalko, 167992577, 0);
			break;
		case 2:
			send(kCharacterCath, kCharacterVesna, 202884544, 0);
			break;
		case 3:
			if (_gameProgress[kProgressChapter] == 5) {
				send(kCharacterCath, kCharacterAbbot, 168646401, 0);
				send(kCharacterCath, kCharacterMilos, 168646401, 0);
			} else {
				send(kCharacterCath, kCharacterClerk, 203339360, 0);
			}

			link->scene = kSceneNone;
			break;
		case 4:
			send(kCharacterCath, kCharacterMilos, 169773228, 0);
			break;
		case 5:
			send(kCharacterCath, kCharacterVesna, 167992577, 0);
			break;
		case 6:
			send(kCharacterCath, kCharacterAugust, 203078272, 0);
			break;
		default:
			break;
		}

		break;
	case kActionFirebirdPuzzle:
		if (_engine->isDemo())
			break;

		if (_gameEvents[kEventKronosBringFirebird]) {
			switch (link->param1) {
			case 1:
				send(kCharacterCath, kCharacterAnna, 205294778, 0);
				break;
			case 2:
				send(kCharacterCath, kCharacterAnna, 224309120, 0);
				break;
			case 3:
				send(kCharacterCath, kCharacterAnna, 270751616, 0);
				break;
			}
		} else {
			switch (link->param1) {
			case 1:
			{
				if (inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
					nisId = kEventCathOpenEgg;
					Link *lnk = _trainData[link->scene].link;
					if (lnk)
						link->scene = lnk->scene;
				} else {
					nisId = kEventCathOpenEggNoBackground;
				}

				_gameProgress[kProgressIsEggOpen] = 1;
				break;
			}
			case 2:
				if (!inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
					nisId = kEventCathCloseEggNoBackground;
				} else {
					nisId = kEventCathCloseEgg;
				}

				_gameProgress[kProgressIsEggOpen] = 0;
				break;
			case 3:
				if (!inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
					nisId = kEventCathUseWhistleOpenEggNoBackground;
				} else {
					nisId = kEventCathUseWhistleOpenEgg;
				}

				break;
			}

			if (nisId) {
				playNIS(nisId);

				if (!link->scene)
					cleanNIS();
			}
		}

		break;
	case kActionOpenMatchBox:
		if (_engine->isDemo())
			break;

		if (_gameInventory[kItemMatch].location && !_gameInventory[kItemMatch].isPresent) {
			_gameInventory[kItemMatch].isPresent = 1;
			_gameInventory[kItemMatch].location = 0;
			queueSFX(kCharacterCath, 102, 0);
		}

		break;
	case kActionOpenBed:
		queueSFX(kCharacterCath, 59, 0);
		break;
	case kActionHintDialog:
		if (_engine->isDemo())
			break;

		if (dialogRunning(getHintDialog(link->param1))) {
			endDialog(getHintDialog(link->param1));
		}

		playDialog(kCharacterTableE, getHintDialog(link->param1), 16, 0);

		break;
	case kActionMusicEggBox:
		if (_engine->isDemo())
			break;

		queueSFX(kCharacterCath, 43, 0);
		if (_gameProgress[kProgressField7C] && !dialogRunning("MUS003")) {
			playDialog(kCharacterCath, "MUS003", 16, 0);
			_gameProgress[kProgressField7C] = 0;
		}

		break;
	case kActionPlayMusic3:
		if (_engine->isDemo())
			break;

		queueSFX(kCharacterCath, 24, 0);
		if (_gameProgress[kProgressField80] && !dialogRunning("MUS003")) {
			playDialog(kCharacterCath, "MUS003", 16, 0);
			_gameProgress[kProgressField80] = 0;
		}

		break;
	case kActionKnockInside:
	case kActionBed:
		if (_engine->isDemo())
			break;

		if (link->action == kActionBed) {
			queueSFX(kCharacterCath, 85, 0);
		}

		if (link->param1 < 128) {
			if (_gameObjects[link->param1].character)
				send(kCharacterCath, _gameObjects[link->param1].character, 8, link->param1);
		}

		return;
	case kActionPlayMusicChapter:
		switch (_gameProgress[kProgressChapter]) {
		case 1:
			musId = link->param1;
			break;
		case 2:
		case 3:
			musId = link->param2;
			break;
		case 4:
		case 5:
			musId = link->param3;
			break;
		}

		if (musId) {
			Common::sprintf_s(filename, "MUS%03d", musId);
			if (!dialogRunning(filename))
				playDialog(kCharacterCath, filename, 16, 0);
		}

		break;
	case kActionPlayMusicChapterSetupTrain:
		if (_engine->isDemo())
			break;

		switch (_gameProgress[kProgressChapter]) {
		case 1:
			musId = 1;
			break;
		case 2:
		case 3:
			musId = 2;
			break;
		case 4:
		case 5:
			musId = 4;
			break;
		}

		Common::sprintf_s(filename, "MUS%03d", link->param1);

		if (!dialogRunning(filename) && (link->param3 & musId) != 0) {
			playDialog(kCharacterCath, filename, 16, 0);

			fedEx(kCharacterCath, kCharacterClerk, 203863200, filename);
			send(kCharacterCath, kCharacterClerk, 222746496, link->param2);
		}

		break;
	case kActionEasterEggs:
		if (_engine->isDemo())
			break;

		if (link->param1 == 1) {
			send(kCharacterCath, kCharacterRebecca, 205034665, 0);
		} else if (link->param1 == 2) {
			send(kCharacterCath, kCharacterMaster, 225358684, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::takeTyler(bool doCleanNIS, int8 bedPosition) {
	if (!_gameProgress[kProgressJacket])
		_gameProgress[kProgressJacket] = 1;

	if (_gameInventory[kItemCorpse].location == 1) {
		if (bedPosition == 4) {
			if (_gameProgress[kProgressJacket])
				playNIS(kEventCorpsePickFloorOpenedBedOriginal);

			_gameInventory[kItemCorpse].location = 5;
		} else if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpsePickFloorGreen);
		} else {
			playNIS(kEventCorpsePickFloorOriginal);
		}
	} else if (_gameInventory[kItemCorpse].location == 2) {
		if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpsePickBedGreen);
		} else {
			playNIS(kEventCorpsePickBedOriginal);
		}
	}

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::dropTyler(bool doCleanNIS) {
	switch (_gameInventory[kItemCorpse].location) {
	case 1:
		if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpseDropFloorGreen);
		} else {
			playNIS(kEventCorpseDropFloorOriginal);
		}

		break;
	case 2:
		if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpseDropBedGreen);
		} else {
			playNIS(kEventCorpseDropBedOriginal);
		}

		break;
	case 4:
		_gameInventory[kItemCorpse].location = 0;
		_gameProgress[kProgressEventCorpseThrown] = 1;

		if (_gameTime <= 1138500) {
			if (_gameProgress[kProgressJacket] == 2) {
				playNIS(kEventCorpseDropWindowGreen);
			} else {
				playNIS(kEventCorpseDropWindowOriginal);
			}

			_gameProgress[kProgressField24] = 1;
		} else {
			playNIS(kEventCorpseDropBridge);
		}

		_gameProgress[kProgressEventCorpseMovedFromFloor] = 1;

		break;
	}

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::takeJacket(bool doCleanNIS) {
	_gameProgress[kProgressJacket] = 2;
	_gameInventory[kItemMatchBox].isPresent = 1;
	_gameInventory[kItemMatchBox].location = 0;
	setDoor(9, kCharacterCath, 2, 255, 255);
	playNIS(kEventPickGreenJacket);
	_gameProgress[kProgressPortrait] = 34;
	_engine->getGraphicsManager()->drawItemDim(_gameProgress[kProgressPortrait], 0, 0, 1);
	_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::takeScarf(bool doCleanNIS) {
	if (_gameProgress[kProgressJacket] == 2) {
		playNIS(kEventPickScarfGreen);
	} else {
		playNIS(kEventPickScarfOriginal);
	}

	if (doCleanNIS)
		cleanNIS();
}

const char *LogicManager::getHintDialog(int character) {
	if (whoRunningDialog(kCharacterTableE))
		return nullptr;

	switch (character) {
	case kCharacterAnna:
		if (_gameEvents[kEventAnnaDialogGoToJerusalem]) {
			return "XANN12";
		} else if (_gameEvents[kEventLocomotiveRestartTrain]) {
			return "XANN11";
		} else if (_gameEvents[kEventAnnaBaggageTies] ||
				   _gameEvents[kEventAnnaBaggageTies2] ||
				   _gameEvents[kEventAnnaBaggageTies3] ||
				   _gameEvents[kEventAnnaBaggageTies4]) {
			return "XANN10";
		} else if (_gameEvents[kEventAnnaTired] ||
				   _gameEvents[kEventAnnaTiredKiss]) {
			return "XANN9";
		} else if (_gameEvents[kEventAnnaBaggageArgument]) {
			return "XANN8";
		} else if (_gameEvents[kEventKronosVisit]) {
			return "XANN7";
		} else if (_gameEvents[kEventAbbotIntroduction]) {
			return "XANN6A";
		} else if (_gameEvents[kEventVassiliSeizure]) {
			return "XANN6";
		} else if (_gameEvents[kEventAugustPresentAnna] ||
				   _gameEvents[kEventAugustPresentAnnaFirstIntroduction]) {
			return "XANN5";
		} else if (_gameProgress[kProgressField60]) {
			return "XANN4";
		} else if (_gameEvents[kEventAnnaGiveScarf] ||
				   _gameEvents[kEventAnnaGiveScarfDiner] ||
				   _gameEvents[kEventAnnaGiveScarfSalon] ||
				   _gameEvents[kEventAnnaGiveScarfMonogram] ||
				   _gameEvents[kEventAnnaGiveScarfDinerMonogram] ||
				   _gameEvents[kEventAnnaGiveScarfSalonMonogram]) {
			return "XANN3";
		} else if (_gameEvents[kEventDinerMindJoin]) {
			return "XANN2";
		} else if (_gameEvents[kEventGotALight] ||
				   _gameEvents[kEventGotALightD]) {
			return "XANN1";
		}

		break;
	case kCharacterAugust:
		if (_gameEvents[kEventAugustTalkCigar]) {
			return "XAUG6";
		} else if (_gameEvents[kEventAugustBringBriefcase]) {
			return "XAUG5";
		} else if (_gameEvents[kEventAugustMerchandise]) {
			if (_gameTime <= 2200500) {
				return "XAUG4";
			} else {
				return "XAUG4A";
			}
		} else if (_gameEvents[kEventDinerAugust] ||
				   _gameEvents[kEventDinerAugustAlexeiBackground] ||
				   _gameEvents[kEventMeetAugustTylerCompartment] ||
				   _gameEvents[kEventMeetAugustHisCompartment] ||
				   _gameEvents[kEventMeetAugustTylerCompartmentBed] ||
				   _gameEvents[kEventMeetAugustHisCompartmentBed]) {
			return "XAUG3";
		} else if (_gameEvents[kEventAugustPresentAnnaFirstIntroduction]) {
			return "XAUG2";
		} else if (_gameProgress[kProgressEventMertensAugustWaiting]) {
			return "XAUG1";
		}

		break;
	case kCharacterTatiana:
		if (_gameEvents[kEventTatianaTylerCompartment]) {
			return "XTAT6";
		} else if (_gameEvents[kEventTatianaCompartmentStealEgg]) {
			return "XTAT5";
		} else if (_gameEvents[kEventTatianaGivePoem]) {
			return "XTAT3";
		} else if (_gameProgress[kProgressField64]) {
			return "XTAT1";
		}

		break;
	case kCharacterVassili:
		if (_gameEvents[kEventCathFreePassengers]) {
			return "XVAS4";
		} else if (_gameEvents[kEventVassiliCompartmentStealEgg]) {
			return "XVAS3";
		} else if (_gameEvents[kEventAbbotIntroduction]) {
			return "XVAS2";
		} else if (_gameEvents[kEventVassiliSeizure]) {
			return "XVAS1A";
		} else if (_gameProgress[kProgressField64]) {
			return "XVAS1";
		}

		break;
	case kCharacterAlexei:
		if (_gameProgress[kProgressField88]) {
			return "XALX6";
		} else if (_gameProgress[kProgressField8C]) {
			return "XALX5";
		} else if (_gameProgress[kProgressField90]) {
			return "XALX4A";
		} else if (_gameProgress[kProgressField68]) {
			return "XALX4";
		} else if (_gameEvents[kEventAlexeiSalonPoem]) {
			return "XALX3";
		} else if (_gameEvents[kEventAlexeiSalonVassili]) {
			return "XALX2";
		} else if (_gameEvents[kEventAlexeiDiner] ||
				   _gameEvents[kEventAlexeiDinerOriginalJacket]) {
			return "XALX1";
		}

		break;
	case kCharacterAbbot:
		if (_gameEvents[kEventAbbotDrinkDefuse]) {
			return "XABB4";
		} else if (_gameEvents[kEventAbbotInvitationDrink] ||
				   _gameEvents[kEventDefuseBomb]) {
			return "XABB3";
		} else if (_gameEvents[kEventAbbotWrongCompartment] ||
				   _gameEvents[kEventAbbotWrongCompartmentBed]) {
			return "XABB2";
		} else if (_gameEvents[kEventAbbotIntroduction]) {
			return "XABB1";
		}

		break;
	case kCharacterMilos:
		if (_gameEvents[kEventLocomotiveMilosDay] || _gameEvents[kEventLocomotiveMilosNight]) {
			return "XMIL5";
		} else if (_gameEvents[kEventMilosCompartmentVisitTyler] &&
				  (_gameProgress[kProgressChapter] == 3 ||
				   _gameProgress[kProgressChapter] == 4)) {
			return "XMIL4";
		} else if (_gameEvents[kEventMilosCorridorThanks] ||
				   _gameProgress[kProgressChapter] == 5) {
			return "XMIL3";
		} else if (_gameEvents[kEventMilosCompartmentVisitAugust]) {
			return "XMIL2";
		} else if (_gameEvents[kEventMilosTylerCompartmentDefeat]) {
			return "XMIL1";
		}

		break;
	case kCharacterVesna:
		if (_gameProgress[kProgressField94]) {
			return "XVES2";
		} else if (_gameProgress[kProgressField98]) {
			return "XVES1";
		}

		break;
	case kCharacterKronos:
		if (_gameEvents[kEventKronosReturnBriefcase])
			return "XKRO6";
		if (_gameEvents[kEventKronosBringEggCeiling] ||
			_gameEvents[kEventKronosBringEgg]) {
			return "XKRO5";
		} else {
			if (_gameEvents[kEventKronosConversation] ||
				_gameEvents[kEventKronosConversationFirebird]) {
				if (_gameInventory[kItemFirebird].location != 6 &&
					_gameInventory[kItemFirebird].location != 5 &&
					_gameInventory[kItemFirebird].location != 2 &&
					_gameInventory[kItemFirebird].location != 1)
					return "XKRO4A";
			}

			if (_gameEvents[kEventKronosConversationFirebird])
				return "XKRO4";

			if (_gameEvents[kEventMilosCompartmentVisitAugust]) {
				if (_gameEvents[kEventKronosConversation])
					return "XKRO3";
			} else if (_gameEvents[kEventKronosConversation]) {
				return "XKRO2";
			}

			if (_gameProgress[kProgressEventMertensChronosInvitation]) {
				return "XKRO1";
			}
		}

		break;
	case kCharacterFrancois:
		if (_gameProgress[kProgressField9C]) {
			return "XFRA3";
		} else if (_gameProgress[kProgressFieldA0] ||
				   _gameEvents[kEventFrancoisWhistle] ||
				   _gameEvents[kEventFrancoisWhistleD] ||
				   _gameEvents[kEventFrancoisWhistleNight] ||
				   _gameEvents[kEventFrancoisWhistleNightD]) {
			return "XFRA2";
		} else if (_gameTime <= 1075500) {
			return "XFRA1";
		}

		break;
	case kCharacterMadame:
		if (_gameProgress[kProgressFieldA4]) {
			return "XMME4";
		} else if (_gameProgress[kProgressFieldA8]) {
			return "XMME3";
		} else if (_gameProgress[kProgressFieldA0]) {
			return "XMME2";
		} else  if (_gameProgress[kProgressFieldAC]) {
			return "XMME1";
		}

		break;
	case kCharacterMonsieur:
		if (_gameProgress[kProgressEventMetBoutarel]) {
			return "XMRB1";
		}

		break;
	case kCharacterRebecca:
		if (_gameProgress[kProgressFieldB4]) {
			return "XREB1A";
		} else if (_gameProgress[kProgressFieldB8]) {
			return "XREB1";
		}

		break;
	case kCharacterSophie:
		if (_gameProgress[kProgressFieldB0]) {
			return "XSOP2";
		} else if (_gameProgress[kProgressFieldBC]) {
			return "XSOP1B";
		} else if (_gameProgress[kProgressFieldB4]) {
			return "XSOP1A";
		} else if (!_gameProgress[kProgressFieldB8]) {
			return "XSOP1";
		}

		break;
	case kCharacterMahmud:
		if (_gameProgress[kProgressFieldC4]) {
			return "XMAH1";
		}

		break;
	case kCharacterYasmin:
		if (_gameProgress[kProgressEventMetYasmin]) {
			return "XHAR2";
		}

		break;
	case kCharacterHadija:
		if (_gameProgress[kProgressEventMetHadija]) {
			return "XHAR1";
		}

		break;
	case kCharacterAlouan:
		if (_gameProgress[kProgressFieldDC]) {
			return "XHAR3";
		}

		break;
	case kCharacterPolice:
		if (_gameProgress[kProgressFieldE0]) {
			return "XHAR4";
		}

		break;
	case kCharacterMaster:
		if (_gameEvents[kEventCathDream] || _gameEvents[kEventCathWakingUp]) {
			return "XTYL3";
		} else {
			return "XTYL1";
		}

		break;
	default:
		break;
	}

	return nullptr;
}

} // End of namespace LastExpress
