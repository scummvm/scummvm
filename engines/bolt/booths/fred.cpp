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

int16 BoltEngine::fredGame(int16 prevBooth) {
	int16 savedTimer = _xp->setInactivityTimer(30);

	if (!initFred()) {
		cleanUpFred();
		return 4; // FredsBooth
	}

	int16 result = playFred();
	cleanUpFred();

	_xp->setInactivityTimer(savedTimer);
	return result;
}

bool BoltEngine::initFred() {
	const char *path = assetPath("fred.blt");

	if (!openBOLTLib(&g_fredBoltLib, &g_fredBoltCallbacks, path))
		return false;

	if (!getBOLTGroup(g_fredBoltLib, 0, 1))
		return false;

	// Load assets...
	g_fredBackground = memberAddr(g_fredBoltLib, (g_displayMode != 0) ? 5 : 4);

	g_fredBalloonString = memberAddr(g_fredBoltLib, 6);

	g_fredFacingLeftRect = memberAddr(g_fredBoltLib, 0);
	g_fredFacingRightRect = memberAddr(g_fredBoltLib, 1);
	g_fredTurningRect = memberAddr(g_fredBoltLib, 2);
	g_fredBalloonRect = memberAddr(g_fredBoltLib, 3);

	g_fredHelpEntries = memberAddr(g_fredBoltLib, 0x38);
	g_fredPlayButton = memberAddr(g_fredBoltLib, 0x39);
	g_fredTimer = 0;

	for (int i = 0; i < 10; i++)
		g_fredSprites[i] = memberAddr(g_fredBoltLib, 0x28 + i);

	getFredSoundInfo(g_fredBoltLib, 0x3A, &g_fredSounds[0]);
	getFredSoundInfo(g_fredBoltLib, 0x3B, &g_fredSounds[1]);
	getFredSoundInfo(g_fredBoltLib, 0x3C, &g_fredSounds[2]);
	getFredSoundInfo(g_fredBoltLib, 0x3D, &g_fredSounds[3]);

	g_fredLoopSound = nullptr;
	g_fredCurrentSound = nullptr;
	g_fredPendingLoop = nullptr;
	g_fredPendingOneShot = nullptr;

	// Init Fred sprite struct...
	g_fredSprite.flags = 1;
	g_fredSprite.direction = 0;
	g_fredSprite.xPos = 0xA000; // Fixed-point coordinate
	g_fredSprite.yPos = 0x1600;	// Fixed-point coordinate
	g_fredSprite.speed = 0x600;
	g_fredSprite.pathTable = nullptr;

	setFredAnimMode(&g_fredSprite, 0);

	// Load save data...
	if (!vLoad(&g_fredSaveData, g_fredSaveFile)) {
		g_fredSaveData[0] = 0; // Balloon catch count
		g_fredSaveData[1] = 0; // Level data group index, increments on level complete, caps at 9
		g_fredSaveData[2] = 0; // Palette group index, increments on level complete, wraps at 10
	}

	if (!initFredLevel(g_fredSaveData[1], g_fredSaveData[2]))
		return false;

	g_fredLevelIndex = 0;
	while (g_fredLevelIndex < g_fredSaveData[0]) {
		FredEntityState *entry = g_fredEntitiesTable[g_fredLevelIndex + 1];
		setFredAnimMode(entry, 10);
		g_fredLevelIndex++;
	}

	// Flush timer events...
	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	// Display background on back and front buffers...
	displayColors(g_fredPalette, stBack, 0);
	displayPic(g_fredBackground, g_displayX, g_displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(g_fredPalette, stFront, 0);
	displayColors(g_fredPalette, stBack, 1);
	displayPic(g_fredBackground, g_displayX, g_displayY, stBack);

	renderFredScene();
	_xp->updateDisplay();

	_xp->startCycle(g_fredCycleSpecs);
	_xp->setFrameRate(15);

	return true;
}

void BoltEngine::cleanUpFred() {
	int16 savedLevel = g_fredSaveData[1]; // Level data group index
	int16 savedPalette = g_fredSaveData[2]; // Palette group index

	g_fredSaveData[0] = g_fredLevelIndex;

	byte *levelPtr = g_fredLevelPtr;
	if (READ_UINT16(levelPtr + 0x0A) <= g_fredLevelIndex) {
		g_fredSaveData[1]++;
		if (g_fredSaveData[1] >= 10)
			g_fredSaveData[1] = 9;

		g_fredSaveData[2]++;
		if (g_fredSaveData[2] >= 10)
			g_fredSaveData[2] = 0;

		g_fredSaveData[0] = 0;
	}

	vSave(&g_fredSaveData, sizeof(g_fredSaveData), g_fredSaveFile);

	_xp->stopCycle();
	termFredLevel(savedLevel, savedPalette);

	freeBOLTGroup(g_fredBoltLib, 0, 1);
	closeBOLTLib(&g_fredBoltLib);

	_xp->setFrameRate(0);
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
}

bool BoltEngine::initFredLevel(int16 levelGroup, int16 palGroup) {
	// Load level data group...
	if (!getBOLTGroup(g_fredBoltLib, levelGroup * 0x200 + 0x100, 1))
		return false;

	// Load palette group...
	if (!getBOLTGroup(g_fredBoltLib, palGroup * 0x200 + 0x200, 1)) {
		freeBOLTGroup(g_fredBoltLib, levelGroup * 0x200 + 0x100, 1);
		return false;
	}

	// Level descriptor member...
	g_fredLevelPtr = memberAddr(g_fredBoltLib, levelGroup * 0x200 + 0x100);

	// Palette member...
	g_fredPalette = memberAddr(g_fredBoltLib, palGroup * 0x200 + 0x200);

	// Cycle data...
	g_fredCycleRaw = memberAddr(g_fredBoltLib, palGroup * 0x200 + 0x20A);
	boltCycleToXPCycle(g_fredCycleRaw, g_fredCycleSpecs);

	// Other resources...
	g_fredBalloonSprite = memberAddr(g_fredBoltLib, palGroup * 0x200 + 0x209);
	g_fredPathMatrix = memberAddr(g_fredBoltLib, levelGroup * 0x200 + 0x132);
	g_fredRowBounds = memberAddr(g_fredBoltLib, levelGroup * 0x200 + 0x101);

	// Allocate shuffle table: numRows * (numCols + 1)
	uint16 numRows = READ_UINT16(g_fredLevelPtr + 4);
	uint16 numCols = READ_UINT16(g_fredLevelPtr + 6);

	g_fredShuffleTable = (byte *)_xp->allocMem(numRows * (numCols + 1));
	if (!g_fredShuffleTable) {
		termFredLevel(levelGroup, palGroup);
		return false;
	}

	// Build and shuffle column indices for each row...
	for (int16 row = 0; row < numRows; row++) {
		numCols = READ_UINT16(g_fredLevelPtr + 6);
		int16 stride = numCols + 1;
		int16 rowBase = stride * row;
		g_fredShuffleTable[rowBase] = 0;

		for (int16 col = 0; col < numCols; col++) {
			g_fredShuffleTable[rowBase + col + 1] = (byte)col;
		}

		// Fisher-Yates shuffle...
		for (int16 col = 0; col < numCols; col++) {
			int16 randIdx = _xp->getRandom(numCols);
			int16 posA = rowBase + col + 1;
			int16 posB = rowBase + randIdx + 1;

			byte tmp = g_fredShuffleTable[posA];
			g_fredShuffleTable[posA] = g_fredShuffleTable[posB];
			g_fredShuffleTable[posB] = tmp;
		}
	}

	// Allocate balloon table...
	uint16 numBalloons = READ_UINT16(g_fredLevelPtr + 0x0A);
	g_fredEntitiesTable = (FredEntityState **)_xp->allocMem((numBalloons + 2) * sizeof(FredEntityState *));
	if (!g_fredEntitiesTable) {
		termFredLevel(levelGroup, palGroup);
		return false;
	}

	// Allocate each balloon state struct...
	for (int16 i = 0; i < numBalloons; i++) {
		FredEntityState *entry = new FredEntityState();

		if (!entry) {
			termFredLevel(levelGroup, palGroup);
			return false;
		}

		entry->flags = 0;
		g_fredEntitiesTable[i + 1] = entry;
	}

	// Sentinel: null pointer after last balloon...
	g_fredEntitiesTable[numBalloons + 1] = nullptr;

	// First entry points to Fred's sprite...
	g_fredEntitiesTable[0] = &g_fredSprite;

	// Allocate balloon type shuffle array...
	uint16 numTypes = READ_UINT16(g_fredLevelPtr + 0x0C);
	byte *typeShuf = (byte *)_xp->allocMem(numTypes);
	if (!typeShuf) {
		termFredLevel(levelGroup, palGroup);
		return false;
	}

	for (int16 i = 0; i < numTypes; i++) {
		typeShuf[i] = (byte)i;
	}

	// Reshuffle every numTypes balloons...
	for (int16 i = 0; i < numBalloons; i++) {
		if ((i % numTypes) == 0) {
			for (int16 j = 0; j < numTypes; j++) {
				int16 randIdx = _xp->getRandom(numTypes);
				byte tmp = typeShuf[j];
				typeShuf[j] = typeShuf[randIdx];
				typeShuf[randIdx] = tmp;
			}
		}

		// Assign balloon type...
		byte balloonType = typeShuf[i % numTypes] + 0x0B;
		FredEntityState *balloonEntry = g_fredEntitiesTable[i + 1];
		setFredAnimMode(balloonEntry, balloonType);
	}

	_xp->freeMem(typeShuf);

	g_fredBalloonSpawnDelay = calcBalloonSpawnDelay();
	g_fredBalloonSearchIdx = 0;
	g_fredLevelIndex = 0;

	return true;
}

void BoltEngine::termFredLevel(int16 levelGroup, int16 palGroup) {
	// Free balloon state structs...
	if (g_fredEntitiesTable) {
		uint16 numBalloons = READ_UINT16(g_fredLevelPtr + 0x0A);
		for (int16 i = 0; i < numBalloons; i++) {
			FredEntityState *entry = g_fredEntitiesTable[i + 1];
			if (entry) {
				delete entry;
				g_fredEntitiesTable[i + 1] = nullptr;
			}
		}

		delete[] g_fredEntitiesTable;
		g_fredEntitiesTable = nullptr;
	}

	// Free shuffle table...
	if (g_fredShuffleTable) {
		_xp->freeMem(g_fredShuffleTable);
		g_fredShuffleTable = nullptr;
	}

	// Free BOLT groups...
	freeBOLTGroup(g_fredBoltLib, palGroup * 0x200 + 0x200, 1);
	freeBOLTGroup(g_fredBoltLib, levelGroup * 0x200 + 0x100, 1);
}

void BoltEngine::swapFredAnimEntry() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	uint32 decompSize = g_boltCurrentMemberEntry->decompSize;
	uint32 offset = 0;

	while (offset < decompSize) {
		WRITE_UINT16(data + offset + 4, READ_BE_INT16(data + offset + 4));
		resolveIt((uint32 *)(data + offset));
		offset += 6;
	}
}

void BoltEngine::swapFredAnimDesc() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	uint32 decompSize = g_boltCurrentMemberEntry->decompSize;
	uint32 offset = 0;
	byte *ptr = data;
	byte *colorCountPtr = data + 0x0E;

	while (offset < decompSize) {
		WRITE_UINT32(ptr + 0x00, READ_BE_INT32(ptr + 0x00));
		WRITE_UINT32(ptr + 0x08, READ_BE_INT32(ptr + 0x08));
		WRITE_UINT16(ptr + 0x0C, READ_BE_INT16(ptr + 0x0C));
		WRITE_UINT16(colorCountPtr, READ_BE_INT16(colorCountPtr));

		uint16 colorCount = READ_UINT16(colorCountPtr);
		unpackColors(colorCount, ptr + 0x10);
		unpackColors(colorCount, ptr + 0x2C);

		resolveIt((uint32 *)(ptr + 0x04));

		offset += 0x48;
		colorCountPtr += 0x48;
		ptr += 0x48;
	}
}

void BoltEngine::swapFredLevelDesc() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;

	WRITE_UINT16(data + 0x00, READ_BE_INT16(data + 0x00));
	WRITE_UINT16(data + 0x02, READ_BE_INT16(data + 0x02));
	WRITE_UINT16(data + 0x04, READ_BE_INT16(data + 0x04));
	WRITE_UINT16(data + 0x06, READ_BE_INT16(data + 0x06));
	WRITE_UINT16(data + 0x08, READ_BE_INT16(data + 0x08));
	WRITE_UINT16(data + 0x0A, READ_BE_INT16(data + 0x0A));
	WRITE_UINT16(data + 0x0C, READ_BE_INT16(data + 0x0C));
}

int16 BoltEngine::playFred() {
	int16 result = 0;
	int16 allCaught = 0;
	int16 exitLoop = 0;
	int16 joystickX = 0;

	// Show help on first play...
	if (g_fredShowHelp != 0) {
		if (!helpFred())
			return 4;
	}

	_xp->setInactivityTimer(30);
	_xp->enableController();

	while (!exitLoop) {
		if (joystickX != 0)
			_xp->setInactivityTimer(30);

		// Process events...
		int16 eventType;
		uint32 eventData;
		while ((eventType = _xp->getEvent(0, &eventData)) != 0) {
			switch (eventType) {
			case etJoystick:
				joystickX = (int16)(eventData >> 16);
				break;
			case etMouseDown:
			case etInactivity:
				if (!allCaught) {
					int16 helpMode = (g_fredSprite.animMode == 8 || g_fredSprite.animMode == 6) ? 1 : 0;
					setFredAnimMode(&g_fredSprite, helpMode);
					if (!helpFred()) {
						result = 4;
						exitLoop = 1;
					}
				}

				break;
			case etSound:
				updateFredSound();
				break;
			default:
				break;
			}
		}

		if (exitLoop)
			break;

		// Render and update...
		renderFredScene();
		_xp->updateDisplay();

		// Balloon spawn timer...
		g_fredBalloonSpawnDelay--;
		if (g_fredBalloonSpawnDelay == 0) {
			if (spawnBalloon())
				g_fredBalloonSpawnDelay = calcBalloonSpawnDelay();
			else
				g_fredBalloonSpawnDelay = 1;
		}

		// --- First pass: update Fred and balloons animations ---
		int16 idx = 0;
		FredEntityState *entry = g_fredEntitiesTable[idx];
		while (entry) {
			if (entry->flags & 1) { // active
				// Save previous position...
				entry->prevXPos = entry->xPos;
				entry->prevYPos = entry->yPos;

				// Store joystick direction...
				entry->direction = joystickX;

				// Clear frame-changed and anim-ended flags...
				entry->flags &= 0xFFF9;

				// Advance frame countdown...
				int16 countdown = entry->frameCountdown - 1;
				entry->frameCountdown = countdown;

				if (countdown == 0) {
					byte *animTable = entry->animTable;
					int16 frameIdx = entry->frameIndex + 1;
					entry->frameIndex = frameIdx;

					// Check if next frame exists...
					byte *nextSprite = getResolvedPtr(animTable, frameIdx * 6);
					if (!nextSprite) {
						// Loop back to frame 0...
						entry->frameIndex = 0;
						entry->flags |= 4; // set anim-ended
					}

					// Read new frame duration...
					entry->frameCountdown = READ_UINT16(animTable + entry->frameIndex * 6 + 4);
					entry->flags |= 2; // frame-changed
				}

				// Mode-specific behavior...
				int16 mode = entry->animMode;
				switch (mode) {
				case 0: // Fred idle facing right
					if (allCaught) {
						int16 newMode = (entry->xPos < 0x8200) ? 7 : 4;
						setFredAnimMode(entry, newMode);
					} else {
						int16 dir = entry->direction;
						if (dir == 1)
							setFredAnimMode(entry, 4); // run right
						else if (dir == -1)
							setFredAnimMode(entry, 7); // turn to run left
					}

					break;
				case 2: // Fred idle facing right (variant)
					if (allCaught) {
						int16 newMode = (entry->xPos < 0x8200) ? 7 : 4;
						setFredAnimMode(entry, newMode);
					} else {
						int16 dir = entry->direction;
						if (dir == 1)
							setFredAnimMode(entry, 4);
						else if (dir == -1)
							setFredAnimMode(entry, 7);
					}

					break;
				case 1: // Fred idle facing left
					if (allCaught) {
						int16 newMode = (entry->xPos < 0x8200) ? 8 : 3;
						setFredAnimMode(entry, newMode);
					} else {
						int16 dir = entry->direction;
						if (dir == -1)
							setFredAnimMode(entry, 8); // run left
						else if (dir == 1)
							setFredAnimMode(entry, 3); // turn to run right
					}

					break;
				case 6: // Fred idle facing left (variant)
					if (allCaught) {
						int16 newMode = (entry->xPos < 0x8200) ? 8 : 3;
						setFredAnimMode(entry, newMode);
					} else {
						int16 dir = entry->direction;
						if (dir == -1)
							setFredAnimMode(entry, 8);
						else if (dir == 1)
							setFredAnimMode(entry, 3);
					}

					break;
				case 3: // Turn right->left
					if (entry->flags & 4) // anim ended
						setFredAnimMode(entry, 4);

					break;
				case 7: // Turn left->right
					if (entry->flags & 4)
						setFredAnimMode(entry, 8);

					break;
				case 4: // Fred running right (stopped)
					if (!allCaught) {
						int16 dir = entry->direction;
						if (dir == -1)
							setFredAnimMode(entry, 7);
						else if (dir == 0)
							setFredAnimMode(entry, 2); // stop facing right
					}

					break;
				case 8: // Fred running left (stopped)
					if (!allCaught) {
						int16 dir = entry->direction;
						if (dir == 1)
							setFredAnimMode(entry, 3);
						else if (dir == 0)
							setFredAnimMode(entry, 6); // stop facing left
					}

					break;
				case 5: // ?
					if (entry->flags & 4)
						setFredAnimMode(entry, 2);

					break;
				case 9: // ?
					if (entry->flags & 4)
						setFredAnimMode(entry, 6);

					break;
				}
			}

			idx++;
			entry = g_fredEntitiesTable[idx];
		}

		// --- Second pass: move balloons and Fred ---
		idx = 0;
		entry = g_fredEntitiesTable[idx];
		while (entry) {
			if (entry->flags & 1) {
				int16 mode = entry->animMode;

				if (mode == 4) {
					// Running right
					int32 speed = (int32)entry->speed;
					int32 xPos = entry->xPos + speed;
					entry->xPos = xPos;

					if (!allCaught) {
						if (xPos > 0x11D00) {
							entry->xPos = 0x11D00;
							setFredAnimMode(entry, 2); // stop at right edge
						}
					} else {
						if (xPos > 0x18000)
							exitLoop = 1; // ran off right side
					}

				} else if (mode == 8) {
					// Running left
					int32 speed = (int32)entry->speed;
					int32 xPos = entry->xPos - speed;
					entry->xPos = xPos;

					if (!allCaught) {
						if (xPos < 0) {
							entry->xPos = 0;
							setFredAnimMode(entry, 6); // stop at left edge
						}
					} else {
						if (xPos < (int32)0xFFFF9400)
							exitLoop = 1; // ran off left side
					}

				} else if (mode == 0x0B) {
					// Balloon floating...
					int16 pathIdx = entry->pathIndex + 1;
					entry->pathIndex = pathIdx;

					byte *pathTable = entry->pathTable;
					int16 pathCount = READ_UINT16(pathTable);

					if (pathIdx >= pathCount) {
						// Balloon escaped, deactivate...
						entry->flags &= 0xFFFE;
					} else {
						// Update position from path table...
						byte *pathEntry = pathTable + pathIdx * 4;
						int32 newX = ((int32)READ_UINT16(pathEntry + 2)) << 8;
						entry->xPos = newX;
						int32 newY = ((int32)READ_UINT16(pathEntry + 4)) << 8;
						entry->yPos = newY;
					}
				}
			}

			idx++;
			entry = g_fredEntitiesTable[idx];
		}

		// --- Skip collision check if all caught ---
		if (allCaught)
			continue;

		// --- Get Fred's collision rect based on current mode ---
		byte *fredRect;
		switch (g_fredSprite.animMode) {
		case 6:
		case 8:
			fredRect = g_fredFacingLeftRect; // facing/running left
			break;
		case 2:
		case 4:
			fredRect = g_fredFacingRightRect; // facing/running right
			break;
		case 7:
			fredRect = g_fredTurningRect; // turning left
			break;
		case 3:
			fredRect = g_fredTurningRect; // turning right
			break;
		default:
			fredRect = nullptr;
			break;
		}

		if (!fredRect)
			continue;

		// Build Fred's bounding rect...
		int16 fredX = ((int32)g_fredSprite.xPos >> 8) + READ_UINT16(fredRect);
		int16 fredY = ((int32)g_fredSprite.yPos >> 8) + READ_UINT16(fredRect + 2);
		int16 fredW = READ_UINT16(fredRect + 4);
		int16 fredH = READ_UINT16(fredRect + 6);

		// --- Third pass: check collisions with balloons ---
		idx = 0;
		entry = g_fredEntitiesTable[idx];
		while (entry) {
			if ((entry->flags & 1) && entry->animMode == 0x0B) {
				// Build balloon bounding rect...
				int16 bx = ((int32)entry->xPos >> 8) + READ_UINT16(g_fredBalloonRect);
				int16 by = ((int32)entry->yPos >> 8) + READ_UINT16(g_fredBalloonRect + 2);
				int16 bw = READ_UINT16(g_fredBalloonRect + 4);
				int16 bh = READ_UINT16(g_fredBalloonRect + 6);

				// {x, y, w, h} rect overlap test...
				Common::Rect balloonRect(bx, by, bx + bw, by + bh);
				Common::Rect playerRect(fredX, fredY, fredX + fredW, fredY + fredH);

				if (playerRect.intersects(balloonRect)) {
					// Caught a balloon!
					int16 catchMode;
					if (g_fredSprite.animMode == 6 || g_fredSprite.animMode == 8 || g_fredSprite.animMode == 7)
						catchMode = 9; // catch facing left
					else
						catchMode = 5; // catch facing right

					setFredAnimMode(&g_fredSprite, catchMode);
					setFredAnimMode(entry, 0x0A); // balloon disappearing

					g_fredLevelIndex++;
					if (g_fredLevelIndex >= READ_UINT16(g_fredLevelPtr + 0x0A)) {
						allCaught = 1;
						result = 0x10;
					}

					// Reset spawn timer...
					if (spawnBalloon())
						g_fredBalloonSpawnDelay = calcBalloonSpawnDelay();
					else
						g_fredBalloonSpawnDelay = 1;

					break; // Only catch one per frame...
				}
			}

			idx++;
			entry = g_fredEntitiesTable[idx];
		}
	}

	_xp->stopSound();
	_xp->disableController();
	return result;
}

int16 BoltEngine::helpFred() {
	byte *firstEntry = getResolvedPtr(g_fredHelpEntries, 0);
	byte *picDesc = getResolvedPtr(firstEntry, 4);

	int16 cursorX = READ_UINT16(picDesc + 6) + READ_UINT16(picDesc + 0x0A) - 10;
	int16 cursorY = READ_UINT16(picDesc + 8) + READ_UINT16(picDesc + 0x0C) - 10;

	byte *playableEntry = nullptr;
	int16 exitResult = -1;
	int16 isPlaying = 0;
	int16 soundPending = 0;

	_xp->setFrameRate(0);
	_xp->setInactivityTimer(0);
	_xp->stopSound();

	g_fredLoopSound = nullptr;
	g_fredCurrentSound = nullptr;
	g_fredPendingLoop = nullptr;
	g_fredPendingOneShot = nullptr;

	renderFredScene();

	// Display all help entry pics on front surface...
	int16 off = 0;
	byte *entry;
	while (true) {
		entry = getResolvedPtr(g_fredHelpEntries, off);
		if (!entry)
			break;
		byte *pic = getResolvedPtr(entry, 4);
		displayPic(pic, 0, 0, stFront);
		off += 4;
	}

	_xp->updateDisplay();

	// Save current palette for each entry, find playable entry...
	off = 0;
	while (true) {
		entry = getResolvedPtr(g_fredHelpEntries, off);
		if (!entry)
			break;

		_xp->getPalette(READ_UINT16(entry + 0x0C), READ_UINT16(entry + 0x0E), entry + 0x2C);
		WRITE_UINT32(entry + 8, READ_UINT32(entry + 8) & 0xFFFFFFFE); // clear highlight flag

		if (READ_UINT32(entry) == 2)
			playableEntry = entry;

		off += 4;
	}

	// Setup cursor...
	_xp->setCursorPos(cursorX, cursorY);
	_xp->setCursorColor(255, 255, 0);
	_xp->disableController();
	_xp->showCursor();

	// Highlight first entry...
	g_fredHoveredEntry = getResolvedPtr(g_fredHelpEntries, 0);
	hiliteFredHelpObject(getResolvedPtr(g_fredHelpEntries, 0), 1);

	// Main help loop
	while (exitResult == -1) {
		// Handle audio playback...
		if (isPlaying) {
			if (!maintainAudioPlay(soundPending)) {
				int16 highlight = (playableEntry == g_fredHoveredEntry) ? 1 : 0;
				hiliteFredHelpObject(playableEntry, highlight);
				isPlaying = 0;
			}
			soundPending = 0;
		}

		// Process events...
		uint32 eventData;
		int16 eventType = _xp->getEvent(0, &eventData);

		switch (eventType) {
		case etTimer: {
			// Timer expired, toggle selected entry highlight...
			if (!g_fredCurrentHelpObject)
				break;

			if (g_fredTimer != eventData)
				break;

			g_fredTimer = _xp->startTimer(500);

			byte *sel = g_fredCurrentHelpObject;
			int16 highlighted = (READ_UINT32(sel + 8) & 1) ? 0 : 1;
			hiliteFredHelpObject(sel, highlighted);
			break;
		}
		case etMouseMove: {
			cursorX = (int16)(eventData >> 16);
			cursorY = (int16)(eventData & 0xFFFF);

			// Hit test against all help entries...
			off = 0;
			byte *hitEntry = nullptr;
			while (true) {
				entry = getResolvedPtr(g_fredHelpEntries, off);
				if (!entry)
					break;

				byte *pd = getResolvedPtr(entry, 4);
				int16 ex = READ_UINT16(pd + 6);
				int16 ey = READ_UINT16(pd + 8);
				int16 ew = READ_UINT16(pd + 0x0A);
				int16 eh = READ_UINT16(pd + 0x0C);

				if (cursorX >= ex && cursorX < ex + ew &&
					cursorY >= ey && cursorY < ey + eh) {
					hitEntry = entry;
					break;
				}
				off += 4;
			}

			// Update hover state...
			if (hitEntry == g_fredHoveredEntry)
				break;

			// Unhighlight previous...
			if (g_fredHoveredEntry &&
				g_fredHoveredEntry != g_fredCurrentHelpObject &&
				!(g_fredHoveredEntry == playableEntry && isPlaying)) {
				hiliteFredHelpObject(g_fredHoveredEntry, 0);
			}

			g_fredHoveredEntry = hitEntry;

			// Highlight new...
			if (hitEntry && hitEntry != g_fredCurrentHelpObject) {
				hiliteFredHelpObject(hitEntry, 1);
			}

			break;
		}
		case etMouseDown: {
			int16 justStopped = 0;

			// Stop current animation if playing...
			if (isPlaying) {
				if (g_fredTimer) {
					_xp->killTimer(g_fredTimer);
					g_fredTimer = 0;
				}

				int16 hl = (playableEntry == g_fredHoveredEntry) ? 1 : 0;
				hiliteFredHelpObject(playableEntry, hl);

				hl = (g_fredCurrentHelpObject == g_fredHoveredEntry) ? 1 : 0;
				hiliteFredHelpObject(g_fredCurrentHelpObject, hl);

				g_fredCurrentHelpObject = nullptr;
				stopAnimation();
				isPlaying = 0;
				justStopped = 1;
			}

			// Handle click on hovered entry...
			if (!g_fredHoveredEntry)
				break;

			uint32 entryType = READ_UINT32(g_fredHoveredEntry);

			if (entryType == 0) {
				// Exit help...
				exitResult = 0;
			} else if (entryType == 1) {
				// Exit help and clear show-help flag...
				g_fredShowHelp = 0;
				exitResult = 1;
			} else if (entryType == 2) {
				// Play animation...
				if (!isPlaying && !justStopped) {
					if (startAnimation(g_rtfHandle, 0x1B)) {
						g_fredHelpStep = 0;
						isPlaying = 1;
					}
				}
			}

			break;
		}
		case etSound:
			soundPending = 1;
			break;
		case etTrigger:
			if (isPlaying)
				helpAnimStep();

			break;
		}
	}

	// Cleanup: restore help screen
	_xp->hideCursor();
	_xp->enableController();

	// Restore all entries to highlight state matching hover
	off = 0;
	while (true) {
		entry = getResolvedPtr(g_fredHelpEntries, off);
		if (!entry)
			break;
		int16 hl = (entry == g_fredHoveredEntry) ? 1 : 0;
		hiliteFredHelpObject(entry, hl);
		off += 4;
	}

	_xp->updateDisplay();

	// Unhighlight all entries
	off = 0;
	while (true) {
		entry = getResolvedPtr(g_fredHelpEntries, off);
		if (!entry)
			break;
		hiliteFredHelpObject(entry, 0);
		off += 4;
	}

	renderFredScene();
	_xp->updateDisplay();

	_xp->setFrameRate(15);
	_xp->setInactivityTimer(30);
	updateFredSound();

	return exitResult;
}

void BoltEngine::hiliteFredHelpObject(byte *entry, int16 highlight) {
	if (!entry)
		return;

	if (highlight) {
		// Apply highlight palette...
		_xp->setPalette(READ_UINT16(entry + 0x0E), READ_UINT16(entry + 0x0C), entry + 0x10);
		WRITE_UINT32(entry + 8, READ_UINT32(entry + 8) | 1);
		_system->updateScreen();
	} else {
		// Restore saved palette...
		_xp->setPalette(READ_UINT16(entry + 0x0E), READ_UINT16(entry + 0x0C), entry + 0x2C);
		WRITE_UINT32(entry + 8, READ_UINT32(entry + 8) & 0xFFFFFFFE);
		_system->updateScreen();
	}
}

void BoltEngine::helpAnimStep() {
	if (g_fredHelpStep < 0 || g_fredHelpStep >= 4)
		return;

	// Unhighlight previous selected if not hovered...
	int16 hl = (g_fredCurrentHelpObject == g_fredHoveredEntry) ? 1 : 0;
	hiliteFredHelpObject(g_fredCurrentHelpObject, hl);

	// On even steps, select entry from g_fredPlayButton table; on odd steps, deselect...
	if (!(g_fredHelpStep & 1)) {
		int16 idx = g_fredHelpStep >> 1;
		g_fredCurrentHelpObject = getResolvedPtr(g_fredPlayButton, idx * 4);
	} else {
		g_fredCurrentHelpObject = nullptr;
	}

	g_fredTimer = 0;

	// Highlight new selected entry...
	hiliteFredHelpObject(g_fredCurrentHelpObject, 1);

	// Start/stop blink timer...
	if (!(g_fredHelpStep & 1)) {
		g_fredTimer = _xp->startTimer(500);
	} else {
		if (g_fredTimer) {
			_xp->killTimer(g_fredTimer);
			g_fredTimer = 0;
		}
	}

	g_fredHelpStep++;
}

bool BoltEngine::spawnBalloon() {
	// Count active balloons...
	int16 activeCount = 0;
	int16 i = 0;

	FredEntityState *entry;
	while (true) {
		entry = g_fredEntitiesTable[i];
		if (!entry)
			break;

		if (entry->animMode == 0x0B && (entry->flags & 1))
			activeCount++;

		i++;
	}

	// Max 6 active balloons at once...
	if (activeCount >= 6)
		return false;

	// Find next free balloon slot starting from the search index...
	int16 searchStart = g_fredBalloonSearchIdx;
	int16 wrapped = 0;
	i = searchStart;

	while (true) {
		entry = g_fredEntitiesTable[i];
		if (!entry) {
			wrapped = 1;
			i = -1;
		} else if (entry->animMode == 0x0B && (entry->flags & 1) == 0) {
			// Found free balloon slot!
			break;
		}

		i++;

		if (i == searchStart && wrapped)
			break;
	}

	// No free slot found...
	if (i == searchStart && wrapped)
		return false;

	g_fredBalloonSearchIdx = i + 1;

	// Pick row from shuffle table...
	int16 row = selectBalloonRow();

	uint16 numCols = READ_UINT16(g_fredLevelPtr + 6);
	int16 stride = numCols + 1;
	int16 rowBase = stride * row;

	// Get next column from shuffle table...
	byte *shuffleRow = g_fredShuffleTable + rowBase;
	byte colCounter = shuffleRow[0];
	byte col = g_fredShuffleTable[rowBase + colCounter + 1];

	// Look up path table: g_fredPathMatrix[row][col]
	byte *rowPaths = getResolvedPtr(g_fredPathMatrix, row * 4);
	byte *pathTable = getResolvedPtr(rowPaths, col * 4);

	// Assign path to balloon...
	entry->pathTable = pathTable;

	// Advance column counter, wrap if needed...
	shuffleRow[0]++;
	if (shuffleRow[0] >= numCols)
		shuffleRow[0] = 0;

	// Init balloon position from path entry 0...
	entry->pathIndex = 0;

	byte *pathEntry0 = pathTable + 0;
	int32 startX = (int32)(int16)READ_UINT16(pathEntry0 + 2) << 8;
	entry->xPos = startX;
	int32 startY = (int32)(int16)READ_UINT16(pathEntry0 + 4) << 8;
	entry->yPos = startY;

	// Activate!
	entry->flags |= 1;

	return true;
}

int16 BoltEngine::calcBalloonSpawnDelay() {
	int16 range = READ_UINT16(g_fredLevelPtr + 2);
	int16 base = READ_UINT16(g_fredLevelPtr + 0);
	return _xp->getRandom(range) + base;
}

int16 BoltEngine::selectBalloonRow() {
	// Get Fred's center X position...
	int32 fredX = g_fredSprite.xPos >> 8;

	// Get Fred's sprite width/2 to find center...
	int16 frameIdx = g_fredSprite.frameIndex;
	byte *spriteDesc = getResolvedPtr(g_fredSprite.animTable, frameIdx * 6);
	int16 halfWidth = READ_UINT16(spriteDesc + 0x0A) >> 1;
	int16 fredCenter = (int16)fredX + halfWidth;

	// Find which row Fred is in...
	int16 row = 0;
	byte *rowBounds = g_fredRowBounds; // Array of int16 x-values
	uint16 numRows = READ_UINT16(g_fredLevelPtr + 4);

	while (row < numRows) {
		if (READ_UINT16(rowBounds + row * 2) > fredCenter)
			break;
		row++;
	}

	// Pick random row, rejecting if too close to Fred's row...
	int16 rowBias = READ_UINT16(g_fredLevelPtr + 8);
	int16 randomRow;
	do {
		randomRow = _xp->getRandom(numRows);
	} while (randomRow >= row - rowBias && randomRow <= row + rowBias);

	return randomRow;
}

void BoltEngine::setFredAnimMode(FredEntityState *state, int16 mode) {
	if (mode >= 0x0B && mode < 0x0B + (g_fredLevelPtr ? READ_UINT16(g_fredLevelPtr + 0x0C) : 0)) {
		state->animMode = 0x0B;

		int16 typeIdx = mode - 0x0B;
		state->animTable = getResolvedPtr(g_fredBalloonSprite, typeIdx * 4);
		state->frameIndex = 0;
		state->frameCountdown = READ_UINT16(state->animTable + 4);
		return;
	}

	if (mode == 0x0A) {
		// Balloon deactivate...
		state->animMode = 0x0A;
		state->flags &= 0xFFFE; // clear active flag
		return;
	}

	// Standard Fred animation...
	state->animMode = mode;
	state->animTable = g_fredSprites[mode];

	state->frameIndex = 0;
	state->frameCountdown = READ_UINT16(state->animTable + 4);

	// Play sound based on mode...
	switch (mode) {
	case 3: // turn right
		playFredSound(&g_fredSounds[2], nullptr);
		break;
	case 4: // run right
		playFredSound(&g_fredSounds[0], &g_fredSounds[0]);
		break;
	case 5: // catch right
		playFredSound(&g_fredSounds[3], nullptr);
		break;
	case 7: // turn left
		playFredSound(&g_fredSounds[2], nullptr);
		break;
	case 8: // run left
		playFredSound(&g_fredSounds[1], &g_fredSounds[1]);
		break;
	case 9: // catch left
		playFredSound(&g_fredSounds[3], nullptr);
		break;
	default: // 0, 1, 2, 6
		playFredSound(nullptr, nullptr);
		break;
	}
}

void BoltEngine::renderFredScene() {
	_xp->fillDisplay(0, stFront);

	int16 idx = 0;

	while (true) {
		FredEntityState *entry = g_fredEntitiesTable[idx];
		if (!entry)
			break;

		if (entry->flags & 1) { // active
			int16 x = (int16)(entry->xPos >> 8);
			int16 y = (int16)(entry->yPos >> 8);

			// Get current frame sprite...
			byte *sprite = getResolvedPtr(entry->animTable, entry->frameIndex * 6);

			displayPic(sprite, x, y, stFront);

			// If balloon (mode 0x0B), draw string below...
			if (entry->animMode == 0x0B) {
				displayPic(g_fredBalloonString, x + 10, y + 25, stFront);
			}
		}

		idx++;
	}
}

void BoltEngine::getFredSoundInfo(BOLTLib *lib, int16 memberId, SoundInfo *soundInfo) {
	soundInfo->data = memberAddr(lib, memberId);
	soundInfo->size = memberSize(lib, memberId);
}

void BoltEngine::playFredSound(SoundInfo *oneShot, SoundInfo *loop) {
	g_fredPendingOneShot = oneShot;
	g_fredPendingLoop = loop; 

	if (g_fredLoopSound != nullptr || loop == nullptr) {
		// Check if current loop is g_fredSounds[2] and new one-shot is g_fredSounds[1]
		// (sound transition check)
		if (g_fredCurrentSound == &g_fredSounds[3] && g_fredPendingOneShot == &g_fredSounds[2]) {
			// Allow transition without stopping...
		} else {
			g_fredLoopSound = nullptr;
			g_fredCurrentSound = nullptr;
			_xp->stopSound();
		}
	}

	// If there's a pending sound and nothing currently playing, trigger it!
	if (g_fredPendingOneShot != nullptr && g_fredCurrentSound == nullptr) {
		updateFredSound();
	}
}

void BoltEngine::updateFredSound() {
	bool startLoop = false;

	if (g_fredLoopSound != nullptr) {
		// Loop sound active, continue with it...
		g_fredCurrentSound = g_fredLoopSound;
	} else if (g_fredPendingOneShot != nullptr) {
		// Start pending one-shot...
		g_fredCurrentSound = g_fredPendingOneShot;
		g_fredLoopSound = g_fredPendingLoop;
		startLoop = (g_fredPendingLoop != nullptr);
		g_fredPendingLoop = 0;
		g_fredPendingOneShot = 0;
	} else {
		// Nothing to play...
		g_fredCurrentSound = nullptr;
	}

	if (g_fredCurrentSound != nullptr) {
		// Play the current sound...
		SoundInfo *snd = g_fredCurrentSound;
		_xp->playSound(snd->data, snd->size, 22050);

		if (startLoop) {
			// Queue loop sound twice...
			SoundInfo *loopSnd = g_fredLoopSound;
			_xp->playSound(loopSnd->data, loopSnd->size, 22050);
			_xp->playSound(loopSnd->data, loopSnd->size, 22050);
		}
	}
}

} // End of namespace Bolt
