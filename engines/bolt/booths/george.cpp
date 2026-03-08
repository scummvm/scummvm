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
	if (!openBOLTLib(&g_georgeBoltLib, &g_georgeBoltCallbacks, assetPath("george.blt")))
		return false;

	if (!getBOLTGroup(g_georgeBoltLib, 0, 1))
		return false;

	g_georgeBgPic = memberAddr(g_georgeBoltLib, (g_displayMode != 0) ? 1 : 0);
	g_georgeHelpObjects = memberAddr(g_georgeBoltLib, 0x1F);
	g_georgeHelpSequence = memberAddr(g_georgeBoltLib, 0x20);

	g_georgeCarPics[0] = memberAddr(g_georgeBoltLib, 0x10);
	g_georgeCarPics[1] = memberAddr(g_georgeBoltLib, 0x11);
	g_georgeCarPics[2] = memberAddr(g_georgeBoltLib, 0x12);
	g_georgeCollisionRect = memberAddr(g_georgeBoltLib, 0x13);

	getGeorgeSoundInfo(g_georgeBoltLib, 0x16, &g_georgeSoundCarTumble, 6);
	getGeorgeSoundInfo(g_georgeBoltLib, 0x15, &g_georgeSoundCarLoopHi, 0);
	getGeorgeSoundInfo(g_georgeBoltLib, 0x14, &g_georgeSoundCarLoopLo, 0);
	getGeorgeSoundInfo(g_georgeBoltLib, 0x17, &g_georgeSoundCarStartUp, 3);
	getGeorgeSoundInfo(g_georgeBoltLib, 0x18, &g_georgeSoundCarGoesAway, 3);

	g_georgeSoundCurrent = nullptr;
	g_georgeSoundToPlay = nullptr;
	g_georgeSoundNext = nullptr;
	g_georgeSoundQueued = nullptr;
	g_georgeSoundChannelCounter = 0;

	// Load save data...
	if (!vLoad(&g_georgeSaveData, g_georgeSaveFileName)) {
		g_georgeSaveData[0] = 0;
		g_georgeSaveData[1] = 0;
		g_georgeSaveData[2] = 0;
	}

	if (!initGeorgeLevel(g_georgeSaveData[1], g_georgeSaveData[2])) {
		freeBOLTGroup(g_georgeBoltLib, 0, 1);
		return false;
	}

	// Initialize satellites from list...
	g_georgeCollectedSatellitesNum = 0;
	while (g_georgeCollectedSatellitesNum < g_georgeSaveData[0]) {
		setSatelliteAnimMode(g_georgeEntityList[g_georgeNumSatellites + g_georgeCollectedSatellitesNum], 4, 0);
		g_georgeCollectedSatellitesNum++;
	}

	// Drain timer events...
	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	// Display palette and background to front...
	displayColors(g_georgePalette, stBack, 0);
	displayPic(g_georgeBgPic, g_displayX, g_displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);

	// Display palette to both surfaces...
	displayColors(g_georgePalette, stFront, 0);
	displayColors(g_georgePalette, stBack, 1);

	// Display background to back surface...
	displayPic(g_georgeBgPic, g_displayX, g_displayY, stBack);

	drawFlyingObjects();

	_xp->updateDisplay();
	_xp->startCycle(g_georgePalCycleSpecs);
	_xp->setFrameRate(12);

	return true;
}

void BoltEngine::cleanUpGeorge() {
	// Save current progress...
	int16 savedLevel = g_georgeSaveData[1];
	int16 savedVariant = g_georgeSaveData[2];

	g_georgeSaveData[0] = g_georgeCollectedSatellitesNum;

	// Check if collection threshold for current level was met...
	if (g_georgeCollectedSatellitesNum >= g_georgeThresholds[0]) {
		// Advance level...
		g_georgeSaveData[1]++;
		if (g_georgeSaveData[1] >= 10)
			g_georgeSaveData[1] = 9;

		// Advance variant...
		g_georgeSaveData[2]++;
		if (g_georgeSaveData[2] >= 10)
			g_georgeSaveData[2] = 0;

		// Reset collected count for next level...
		g_georgeSaveData[0] = 0;
	}

	vSave(g_georgeSaveData, sizeof(g_georgeSaveData), g_georgeSaveFileName);

	_xp->stopCycle();

	termGeorgeLevel(savedLevel, savedVariant);

	freeBOLTGroup(g_georgeBoltLib, 0, 1);
	closeBOLTLib(&g_georgeBoltLib);

	_xp->setFrameRate(0);
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
}

bool BoltEngine::initGeorgeLevel(int16 level, int16 variant) {
	int16 levelGroup = (level * 2) << 8 | 0x100;
	int16 variantGroup = (variant * 2) << 8 | 0x200;

	if (!getBOLTGroup(g_georgeBoltLib, levelGroup, 1))
		return false;

	if (!getBOLTGroup(g_georgeBoltLib, variantGroup, 1)) {
		freeBOLTGroup(g_georgeBoltLib, levelGroup, 1);
		return false;
	}

	g_georgeThresholds = (int16 *)memberAddr(g_georgeBoltLib, levelGroup);
	g_georgePalette = memberAddr(g_georgeBoltLib, variantGroup);
	g_georgePalCycleRawData = memberAddr(g_georgeBoltLib, variantGroup + 1);
	boltCycleToXPCycle(g_georgePalCycleRawData, g_georgePalCycleSpecs);

	g_georgeSatelliteGfx = memberAddr(g_georgeBoltLib, variantGroup + 0x12);
	g_georgeAsteroidGfx = memberAddr(g_georgeBoltLib, variantGroup + 0x26);
	g_georgeSatelliteThresholds = (int16 *)memberAddr(g_georgeBoltLib, levelGroup + 0x01);
	g_georgeAsteroidThresholds = (int16 *)memberAddr(g_georgeBoltLib, levelGroup + 0x02);
	g_georgeSatelliteCollisionRects = memberAddr(g_georgeBoltLib, variantGroup + 0x19);
	g_georgeAsteroidCollisionRects = memberAddr(g_georgeBoltLib, variantGroup + 0x2A);
	g_georgeSatellitePaths = memberAddr(g_georgeBoltLib, levelGroup + 0x27);
	g_georgeAsteroidPaths = memberAddr(g_georgeBoltLib, levelGroup + 0x3C);

	// Allocate and build asteroid shuffle table...
	int16 numAsteroidRows = g_georgeThresholds[3];
	int16 numAsteroidCols = g_georgeThresholds[4];
	g_georgeSatelliteShuffleTable = (byte *)_xp->allocMem((uint32)(numAsteroidCols + 1) * (numAsteroidRows));
	if (!g_georgeSatelliteShuffleTable) {
		termGeorgeLevel(level, variant);
		return false;
	}

	// Fill each row with sequential values 0..numAsteroidCols, then shuffle...
	for (int16 row = 0; row < numAsteroidRows; row++) {
		int16 rowBase = (numAsteroidCols + 1) * row;
		g_georgeSatelliteShuffleTable[rowBase] = 0;

		for (int16 col = 0; col < numAsteroidCols; col++)
			g_georgeSatelliteShuffleTable[rowBase + col + 1] = (byte)col;

		// Fisher-Yates shuffle...
		for (int16 i = 0; i < numAsteroidCols; i++) {
			int16 j = _xp->getRandom(numAsteroidCols);
			byte tmp = g_georgeSatelliteShuffleTable[rowBase + i + 1];
			g_georgeSatelliteShuffleTable[rowBase + i + 1] = g_georgeSatelliteShuffleTable[rowBase + j + 1];
			g_georgeSatelliteShuffleTable[rowBase + j + 1] = tmp;
		}
	}

	// Allocate and build satellite shuffle table...
	int16 numSatelliteRows = g_georgeThresholds[9];
	int16 numSatelliteCols = g_georgeThresholds[10];
	g_georgeAsteroidShuffleTable = (byte *)_xp->allocMem((uint32)(numSatelliteCols + 1) * (numSatelliteRows));
	if (!g_georgeAsteroidShuffleTable) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 row = 0; row < numSatelliteRows; row++) {
		int16 rowBase = (numSatelliteCols + 1) * row;
		g_georgeAsteroidShuffleTable[rowBase] = 0;

		for (int16 col = 0; col < numSatelliteCols; col++)
			g_georgeAsteroidShuffleTable[rowBase + col + 1] = (byte)col;

		for (int16 i = 0; i < numSatelliteCols; i++) {
			int16 j = _xp->getRandom(numSatelliteCols);
			byte tmp = g_georgeAsteroidShuffleTable[rowBase + i + 1];
			g_georgeAsteroidShuffleTable[rowBase + i + 1] = g_georgeAsteroidShuffleTable[rowBase + j + 1];
			g_georgeAsteroidShuffleTable[rowBase + j + 1] = tmp;
		}
	}

	// Compute satellite list indices...
	g_georgeNumSatellites = 0;
	int16 numSatellites = g_georgeThresholds[0];
	g_georgeFirstAsteroidIdx = numSatellites;

	int16 numAsteroids = g_georgeThresholds[11];
	if (numAsteroids < 1)
		numAsteroids = 1;

	g_georgeCarIdx = g_georgeFirstAsteroidIdx + numAsteroids;
	g_georgeTotalSatellites = g_georgeCarIdx + 1;

	// Allocate satellite list...
	g_georgeEntityList = (GeorgeEntityState **)_xp->allocMem((g_georgeTotalSatellites + 1) * sizeof(GeorgeEntityState *));
	if (!g_georgeEntityList) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 i = 0; i < g_georgeTotalSatellites; i++) {
		g_georgeEntityList[i] = new GeorgeEntityState();
		if (!g_georgeEntityList[i]) {
			termGeorgeLevel(level, variant);
			return false;
		}

		g_georgeEntityList[i]->flags = 0;
	}

	g_georgeEntityList[g_georgeTotalSatellites] = nullptr;

	GeorgeEntityState *carSat = g_georgeEntityList[g_georgeCarIdx];
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

	int32 numObjectTypes = g_georgeThresholds[6];
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
		setSatelliteAnimMode(g_georgeEntityList[g_georgeNumSatellites + i], 3, animType);
	}

	_xp->freeMem(objShuf);

	// Allocate asteroid shuffled list...
	int16 numAsteroidTypes = g_georgeThresholds[11];
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
		setAsteroidAnimMode(g_georgeEntityList[g_georgeFirstAsteroidIdx + i], 5, animType);
	}

	_xp->freeMem(astShuf);

	// Allocate and load sound list for satellites...
	g_georgeSatelliteSoundList = (SoundInfo **)_xp->allocMem(numObjectTypes * sizeof(SoundInfo *));
	if (!g_georgeSatelliteSoundList) {
		termGeorgeLevel(level, variant);
		return false;
	}

	for (int16 i = 0; i < numObjectTypes; i++) {
		g_georgeSatelliteSoundList[i] = new SoundInfo();
		if (!g_georgeSatelliteSoundList[i]) {
			termGeorgeLevel(level, variant);
			return false;
		}

		int16 member = variantGroup | ((i + 0x1A) & 0xFF);
		getGeorgeSoundInfo(g_georgeBoltLib, member, g_georgeSatelliteSoundList[i], 6);
	}

	g_georgeSatelliteWait = 1;
	g_georgeSatelliteSearchIdx = 0;
	g_georgeAsteroidSearchIdx = 0;
	g_georgeAsteroidWait = getRandomAsteroidWait();
	g_georgeHitSearchIdx = 0;
	g_georgeCollectedSatellitesNum = 0;

	return true;
}

void BoltEngine::termGeorgeLevel(int16 level, int16 variant) {
	if (g_georgeEntityList) {
		for (int16 i = 0; i < g_georgeTotalSatellites; i++) {
			GeorgeEntityState *sat = g_georgeEntityList[i];
			if (sat) {
				delete sat;
				g_georgeEntityList[i] = nullptr;
			}
		}

		_xp->freeMem(g_georgeEntityList);
		g_georgeEntityList = nullptr;
	}

	if (g_georgeSatelliteSoundList) {
		int32 numObjectTypes = g_georgeThresholds[6];
		for (int16 i = 0; i < numObjectTypes; i++) {
			SoundInfo *snd = g_georgeSatelliteSoundList[i];
			if (snd) {
				delete snd;
				g_georgeSatelliteSoundList[i] = nullptr;
			}
		}

		_xp->freeMem(g_georgeSatelliteSoundList);
		g_georgeSatelliteSoundList = nullptr;
	}

	if (g_georgeAsteroidShuffleTable) {
		_xp->freeMem(g_georgeAsteroidShuffleTable);
		g_georgeAsteroidShuffleTable = nullptr;
	}

	if (g_georgeSatelliteShuffleTable) {
		_xp->freeMem(g_georgeSatelliteShuffleTable);
		g_georgeSatelliteShuffleTable = nullptr;
	}

	freeBOLTGroup(g_georgeBoltLib, (variant * 2) << 8 | 0x200, 1);
	freeBOLTGroup(g_georgeBoltLib, (level * 2) << 8 | 0x100, 1);
}

void BoltEngine::swapGeorgeFrameArray() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	uint32 size = g_boltCurrentMemberEntry->decompSize;
	uint32 off = 0;

	while ((int32)(size - off) > 0) {
		WRITE_UINT16(data + off + 4, READ_BE_UINT16(data + off + 4));
		resolveIt((uint32 *)(data + off));
		off += 6;
	}
}

void BoltEngine::swapGeorgeHelpEntry() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	uint32 size = g_boltCurrentMemberEntry->decompSize;
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
	byte *data = g_boltCurrentMemberEntry->dataPtr;
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

	if (g_georgeHelpActive) {
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
				playGeorgeSound(&g_georgeSoundCarStartUp, &g_georgeSoundCarLoopHi);
			}

			if (joyY) {
				flyActive = 0;
			} else {
				GeorgeEntityState *carSat = g_georgeEntityList[g_georgeCarIdx];
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
				GeorgeEntityState *carSat = g_georgeEntityList[g_georgeCarIdx];
				carSat->accelY = (joyY * 3) << 8;

				if (--g_georgeSatelliteWait == 0) {
					if (spawnSatellite())
						g_georgeSatelliteWait = getRandomSatelliteWait();
					else
						g_georgeSatelliteWait = 1;
				}

				if (--g_georgeAsteroidWait == 0) {
					if (spawnAsteroid())
						g_georgeAsteroidWait = getRandomAsteroidWait();
					else
						g_georgeAsteroidWait = 1;
				}
			}
		}

		// ---- Pass 1: animate all satellites ----
		for (int16 i = 0;; i++) {
			GeorgeEntityState *sat = g_georgeEntityList[i];
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
			GeorgeEntityState *sat = g_georgeEntityList[i];
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
						if (g_georgeSoundToPlay != &g_georgeSoundCarLoopHi)
							playGeorgeSound(&g_georgeSoundCarLoopHi, &g_georgeSoundCarLoopHi);
					} else {
						if (g_georgeSoundToPlay != &g_georgeSoundCarLoopLo)
							playGeorgeSound(&g_georgeSoundCarLoopLo, &g_georgeSoundCarLoopLo);
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
			GeorgeEntityState *carSat = g_georgeEntityList[g_georgeCarIdx];

			byte *carFrameData = getResolvedPtr(carSat->animTable, carSat->frameIndex * 6);
			int16 carX1 = (int16)(carSat->x >> 8) + READ_UINT16(g_georgeCollisionRect + 0x00) + READ_UINT16(carFrameData + 0x06);
			int16 carY1 = (int16)(carSat->y >> 8) + READ_UINT16(g_georgeCollisionRect + 0x02) + READ_UINT16(carFrameData + 0x08);
			int16 carW = READ_UINT16(g_georgeCollisionRect + 0x04);
			int16 carH = READ_UINT16(g_georgeCollisionRect + 0x06);

			for (int16 i = 0;; i++) {
				GeorgeEntityState *sat = g_georgeEntityList[i];
				if (!sat)
					break;

				if (!(sat->flags & 1))
					continue;

				if (sat->animMode == 3) {
					if (carSat->animMode != 0)
						continue;

					byte *collRect = getResolvedPtr(g_georgeSatelliteCollisionRects, sat->variant * 4);
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
					g_georgeCollectedSatellitesNum++;

					if (spawnSatellite())
						g_georgeSatelliteWait = getRandomSatelliteWait();
					else
						g_georgeSatelliteWait = 1;

					playGeorgeSound(g_georgeSatelliteSoundList[sat->variant], nullptr);
				} else if (sat->animMode == 5) {
					if (carSat->animMode != 0 && carSat->animMode != 1)
						continue;

					byte *collRects = getResolvedPtr(g_georgeAsteroidCollisionRects, sat->variant * 4);
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
						g_georgeCollectedSatellitesNum--;

					playGeorgeSound(&g_georgeSoundCarTumble, nullptr);
					break;
				}
			}
		}

		if (!winSeq) {
			if (g_georgeCollectedSatellitesNum >= g_georgeThresholds[0]) {
				if (g_georgeSoundToPlay &&
					g_georgeSoundToPlay->priority > g_georgeSoundCarGoesAway.priority)
					continue;

				if (g_georgeSoundQueued &&
					g_georgeSoundQueued->priority > g_georgeSoundCarGoesAway.priority)
					continue;

				bool allDone = true;
				for (int16 i = 0;; i++) {
					GeorgeEntityState *sat = g_georgeEntityList[i];
					if (!sat)
						break;

					if (i == g_georgeCarIdx)
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
					GeorgeEntityState *carSat = g_georgeEntityList[g_georgeCarIdx];
					carSat->accelX = 0x200;
					carSat->accelY = 0;
					playGeorgeSound(&g_georgeSoundCarGoesAway, nullptr);
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
	byte *firstObj = getResolvedPtr(g_georgeHelpObjects, 0);
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

	g_georgeSoundCurrent = g_georgeSoundToPlay = g_georgeSoundNext = g_georgeSoundQueued = nullptr;

	drawFlyingObjects();

	for (int16 i = 0;; i++) {
		byte *obj = getResolvedPtr(g_georgeHelpObjects, i * 4);
		if (!obj)
			break;

		byte *sprite = getResolvedPtr(obj, 0x04);
		displayPic(sprite, 0, 0, 0);
	}

	_xp->updateDisplay();

	for (int16 i = 0;; i++) {
		byte *obj = getResolvedPtr(g_georgeHelpObjects, i * 4);
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
	g_georgeActiveHelpObject = getResolvedPtr(g_georgeHelpObjects, 0);
	hiliteGeorgeHelpObject(g_georgeActiveHelpObject, 1);

	// ---- Event loop ----
	while (!shouldQuit()) {
		if (animPlaying) {
			audioTick = maintainAudioPlay(audioTick);
			if (!audioTick) {
				bool sameAsHover = (hoveredObj == g_georgeActiveHelpObject);
				hiliteGeorgeHelpObject(hoveredObj, sameAsHover ? 1 : 0);
				animPlaying = 0;
			}
			audioTick = 0;
		}

		uint32 eventData = 0;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);

		switch (eventType) {
		case etTimer: {
			if (!g_georgePrevActiveHelpObject)
				break;

			if (g_georgeHelpTimer != eventData)
				break;

			g_georgeHelpTimer = _xp->startTimer(500);
			
				bool lit = !(READ_UINT32(g_georgePrevActiveHelpObject + 0x08) & 1u);
				hiliteGeorgeHelpObject(g_georgePrevActiveHelpObject, lit ? 1 : 0);
			
			break;
		}

		case etMouseMove: {
			curX = (int16)((int32)eventData >> 16);
			curY = (int16)eventData;

			byte *hit = nullptr;
			for (int16 i = 0;; i++) {
				byte *obj = getResolvedPtr(g_georgeHelpObjects, i * 4);
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

			if (hit == g_georgeActiveHelpObject)
				break;

			if (g_georgeActiveHelpObject) {
				if (g_georgeActiveHelpObject != g_georgePrevActiveHelpObject) {
					bool keepLit = (g_georgeActiveHelpObject == hoveredObj && animPlaying);
					if (!keepLit)
						hiliteGeorgeHelpObject(g_georgeActiveHelpObject, 0);
				}
			}

			g_georgeActiveHelpObject = hit;
			if (!hit)
				break;

			if (hit == g_georgePrevActiveHelpObject)
				break;

			hiliteGeorgeHelpObject(g_georgeActiveHelpObject, 1);
			break;
		}

		case etMouseDown: {
			animReady = 0;

			if (animPlaying) {
				if (g_georgeHelpTimer) {
					_xp->killTimer(g_georgeHelpTimer);
					g_georgeHelpTimer = 0;
				}

				bool hovLit = (hoveredObj == g_georgeActiveHelpObject);
				hiliteGeorgeHelpObject(hoveredObj, hovLit ? 1 : 0);
				bool actLit = (g_georgePrevActiveHelpObject == g_georgeActiveHelpObject);
				hiliteGeorgeHelpObject(g_georgePrevActiveHelpObject, actLit ? 1 : 0);

				g_georgePrevActiveHelpObject = nullptr;
				stopAnimation();
				animPlaying = 0;
				animReady = 1;
			}

			if (!g_georgeActiveHelpObject)
				break;

			switch (READ_UINT16(g_georgeActiveHelpObject)) {
			case 0:
			case 1:
				if (READ_UINT16(g_georgeActiveHelpObject) == 1) {
					g_georgeHelpActive = 0;
				}

				exitCode = READ_UINT16(g_georgeActiveHelpObject);
				break;
			case 2:
				if (animPlaying)
					break;

				if (animReady)
					break;

				if (startAnimation(g_rtfHandle, 30)) {
					g_georgeHelpStep = 0;
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
				byte *obj = getResolvedPtr(g_georgeHelpObjects, i * 4);
				if (!obj)
					break;

				bool isHovered = (obj == g_georgeActiveHelpObject);
				hiliteGeorgeHelpObject(obj, isHovered ? 1 : 0);
			}
			_xp->updateDisplay();

			// Unhighlight all...
			for (int16 i = 0;; i++) {
				byte *obj = getResolvedPtr(g_georgeHelpObjects, i * 4);
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
	if (g_georgeHelpStep < 0 || g_georgeHelpStep >= 4)
		return;

	bool isHovered = (g_georgePrevActiveHelpObject == g_georgeActiveHelpObject);
	hiliteGeorgeHelpObject(g_georgePrevActiveHelpObject, isHovered ? 1 : 0);

	byte *newActive;
	if (!(g_georgeHelpStep & 1)) {
		newActive = getResolvedPtr(g_georgeHelpSequence, (g_georgeHelpStep >> 1) * 4);
	} else {
		newActive = nullptr;
	}

	g_georgePrevActiveHelpObject = newActive;
	g_georgeHelpTimer = 0;

	hiliteGeorgeHelpObject(g_georgePrevActiveHelpObject, 1);

	if (!(g_georgeHelpStep & 1)) {
		g_georgeHelpTimer = _xp->startTimer(500);
	} else {
		if (g_georgeHelpTimer) {
			_xp->killTimer(g_georgeHelpTimer);
			g_georgeHelpTimer = 0;
		}
	}

	g_georgeHelpStep++;
}
bool BoltEngine::spawnSatellite() {
	// Count active satellites (type 3)...
	int16 activeCount = 0;
	for (int16 i = 0;; i++) {
		GeorgeEntityState *sat = g_georgeEntityList[i];
		if (!sat)
			break;

		if (sat->animMode == 3 && (sat->flags & 1))
			activeCount++;
	}

	// Too many active, no need to spawn more...
	if (activeCount >= 6)
		return false;

	// Find next inactive satellite slot...
	int16 startIdx = g_georgeSatelliteSearchIdx;
	int16 idx = startIdx;
	bool wrapped = false;
	GeorgeEntityState *candidate = nullptr;

	while (true) {
		GeorgeEntityState *s = g_georgeEntityList[idx];
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

	g_georgeSatelliteSearchIdx = idx + 1;

	// Get a shuffle-table row for this asteroid...
	int16 row = getRandomAsteroidRow();
	int16 numCols = g_georgeThresholds[4];
	int16 rowBase = (numCols + 1) * row;
	int16 colIdx = g_georgeSatelliteShuffleTable[rowBase];
	byte animIdx = g_georgeSatelliteShuffleTable[rowBase + colIdx + 1];

	// Get the motion path...
	byte *rowPaths = getResolvedPtr(g_georgeSatellitePaths, row * 4);
	byte *path = getResolvedPtr(rowPaths, animIdx * 4);

	candidate->pathTable = path;

	colIdx++;
	g_georgeSatelliteShuffleTable[rowBase] = (byte)colIdx;
	if (colIdx >= numCols)
		g_georgeSatelliteShuffleTable[rowBase] = 0;

	// Set initial position from pic's embedded coords...
	candidate->pathIndex = 0;
	candidate->x = (int32)((int16)READ_UINT16(path + 0x02) << 8);
	candidate->y = (int32)((int16)READ_UINT16(path + 0x04) << 8);

	candidate->flags |= 1;

	return true;
}

int16 BoltEngine::getRandomSatelliteWait() {
	int16 range = g_georgeThresholds[2];
	int16 topY = g_georgeThresholds[1];
	return _xp->getRandom(range) + topY;
}

int16 BoltEngine::getRandomAsteroidRow() {
	GeorgeEntityState *carSat = g_georgeEntityList[g_georgeCarIdx];
	int16 carY = carSat->y >> 8;

	// Find which row the car is in by scanning frame Y thresholds...
	int16 numRows = g_georgeThresholds[3];
	int16 carRow = 0;
	int16 *thresholds = g_georgeSatelliteThresholds;

	for (int16 i = 0; i < numRows; i++) {
		if (thresholds[i] > carY)
			break;

		carRow = i + 1;
	}

	// Pick a random row, retrying if too close to car's row...
	int16 exclusionBand = g_georgeThresholds[5];
	int16 randRow;
	do {
		randRow = _xp->getRandom(g_georgeThresholds[3]);
	} while (randRow >= (carRow - exclusionBand) && randRow < (carRow + exclusionBand));

	return randRow;
}

bool BoltEngine::confirmAsteroidHitTest() {
	GeorgeEntityState *candidate = nullptr;
	int16 startIdx = g_georgeHitSearchIdx;
	int16 idx = startIdx;
	bool wrapped = false;

	while (true) {
		GeorgeEntityState *s = g_georgeEntityList[idx];
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

	g_georgeHitSearchIdx = idx + 1;

	setSatelliteAnimMode(candidate, 3, candidate->variant);

	return true;
}

bool BoltEngine::spawnAsteroid() {
	// Count active asteroids (type 5)...
	int16 activeCount = 0;
	for (int16 i = 0;; i++) {
		GeorgeEntityState *sat = g_georgeEntityList[i];
		if (!sat)
			break;

		if (sat->animMode == 5 && (sat->flags & 1))
			activeCount++;
	}

	// No need to spawn more...
	if (activeCount >= 1)
		return false;

	// Find next inactive asteroid slot...
	int16 startIdx = g_georgeAsteroidSearchIdx;
	int16 idx = startIdx;
	bool wrapped = false;
	GeorgeEntityState *candidate = nullptr;

	while (true) {
		GeorgeEntityState *s = g_georgeEntityList[idx];
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

	g_georgeAsteroidSearchIdx = idx + 1;

	// Get shuffle table row for this asteroid...
	int16 row = getAsteroidRow();
	int16 numCols = g_georgeThresholds[10];
	int16 rowBase = (numCols + 1) * row;
	int16 colIdx = g_georgeAsteroidShuffleTable[rowBase];
	byte animIdx = g_georgeAsteroidShuffleTable[rowBase + colIdx + 1];

	// Assign motion path...
	byte *rowPaths = getResolvedPtr(g_georgeAsteroidPaths, row * 4);
	byte *path = getResolvedPtr(rowPaths, animIdx * 4);

	candidate->pathTable = path;
	colIdx++;
	g_georgeAsteroidShuffleTable[rowBase] = (byte)colIdx;
	if (colIdx >= numCols)
		g_georgeAsteroidShuffleTable[rowBase] = 0;

	// Set initial position from pic's embedded coords...
	candidate->pathIndex = 0;
	candidate->x = (int32)(int16)READ_UINT16(path + 0x02) << 8;
	candidate->y = (int32)(int16)READ_UINT16(path + 0x04) << 8;

	candidate->flags |= 1;

	return true;
}

int16 BoltEngine::getRandomAsteroidWait() {
    int16 base  = g_georgeThresholds[7];
    int16 range = g_georgeThresholds[8];
    return _xp->getRandom(range) + base;
}

int16 BoltEngine::getAsteroidRow() {
	GeorgeEntityState *carSat = g_georgeEntityList[g_georgeCarIdx];
	int16 carY = carSat->y >> 8;

	// Find which row the car is in by scanning frame Y thresholds...
	int16 numRows = g_georgeThresholds[9];
	int16 row = 0;
	int16 *thresholds = g_georgeAsteroidThresholds;

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
	entity->animTable = g_georgeCarPics[mode];
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
	entity->animTable = getResolvedPtr(g_georgeSatelliteGfx, variant * 4);
	entity->frameIndex = 0;
	entity->frameCountdown = READ_UINT16(entity->animTable + 0x04);
}

void BoltEngine::setAsteroidAnimMode(GeorgeEntityState *entity, int16 mode, int16 variant) {
	entity->animMode = mode;
	entity->variant = variant;
	entity->animTable = getResolvedPtr(g_georgeAsteroidGfx, variant * 4);
	entity->frameIndex = 0;
	entity->frameCountdown = READ_UINT16(entity->animTable + 0x04);
}

void BoltEngine::drawFlyingObjects() {
	_xp->fillDisplay(0, stFront);

	for (int16 i = 0;; i++) {
		GeorgeEntityState *sat = g_georgeEntityList[i];
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
	outInfo->channel = g_georgeSoundChannelCounter++;
}

void BoltEngine::playGeorgeSound(SoundInfo *newSound, SoundInfo *nextSound) {
	// If a queued sound exists and has higher priority than new sound, don't replace it...
	if (newSound && g_georgeSoundQueued) {
		if (g_georgeSoundQueued->priority > newSound->priority)
			return;
	}

	// Queue the new sounds...
	g_georgeSoundQueued = newSound;
	g_georgeSoundNext = nextSound;

	// If currently playing sound has higher priority than new one, keep it...
	if (g_georgeSoundToPlay && newSound) {
		if (g_georgeSoundToPlay->priority > g_georgeSoundQueued->priority) {
			// If something is queued but nothing playing, kick off playback now...
			if (g_georgeSoundQueued && !g_georgeSoundToPlay)
				updateGeorgeSound();

			return;
		}
	}

	// New sound wins...
	_xp->stopSound();
	g_georgeSoundCurrent = nullptr;
	g_georgeSoundToPlay = nullptr;

	// If something is queued but nothing playing, kick off playback now...
	if (g_georgeSoundQueued && !g_georgeSoundToPlay)
		updateGeorgeSound();
}

void BoltEngine::updateGeorgeSound() {
	int16 playBoth = 0;

	if (g_georgeSoundCurrent != nullptr) {
		// Current sound still set, promote directly to play slot...
		g_georgeSoundToPlay = g_georgeSoundCurrent;
	} else if (g_georgeSoundQueued != nullptr) {
		// Nothing current, promote queued sound...
		g_georgeSoundToPlay = g_georgeSoundQueued;
		g_georgeSoundCurrent = g_georgeSoundNext;

		if (g_georgeSoundNext != nullptr)
			playBoth = 1;

		g_georgeSoundNext = nullptr;
		g_georgeSoundQueued = nullptr;
	} else {
		g_georgeSoundToPlay = nullptr;
	}

	if (g_georgeSoundToPlay == nullptr)
		return;

	// Play primary sound...
	_xp->playSound(g_georgeSoundToPlay->data, g_georgeSoundToPlay->size, 22050);

	if (!playBoth)
		return;

	// Queue secondary sound twice...
	_xp->playSound(g_georgeSoundCurrent->data, g_georgeSoundCurrent->size, 22050);
	_xp->playSound(g_georgeSoundCurrent->data, g_georgeSoundCurrent->size, 22050);
}

} // End of namespace Bolt
