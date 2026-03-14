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

#include "bolt/bolt.h"

namespace Bolt {

int16 BoltEngine::georgeGame(int16 prevBooth) {
	int16 result;
	int16 prevInactivity = _xp->setInactivityTimer(30);

	if (!initGeorge()) {
		cleanUpGeorge();
		return 7;
	}

	result = playGeorge();
	cleanUpGeorge();
	_xp->setInactivityTimer(prevInactivity);
	return result;
}

bool BoltEngine::initGeorge() {
	if (!openBOLTLib(&_georgeBoltLib, &_georgeBoltCallbacks, assetPath("george.blt")))
		return false;

	if (!getBOLTGroup(_georgeBoltLib, 0, 1))
		return false;

	_georgeBgPic = memberAddr(_georgeBoltLib, (_displayMode != 0) ? 1 : 0);
	_georgeHelpObjects = memberAddr(_georgeBoltLib, 0x1F);
	_georgeHelpSequence = memberAddr(_georgeBoltLib, 0x20);

	_georgeCarPics[0] = memberAddr(_georgeBoltLib, 0x10);
	_georgeCarPics[1] = memberAddr(_georgeBoltLib, 0x11);
	_georgeCarPics[2] = memberAddr(_georgeBoltLib, 0x12);
	_georgeCollisionRect = memberAddr(_georgeBoltLib, 0x13);

	getGeorgeSoundInfo(_georgeBoltLib, 0x16, &_georgeSoundCarTumble, 6);
	getGeorgeSoundInfo(_georgeBoltLib, 0x15, &_georgeSoundCarLoopHi, 0);
	getGeorgeSoundInfo(_georgeBoltLib, 0x14, &_georgeSoundCarLoopLo, 0);
	getGeorgeSoundInfo(_georgeBoltLib, 0x17, &_georgeSoundCarStartUp, 3);
	getGeorgeSoundInfo(_georgeBoltLib, 0x18, &_georgeSoundCarGoesAway, 3);

	_georgeSoundCurrent = nullptr;
	_georgeSoundToPlay = nullptr;
	_georgeSoundNext = nullptr;
	_georgeSoundQueued = nullptr;
	_georgeSoundChannelCounter = 0;

	// Load save data...
	if (!vLoad(&_georgeSaveData, "GeorgeBE")) {
		_georgeSaveData[0] = 0;
		_georgeSaveData[1] = 0;
		_georgeSaveData[2] = 0;
	}

	if (!initGeorgeLevel(_georgeSaveData[1], _georgeSaveData[2])) {
		freeBOLTGroup(_georgeBoltLib, 0, 1);
		return false;
	}

	// Initialize satellites from list...
	_georgeCollectedSatellitesNum = 0;
	while (_georgeCollectedSatellitesNum < _georgeSaveData[0]) {
		setSatelliteAnimMode(_georgeEntityList[_georgeNumSatellites + _georgeCollectedSatellitesNum], 4, 0);
		_georgeCollectedSatellitesNum++;
	}

	// Drain timer events...
	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	// Display palette and background to front...
	displayColors(_georgePalette, stBack, 0);
	displayPic(_georgeBgPic, _displayX, _displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);

	// Display palette to both surfaces...
	displayColors(_georgePalette, stFront, 0);
	displayColors(_georgePalette, stBack, 1);

	// Display background to back surface...
	displayPic(_georgeBgPic, _displayX, _displayY, stBack);

	drawFlyingObjects();

	_xp->updateDisplay();
	_xp->startCycle(_georgePalCycleSpecs);
	_xp->setFrameRate(12);

	return true;
}

void BoltEngine::cleanUpGeorge() {
	// Save current progress...
	int16 savedLevel = _georgeSaveData[1];
	int16 savedVariant = _georgeSaveData[2];

	_georgeSaveData[0] = _georgeCollectedSatellitesNum;

	// Check if collection threshold for current level was met...
	if (_georgeCollectedSatellitesNum >= _georgeThresholds[0]) {
		// Advance level...
		_georgeSaveData[1]++;
		if (_georgeSaveData[1] >= 10)
			_georgeSaveData[1] = 9;

		// Advance variant...
		_georgeSaveData[2]++;
		if (_georgeSaveData[2] >= 10)
			_georgeSaveData[2] = 0;

		// Reset collected count for next level...
		_georgeSaveData[0] = 0;
	}

	vSave(_georgeSaveData, sizeof(_georgeSaveData), "GeorgeBE");

	_xp->stopCycle();

	termGeorgeLevel(savedLevel, savedVariant);

	freeBOLTGroup(_georgeBoltLib, 0, 1);
	closeBOLTLib(&_georgeBoltLib);

	_xp->setFrameRate(0);
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
}

bool BoltEngine::initGeorgeLevel(int16 level, int16 variant) {
	int16 levelGroup = (level * 2) << 8 | 0x100;
	int16 variantGroup = (variant * 2) << 8 | 0x200;

	if (!getBOLTGroup(_georgeBoltLib, levelGroup, 1))
		return false;

	if (!getBOLTGroup(_georgeBoltLib, variantGroup, 1)) {
		freeBOLTGroup(_georgeBoltLib, levelGroup, 1);
		return false;
	}

	_georgeThresholds = (int16 *)memberAddr(_georgeBoltLib, levelGroup);
	_georgePalette = memberAddr(_georgeBoltLib, variantGroup);
	_georgePalCycleRawData = memberAddr(_georgeBoltLib, variantGroup + 1);
	boltCycleToXPCycle(_georgePalCycleRawData, _georgePalCycleSpecs);

	_georgeSatelliteGfx = memberAddr(_georgeBoltLib, variantGroup + 0x12);
	_georgeAsteroidGfx = memberAddr(_georgeBoltLib, variantGroup + 0x26);
	_georgeSatelliteThresholds = (int16 *)memberAddr(_georgeBoltLib, levelGroup + 0x01);
	_georgeAsteroidThresholds = (int16 *)memberAddr(_georgeBoltLib, levelGroup + 0x02);
	_georgeSatelliteCollisionRects = memberAddr(_georgeBoltLib, variantGroup + 0x19);
	_georgeAsteroidCollisionRects = memberAddr(_georgeBoltLib, variantGroup + 0x2A);
	_georgeSatellitePaths = memberAddr(_georgeBoltLib, levelGroup + 0x27);
	_georgeAsteroidPaths = memberAddr(_georgeBoltLib, levelGroup + 0x3C);

	// Allocate and build asteroid shuffle table...
	int16 numAsteroidRows = _georgeThresholds[3];
	int16 numAsteroidCols = _georgeThresholds[4];
	_georgeSatelliteShuffleTable = (byte *)_xp->allocMem((uint32)(numAsteroidCols + 1) * (numAsteroidRows));
	if (!_georgeSatelliteShuffleTable) {
		termGeorgeLevel(level, variant);
		return false;
	}

	// Fill each row with sequential values 0..numAsteroidCols, then shuffle...
	for (int16 row = 0; row < numAsteroidRows; row++) {
		int16 rowBase = (numAsteroidCols + 1) * row;
		_georgeSatelliteShuffleTable[rowBase] = 0;

		for (int16 col = 0; col < numAsteroidCols; col++)
			_georgeSatelliteShuffleTable[rowBase + col + 1] = (byte)col;

		// Fisher-Yates shuffle...
		for (int16 i = 0; i < numAsteroidCols; i++) {
			int16 j = _xp->getRandom(numAsteroidCols);
			byte tmp = _georgeSatelliteShuffleTable[rowBase + i + 1];
			_georgeSatelliteShuffleTable[rowBase + i + 1] = _georgeSatelliteShuffleTable[rowBase + j + 1];
			_georgeSatelliteShuffleTable[rowBase + j + 1] = tmp;
		}
	}

	// Allocate and build satellite shuffle table...
	int16 numSatelliteRows = _georgeThresholds[9];
	int16 numSatelliteCols = _georgeThresholds[10];
	_georgeAsteroidShuffleTable = (byte *)_xp->allocMem((uint32)(numSatelliteCols + 1) * (numSatelliteRows));
	if (!_georgeAsteroidShuffleTable) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 row = 0; row < numSatelliteRows; row++) {
		int16 rowBase = (numSatelliteCols + 1) * row;
		_georgeAsteroidShuffleTable[rowBase] = 0;

		for (int16 col = 0; col < numSatelliteCols; col++)
			_georgeAsteroidShuffleTable[rowBase + col + 1] = (byte)col;

		for (int16 i = 0; i < numSatelliteCols; i++) {
			int16 j = _xp->getRandom(numSatelliteCols);
			byte tmp = _georgeAsteroidShuffleTable[rowBase + i + 1];
			_georgeAsteroidShuffleTable[rowBase + i + 1] = _georgeAsteroidShuffleTable[rowBase + j + 1];
			_georgeAsteroidShuffleTable[rowBase + j + 1] = tmp;
		}
	}

	// Compute satellite list indices...
	_georgeNumSatellites = 0;
	int16 numSatellites = _georgeThresholds[0];
	_georgeFirstAsteroidIdx = numSatellites;

	int16 numAsteroids = _georgeThresholds[11];
	if (numAsteroids < 1)
		numAsteroids = 1;

	_georgeCarIdx = _georgeFirstAsteroidIdx + numAsteroids;
	_georgeTotalSatellites = _georgeCarIdx + 1;

	// Allocate satellite list...
	_georgeEntityList = (GeorgeEntityState **)_xp->allocMem((_georgeTotalSatellites + 1) * sizeof(GeorgeEntityState *));
	if (!_georgeEntityList) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 i = 0; i < _georgeTotalSatellites; i++) {
		_georgeEntityList[i] = new GeorgeEntityState();
		if (!_georgeEntityList[i]) {
			termGeorgeLevel(level, variant);
			return false;
		}

		_georgeEntityList[i]->flags = 0;
	}

	_georgeEntityList[_georgeTotalSatellites] = nullptr;

	GeorgeEntityState *carSat = _georgeEntityList[_georgeCarIdx];
	assert(carSat);

	carSat->flags = 1;
	carSat->joyInput = 0;
	carSat->x = 0x4000;  // fixed point = 64.0
	carSat->y = 0x12C00; // fixed point = 300.0
	carSat->velX = 0;
	carSat->velY = 0;
	carSat->accelX = 0;
	carSat->accelY = 0;
	carSat->pathTable = nullptr;
	carSat->pathIndex = 0;
	setGeorgeAnimMode(carSat, 0);

	int32 numObjectTypes = _georgeThresholds[6];
	byte *objShuf = (byte *)_xp->allocMem(numObjectTypes);
	if (!objShuf) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 i = 0; i < numObjectTypes; i++)
		objShuf[i] = (byte)i;

	// Assign random anim modes to satellites...
	for (int16 i = 0; i < numSatellites; i++) {
		if ((i % numObjectTypes) == 0) {
			// Re-shuffle every full cycle
			for (int16 j = 0; j < numObjectTypes; j++) {
				int16 k = _xp->getRandom(numObjectTypes);
				byte tmp = objShuf[j];
				objShuf[j] = objShuf[k];
				objShuf[k] = tmp;
			}
		}

		int16 animType = objShuf[i % numObjectTypes];
		setSatelliteAnimMode(_georgeEntityList[_georgeNumSatellites + i], 3, animType);
	}

	_xp->freeMem(objShuf);

	// Allocate asteroid shuffled list...
	int16 numAsteroidTypes = _georgeThresholds[11];
	if (numAsteroidTypes < 1)
		numAsteroidTypes = 1;

	byte *astShuf = (byte *)_xp->allocMem(numAsteroidTypes);
	if (!astShuf) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 i = 0; i < numAsteroidTypes; i++)
		astShuf[i] = (byte)i;

	for (int16 i = 0; i < numAsteroids; i++) {
		if ((i % numAsteroidTypes) == 0) {
			for (int16 j = 0; j < numAsteroidTypes; j++) {
				int16 k = _xp->getRandom(numAsteroidTypes);
				byte tmp = astShuf[j];
				astShuf[j] = astShuf[k];
				astShuf[k] = tmp;
			}
		}

		int16 animType = astShuf[i % numAsteroidTypes];
		setAsteroidAnimMode(_georgeEntityList[_georgeFirstAsteroidIdx + i], 5, animType);
	}

	_xp->freeMem(astShuf);

	// Allocate and load sound list for satellites...
	_georgeSatelliteSoundList = (SoundInfo **)_xp->allocMem(numObjectTypes * sizeof(SoundInfo *));
	if (!_georgeSatelliteSoundList) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 i = 0; i < numObjectTypes; i++) {
		_georgeSatelliteSoundList[i] = new SoundInfo();
		if (!_georgeSatelliteSoundList[i]) {
			termGeorgeLevel(level, variant);
			return false;
		}

		int16 member = variantGroup | ((i + 0x1A) & 0xFF);
		getGeorgeSoundInfo(_georgeBoltLib, member, _georgeSatelliteSoundList[i], 6);
	}

	_georgeSatelliteWait = 1;
	_georgeSatelliteSearchIdx = 0;
	_georgeAsteroidSearchIdx = 0;
	_georgeAsteroidWait = getRandomAsteroidWait();
	_georgeHitSearchIdx = 0;
	_georgeCollectedSatellitesNum = 0;

	return true;
}

void BoltEngine::termGeorgeLevel(int16 level, int16 variant) {
	if (_georgeEntityList) {
		for (int16 i = 0; i < _georgeTotalSatellites; i++) {
			GeorgeEntityState *sat = _georgeEntityList[i];
			if (sat) {
				delete sat;
				_georgeEntityList[i] = nullptr;
			}
		}

		_xp->freeMem(_georgeEntityList);
		_georgeEntityList = nullptr;
	}

	if (_georgeSatelliteSoundList) {
		int32 numObjectTypes = _georgeThresholds[6];
		for (int16 i = 0; i < numObjectTypes; i++) {
			SoundInfo *snd = _georgeSatelliteSoundList[i];
			if (snd) {
				delete snd;
				_georgeSatelliteSoundList[i] = nullptr;
			}
		}

		_xp->freeMem(_georgeSatelliteSoundList);
		_georgeSatelliteSoundList = nullptr;
	}

	if (_georgeAsteroidShuffleTable) {
		_xp->freeMem(_georgeAsteroidShuffleTable);
		_georgeAsteroidShuffleTable = nullptr;
	}

	if (_georgeSatelliteShuffleTable) {
		_xp->freeMem(_georgeSatelliteShuffleTable);
		_georgeSatelliteShuffleTable = nullptr;
	}

	freeBOLTGroup(_georgeBoltLib, (variant * 2) << 8 | 0x200, 1);
	freeBOLTGroup(_georgeBoltLib, (level * 2) << 8 | 0x100, 1);
}

void BoltEngine::swapGeorgeFrameArray() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	uint32 size = _boltCurrentMemberEntry->decompSize;
	uint32 off = 0;

	while ((int32)(size - off) > 0) {
		WRITE_UINT16(data + off + 4, READ_BE_UINT16(data + off + 4));
		resolveIt((uint32 *)(data + off));
		off += 6;
	}
}

void BoltEngine::swapGeorgeHelpEntry() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	uint32 size = _boltCurrentMemberEntry->decompSize;
	uint32 off = 0;
	uint16 palCountOff = (uint16)(off + 0x10);

	while ((int32)(size - off) > 0) {
		WRITE_UINT32(data + off, READ_BE_UINT32(data + off));
		WRITE_UINT32(data + off + 0x08, READ_BE_UINT32(data + off + 0x08));
		WRITE_UINT16(data + off + 0x0C, READ_BE_UINT16(data + off + 0x0C));
		WRITE_UINT16(data + off + 0x0E, READ_BE_UINT16(data + off + 0x0E));
		WRITE_UINT16(data + palCountOff, READ_BE_UINT16(data + palCountOff));
		unpackColors(READ_UINT16(data + palCountOff), data + off + 0x12);
		unpackColors(READ_UINT16(data + palCountOff), data + off + 0x2E);
		resolveIt((uint32 *)(data + off + 0x04));

		off += 0x4A;
		palCountOff += 0x4A;
	}
}

void BoltEngine::swapGeorgeThresholds() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	for (int16 i = 0; i < 12; i++)
		WRITE_UINT16(data + i * 2, READ_BE_UINT16(data + i * 2));
}

int16 BoltEngine::playGeorge() {
	int16 returnCode = 0;
	int16 flyActive = 1;
	int16 winSeq = 0;
	int16 exitGame = 0;
	int16 firstFrame = 1;
	int32 joyY = 0;

	if (_georgeHelpActive) {
		if (!helpGeorge())
			return 7;
	}

	_xp->setInactivityTimer(30);
	_xp->enableController();

	while (!shouldQuit()) {
		// ---- Event loop ----
		while (!shouldQuit()) {
			if (joyY)
				_xp->setInactivityTimer(30);

			uint32 eventData = 0;
			int16 eventType = _xp->getEvent(etEmpty, &eventData);
			if (!eventType)
				break;

			switch (eventType) {
			case etJoystick:
				joyY = (int16)eventData;
				break;
			case etMouseDown:
			case etInactivity:
				if (!winSeq) {
					if (!helpGeorge()) {
						returnCode = 7;
						exitGame = 1;
					}
				}

				break;
			case etSound:
				updateGeorgeSound();
				break;
			default:
				break;
			}
		}

		if (exitGame) {
			if (!winSeq)
				playGeorgeSound(nullptr, nullptr);

			_xp->stopSound();
			return returnCode;
		}

		drawFlyingObjects();
		_xp->updateDisplay();

		// ---- Car physics / input ----
		if (flyActive) {
			if (firstFrame) {
				firstFrame = 0;
				playGeorgeSound(&_georgeSoundCarStartUp, &_georgeSoundCarLoopHi);
			}

			if (joyY) {
				flyActive = 0;
			} else {
				GeorgeEntityState *carSat = _georgeEntityList[_georgeCarIdx];
				int16 carY = (int16)(carSat->y >> 8);

				if (carY > 0xD0)
					carSat->velY = 0xFFFFFA00;
				else if (carY > 0xBC)
					carSat->velY = 0xFFFFFB00;
				else if (carY > 0xA8)
					carSat->velY = 0xFFFFFC00;
				else if (carY > 0x9E)
					carSat->velY = 0xFFFFFD00;
				else if (carY > 0x94)
					carSat->velY = 0xFFFFFE00;
				else if (carY > 0x86)
					carSat->velY = 0xFFFFFF00;
				else {
					carSat->velY = 0;
					flyActive = 0;
				}
			}
		} else {
			if (!winSeq) {
				GeorgeEntityState *carSat = _georgeEntityList[_georgeCarIdx];
				carSat->accelY = (joyY * 3) << 8;

				if (--_georgeSatelliteWait == 0) {
					if (spawnSatellite())
						_georgeSatelliteWait = getRandomSatelliteWait();
					else
						_georgeSatelliteWait = 1;
				}

				if (--_georgeAsteroidWait == 0) {
					if (spawnAsteroid())
						_georgeAsteroidWait = getRandomAsteroidWait();
					else
						_georgeAsteroidWait = 1;
				}
			}
		}

		// ---- Pass 1: animate all satellites ----
		for (int16 i = 0;; i++) {
			GeorgeEntityState *sat = _georgeEntityList[i];
			if (!sat)
				break;
			if (!(sat->flags & 1))
				continue;

			sat->prevX = sat->x;
			sat->prevY = sat->y;
			sat->joyInput = joyY;
			sat->flags &= ~0x06;

			if (--sat->frameCountdown == 0) {
				int16 nextFrame = sat->frameIndex + 1;
				sat->frameIndex = nextFrame;

				byte *entry = sat->animTable + nextFrame * 6;
				if (!READ_UINT32(entry)) {
					sat->frameIndex = 0;
					sat->flags |= 4;
				}

				entry = sat->animTable + sat->frameIndex * 6;
				sat->frameCountdown = READ_UINT16(entry + 0x04);
				sat->flags |= 2;
			}

			if (sat->animMode == 1 || sat->animMode == 2) {
				if (sat->flags & 4)
					setGeorgeAnimMode(sat, 0);
			}
		}

		// ---- Pass 2: physics ----
		for (int16 i = 0;; i++) {
			GeorgeEntityState *sat = _georgeEntityList[i];
			if (!sat)
				break;
			if (!(sat->flags & 1))
				continue;

			switch (sat->animMode) {
			case 0:
			case 1:
			case 2: {
				sat->x += sat->velX;
				sat->y += sat->velY;
				sat->velX += sat->accelX;
				sat->velY += sat->accelY;

				if (sat->x < 0x4000) {
					sat->x = 0x4000;
				} else {
					int32 xMax = (int32)(winSeq ? 0x1C2 : 0x40) << 8;
					if (sat->x > xMax)
						sat->x = xMax;
				}

				if (sat->y < 0x2600) {
					sat->y = 0x2600;
				} else {
					int32 yMax = (int32)(flyActive ? 0x12C : 0xD0) << 8;
					if (sat->y > yMax)
						sat->y = yMax;
				}

				if (sat->velY < (int32)0xFFFFFA00)
					sat->velY = (int32)0xFFFFFA00;
				else if (sat->velY > 0x600)
					sat->velY = 0x600;

				if (!winSeq) {
					if (sat->velY <= 0) {
						if (_georgeSoundToPlay != &_georgeSoundCarLoopHi)
							playGeorgeSound(&_georgeSoundCarLoopHi, &_georgeSoundCarLoopHi);
					} else {
						if (_georgeSoundToPlay != &_georgeSoundCarLoopLo)
							playGeorgeSound(&_georgeSoundCarLoopLo, &_georgeSoundCarLoopLo);
					}
				} else {
					int32 xExit = (int32)0x1C2 << 8;
					if (sat->x >= xExit)
						exitGame = 1;
				}
				break;
			}

			case 3:
			case 5: {
				sat->pathIndex++;
				byte *pathBase = sat->pathTable;
				int16 pathLen = READ_UINT16(pathBase);

				if (sat->pathIndex >= pathLen) {
					sat->flags &= ~1;
				} else {
					byte *entry = pathBase + sat->pathIndex * 4;
					sat->x = (int32)(int16)READ_UINT16(entry + 0x02) << 8;
					sat->y = (int32)(int16)READ_UINT16(entry + 0x04) << 8;
				}
				break;
			}

			default:
				break;
			}
		}

		// ---- Pass 3: collision detection ----
		if (!winSeq) {
			GeorgeEntityState *carSat = _georgeEntityList[_georgeCarIdx];

			byte *carFrameData = getResolvedPtr(carSat->animTable, carSat->frameIndex * 6);
			int16 carX1 = (int16)(carSat->x >> 8) + READ_UINT16(_georgeCollisionRect + 0x00) + READ_UINT16(carFrameData + 0x06);
			int16 carY1 = (int16)(carSat->y >> 8) + READ_UINT16(_georgeCollisionRect + 0x02) + READ_UINT16(carFrameData + 0x08);
			int16 carW = READ_UINT16(_georgeCollisionRect + 0x04);
			int16 carH = READ_UINT16(_georgeCollisionRect + 0x06);

			for (int16 i = 0;; i++) {
				GeorgeEntityState *sat = _georgeEntityList[i];
				if (!sat)
					break;

				if (!(sat->flags & 1))
					continue;

				if (sat->animMode == 3) {
					if (carSat->animMode != 0)
						continue;

					byte *collRect = getResolvedPtr(_georgeSatelliteCollisionRects, sat->variant * 4);
					byte *satFrameData = getResolvedPtr(sat->animTable, sat->frameIndex * 6);

					int16 sx1 = (int16)(sat->x >> 8) + READ_UINT16(collRect + 0x00) + READ_UINT16(satFrameData + 0x06);
					int16 sy1 = (int16)(sat->y >> 8) + READ_UINT16(collRect + 0x02) + READ_UINT16(satFrameData + 0x08);
					int16 sw = READ_UINT16(collRect + 0x04);
					int16 sh = READ_UINT16(collRect + 0x06);

					Common::Rect carRect(carX1, carY1, carX1 + carW, carY1 + carH);
					Common::Rect satRect(sx1, sy1, sx1 + sw, sy1 + sh);
					if (!carRect.intersects(satRect))
						continue;

					setGeorgeAnimMode(carSat, 1);
					setSatelliteAnimMode(sat, 4, 0);
					_georgeCollectedSatellitesNum++;

					if (spawnSatellite())
						_georgeSatelliteWait = getRandomSatelliteWait();
					else
						_georgeSatelliteWait = 1;

					playGeorgeSound(_georgeSatelliteSoundList[sat->variant], nullptr);
				} else if (sat->animMode == 5) {
					if (carSat->animMode != 0 && carSat->animMode != 1)
						continue;

					byte *collRects = getResolvedPtr(_georgeAsteroidCollisionRects, sat->variant * 4);
					byte *satFrameData = getResolvedPtr(sat->animTable, sat->frameIndex * 6);

					int16 sx1 = (int16)(sat->x >> 8) + READ_UINT16(collRects + 0x00) + READ_UINT16(satFrameData + 0x06);
					int16 sy1 = (int16)(sat->y >> 8) + READ_UINT16(collRects + 0x02) + READ_UINT16(satFrameData + 0x08);
					int16 sw = READ_UINT16(collRects + 0x04);
					int16 sh = READ_UINT16(collRects + 0x06);

					Common::Rect carRect(carX1, carY1, carX1 + carW, carY1 + carH);
					Common::Rect astRect(sx1, sy1, sx1 + sw, sy1 + sh);
					if (!carRect.intersects(astRect))
						continue;

					setGeorgeAnimMode(carSat, 2);

					if (confirmAsteroidHitTest())
						_georgeCollectedSatellitesNum--;

					playGeorgeSound(&_georgeSoundCarTumble, nullptr);
					break;
				}
			}
		}

		if (!winSeq) {
			if (_georgeCollectedSatellitesNum >= _georgeThresholds[0]) {
				if (_georgeSoundToPlay &&
					_georgeSoundToPlay->priority > _georgeSoundCarGoesAway.priority)
					continue;

				if (_georgeSoundQueued &&
					_georgeSoundQueued->priority > _georgeSoundCarGoesAway.priority)
					continue;

				bool allDone = true;
				for (int16 i = 0;; i++) {
					GeorgeEntityState *sat = _georgeEntityList[i];
					if (!sat)
						break;

					if (i == _georgeCarIdx)
						continue;

					if (sat->flags & 1) {
						allDone = false;
						break;
					}
				}

				if (allDone) {
					// We've won!
					winSeq = 1;
					returnCode = 0x10;
					GeorgeEntityState *carSat = _georgeEntityList[_georgeCarIdx];
					carSat->accelX = 0x200;
					carSat->accelY = 0;
					playGeorgeSound(&_georgeSoundCarGoesAway, nullptr);
				}
			}
		}
	}

	if (!winSeq)
		playGeorgeSound(nullptr, nullptr);

	_xp->stopSound();
	return returnCode;
}

int16 BoltEngine::helpGeorge() {
	byte *firstObj = getResolvedPtr(_georgeHelpObjects, 0);
	byte *firstInfo = getResolvedPtr(firstObj, 0x04);
	int16 curX = READ_UINT16(firstInfo + 0x06) + READ_UINT16(firstInfo + 0x0A) - 10;
	int16 curY = READ_UINT16(firstInfo + 0x08) + READ_UINT16(firstInfo + 0x0C) - 10;

	byte *hoveredObj = nullptr;
	int16 exitCode = -1;
	int16 animPlaying = 0;
	int16 audioTick = 0;
	int16 animReady = 0;

	_xp->setFrameRate(0);
	_xp->setInactivityTimer(0);
	_xp->stopSound();

	_georgeSoundCurrent = _georgeSoundToPlay = _georgeSoundNext = _georgeSoundQueued = nullptr;

	drawFlyingObjects();

	for (int16 i = 0;; i++) {
		byte *obj = getResolvedPtr(_georgeHelpObjects, i * 4);
		if (!obj)
			break;

		byte *sprite = getResolvedPtr(obj, 0x04);
		displayPic(sprite, 0, 0, 0);
	}

	_xp->updateDisplay();

	for (int16 i = 0;; i++) {
		byte *obj = getResolvedPtr(_georgeHelpObjects, i * 4);
		if (!obj)
			break;

		_xp->getPalette(READ_UINT16(obj + 0x0C), READ_UINT16(obj + 0x10), obj + 0x2E);
		_xp->getPalette(READ_UINT16(obj + 0x0E), READ_UINT16(obj + 0x10), obj + 0x12);

		WRITE_UINT32(obj + 0x08, READ_UINT32(obj + 0x08) & ~1u);
		if (READ_UINT32(obj) == 2)
			hoveredObj = obj;
	}

	// Show cursor, highlight "PLAY" object
	_xp->setCursorPos(curX, curY);
	_xp->setCursorColor(0, 0, 0xFF);
	_xp->showCursor();
	_georgeActiveHelpObject = getResolvedPtr(_georgeHelpObjects, 0);
	hiliteGeorgeHelpObject(_georgeActiveHelpObject, 1);

	// ---- Event loop ----
	while (!shouldQuit()) {
		if (animPlaying) {
			audioTick = maintainAudioPlay(audioTick);
			if (!audioTick) {
				bool sameAsHover = (hoveredObj == _georgeActiveHelpObject);
				hiliteGeorgeHelpObject(hoveredObj, sameAsHover ? 1 : 0);
				animPlaying = 0;
			}
			audioTick = 0;
		}

		uint32 eventData = 0;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);

		switch (eventType) {
		case etTimer: {
			if (!_georgePrevActiveHelpObject)
				break;

			if (_georgeHelpTimer != eventData)
				break;

			_georgeHelpTimer = _xp->startTimer(500);
			
				bool lit = !(READ_UINT32(_georgePrevActiveHelpObject + 0x08) & 1u);
				hiliteGeorgeHelpObject(_georgePrevActiveHelpObject, lit ? 1 : 0);
			
			break;
		}

		case etMouseMove: {
			curX = (int16)((int32)eventData >> 16);
			curY = (int16)eventData;

			byte *hit = nullptr;
			for (int16 i = 0;; i++) {
				byte *obj = getResolvedPtr(_georgeHelpObjects, i * 4);
				if (!obj)
					break;

				byte *info = getResolvedPtr(obj, 0x04);
				int16 ox = READ_UINT16(info + 0x06);
				int16 ow = READ_UINT16(info + 0x0A);
				int16 oy = READ_UINT16(info + 0x08);
				int16 oh = READ_UINT16(info + 0x0C);
				if (curX > ox && curX < ox + ow &&
					curY > oy && curY < oy + oh) {
					hit = obj;
					break;
				}
			}

			if (hit == _georgeActiveHelpObject)
				break;

			if (_georgeActiveHelpObject) {
				if (_georgeActiveHelpObject != _georgePrevActiveHelpObject) {
					bool keepLit = (_georgeActiveHelpObject == hoveredObj && animPlaying);
					if (!keepLit)
						hiliteGeorgeHelpObject(_georgeActiveHelpObject, 0);
				}
			}

			_georgeActiveHelpObject = hit;
			if (!hit)
				break;

			if (hit == _georgePrevActiveHelpObject)
				break;

			hiliteGeorgeHelpObject(_georgeActiveHelpObject, 1);
			break;
		}

		case etMouseDown: {
			animReady = 0;

			if (animPlaying) {
				if (_georgeHelpTimer) {
					_xp->killTimer(_georgeHelpTimer);
					_georgeHelpTimer = 0;
				}

				bool hovLit = (hoveredObj == _georgeActiveHelpObject);
				hiliteGeorgeHelpObject(hoveredObj, hovLit ? 1 : 0);
				bool actLit = (_georgePrevActiveHelpObject == _georgeActiveHelpObject);
				hiliteGeorgeHelpObject(_georgePrevActiveHelpObject, actLit ? 1 : 0);

				_georgePrevActiveHelpObject = nullptr;
				stopAnimation();
				animPlaying = 0;
				animReady = 1;
			}

			if (!_georgeActiveHelpObject)
				break;

			switch (READ_UINT16(_georgeActiveHelpObject)) {
			case 0:
			case 1:
				if (READ_UINT16(_georgeActiveHelpObject) == 1) {
					_georgeHelpActive = 0;
				}

				exitCode = READ_UINT16(_georgeActiveHelpObject);
				break;
			case 2:
				if (animPlaying)
					break;

				if (animReady)
					break;

				if (startAnimation(_rtfHandle, 30)) {
					_georgeHelpStep = 0;
					animPlaying = 1;
				}

				break;
			}

			break;
		}

		case etSound:
			audioTick = 1;
			break;

		case etTrigger:
			if (animPlaying)
				advanceHelpAnimation();
			break;

		default:
			break;
		}

		if (exitCode != -1) {
			_xp->hideCursor();
			_xp->enableController();

			// Highlight hovered object, unhighlight the rest...
			for (int16 i = 0;; i++) {
				byte *obj = getResolvedPtr(_georgeHelpObjects, i * 4);
				if (!obj)
					break;

				bool isHovered = (obj == _georgeActiveHelpObject);
				hiliteGeorgeHelpObject(obj, isHovered ? 1 : 0);
			}
			_xp->updateDisplay();

			// Unhighlight all...
			for (int16 i = 0;; i++) {
				byte *obj = getResolvedPtr(_georgeHelpObjects, i * 4);
				if (!obj)
					break;

				hiliteGeorgeHelpObject(obj, 0);
			}

			drawFlyingObjects();
			_xp->updateDisplay();
			_xp->setFrameRate(12);
			_xp->setInactivityTimer(30);
			updateGeorgeSound();

			return exitCode;
		}
	}

	return exitCode;
}

void BoltEngine::hiliteGeorgeHelpObject(byte *entry, int16 highlight) {
	if (!entry)
		return;

	if (highlight) {
		_xp->setPalette(READ_UINT16(entry + 0x10), READ_UINT16(entry + 0x0C), entry + 0x12);
		WRITE_UINT32(entry + 0x08, READ_UINT32(entry + 0x08) | 1);
	} else {
		_xp->setPalette(READ_UINT16(entry + 0x10), READ_UINT16(entry + 0x0C), entry + 0x2E);
		WRITE_UINT32(entry + 0x08, READ_UINT32(entry + 0x08) & ~1);
	}
}

void BoltEngine::advanceHelpAnimation() {
	if (_georgeHelpStep < 0 || _georgeHelpStep >= 4)
		return;

	bool isHovered = (_georgePrevActiveHelpObject == _georgeActiveHelpObject);
	hiliteGeorgeHelpObject(_georgePrevActiveHelpObject, isHovered ? 1 : 0);

	byte *newActive;
	if (!(_georgeHelpStep & 1)) {
		newActive = getResolvedPtr(_georgeHelpSequence, (_georgeHelpStep >> 1) * 4);
	} else {
		newActive = nullptr;
	}

	_georgePrevActiveHelpObject = newActive;
	_georgeHelpTimer = 0;

	hiliteGeorgeHelpObject(_georgePrevActiveHelpObject, 1);

	if (!(_georgeHelpStep & 1)) {
		_georgeHelpTimer = _xp->startTimer(500);
	} else {
		if (_georgeHelpTimer) {
			_xp->killTimer(_georgeHelpTimer);
			_georgeHelpTimer = 0;
		}
	}

	_georgeHelpStep++;
}
bool BoltEngine::spawnSatellite() {
	// Count active satellites (type 3)...
	int16 activeCount = 0;
	for (int16 i = 0;; i++) {
		GeorgeEntityState *sat = _georgeEntityList[i];
		if (!sat)
			break;

		if (sat->animMode == 3 && (sat->flags & 1))
			activeCount++;
	}

	// Too many active, no need to spawn more...
	if (activeCount >= 6)
		return false;

	// Find next inactive satellite slot...
	int16 startIdx = _georgeSatelliteSearchIdx;
	int16 idx = startIdx;
	bool wrapped = false;
	GeorgeEntityState *candidate = nullptr;

	while (true) {
		GeorgeEntityState *s = _georgeEntityList[idx];
		if (!s) {
			wrapped = true;
			idx = -1;
		} else if (s->animMode == 3 && !(s->flags & 1)) {
			candidate = s;
			break;
		}

		idx++;
		if (idx == startIdx && wrapped)
			return false;
	}

	_georgeSatelliteSearchIdx = idx + 1;

	// Get a shuffle-table row for this asteroid...
	int16 row = getRandomAsteroidRow();
	int16 numCols = _georgeThresholds[4];
	int16 rowBase = (numCols + 1) * row;
	int16 colIdx = _georgeSatelliteShuffleTable[rowBase];
	byte animIdx = _georgeSatelliteShuffleTable[rowBase + colIdx + 1];

	// Get the motion path...
	byte *rowPaths = getResolvedPtr(_georgeSatellitePaths, row * 4);
	byte *path = getResolvedPtr(rowPaths, animIdx * 4);

	candidate->pathTable = path;

	colIdx++;
	_georgeSatelliteShuffleTable[rowBase] = (byte)colIdx;
	if (colIdx >= numCols)
		_georgeSatelliteShuffleTable[rowBase] = 0;

	// Set initial position from pic's embedded coords...
	candidate->pathIndex = 0;
	candidate->x = (int32)((int16)READ_UINT16(path + 0x02) << 8);
	candidate->y = (int32)((int16)READ_UINT16(path + 0x04) << 8);

	candidate->flags |= 1;

	return true;
}

int16 BoltEngine::getRandomSatelliteWait() {
	int16 range = _georgeThresholds[2];
	int16 topY = _georgeThresholds[1];
	return _xp->getRandom(range) + topY;
}

int16 BoltEngine::getRandomAsteroidRow() {
	GeorgeEntityState *carSat = _georgeEntityList[_georgeCarIdx];
	int16 carY = carSat->y >> 8;

	// Find which row the car is in by scanning frame Y thresholds...
	int16 numRows = _georgeThresholds[3];
	int16 carRow = 0;
	int16 *thresholds = _georgeSatelliteThresholds;

	for (int16 i = 0; i < numRows; i++) {
		if (thresholds[i] > carY)
			break;

		carRow = i + 1;
	}

	// Pick a random row, retrying if too close to car's row...
	int16 exclusionBand = _georgeThresholds[5];
	int16 randRow;
	do {
		randRow = _xp->getRandom(_georgeThresholds[3]);
	} while (randRow >= (carRow - exclusionBand) && randRow < (carRow + exclusionBand));

	return randRow;
}

bool BoltEngine::confirmAsteroidHitTest() {
	GeorgeEntityState *candidate = nullptr;
	int16 startIdx = _georgeHitSearchIdx;
	int16 idx = startIdx;
	bool wrapped = false;

	while (true) {
		GeorgeEntityState *s = _georgeEntityList[idx];
		if (!s) {
			wrapped = true;
			idx = -1;
		} else if (s->animMode == 4 && (s->flags & 1)) {
			candidate = s;
			break;
		}

		idx++;
		if (idx == startIdx && wrapped)
			return false;
	}

	_georgeHitSearchIdx = idx + 1;

	setSatelliteAnimMode(candidate, 3, candidate->variant);

	return true;
}

bool BoltEngine::spawnAsteroid() {
	// Count active asteroids (type 5)...
	int16 activeCount = 0;
	for (int16 i = 0;; i++) {
		GeorgeEntityState *sat = _georgeEntityList[i];
		if (!sat)
			break;

		if (sat->animMode == 5 && (sat->flags & 1))
			activeCount++;
	}

	// No need to spawn more...
	if (activeCount >= 1)
		return false;

	// Find next inactive asteroid slot...
	int16 startIdx = _georgeAsteroidSearchIdx;
	int16 idx = startIdx;
	bool wrapped = false;
	GeorgeEntityState *candidate = nullptr;

	while (true) {
		GeorgeEntityState *s = _georgeEntityList[idx];
		if (!s) {
			wrapped = true;
			idx = -1;
		} else if (s->animMode == 5 && !(s->flags & 1)) {
			candidate = s;
			break;
		}

		idx++;
		if (idx == startIdx && wrapped)
			return false;
	}

	_georgeAsteroidSearchIdx = idx + 1;

	// Get shuffle table row for this asteroid...
	int16 row = getAsteroidRow();
	int16 numCols = _georgeThresholds[10];
	int16 rowBase = (numCols + 1) * row;
	int16 colIdx = _georgeAsteroidShuffleTable[rowBase];
	byte animIdx = _georgeAsteroidShuffleTable[rowBase + colIdx + 1];

	// Assign motion path...
	byte *rowPaths = getResolvedPtr(_georgeAsteroidPaths, row * 4);
	byte *path = getResolvedPtr(rowPaths, animIdx * 4);

	candidate->pathTable = path;
	colIdx++;
	_georgeAsteroidShuffleTable[rowBase] = (byte)colIdx;
	if (colIdx >= numCols)
		_georgeAsteroidShuffleTable[rowBase] = 0;

	// Set initial position from pic's embedded coords...
	candidate->pathIndex = 0;
	candidate->x = (int32)(int16)READ_UINT16(path + 0x02) << 8;
	candidate->y = (int32)(int16)READ_UINT16(path + 0x04) << 8;

	candidate->flags |= 1;

	return true;
}

int16 BoltEngine::getRandomAsteroidWait() {
    int16 base  = _georgeThresholds[7];
    int16 range = _georgeThresholds[8];
    return _xp->getRandom(range) + base;
}

int16 BoltEngine::getAsteroidRow() {
	GeorgeEntityState *carSat = _georgeEntityList[_georgeCarIdx];
	int16 carY = carSat->y >> 8;

	// Find which row the car is in by scanning frame Y thresholds...
	int16 numRows = _georgeThresholds[9];
	int16 row = 0;
	int16 *thresholds = _georgeAsteroidThresholds;

	for (int16 i = 0; i < numRows; i++) {
		if (thresholds[i] > carY)
			break;

		row = i + 1;
	}

	return row;
}

void BoltEngine::setGeorgeAnimMode(GeorgeEntityState *entity, int16 mode) {
	entity->animMode = mode;
	entity->variant = 0;
	entity->animTable = _georgeCarPics[mode];
	entity->frameIndex = 0;
	entity->frameCountdown = READ_UINT16(entity->animTable + 0x04);
}

void BoltEngine::setSatelliteAnimMode(GeorgeEntityState *entity, int16 mode, int16 variant) {
	if (mode == 4) {
		// Deactivate...
		entity->animMode = mode;
		entity->flags &= ~1;
		return;
	}

	entity->animMode = mode;
	entity->variant = variant;
	entity->animTable = getResolvedPtr(_georgeSatelliteGfx, variant * 4);
	entity->frameIndex = 0;
	entity->frameCountdown = READ_UINT16(entity->animTable + 0x04);
}

void BoltEngine::setAsteroidAnimMode(GeorgeEntityState *entity, int16 mode, int16 variant) {
	entity->animMode = mode;
	entity->variant = variant;
	entity->animTable = getResolvedPtr(_georgeAsteroidGfx, variant * 4);
	entity->frameIndex = 0;
	entity->frameCountdown = READ_UINT16(entity->animTable + 0x04);
}

void BoltEngine::drawFlyingObjects() {
	_xp->fillDisplay(0, stFront);

	for (int16 i = 0;; i++) {
		GeorgeEntityState *sat = _georgeEntityList[i];
		if (!sat)
			break;

		if (!(sat->flags & 1))
			continue;

		int16 x = sat->x >> 8;
		int16 y = sat->y >> 8;

		byte *pic = getResolvedPtr(sat->animTable, sat->frameIndex * 6);

		displayPic(pic, x, y, stFront);
	}
}

void BoltEngine::getGeorgeSoundInfo(BOLTLib *boltLib, int16 member, SoundInfo *outInfo, byte priority) {
	outInfo->data = memberAddr(boltLib, member);
	outInfo->size = memberSize(boltLib, member);
	outInfo->priority = priority;
	outInfo->channel = _georgeSoundChannelCounter++;
}

void BoltEngine::playGeorgeSound(SoundInfo *newSound, SoundInfo *nextSound) {
	// If a queued sound exists and has higher priority than new sound, don't replace it...
	if (newSound && _georgeSoundQueued) {
		if (_georgeSoundQueued->priority > newSound->priority)
			return;
	}

	// Queue the new sounds...
	_georgeSoundQueued = newSound;
	_georgeSoundNext = nextSound;

	// If currently playing sound has higher priority than new one, keep it...
	if (_georgeSoundToPlay && newSound) {
		if (_georgeSoundToPlay->priority > _georgeSoundQueued->priority) {
			// If something is queued but nothing playing, kick off playback now...
			if (_georgeSoundQueued && !_georgeSoundToPlay)
				updateGeorgeSound();

			return;
		}
	}

	// New sound wins...
	_xp->stopSound();
	_georgeSoundCurrent = nullptr;
	_georgeSoundToPlay = nullptr;

	// If something is queued but nothing playing, kick off playback now...
	if (_georgeSoundQueued && !_georgeSoundToPlay)
		updateGeorgeSound();
}

void BoltEngine::updateGeorgeSound() {
	int16 playBoth = 0;

	if (_georgeSoundCurrent != nullptr) {
		// Current sound still set, promote directly to play slot...
		_georgeSoundToPlay = _georgeSoundCurrent;
	} else if (_georgeSoundQueued != nullptr) {
		// Nothing current, promote queued sound...
		_georgeSoundToPlay = _georgeSoundQueued;
		_georgeSoundCurrent = _georgeSoundNext;

		if (_georgeSoundNext != nullptr)
			playBoth = 1;

		_georgeSoundNext = nullptr;
		_georgeSoundQueued = nullptr;
	} else {
		_georgeSoundToPlay = nullptr;
	}

	if (_georgeSoundToPlay == nullptr)
		return;

	// Play primary sound...
	_xp->playSound(_georgeSoundToPlay->data, _georgeSoundToPlay->size, 22050);

	if (!playBoth)
		return;

	// Queue secondary sound twice...
	_xp->playSound(_georgeSoundCurrent->data, _georgeSoundCurrent->size, 22050);
	_xp->playSound(_georgeSoundCurrent->data, _georgeSoundCurrent->size, 22050);
}

} // End of namespace Bolt
