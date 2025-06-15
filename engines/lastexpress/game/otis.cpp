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
#include "lastexpress/data/archive.h"
#include "lastexpress/game/otis.h"

namespace LastExpress {

OtisManager::OtisManager(LastExpressEngine *engine) {
	_engine = engine;
}

void OtisManager::wipeLooseSprites() {
	for (int i = 0; i < 8; i++) {
		if (_engine->_doorSeqs[i]) {
			_engine->getMemoryManager()->freeMem(_engine->_doorSeqs[i]->rawSeqData);
			delete _engine->_doorSeqs[i];
			_engine->_doorSeqs[i] = nullptr;
		}
	}
	for (int j = 0; j < 2; j++) {
		if (_engine->_clockHandsSeqs[j]) {
			_engine->getMemoryManager()->freeMem(_engine->_clockHandsSeqs[j]->rawSeqData);
			delete _engine->_clockHandsSeqs[j];
			_engine->_clockHandsSeqs[j] = nullptr;
		}
	}

	if (_engine->_beetle)
		_engine->endBeetle();

	_engine->getSpriteManager()->killSpriteQueue();
}

void OtisManager::wipeGSysInfo(int character) {
	if (getCharacter(character).direction == 5) {
		getCharacter(character).direction = getCharacter(character).directionSwitch;
		getCharacter(character).currentFrameSeq1 = -1;
		getCharacter(character).waitedTicksUntilCycleRestart = 0;
	}

	getCharacter(character).frame1 = nullptr;
	getCharacter(character).frame2 = nullptr;

	if (getCharacter(character).sequence3) {
		_engine->getMemoryManager()->freeMem(getCharacter(character).sequence3->rawSeqData);
		delete getCharacter(character).sequence3;
		getCharacter(character).sequence3 = nullptr;
	}

	if (getCharacter(character).sequence1) {
		_engine->getMemoryManager()->freeMem(getCharacter(character).sequence1->rawSeqData);
		delete getCharacter(character).sequence1;
		getCharacter(character).sequence1 = nullptr;
	}

	if (getCharacter(character).sequence2) {
		_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
		delete getCharacter(character).sequence2;
		getCharacter(character).sequence2 = nullptr;
	}

	getCharacter(character).needsPosFudge = 0;
	getCharacter(character).needsSecondaryPosFudge = 0;
	getCharacter(character).sequenceName[0] = '\0';
	getCharacter(character).sequenceName2[0] = '\0';
	getCharacter(character).sequenceNameCopy[0] = '\0';

	_engine->getSpriteManager()->killSpriteQueue();
}

void OtisManager::wipeAllGSysInfo() {
	for (int i = 0; i < 40; i++)
		wipeGSysInfo(i);

	wipeLooseSprites();
}

bool OtisManager::fDirection(int nodeIdx) {
	uint16 car;
	uint8 cathDir;

	if (!nodeIdx)
		nodeIdx = _engine->getLogicManager()->_activeNode;

	car = _engine->getLogicManager()->_trainData[nodeIdx].nodePosition.car;
	if (car == kCarGreenSleeping || car == kCarRedSleeping) {
		cathDir = _engine->getLogicManager()->_trainData[nodeIdx].cathDir;
		if (cathDir > 0 && cathDir < 20) {
			return true;
		}
	}

	return false;
}

bool OtisManager::rDirection(int nodeIdx) {
	uint16 car;
	uint8 cathDir;

	if (!nodeIdx)
		nodeIdx = _engine->getLogicManager()->_activeNode;

	car = _engine->getLogicManager()->_trainData[nodeIdx].nodePosition.car;
	if (car == kCarGreenSleeping || car == kCarRedSleeping) {
		cathDir = _engine->getLogicManager()->_trainData[nodeIdx].cathDir;
		if (cathDir >= 21 && cathDir <= 40) {
			return true;
		}
	}

	return false;
}

bool OtisManager::doorView() {
	uint16 car;
	uint8 cathDir;

	int32 nodeIdx = _engine->getLogicManager()->_activeNode;

	car = _engine->getLogicManager()->_trainData[nodeIdx].nodePosition.car;
	if (car == kCarGreenSleeping || car == kCarRedSleeping) {
		cathDir = _engine->getLogicManager()->_trainData[nodeIdx].cathDir;

		if (cathDir >= 41 && cathDir <= 48)
			return true;

		if (cathDir >= 51 && cathDir <= 58)
			return true;
	}

	return false;
}

bool OtisManager::corrRender(int nodeIdx) {
	uint16 car;
	uint8 cathDir;

	if (!nodeIdx)
		nodeIdx = _engine->getLogicManager()->_activeNode;

	car = _engine->getLogicManager()->_trainData[nodeIdx].nodePosition.car;
	if (car == kCarGreenSleeping || car == kCarRedSleeping) {
		cathDir = _engine->getLogicManager()->_trainData[nodeIdx].cathDir;

		if (cathDir >= 2 && cathDir <= 17)
			return true;

		if (cathDir >= 23 && cathDir <= 39)
			return true;
	}

	return false;
}

bool OtisManager::walkingRender() {
	uint8 cathDir = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir;
	uint16 car = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].nodePosition.car;

	if (cathDir == 99)
		return true;

	if ((car == kCarGreenSleeping || car == kCarRedSleeping || car == kCarLocomotive) && ((cathDir > 0 && cathDir <= 18) || (cathDir >= 22 && cathDir <= 40)))
		return true;

	if (car == kCarRestaurant) {
		if ((cathDir >= 73 && cathDir <= 76) || (cathDir >= 77 && cathDir <= 80))
			return true;

		if (cathDir == 10 || cathDir == 11)
			return true;
	}

	if (car != kCarBaggage)
		return car == kCarCoalTender && cathDir == 2;

	switch (cathDir) {
	case 80:
	case 81:
	case 82:
	case 83:
	case 84:
	case 90:
	case 91:
		return true;
	default:
		return car == kCarCoalTender && cathDir == 2;
	}

	return false;
}

int OtisManager::checkMouse(int32 cursorX, int32 cursorY) {
	int selectedCharacter = 0;
	uint16 hotspotPriority = 10000;

	if (!_engine->_navigationEngineIsRunning)
		return 0;

	for (int i = 0; i < 40; ++i) {
		if (getCharacter(i).frame1 &&
			cursorX >= getCharacter(i).frame1->hotspotX1 &&
			cursorX <= getCharacter(i).frame1->hotspotX2 &&
			cursorY >= getCharacter(i).frame1->hotspotY1 &&
			cursorY <= getCharacter(i).frame1->hotspotY2 &&
			getCharacter(i).frame1->hotspotPriority < hotspotPriority) {
			selectedCharacter = i;
			hotspotPriority = getCharacter(i).frame1->hotspotPriority;
		}
	}

	if (getCharacter(selectedCharacter).inventoryItem != kItemNone) {
		return selectedCharacter;
	} else {
		return 0;
	}
}

void OtisManager::startSeq(int character, int direction, bool loadSequence) {
	uint8 ticksToWaitUntilRestartCycle;

	char effSeqName2[13];
	char effSeqName1[13];
	char seqName2[13];
	char seqName1[13];

	memset(effSeqName2, 0, sizeof(effSeqName2));
	memset(effSeqName1, 0, sizeof(effSeqName1));
	memset(seqName2, 0, sizeof(seqName2));
	memset(seqName1, 0, sizeof(seqName1));

	if (direction == 3)
		ticksToWaitUntilRestartCycle = character + 35;
	else
		ticksToWaitUntilRestartCycle = 15;

	int8 savedNeedsPosFudge = getCharacter(character).needsPosFudge;
	getCharacter(character).doProcessEntity = 1;

	if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car || direction == 4) {
		getCharacter(character).directionSwitch = 0;
		getNewSeqName(character, direction, seqName1, seqName2);

		if (seqName1[0]) {
			if (!strcmp(seqName1, getCharacter(character).sequenceNameCopy)) {
				getCharacter(character).direction = direction;
				return;
			}

			if (direction == 3 || direction == 4) {
				strncpy(effSeqName1, seqName1, strlen(seqName1) - 6);
				effSeqName1[strlen(seqName1) - 6] = '\0';

				if (_engine->getLogicManager()->checkLoc(character, kCarGreenSleeping) || _engine->getLogicManager()->checkLoc(character, kCarRedSleeping)) {
					if (getCharacter(kCharacterCath).characterPosition.car > getCharacter(character).characterPosition.car ||
						(getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car &&
						getCharacter(kCharacterCath).characterPosition.position > getCharacter(character).characterPosition.position)) {
						Common::strcat_s(effSeqName1, "R.SEQ");
					} else {
						Common::strcat_s(effSeqName1, "F.SEQ");
					}
				} else {
					Common::strcat_s(effSeqName1, ".SEQ");
				}

				// The following appears to be an original bug, or some deliberate choice;
				// it seems this should check for seqName2[0] being not null instead.
				// I'm leaving this here for documentation purposes...
				if (effSeqName2[0]) {
					strncpy(effSeqName2, seqName2, strlen(seqName2) - 6);
					effSeqName2[strlen(seqName2) - 6] = '\0';

					if (_engine->getLogicManager()->checkLoc(character, 3) || _engine->getLogicManager()->checkLoc(character, 4)) {
						if (getCharacter(kCharacterCath).characterPosition.car > getCharacter(character).characterPosition.car ||
							(getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car &&
							getCharacter(kCharacterCath).characterPosition.position > getCharacter(character).characterPosition.position)) {
							Common::strcat_s(effSeqName2, "R.SEQ");
						} else {
							Common::strcat_s(effSeqName2, "F.SEQ");
						}
					} else {
						Common::strcat_s(effSeqName2, ".SEQ");
					}
				}
			}

			if (getCharacter(character).frame1) {
				if (strcmp(getCharacter(character).sequenceName, seqName1)) {
					if (strcmp(getCharacter(character).sequenceName2, seqName1)) {
						if (getCharacter(character).sequence2) {
							_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
							delete getCharacter(character).sequence2;
							getCharacter(character).sequence2 = nullptr;
						}

						if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car)
							getCharacter(character).sequence2 = _engine->getArchiveManager()->loadSeq(seqName1, ticksToWaitUntilRestartCycle, character);

						if (getCharacter(character).sequence2) {
							Common::strcpy_s(getCharacter(character).sequenceName2, seqName1);
						} else {
							if (effSeqName1[0])
								getCharacter(character).sequence2 = _engine->getArchiveManager()->loadSeq(effSeqName1, ticksToWaitUntilRestartCycle, character);

							if (getCharacter(character).sequence2) {
								Common::strcpy_s(getCharacter(character).sequenceName2, effSeqName1);
							} else {
								Common::strcpy_s(getCharacter(character).sequenceName2, "");
							}
						}
					}

					getCharacter(character).needsSecondaryPosFudge = getCharacter(character).needsPosFudge;

					if ((direction == 1 || direction == 2) && !getCharacter(character).needsSecondaryPosFudge && getCharacter(character).sequence2) {
						getCharacter(character).currentFrameSeq2 = findFrame(character, getCharacter(character).sequence2, 0, false);

						if (getCharacter(character).currentFrameSeq2 == -1) {
							_engine->getLogicManager()->endGraphics(character);
							return;
						}
					} else {
						getCharacter(character).currentFrameSeq2 = 0;
					}

					getCharacter(character).needsPosFudge = savedNeedsPosFudge;
					getCharacter(character).waitedTicksUntilCycleRestart = getCharacter(character).frame1->ticksToWaitUntilCycleRestart;
					getCharacter(character).currentFrameSeq1 = getCharacter(character).sequence1->numFrames - 1;
					getCharacter(character).direction = 5;
					getCharacter(character).directionSwitch = direction;
				} else {
					if (getCharacter(character).sequence2) {
						_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
						delete getCharacter(character).sequence2;
						getCharacter(character).sequence2 = nullptr;
					}

					getCharacter(character).sequence2 = _engine->getMemoryManager()->copySeq(getCharacter(character).sequence1);
					Common::strcpy_s(getCharacter(character).sequenceName2, getCharacter(character).sequenceName);

					getCharacter(character).needsSecondaryPosFudge = getCharacter(character).needsPosFudge;
					getCharacter(character).waitedTicksUntilCycleRestart = getCharacter(character).frame1->ticksToWaitUntilCycleRestart;
					getCharacter(character).currentFrameSeq1 = getCharacter(character).sequence1->numFrames - 1;
					getCharacter(character).direction = 5;
					getCharacter(character).directionSwitch = direction;

					if ((direction == 1 || direction == 2) && !getCharacter(character).needsSecondaryPosFudge && getCharacter(character).sequence2) {
						getCharacter(character).currentFrameSeq2 = findFrame(character, getCharacter(character).sequence2, 0, false);

						if (getCharacter(character).currentFrameSeq2 == -1)
							_engine->getLogicManager()->endGraphics(character);

						return;
					}

					getCharacter(character).currentFrameSeq2 = 0;
				}
			} else {
				getCharacter(character).direction = direction;

				if (strcmp(getCharacter(character).sequenceName, seqName1)) {
					if (getCharacter(character).sequence1) {
						_engine->getMemoryManager()->freeMem(getCharacter(character).sequence1->rawSeqData);
						delete getCharacter(character).sequence1;
						getCharacter(character).sequence1 = nullptr;
					}

					if (strcmp(getCharacter(character).sequenceName2, seqName1)) {
						if (loadSequence) {
							if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car)
								getCharacter(character).sequence1 = _engine->getArchiveManager()->loadSeq(seqName1, ticksToWaitUntilRestartCycle, character);

							if (getCharacter(character).sequence1) {
								Common::strcpy_s(getCharacter(character).sequenceName, seqName1);
								Common::strcpy_s(getCharacter(character).sequenceNameCopy, "");
							} else {
								if (effSeqName1[0])
									getCharacter(character).sequence1 = _engine->getArchiveManager()->loadSeq(effSeqName1, ticksToWaitUntilRestartCycle, character);

								if (getCharacter(character).sequence1) {
									Common::strcpy_s(getCharacter(character).sequenceName, effSeqName1);
									Common::strcpy_s(getCharacter(character).sequenceNameCopy, "");
								} else {
									Common::strcpy_s(getCharacter(character).sequenceName, "");
									Common::strcpy_s(getCharacter(character).sequenceNameCopy, seqName1);
								}
							}
						} else {
							Common::strcpy_s(getCharacter(character).sequenceName, seqName1);
						}

						if (!seqName2[0] && getCharacter(character).sequence2) {
							_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
							delete getCharacter(character).sequence2;
							getCharacter(character).sequence2 = nullptr;
							Common::strcpy_s(getCharacter(character).sequenceName2, "");
						}
					} else {
						getCharacter(character).sequence1 = getCharacter(character).sequence2;
						Common::strcpy_s(getCharacter(character).sequenceName, getCharacter(character).sequenceName2);
						getCharacter(character).sequence2 = nullptr;
						Common::strcpy_s(getCharacter(character).sequenceName2, "");
					}
				}

				if (seqName2[0] && strcmp(getCharacter(character).sequenceName2, seqName2)) {
					if (getCharacter(character).sequence2) {
						_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
						delete getCharacter(character).sequence2;
						getCharacter(character).sequence2 = nullptr;
					}

					if (loadSequence) {
						if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car)
							getCharacter(character).sequence2 = _engine->getArchiveManager()->loadSeq(seqName2, ticksToWaitUntilRestartCycle, character);

						if (!getCharacter(character).sequence2) {
							if (effSeqName2[0])
								getCharacter(character).sequence2 = _engine->getArchiveManager()->loadSeq(effSeqName2, ticksToWaitUntilRestartCycle, character);

							if (getCharacter(character).sequence2) {
								Common::strcpy_s(getCharacter(character).sequenceName2, effSeqName2);
							} else {
								Common::strcpy_s(getCharacter(character).sequenceName2, "");
							}
						} else {
							Common::strcpy_s(getCharacter(character).sequenceName2, seqName2);
						}
					} else {
						Common::strcpy_s(getCharacter(character).sequenceName2, seqName2);
					}
				}
			}
		} else {
			_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame1, false);
			_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame2, false);

			if (getCharacter(character).sequence1) {
				_engine->getMemoryManager()->freeMem(getCharacter(character).sequence1->rawSeqData);
				delete getCharacter(character).sequence1;
				getCharacter(character).sequence1 = nullptr;
			}

			if (getCharacter(character).sequence2) {
				_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
				delete getCharacter(character).sequence2;
				getCharacter(character).sequence2 = nullptr;
			}

			Common::strcpy_s(getCharacter(character).sequenceName, "");
			Common::strcpy_s(getCharacter(character).sequenceName2, "");

			getCharacter(character).needsPosFudge = 0;
			getCharacter(character).needsSecondaryPosFudge = 0;
			getCharacter(character).directionSwitch = 0;
			getCharacter(character).currentFrameSeq1 = -1;
			getCharacter(character).currentFrameSeq2 = 0;
			getCharacter(character).direction = direction;
		}
	} else {
		_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame1, false);
		_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame2, false);

		if (getCharacter(character).sequence1) {
			_engine->getMemoryManager()->freeMem(getCharacter(character).sequence1->rawSeqData);
			delete getCharacter(character).sequence1;
			getCharacter(character).sequence1 = nullptr;
		}

		if (getCharacter(character).sequence2) {
			_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
			delete getCharacter(character).sequence2;
			getCharacter(character).sequence2 = nullptr;
		}

		Common::strcpy_s(getCharacter(character).sequenceName, "");
		Common::strcpy_s(getCharacter(character).sequenceName2, "");

		getCharacter(character).needsPosFudge = 0;
		getCharacter(character).needsSecondaryPosFudge = 0;
		getCharacter(character).directionSwitch = 0;
		getCharacter(character).currentFrameSeq1 = -1;
		getCharacter(character).currentFrameSeq2 = 0;
		getCharacter(character).direction = direction;
	}
}

void OtisManager::getNewSeqName(int character, int direction, char *outSeqName, char *outSecondarySeqName) {
	getCharacter(character).needsPosFudge = 0;
	getCharacter(character).needsSecondaryPosFudge = 0;

	int outSeqNameSize = sizeof(getCharacter(kCharacterCath).sequenceName);

	if (direction == 2) {
		switch (_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir) {
		case 1:
			if (getCharacter(character).characterPosition.position > getCharacter(kCharacterCath).characterPosition.position)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-01d.seq", character, getCharacter(character).clothes);

			break;
		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			if (getCharacter(character).characterPosition.position > getCharacter(kCharacterCath).characterPosition.position) {
				if (getCharacter(character).characterPosition.position <= 8513) {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-03d.seq", character, getCharacter(character).clothes);
					getCharacter(character).needsPosFudge = 1;
				} else {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-%02ded.seq", character, getCharacter(character).clothes, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
					Common::sprintf_s(outSecondarySeqName, outSeqNameSize, "%02d%01d-03d.seq", character, getCharacter(character).clothes);
					getCharacter(character).needsSecondaryPosFudge = 1;
				}
			}

			break;
		case 18:
			if (getCharacter(character).characterPosition.position > getCharacter(kCharacterCath).characterPosition.position)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-18d.seq", character, getCharacter(character).clothes);

			break;
		case 22:
			if (getCharacter(character).characterPosition.position > 850)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-22d.seq", character, getCharacter(character).clothes);

			break;
		case 23:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
			if (getCharacter(character).characterPosition.position > 850) {
				if (getCharacter(character).characterPosition.position <= 2087) {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-%02ded.seq", character, getCharacter(character).clothes, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
				} else {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-38d.seq", character, getCharacter(character).clothes);
					Common::sprintf_s(outSecondarySeqName, outSeqNameSize, "%02d%01d-%02ded.seq", character, getCharacter(character).clothes, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
					getCharacter(character).needsPosFudge = 1;
				}
			}

			break;
		case 40:
			if (getCharacter(character).characterPosition.position > 8013)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-40d.seq", character, getCharacter(character).clothes);

			break;
		default:
			return;
		}
	} else if (direction >= 2) {
		if (direction <= 4)
			Common::sprintf_s(outSeqName, outSeqNameSize, "%s%02d.seq", getCharacter(character).sequenceNamePrefix, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
	} else if (direction > 0) {
		switch (_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir) {
		case 1:
			if (getCharacter(character).characterPosition.position < 2587)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-01u.seq", character, getCharacter(character).clothes);

			break;
		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			if (getCharacter(character).characterPosition.position < 9270) {
				if (getCharacter(character).characterPosition.position >= 8513) {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-%02deu.seq", character, getCharacter(character).clothes, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
				} else {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-03u.seq", character, getCharacter(character).clothes);
					Common::sprintf_s(outSecondarySeqName, outSeqNameSize, "%02d%01d-%02deu.seq", character, getCharacter(character).clothes, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
					getCharacter(character).needsPosFudge = 1;
				}
			}

			break;
		case 18:
			if (getCharacter(character).characterPosition.position < 9270)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-18u.seq", character, getCharacter(character).clothes);

			break;
		case 22:
			if (getCharacter(character).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-22u.seq", character, getCharacter(character).clothes);

			break;
		case 23:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
			if (getCharacter(character).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position) {
				if (getCharacter(character).characterPosition.position >= 2087) {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-38u.seq", character, getCharacter(character).clothes);
					getCharacter(character).needsPosFudge = 1;
				} else {
					Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-%02deu.seq", character, getCharacter(character).clothes, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
					Common::sprintf_s(outSecondarySeqName, outSeqNameSize, "%02d%01d-38u.seq", character, getCharacter(character).clothes);
					getCharacter(character).needsSecondaryPosFudge = 1;
				}
			}

			break;
		case 40:
			if (getCharacter(character).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position)
				Common::sprintf_s(outSeqName, outSeqNameSize, "%02d%01d-40u.seq", character, getCharacter(character).clothes);

			break;
		default:
			return;
		}
	}
}

void OtisManager::drawLooseSprites() {
	char seqName[28];

	for (int i = 0; i < 8; i++) {
		if (_engine->_doorSeqs[i]) {
			_engine->getSpriteManager()->removeSprite(_engine->_doorSeqs[i]->sprites);
			_engine->getSpriteManager()->queueErase(_engine->_doorSeqs[i]->sprites);
			_engine->getMemoryManager()->freeMem(_engine->_doorSeqs[i]->rawSeqData);
			delete _engine->_doorSeqs[i];
			_engine->_doorSeqs[i] = nullptr;
		}
	}

	for (int j = 0; j < 2; j++) {
		if (_engine->_clockHandsSeqs[j]) {
			_engine->getSpriteManager()->removeSprite(&_engine->_clockHandsSeqs[j]->sprites[_engine->_clockHandsValues[j]]);
			_engine->getSpriteManager()->queueErase(_engine->_clockHandsSeqs[j]->sprites);
			_engine->getMemoryManager()->freeMem(_engine->_clockHandsSeqs[j]->rawSeqData);
			delete _engine->_clockHandsSeqs[j];
			_engine->_clockHandsSeqs[j] = nullptr;
		}
	}

	int offset;
	if (corrRender(0)) {
		if (getCharacter(kCharacterCath).characterPosition.car == kCarGreenSleeping) {
			offset = 1;
		} else {
			if (getCharacter(kCharacterCath).characterPosition.car != kCarRedSleeping)
				return;
			offset = 32;
		}

		for (int k = offset; k < offset + 8; ++k) {
			if (_engine->getLogicManager()->_doors[k].status == 2) {
				Common::sprintf_s(seqName, "633X%c-%02d.seq", k - offset + 65, _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
				_engine->_doorSeqs[k - offset] = _engine->getArchiveManager()->loadSeq(seqName, 255, 0);
				if (_engine->_doorSeqs[k - offset]) {
					if (fDirection(0)) {
						_engine->_doorSeqs[k - offset]->sprites->hotspotPriority = offset - k - 1;
					} else {
						_engine->_doorSeqs[k - offset]->sprites->hotspotPriority = k - offset - 8;
					}

					_engine->getSpriteManager()->drawSprite(_engine->_doorSeqs[k - offset]->sprites);
				}
			}
		}
	} else if (_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].nodePosition.car == kCarRestaurant && _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir == 81) {
		Common::sprintf_s(seqName, "SCLKH-%02d.seq", _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
		_engine->_clockHandsSeqs[0] = _engine->getArchiveManager()->loadSeq(seqName, 255, 0);
		int32 hours = _engine->getLogicManager()->_gameTime % 1296000 % 54000 / 900;

		Common::sprintf_s(seqName, "SCLKM-%02d.seq", _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir);
		_engine->_clockHandsSeqs[1] = _engine->getArchiveManager()->loadSeq(seqName, 255, 0);

		int32 period = _engine->getLogicManager()->_gameTime % 1296000 / 54000;
		if (period >= 12)
			period -= 12;

		int32 minutes = hours / 12 + 5 * period;

		if (_engine->_clockHandsSeqs[0] && _engine->_clockHandsSeqs[1]) {
			_engine->_clockHandsSeqs[0]->sprites[hours].hotspotPriority = 0xFFFE;
			_engine->getSpriteManager()->drawSprite(&_engine->_clockHandsSeqs[0]->sprites[hours]);
			_engine->_clockHandsValues[0] = hours;

			_engine->_clockHandsSeqs[1]->sprites[minutes].hotspotPriority = 0xFFFF;
			_engine->getSpriteManager()->drawSprite(&_engine->_clockHandsSeqs[1]->sprites[minutes]);
			_engine->_clockHandsValues[1] = minutes;
		}
	}
}

void OtisManager::refreshSequences() {
	int targetFramerate;
	char seqName[13];
	char seqName2[13];

	memset(seqName, 0, sizeof(seqName));
	memset(seqName2, 0, sizeof(seqName2));

	if (_engine->_navigationEngineIsRunning) {
		drawLooseSprites();

		for (int i = 1; i < 40; ++i) {
			if (_engine->getMessageManager()->getMessageHandle(i)) {
				if (getCharacter(i).frame1) {
					_engine->getSpriteManager()->removeSprite(getCharacter(i).frame1);
					getCharacter(i).frame1 = nullptr;
				}

				if (getCharacter(i).frame2) {
					_engine->getSpriteManager()->removeSprite(getCharacter(i).frame2);
					getCharacter(i).frame2 = nullptr;
				}

				if (getCharacter(i).direction == 5) {
					if (getCharacter(i).sequence1) {
						_engine->getMemoryManager()->freeMem(&getCharacter(i).sequence1->rawSeqData);
						delete getCharacter(i).sequence1;
						getCharacter(i).sequence1 = nullptr;
					}

					if (getCharacter(i).sequence2) {
						getCharacter(i).sequence1 = getCharacter(i).sequence2;
						Common::strcpy_s(getCharacter(i).sequenceName, getCharacter(i).sequenceName2);
						getCharacter(i).sequence2 = nullptr;
						Common::strcpy_s(getCharacter(i).sequenceName2, "");
					}

					getCharacter(i).direction = getCharacter(i).directionSwitch;
					getCharacter(i).currentFrameSeq1 = -1;
					getCharacter(i).waitedTicksUntilCycleRestart = 0;
				}

				startSeq(i, getCharacter(i).direction, 0);
			}
		}

		for (int j = 1; j < 40; ++j) {
			if (_engine->getMessageManager()->getMessageHandle(j)) {
				if (getCharacter(j).direction == 3) {
					targetFramerate = j + 35;
				} else {
					targetFramerate = 15;
				}

				if (getCharacter(j).sequenceName[0] && !getCharacter(j).sequence1) {
					if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(j).characterPosition.car)
						getCharacter(j).sequence1 = _engine->getArchiveManager()->loadSeq(getCharacter(j).sequenceName, targetFramerate, j);

					if (getCharacter(j).sequence1) {
						Common::strcpy_s(getCharacter(j).sequenceNameCopy, "");
					} else {
						if (getCharacter(j).direction == 3 || getCharacter(j).direction == 4) {
							strncpy(seqName, getCharacter(j).sequenceName, strlen(getCharacter(j).sequenceName) - 6);
							seqName[strlen(getCharacter(j).sequenceName) - 6] = '\0';

							if (_engine->getLogicManager()->checkLoc(j, 3) || _engine->getLogicManager()->checkLoc(j, 4)) {
								if (getCharacter(j).characterPosition.car < getCharacter(kCharacterCath).characterPosition.car ||
									(getCharacter(j).characterPosition.car == getCharacter(kCharacterCath).characterPosition.car &&
									getCharacter(kCharacterCath).characterPosition.position > getCharacter(j).characterPosition.position)) {
									Common::strcat_s(seqName, "R.SEQ");
								} else {
									Common::strcat_s(seqName, "F.SEQ");
								}
							} else {
								Common::strcat_s(seqName, ".SEQ");
							}

							getCharacter(j).sequence1 = _engine->getArchiveManager()->loadSeq(seqName, targetFramerate, j);
						}

						if (getCharacter(j).sequence1) {
							Common::strcpy_s(getCharacter(j).sequenceName, seqName);
							Common::strcpy_s(getCharacter(j).sequenceNameCopy, "");
						} else {
							Common::strcpy_s(getCharacter(j).sequenceNameCopy, getCharacter(j).sequenceName);
							Common::strcpy_s(getCharacter(j).sequenceName, "");
						}
					}
				}

				if (getCharacter(j).sequenceName2[0] && !getCharacter(j).sequence2) {
					if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(j).characterPosition.car)
						getCharacter(j).sequence2 = _engine->getArchiveManager()->loadSeq(getCharacter(j).sequenceName2, targetFramerate, j);

					if (!getCharacter(j).sequence2) {
						if (getCharacter(j).directionSwitch == 3 || getCharacter(j).directionSwitch == 4) {
							strncpy(seqName2, getCharacter(j).sequenceName2, strlen(getCharacter(j).sequenceName2) - 6);
							seqName2[strlen(getCharacter(j).sequenceName2) - 6] = '\0';

							if (_engine->getLogicManager()->checkLoc(j, 3) || _engine->getLogicManager()->checkLoc(j, 4)) {
								if (getCharacter(j).characterPosition.car < getCharacter(kCharacterCath).characterPosition.car ||
									(getCharacter(j).characterPosition.car == getCharacter(kCharacterCath).characterPosition.car &&
									getCharacter(kCharacterCath).characterPosition.position > getCharacter(j).characterPosition.position)) {
									Common::strcat_s(seqName2, "R.SEQ");
								} else {
									Common::strcat_s(seqName2, "F.SEQ");
								}
							} else {
								Common::strcat_s(seqName2, ".SEQ");
							}

							getCharacter(j).sequence2 = _engine->getArchiveManager()->loadSeq(seqName2, targetFramerate, j);
						}

						if (getCharacter(j).sequence2) {
							Common::strcpy_s(getCharacter(j).sequenceName2, seqName2);
						} else {
							Common::strcpy_s(getCharacter(j).sequenceName2, "");
						}
						
					}
				}
			}
		}
	}
}

int OtisManager::findFrame(int character, Seq *sequence, int position, bool doProcessing) {
	if (!doProcessing) {
		if ((sequence->sprites->visibilityDist < sequence->sprites[sequence->numFrames - 1].visibilityDist) &&
			((getCharacter(character).characterPosition.position < sequence->sprites->visibilityDist - getCharacter(character).walkStepSize) ||
			 (getCharacter(character).characterPosition.position > sequence->sprites[sequence->numFrames - 1].visibilityDist + getCharacter(character).walkStepSize))) {
			return -1;
		}

		if ((sequence->sprites->visibilityDist >= sequence->sprites[sequence->numFrames - 1].visibilityDist) &&
			((getCharacter(character).characterPosition.position > sequence->sprites->visibilityDist + getCharacter(character).walkStepSize) ||
			 (getCharacter(character).characterPosition.position < sequence->sprites[sequence->numFrames - 1].visibilityDist - getCharacter(character).walkStepSize))) {
			return -1;
		}
	}

	int targetFrameIdx = 0;
	int lowerBound = 0;
	int numFrames = sequence->numFrames;

	while (numFrames > lowerBound) {
		targetFrameIdx = (lowerBound + numFrames) >> 1;
		if (getCharacter(character).characterPosition.position >= position + sequence->sprites[targetFrameIdx].visibilityDist) {
			if (sequence->sprites->visibilityDist >= sequence->sprites[sequence->numFrames - 1].visibilityDist) {
				numFrames = (lowerBound + numFrames) >> 1;
			} else {
				lowerBound = (lowerBound + numFrames) >> 1;
			}
		} else if (sequence->sprites->visibilityDist >= sequence->sprites[sequence->numFrames - 1].visibilityDist) {
			lowerBound = (lowerBound + numFrames) >> 1;
		} else {
			numFrames = (lowerBound + numFrames) >> 1;
		}

		if (numFrames - lowerBound == 1) {
			if (ABS<int>(position + getCharacter(character).characterPosition.position - sequence->sprites[lowerBound].visibilityDist) >=
				ABS<int>(position + getCharacter(character).characterPosition.position - sequence->sprites[numFrames].visibilityDist)) {
				targetFrameIdx = numFrames;
			} else {
				targetFrameIdx = lowerBound;
			}

			numFrames = lowerBound;
		}
	}

	return targetFrameIdx;
}

void OtisManager::initCurFrame(int character) {
	int32 ticks;
	int curFrameIdx;
	int tmpRemainingFrames;
	bool foundCurrentFrame = false;
	bool waitingForEndMarker = false;
	bool foundEndMarker = false;

	if (!getCharacter(character).sequence1) {
		getCharacter(character).currentFrameSeq1 = -1;
		return;
	}

	int16 currentFrameSeq1 = getCharacter(character).currentFrameSeq1;

	if (getCharacter(character).direction == 3) {
		if (getCharacter(character).currentFrameSeq1 == -1 || getCharacter(character).currentFrameSeq1 >= getCharacter(character).sequence1->numFrames) {
			getCharacter(character).currentFrameSeq1 = 0;
			getCharacter(character).waitedTicksUntilCycleRestart = 0;
		}
	} else {
		if (getCharacter(character).direction < 3) {
			if (getCharacter(character).direction) {
				switch (_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir) {
				case 1:
				case 18:
				case 22:
				case 40:
					getCharacter(character).currentFrameSeq1 = findFrame(
						character,
						getCharacter(character).sequence1,
						0,
						false);
					return;
				case 2:
				case 3:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
					if (!getCharacter(character).needsPosFudge) {
						getCharacter(character).currentFrameSeq1 = findFrame(
							character,
							getCharacter(character).sequence1,
							0,
							false);
						return;
					}

					if (getCharacter(character).characterPosition.position <= getCharacter(kCharacterCath).characterPosition.position) {
						getCharacter(character).currentFrameSeq1 = -1;
					} else {
						getCharacter(character).currentFrameSeq1 = findFrame(
							character,
							getCharacter(character).sequence1,
							getFudge(),
							true);

						if (getCharacter(character).currentFrameSeq1 != -1 && currentFrameSeq1 == getCharacter(character).currentFrameSeq1 && getCharacter(character).currentFrameSeq1 < (getCharacter(character).sequence1->numFrames - 2)) {
							getCharacter(character).currentFrameSeq1 += 2;
						}
					}

					break;

				case 23:
				case 25:
				case 26:
				case 27:
				case 28:
				case 29:
				case 30:
				case 31:
				case 32:
				case 33:
				case 34:
				case 35:
				case 36:
				case 37:
				case 38:
				case 39:
					if (getCharacter(character).needsPosFudge) {
						if (getCharacter(character).characterPosition.position >= getCharacter(kCharacterCath).characterPosition.position) {
							getCharacter(character).currentFrameSeq1 = -1;
						} else {
							getCharacter(character).currentFrameSeq1 = findFrame(
								character,
								getCharacter(character).sequence1,
								getFudge(),
								true);

							if (getCharacter(character).currentFrameSeq1 != -1 && currentFrameSeq1 == getCharacter(character).currentFrameSeq1 && getCharacter(character).currentFrameSeq1 < (getCharacter(character).sequence1->numFrames - 2)) {
								getCharacter(character).currentFrameSeq1 += 2;
							}
						}
					} else {
						getCharacter(character).currentFrameSeq1 = findFrame(
							character,
							getCharacter(character).sequence1,
							0,
							false);
					}

					break;
				default:
					return;
				}
				return;
			}

			getCharacter(character).currentFrameSeq1 = -1;
			return;
		}

		if (getCharacter(character).direction == 5) {
			getCharacter(character).currentFrameSeq1 = -1;
			return;
		}

		if (getCharacter(character).direction < 5) {
			ticks = 0;
			curFrameIdx = 0;
			tmpRemainingFrames = getCharacter(character).currentFrameSeq1;
			foundCurrentFrame = false;
			waitingForEndMarker = false;

			if (!getCharacter(character).currentFrameSeq1 && !getCharacter(character).elapsedFrames)
				tmpRemainingFrames = -1;

			while (!foundCurrentFrame && curFrameIdx < getCharacter(character).sequence1->numFrames) {
				if (ticks + getCharacter(character).sequence1->sprites[curFrameIdx].ticksToWaitUntilCycleRestart >= getCharacter(character).elapsedFrames) {
					foundCurrentFrame = true;
				} else {
					if (ticks > (getCharacter(character).elapsedFrames - 10) && getCharacter(character).sequence1->sprites[curFrameIdx].soundAction) {
						if (getCharacter(character).sequence1->sprites[curFrameIdx].soundDelay <= (getCharacter(character).elapsedFrames - ticks)) {
							_engine->getLogicManager()->queueSFX(character, getCharacter(character).sequence1->sprites[curFrameIdx].soundAction, 0);
						} else {
							_engine->getLogicManager()->queueSFX(
								character,
								getCharacter(character).sequence1->sprites[curFrameIdx].soundAction,
								getCharacter(character).sequence1->sprites[curFrameIdx].soundDelay - (getCharacter(character).elapsedFrames - ticks));
						}
					}

					ticks += getCharacter(character).sequence1->sprites[curFrameIdx].ticksToWaitUntilCycleRestart;
					if ((getCharacter(character).sequence1->sprites[curFrameIdx].flags & 4) != 0)
						waitingForEndMarker = true;

					if ((getCharacter(character).sequence1->sprites[curFrameIdx].flags & 2) != 0) {
						waitingForEndMarker = false;
						_engine->getLogicManager()->send(kCharacterCath, character, 10, 0);
						_engine->getMessageManager()->flush();
						if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
							return;
					}

					if ((getCharacter(character).sequence1->sprites[curFrameIdx].flags & 0x10) != 0) {
						_engine->getLogicManager()->send(kCharacterCath, character, 4, 0);
						_engine->getMessageManager()->flush();
						if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
							return;
					}

					curFrameIdx++;
				}
			}

			if (foundCurrentFrame) {
				if (waitingForEndMarker) {
					foundEndMarker = false;
					while (!foundEndMarker && curFrameIdx < getCharacter(character).sequence1->numFrames) {
						if ((getCharacter(character).sequence1->sprites[curFrameIdx].flags & 2) != 0) {
							foundEndMarker = true;
							_engine->getLogicManager()->send(kCharacterCath, character, 10, 0);
							_engine->getMessageManager()->flush();

							if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
								return;
						} else {
							getCharacter(character).elapsedFrames += getCharacter(character).sequence1->sprites[curFrameIdx].ticksToWaitUntilCycleRestart;

							if (getCharacter(character).sequence1->sprites[curFrameIdx].soundAction)
								_engine->getLogicManager()->queueSFX(character, getCharacter(character).sequence1->sprites[curFrameIdx].soundAction, 0);

							curFrameIdx++;
						}
					}

					if (foundEndMarker) {
						getCharacter(character).currentFrameSeq1 = curFrameIdx;
						getCharacter(character).waitedTicksUntilCycleRestart = 0;

						if (getCharacter(character).sequence1->sprites[curFrameIdx].soundAction && curFrameIdx != tmpRemainingFrames)
							_engine->getLogicManager()->queueSFX(
								character,
								getCharacter(character).sequence1->sprites[curFrameIdx].soundAction,
								getCharacter(character).sequence1->sprites[curFrameIdx].soundDelay);
					} else {
						getCharacter(character).currentFrameSeq1 = getCharacter(character).sequence1->numFrames - 1;
						getCharacter(character).waitedTicksUntilCycleRestart = getCharacter(character).sequence1->sprites[getCharacter(character).currentFrameSeq1].ticksToWaitUntilCycleRestart;
					}
				} else {
					getCharacter(character).currentFrameSeq1 = curFrameIdx;
					getCharacter(character).waitedTicksUntilCycleRestart = getCharacter(character).elapsedFrames - ticks;

					if (getCharacter(character).sequence1->sprites[curFrameIdx].soundAction && curFrameIdx != tmpRemainingFrames) {
						if (getCharacter(character).sequence1->sprites[curFrameIdx].soundDelay <= getCharacter(character).waitedTicksUntilCycleRestart) {
							_engine->getLogicManager()->queueSFX(character, getCharacter(character).sequence1->sprites[curFrameIdx].soundAction, 0);
						} else {
							_engine->getLogicManager()->queueSFX(
								character,
								getCharacter(character).sequence1->sprites[curFrameIdx].soundAction,
								getCharacter(character).sequence1->sprites[curFrameIdx].soundDelay - getCharacter(character).waitedTicksUntilCycleRestart);
						}
					}
				}
			} else {
				getCharacter(character).currentFrameSeq1 = getCharacter(character).sequence1->numFrames - 1;
				getCharacter(character).waitedTicksUntilCycleRestart = getCharacter(character).sequence1->sprites[getCharacter(character).currentFrameSeq1].ticksToWaitUntilCycleRestart;

				_engine->getLogicManager()->send(kCharacterCath, character, 3, 0);
				_engine->getMessageManager()->flush();
			}
		}
	}
}

bool OtisManager::mainWalkTooFar(int character) {
	if (getCharacter(character).direction == 1) {
		if (fDirection(0)) {
			if (getFudge() + getCharacter(character).sequence1->sprites[getCharacter(character).currentFrameSeq1].visibilityDist > 8513)
				return true;
		}

		return false;
	}

	if (getCharacter(character).direction != 2)
		return false;

	if (!rDirection(0))
		return false;

	return getFudge() + getCharacter(character).sequence1->sprites[getCharacter(character).currentFrameSeq1].visibilityDist < 2087;
}

int OtisManager::getFudge() {
	if (fDirection(0))
		return _engine->_fudgePosition[_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir] - _engine->_fudgePosition[2];

	if (rDirection(0))
		return _engine->_fudgePosition[_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].cathDir] - _engine->_fudgePosition[39];

	return 0;
}

void OtisManager::updateCharacter(int character) {
	bool keepPreviousFrame = false;

	getCharacter(character).doProcessEntity = 0;

	if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car ||
		getCharacter(character).direction == 4 || getCharacter(character).direction == 5) {
		if (getCharacter(character).frame2) {
			_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame2, false);

			if (getCharacter(character).frame1) {
				if (getCharacter(character).frame1->compType != 3) {
					getCharacter(character).frame1->compType = 0;
					_engine->getSpriteManager()->touchSpriteQueue();
				}
			}
		}

		if (getCharacter(character).sequence3) {
			_engine->getMemoryManager()->freeMem(getCharacter(character).sequence3->rawSeqData);
			delete getCharacter(character).sequence3;
			getCharacter(character).sequence3 = nullptr;
		}

		if (getCharacter(character).frame1 && getCharacter(character).direction && getCharacter(character).sequence1) {
			if (getCharacter(character).waitedTicksUntilCycleRestart < getCharacter(character).frame1->ticksToWaitUntilCycleRestart - 1 ||
				(getCharacter(character).direction == 3 && getCharacter(character).sequence1->numFrames == 1)) {
				getCharacter(character).waitedTicksUntilCycleRestart++;
			} else if (getCharacter(character).waitedTicksUntilCycleRestart >= getCharacter(character).frame1->ticksToWaitUntilCycleRestart || getCharacter(character).frame1->spritesUnk3) {
				if (getCharacter(character).frame1->spritesUnk3 == 1)
					keepPreviousFrame = true;

				getCharacter(character).currentFrameSeq1++;

				if (getCharacter(character).currentFrameSeq1 > (getCharacter(character).sequence1->numFrames - 1) || (getCharacter(character).needsPosFudge && mainWalkTooFar(character))) {
					if (getCharacter(character).direction == 3) {
						getCharacter(character).currentFrameSeq1 = 0;
					} else {
						keepPreviousFrame = true;
						doSeqChange(character);

						if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
							return;

						if (!getCharacter(character).sequence2) {
							doNoSeq(character);
							getCharacter(character).doProcessEntity = 0;
							return;
						}

						doNextSeq(character);
					}
				}

				doNewSprite(character, keepPreviousFrame, false);

				if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
					return;
			} else {
				getCharacter(character).waitedTicksUntilCycleRestart++;
			}
		} else {
			if (getCharacter(character).sequence1) {
				getCharacter(character).doProcessEntity = 0;
				initCurFrame(character);

				if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
					return;
			} else {
				startSeq(character, getCharacter(character).direction, true);
				getCharacter(character).doProcessEntity = 0;
				initCurFrame(character);

				if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
					return;
			}

			if (!getCharacter(character).sequence1 || getCharacter(character).currentFrameSeq1 == -1 ||
				getCharacter(character).currentFrameSeq1 > (getCharacter(character).sequence1->numFrames - 1)) {
				if (getCharacter(character).direction == 4 && getCharacter(character).elapsedFrames > 100) {
					_engine->getLogicManager()->send(kCharacterCath, character, 3, 0);
					_engine->getMessageManager()->flush();

					if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
						return;
				}

				if (getCharacter(character).position2) {
					_engine->getLogicManager()->releaseView(character, getCharacter(character).car2, getCharacter(character).position2);
					getCharacter(character).car2 = 0;
					getCharacter(character).position2 = 0;
				}
			} else {
				doNewSprite(character, false, true);
				if (_engine->_stopUpdatingCharacters || getCharacter(character).doProcessEntity)
					return;
			}
		}

		getCharacter(character).doProcessEntity = 0;

		if (getCharacter(character).direction == 4 || (getCharacter(character).direction == 5 && getCharacter(character).directionSwitch == 4))
			getCharacter(character).elapsedFrames++;
	} else {
		if (getCharacter(character).position2) {
			_engine->getLogicManager()->releaseView(character, getCharacter(character).car2, getCharacter(character).position2);
			getCharacter(character).car2 = 0;
			getCharacter(character).position2 = 0;
		}

		_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame1, false);
		_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame2, false);

		getCharacter(character).doProcessEntity = 0;

		if (getCharacter(character).direction == 4 || (getCharacter(character).direction == 5 && getCharacter(character).directionSwitch == 4))
			getCharacter(character).elapsedFrames++;
	}
}

void OtisManager::doNewSprite(int character, bool keepPreviousFrame, bool dontPlaySound) {
	if (getCharacter(character).frame1 && keepPreviousFrame) {
		if (getCharacter(character).frame1->compType != 3)
			getCharacter(character).frame1->compType = 2;

		_engine->getSpriteManager()->touchSpriteQueue();
	}

	if (getCharacter(character).frame1 && !keepPreviousFrame)
		_engine->getSpriteManager()->removeSprite(getCharacter(character).frame1);

	if (getCharacter(character).currentFrameSeq1 >= 0 && getCharacter(character).currentFrameSeq1 < getCharacter(character).sequence1->numFrames) {
		Sprite *newSprite = &getCharacter(character).sequence1->sprites[getCharacter(character).currentFrameSeq1];

		if (getCharacter(character).frame1 && getCharacter(character).frame1->compType != 3 && (!newSprite->copyScreenAndRedrawFlag || keepPreviousFrame)) {
			_engine->getSpriteManager()->queueErase(getCharacter(character).frame1);
		}

		if (newSprite->visibilityDist) {
			getCharacter(character).characterPosition.position = newSprite->visibilityDist;
			if (getCharacter(character).needsPosFudge)
				getCharacter(character).characterPosition.position += getFudge();
		}

		newSprite->hotspotPriority = character + _engine->getLogicManager()->absPosition(character, kCharacterCath);

		if (newSprite->compType != 3) {
			if (keepPreviousFrame) {
				newSprite->compType = 1;
			} else {
				newSprite->compType = 0;
			}
		}

		if ((newSprite->flags & 1) != 0)
			_engine->getLogicManager()->send(kCharacterCath, character, 5, 0);

		if ((newSprite->flags & 2) == 0 || ((_engine->getLogicManager()->send(kCharacterCath, character, 10, 0), _engine->getMessageManager()->flush(), !_engine->_stopUpdatingCharacters) && !getCharacter(character).doProcessEntity)) {
			if ((newSprite->flags & 0x10) == 0 || ((_engine->getLogicManager()->send(kCharacterCath, character, 4, 0), _engine->getMessageManager()->flush(), !_engine->_stopUpdatingCharacters) && !getCharacter(character).doProcessEntity)) {
				if (getCharacter(character).position2) {
					_engine->getLogicManager()->releaseView(character, getCharacter(character).car2, getCharacter(character).position2);
					getCharacter(character).car2 = 0;
					getCharacter(character).position2 = 0;
				}

				if (!newSprite->position ||
					((getCharacter(character).car2 = getCharacter(character).characterPosition.car, getCharacter(character).position2 = newSprite->position,
						_engine->getLogicManager()->blockView(character, getCharacter(character).car2, getCharacter(character).position2),
						!_engine->_stopUpdatingCharacters) && !getCharacter(character).doProcessEntity)) {
					if (newSprite->soundAction && !dontPlaySound)
						_engine->getLogicManager()->queueSFX(character, newSprite->soundAction, newSprite->soundDelay);

					_engine->getSpriteManager()->drawSprite(newSprite);

					if (keepPreviousFrame)
						getCharacter(character).frame2 = getCharacter(character).frame1;

					getCharacter(character).frame1 = newSprite;

					if (!dontPlaySound) {
						if (keepPreviousFrame) {
							getCharacter(character).waitedTicksUntilCycleRestart = 0;
						} else {
							getCharacter(character).waitedTicksUntilCycleRestart = 1;
						}
					}
				}
			}
		}
	}
}

void OtisManager::doSeqChange(int character) {
	if (getCharacter(character).direction != 4 || ((_engine->getLogicManager()->send(kCharacterCath, character, 3, 0), _engine->getMessageManager()->flush(), !_engine->_stopUpdatingCharacters) && !getCharacter(character).doProcessEntity)) {
		if (_engine->getLogicManager()->whoWalking(character) && !getCharacter(character).sequence2 && corrRender(0) && getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car) {
			if (getCharacter(character).needsPosFudge && !_engine->getLogicManager()->whoFacingCath(character)) {
				getCharacter(character).characterPosition.position = 8514;

				if (getCharacter(character).direction != 1)
					getCharacter(character).characterPosition.position = 2086;

				startSeq(character, getCharacter(character).direction, 1);
			} else if (!getCharacter(character).needsPosFudge && _engine->getLogicManager()->whoFacingCath(character)) {
				getCharacter(character).characterPosition.position = 2088;

				if (getCharacter(character).direction != 1)
					getCharacter(character).characterPosition.position = 8512;

				startSeq(character, getCharacter(character).direction, 1);
			}
		}
	}
}

void OtisManager::doNextSeq(int character) {
	if (getCharacter(character).sequence1)
		getCharacter(character).sequence3 = getCharacter(character).sequence1;

	getCharacter(character).sequence1 = getCharacter(character).sequence2;
	Common::strcpy_s(getCharacter(character).sequenceName, getCharacter(character).sequenceName2);
	getCharacter(character).needsPosFudge = getCharacter(character).needsSecondaryPosFudge;

	if (getCharacter(character).directionSwitch)
		getCharacter(character).direction = getCharacter(character).directionSwitch;

	getCharacter(character).sequence2 = nullptr;
	Common::strcpy_s(getCharacter(character).sequenceName2, "");
	getCharacter(character).needsSecondaryPosFudge = 0;
	getCharacter(character).directionSwitch = 0;

	if (getCharacter(character).needsPosFudge) {
		initCurFrame(character);

		if (getCharacter(character).currentFrameSeq1 == -1)
			getCharacter(character).currentFrameSeq1 = 0;
	} else {
		getCharacter(character).currentFrameSeq1 = getCharacter(character).currentFrameSeq2;
		getCharacter(character).currentFrameSeq2 = 0;

		if (getCharacter(character).currentFrameSeq1 == -1)
			getCharacter(character).currentFrameSeq1 = 0;
	}
}

void OtisManager::doNoSeq(int character) {
	_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame1, false);
	getCharacter(character).frame2 = nullptr;
	getCharacter(character).waitedTicksUntilCycleRestart = 0;

	if (_engine->getLogicManager()->whoWalking(character) && (fDirection(0) || rDirection(0)) && getCharacter(character).characterPosition.car == getCharacter(kCharacterCath).characterPosition.car) {
		if (_engine->getLogicManager()->whoFacingCath(character)) {
			getCharacter(character).characterPosition.position = getCharacter(0).characterPosition.position;
		} else {
			if (getCharacter(character).needsPosFudge) {
				if (getCharacter(character).direction == 1) {
					getCharacter(character).characterPosition.position = 8514;
				} else {
					getCharacter(character).characterPosition.position = 2086;
				}
			} else if (_engine->getLogicManager()->checkCathDir(kCarGreenSleeping, 1) || _engine->getLogicManager()->checkCathDir(kCarGreenSleeping, 40) || _engine->getLogicManager()->checkCathDir(kCarRedSleeping, 1) || _engine->getLogicManager()->checkCathDir(kCarRedSleeping, 40)) {
				getCharacter(character).characterPosition.position = 2588;

				if (getCharacter(character).direction != 1)
					getCharacter(character).characterPosition.position = 8012;
			} else {
				if (getCharacter(character).direction == 1) {
					getCharacter(character).characterPosition.position = 9271;
				} else {
					getCharacter(character).characterPosition.position = 849;
				}
			}
		}
	}

	if (getCharacter(character).sequence1) {
		_engine->getMemoryManager()->freeMem(getCharacter(character).sequence1->rawSeqData);
		delete getCharacter(character).sequence1;
		getCharacter(character).sequence1 = nullptr;
	}

	Common::strcpy_s(getCharacter(character).sequenceName, "");
	getCharacter(character).needsPosFudge = 0;

	if (getCharacter(character).directionSwitch)
		getCharacter(character).direction = getCharacter(character).directionSwitch;
}

void OtisManager::updateAll() {
	if (_engine->_navigationEngineIsRunning) {
		_engine->_stopUpdatingCharacters = false;
		if (_engine->_charactersUpdateRecursionFlag) {
			goUpdateAll();
			_engine->_stopUpdatingCharacters = true;
		} else {
			_engine->_charactersUpdateRecursionFlag = true;
			goUpdateAll();
			_engine->_charactersUpdateRecursionFlag = false;
		}
	}
}

void OtisManager::goUpdateAll() {
	for (int i = 1; i < 40; ++i) {
		if (_engine->_stopUpdatingCharacters)
			break;

		if (_engine->getMessageManager()->getMessageHandle(i))
			updateCharacter(i);
	}

	bool done = false;
	if (!_engine->_stopUpdatingCharacters) {
		while (!done) {
			done = true;
			for (int j = 1; j < 40; ++j) {
				if (_engine->_stopUpdatingCharacters)
					break;

				if (_engine->getMessageManager()->getMessageHandle(j)) {
					if (getCharacter(j).doProcessEntity) {
						updateCharacter(j);
						done = false;
					}
				}
			}
		}
	}
}

void OtisManager::adjustOtisTrueTime() {
	if (_engine->_navigationEngineIsRunning) {
		for (int i = 0; i < 40; ++i) {
			if (_engine->getMessageManager()->getMessageHandle(i)) {
				if (getCharacter(i).direction == 4 ||
					(getCharacter(i).direction == 5 && getCharacter(i).directionSwitch == 4)) {
					getCharacter(i).elapsedFrames += 9;
				} else if (getCharacter(i).direction == 3) {
					getCharacter(i).currentFrameSeq1++;
				} else if (_engine->getLogicManager()->whoWalking(i)) {
					int16 step = 10 * getCharacter(i).walkStepSize;

					if (getCharacter(i).direction == 1 &&
						((int16)getCharacter(i).characterPosition.position < 10000 - (int16)(10 * getCharacter(i).walkStepSize))) {
						getCharacter(i).characterPosition.position += step;
					} else if (getCharacter(i).direction == 2 &&
						(getCharacter(i).characterPosition.position > (int16)(10 * getCharacter(i).walkStepSize))) {
						getCharacter(i).characterPosition.position -= step;
					}
				}
			}
		}
	}
}

} // End of namespace LastExpress
