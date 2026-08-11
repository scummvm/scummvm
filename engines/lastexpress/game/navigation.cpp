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


#include "lastexpress/data/archive.h"
#include "lastexpress/game/logic.h"

#include "lastexpress/lastexpress.h"

#include "common/memstream.h"

namespace LastExpress {

void LogicManager::loadTrain(int cd) {
	const char *datFilename;

	if (cd > 3 || (_trainIndex && _trainIndex != cd)) {
		switch (cd) {
		case 1:
			datFilename = "CD1TRAIN.DAT";
			break;
		case 2:
			datFilename = "CD2TRAIN.DAT";
			break;
		case 3:
			datFilename = "CD3TRAIN.DAT";
			break;
		default:
			datFilename = "TRAIN.DAT";
			break;
		}

		_numberOfScenes = 0;

		HPF *archive = _engine->getArchiveManager()->openHPF(datFilename);
		if (archive) {
			if ((archive->size & 0x1FFFFF) > 270) {
				error("Could not open \"%s\".  Please reinstall.", datFilename);
			}

			// Originally the game directly uses the _trainData struct as the
			// read buffer for the archive routines, and then performs pointer
			// fix-ups on it. Since this is not portable we use a more reliable
			// system.

			byte *trainDataRaw = (byte *)malloc(MEM_PAGE_SIZE * archive->size);
			if (trainDataRaw) {
				_engine->getArchiveManager()->readHPF(archive, trainDataRaw, archive->size);
				_engine->getArchiveManager()->closeHPF(archive);

				Common::SeekableReadStream *trainDataStream = new Common::MemoryReadStream(trainDataRaw, MEM_PAGE_SIZE * archive->size, DisposeAfterUse::YES);

				// First node
				trainDataStream->read(_trainData[0].sceneFilename, sizeof(_trainData[0].sceneFilename));
				_trainData[0].nodePosition.position = trainDataStream->readUint16LE();
				_trainData[0].nodePosition.location = trainDataStream->readUint16LE();
				_trainData[0].nodePosition.car = trainDataStream->readUint16LE();
				_trainData[0].cathDir = (uint8)trainDataStream->readByte();
				_trainData[0].property = (uint8)trainDataStream->readByte();
				_trainData[0].parameter1 = (uint8)trainDataStream->readByte();
				_trainData[0].parameter2 = (uint8)trainDataStream->readByte();
				_trainData[0].parameter3 = (uint8)trainDataStream->readByte();

				// The first link offset is going to be zero
				trainDataStream->readUint32LE();
				_trainData[0].link = nullptr; 

				_numberOfScenes = _trainData[0].nodePosition.position;

				for (int i = 1; i < _numberOfScenes; i++) {
					trainDataStream->read(_trainData[i].sceneFilename, sizeof(_trainData[i].sceneFilename));
					_trainData[i].nodePosition.position = trainDataStream->readUint16LE();
					_trainData[i].nodePosition.location = trainDataStream->readUint16LE();
					_trainData[i].nodePosition.car = trainDataStream->readUint16LE();
					_trainData[i].cathDir = (uint8)trainDataStream->readByte();
					_trainData[i].property = (uint8)trainDataStream->readByte();
					_trainData[i].parameter1 = (uint8)trainDataStream->readByte();
					_trainData[i].parameter2 = (uint8)trainDataStream->readByte();
					_trainData[i].parameter3 = (uint8)trainDataStream->readByte();

					uint32 ptrLinkOffset = trainDataStream->readUint32LE();
					_trainData[i].link = nullptr;

					int64 curStreamNodePos = trainDataStream->pos();

					if (ptrLinkOffset) {
						trainDataStream->seek(ptrLinkOffset, SEEK_SET);

						// Process the entire link chain...
						Link **currentLinkPtr = &_trainData[i].link;

						while (true) {
							*currentLinkPtr = new Link();
							Link *curLink = *currentLinkPtr;

							curLink->left = trainDataStream->readUint16LE();
							curLink->right = trainDataStream->readUint16LE();
							curLink->top = trainDataStream->readUint16LE();
							curLink->bottom = trainDataStream->readUint16LE();

							uint32 ptrLineOffset = trainDataStream->readUint32LE();
							curLink->lineList = nullptr;

							curLink->scene = trainDataStream->readUint16LE();
							curLink->location = (uint8)trainDataStream->readByte();
							curLink->action = (uint8)trainDataStream->readByte();
							curLink->param1 = (uint8)trainDataStream->readByte();
							curLink->param2 = (uint8)trainDataStream->readByte();
							curLink->param3 = (uint8)trainDataStream->readByte();
							curLink->cursor = (uint8)trainDataStream->readByte();

							uint32 ptrNextLinkOffset = trainDataStream->readUint32LE();

							if (ptrLineOffset) {
								int64 currentStreamLinkPos = trainDataStream->pos();
								trainDataStream->seek(ptrLineOffset, SEEK_SET);

								// Process the line chain...
								Line7 **currentLinePtr = &(curLink->lineList);

								while (true) {
									*currentLinePtr = new Line7();
									Line7 *curLine = *currentLinePtr;

									curLine->slope = trainDataStream->readSint32LE();
									curLine->intercept = trainDataStream->readSint32LE();
									curLine->lineType = (char)trainDataStream->readByte();

									uint32 ptrNextLineOffset = trainDataStream->readUint32LE();

									if (ptrNextLineOffset) {
										trainDataStream->seek(ptrNextLineOffset, SEEK_SET);
										currentLinePtr = &(curLine->next);
									} else {
										// End of line chain!
										curLine->next = nullptr;
										break;
									}
								}

								// Return to previous link position...
								trainDataStream->seek(currentStreamLinkPos, SEEK_SET);
							}

							if (ptrNextLinkOffset) {
								trainDataStream->seek(ptrNextLinkOffset, SEEK_SET);
								currentLinkPtr = &(curLink->next);
							} else {
								// End of link chain!
								curLink->next = nullptr;
								break;
							}
						}
					}

					// Return to previous node position...
					trainDataStream->seek(curStreamNodePos, SEEK_SET);
				}

				delete trainDataStream;

				_trainIndex = cd;
			} else {
				error("Out of memory");
			}
		} else {
			error("Could not open \"%s\".  Please reinstall.", datFilename);
		}
	}
}

void LogicManager::nodeStepTimer(Event *event) {
	int32 ticks = 1;
	if (_actionJustPerformed)
		ticks = 10;

	_engine->setEventTickInternal(false);
	_actionJustPerformed = false;

	if (_engine->_gracePeriodTimer && !_engine->getGraphicsManager()->_shouldDrawEggOrHourglass) {
		if ((_globals[kGlobalJacket] < 2 ? 225 : 450) == _engine->_gracePeriodTimer || _engine->_gracePeriodTimer == 900) {
			_navigationItemBrightness = 0;
			_navigationItemBrighnessStep = 1;
		}

		if (!_engine->_lockGracePeriod) { // Gets set to true only by the debugger only...
			if (_engine->_gracePeriodTimer <= ticks) {
				_engine->_gracePeriodTimer = 0;
			} else {
				_engine->_gracePeriodTimer -= ticks;
			}
		}

		bool shouldDraw = false;

		if (_doubleClickFlag) {
			shouldDraw = true;
		} else if (_engine->_gracePeriodTimer <= 500) {
			if (!(_engine->_gracePeriodTimer % ((_engine->_gracePeriodTimer + 100) / 100))) {
				shouldDraw = true;
			}
		} else if (!(_engine->_gracePeriodTimer % 5)) {
			shouldDraw = true;
		}

		if (shouldDraw) {
			if (_navigationItemBrightness) {
				_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, _navigationItemBrightness);
			} else {
				_engine->getGraphicsManager()->drawItem(_engine->_currentGameFileColorId + 39, 608, 448);
			}

			_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
			_navigationItemBrightness += _navigationItemBrighnessStep;
			if (!_navigationItemBrightness || _navigationItemBrightness == 3)
				_navigationItemBrighnessStep = -_navigationItemBrighnessStep;
		}

		if (_engine->_gracePeriodTimer < 90) {
			if (ticks + _engine->_gracePeriodTimer >= 90)
				_engine->getSoundManager()->playSoundFile("TIMER.SND", kSoundTypeMenu | kVolumeFull, 0, 0);

			if (_engine->_gracePeriodTimer < 90 && !dialogRunning("TIMER"))
				_engine->_gracePeriodTimer = 0;
		}

		if (!_engine->_gracePeriodTimer) {
			if (_engine->_cursorX < 608 || _engine->_cursorY < 448 || _engine->_cursorX >= 640 || _engine->_cursorY >= 480) {
				_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, 1);
			} else {
				_engine->getGraphicsManager()->drawItem(_engine->_currentGameFileColorId + 39, 608, 448);
			}

			_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
			_engine->getVCR()->writeSavePoint(4, 31, 0);
		}
	}

	if (_engine->_navigationEngineIsRunning) {
		_realTime += ticks;
		_gameTime += ticks * _timeSpeed;

		if (_timeSpeed) {
			if (_engine->isDemo()) {
				if (!_eventTicksSinceLastDemoSavegame--) {
					_eventTicksSinceLastDemoSavegame = 150;
					_engine->getVCR()->writeSavePoint(4, 31, 0);
				}

				if ((_realTime - _lastSavegameSessionTicks) > 450)
					_engine->getVCR()->writeSavePoint(5, 31, 0);
			} else {
				if (!_eventTicksUntilNextSavePoint--) {
					_eventTicksUntilNextSavePoint = 450;
					_engine->getVCR()->writeSavePoint(4, 31, 0);
				}

				if ((_realTime - _lastSavegameSessionTicks) > 2700)
					_engine->getVCR()->writeSavePoint(5, 31, 0);
			}
		}
	}

	if (_doubleClickFlag && !_engine->mouseHasLeftClicked() && !_engine->mouseHasRightClicked()) {
		if (_engine->getOtisManager()->walkingRender() && (!_blockedViews[100 * _trainData[_activeNode].nodePosition.car + _trainData[_activeNode].cathDir])) {
			Link tmp;

			tmp.copyFrom(_trainData[_activeNode].link);
			doAction(&tmp);

			if (tmp.scene) {
				_engine->getGraphicsManager()->stepBG(tmp.scene);
			} else {
				_doubleClickFlag = false;
				_engine->getGraphicsManager()->setMouseDrawable(true);
				mouseStatus();
			}

			if (_engine->_navigationEngineIsRunning)
				_engine->getMessageManager()->flushTime();
		} else {
			_doubleClickFlag = false;
			_engine->getGraphicsManager()->setMouseDrawable(true);
			mouseStatus();
		}

		return;
	}

	if (_engine->_navigationEngineIsRunning) {
		if (!_engine->isDemo() && _engine->_beetle)
			_engine->_beetle->tick();

		_engine->getMessageManager()->flushTime();
		_engine->getOtisManager()->updateAll();
		_engine->getSpriteManager()->drawCycle();

		int foundCharacter = _engine->getOtisManager()->checkMouse(_engine->_cursorX, _engine->_cursorY);
		int oldCursor = _engine->_cursorType;

		if (foundCharacter && cathHasItem(getCharacter(foundCharacter).inventoryItem & 0x7F)) {
			_engine->_cursorType = _items[getCharacter(foundCharacter).inventoryItem & 0x7F].mnum;
		} else if (!foundCharacter || (getCharacter(foundCharacter).inventoryItem & 0x80) == 0) {
			if (_engine->_cursorType >= 15) {
				getNewMnum();
			}
		} else {
			_engine->_cursorType = 15;
		}

		if (_engine->_cursorType != oldCursor) {
			_engine->getGraphicsManager()->burstMouseArea();
		}
	}
}

void LogicManager::mouseStatus() {
	getNewMnum();
	_engine->getGraphicsManager()->burstMouseArea();
}

void LogicManager::getNewMnum() {
	int character;
	uint16 location;

	bool found = false;

	if (_activeItem != kItemWhistle ||
		_globals[kGlobalEggIsOpen] ||
		checkCathDir(kCarGreenSleeping, 59) ||
		checkCathDir(kCarGreenSleeping, 76) ||
		_inventoryFlag1 ||
		_inventoryFlag2 ||
		_isEggHighlighted ||
		_isMagnifierInUse) {

		if (_activeItem != kItemMatch ||
			(!cathInCorridor(kCarGreenSleeping) &&
			!cathInCorridor(kCarRedSleeping)) ||
			_globals[kGlobalJacket] != 2 ||
			_inventoryFlag1 ||
			_inventoryFlag2 ||
			_isEggHighlighted ||
			_isMagnifierInUse ||
			(_items[kItem2].floating &&
			getCharacter(kCharacterCath).characterPosition.car == kCarRedSleeping &&
			getCharacter(kCharacterCath).characterPosition.position == 2300)) {

			character = _engine->getOtisManager()->checkMouse(_engine->_cursorX, _engine->_cursorY);

			if (character && !_inventoryFlag1 && !_inventoryFlag2 && !_isEggHighlighted && !_isMagnifierInUse) {
				if (cathHasItem(getCharacter(character).inventoryItem & 0x7F)) {
					found = true;
					_engine->_cursorType = _items[getCharacter(character).inventoryItem & 0x7F].mnum;
				} else if ((getCharacter(character).inventoryItem & 0x80) != 0) {
					found = true;
					_engine->_cursorType = 15;
				}
			}

			if (!found && !_inventoryFlag1 && !_inventoryFlag2 && !_isEggHighlighted && !_isMagnifierInUse) {
				location = 0;
				Link *selectedLink = nullptr;

				for (Link *i = _trainData[_activeNode].link; i; i = i->next) {
					if (pointIn(_engine->_cursorX, _engine->_cursorY, i) && i->location >= location) {
						if (findCursor(i)) {
							if (!_blockedViews[100 * _trainData[i->scene].nodePosition.car + _trainData[i->scene].cathDir] || i->cursor == 3 || i->cursor == 4) {
								location = i->location;
								selectedLink = i;
							}
						}
					}
				}

				if (selectedLink) {
					_engine->_cursorType = findCursor(selectedLink);
				} else {
					_engine->_cursorType = kCursorNormal;
				}
			}
		} else {
			_engine->_cursorType = _items[kItemMatch].mnum;
		}
	} else {
		_engine->_cursorType = _items[kItemWhistle].mnum;
	}

	if (_isMagnifierInUse)
		_engine->_cursorType = kCursorMagnifier;

	if (_inventoryFlag1 || _inventoryFlag2 || _isEggHighlighted)
		_engine->_cursorType = kCursorNormal;
}

void LogicManager::nodeStepMouse(Event *event) {
	Link actionLink;
	int foundCharacter;
	int oldCursorX = _engine->_cursorX;
	int oldCursorY = _engine->_cursorY;
	int oldCursorType = _engine->_cursorType;
	bool flag = true;

	actionLink.left = 0;
	_engine->mouseSetLeftClicked(false);
	_engine->mouseSetRightClicked(false);

	if ((event->flags & kMouseFlagLeftDown) != 0) {
		if ((event->flags & kMouseFlagDoubleClick) != 0 && _engine->_fastWalkJustDeactivated)
			event->flags &= ~kMouseFlagDoubleClick;

		_engine->_fastWalkJustDeactivated = false;
	}

	if (_doubleClickFlag) {
		if ((event->flags & (kMouseFlagRightDown | kMouseFlagLeftDown)) != 0) {
			_doubleClickFlag = false;
			_engine->getGraphicsManager()->setMouseDrawable(true);
			mouseStatus();
			_engine->_fastWalkJustDeactivated = true;
		}

		return;
	}

	if ((event->flags & kMouseFlagDoubleClick) != 0 && _engine->getOtisManager()->walkingRender() && _engine->_cursorType == kCursorForward) {
		_engine->getGraphicsManager()->setMouseDrawable(false);
		_engine->getGraphicsManager()->burstMouseArea();
		_doubleClickFlag = true;
		return;
	}

	_engine->_cursorX = event->x;
	_engine->_cursorY = event->y;
	checkInventory(event->flags);

	if (!_engine->getMenu()->isShowingMenu()) {
		if (_activeItem != kItemWhistle ||
			_globals[kGlobalEggIsOpen] ||
			checkCathDir(kCarGreenSleeping, 59) ||
			checkCathDir(kCarGreenSleeping, 76) ||
			_inventoryFlag1 ||
			_inventoryFlag2 ||
			_isEggHighlighted ||
			_isMagnifierInUse) {

			if (_activeItem != kItemMatch ||
				(!cathInCorridor(kCarGreenSleeping) &&
				!cathInCorridor(kCarRedSleeping)) ||
				_globals[kGlobalJacket] != 2 ||
				_inventoryFlag1 ||
				_inventoryFlag2 ||
				_isEggHighlighted ||
				_isMagnifierInUse ||
				(_items[kItem2].floating &&
					getCharacter(kCharacterCath).characterPosition.car == kCarRedSleeping &&
					getCharacter(kCharacterCath).characterPosition.position == 2300)) {

				foundCharacter = _engine->getOtisManager()->checkMouse(_engine->_cursorX, _engine->_cursorY);

				if (foundCharacter && !_inventoryFlag1 && !_inventoryFlag2 && !_isEggHighlighted && !_isMagnifierInUse) {
					if (cathHasItem(getCharacter(foundCharacter).inventoryItem & 0x7F)) {
						actionLink.left = 1;
						_engine->_cursorType = _items[getCharacter(foundCharacter).inventoryItem & 0x7F].mnum;

						if ((event->flags & kMouseFlagLeftDown) != 0)
							send(kCharacterCath, foundCharacter, 1, getCharacter(foundCharacter).inventoryItem & 0x7F);
					} else if ((getCharacter(foundCharacter).inventoryItem & 0x80) != 0) {
						actionLink.left = 1;
						_engine->_cursorType = 15;

						if ((event->flags & kMouseFlagLeftDown) != 0)
							send(kCharacterCath, foundCharacter, 1, 0);
					}
				}

				if (!actionLink.left && !_inventoryFlag1 && !_inventoryFlag2 && !_isEggHighlighted) {
					if (_isMagnifierInUse) {
						_engine->_cursorType = 11;

						if (_inventoryFlag1 || _inventoryFlag2 || _isEggHighlighted)
							_engine->_cursorType = 0;

						if (!_doubleClickFlag && flag && (oldCursorX != _engine->_cursorX || _engine->_cursorY != oldCursorY || _engine->_cursorType != oldCursorType || !_engine->getGraphicsManager()->canDrawMouse())) {
							_engine->getGraphicsManager()->setMouseDrawable(false);
							_engine->getGraphicsManager()->burstMouseArea(false); // The original updated the screen, we don't to avoid flickering...
							_engine->getGraphicsManager()->setMouseDrawable(true);
							_engine->getGraphicsManager()->newMouseLoc();
							_engine->getGraphicsManager()->burstMouseArea();
						}

						return;
					}

					uint8 location = 0;
					Link *foundLink = nullptr;

					for (Link *i = _trainData[_activeNode].link; i; i = i->next) {
						if (pointIn(_engine->_cursorX, _engine->_cursorY, i) && i->location >= location) {
							if (findCursor(i)) {
								if (!_blockedViews[100 * _trainData[i->scene].nodePosition.car + _trainData[i->scene].cathDir] || i->cursor == 3 || i->cursor == 4) {
									location = i->location;
									foundLink = i;
								}
							}
						}
					}

					if (foundLink) {
						_engine->_cursorType = findCursor(foundLink);

						actionLink.copyFrom(foundLink);

						if ((event->flags & kMouseFlagLeftDown) != 0 && !_actionJustPerformed) {
							_actionJustPerformed = true;
							doAction(&actionLink);

							if (actionLink.scene) {
								_engine->getGraphicsManager()->setMouseDrawable(false);
								_engine->getGraphicsManager()->burstMouseArea();
								_engine->getGraphicsManager()->stepBG(actionLink.scene);

								if (_engine->getGraphicsManager()->_shouldDrawEggOrHourglass)
									restoreEggIcon();

								_engine->getGraphicsManager()->setMouseDrawable(true);
								mouseStatus();
								flag = false;
							}

							if (!_engine->isDemo() && actionLink.action == kActionSwitchChapter &&
								actionLink.param1 == _globals[kGlobalChapter] &&
								(event->flags & kMouseFlagRightButton) != 0) {
								doF4();
								return;
							}
						}
					} else {
						_engine->_cursorType = 0;
					}
				}
			} else {
				_engine->_cursorType = _items[kItemMatch].mnum;

				if ((event->flags & kMouseFlagLeftDown) != 0) {
					if (isNight()) {
						playNIS(kEventCathSmokeNight);
					} else {
						playNIS(kEventCathSmokeDay);
					}

					if (!_closeUp) {
						_activeItem = 0;
						if (_engine->getGraphicsManager()->acquireSurface()) {
							_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
							_engine->getGraphicsManager()->unlockSurface();
						}

						_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
					}

					cleanNIS();
				}
			}
		} else {
			_engine->_cursorType = _items[kItemWhistle].mnum;
			if ((event->flags & kMouseFlagLeftDown) != 0 && !dialogRunning("LIB045")) {
				queueSFX(kCharacterCath, 45, 0);

				if (checkCathDir(kCarGreenSleeping, 26) ||
					checkCathDir(kCarGreenSleeping, 25) ||
					checkCathDir(kCarGreenSleeping, 23)) {
					send(kCharacterCath, kCharacterCond1, 226078300, 0);
				} else if (checkCathDir(kCarRedSleeping, 26) ||
						   checkCathDir(kCarRedSleeping, 25) ||
						   checkCathDir(kCarRedSleeping, 23)) {
					send(kCharacterCath, kCharacterCond2, 226078300, 0);
				}

				if (!_closeUp) {
					_activeItem = 0;
					if (_engine->getGraphicsManager()->acquireSurface()) {
						_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
						_engine->getGraphicsManager()->unlockSurface();
					}

					_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
				}
			}
		}

		if (_isMagnifierInUse) {
			_engine->_cursorType = 11;
		}

		if (_inventoryFlag1 || _inventoryFlag2 || _isEggHighlighted) {
			_engine->_cursorType = 0;
		}

		if (!_doubleClickFlag && flag && (oldCursorX != _engine->_cursorX || _engine->_cursorY != oldCursorY || _engine->_cursorType != oldCursorType || !_engine->getGraphicsManager()->canDrawMouse())) {
			_engine->getGraphicsManager()->setMouseDrawable(false);
			_engine->getGraphicsManager()->burstMouseArea(false); // The original updated the screen, we don't to avoid flickering...
			_engine->getGraphicsManager()->setMouseDrawable(true);
			_engine->getGraphicsManager()->newMouseLoc();
			_engine->getGraphicsManager()->burstMouseArea();
		}
	}
}

void LogicManager::doF4() {
	_engine->getSoundManager()->killAllSlots();

	switch (_globals[kGlobalChapter]) {
	case 1:
		giveCathItem(kItemParchemin);
		giveCathItem(kItemMatchBox);
		forceJump(kCharacterMaster, &LogicManager::CONS_Master_StartPart2);
		break;
	case 2:
		giveCathItem(kItemScarf);
		forceJump(kCharacterMaster, &LogicManager::CONS_Master_StartPart3);
		break;
	case 3:
		_items[kItemFirebird].floating = 4;
		_items[kItemFirebird].haveIt = 0;
		_items[kItem11].floating = 1;
		giveCathItem(kItemWhistle);
		giveCathItem(kItemKey);
		forceJump(kCharacterMaster, &LogicManager::CONS_Master_StartPart4);
		break;
	case 4:
		forceJump(kCharacterMaster, &LogicManager::CONS_Master_StartPart5);
		break;
	case 5:
		winGame();
		break;
	default:
		return;
	}
}

bool LogicManager::pointIn(int32 cursorX, int32 cursorY, Link *hotspot) {
	bool result = true;

	Line7 *lineList = hotspot->lineList;
	if (!lineList || hotspot->left > cursorX || hotspot->right < cursorX || hotspot->top > cursorY || hotspot->bottom < cursorY) {
		return false;
	}

	do {
		if (!result)
			break;

		result = lineList->lineType
					 ? lineList->intercept + cursorX * lineList->slope >= (-1000 * cursorY)
					 : lineList->intercept + cursorX * lineList->slope <= (-1000 * cursorY);

		lineList = lineList->next;
	} while (lineList);

	return result;
}

void LogicManager::checkInventory(int32 flags) {
	int sceneIdx;
	int selectedItemIdx = 0;

	_isMagnifierInUse = false;

	if (_engine->_cursorX <  608 || _engine->_cursorY <  448 ||
		_engine->_cursorX >= 640 || _engine->_cursorY >= 480) {
		if (_isEggHighlighted) {
			if (!_engine->_gracePeriodTimer) {
				_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, 1);
				_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
			}

			_isEggHighlighted = false;
		}

	} else {
		if (!_isEggHighlighted) {
			if (!_engine->_gracePeriodTimer) {
				_engine->getGraphicsManager()->drawItem(_engine->_currentGameFileColorId + 39, 608, 448);
				_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
			}

			_isEggHighlighted = true;
		}

		if ((flags & kMouseFlagLeftDown) != 0) {
			_isEggHighlighted = false;
			_inventoryFlag1 = false;
			_inventoryFlag2 = false;

			_engine->getSoundManager()->playSoundFile("LIB039.SND", kSoundTypeMenu | kVolumeFull, 0, 0);
			_engine->getMenu()->doEgg(true, 0, 0);
		} else if ((flags & kMouseFlagRightDown) != 0 && _engine->_gracePeriodTimer) {
			if (dialogRunning("TIMER"))
				endDialog("TIMER");

			_engine->_gracePeriodTimer = 900;
		}
	}

	if (_engine->_cursorX >= 32) {
		if (_inventoryFlag2) {
			if ((flags & kMouseFlagLeftButton) != 0) {
				if (_highlightedItem) {
					int count = 0;

					for (int i = 1; i < 32; i++) {
						if (_items[i].haveIt && _items[i].inPocket && count < 11) {
							count++;
							if (count == _highlightedItem) {
								selectedItemIdx = i;
								break;
							}
						}
					}

					if (selectedItemIdx) {
						_engine->getGraphicsManager()->drawItemDim(_items[selectedItemIdx].mnum, 0, 40 * _highlightedItem + 4, 1);
						_engine->getGraphicsManager()->burstBox(0, 40 * _highlightedItem + 4, 32, 32);
						_highlightedItem = 0;
					}
				}

				return;
			}

			_engine->getGraphicsManager()->drawItemDim(_globals[kGlobalCathIcon], 0, 0, 1);

			if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 0, 44, 32, 40 * _inventoryVerticalSlot);
				_engine->getGraphicsManager()->unlockSurface();
			}

			_engine->getGraphicsManager()->burstBox(0, 0, 32, 8 * (5 * _inventoryVerticalSlot + 5));
			_inventoryFlag2 = false;

			if (!_activeItem || _items[_activeItem].inPocket) {
				_activeItem = findLargeItem();
				if (_activeItem) {
					_engine->getGraphicsManager()->drawItem(_items[_activeItem].mnum, 44, 0);
				} else if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}

				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}

			if (!_closeUp)
				return;

			if (!_nodeReturn2) {
				if (!_doneNIS[kEventKronosBringFirebird] && !_globals[kGlobalEggIsOpen]) {
					_closeUp = 0;
					if (_blockedViews[100 * _trainData[_nodeReturn].nodePosition.car + _trainData[_nodeReturn].cathDir]) {
						bumpCathNode(getSmartBumpNode(_nodeReturn));
					} else {
						bumpCathNode(_nodeReturn);
					}
				}

				return;
			}
		} else {
			if (_inventoryFlag1) {
				_engine->getGraphicsManager()->drawItemDim(_globals[kGlobalCathIcon], 0, 0, 1);
				_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);
				_inventoryFlag1 = false;
			}

			if (!_activeItem ||
				!_items[_activeItem].closeUp ||
				_engine->_cursorX < 44 ||
				_engine->_cursorX >= 76 ||
				_engine->_cursorY >= 32) {
				return;
			}

			if (!_closeUp || (_nodeReturn2 && findLargeItem() == _activeItem)) {
				_isMagnifierInUse = true;
			}

			if ((flags & kMouseFlagLeftDown) == 0)
				return;

			if (!_closeUp) {
				_nodeReturn = _activeNode;
				_closeUp = 1;
				bumpCathNode(_items[_activeItem].closeUp);
				return;
			}

			if (!_nodeReturn2 || findLargeItem() != _activeItem)
				return;
		}

		sceneIdx = _nodeReturn2;
		_nodeReturn2 = 0;
		bumpCathNode(sceneIdx);
		return;
	}

	if (_engine->_cursorY >= 32) {
		selectedItemIdx = 0;
		if (!_inventoryFlag2) {
			if (_inventoryFlag1) {
				_engine->getGraphicsManager()->drawItemDim(_globals[kGlobalCathIcon], 0, 0, 1);
				_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);
				_inventoryFlag1 = false;
			}

			return;
		}

		if ((flags & kMouseFlagLeftButton) != 0) {
			int itemToHighlight = _engine->_cursorY / 40;
			if (_highlightedItem && itemToHighlight != _highlightedItem) {
				int count = 0;

				for (int i = 1; i < 32; i++) {
					if (_items[i].haveIt && _items[i].inPocket && count < 11) {
						count++;
						if (count == _highlightedItem) {
							selectedItemIdx = i;
							break;
						}
					}
				}

				if (selectedItemIdx) {
					_engine->getGraphicsManager()->drawItemDim(_items[selectedItemIdx].mnum, 0, 40 * _highlightedItem + 4, 1);
					_engine->getGraphicsManager()->burstBox(0, 40 * _highlightedItem + 4, 32, 32);
					_highlightedItem = 0;
				}
			}

			if (itemToHighlight && itemToHighlight <= _inventoryVerticalSlot && itemToHighlight != _highlightedItem) {
				int count = 0;

				for (int i = 1; i < 32; i++) {
					if (_items[i].haveIt && _items[i].inPocket && count < 11) {
						count++;
						if (count == itemToHighlight) {
							selectedItemIdx = i;
							break;
						}
					}
				}

				if (selectedItemIdx) {
					_engine->getGraphicsManager()->drawItem(_items[selectedItemIdx].mnum, 0, 40 * itemToHighlight + 4);
					_engine->getGraphicsManager()->burstBox(0, 40 * itemToHighlight + 4, 32, 32);
					_highlightedItem = itemToHighlight;
				}
			}

			return;
		}

		if (_highlightedItem) {
			int count = 0;

			for (int i = 1; i < 32; i++) {
				if (_items[i].haveIt && _items[i].inPocket && count < 11) {
					count++;
					if (count == _highlightedItem) {
						selectedItemIdx = i;
						break;
					}
				}
			}
		}

		_engine->getGraphicsManager()->drawItemDim(_globals[kGlobalCathIcon], 0, 0, 1);
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 0, 44, 32, 40 * _inventoryVerticalSlot);
			_engine->getGraphicsManager()->unlockSurface();
		}

		_engine->getGraphicsManager()->burstBox(0, 0, 32, 8 * (5 * _inventoryVerticalSlot + 5));
		_highlightedItem = 0;
		_inventoryVerticalSlot = 0;

		if (selectedItemIdx) {
			if (_items[selectedItemIdx].closeUp) {
				if (_closeUp) {
					if (findLargeItem() && !_nodeReturn2)
						_nodeReturn2 = _activeNode;
				} else {
					_closeUp = 1;
					_nodeReturn = _activeNode;
				}

				bumpCathNode(_items[selectedItemIdx].closeUp);
			}

			if (_items[selectedItemIdx].useable) {
				_activeItem = selectedItemIdx;
				_engine->getGraphicsManager()->drawItem(_items[selectedItemIdx].mnum, 44, 0);
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			} else if (!_activeItem || _items[_activeItem].inPocket) {
				_activeItem = findLargeItem();
				if (_activeItem) {
					_engine->getGraphicsManager()->drawItem(_items[_activeItem].mnum, 44, 0);
				} else if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}

				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}

			_inventoryFlag2 = false;
			return;
		}

		if (!_activeItem || _items[_activeItem].inPocket) {
			_activeItem = findLargeItem();
			if (_activeItem) {
				_engine->getGraphicsManager()->drawItem(_items[_activeItem].mnum, 44, 0);
			} else if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
				_engine->getGraphicsManager()->unlockSurface();
			}

			_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
		}

		if (!_closeUp) {
			_inventoryFlag2 = false;
			return;
		}

		if (_nodeReturn2) {
			sceneIdx = _nodeReturn2;
			_nodeReturn2 = 0;
		} else {
			if (_doneNIS[kEventKronosBringFirebird] || _globals[kGlobalEggIsOpen]) {
				_inventoryFlag2 = false;
				return;
			}

			_closeUp = 0;
			sceneIdx = _nodeReturn;
			if (_blockedViews[100 * _trainData[_nodeReturn].nodePosition.car + _trainData[_nodeReturn].cathDir]) {
				sceneIdx = getSmartBumpNode(_nodeReturn);
			}
		}

		bumpCathNode(sceneIdx);
		_inventoryFlag2 = false;
		return;
	}

	if (_globals[kGlobalCathInSpecialState] ||
		getCharacter(kCharacterCath).characterPosition.location == 2 ||
		_globals[kGlobalPhaseOfTheNight] == 4 ||
		(_activeItem && !_items[_activeItem].inPocket && !_closeUp)) {
		return;
	}

	if ((flags & kMouseFlagLeftDown) != 0) {
		_inventoryFlag1 = false;
		_inventoryFlag2 = true;
		_engine->getGraphicsManager()->drawItem(_globals[kGlobalCathIcon] + 1, 0, 0);
		_inventoryVerticalSlot = 0;

		for (int i = 1; i < 32; i++) {
			if (_items[i].haveIt && _items[i].inPocket && _inventoryVerticalSlot < 11) {
				_engine->getGraphicsManager()->drawItemDim(_items[i].mnum, 0, 40 * _inventoryVerticalSlot + 44, 1);
				_inventoryVerticalSlot++;
			}
		}

		_engine->getGraphicsManager()->burstBox(0, 0, 32, 8 * (5 * _inventoryVerticalSlot + 5));
		return;
	}

	if (!_inventoryFlag1 && !_inventoryFlag2) {
		_engine->getGraphicsManager()->drawItem(_globals[kGlobalCathIcon], 0, 0);
		_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);

		_inventoryFlag1 = true;
	} else if (_inventoryFlag2 && (flags & kMouseFlagLeftButton) == 0) {
		if (_inventoryFlag2) {
			_inventoryFlag2 = false;

			if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 0, 44, 32, 40 * _inventoryVerticalSlot);
				_engine->getGraphicsManager()->unlockSurface();
			}

			_engine->getGraphicsManager()->drawItem(_globals[kGlobalCathIcon], 0, 0);
			_engine->getGraphicsManager()->burstBox(0, 0, 32, 8 * (5 * _inventoryVerticalSlot + 5));
			_inventoryVerticalSlot = 0;

			if (!_activeItem || _items[_activeItem].inPocket) {
				_activeItem = findLargeItem();
				if (_activeItem) {
					_engine->getGraphicsManager()->drawItem(_items[_activeItem].mnum, 44, 0);
				} else if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}

			if (_closeUp) {
				if (_nodeReturn2) {
					sceneIdx = _nodeReturn2;
					_nodeReturn2 = 0;
					bumpCathNode(sceneIdx);
				} else if (!_doneNIS[kEventKronosBringFirebird] && !_globals[kGlobalEggIsOpen]) {
					_closeUp = 0;
					if (_blockedViews[100 * _trainData[_nodeReturn].nodePosition.car + _trainData[_nodeReturn].cathDir]) {
						sceneIdx = getSmartBumpNode(_nodeReturn);
						bumpCathNode(sceneIdx);
					} else {
						bumpCathNode(_nodeReturn);
					}
				}
			}
		} else {
			_engine->getGraphicsManager()->drawItem(_globals[kGlobalCathIcon], 0, 0);
			_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);
		}

		_inventoryFlag1 = true;
	}

	if (_highlightedItem) {
		int count = 0;

		for (int i = 1; i < 32; i++) {
			if (_items[i].haveIt && _items[i].inPocket && count < 11) {
				count++;
				if (count == _highlightedItem) {
					selectedItemIdx = i;
					break;
				}
			}
		}

		if (selectedItemIdx) {
			_engine->getGraphicsManager()->drawItemDim(_items[selectedItemIdx].mnum, 0, 40 * _highlightedItem + 4, 1);
			_engine->getGraphicsManager()->burstBox(0, 40 * _highlightedItem + 4, 32, 32);
			_highlightedItem = 0;
		}
	}
}

void LogicManager::bumpCathNode(int sceneIndex) {
	_doubleClickFlag = false;
	_flagBumpCathNode = true;
	if (_closeUp && _trainData[sceneIndex].parameter3 != 0xFF) {
		_closeUp = 0;
		_nodeReturn2 = 0;
	}

	bool oldShouldRedraw = _engine->getGraphicsManager()->canDrawMouse();
	if (_engine->getGraphicsManager()->canDrawMouse()) {
		_engine->getGraphicsManager()->setMouseDrawable(false);
		_engine->getGraphicsManager()->burstMouseArea();
	}

	_engine->getGraphicsManager()->stepBG(sceneIndex);
	_engine->getMessageManager()->clearClickEvents();

	if (_engine->_navigationEngineIsRunning && _engine->getGraphicsManager()->_shouldDrawEggOrHourglass)
		restoreEggIcon();

	_engine->getGraphicsManager()->setMouseDrawable(oldShouldRedraw);
	mouseStatus();
}

void LogicManager::displayWaitIcon() {
	if (!_engine->getMenu()->isShowingMenu()) {
		_engine->getGraphicsManager()->drawItem(38, 608, 448);
		_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
	}

	_engine->getGraphicsManager()->setMouseDrawable(false);
	_engine->getGraphicsManager()->burstMouseArea();
	_engine->getGraphicsManager()->_shouldDrawEggOrHourglass = true;
}

void LogicManager::restoreEggIcon() {
	if (!_engine->getMenu()->isShowingMenu()) {
		_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, 1);
		_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
	}

	_engine->getGraphicsManager()->_shouldDrawEggOrHourglass = false;
}

} // End of namespace LastExpress
