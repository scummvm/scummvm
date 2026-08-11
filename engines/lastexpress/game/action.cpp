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
	case kActionInventory:
		if (!_nodeReturn2 && (_doneNIS[kEventKronosBringFirebird] || _globals[kEventAugustBringEgg])) {
			result = kCursorNormal;
		} else {
			result = kCursorBackward;
		}

		break;
	case kActionKnock:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else {
			result = _doors[link->param1].windowCursor;
		}

		break;
	case kActionCompartment:
	case kActionExitCompartment:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else if (_activeItem != kItemKey || (_doors[link->param1].who) || _doors[link->param1].status != 1 || !_doors[link->param1].handleCursor || inComp(kCharacterCath) || preventEnterComp(link->param1)) {
			result = _doors[link->param1].handleCursor;
		} else {
			result = _items[kItemKey].mnum;
		}

		break;
	case kActionKnockInside:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else {
			if (_doors[link->param1].who)
				result = _doors[link->param1].windowCursor;
			else
				result = kCursorNormal;
		}

		break;
	case kActionTakeItem:
		if (link->param1 >= 32) {
			result = kCursorNormal;
		} else if ((!_activeItem || _items[_activeItem].inPocket) && (link->param1 != 21 || _globals[kGlobalCorpseMovedFromFloor] == 1)) {
			result = kCursorHand;
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionDropItem:
		if (link->param1 >= 32) {
			result = kCursorNormal;
		} else if (link->param1 != _activeItem || (link->param1 == 20 && !_globals[kGlobalTrainIsRunning] && link->param2 == 4) || (link->param1 == 18 && link->param2 == 1 && _globals[kGlobalTatianaFoundOutEggStolen])) {
			result = kCursorNormal;
		} else {
			result = _items[_activeItem].mnum;
		}

		break;
	case kActionLinkOnGlobal:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else if (_globals[link->param1] == link->param2) {
			result = link->param3;
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionRattle:
		if ((_activeItem == kItemKey && !_doors[1].status) || (_doors[1].status == 1 && cathHasItem(kItemKey) && (_activeItem == kItemBriefcase || _activeItem == kItemFirebird))) {
			result = _items[kItemKey].mnum;
		} else {
			if (link->param1 >= 128) {
				result = 0;
			} else if (_activeItem != kItemKey ||
				_doors[link->param1].who ||
				_doors[link->param1].status != 1 ||
				!_doors[link->param1].handleCursor ||
				inComp(kCharacterCath) || preventEnterComp(link->param1)) {
				result = _doors[link->param1].handleCursor;
			} else {
				result = _items[kItemKey].mnum;
			}
		}

		break;
	case kActionLeanOutWindow:
		if (_globals[kGlobalJacket] == 2) {
			if ((_doneNIS[kEventCathLookOutsideWindowDay] || _doneNIS[kEventCathLookOutsideWindowNight] || getModel(1) == 1) &&
				_globals[kGlobalTrainIsRunning] &&
				(link->param1 != 45 || (!inComp(kCharacterRebecca, kCarRedSleeping, 4840) && _doors[kObjectOutsideBetweenCompartments].status == 2)) &&
				_activeItem != kItemBriefcase && _activeItem != kItemFirebird) {
				result = kCursorForward;
			} else {
				result = getModel(1) == 1 ? kCursorNormal : kCursorMagnifier;
			}
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionAlmostFall:
		result = _globals[kGlobalAlmostFallActionIsAvailable] == 0 ? kCursorNormal : kCursorLeft;
		break;
	case kActionClimbLadder:
		if (_globals[kGlobalTrainIsRunning] && _activeItem != kItemBriefcase && _activeItem != kItemFirebird &&
			(_globals[kGlobalChapter] == 2 || _globals[kGlobalChapter] == 3 || _globals[kGlobalChapter] == 5)) {
			result = kCursorUp;
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionKronosSanctum:
		if (link->param1 == 1) {
			result = checkDoor(73) == 0 ? kCursorHand : kCursorNormal;
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionEscapeBaggage:
		if (link->param1 == 2) {
			if (!_doneNIS[kEventCathStruggleWithBonds2] || _doneNIS[kEventCathBurnRope])
				result = kCursorNormal;
			else
				result = kCursorHand;
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionCatchBeetle:
		if (_engine->_beetle) {
			if (_engine->_beetle->onTable()) {
				if (_activeItem == kItemMatchBox && cathHasItem(kItemMatch))
					result = _items[kItemMatchBox].mnum;
				else
					result = kCursorHandPointer;
			} else {
				result = kCursorNormal;
			}
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionFirebirdPuzzle:
		if (link->param1 == 3) {
			if (_activeItem == kItemWhistle)
				result = _items[kItemWhistle].mnum;
			else
				result = kCursorNormal;
		} else {
			result = kCursorNormal;
		}

		break;
	case kActionOpenBed:
		result = _globals[kGlobalChapter] == 1 ? kCursorHand : kCursorNormal;
		break;
	case kActionHintDialog:
		result = getHintDialog(link->param1) == 0 ? kCursorNormal : kCursorHandPointer;
		break;
	case kActionBed:
		if (_globals[kGlobalPhaseOfTheNight] == 2 && !_globals[kGlobalTatianaScheduledToVisitCath] && (_gameTime > 1404000 || (_globals[kGlobalMetAugust] && _globals[kGlobalMetMilos] && (!_globals[kGlobalFrancoisHasSeenCorpseThrown] || _globals[kGlobalPoliceHasBoardedAndGone])))) {
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
	switch (_trainData[_activeNode].property) {
	case kNodeHasItem:
		if (_trainData[_activeNode].parameter1 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kNodeHas2Items:
		if (_trainData[_activeNode].parameter1 != item && _trainData[_activeNode].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kNodeHasDoorItem:
		if (_trainData[_activeNode].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kNodeHas3Items:
		if (_trainData[_activeNode].parameter1 != item && _trainData[_activeNode].parameter2 != item && _trainData[_activeNode].parameter3 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kNodeSoftPointItem:
		if (_trainData[_activeNode].parameter2 != item) {
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

	switch (_trainData[*sceneOut].property) {
	case kNodeHasDoor:
		if (_trainData[*sceneOut].parameter1 < 128) {
			if (_doors[_trainData[*sceneOut].parameter1].status) {
				link = _trainData[*sceneOut].link;
				for (bool found = false; link && !found; link = link->next) {
					if (_doors[_trainData[*sceneOut].parameter1].status == link->location) {
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
	case kNodeHasItem:
		if (_trainData[*sceneOut].parameter1 < 32) {
			if (_items[_trainData[*sceneOut].parameter1].floating) {
				link = _trainData[*sceneOut].link;
				for (bool found = false; link && !found; link = link->next) {
					if (_items[_trainData[*sceneOut].parameter1].floating == link->location) {
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
	case kNodeHas2Items:
		if (_trainData[*sceneOut].parameter1 < 32) {
			if (_trainData[*sceneOut].parameter2 < 32) {
				int locFlag = (_items[_trainData[*sceneOut].parameter1].floating != 0) ? 1 : 0;

				if (_items[_trainData[*sceneOut].parameter2].floating)
					locFlag |= 2;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_items[_trainData[*sceneOut].parameter1].floating == link->param1 && _items[_trainData[*sceneOut].parameter2].floating == link->param2) {
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
	case kNodeHasDoorItem:
		if (_trainData[*sceneOut].parameter1 < 128) {
			if (_trainData[*sceneOut].parameter2 < 32) {
				int locFlag = (_doors[_trainData[*sceneOut].parameter1].status == 2) ? 1 : 0;

				if (_items[_trainData[*sceneOut].parameter2].floating)
					locFlag |= 2;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_doors[_trainData[*sceneOut].parameter1].status == link->param1 && _items[_trainData[*sceneOut].parameter2].floating == link->param2) {
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
	case kNodeHas3Items:
		if (_trainData[*sceneOut].parameter1 < 32 && _trainData[*sceneOut].parameter2 < 32) {
			if (_trainData[*sceneOut].parameter3 < 32) {
				int locFlag = (_items[_trainData[*sceneOut].parameter1].floating != 0) ? 1 : 0;

				if (_items[_trainData[*sceneOut].parameter2].floating)
					locFlag |= 2;

				if (_items[_trainData[*sceneOut].parameter3].floating)
					locFlag |= 4;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_items[_trainData[*sceneOut].parameter1].floating == link->param1 && _items[_trainData[*sceneOut].parameter2].floating == link->param2 && _items[_trainData[*sceneOut].parameter3].floating == link->param3) {
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
	case kNodeModelPad:
		if (_trainData[*sceneOut].parameter1 < 128) {
			link = _trainData[*sceneOut].link;
			bool found = false;
			if (link) {
				while (!found) {
					if (_doors[_trainData[*sceneOut].parameter1].model == link->location) {
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
	case kNodeSoftPoint:
		if (_trainData[*sceneOut].parameter1 < 16 &&
			(_softBlockedX[_trainData[*sceneOut].parameter1] || _blockedX[_trainData[*sceneOut].parameter1])) {
			if ((!_engine->getOtisManager()->fDirection(_activeNode) || !_engine->getOtisManager()->fDirection(*sceneOut) || _trainData[_activeNode].nodePosition.position >= _trainData[*sceneOut].nodePosition.position) &&
				(!_engine->getOtisManager()->rDirection(_activeNode) || !_engine->getOtisManager()->rDirection(*sceneOut) || _trainData[_activeNode].nodePosition.position <= _trainData[*sceneOut].nodePosition.position)) {
				next = _trainData[*sceneOut].link->next;
				scene = next->scene;
				*sceneOut = (int)scene;
				doPreFunction(sceneOut);

				break;
			}

			if (whoseBit(_softBlockedX[_trainData[*sceneOut].parameter1]) != 30 &&
				whoseBit(_blockedX[_trainData[*sceneOut].parameter1]) != 30) {
				playDialog(kCharacterCath, "CAT1126A", -1, 0);
			}

			scene = _trainData[*sceneOut].link->scene;
			*sceneOut = (int)scene;
			doPreFunction(sceneOut);
		}

		break;
	case kNodeSoftPointItem:
		if (_trainData[*sceneOut].parameter1 < (_engine->isDemo() ? 16 : 32)) {
			if (_softBlockedX[_trainData[*sceneOut].parameter1] || _blockedX[_trainData[*sceneOut].parameter1]) {
				if ((_engine->getOtisManager()->fDirection(_activeNode) &&
					_engine->getOtisManager()->fDirection(*sceneOut) &&
					_trainData[_activeNode].nodePosition.position < _trainData[*sceneOut].nodePosition.position) ||
					(_engine->getOtisManager()->rDirection(_activeNode) &&
					_engine->getOtisManager()->rDirection(*sceneOut) &&
					_trainData[_activeNode].nodePosition.position > _trainData[*sceneOut].nodePosition.position)) {

					if (whoseBit(_softBlockedX[_trainData[*sceneOut].parameter1]) != 30 && whoseBit(_blockedX[_trainData[*sceneOut].parameter1]) != 30) {
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
					if (_items[_trainData[*sceneOut].parameter2].floating) {
						link = _trainData[*sceneOut].link;
						for (bool found = false; link && !found; link = link->next) {
							if (_items[_trainData[*sceneOut].parameter2].floating == link->location) {
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
		if (_trainData[*sceneOut].property != kNodeRebeccaDiary || _trainData[*sceneOut].parameter1)
			fadeDialog(kCharacterTableE);
	}

	if (!_engine->isDemo() && _engine->_beetle) {
		if (_trainData[*sceneOut].property != kNodeBeetle)
			_engine->endBeetle();
	}
}

void LogicManager::doPostFunction() {
	Link tmp;

	switch (_trainData[_activeNode].property) {
	case kNodeAutoWalk:
	{
		int32 delta = _trainData[_activeNode].parameter1 + 10;
		_gameTime += delta * _timeSpeed;
		_realTime += delta;
		int32 delayedTicks = _engine->getSoundFrameCounter() + 4 * _trainData[_activeNode].parameter1;

		if (!_engine->mouseHasRightClicked() && delayedTicks > _engine->getSoundFrameCounter()) {
			do {
				if (_engine->mouseHasRightClicked())
					break;
				_engine->getSoundManager()->soundThread();
				_engine->getSubtitleManager()->subThread();
				_engine->waitForTimer(4);
			} while (delayedTicks > _engine->getSoundFrameCounter());
		}

		tmp.copyFrom(_trainData[_activeNode].link);
		doAction(&tmp);

		if (_engine->mouseHasRightClicked() && _trainData[tmp.scene].property == kNodeAutoWalk) {
			do {
				tmp.copyFrom(_trainData[tmp.scene].link);
				doAction(&tmp);
			} while (_trainData[tmp.scene].property == kNodeAutoWalk);
		}

		if (getCharacter(kCharacterCath).characterPosition.car == kCarVestibule &&
			(getCharacter(kCharacterCath).characterPosition.position == 4 ||
			 getCharacter(kCharacterCath).characterPosition.position == 3)) {

			int characterIdx = 0;
			int charactersRndArray[39];

			memset(charactersRndArray, 0, sizeof(charactersRndArray));

			for (int j = 1; j < 40; j++) {
				if (getCharacter(kCharacterCath).characterPosition.position == 4) {
					if ((getCharacter(j).characterPosition.car == kCarRedSleeping && getCharacter(j).characterPosition.position > 9270) || (getCharacter(j).characterPosition.car == kCarRestaurant && getCharacter(j).characterPosition.position < 1540)) {
						charactersRndArray[characterIdx] = j;
						characterIdx++;
					}
				} else if ((getCharacter(j).characterPosition.car == kCarGreenSleeping && getCharacter(j).characterPosition.position > 9270) || (getCharacter(j).characterPosition.car == kCarRedSleeping && getCharacter(j).characterPosition.position < 850)) {
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
	case kNodeSleepingOnBed:
		if (_engine->isDemo())
			break;

		if (_globals[kGlobalPhaseOfTheNight] == 2)
			send(kCharacterCath, kCharacterMaster, 190346110, 0);

		return;
	case kNodeBeetle:
		if (_engine->isDemo())
			break;

		_engine->doBeetle();
		return;
	case kNodePullingStop:
	{
		if (_gameTime < 2418300 && _globals[kGlobalPhaseOfTheNight] != 4) {
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

			if (_globals[kGlobalChapter] == 1) {
				endGame(0, 0, 62, true);
			} else if (_globals[kGlobalChapter] == 4) {
				endGame(0, 0, 64, true);
			} else {
				endGame(0, 0, 63, true);
			}
		}

		return;
	}
	case kNodeRebeccaDiary:
		if (!whoRunningDialog(kCharacterTableE)) {
			switch (_trainData[_activeNode].parameter1) {
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
	case kNodeExitFastWalk:
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
		if (_closeUp) {
			int bumpScene = 0;
			if (_nodeReturn2) {
				bumpScene = _nodeReturn2;
				_nodeReturn2 = 0;
				bumpCathNode(bumpScene);
			} else {
				_closeUp = 0;

				if (_blockedViews[100 * _trainData[_nodeReturn].nodePosition.car + _trainData[_nodeReturn].cathDir]) {
					bumpCathNode(getSmartBumpNode(_nodeReturn));
				} else {
					bumpCathNode(_nodeReturn);
				}
			}

			if (_activeItem && (!_items[_activeItem].useable || (bumpScene == 0 && findLargeItem()))) {
				_activeItem = findLargeItem();

				if (_activeItem) {
					_engine->getGraphicsManager()->drawItem(_items[_activeItem].mnum, 44, 0);
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
		if (!dialogRunning(filename) && (link->param1 != 50 || _globals[kGlobalChapter] == 5))
			playDialog(kCharacterCath, filename, 16, link->param2);

		break;
	case kActionKnock:
		if (link->param1 < 128) {
			if (_doors[link->param1].who) {
				send(kCharacterCath, _doors[link->param1].who, 8, link->param1);
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
		if (!_doneNIS[link->param1]) {
			playNIS(link->param1);

			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionSetDoor:
		if (link->param1 >= 128)
			return;

		setDoor(link->param1, _doors[link->param1].who, link->param2, 255, 255);
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
			if (!_items[link->param1].haveIt) {
				_items[link->param1].floating = link->param2;

				if (link->param1 == kItemCorpse) {
					_globals[kGlobalCorpseMovedFromFloor] = (_items[kItemCorpse].floating == 3 || _items[kItemCorpse].floating == 4) ? 1 : 0;
				}
			}
		}

		break;
	case kActionTakeItem:
		if (_engine->isDemo())
			break;

		if (link->param1 >= 32)
			return;

		if (!_items[link->param1].floating)
			return;

		if (link->param1 == kItemCorpse) {
			takeTyler(link->scene == 0, link->param2);

			if (link->param2 != 4) {
				_items[kItemCorpse].haveIt = 1;
				_items[kItemCorpse].floating = 0;
				_activeItem = kItemCorpse;
				_engine->getGraphicsManager()->drawItem(_items[kItemCorpse].mnum, 44, 0);
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}
		} else {
			_items[link->param1].haveIt = 1;
			_items[link->param1].floating = 0;

			if (link->param1 == kItemGreenJacket) {
				takeJacket(link->scene == 0);
			} else {
				if (link->param1 == kItemScarf) {
					takeScarf(link->scene == 0);
					return;
				}

				if (link->param1 == kItemParchemin && link->param2 == 2) {
					_items[kItemParchemin].haveIt = 1;
					_items[kItemParchemin].floating = 0;
					_items[kItem11].floating = 1;
					queueSFX(kCharacterCath, 9, 0);
				} else if (link->param1 == kItemBomb) {
					forceJump(kCharacterAbbot, &LogicManager::CONS_Abbot_CatchCath);
				} else if (link->param1 == kItemBriefcase) {
					queueSFX(kCharacterCath, 83, 0);
				}
			}

			if (_items[link->param1].closeUp) {
				if (!_closeUp) {
					if (!link->scene)
						link->scene = _activeNode;

					_closeUp = 1;
					_nodeReturn = link->scene;
				}

				bumpCathNode(_items[link->param1].closeUp);
				link->scene = kSceneNone;
			}

			if (_items[link->param1].useable) {
				_activeItem = link->param1;
				_engine->getGraphicsManager()->drawItem(_items[link->param1].mnum, 44, 0);
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}
		}

		break;
	case kActionDropItem:
		if (_engine->isDemo())
			break;

		if (link->param1 >= 32 || !_items[link->param1].haveIt || !link->param2)
			return;

		if (link->param1 == kItemBriefcase) {
			queueSFX(kCharacterCath, 82, 0);

			if (link->param2 == 2) {
				if (!_globals[kGlobalDoneSavePointAfterLeavingSuitcaseInCathComp]) {
					_engine->getVCR()->writeSavePoint(1, 0, 0);
					_globals[kGlobalDoneSavePointAfterLeavingSuitcaseInCathComp] = 1;
				}

				if (_items[kItemParchemin].floating == 2) {
					_items[kItemParchemin].haveIt = 1;
					_items[kItemParchemin].floating = 0;
					_items[kItem11].floating = 1;
					queueSFX(kCharacterCath, 9, 0);
				}
			}
		}

		_items[link->param1].haveIt = 0;
		_items[link->param1].floating = link->param2;
		if (link->param1 == kItemCorpse)
			dropTyler(link->scene == 0);

		_activeItem = 0;

		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
			_engine->getGraphicsManager()->unlockSurface();
		}

		_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
		break;
	
	case kActionLeanOutWindow:
		if (_engine->isDemo())
			break;

		if ((!_doneNIS[kEventCathLookOutsideWindowDay] && !_doneNIS[kEventCathLookOutsideWindowNight] && getModel(1) != 1) || !_globals[kGlobalTrainIsRunning] || (link->param1 == 45 && (inComp(kCharacterRebecca, kCarRedSleeping, 4840) || _doors[44].status != 2)) || _activeItem == kItemBriefcase || _activeItem == kItemFirebird) {
			if (link->param1 == 9 || (link->param1 >= 44 && link->param1 <= 45)) {
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
			_doneNIS[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGoOutsideTylerCompartmentNight);
			} else {
				playNIS(kEventCathGoOutsideTylerCompartmentDay);
			}

			_globals[kGlobalAlmostFallActionIsAvailable] = 1;
			break;
		case 44:
			_doneNIS[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGoOutsideNight);
			} else {
				playNIS(kEventCathGoOutsideDay);
			}

			_globals[kGlobalAlmostFallActionIsAvailable] = 1;
			break;
		case 45:
			_doneNIS[kEventCathLookOutsideWindowDay] = 1;

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

			_globals[kGlobalAlmostFallActionIsAvailable] = 0;

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

			_globals[kGlobalAlmostFallActionIsAvailable] = 0;

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
			if (_globals[kGlobalChapter] == 2 || _globals[kGlobalChapter] == 3) {
				playNIS(kEventCathTopTrainGreenJacket);
			} else if (_globals[kGlobalChapter] == 5) {
				playNIS(kEventCathTopTrainNoJacketDay - (_globals[kGlobalIsDayTime] == 0));
			}

			if (link->scene)
				return;
		} else {
			if (link->param1 != 2)
				return;

			if (_globals[kGlobalChapter] == 2 || _globals[kGlobalChapter] == 3) {
				playNIS(kEventCathClimbUpTrainGreenJacket);
				playNIS(kEventCathTopTrainGreenJacket);
			} else if (_globals[kGlobalChapter] == 5) {
				playNIS(kEventCathClimbUpTrainNoJacketDay - (_globals[kGlobalIsDayTime] == 0));
				playNIS(kEventCathTopTrainNoJacketDay - (_globals[kGlobalIsDayTime] == 0));
			}

			if (link->scene)
				return;
		}

		cleanNIS();
		break;
	case kActionClimbDownTrain:
		if (_engine->isDemo())
			break;

		if (_globals[kGlobalChapter] == 2 || _globals[kGlobalChapter] == 3) {
			nisId = kEventCathClimbDownTrainGreenJacket;
		} else if (_globals[kGlobalChapter] == 5) {
			if (_globals[kGlobalIsDayTime] == 0) {
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
			if (_activeItem == kItemBriefcase) {
				_items[kItemBriefcase].floating = 3;
				_items[kItemBriefcase].haveIt = 0;
				queueSFX(kCharacterCath, 82, 0);
				_activeItem = 0;
				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}

			if (_items[kItemBriefcase].floating == 3) {
				nisId = kEventCathJumpUpCeiling;
			} else {
				nisId = kEventCathJumpUpCeilingBriefcase;
			}

			break;
		case 4:
			if (_globals[kGlobalChapter] == 1)
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
			if (_doneNIS[kEventCathBurnRope]) {
				playNIS(kEventCathRemoveBonds);
				_globals[kGlobalCathInSpecialState] = 0;
				bumpCath(kCarBaggageRear, 89, 255);
				link->scene = kSceneNone;
			}

			break;
		case 4:
			if (!_doneNIS[kEventCathStruggleWithBonds2]) {
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
					if (!_doneNIS[kEventAnnaBaggageArgument]) {
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
			_activeItem = 0;

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
	case kActionLetterInAugustSuitcase:
		if (_engine->isDemo())
			break;

		_globals[kGlobalReadLetterInAugustSuitcase] = 1;
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
			_items[kItemBeetle].floating = 1;
			send(kCharacterCath, kCharacterClerk, 202613084, 0);
		}

		break;

	case kActionCompartment:
	case kActionExitCompartment:
	case kActionRattle:
	{
		bool skipFlag = false;

		if (link->action != kActionCompartment) {
			if (!_engine->isDemo()) {
				if (link->action == kActionExitCompartment) {
					if (!_globals[kGlobalDoneSavePointAfterLeftCompWithNewJacket] && _globals[kGlobalJacket] != 0) {
						_engine->getVCR()->writeSavePoint(1, kCharacterCath, 0);
						_globals[kGlobalDoneSavePointAfterLeftCompWithNewJacket] = 1;
					}

					setModel(1, link->param2);
				}

				if (_doors[1].status != 1 && _doors[1].status != 3 && _activeItem != kItemKey) {
					if (!_globals[kGlobalFoundCorpse]) {
						_engine->getVCR()->writeSavePoint(1, kCharacterCath, 0);
						playDialog(kCharacterCath, "LIB014", -1, 0);
						playNIS(kEventCathFindCorpse);
						playDialog(kCharacterCath, "LIB015", -1, 0);
						_globals[kGlobalFoundCorpse] = 1;
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

		if (skipFlag || link->action == kActionCompartment || (link->action != kActionRattle || _items[kItemBriefcase].floating != 2)) {
			if (link->param1 >= 128)
				return;

			if (_doors[link->param1].who) {
				send(kCharacterCath, _doors[link->param1].who, 9, link->param1);
				link->scene = kSceneNone;
				return;
			}

			if (!_engine->isDemo()) {
				if (bumpCathTowardsCond(link->param1, 1, 1)) {
					link->scene = kSceneNone;
					return;
				}
			}

			if (_doors[link->param1].status == 1 || _doors[link->param1].status == 3 || preventEnterComp(link->param1)) {
				if (_doors[link->param1].status != 1 || preventEnterComp(link->param1) || (_activeItem != 15 && (link->param1 != 1 || !cathHasItem(kItemKey) || (_activeItem != kItemBriefcase && _activeItem != kItemFirebird)))) {
					if (!cathRunningDialog("LIB013"))
						queueSFX(kCharacterCath, 13, 0);

					link->scene = kSceneNone;
					return;
				}

				queueSFX(kCharacterCath, 32, 0);

				if ((link->param1 != 0 && link->param1 <= 3) || (link->param1 >= 32 && link->param1 <= 37))
					setDoor(link->param1, kCharacterCath, 0, 10, 9);

				queueSFX(kCharacterCath, 15, 22);
				_activeItem = 0;
				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
			} else {
				if (link->action != kActionRattle || _activeItem != kItemKey) {
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
				_activeItem = 0;
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

			if (_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] && !dialogRunning("MUS003")) {
				playDialog(kCharacterCath, "MUS003", 16, 0);
				_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] = 0;
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
			if (_globals[kGlobalChapter] == 5) {
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

		if (_doneNIS[kEventKronosBringFirebird]) {
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

				_globals[kGlobalEggIsOpen] = 1;
				break;
			}
			case 2:
				if (!inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
					nisId = kEventCathCloseEggNoBackground;
				} else {
					nisId = kEventCathCloseEgg;
				}

				_globals[kGlobalEggIsOpen] = 0;
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

		if (_items[kItemMatch].floating && !_items[kItemMatch].haveIt) {
			_items[kItemMatch].haveIt = 1;
			_items[kItemMatch].floating = 0;
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
		if (_globals[kGlobalCanPlayEggSuitcaseMusic] && !dialogRunning("MUS003")) {
			playDialog(kCharacterCath, "MUS003", 16, 0);
			_globals[kGlobalCanPlayEggSuitcaseMusic] = 0;
		}

		break;
	case kActionFindEggUnderSink:
		if (_engine->isDemo())
			break;

		queueSFX(kCharacterCath, 24, 0);
		if (_globals[kGlobalCanPlayEggUnderSinkMusic] && !dialogRunning("MUS003")) {
			playDialog(kCharacterCath, "MUS003", 16, 0);
			_globals[kGlobalCanPlayEggUnderSinkMusic] = 0;
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
			if (_doors[link->param1].who)
				send(kCharacterCath, _doors[link->param1].who, 8, link->param1);
		}

		return;
	case kActionPlayMusicChapter:
		switch (_globals[kGlobalChapter]) {
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

		switch (_globals[kGlobalChapter]) {
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
	case kActionEasterEgg:
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
	if (!_globals[kGlobalJacket])
		_globals[kGlobalJacket] = 1;

	if (_items[kItemCorpse].floating == 1) {
		if (bedPosition == 4) {
			if (_globals[kGlobalJacket])
				playNIS(kEventCorpsePickFloorOpenedBedOriginal);

			_items[kItemCorpse].floating = 5;
		} else if (_globals[kGlobalJacket] == 2) {
			playNIS(kEventCorpsePickFloorGreen);
		} else {
			playNIS(kEventCorpsePickFloorOriginal);
		}
	} else if (_items[kItemCorpse].floating == 2) {
		if (_globals[kGlobalJacket] == 2) {
			playNIS(kEventCorpsePickBedGreen);
		} else {
			playNIS(kEventCorpsePickBedOriginal);
		}
	}

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::dropTyler(bool doCleanNIS) {
	switch (_items[kItemCorpse].floating) {
	case 1:
		if (_globals[kGlobalJacket] == 2) {
			playNIS(kEventCorpseDropFloorGreen);
		} else {
			playNIS(kEventCorpseDropFloorOriginal);
		}

		break;
	case 2:
		if (_globals[kGlobalJacket] == 2) {
			playNIS(kEventCorpseDropBedGreen);
		} else {
			playNIS(kEventCorpseDropBedOriginal);
		}

		break;
	case 4:
		_items[kItemCorpse].floating = 0;
		_globals[kGlobalCorpseHasBeenThrown] = 1;

		if (_gameTime <= 1138500) {
			if (_globals[kGlobalJacket] == 2) {
				playNIS(kEventCorpseDropWindowGreen);
			} else {
				playNIS(kEventCorpseDropWindowOriginal);
			}

			_globals[kGlobalFrancoisHasSeenCorpseThrown] = 1;
		} else {
			playNIS(kEventCorpseDropBridge);
		}

		_globals[kGlobalCorpseMovedFromFloor] = 1;

		break;
	}

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::takeJacket(bool doCleanNIS) {
	_globals[kGlobalJacket] = 2;
	_items[kItemMatchBox].haveIt = 1;
	_items[kItemMatchBox].floating = 0;
	setDoor(9, kCharacterCath, 2, 255, 255);
	playNIS(kEventPickGreenJacket);
	_globals[kGlobalCathIcon] = 34;
	_engine->getGraphicsManager()->drawItemDim(_globals[kGlobalCathIcon], 0, 0, 1);
	_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::takeScarf(bool doCleanNIS) {
	if (_globals[kGlobalJacket] == 2) {
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
		if (_doneNIS[kEventAnnaDialogGoToJerusalem]) {
			return "XANN12";
		} else if (_doneNIS[kEventLocomotiveRestartTrain]) {
			return "XANN11";
		} else if (_doneNIS[kEventAnnaBaggageTies] ||
				   _doneNIS[kEventAnnaBaggageTies2] ||
				   _doneNIS[kEventAnnaBaggageTies3] ||
				   _doneNIS[kEventAnnaBaggageTies4]) {
			return "XANN10";
		} else if (_doneNIS[kEventAnnaTired] ||
				   _doneNIS[kEventAnnaTiredKiss]) {
			return "XANN9";
		} else if (_doneNIS[kEventAnnaBaggageArgument]) {
			return "XANN8";
		} else if (_doneNIS[kEventKronosVisit]) {
			return "XANN7";
		} else if (_doneNIS[kEventAbbotIntroduction]) {
			return "XANN6A";
		} else if (_doneNIS[kEventVassiliSeizure]) {
			return "XANN6";
		} else if (_doneNIS[kEventAugustPresentAnna] ||
				   _doneNIS[kEventAugustPresentAnnaFirstIntroduction]) {
			return "XANN5";
		} else if (_globals[kGlobalOverheardAugustInterruptingAnnaAtDinner]) {
			return "XANN4";
		} else if (_doneNIS[kEventAnnaGiveScarf] ||
				   _doneNIS[kEventAnnaGiveScarfDiner] ||
				   _doneNIS[kEventAnnaGiveScarfSalon] ||
				   _doneNIS[kEventAnnaGiveScarfMonogram] ||
				   _doneNIS[kEventAnnaGiveScarfDinerMonogram] ||
				   _doneNIS[kEventAnnaGiveScarfSalonMonogram]) {
			return "XANN3";
		} else if (_doneNIS[kEventDinerMindJoin]) {
			return "XANN2";
		} else if (_doneNIS[kEventGotALight] ||
				   _doneNIS[kEventGotALightD]) {
			return "XANN1";
		}

		break;
	case kCharacterAugust:
		if (_doneNIS[kEventAugustTalkCigar]) {
			return "XAUG6";
		} else if (_doneNIS[kEventAugustBringBriefcase]) {
			return "XAUG5";
		} else if (_doneNIS[kEventAugustMerchandise]) {
			if (_gameTime <= 2200500) {
				return "XAUG4";
			} else {
				return "XAUG4A";
			}
		} else if (_doneNIS[kEventDinerAugust] ||
				   _doneNIS[kEventDinerAugustAlexeiBackground] ||
				   _doneNIS[kEventMeetAugustTylerCompartment] ||
				   _doneNIS[kEventMeetAugustHisCompartment] ||
				   _doneNIS[kEventMeetAugustTylerCompartmentBed] ||
				   _doneNIS[kEventMeetAugustHisCompartmentBed]) {
			return "XAUG3";
		} else if (_doneNIS[kEventAugustPresentAnnaFirstIntroduction]) {
			return "XAUG2";
		} else if (_globals[kGlobalKnowAboutAugust]) {
			return "XAUG1";
		}

		break;
	case kCharacterTatiana:
		if (_doneNIS[kEventTatianaTylerCompartment]) {
			return "XTAT6";
		} else if (_doneNIS[kEventTatianaCompartmentStealEgg]) {
			return "XTAT5";
		} else if (_doneNIS[kEventTatianaGivePoem]) {
			return "XTAT3";
		} else if (_globals[kGlobalMetTatianaAndVassili]) {
			return "XTAT1";
		}

		break;
	case kCharacterVassili:
		if (_doneNIS[kEventCathFreePassengers]) {
			return "XVAS4";
		} else if (_doneNIS[kEventVassiliCompartmentStealEgg]) {
			return "XVAS3";
		} else if (_doneNIS[kEventAbbotIntroduction]) {
			return "XVAS2";
		} else if (_doneNIS[kEventVassiliSeizure]) {
			return "XVAS1A";
		} else if (_globals[kGlobalMetTatianaAndVassili]) {
			return "XVAS1";
		}

		break;
	case kCharacterAlexei:
		if (_globals[kGlobalOverheardAlexeiTellingTatianaAboutBomb]) {
			return "XALX6";
		} else if (_globals[kGlobalOverheardAlexeiTellingTatianaAboutWantingToKillVassili]) {
			return "XALX5";
		} else if (_globals[kGlobalOverheardTatianaAndAlexeiPlayingChess]) {
			return "XALX4A";
		} else if (_globals[kGlobalOverheardTatianaAndAlexeiAtBreakfast]) {
			return "XALX4";
		} else if (_doneNIS[kEventAlexeiSalonPoem]) {
			return "XALX3";
		} else if (_doneNIS[kEventAlexeiSalonVassili]) {
			return "XALX2";
		} else if (_doneNIS[kEventAlexeiDiner] ||
				   _doneNIS[kEventAlexeiDinerOriginalJacket]) {
			return "XALX1";
		}

		break;
	case kCharacterAbbot:
		if (_doneNIS[kEventAbbotDrinkDefuse]) {
			return "XABB4";
		} else if (_doneNIS[kEventAbbotInvitationDrink] ||
				   _doneNIS[kEventDefuseBomb]) {
			return "XABB3";
		} else if (_doneNIS[kEventAbbotWrongCompartment] ||
				   _doneNIS[kEventAbbotWrongCompartmentBed]) {
			return "XABB2";
		} else if (_doneNIS[kEventAbbotIntroduction]) {
			return "XABB1";
		}

		break;
	case kCharacterMilos:
		if (_doneNIS[kEventLocomotiveMilosDay] || _doneNIS[kEventLocomotiveMilosNight]) {
			return "XMIL5";
		} else if (_doneNIS[kEventMilosCompartmentVisitTyler] &&
				  (_globals[kGlobalChapter] == 3 ||
				   _globals[kGlobalChapter] == 4)) {
			return "XMIL4";
		} else if (_doneNIS[kEventMilosCorridorThanks] ||
				   _globals[kGlobalChapter] == 5) {
			return "XMIL3";
		} else if (_doneNIS[kEventMilosCompartmentVisitAugust]) {
			return "XMIL2";
		} else if (_doneNIS[kEventMilosTylerCompartmentDefeat]) {
			return "XMIL1";
		}

		break;
	case kCharacterVesna:
		if (_globals[kGlobalOverheardMilosAndVesnaConspiring]) {
			return "XVES2";
		} else if (_globals[kGlobalOverheardVesnaAndMilosDebatingAboutCath]) {
			return "XVES1";
		}

		break;
	case kCharacterKronos:
		if (_doneNIS[kEventKronosReturnBriefcase])
			return "XKRO6";
		if (_doneNIS[kEventKronosBringEggCeiling] ||
			_doneNIS[kEventKronosBringEgg]) {
			return "XKRO5";
		} else {
			if (_doneNIS[kEventKronosConversation] ||
				_doneNIS[kEventKronosConversationFirebird]) {
				if (_items[kItemFirebird].floating != 6 &&
					_items[kItemFirebird].floating != 5 &&
					_items[kItemFirebird].floating != 2 &&
					_items[kItemFirebird].floating != 1)
					return "XKRO4A";
			}

			if (_doneNIS[kEventKronosConversationFirebird])
				return "XKRO4";

			if (_doneNIS[kEventMilosCompartmentVisitAugust]) {
				if (_doneNIS[kEventKronosConversation])
					return "XKRO3";
			} else if (_doneNIS[kEventKronosConversation]) {
				return "XKRO2";
			}

			if (_globals[kGlobalKnowAboutKronos]) {
				return "XKRO1";
			}
		}

		break;
	case kCharacterFrancois:
		if (_globals[kGlobalFrancoisSawABlackBeetle]) {
			return "XFRA3";
		} else if (_globals[kGlobalOverheardMadameAndFrancoisTalkingAboutWhistle] ||
				   _doneNIS[kEventFrancoisWhistle] ||
				   _doneNIS[kEventFrancoisWhistleD] ||
				   _doneNIS[kEventFrancoisWhistleNight] ||
				   _doneNIS[kEventFrancoisWhistleNightD]) {
			return "XFRA2";
		} else if (_gameTime <= 1075500) {
			return "XFRA1";
		}

		break;
	case kCharacterMadame:
		if (_globals[kGlobalMadameDemandedMaxInBaggage]) {
			return "XMME4";
		} else if (_globals[kGlobalMadameComplainedAboutMax]) {
			return "XMME3";
		} else if (_globals[kGlobalOverheardMadameAndFrancoisTalkingAboutWhistle]) {
			return "XMME2";
		} else  if (_globals[kGlobalMetMadame]) {
			return "XMME1";
		}

		break;
	case kCharacterMonsieur:
		if (_globals[kGlobalMetMonsieur]) {
			return "XMRB1";
		}

		break;
	case kCharacterRebecca:
		if (_globals[kGlobalOverheardSophieTalkingAboutCath]) {
			return "XREB1A";
		} else if (_globals[kGlobalMetSophieAndRebecca]) {
			return "XREB1";
		}

		break;
	case kCharacterSophie:
		if (_globals[kGlobalKnowAboutRebeccaDiary]) {
			return "XSOP2";
		} else if (_globals[kGlobalKnowAboutRebeccaAndSophieRelationship]) {
			return "XSOP1B";
		} else if (_globals[kGlobalOverheardSophieTalkingAboutCath]) {
			return "XSOP1A";
		} else if (!_globals[kGlobalMetSophieAndRebecca]) {
			return "XSOP1";
		}

		break;
	case kCharacterMahmud:
		if (_globals[kGlobalMetMahmud]) {
			return "XMAH1";
		}

		break;
	case kCharacterYasmin:
		if (_globals[kGlobalMetYasmin]) {
			return "XHAR2";
		}

		break;
	case kCharacterHadija:
		if (_globals[kGlobalMetHadija]) {
			return "XHAR1";
		}

		break;
	case kCharacterAlouan:
		if (_globals[kGlobalMetAlouan]) {
			return "XHAR3";
		}

		break;
	case kCharacterPolice:
		if (_globals[kGlobalMetFatima]) {
			return "XHAR4";
		}

		break;
	case kCharacterMaster:
		if (_doneNIS[kEventCathDream] || _doneNIS[kEventCathWakingUp]) {
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
