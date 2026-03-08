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

#include "common/memstream.h"

namespace Bolt {

bool BoltEngine::loadScoobyBaseAssets() {
	if (!getBOLTGroup(g_scoobyBoltLib, 0, 1))
		return false;

	g_scoobyBaseData = memberAddr(g_scoobyBoltLib, 0);
	return true;
}

void BoltEngine::cleanUpScoobyBaseAssets() {
	freeBOLTGroup(g_scoobyBoltLib, 0, 1);
}

void BoltEngine::displayPicClipHack(byte *pic, int16 offsetX, int16 offsetY, int16 *clipRect, int16 displayMode) {
	int16 clipW = clipRect[2];
	int16 clipH = clipRect[3];

	int16 picWidth = READ_UINT16(pic + 0x0A);
	byte *pixelData = getResolvedPtr(pic, 0x12);

	int32 srcOffset = (clipRect[0] - g_displayX) + picWidth * (clipRect[1] - g_displayY);

	_xp->blit(pixelData + srcOffset, picWidth, g_scoobyTempPic.pixelData, clipW, clipW, clipH);

	g_scoobyTempPic.width = clipW;
	g_scoobyTempPic.height = clipH;
	g_scoobyTempPic.palette = nullptr;
	g_scoobyTempPic.paletteStart = 0;
	g_scoobyTempPic.paletteCount = 0;
	g_scoobyTempPic.flags = 0;

	if (*pic & 2)
		g_scoobyTempPic.flags |= 2;

	_xp->displayPic(&g_scoobyTempPic,
					READ_UINT16(pic + 6) + offsetX + clipRect[0],
					READ_UINT16(pic + 8) + offsetY + clipRect[1],
					displayMode);
}

void BoltEngine::drawMovingWalls(int16 cellIdx, int16 direction, int16 picFrame, int16 bgFrame) {
	// Clamp frames to valid range
	if (picFrame > 5)
		picFrame = 5;

	if (bgFrame > 4)
		bgFrame = 4;

	if (picFrame < 0)
		picFrame = 0;

	if (bgFrame < 0)
		bgFrame = 0;

	int16 wallMarginX = READ_UINT16(g_scoobyLevelData + 0x0E);
	int16 wallMarginY = READ_UINT16(g_scoobyLevelData + 0x10);

	int16 posX = 0, posY = 0;

	// Compute position along wall's primary axis
	switch (direction) {
	case 0: // up wall
		posY = g_scoobyCellBounds[cellIdx].top - 1 - wallMarginY;
		break;
	case 1: // right wall
		posX = g_scoobyCellBounds[cellIdx].right - wallMarginX;
		break;
	case 2: // down wall
		posY = g_scoobyCellBounds[cellIdx].bottom - wallMarginY;
		break;
	case 3: // left wall
		posX = g_scoobyCellBounds[cellIdx].left - 1 - wallMarginX;
		break;
	default:
		break;
	}

	// Draw wall based on orientation
	switch (direction) {
	case 1: // right wall (vertical)
	case 3: // left wall (vertical)
	{
		posY = g_scoobyCellBounds[cellIdx].top + wallMarginY + 1;

		byte *basePic = g_scoobyWallPicsA[3];
		int16 picW = READ_UINT16(basePic + 0x0A);
		int16 picH = READ_UINT16(basePic + 0x0C);

		int16 clipRect[4];
		clipRect[0] = posX;
		clipRect[1] = posY;
		clipRect[2] = picW;
		clipRect[3] = picH;

		displayPicClipHack(g_scoobyBgPic, 0, 0, clipRect, 1);

		if (bgFrame != 0) {
			displayPic(g_scoobyWallPicsA[bgFrame - 1], posX, posY, 1);
		}

		break;
	}

	case 0: // up wall (horizontal)
	case 2: // down wall (horizontal)
	{
		posX = g_scoobyCellBounds[cellIdx].left + wallMarginX + 1;

		byte *basePic = g_scoobyWallPicsB[4];
		int16 picW = READ_UINT16(basePic + 0x0A);
		int16 picH = READ_UINT16(basePic + 0x0C);

		int16 clipRect[4];
		clipRect[0] = posX;
		clipRect[1] = posY;
		clipRect[2] = picW;
		clipRect[3] = picH;

		displayPicClipHack(g_scoobyBgPic, 0, 0, clipRect, 1);

		if (picFrame != 0) {
			displayPic(g_scoobyWallPicsB[picFrame - 1], posX, posY, 1);
		}

		break;
	}

	default:
		break;
	}
}

void BoltEngine::drawAllMovingWalls() {
	ScoobyState *state = &g_scoobyGameState;

	// Phase 1: Draw open passages between adjacent cells (both sides state 2)
	for (int16 i = 0; i < 25; i++) {
		int16 col = i % 5;

		// Left wall: this cell's left and left neighbor's right both open
		if (col != 0) {
			if (state->wallStates[i][3] == 2 &&
				state->wallStates[i - 1][1] == 2) {
				drawMovingWalls(i, 3, 5, 4);
			}
		}

		// Right wall: this cell's right and right neighbor's left both open
		if (col != 4) {
			if (state->wallStates[i][1] == 2 &&
				state->wallStates[i + 1][3] == 2) {
				drawMovingWalls(i, 1, 5, 4);
			}
		}

		// Up wall: this cell's up and upper neighbor's down both open
		if (i > 4) {
			if (state->wallStates[i][0] == 2 &&
				state->wallStates[i - 5][2] == 2) {
				drawMovingWalls(i, 0, 5, 4);
			}
		}

		// Down wall: this cell's down and lower neighbor's up both open
		if (i < 20) {
			if (state->wallStates[i][2] == 2 &&
				state->wallStates[i + 5][0] == 2) {
				drawMovingWalls(i, 2, 5, 4);
			}
		}
	}

	// Phase 2: For non-current cells, draw closed moveable walls (state 1)
	// Skip if wall connects to current cell and matching border is also 1
	for (int16 i = 0; i < 25; i++) {
		if (i == state->scoobyCell)
			continue;

		for (int16 dir = 0; dir < 4; dir++) {
			if (state->wallStates[i][dir] != 1)
				continue;

			bool shouldDraw = true;

			if (i == state->leftNeighbor && dir == 1) {
				if (state->wallStates[state->scoobyCell][3] == 1)
					shouldDraw = false;
			} else if (i == state->rightNeighbor && dir == 3) {
				if (state->wallStates[state->scoobyCell][1] == 1)
					shouldDraw = false;
			} else if (i == state->upNeighbor && dir == 2) {
				if (state->wallStates[state->scoobyCell][0] == 1)
					shouldDraw = false;
			} else if (i == state->downNeighbor && dir == 0) {
				if (state->wallStates[state->scoobyCell][2] == 1)
					shouldDraw = false;
			}

			if (shouldDraw)
				drawMovingWalls(i, dir, 5, 4);
		}
	}

	// Phase 3: Draw dynamic walls (state 3) for the current cell
	for (int16 dir = 0; dir < 4; dir++) {
		if (state->wallStates[state->scoobyCell][dir] == 3) {
			drawMovingWalls(state->scoobyCell, dir, 5, 4);
		}
	}
}

void BoltEngine::animateTransition(int16 memberIdx) {
	g_scoobyGameState.spriteFrameCount = 1;

	g_scoobyGameState.frameData[0] = memberAddr(g_scoobyBoltLib, memberIdx - 1);

	setSSpriteFrames(&g_scoobySprite, g_scoobyGameState.spriteFrameCount, g_scoobyGameState.frameData, 1);

	g_scoobyGameState.velocityX = 0;
	g_scoobyGameState.velocityY = 0;
}

void BoltEngine::clearPictMSb(byte *pic) {
	// Check if bit 8 of flags word at pic+0 is set (byte at pic+1, bit 0)
	if (READ_UINT16(pic) & 0x100)
		return;

	byte *pixelData = getResolvedPtr(pic, 0x12);
	uint32 size = (uint32)READ_UINT16(pic + 0x0A) * (uint32)READ_UINT16(pic + 0x0C);

	for (uint32 i = 0; i <= size; i++) {
		pixelData[i] &= 0x7F;
	}
}

void BoltEngine::initScoobyLevelGraphics() {
	// Get palette from level data
	byte *palette = memberAddr(g_scoobyBoltLib, READ_UINT16(g_scoobyLevelData));

	// Get background pic based on display mode
	int16 picMember;
	if (g_displayMode != 0) {
		picMember = READ_UINT16(g_scoobyLevelData + 4);
	} else {
		picMember = READ_UINT16(g_scoobyLevelData + 2);
	}

	g_scoobyBgPic = memberAddr(g_scoobyBoltLib, picMember);
	clearPictMSb(g_scoobyBgPic);

	// Drain timer events
	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	// Display palette to front surface
	displayColors(palette, stBack, 0);

	// Display background to front surface
	displayPic(g_scoobyBgPic, g_displayX, g_displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);

	// Display palette to both back surfaces
	displayColors(palette, stFront, 0);
	displayColors(palette, stBack, 1);

	// Display background to back surface
	displayPic(g_scoobyBgPic, g_displayX, g_displayY, stBack);

	drawAllMovingWalls();

	// Setup Scooby sprite
	g_scoobyGameState.spriteFrameCount = 1;

	int16 spriteMember;
	if (g_scoobyGameState.direction == 6) {
		spriteMember = 0x23;
	} else {
		spriteMember = 0x22;
	}

	g_scoobyGameState.frameData[0] = memberAddr(g_scoobyBoltLib, spriteMember);

	setUpSSprite(&g_scoobySprite, g_scoobyGameState.spriteFrameCount, g_scoobyGameState.frameData, 1, 0, 0);
	setSSpriteFrames(&g_scoobySprite, g_scoobyGameState.spriteFrameCount, g_scoobyGameState.frameData, 1);
	setSSpriteVelocity(&g_scoobySprite, g_scoobyGameState.velocityX, g_scoobyGameState.velocityY);
	displaySSprite(&g_scoobySprite, g_scoobyGameState.scoobyX, g_scoobyGameState.scoobyY);
	animateSSprite(&g_scoobySprite, 0);

	// Setup palette cycle
	XPCycleState cycleData[4];
	byte *cycleMember = memberAddr(g_scoobyBoltLib, READ_UINT16(g_scoobyLevelData + 0x0A));
	boltCycleToXPCycle(cycleMember, cycleData);
	_xp->startCycle(cycleData);

	_xp->updateDisplay();
}

bool BoltEngine::initScoobyLevelAssets() {
	uint32 maxPicSize = 0;

	// Compute graphics group index from level number
	g_scoobySelectedGraphicsGroup = (g_scoobyGameState.levelNumber - 1) * 0x100 + 0x100;

	if (!getBOLTGroup(g_scoobyBoltLib, g_scoobySelectedGraphicsGroup, 1))
		return false;

	g_scoobyLevelData = memberAddr(g_scoobyBoltLib, g_scoobySelectedGraphicsGroup);

	// Load wall pic set A (4 pics at level data offset +6)
	for (int16 i = 0; i < 4; i++) {
		int16 memberIdx = READ_UINT16(g_scoobyLevelData + 6) + i;
		g_scoobyWallPicsA[i] = memberAddr(g_scoobyBoltLib, memberIdx);
		clearPictMSb(g_scoobyWallPicsA[i]);

		uint32 picSize = (uint32)READ_UINT16(g_scoobyWallPicsA[i] + 0x0A) *
						 (uint32)READ_UINT16(g_scoobyWallPicsA[i] + 0x0C);
		if (picSize > maxPicSize)
			maxPicSize = picSize;
	}

	// Load wall pic set B (5 pics at level data offset +8)
	for (int16 i = 0; i < 5; i++) {
		int16 memberIdx = READ_UINT16(g_scoobyLevelData + 8) + i;
		g_scoobyWallPicsB[i] = memberAddr(g_scoobyBoltLib, memberIdx);
		clearPictMSb(g_scoobyWallPicsB[i]);

		uint32 picSize = (uint32)READ_UINT16(g_scoobyWallPicsB[i] + 0x0A) *
						 (uint32)READ_UINT16(g_scoobyWallPicsB[i] + 0x0C);
		if (picSize > maxPicSize)
			maxPicSize = picSize;
	}

	// Allocate temp buffer for largest pic
	g_scoobyTempPic.pixelData = (byte *)_xp->allocMem(maxPicSize);
	if (!g_scoobyTempPic.pixelData)
		return false;

	// Determine effective level count (capped at 10)
	if (g_scoobyGameState.levelNumber > 10) {
		g_scoobyLevelCount = 10;
	} else {
		g_scoobyLevelCount = g_scoobyGameState.levelNumber;
	}

	// Look up difficulty from global save data
	int16 levelIdx = g_scoobyLevelCount - 1;
	int16 globalOffset = levelIdx * 6;
	int16 saveSlot = g_scoobyGameState.slotIndex[levelIdx];
	g_scoobyDifficulty = g_scoobyGlobalSaveData[globalOffset + saveSlot * 2];

	// Load wall states for all 25 cells based on difficulty
	for (int16 i = 0; i < 25; i++) {
		int16 srcOffset;
		switch (g_scoobyDifficulty) {
		case 0:
			srcOffset = 0x1C;
			break;
		case 1:
			srcOffset = 0xE6;
			break;
		case 2:
			srcOffset = 0x1B0;
			break;
		default:
			continue;
		}

		for (int16 j = 0; j < 4; j++) {
			g_scoobyGameState.wallStates[i][j] =
				READ_UINT16(g_scoobyLevelData + i * 8 + j * 2 + srcOffset);
		}
	}

	// Compute cell bounds and start positions for 5x5 grid
	int16 cellW = READ_UINT16(g_scoobyLevelData + 0x16);
	int16 cellH = READ_UINT16(g_scoobyLevelData + 0x18);

	int16 yPos = READ_UINT16(g_scoobyLevelData + 0x14);
	for (int16 row = 0; row < 5; row++) {
		int16 xPos = READ_UINT16(g_scoobyLevelData + 0x12);

		for (int16 col = 0; col < 5; col++) {
			int16 cellIdx = row * 5 + col;

			g_scoobyCellBounds[cellIdx].left = xPos;
			g_scoobyCellBounds[cellIdx].right = xPos + cellW - 1;
			g_scoobyCellBounds[cellIdx].top = yPos;
			g_scoobyCellBounds[cellIdx].bottom = yPos + cellH - 1;

			g_scoobyLevelStartXY[cellIdx].x = xPos + ((uint16)cellW >> 1);
			g_scoobyLevelStartXY[cellIdx].y = yPos + ((uint16)cellH >> 1);

			xPos += cellW;
		}

		yPos += cellH;
	}

	return true;
}

void BoltEngine::cleanUpScoobyLevelGraphics() {
	_xp->stopCycle();
	freeBOLTGroup(g_scoobyBoltLib, g_scoobySelectedGraphicsGroup, 1);
}

void BoltEngine::setScoobySpriteDirection(int16 startMember) {
	g_scoobyGameState.spriteFrameCount = 6;

	int16 memberIdx = startMember;
	for (int16 i = 0; i < 6; i++) {
		g_scoobyGameState.frameData[i] = memberAddr(g_scoobyBoltLib, memberIdx + i);
	}

	setSSpriteFrames(&g_scoobySprite, g_scoobyGameState.spriteFrameCount, g_scoobyGameState.frameData, 1);
}

void BoltEngine::playSoundMapScooby(int16 memberIdx) {
	byte *soundData = getBOLTMember(g_scoobyBoltLib, memberIdx);
	uint32 soundSize = memberSize(g_scoobyBoltLib, memberIdx);

	if (soundData) {
		_xp->playSound(soundData, soundSize, 22050);
	}
}

void BoltEngine::playWallSound() {
	if (g_scoobySoundPlaying == 0)
		return;

	_xp->stopSound();

	int16 soundMember;
	switch (g_scoobySoundPlaying) {
	case 1:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x27C);
		break;
	case 2:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x27E);
		break;
	case 3:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x278);
		break;
	case 4:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x27A);
		break;
	case 5:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x284);
		break;
	case 6:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x286);
		break;
	case 7:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x280);
		break;
	case 8:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x282);
		break;
	case 9:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x288);
		break;
	case 10:
		soundMember = READ_UINT16(g_scoobyLevelData + 0x28A);
		break;
	default:
		return;
	}

	playSoundMapScooby(soundMember);
}

void BoltEngine::animateWalls() {
	// Phase 1: Animate current cell's walls
	int16 curCell = g_scoobyGameState.scoobyCell;

	for (int16 dir = 0; dir < 4; dir++) {
		if (g_scoobyGameState.wallStates[curCell][dir] == 1) {
			// Closed wall, opening animation
			bool shouldDraw = true;

			switch (dir) {
			case 0: // up neighbor
				if (curCell - 5 == g_scoobyGameState.scoobySavedCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobySavedCell][2] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 5;

				break;
			case 1: // right neighbor
				if (curCell + 1 == g_scoobyGameState.scoobySavedCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobySavedCell][3] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 1;

				break;
			case 2: // down neighbor
				if (curCell + 5 == g_scoobyGameState.scoobySavedCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobySavedCell][0] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 5;

				break;
			case 3: // left neighbor
				if (curCell - 1 == g_scoobyGameState.scoobySavedCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobySavedCell][1] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 1;

				break;
			}

			if (shouldDraw) {
				drawMovingWalls(curCell, dir, 5 - g_scoobyWallAnimStep, 4 - g_scoobyWallAnimStep);
			}
		} else if (g_scoobyGameState.wallStates[curCell][dir] == 3) {
			// Dynamic wall, closing animation
			if (dir == 3 || dir == 1) {
				g_scoobySoundPlaying = 4;
			} else {
				g_scoobySoundPlaying = 8;
			}

			drawMovingWalls(curCell, dir, g_scoobyWallAnimStep, g_scoobyWallAnimStep);
		}
	}

	// Phase 2: Animate saved cell's walls
	int16 savedCell = g_scoobyGameState.scoobySavedCell;

	for (int16 dir = 0; dir < 4; dir++) {
		if (g_scoobyGameState.wallStates[savedCell][dir] == 1) {
			// Closed wall in saved cell, closing animation
			bool shouldDraw = true;

			switch (dir) {
			case 0: // up neighbor
				if (savedCell - 5 == g_scoobyGameState.scoobyCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobyCell][2] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 6;

				break;
			case 1: // right neighbor
				if (savedCell + 1 == g_scoobyGameState.scoobyCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobyCell][3] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 2;

				break;
			case 2: // down neighbor
				if (savedCell + 5 == g_scoobyGameState.scoobyCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobyCell][0] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 6;

				break;
			case 3: // left neighbor
				if (savedCell - 1 == g_scoobyGameState.scoobyCell) {
					if (g_scoobyGameState.wallStates[g_scoobyGameState.scoobyCell][1] == 1)
						shouldDraw = false;
				}

				if (shouldDraw)
					g_scoobySoundPlaying = 2;

				break;
			}

			if (shouldDraw) {
				drawMovingWalls(savedCell, dir, g_scoobyWallAnimStep, g_scoobyWallAnimStep);
			}
		} else if (g_scoobyGameState.wallStates[savedCell][dir] == 3) {
			// Dynamic wall in saved cell, opening animation
			if (dir == 3 || dir == 1) {
				g_scoobySoundPlaying = 3;
			} else {
				g_scoobySoundPlaying = 7;
			}

			drawMovingWalls(savedCell, dir, 5 - g_scoobyWallAnimStep, 4 - g_scoobyWallAnimStep);
		}
	}

	// Determine combined sound on first step
	if (g_scoobyWallAnimStep == 1) {
		if (g_scoobyWallsToClose == 0 && g_scoobyWallsToOpen > 1) {
			g_scoobySoundPlaying = 9;
		} else if (g_scoobyWallsToClose > 1 || (g_scoobyWallsToClose == 1 && g_scoobyWallsToOpen >= 1)) {
			g_scoobySoundPlaying = 10;
		}

		playWallSound();
	}
}

void BoltEngine::decideDirection() {
	int16 cell = g_scoobyGameState.scoobyCell;
	int16 targetDir = g_scoobyGameState.transitionTarget;

	int16 primaryPos, secondaryPos, primaryCenter, secondaryCenter;
	int16 wallDir;

	switch (targetDir) {
	case 2: // right
	case 6: // left
		primaryPos = g_scoobyGameState.scoobyX;
		secondaryPos = g_scoobyGameState.scoobyY;
		primaryCenter = g_scoobyLevelStartXY[cell].x;
		secondaryCenter = g_scoobyLevelStartXY[cell].y;
		wallDir = (targetDir == 6) ? 3 : 1;

		break;
	case 0: // up
	case 4: // down
		primaryPos = g_scoobyGameState.scoobyY;
		secondaryPos = g_scoobyGameState.scoobyX;
		primaryCenter = g_scoobyLevelStartXY[cell].y;
		secondaryCenter = g_scoobyLevelStartXY[cell].x;
		wallDir = (targetDir == 0) ? 0 : 2;

		break;
	default:
		return;
	}

	if (secondaryPos == secondaryCenter) {
		// Aligned on secondary axis, can move in target direction
		g_scoobyMoveRequested = 1;

		if (primaryPos == primaryCenter) {
			// At cell center, check wall
			if (g_scoobyGameState.wallStates[cell][wallDir] == 2 ||
				g_scoobyGameState.wallStates[cell][wallDir] == 3) {
				// Open passage or dynamic wall, stop
				g_scoobyGameState.targetVelocityX = 0;
				g_scoobyGameState.targetVelocityY = 0;
				return;
			}
		}

		// Wall closed, set velocity in target direction
		switch (targetDir) {
		case 0: // up
			g_scoobyGameState.targetVelocityX = 0;
			g_scoobyGameState.targetVelocityY = -3;
			return;
		case 2: // right
			g_scoobyGameState.targetVelocityX = 3;
			g_scoobyGameState.targetVelocityY = 0;
			return;
		case 4: // down
			g_scoobyGameState.targetVelocityX = 0;
			g_scoobyGameState.targetVelocityY = 3;
			return;
		case 6: // left
			g_scoobyGameState.targetVelocityX = -3;
			g_scoobyGameState.targetVelocityY = 0;
			return;
		default:
			return;
		}
	} else {
		// Not aligned on secondary axis, slide toward center
		g_scoobyMoveRequested = 1;

		if (g_scoobyGameState.wallStates[cell][wallDir] == 2) {
			g_scoobyGameState.targetVelocityX = 0;
			g_scoobyGameState.targetVelocityY = 0;
			return;
		}

		switch (targetDir) {
		case 2: // right, slide vertically
		case 6: // left
			g_scoobyGameState.targetVelocityX = 0;
			if (g_scoobyLevelStartXY[cell].y < g_scoobyGameState.scoobyY) {
				g_scoobyGameState.transitionTarget = 0;
				g_scoobyGameState.targetVelocityY = -3;
			} else {
				g_scoobyGameState.transitionTarget = 4;
				g_scoobyGameState.targetVelocityY = 3;
			}

			return;
		case 0: // up, slide horizontally
		case 4: // down
			g_scoobyGameState.targetVelocityY = 0;
			if (g_scoobyLevelStartXY[cell].x < g_scoobyGameState.scoobyX) {
				g_scoobyGameState.transitionTarget = 6;
				g_scoobyGameState.targetVelocityX = -3;
			} else {
				g_scoobyGameState.transitionTarget = 2;
				g_scoobyGameState.targetVelocityX = 3;
			}

			return;
		default:
			return;
		}
	}
}

void BoltEngine::updateScoobySound() {
	switch (g_scoobySoundMode) {
	case 0:
		if (g_scoobySoundPlaying == 0) {
			_xp->stopSound();
		}

		break;
	case 1:
		playSoundMapScooby(0x24);
		break;
	case 2:
		playSoundMapScooby(0x25);
		break;
	default:
		break;
	}
}

void BoltEngine::setScoobySound(int16 mode) {
	if (mode != g_scoobySoundMode) {
		g_scoobySoundMode = mode;
		updateScoobySound();
	}
}

void BoltEngine::updateScoobyLocation() {
	Common::Point loc;
	getSSpriteLoc(&g_scoobySprite, &loc);
	g_scoobyGameState.scoobyX = loc.x;
	g_scoobyGameState.scoobyY = loc.y;

	// Find which cell Scooby is in
	g_scoobyGameState.scoobyCell = -1;
	for (int16 i = 0; i < 25; i++) {
		if (g_scoobyCellBounds[i].left > g_scoobyGameState.scoobyX ||
			g_scoobyCellBounds[i].right < g_scoobyGameState.scoobyX ||
			g_scoobyCellBounds[i].top > g_scoobyGameState.scoobyY ||
			g_scoobyCellBounds[i].bottom < g_scoobyGameState.scoobyY) {
			continue;
		}

		// Found cell
		g_scoobyGameState.scoobyCell = i;

		int16 col = i % 5;
		g_scoobyGameState.leftNeighbor = (col == 0) ? -1 : i - 1;
		g_scoobyGameState.rightNeighbor = (col == 4) ? -1 : i + 1;
		g_scoobyGameState.upNeighbor = (i < 5) ? -1 : i - 5;
		g_scoobyGameState.downNeighbor = (i > 19) ? -1 : i + 5;
		break;
	}

	if (g_scoobyGameState.scoobyCell != -1)
		return;

	// Scooby is outside all cells, force movement back toward grid
	int16 gridStartX = READ_UINT16(g_scoobyLevelData + 0x12);
	int16 cellW = READ_UINT16(g_scoobyLevelData + 0x16);
	int16 gridStartY = READ_UINT16(g_scoobyLevelData + 0x14);
	int16 cellH = READ_UINT16(g_scoobyLevelData + 0x18);

	if (gridStartX > g_scoobyGameState.scoobyX) {
		g_scoobyGameState.velocityX = -3;
		g_scoobyGameState.velocityY = 0;
		setScoobySpriteDirection(6);
	} else if (gridStartX + cellW * 5 - 1 < g_scoobyGameState.scoobyX) {
		g_scoobyGameState.velocityX = 3;
		g_scoobyGameState.velocityY = 0;
		setScoobySpriteDirection(0);
	} else if (gridStartY > g_scoobyGameState.scoobyY) {
		g_scoobyGameState.velocityX = 0;
		g_scoobyGameState.velocityY = -3;
		setScoobySpriteDirection(0x0C);
	} else if (gridStartY + (cellH - 1) * 5 - 1 < g_scoobyGameState.scoobyY) {
		g_scoobyGameState.velocityX = 0;
		g_scoobyGameState.velocityY = 3;
		setScoobySpriteDirection(0x12);
	}

	// Scroll Scooby off-screen
	int16 halfW = READ_UINT16(g_scoobyBaseData + 0x0A) / 2;

	while (!shouldQuit()) {
		if (g_displayX - halfW >= g_scoobyGameState.scoobyX)
			break;

		if (g_displayX + g_displayWidth + halfW <= g_scoobyGameState.scoobyX)
			break;

		int16 halfH = READ_UINT16(g_scoobyBaseData + 0x0C) / 2;
		if (g_displayY - halfH >= g_scoobyGameState.scoobyY)
			break;

		if (g_displayY + g_displayHeight + halfH <= g_scoobyGameState.scoobyY)
			break;

		// Still visible, animate one frame
		setSSpriteVelocity(&g_scoobySprite, g_scoobyGameState.velocityX, g_scoobyGameState.velocityY);
		animateSSprite(&g_scoobySprite, 0);
		_xp->updateDisplay();

		uint32 dummy;
		if (_xp->getEvent(etSound, &dummy) == etSound)
			updateScoobySound();

		getSSpriteLoc(&g_scoobySprite, &loc);
		g_scoobyGameState.scoobyX = loc.x;
		g_scoobyGameState.scoobyY = loc.y;
	}

	// Scooby left the screen, level complete
	_xp->stopSound();
	g_scoobyGameState.levelComplete = 1;

	g_scoobyGameState.levelNumber++;
	if (g_scoobyGameState.levelNumber > 12)
		g_scoobyGameState.levelNumber = 10;

	// Cycle slot for current level
	int16 slotIdx = g_scoobyLevelCount - 1;
	g_scoobyGameState.slotIndex[slotIdx]++;
	if (g_scoobyGameState.slotIndex[slotIdx] == 3)
		g_scoobyGameState.slotIndex[slotIdx] = 0;
}

void BoltEngine::updateScoobyWalls() {
	if (g_scoobyWallAnimating != 0) {
		g_scoobyWallAnimStep++;
		if (g_scoobyWallAnimStep > 5) {
			g_scoobyWallAnimating = 0;
			g_scoobyWallAnimStep = 5;
		}

		animateWalls();

		if (g_scoobyWallAnimating == 0) {
			g_scoobyGameState.scoobySavedCell = g_scoobyGameState.scoobyCell;
		}

		return;
	}

	// Not animating, check if Scooby is at cell center
	int16 curCell = g_scoobyGameState.scoobyCell;
	if (g_scoobyLevelStartXY[curCell].x != g_scoobyGameState.scoobyX)
		return;

	if (g_scoobyLevelStartXY[curCell].y != g_scoobyGameState.scoobyY)
		return;

	// At cell center, update active level
	g_scoobyGameState.activeLevel = g_scoobyGameState.scoobyCell;

	// If same cell as saved, no wall changes needed
	if (g_scoobyGameState.scoobyCell == g_scoobyGameState.scoobySavedCell)
		return;

	// Detect wall changes between current and saved cells
	g_scoobyWallsToOpen = 0;
	g_scoobyWallsToClose = 0;

	int16 savedCell = g_scoobyGameState.scoobySavedCell;

	int16 curUp = curCell - 5;
	int16 curLeft = curCell - 1;
	int16 curRight = curCell + 1;
	int16 curDown = curCell + 5;

	int16 savedUp = savedCell - 5;
	int16 savedLeft = savedCell - 1;
	int16 savedRight = savedCell + 1;
	int16 savedDown = savedCell + 5;

	for (int16 dir = 0; dir <= 3; dir++) {
		// Check current cell wall
		if (g_scoobyGameState.wallStates[curCell][dir] == 3) {
			g_scoobyWallAnimating = 1;
			g_scoobyWallAnimStep = 1;
			g_scoobyWallsToClose++;
		} else if (g_scoobyGameState.wallStates[curCell][dir] == 1) {
			g_scoobyWallAnimating = 1;
			g_scoobyWallAnimStep = 1;
			g_scoobyWallsToOpen++;

			bool cancelOpen = false;
			switch (dir) {
			case 0:
				if (curUp == savedCell &&
					g_scoobyGameState.wallStates[savedCell][2] == 1)
					cancelOpen = true;

				break;
			case 1:
				if (curRight == savedCell &&
					g_scoobyGameState.wallStates[savedCell][3] == 1)
					cancelOpen = true;

				break;
			case 2:
				if (curDown == savedCell &&
					g_scoobyGameState.wallStates[savedCell][0] == 1)
					cancelOpen = true;

				break;
			case 3:
				if (curLeft == savedCell &&
					g_scoobyGameState.wallStates[savedCell][1] == 1)
					cancelOpen = true;

				break;
			default:
				break;
			}

			if (cancelOpen)
				g_scoobyWallsToOpen--;
		}

		// Check saved cell wall
		if (g_scoobyGameState.wallStates[savedCell][dir] == 3) {
			g_scoobyWallAnimating = 1;
			g_scoobyWallAnimStep = 1;
			g_scoobyWallsToOpen++;
		} else if (g_scoobyGameState.wallStates[savedCell][dir] == 1) {
			g_scoobyWallAnimating = 1;
			g_scoobyWallAnimStep = 1;
			g_scoobyWallsToClose++;

			bool cancelClose = false;
			switch (dir) {
			case 0:
				if (savedUp == curCell &&
					g_scoobyGameState.wallStates[curCell][2] == 1)
					cancelClose = true;

				break;
			case 1:
				if (savedRight == curCell &&
					g_scoobyGameState.wallStates[curCell][3] == 1)
					cancelClose = true;

				break;
			case 2:
				if (savedDown == curCell &&
					g_scoobyGameState.wallStates[curCell][0] == 1)
					cancelClose = true;

				break;
			case 3:
				if (savedLeft == curCell &&
					g_scoobyGameState.wallStates[curCell][1] == 1)
					cancelClose = true;

				break;
			default:
				break;
			}

			if (cancelClose)
				g_scoobyWallsToClose--;
		}
	}

	if (g_scoobyWallAnimating != 0) {
		animateWalls();
	}
}

void BoltEngine::updateScoobyDirection(int16 inputDir) {
	int16 cell = g_scoobyGameState.scoobyCell;

	// If at cell center, wall passable, and no transition active then accept immediately
	if (g_scoobyLevelStartXY[cell].x == g_scoobyGameState.scoobyX &&
		g_scoobyLevelStartXY[cell].y == g_scoobyGameState.scoobyY &&
		g_scoobyGameState.wallStates[cell][inputDir] != 2 &&
		g_scoobyTransitioning == 0) {
		g_scoobyDesiredDir = inputDir;
		g_scoobyInputHoldCount = 3;
		g_scoobyLastInputDir = inputDir; 
	} else {
		if (g_scoobyLastInputDir == inputDir) {
			g_scoobyInputHoldCount++;
			if (g_scoobyInputHoldCount >= 3 && g_scoobyTransitioning == 0) {
				g_scoobyDesiredDir = g_scoobyLastInputDir;
			}
		} else {
			g_scoobyInputHoldCount = 0;
			g_scoobyLastInputDir = inputDir;
		}
	}

	// Resolve diagonal inputs into cardinal directions
	int16 resolvedDir = 0;

#define WALL_OPEN(c, d) (g_scoobyGameState.wallStates[c][d] == 2 || g_scoobyGameState.wallStates[c][d] == 3)

	switch (g_scoobyDesiredDir) {
	case 0:
	case 2:
	case 4:
	case 6:
		resolvedDir = g_scoobyDesiredDir;
		break;
	case 1: // up-right
		if (WALL_OPEN(cell, 0)) {
			resolvedDir = WALL_OPEN(cell, 1) ? (g_scoobyDesiredDir & 0xFE) : 2;
		} else {
			resolvedDir = WALL_OPEN(cell, 1) ? 0 : (g_scoobyDesiredDir & 0xFE);
		}

		break;
	case 3: // down-right
		if (WALL_OPEN(cell, 2)) {
			resolvedDir = WALL_OPEN(cell, 1) ? (g_scoobyDesiredDir & 0xFE) : 2;
		} else {
			resolvedDir = WALL_OPEN(cell, 1) ? 4 : (g_scoobyDesiredDir & 0xFE);
		}

		break;
	case 5: // down-left
		if (WALL_OPEN(cell, 2)) {
			resolvedDir = WALL_OPEN(cell, 3) ? (g_scoobyDesiredDir & 0xFE) : 6;
		} else {
			resolvedDir = WALL_OPEN(cell, 3) ? 4 : (g_scoobyDesiredDir & 0xFE);
		}

		break;
	case 7: // up-left
		if (WALL_OPEN(cell, 0)) {
			resolvedDir = WALL_OPEN(cell, 3) ? (g_scoobyDesiredDir & 0xFE) : 6;
		} else {
			resolvedDir = WALL_OPEN(cell, 3) ? 0 : (g_scoobyDesiredDir & 0xFE);
		}

		break;
	default:
		break;
	}

#undef WALL_OPEN

	g_scoobyMoveRequested = 0;

	if (g_scoobyDesiredDir == -1) {
		g_scoobyMoveRequested = 1;

		if (g_scoobyGameState.direction == 6) {
			g_scoobyGameState.transitionTarget = -3;
		} else {
			g_scoobyGameState.transitionTarget = -2;
		}

		g_scoobyGameState.targetVelocityX = 0;
		g_scoobyGameState.targetVelocityY = 0;
	} else {
		g_scoobyGameState.transitionTarget = resolvedDir;
		decideDirection();
	}
}

void BoltEngine::updateScoobyTransition() {
	if (g_scoobyTransitioning != 0) {
		// Active transition in progress
		switch (g_scoobyTransitionTarget) {
		case -3:
			animateTransition(0x24);
			g_scoobyTransitioning = 0;
			break;
		case -2:
			animateTransition(0x23);
			g_scoobyTransitioning = 0;
			break;
		case 0:
			setScoobySpriteDirection(0x0C);
			g_scoobyTransitioning = 0;
			break;
		case 2:
			if (g_scoobyTransitionFrom == 4) {
				animateTransition(0x1C);
				g_scoobyTransitionFrom = 0;
			} else {
				setScoobySpriteDirection(0x0);
				g_scoobyTransitioning = 0;
			}
			break;
		case 4:
			if (g_scoobyTransitionFrom == 0) {
				animateTransition(0x19);
				g_scoobyTransitionFrom = 4;
			} else {
				setScoobySpriteDirection(0x12);
				g_scoobyTransitioning = 0;
			}
			break;
		case 6:
			setScoobySpriteDirection(6);
			g_scoobyTransitioning = 0;
			break;
		default:
			break;
		}

		if (g_scoobyTransitioning == 0) {
			// Transition complete, commit state
			switch (g_scoobyTransitionTarget) {
			case 0:
			case 2:
			case 4:
			case 6:
				g_scoobyGameState.velocityX = g_scoobyTransitionVelX;
				g_scoobyGameState.velocityY = g_scoobyTransitionVelY;
				break;
			default:
				break;
			}

			g_scoobyGameState.currentAnim = g_scoobyTransitionTarget;

			if (g_scoobyTransitionTarget == 6) {
				g_scoobyGameState.direction = 6;
			} else if (g_scoobyGameState.currentAnim == 2) {
				g_scoobyGameState.direction = 2;
			}
		}
	} else {
		// No transition active, check if one should start
		if (g_scoobyMoveRequested == 0)
			goto epilogue;

		g_scoobyMoveRequested = 0;

		if (g_scoobyGameState.transitionTarget == g_scoobyGameState.currentAnim) {
			// Same direction, just update velocity
			g_scoobyGameState.velocityX = g_scoobyGameState.targetVelocityX;
			g_scoobyGameState.velocityY = g_scoobyGameState.targetVelocityY;
			goto epilogue;
		}

		// Start new transition
		g_scoobyTransitioning = 1;
		g_scoobyTransitionTarget = g_scoobyGameState.transitionTarget;
		g_scoobyTransitionFrom = g_scoobyGameState.currentAnim;
		g_scoobyTransitionVelX = g_scoobyGameState.targetVelocityX;
		g_scoobyTransitionVelY = g_scoobyGameState.targetVelocityY;

		// Pick transition animation based on current -> target direction.
		switch (g_scoobyGameState.currentAnim) {
		case -3:
		case -2:
			switch (g_scoobyGameState.transitionTarget) {
			case 0:
				if (g_scoobyGameState.currentAnim == -2)
					animateTransition(0x1B);
				else
					animateTransition(0x1C);
				break;
			case 2:
				animateTransition(0x1D);
				break;
			case 4:
				if (g_scoobyGameState.currentAnim == -2)
					animateTransition(0x19);
				else
					animateTransition(0x1A);
				break;
			case 6:
				animateTransition(0x1E);
				break;

			default:
				break;
			}
			break;
		case 0:
			switch (g_scoobyGameState.transitionTarget) {
			case -3:
				animateTransition(0x1C);
				break;
			case -2:
				animateTransition(0x1B);
				break;
			case 2:
				animateTransition(0x1F);
				break;
			case 4:
				animateTransition(0x1B);
				break;
			case 6:
				animateTransition(0x20);
				break;
			default:
				break;
			}
			break;

		case 2:
			switch (g_scoobyGameState.transitionTarget) {
			case -3:
				animateTransition(0x1D);
				break;
			case -2:
				animateTransition(0x1D);
				break;
			case 0:
				animateTransition(0x1F);
				break;
			case 4:
				animateTransition(0x21);
				break;
			case 6:
				animateTransition(0x19);
				break;
			default:
				break;
			}
			break;

		case 4:
			switch (g_scoobyGameState.transitionTarget) {
			case -3:
				animateTransition(0x1A);
				break;
			case -2:
				animateTransition(0x19);
				break;
			case 0:
				animateTransition(0x1A);
				break;
			case 2:
				animateTransition(0x21);
				break;
			case 6:
				animateTransition(0x22);
				break;
			default:
				break;
			}
			break;

		case 6:
			switch (g_scoobyGameState.transitionTarget) {
			case -3:
				animateTransition(0x1E);
				break;
			case -2:
				animateTransition(0x1E);
				break;
			case 0:
				animateTransition(0x20);
				break;
			case 2:

				animateTransition(0x1A);
				break;
			case 4:
				animateTransition(0x22);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

epilogue:
	setSSpriteVelocity(&g_scoobySprite, g_scoobyGameState.velocityX, g_scoobyGameState.velocityY);
	animateSSprite(&g_scoobySprite, 0);

	int16 soundMode;
	if (g_scoobyGameState.currentAnim == -2 || g_scoobyGameState.currentAnim == -3) {
		soundMode = 0;
	} else if (g_scoobyGameState.velocityX == 0 && g_scoobyGameState.velocityY == 0 && g_scoobyTransitioning == 0) {
		soundMode = 2;
	} else {
		soundMode = 1;
	}

	setScoobySound(soundMode);
}

bool BoltEngine::initScoobyLevel() {
	if (!initScoobyLevelAssets())
		return false;

	// Set current level based on difficulty
	int16 level;
	switch (g_scoobyDifficulty) {
	case 0:
		level = READ_UINT16(g_scoobyLevelData + 0x1A);
		break;
	case 1:
		level = READ_UINT16(g_scoobyLevelData + 0xE4);
		break;
	case 2:
		level = READ_UINT16(g_scoobyLevelData + 0x1AE);
		break;
	default:
		goto skipLevelSet;
	}

	g_scoobyGameState.scoobyCell = level;
	g_scoobyGameState.scoobySavedCell = level;

skipLevelSet:
	g_scoobyGameState.activeLevel = g_scoobyGameState.scoobyCell;

	// Compute neighbor levels in a 5-wide grid
	int16 col = g_scoobyGameState.scoobyCell % 5;

	g_scoobyGameState.leftNeighbor = (col == 0) ? -1 : g_scoobyGameState.scoobyCell - 1;
	g_scoobyGameState.rightNeighbor = (col == 4) ? -1 : g_scoobyGameState.scoobyCell + 1;
	g_scoobyGameState.upNeighbor = (g_scoobyGameState.scoobyCell < 5) ? -1 : g_scoobyGameState.scoobyCell - 5;
	g_scoobyGameState.downNeighbor = (g_scoobyGameState.scoobyCell > 19) ? -1 : g_scoobyGameState.scoobyCell + 5;

	g_scoobyGameState.levelComplete = 0;

	// Starting position from level table
	g_scoobyGameState.scoobyX = g_scoobyLevelStartXY[g_scoobyGameState.activeLevel].x;
	g_scoobyGameState.scoobyY = g_scoobyLevelStartXY[g_scoobyGameState.activeLevel].y;

	g_scoobyGameState.velocityX = 0;
	g_scoobyGameState.velocityY = 0;
	g_scoobyGameState.targetVelocityX = 0;
	g_scoobyGameState.targetVelocityY = 0;
	g_scoobyGameState.currentAnim = -2;
	g_scoobyGameState.transitionTarget = -2;
	g_scoobyGameState.direction = 2;

	initScoobyLevelGraphics();
	return true;
}

bool BoltEngine::resumeScoobyLevel() {
	if (!initScoobyLevelAssets())
		return false;

	// Restore level from saved level
	g_scoobyGameState.scoobyCell = g_scoobyGameState.scoobySavedCell;

	// Compute neighbor levels
	int16 col = g_scoobyGameState.scoobyCell % 5;

	g_scoobyGameState.leftNeighbor = (col == 0) ? -1 : g_scoobyGameState.scoobyCell - 1;
	g_scoobyGameState.rightNeighbor = (col == 4) ? -1 : g_scoobyGameState.scoobyCell + 1;
	g_scoobyGameState.upNeighbor = (g_scoobyGameState.scoobyCell < 5) ? -1 : g_scoobyGameState.scoobyCell - 5;
	g_scoobyGameState.downNeighbor = (g_scoobyGameState.scoobyCell > 19) ? -1 : g_scoobyGameState.scoobyCell + 5;

	// Starting position from level table
	g_scoobyGameState.scoobyX = g_scoobyLevelStartXY[g_scoobyGameState.activeLevel].x;
	g_scoobyGameState.scoobyY = g_scoobyLevelStartXY[g_scoobyGameState.activeLevel].y;

	initScoobyLevelGraphics();
	return true;
}

bool BoltEngine::initScooby() {
	_xp->randomize();

	g_scoobyMoveRequested = 0;
	g_scoobyTransitioning = 0;
	g_scoobyWallAnimating = 0;
	g_scoobyDesiredDir = -1;
	g_scoobyInputHoldCount = 0;

	if (!loadScoobyBaseAssets())
		return false;

	if (!vLoad(&g_scoobyGlobalSaveData, "ScoobyGlobal")) {
		// Initialize global save data: 10 levels, 3 slots each
		for (int16 level = 0; level < 10; level++) {
			g_scoobyGameState.slotIndex[level] = 0;

			int16 baseOff = level * 6;
			for (int16 j = 0; j < 3; j++) {
				g_scoobyGlobalSaveData[baseOff + j * 2] = 3;
			}

			// Randomly assign difficulty values 0, 1, 2 to the 3 slots
			for (int16 j = 0; j < 3; j++) {
				int16 slot = _xp->getRandom(3);

				while (g_scoobyGlobalSaveData[baseOff + slot * 2] != 3) {
					slot++;
					if (slot >= 3)
						slot = 0;
				}

				g_scoobyGlobalSaveData[baseOff + slot * 2] = j;
			}
		}
	}

	byte scoobyStateBuf[0x11A];
	if (!vLoad(scoobyStateBuf, "Scooby")) {
		g_scoobyGameState.levelNumber = 1;

		if (!initScoobyLevel())
			return false;
	} else {
		Common::SeekableReadStream *scoobyStateReadStream = new Common::MemoryReadStream(scoobyStateBuf, sizeof(scoobyStateBuf), DisposeAfterUse::NO);

		g_scoobyGameState.levelNumber = scoobyStateReadStream->readSint16BE(); // +0x00

		for (int i = 0; i < 10; i++) // +0x02
			g_scoobyGameState.slotIndex[i] = scoobyStateReadStream->readSint16BE();

		g_scoobyGameState.levelComplete = scoobyStateReadStream->readSint16BE(); // +0x16

		for (int i = 0; i < 25; i++) // +0x18
			for (int j = 0; j < 4; j++)
				g_scoobyGameState.wallStates[i][j] = scoobyStateReadStream->readSint16BE();

		g_scoobyGameState.scoobyCell = scoobyStateReadStream->readSint16BE();       // +0xE0
		g_scoobyGameState.scoobySavedCell = scoobyStateReadStream->readSint16BE();  // +0xE2
		g_scoobyGameState.leftNeighbor = scoobyStateReadStream->readSint16BE();     // +0xE4
		g_scoobyGameState.rightNeighbor = scoobyStateReadStream->readSint16BE();    // +0xE6
		g_scoobyGameState.upNeighbor = scoobyStateReadStream->readSint16BE();       // +0xE8
		g_scoobyGameState.downNeighbor = scoobyStateReadStream->readSint16BE();     // +0xEA
		g_scoobyGameState.activeLevel = scoobyStateReadStream->readSint16BE();      // +0xEC
		g_scoobyGameState.scoobyX = scoobyStateReadStream->readSint16BE();          // +0xEE
		g_scoobyGameState.scoobyY = scoobyStateReadStream->readSint16BE();          // +0xF0
		g_scoobyGameState.velocityX = scoobyStateReadStream->readSint16BE();        // +0xF2
		g_scoobyGameState.velocityY = scoobyStateReadStream->readSint16BE();        // +0xF4
		g_scoobyGameState.targetVelocityX = scoobyStateReadStream->readSint16BE();  // +0xF6
		g_scoobyGameState.targetVelocityY = scoobyStateReadStream->readSint16BE();  // +0xF8
		g_scoobyGameState.transitionTarget = scoobyStateReadStream->readSint16BE(); // +0xFA
		g_scoobyGameState.currentAnim = scoobyStateReadStream->readSint16BE();      // +0xFC
		g_scoobyGameState.direction = scoobyStateReadStream->readSint16BE();        // +0xFE
		g_scoobyGameState.spriteFrameCount = scoobyStateReadStream->readSint16BE(); // +0x100

		for (int i = 0; i < 6; i++) { // +0x102
			scoobyStateReadStream->readUint32BE(); // dummy values
			g_scoobyGameState.frameData[i] = nullptr;
		}

		assert(scoobyStateReadStream->pos() == 0x11A);
		delete scoobyStateReadStream;

		if (g_scoobyGameState.levelComplete != 0) {
			if (!initScoobyLevel())
				return false;
		} else {
			if (!resumeScoobyLevel())
				return false;
		}
	}

	_xp->setFrameRate(12);
	return true;
}

void BoltEngine::cleanUpScooby() {
	vSave(&g_scoobyGlobalSaveData, 0x3C, "ScoobyGlobal");

	// Serialize the game state into a flat 0x11A-byte BE buffer
	byte scoobyStateBuf[0x11A] = {0};
	Common::MemoryWriteStream *scoobyStateWriteStream = new Common::MemoryWriteStream(scoobyStateBuf, sizeof(scoobyStateBuf));

	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.levelNumber); // +0x00

	for (int i = 0; i < 10; i++) // +0x02
		scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.slotIndex[i]);

	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.levelComplete); // +0x16

	for (int i = 0; i < 25; i++) // +0x18
		for (int j = 0; j < 4; j++)
			scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.wallStates[i][j]);

	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.scoobyCell);       // +0xE0
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.scoobySavedCell);  // +0xE2
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.leftNeighbor);     // +0xE4
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.rightNeighbor);    // +0xE6
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.upNeighbor);       // +0xE8
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.downNeighbor);     // +0xEA
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.activeLevel);      // +0xEC
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.scoobyX);          // +0xEE
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.scoobyY);          // +0xF0
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.velocityX);        // +0xF2
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.velocityY);        // +0xF4
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.targetVelocityX);  // +0xF6
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.targetVelocityY);  // +0xF8
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.transitionTarget); // +0xFA
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.currentAnim);      // +0xFC
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.direction);        // +0xFE
	scoobyStateWriteStream->writeSint16BE(g_scoobyGameState.spriteFrameCount); // +0x100

	for (int i = 0; i < 6; i++) // +0x102
		scoobyStateWriteStream->writeUint32BE(0); // pointers saved as zero

	// Sanity check: should be exactly 0x11A
	assert(scoobyStateWriteStream->pos() == 0x11A);

	vSave(scoobyStateBuf, 0x11A, "Scooby");

	cleanUpScoobyLevelGraphics();
	cleanUpScoobyBaseAssets();

	if (g_scoobyTempPic.pixelData) {
		_xp->freeMem(g_scoobyTempPic.pixelData);
		g_scoobyTempPic.pixelData = nullptr;
		g_scoobyTempPic.width = 0;
		g_scoobyTempPic.height = 0;
	}

	_xp->setFrameRate(0);
	_xp->fillDisplay(0, 0);
	_xp->updateDisplay();
}

int16 BoltEngine::helpScooby() {
	int16 selection = 2;
	int16 isPlaying = 0;
	int16 soundParam = 0;
	byte *blinkEntry = nullptr;
	uint32 blinkTimer = 0;
	byte *hoveredEntry = nullptr;
	int16 animFrameIdx = -1;
	byte *backButton = nullptr;
	byte *helpButton = nullptr;

	byte *helpEntries[3];
	byte *helpPics[3];

	// If Scooby is mid-movement, stop and face idle direction
	if (g_scoobyGameState.currentAnim != -2 && g_scoobyGameState.currentAnim != -3) {
		g_scoobyGameState.velocityX = 0;
		g_scoobyGameState.velocityY = 0;

		if (g_scoobyGameState.direction == 2)
			g_scoobyGameState.transitionTarget = -2;
		else
			g_scoobyGameState.transitionTarget = -3;

		g_scoobyDesiredDir = -1;
		g_scoobyInputHoldCount = 0;
		g_scoobyMoveRequested = 1;

		do {
			updateScoobyTransition();
			updateScoobyWalls();
			_xp->updateDisplay();
		} while (g_scoobyTransitioning != 0);

		updateScoobyTransition();
		updateScoobyWalls();
		_xp->updateDisplay();

		updateScoobyTransition();
		updateScoobyWalls();
		_xp->updateDisplay();
	}

	_xp->setFrameRate(0);
	_xp->setInactivityTimer(0);
	animateSSprite(&g_scoobySprite, 0);

	// Load 3 help screen entries
	for (int16 i = 0; i < 3; i++) {
		int16 memberIdx = READ_UINT16(g_scoobyLevelData + 0x0C) + i;
		helpEntries[i] = memberAddr(g_scoobyBoltLib, memberIdx);

		byte *entry = helpEntries[i];
		if (READ_UINT32(entry) == 2)
			backButton = entry;
		if (READ_UINT32(entry) == 1)
			helpButton = entry;

		// Clear alive flag
		WRITE_UINT32(entry + 6, READ_UINT32(entry + 6) & ~1);

		// Load and display pic
		int16 picMember = READ_UINT16(entry + 4);
		helpPics[i] = memberAddr(g_scoobyBoltLib, picMember);
		displayPic(helpPics[i], 0, 0, 0);
	}

	_xp->updateDisplay();

	// Save normal palettes from each help entry
	for (int16 i = 0; i < 3; i++) {
		byte *entry = helpEntries[i];
		_xp->getPalette(READ_UINT16(entry + 0x0A), READ_UINT16(entry + 0x0C), entry + 0x42);
	}

	// Position cursor at bottom-right of first help pic
	int16 cursorX = READ_UINT16(helpPics[0] + 6) + READ_UINT16(helpPics[0] + 0x0A) - 10;
	int16 cursorY = READ_UINT16(helpPics[0] + 8) + READ_UINT16(helpPics[0] + 0x0C) - 10;
	_xp->setCursorPos(cursorX, cursorY);
	_xp->setCursorColor(0, 127, 127);
	_xp->disableController();
	_xp->showCursor();

	hoveredEntry = helpButton;
	hiliteScoobyHelpObject(helpButton, 1);

	// Event loop
	while (!shouldQuit()) {
		// Handle ongoing audio playback
		if (isPlaying != 0) {
			if (!maintainAudioPlay(soundParam)) {
				int16 hl = (backButton == hoveredEntry) ? 1 : 0;
				hiliteScoobyHelpObject(backButton, hl);
				isPlaying = 0;
			}
			soundParam = 0;
		}

		uint32 eventData;
		int16 eventType = _xp->getEvent(0, &eventData);

		switch (eventType) {
		case etTimer: {
			if (blinkEntry == nullptr)
				break;
			if (eventData != blinkTimer)
				break;

			blinkTimer = _xp->startTimer(500);

			int16 hl = (READ_UINT32(blinkEntry + 6) & 1) ? 0 : 1;
			hiliteScoobyHelpObject(blinkEntry, hl);
			break;
		}

		case etMouseMove: {
			int16 mouseX = (int32)eventData >> 16;
			int16 mouseY = (int16)eventData;

			byte *hitEntry = nullptr;
			for (int16 i = 0; i < 3; i++) {
				byte *pic = helpPics[i];
				int16 picX = READ_UINT16(pic + 6);
				int16 picY = READ_UINT16(pic + 8);
				int16 picW = READ_UINT16(pic + 0x0A);
				int16 picH = READ_UINT16(pic + 0x0C);

				if (mouseX <= picX || mouseX >= picX + picW)
					continue;
				if (mouseY <= picY || mouseY >= picY + picH)
					continue;

				hitEntry = helpEntries[i];
				break;
			}

			if (hoveredEntry != hitEntry) {
				// Un-highlight old (unless null, blinking, or back button)
				if (hoveredEntry != nullptr &&
					hoveredEntry != blinkEntry &&
					hoveredEntry != backButton) {
					hiliteScoobyHelpObject(hoveredEntry, 0);
				}

				hoveredEntry = hitEntry;

				// Highlight new (unless null or blinking)
				if (hitEntry != nullptr && hitEntry != blinkEntry) {
					hiliteScoobyHelpObject(hoveredEntry, 1);
				}
			}

			// Un-highlight back button if not actively needed
			if (backButton != nullptr) {
				if (READ_UINT32(backButton + 6) & 1) {
					if (hoveredEntry != backButton &&
						blinkEntry != backButton &&
						isPlaying == 0) {
						hiliteScoobyHelpObject(backButton, 0);
					}
				}
			}
			break;
		}

		case etMouseDown: {
			int16 wasPlaying = isPlaying;

			if (isPlaying != 0) {
				if (blinkTimer != 0) {
					_xp->killTimer(blinkTimer);
					blinkTimer = 0;
				}

				int16 hl = (backButton == hoveredEntry) ? 1 : 0;
				hiliteScoobyHelpObject(backButton, hl);

				hl = (blinkEntry == hoveredEntry) ? 1 : 0;
				hiliteScoobyHelpObject(blinkEntry, hl);

				blinkEntry = nullptr;
				stopAnimation();
				isPlaying = 0;
			}

			if (hoveredEntry == nullptr)
				break;

			selection = READ_UINT16(hoveredEntry);

			if (READ_UINT32(hoveredEntry) != 2)
				break;
			if (wasPlaying != 0)
				break;

			if (startAnimation(g_rtfHandle, 0x1C)) {
				animFrameIdx = 0;
				isPlaying = 1;
			}
			break;
		}

		case etSound:
			soundParam = 1;
			break;

		case etTrigger: {
			if (isPlaying == 0)
				break;
			if (animFrameIdx < 0)
				break;

			int16 hl = (blinkEntry == hoveredEntry) ? 1 : 0;
			hiliteScoobyHelpObject(blinkEntry, hl);

			if (animFrameIdx == 0)
				blinkEntry = helpEntries[1];
			else if (animFrameIdx == 2)
				blinkEntry = helpEntries[2];
			else
				blinkEntry = nullptr;

			if (blinkEntry != nullptr) {
				blinkTimer = _xp->startTimer(500);
			} else {
				if (blinkTimer != 0) {
					_xp->killTimer(blinkTimer);
					blinkTimer = 0;
				}
			}

			hiliteScoobyHelpObject(blinkEntry, 1);
			animFrameIdx++;
			break;
		}

		default:
			break;
		}

		// Exit conditions
		if (selection == 0 || selection == 1)
			break;
	}

	// Clean-up
	_xp->hideCursor();
	_xp->enableController();

	hiliteScoobyHelpObject(helpEntries[0], 0);

	for (int16 i = 0; i < 3; i++) {
		hiliteScoobyHelpObject(helpEntries[i], 0);
	}

	animateSSprite(&g_scoobySprite, 0);
	_xp->updateDisplay();
	_xp->setFrameRate(12);

	return selection;
}

void BoltEngine::hiliteScoobyHelpObject(byte *entry, int16 highlight) {
	if (!entry)
		return;

	if (highlight != 0) {
		// Set highlighted palette
		_xp->setPalette(READ_UINT16(entry + 0x0C), READ_UINT16(entry + 0x0A), entry + 0x0E);
		WRITE_UINT32(entry + 6, READ_UINT32(entry + 6) | 1);
	} else {
		// Set normal palette
		_xp->setPalette(READ_UINT16(entry + 0x0C), READ_UINT16(entry + 0x0A), entry + 0x42);
		WRITE_UINT32(entry + 6, READ_UINT32(entry + 6) & ~1);
	}
}

int16 BoltEngine::xpDirToBOLTDir(uint32 xpDir) {
	// xpDir packs (X << 16 | Y & 0xFFFF) where X,Y are -1/0/1
	// Returns BOLT direction:
	//   0=up,   1=up-right,  2=right, 3=down-right,
	//   4=down, 5=down-left, 6=left,  7=up-left,    -1=none

	static const uint32 dirTable[] = {
		0xFFFF0000, // X=-1, Y= 0 -> 6 (left)
		0xFFFF0001, // X=-1, Y= 1 -> 5 (down-left)
		0xFFFFFFFF, // X=-1, Y=-1 -> 7 (up-left)
		0x00000000, // X= 0, Y= 0 -> -1 (none)
		0x00000001, // X= 0, Y= 1 -> 4 (down)
		0x0000FFFF, // X= 0, Y=-1 -> 0 (up)
		0x00010000, // X= 1, Y= 0 -> 2 (right)
		0x00010001, // X= 1, Y= 1 -> 3 (down-right)
		0x0001FFFF, // X= 1, Y=-1 -> 1 (up-right)
	};

	static const int16 dirMap[] = {6, 5, 7, -1, 4, 0, 2, 3, 1};

	for (int i = 0; i < 9; i++) {
		if (dirTable[i] == xpDir)
			return dirMap[i];
	}

	return 0;
}

int16 BoltEngine::playScooby() {
	int16 inputDir = -1;

	// Check if help screen should show on startup
	if (g_scoobyShowHelp != 0) {
		int16 helpResult = helpScooby();
		if (helpResult == 0)
			return 5; // exit/quit
	}

	g_scoobyShowHelp = 0;
	_xp->enableController();

	// Main game loop
	while (!shouldQuit()) {
		updateScoobyLocation();

		// Level complete?
		if (g_scoobyGameState.levelComplete != 0)
			return 16;

		// Poll for event
		uint32 eventData;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);

		switch (eventType) {
		case etJoystick:
			if (eventData != 0)
				_xp->setInactivityTimer(30);

			inputDir = xpDirToBOLTDir(eventData);
			break;

		case etMouseDown:
		case etInactivity:
			if (helpScooby() == 0)
				return 5;
			g_scoobyMoveRequested = 0;
			break;

		case etSound:
			g_scoobySoundPlaying = 0;
			updateScoobySound();
			break;

		default:
			break;
		}

		updateScoobyDirection(inputDir);
		updateScoobyTransition();
		updateScoobyWalls();
		_xp->updateDisplay();
	}

	return 0;
}

int16 BoltEngine::scoobyGame(int16 prevBooth) {
	int16 result = 5;

	if (!openBOLTLib(&g_scoobyBoltLib, &g_scoobyBoltCallbacks, assetPath("scooby.blt")))
		return result;

	int16 prevInactivity = _xp->setInactivityTimer(30);

	if (initScooby()) {
		result = playScooby();
	}

	cleanUpScooby();
	_xp->setInactivityTimer(prevInactivity);
	closeBOLTLib(&g_scoobyBoltLib);

	return result;
}

void BoltEngine::swapScoobyHelpEntry() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (data == nullptr)
		return;

	WRITE_UINT32(data, READ_BE_UINT32(data));
	WRITE_UINT16(data + 4, READ_BE_UINT16(data + 4));
	WRITE_UINT32(data + 6, READ_BE_UINT32(data + 6));
	WRITE_UINT16(data + 0x0A, READ_BE_UINT16(data + 0x0A));
	WRITE_UINT16(data + 0x0C, READ_BE_UINT16(data + 0x0C));
	unpackColors(13, data + 0x0E);
}

void BoltEngine::swapScoobyWordArray() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (data == nullptr)
		return;

	int16 count = g_boltCurrentMemberEntry->decompSize >> 1;

	for (int16 i = 0; i < count; i++) {
		WRITE_UINT16(data, READ_BE_UINT16(data));
		data += 2;
	}
}

} // End of namespace Bolt
