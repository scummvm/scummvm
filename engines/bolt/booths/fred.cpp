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

	if (!openBOLTLib(&_fredBoltLib, &_fredBoltCallbacks, path))
		return false;

	if (!getBOLTGroup(_fredBoltLib, 0, 1))
		return false;

	// Load assets...
	_fredBackground = memberAddr(_fredBoltLib, (_displayMode != 0) ? 5 : 4);

	_fredBalloonString = memberAddr(_fredBoltLib, 6);

	_fredFacingLeftRect = memberAddr(_fredBoltLib, 0);
	_fredFacingRightRect = memberAddr(_fredBoltLib, 1);
	_fredTurningRect = memberAddr(_fredBoltLib, 2);
	_fredBalloonRect = memberAddr(_fredBoltLib, 3);

	_fredHelpEntries = memberAddr(_fredBoltLib, 0x38);
	_fredPlayButton = memberAddr(_fredBoltLib, 0x39);
	_fredTimer = 0;

	for (int i = 0; i < 10; i++)
		_fredSprites[i] = memberAddr(_fredBoltLib, 0x28 + i);

	getFredSoundInfo(_fredBoltLib, 0x3A, &_fredSounds[0]);
	getFredSoundInfo(_fredBoltLib, 0x3B, &_fredSounds[1]);
	getFredSoundInfo(_fredBoltLib, 0x3C, &_fredSounds[2]);
	getFredSoundInfo(_fredBoltLib, 0x3D, &_fredSounds[3]);

	_fredLoopSound = nullptr;
	_fredCurrentSound = nullptr;
	_fredPendingLoop = nullptr;
	_fredPendingOneShot = nullptr;

	// Init Fred sprite struct...
	_fredSprite.flags = 1;
	_fredSprite.direction = 0;
	_fredSprite.xPos = 0xA000; // Fixed-point coordinate
	_fredSprite.yPos = 0x1600;	// Fixed-point coordinate
	_fredSprite.speed = 0x600;
	_fredSprite.pathTable = nullptr;

	setFredAnimMode(&_fredSprite, 0);

	// Load save data...
	if (!vLoad(&_fredSaveData, "FredBC")) {
		_fredSaveData[0] = 0; // Balloon catch count
		_fredSaveData[1] = 0; // Level data group index, increments on level complete, caps at 9
		_fredSaveData[2] = 0; // Palette group index, increments on level complete, wraps at 10
	}

	if (!initFredLevel(_fredSaveData[1], _fredSaveData[2]))
		return false;

	_fredLevelIndex = 0;
	while (_fredLevelIndex < _fredSaveData[0]) {
		FredEntityState *entry = _fredEntitiesTable[_fredLevelIndex + 1];
		setFredAnimMode(entry, 10);
		_fredLevelIndex++;
	}

	// Flush non-timer events...
	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	// Display background on back and front buffers...
	displayColors(_fredPalette, stBack, 0);
	displayPic(_fredBackground, _displayX, _displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(_fredPalette, stFront, 0);
	displayColors(_fredPalette, stBack, 1);
	displayPic(_fredBackground, _displayX, _displayY, stBack);

	renderFredScene();
	_xp->updateDisplay();

	_xp->startCycle(_fredCycleSpecs);
	_xp->setFrameRate(15);

	return true;
}

void BoltEngine::cleanUpFred() {
	int16 savedLevel = _fredSaveData[1]; // Level data group index
	int16 savedPalette = _fredSaveData[2]; // Palette group index

	_fredSaveData[0] = _fredLevelIndex;

	byte *levelPtr = _fredLevelPtr;
	if (READ_UINT16(levelPtr + 0x0A) <= _fredLevelIndex) {
		_fredSaveData[1]++;
		if (_fredSaveData[1] >= 10)
			_fredSaveData[1] = 9;

		_fredSaveData[2]++;
		if (_fredSaveData[2] >= 10)
			_fredSaveData[2] = 0;

		_fredSaveData[0] = 0;
	}

	vSave(&_fredSaveData, sizeof(_fredSaveData), "FredBC");

	_xp->stopCycle();
	termFredLevel(savedLevel, savedPalette);

	freeBOLTGroup(_fredBoltLib, 0, 1);
	closeBOLTLib(&_fredBoltLib);

	_xp->setFrameRate(0);
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
}

bool BoltEngine::initFredLevel(int16 levelGroup, int16 palGroup) {
	// Load level data group...
	if (!getBOLTGroup(_fredBoltLib, levelGroup * 0x200 + 0x100, 1))
		return false;

	// Load palette group...
	if (!getBOLTGroup(_fredBoltLib, palGroup * 0x200 + 0x200, 1)) {
		freeBOLTGroup(_fredBoltLib, levelGroup * 0x200 + 0x100, 1);
		return false;
	}

	// Level descriptor member...
	_fredLevelPtr = memberAddr(_fredBoltLib, levelGroup * 0x200 + 0x100);

	// Palette member...
	_fredPalette = memberAddr(_fredBoltLib, palGroup * 0x200 + 0x200);

	// Cycle data...
	_fredCycleRaw = memberAddr(_fredBoltLib, palGroup * 0x200 + 0x20A);
	boltCycleToXPCycle(_fredCycleRaw, _fredCycleSpecs);

	// Other resources...
	_fredBalloonSprite = memberAddr(_fredBoltLib, palGroup * 0x200 + 0x209);
	_fredPathMatrix = memberAddr(_fredBoltLib, levelGroup * 0x200 + 0x132);
	_fredRowBounds = memberAddr(_fredBoltLib, levelGroup * 0x200 + 0x101);

	// Allocate shuffle table: numRows * (numCols + 1)
	uint16 numRows = READ_UINT16(_fredLevelPtr + 4);
	uint16 numCols = READ_UINT16(_fredLevelPtr + 6);

	_fredShuffleTable = (byte *)_xp->allocMem(numRows * (numCols + 1));
	if (!_fredShuffleTable) {
		termFredLevel(levelGroup, palGroup);
		return false;
	}

	// Build and shuffle column indices for each row...
	for (int16 row = 0; row < numRows; row++) {
		numCols = READ_UINT16(_fredLevelPtr + 6);
		int16 stride = numCols + 1;
		int16 rowBase = stride * row;
		_fredShuffleTable[rowBase] = 0;

		for (int16 col = 0; col < numCols; col++) {
			_fredShuffleTable[rowBase + col + 1] = (byte)col;
		}

		// Fisher-Yates shuffle...
		for (int16 col = 0; col < numCols; col++) {
			int16 randIdx = _xp->getRandom(numCols);
			int16 posA = rowBase + col + 1;
			int16 posB = rowBase + randIdx + 1;

			byte tmp = _fredShuffleTable[posA];
			_fredShuffleTable[posA] = _fredShuffleTable[posB];
			_fredShuffleTable[posB] = tmp;
		}
	}

	// Allocate balloon table...
	uint16 numBalloons = READ_UINT16(_fredLevelPtr + 0x0A);
	_fredEntitiesTable = (FredEntityState **)_xp->allocMem((numBalloons + 2) * sizeof(FredEntityState *));
	if (!_fredEntitiesTable) {
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
		_fredEntitiesTable[i + 1] = entry;
	}

	// Sentinel: null pointer after last balloon...
	_fredEntitiesTable[numBalloons + 1] = nullptr;

	// First entry points to Fred's sprite...
	_fredEntitiesTable[0] = &_fredSprite;

	// Allocate balloon type shuffle array...
	uint16 numTypes = READ_UINT16(_fredLevelPtr + 0x0C);
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
		FredEntityState *balloonEntry = _fredEntitiesTable[i + 1];
		setFredAnimMode(balloonEntry, balloonType);
	}

	_xp->freeMem(typeShuf);

	_fredBalloonSpawnDelay = calcBalloonSpawnDelay();
	_fredBalloonSearchIdx = 0;
	_fredLevelIndex = 0;

	return true;
}

void BoltEngine::termFredLevel(int16 levelGroup, int16 palGroup) {
	// Free balloon state structs...
	if (_fredEntitiesTable) {
		uint16 numBalloons = READ_UINT16(_fredLevelPtr + 0x0A);
		for (int16 i = 0; i < numBalloons; i++) {
			FredEntityState *entry = _fredEntitiesTable[i + 1];
			if (entry) {
				delete entry;
				_fredEntitiesTable[i + 1] = nullptr;
			}
		}

		delete[] _fredEntitiesTable;
		_fredEntitiesTable = nullptr;
	}

	// Free shuffle table...
	if (_fredShuffleTable) {
		_xp->freeMem(_fredShuffleTable);
		_fredShuffleTable = nullptr;
	}

	// Free BOLT groups...
	freeBOLTGroup(_fredBoltLib, palGroup * 0x200 + 0x200, 1);
	freeBOLTGroup(_fredBoltLib, levelGroup * 0x200 + 0x100, 1);
}

void BoltEngine::swapFredAnimEntry() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	uint32 decompSize = _boltCurrentMemberEntry->decompSize;
	uint32 offset = 0;

	while (offset < decompSize) {
		WRITE_UINT16(data + offset + 4, READ_BE_INT16(data + offset + 4));
		resolveIt((uint32 *)(data + offset));
		offset += 6;
	}
}

void BoltEngine::swapFredAnimDesc() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	uint32 decompSize = _boltCurrentMemberEntry->decompSize;
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
	byte *data = _boltCurrentMemberEntry->dataPtr;

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
	if (_fredShowHelp != 0) {
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
		while ((eventType = _xp->getEvent(etEmpty, &eventData)) != etEmpty) {
			switch (eventType) {
			case etJoystick:
				joystickX = (int16)(eventData >> 16);
				break;
			case etMouseDown:
			case etInactivity:
				if (!allCaught) {
					int16 helpMode = (_fredSprite.animMode == 8 || _fredSprite.animMode == 6) ? 1 : 0;
					setFredAnimMode(&_fredSprite, helpMode);
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
		_fredBalloonSpawnDelay--;
		if (_fredBalloonSpawnDelay == 0) {
			if (spawnBalloon())
				_fredBalloonSpawnDelay = calcBalloonSpawnDelay();
			else
				_fredBalloonSpawnDelay = 1;
		}

		// --- First pass: update Fred and balloons animations ---
		int16 idx = 0;
		FredEntityState *entry = _fredEntitiesTable[idx];
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
			entry = _fredEntitiesTable[idx];
		}

		// --- Second pass: move balloons and Fred ---
		idx = 0;
		entry = _fredEntitiesTable[idx];
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
			entry = _fredEntitiesTable[idx];
		}

		// --- Skip collision check if all caught ---
		if (allCaught)
			continue;

		// --- Get Fred's collision rect based on current mode ---
		byte *fredRect;
		switch (_fredSprite.animMode) {
		case 6:
		case 8:
			fredRect = _fredFacingLeftRect; // facing/running left
			break;
		case 2:
		case 4:
			fredRect = _fredFacingRightRect; // facing/running right
			break;
		case 7:
			fredRect = _fredTurningRect; // turning left
			break;
		case 3:
			fredRect = _fredTurningRect; // turning right
			break;
		default:
			fredRect = nullptr;
			break;
		}

		if (!fredRect)
			continue;

		// Build Fred's bounding rect...
		int16 fredX = ((int32)_fredSprite.xPos >> 8) + READ_UINT16(fredRect);
		int16 fredY = ((int32)_fredSprite.yPos >> 8) + READ_UINT16(fredRect + 2);
		int16 fredW = READ_UINT16(fredRect + 4);
		int16 fredH = READ_UINT16(fredRect + 6);

		// --- Third pass: check collisions with balloons ---
		idx = 0;
		entry = _fredEntitiesTable[idx];
		while (entry) {
			if ((entry->flags & 1) && entry->animMode == 0x0B) {
				// Build balloon bounding rect...
				int16 bx = ((int32)entry->xPos >> 8) + READ_UINT16(_fredBalloonRect);
				int16 by = ((int32)entry->yPos >> 8) + READ_UINT16(_fredBalloonRect + 2);
				int16 bw = READ_UINT16(_fredBalloonRect + 4);
				int16 bh = READ_UINT16(_fredBalloonRect + 6);

				// {x, y, w, h} rect overlap test...
				Common::Rect balloonRect(bx, by, bx + bw, by + bh);
				Common::Rect playerRect(fredX, fredY, fredX + fredW, fredY + fredH);

				if (playerRect.intersects(balloonRect)) {
					// Caught a balloon!
					int16 catchMode;
					if (_fredSprite.animMode == 6 || _fredSprite.animMode == 8 || _fredSprite.animMode == 7)
						catchMode = 9; // catch facing left
					else
						catchMode = 5; // catch facing right

					setFredAnimMode(&_fredSprite, catchMode);
					setFredAnimMode(entry, 0x0A); // balloon disappearing

					_fredLevelIndex++;
					if (_fredLevelIndex >= READ_UINT16(_fredLevelPtr + 0x0A)) {
						allCaught = 1;
						result = 0x10;
					}

					// Reset spawn timer...
					if (spawnBalloon())
						_fredBalloonSpawnDelay = calcBalloonSpawnDelay();
					else
						_fredBalloonSpawnDelay = 1;

					break; // Only catch one per frame...
				}
			}

			idx++;
			entry = _fredEntitiesTable[idx];
		}
	}

	_xp->stopSound();
	_xp->disableController();
	return result;
}

int16 BoltEngine::helpFred() {
	byte *firstEntry = getResolvedPtr(_fredHelpEntries, 0);
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

	_fredLoopSound = nullptr;
	_fredCurrentSound = nullptr;
	_fredPendingLoop = nullptr;
	_fredPendingOneShot = nullptr;

	renderFredScene();

	// Display all help entry pics on front surface...
	int16 off = 0;
	byte *entry;
	while (true) {
		entry = getResolvedPtr(_fredHelpEntries, off);
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
		entry = getResolvedPtr(_fredHelpEntries, off);
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
	_fredHoveredEntry = getResolvedPtr(_fredHelpEntries, 0);
	hiliteFredHelpObject(getResolvedPtr(_fredHelpEntries, 0), 1);

	// Main help loop
	while (exitResult == -1) {
		// Handle audio playback...
		if (isPlaying) {
			if (!maintainAudioPlay(soundPending)) {
				int16 highlight = (playableEntry == _fredHoveredEntry) ? 1 : 0;
				hiliteFredHelpObject(playableEntry, highlight);
				isPlaying = 0;
			}
			soundPending = 0;
		}

		// Process events...
		uint32 eventData;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);

		switch (eventType) {
		case etTimer: {
			// Timer expired, toggle selected entry highlight...
			if (!_fredCurrentHelpObject)
				break;

			if (_fredTimer != eventData)
				break;

			_fredTimer = _xp->startTimer(500);

			byte *sel = _fredCurrentHelpObject;
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
				entry = getResolvedPtr(_fredHelpEntries, off);
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
			if (hitEntry == _fredHoveredEntry)
				break;

			// Unhighlight previous...
			if (_fredHoveredEntry &&
				_fredHoveredEntry != _fredCurrentHelpObject &&
				!(_fredHoveredEntry == playableEntry && isPlaying)) {
				hiliteFredHelpObject(_fredHoveredEntry, 0);
			}

			_fredHoveredEntry = hitEntry;

			// Highlight new...
			if (hitEntry && hitEntry != _fredCurrentHelpObject) {
				hiliteFredHelpObject(hitEntry, 1);
			}

			break;
		}
		case etMouseDown: {
			int16 justStopped = 0;

			// Stop current animation if playing...
			if (isPlaying) {
				if (_fredTimer) {
					_xp->killTimer(_fredTimer);
					_fredTimer = 0;
				}

				int16 hl = (playableEntry == _fredHoveredEntry) ? 1 : 0;
				hiliteFredHelpObject(playableEntry, hl);

				hl = (_fredCurrentHelpObject == _fredHoveredEntry) ? 1 : 0;
				hiliteFredHelpObject(_fredCurrentHelpObject, hl);

				_fredCurrentHelpObject = nullptr;
				stopAnimation();
				isPlaying = 0;
				justStopped = 1;
			}

			// Handle click on hovered entry...
			if (!_fredHoveredEntry)
				break;

			uint32 entryType = READ_UINT32(_fredHoveredEntry);

			if (entryType == 0) {
				// Exit help...
				exitResult = 0;
			} else if (entryType == 1) {
				// Exit help and clear show-help flag...
				_fredShowHelp = 0;
				exitResult = 1;
			} else if (entryType == 2) {
				// Play animation...
				if (!isPlaying && !justStopped) {
					if (startAnimation(_rtfHandle, 0x1B)) {
						_fredHelpStep = 0;
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
		entry = getResolvedPtr(_fredHelpEntries, off);
		if (!entry)
			break;
		int16 hl = (entry == _fredHoveredEntry) ? 1 : 0;
		hiliteFredHelpObject(entry, hl);
		off += 4;
	}

	_xp->updateDisplay();

	// Unhighlight all entries
	off = 0;
	while (true) {
		entry = getResolvedPtr(_fredHelpEntries, off);
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
	if (_fredHelpStep < 0 || _fredHelpStep >= 4)
		return;

	// Unhighlight previous selected if not hovered...
	int16 hl = (_fredCurrentHelpObject == _fredHoveredEntry) ? 1 : 0;
	hiliteFredHelpObject(_fredCurrentHelpObject, hl);

	// On even steps, select entry from _fredPlayButton table; on odd steps, deselect...
	if (!(_fredHelpStep & 1)) {
		int16 idx = _fredHelpStep >> 1;
		_fredCurrentHelpObject = getResolvedPtr(_fredPlayButton, idx * 4);
	} else {
		_fredCurrentHelpObject = nullptr;
	}

	_fredTimer = 0;

	// Highlight new selected entry...
	hiliteFredHelpObject(_fredCurrentHelpObject, 1);

	// Start/stop blink timer...
	if (!(_fredHelpStep & 1)) {
		_fredTimer = _xp->startTimer(500);
	} else {
		if (_fredTimer) {
			_xp->killTimer(_fredTimer);
			_fredTimer = 0;
		}
	}

	_fredHelpStep++;
}

bool BoltEngine::spawnBalloon() {
	// Count active balloons...
	int16 activeCount = 0;
	int16 i = 0;

	FredEntityState *entry;
	while (true) {
		entry = _fredEntitiesTable[i];
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
	int16 searchStart = _fredBalloonSearchIdx;
	int16 wrapped = 0;
	i = searchStart;

	while (true) {
		entry = _fredEntitiesTable[i];
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

	_fredBalloonSearchIdx = i + 1;

	// Pick row from shuffle table...
	int16 row = selectBalloonRow();

	uint16 numCols = READ_UINT16(_fredLevelPtr + 6);
	int16 stride = numCols + 1;
	int16 rowBase = stride * row;

	// Get next column from shuffle table...
	byte *shuffleRow = _fredShuffleTable + rowBase;
	byte colCounter = shuffleRow[0];
	byte col = _fredShuffleTable[rowBase + colCounter + 1];

	// Look up path table: _fredPathMatrix[row][col]
	byte *rowPaths = getResolvedPtr(_fredPathMatrix, row * 4);
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
	int16 range = READ_UINT16(_fredLevelPtr + 2);
	int16 base = READ_UINT16(_fredLevelPtr + 0);
	return _xp->getRandom(range) + base;
}

int16 BoltEngine::selectBalloonRow() {
	// Get Fred's center X position...
	int32 fredX = _fredSprite.xPos >> 8;

	// Get Fred's sprite width/2 to find center...
	int16 frameIdx = _fredSprite.frameIndex;
	byte *spriteDesc = getResolvedPtr(_fredSprite.animTable, frameIdx * 6);
	int16 halfWidth = READ_UINT16(spriteDesc + 0x0A) >> 1;
	int16 fredCenter = (int16)fredX + halfWidth;

	// Find which row Fred is in...
	int16 row = 0;
	byte *rowBounds = _fredRowBounds; // Array of int16 x-values
	uint16 numRows = READ_UINT16(_fredLevelPtr + 4);

	while (row < numRows) {
		if (READ_UINT16(rowBounds + row * 2) > fredCenter)
			break;
		row++;
	}

	// Pick random row, rejecting if too close to Fred's row...
	int16 rowBias = READ_UINT16(_fredLevelPtr + 8);
	int16 randomRow;
	do {
		randomRow = _xp->getRandom(numRows);
	} while (randomRow >= row - rowBias && randomRow <= row + rowBias);

	return randomRow;
}

void BoltEngine::setFredAnimMode(FredEntityState *state, int16 mode) {
	if (mode >= 0x0B && mode < 0x0B + (_fredLevelPtr ? READ_UINT16(_fredLevelPtr + 0x0C) : 0)) {
		state->animMode = 0x0B;

		int16 typeIdx = mode - 0x0B;
		state->animTable = getResolvedPtr(_fredBalloonSprite, typeIdx * 4);
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
	state->animTable = _fredSprites[mode];

	state->frameIndex = 0;
	state->frameCountdown = READ_UINT16(state->animTable + 4);

	// Play sound based on mode...
	switch (mode) {
	case 3: // turn right
		playFredSound(&_fredSounds[2], nullptr);
		break;
	case 4: // run right
		playFredSound(&_fredSounds[0], &_fredSounds[0]);
		break;
	case 5: // catch right
		playFredSound(&_fredSounds[3], nullptr);
		break;
	case 7: // turn left
		playFredSound(&_fredSounds[2], nullptr);
		break;
	case 8: // run left
		playFredSound(&_fredSounds[1], &_fredSounds[1]);
		break;
	case 9: // catch left
		playFredSound(&_fredSounds[3], nullptr);
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
		FredEntityState *entry = _fredEntitiesTable[idx];
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
				displayPic(_fredBalloonString, x + 10, y + 25, stFront);
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
	_fredPendingOneShot = oneShot;
	_fredPendingLoop = loop; 

	if (_fredLoopSound != nullptr || loop == nullptr) {
		// Check if current loop is _fredSounds[2] and new one-shot is _fredSounds[1]
		// (sound transition check)
		if (_fredCurrentSound == &_fredSounds[3] && _fredPendingOneShot == &_fredSounds[2]) {
			// Allow transition without stopping...
		} else {
			_fredLoopSound = nullptr;
			_fredCurrentSound = nullptr;
			_xp->stopSound();
		}
	}

	// If there's a pending sound and nothing currently playing, trigger it!
	if (_fredPendingOneShot != nullptr && _fredCurrentSound == nullptr) {
		updateFredSound();
	}
}

void BoltEngine::updateFredSound() {
	bool startLoop = false;

	if (_fredLoopSound != nullptr) {
		// Loop sound active, continue with it...
		_fredCurrentSound = _fredLoopSound;
	} else if (_fredPendingOneShot != nullptr) {
		// Start pending one-shot...
		_fredCurrentSound = _fredPendingOneShot;
		_fredLoopSound = _fredPendingLoop;
		startLoop = (_fredPendingLoop != nullptr);
		_fredPendingLoop = 0;
		_fredPendingOneShot = 0;
	} else {
		// Nothing to play...
		_fredCurrentSound = nullptr;
	}

	if (_fredCurrentSound != nullptr) {
		// Play the current sound...
		SoundInfo *snd = _fredCurrentSound;
		_xp->playSound(snd->data, snd->size, 22050);

		if (startLoop) {
			// Queue loop sound twice...
			SoundInfo *loopSnd = _fredLoopSound;
			_xp->playSound(loopSnd->data, loopSnd->size, 22050);
			_xp->playSound(loopSnd->data, loopSnd->size, 22050);
		}
	}
}

} // End of namespace Bolt
