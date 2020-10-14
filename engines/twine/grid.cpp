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

#include "twine/grid.h"
#include "common/textconsole.h"
#include "twine/actor.h"
#include "twine/collision.h"
#include "twine/interface.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/twine.h"

namespace TwinE {

/** Grip X size */
#define GRID_SIZE_X 64
/** Grip Y size */
#define GRID_SIZE_Y 25
/** Grip Z size */
#define GRID_SIZE_Z GRID_SIZE_X

Grid::Grid(TwinEEngine *engine) : _engine(engine) {}

void Grid::copyGridMask(int32 index, int32 x, int32 y, uint8 *buffer) {
	uint8 *ptr = brickMaskTable[index];

	int32 left = x + *(ptr + 2);
	int32 top = y + *(ptr + 3);
	int32 right = *ptr + left - 1;
	int32 bottom = *(ptr + 1) + top - 1;

	if (left > _engine->_interface->textWindowRight || right < _engine->_interface->textWindowLeft || bottom < _engine->_interface->textWindowTop || top > _engine->_interface->textWindowBottom)
		return;

	ptr += 4;

	int32 absX = left;
	int32 absY = top;

	int32 vSize = (bottom - top) + 1;

	if (vSize <= 0)
		return;

	int32 offset = -((right - left) - SCREEN_WIDTH) - 1;

	right++;
	bottom++;

	// if line on top aren't in the blitting area...
	if (absY < _engine->_interface->textWindowTop) {
		int numOfLineToRemove = _engine->_interface->textWindowTop - absY;

		vSize -= numOfLineToRemove;
		if (vSize <= 0)
			return;

		absY += numOfLineToRemove;

		do {
			int lineDataSize;

			lineDataSize = *(ptr++);
			ptr += lineDataSize;
		} while (--numOfLineToRemove);
	}

	// reduce the vSize to remove lines on bottom
	if (absY + vSize - 1 > _engine->_interface->textWindowBottom) {
		vSize = _engine->_interface->textWindowBottom - absY + 1;
		if (vSize <= 0)
			return;
	}

	uint8 *outPtr = _engine->frontVideoBuffer + _engine->screenLookupTable[absY] + left;
	uint8 *inPtr = buffer + _engine->screenLookupTable[absY] + left;

	do {
		int32 vc3 = *(ptr++);

		do {
			int32 temp = *(ptr++); // skip size
			outPtr += temp;
			inPtr += temp;

			absX += temp;

			vc3--;
			if (!vc3)
				break;

			temp = *(ptr++); // copy size

			for (int32 j = 0; j < temp; j++) {
				if (absX >= _engine->_interface->textWindowLeft && absX <= _engine->_interface->textWindowRight)
					*outPtr = *inPtr;

				absX++;
				outPtr++;
				inPtr++;
			}
		} while (--vc3);

		absX = left;

		outPtr += offset;
		inPtr += offset;
	} while (--vSize);
}

void Grid::drawOverModelActor(int32 X, int32 Y, int32 Z) {
	const int32 copyBlockPhysLeft = ((_engine->_interface->textWindowLeft + 24) / 24) - 1;
	const int32 copyBlockPhysRight = ((_engine->_interface->textWindowRight + 24) / 24);

	for (int32 j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (int32 i = 0; i < brickInfoBuffer[j]; i++) {
			BrickEntry *currBrickEntry = &bricksDataBuffer[j][i];

			if (currBrickEntry->posY + 38 > _engine->_interface->textWindowTop && currBrickEntry->posY <= _engine->_interface->textWindowBottom && currBrickEntry->y >= Y) {
				if (currBrickEntry->x + currBrickEntry->z > Z + X) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->workVideoBuffer);
				}
			}
		}
	}
}

void Grid::drawOverSpriteActor(int32 X, int32 Y, int32 Z) {
	int32 copyBlockPhysLeft;
	int32 copyBlockPhysRight;
	int32 i;
	int32 j;
	BrickEntry *currBrickEntry;

	copyBlockPhysLeft = ((_engine->_interface->textWindowLeft + 24) / 24) - 1;
	copyBlockPhysRight = (_engine->_interface->textWindowRight + 24) / 24;

	for (j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (i = 0; i < brickInfoBuffer[j]; i++) {
			currBrickEntry = &bricksDataBuffer[j][i];

			if (currBrickEntry->posY + 38 > _engine->_interface->textWindowTop && currBrickEntry->posY <= _engine->_interface->textWindowBottom && currBrickEntry->y >= Y) {
				if ((currBrickEntry->x == X) && (currBrickEntry->z == Z)) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->workVideoBuffer);
				}

				if ((currBrickEntry->x > X) || (currBrickEntry->z > Z)) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->workVideoBuffer);
				}
			}
		}
	}
}

int Grid::processGridMask(uint8 *buffer, uint8 *ptr) {
	uint32 *ptrSave = (uint32 *)ptr;
	int32 ebx = *((uint32 *)buffer); // brick flag
	buffer += 4;
	*((uint32 *)ptr) = ebx;
	ptr += 4;

	uint8 bh = (ebx & 0x0000FF00) >> 8;

	uint8 *esi = (uint8 *)buffer;
	uint8 *edi = (uint8 *)ptr;

	uint8 iteration = 0;

	do {
		uint8 numOfBlock = 0;
		uint8 ah = 0;
		uint8 *ptr2 = edi;

		edi++;

		uint8 bl = *(esi++);

		if (*(esi)&0xC0) { // the first time isn't skip. the skip size is 0 in that case
			*edi++ = 0;
			numOfBlock++;
		}

		do {
			uint8 al = *esi++;
			iteration = al;
			iteration &= 0x3F;
			iteration++;

			if (al & 0x80) {
				ah += iteration;
				esi++;
			} else if (al & 0x40) {
				ah += iteration;
				esi += iteration;
			} else { // skip
				if (ah) {
					*edi++ = ah; // write down the number of pixel passed so far
					numOfBlock++;
					ah = 0;
				}
				*(edi++) = iteration; //write skip
				numOfBlock++;
			}
		} while (--bl > 0);

		if (ah) {
			*edi++ = ah;
			numOfBlock++;

			ah = 0;
		}

		*ptr2 = numOfBlock;
	} while (--bh > 0);

	return ((int)((uint8 *)edi - (uint8 *)ptrSave));
}

void Grid::createGridMask() {
	for (int32 b = 0; b < NUM_BRICKS; b++) {
		if (!brickUsageTable[b]) {
			continue;
		}
		if (brickMaskTable[b])
			free(brickMaskTable[b]);
		brickMaskTable[b] = (uint8 *)malloc(brickSizeTable[b]);
		processGridMask(brickTable[b], brickMaskTable[b]);
	}
}

void Grid::getSpriteSize(int32 offset, int32 *width, int32 *height, uint8 *spritePtr) {
	spritePtr += *((int32 *)(spritePtr + offset * 4));

	*width = *spritePtr;
	*height = *(spritePtr + 1);
}

int32 Grid::loadGridBricks(int32 gridSize) {
	uint32 firstBrick = 60000;
	uint32 lastBrick = 0;
	uint32 currentBllEntryIdx = 0;

	memset(brickTable, 0, sizeof(brickTable));
	memset(brickSizeTable, 0, sizeof(brickSizeTable));
	memset(brickUsageTable, 0, sizeof(brickUsageTable));

	// get block librarie usage bits
	uint8 *ptrToBllBits = currentGrid + (gridSize - 32);

	// for all bits under the 32bytes (256bits)
	for (uint32 i = 1; i < 256; i++) {
		uint8 currentBitByte = *(ptrToBllBits + (i / 8));
		uint8 currentBitMask = 1 << (7 - (i & 7));

		if (currentBitByte & currentBitMask) {
			uint32 currentBllOffset = *((uint32 *)(currentBll + currentBllEntryIdx));
			uint8 *currentBllPtr = currentBll + currentBllOffset;

			uint32 bllSizeX = currentBllPtr[0];
			uint32 bllSizeY = currentBllPtr[1];
			uint32 bllSizeZ = currentBllPtr[2];

			uint32 bllSize = bllSizeX * bllSizeY * bllSizeZ;

			uint8 *bllDataPtr = currentBllPtr + 5;

			for (uint32 j = 0; j < bllSize; j++) {
				uint32 brickIdx = *((int16 *)(bllDataPtr));

				if (brickIdx) {
					brickIdx--;

					if (brickIdx <= firstBrick)
						firstBrick = brickIdx;

					if (brickIdx > lastBrick)
						lastBrick = brickIdx;

					brickUsageTable[brickIdx] = 1;
				}
				bllDataPtr += 4;
			}
		}
		currentBllEntryIdx += 4;
	}

	for (uint32 i = firstBrick; i <= lastBrick; i++) {
		if (brickUsageTable[i]) {
			brickSizeTable[i] = _engine->_hqrdepack->hqrGetallocEntry(&brickTable[i], Resources::HQR_LBA_BRK_FILE, i);
		}
	}

	return 1;
}

void Grid::createGridColumn(uint8 *gridEntry, uint8 *dest) {
	int32 brickCount = *(gridEntry++);

	do {
		int32 flag = *(gridEntry++);
		int32 blockCount = (flag & 0x3F) + 1;

		uint16 *gridBuffer = (uint16 *)gridEntry;
		uint16 *blockByffer = (uint16 *)dest;

		if (!(flag & 0xC0)) {
			for (int32 i = 0; i < blockCount; i++)
				*(blockByffer++) = 0;
		} else if (flag & 0x40) {
			for (int32 i = 0; i < blockCount; i++)
				*(blockByffer++) = *(gridBuffer++);
		} else {
			int32 gridIdx = *(gridBuffer++);
			for (int32 i = 0; i < blockCount; i++)
				*(blockByffer++) = gridIdx;
		}

		gridEntry = (uint8 *)gridBuffer;
		dest = (uint8 *)blockByffer;

	} while (--brickCount);
}

void Grid::createCellingGridColumn(uint8 *gridEntry, uint8 *dest) {
	int32 brickCount = *(gridEntry++);

	do {
		int32 flag = *(gridEntry++);

		int32 blockCount = (flag & 0x3F) + 1;

		uint16 *gridBuffer = (uint16 *)gridEntry;
		uint16 *blockByffer = (uint16 *)dest;

		if (!(flag & 0xC0)) {
			for (int32 i = 0; i < blockCount; i++)
				blockByffer++;
		} else if (flag & 0x40) {
			for (int32 i = 0; i < blockCount; i++)
				*(blockByffer++) = *(gridBuffer++);
		} else {
			int32 gridIdx = *(gridBuffer++);
			for (int32 i = 0; i < blockCount; i++)
				*(blockByffer++) = gridIdx;
		}

		gridEntry = (uint8 *)gridBuffer;
		dest = (uint8 *)blockByffer;

	} while (--brickCount);
}

void Grid::createGridMap() {
	int32 currOffset = 0;

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		int32 blockOffset = currOffset;
		int32 gridIdx = z << 6;

		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			int32 gridOffset = *((uint16 *)(currentGrid + 2 * (x + gridIdx)));
			createGridColumn(currentGrid + gridOffset, blockBuffer + blockOffset);
			blockOffset += 50;
		}
		currOffset += 3200;
	}
}

void Grid::createCellingGridMap(uint8 *gridPtr) {
	int32 currGridOffset = 0;
	int32 currOffset = 0;
	int32 blockOffset;
	int32 z, x;
	uint8 *tempGridPtr;

	for (z = 0; z < GRID_SIZE_Z; z++) {
		blockOffset = currOffset;
		tempGridPtr = gridPtr + currGridOffset;

		for (x = 0; x < GRID_SIZE_X; x++) {
			int gridOffset = *((uint16 *)tempGridPtr);
			tempGridPtr += 2;
			createCellingGridColumn(gridPtr + gridOffset, blockBuffer + blockOffset);
			blockOffset += 50;
		}
		currGridOffset += 128;
		currOffset += 3200;
	}
}

int32 Grid::initGrid(int32 index) {
	// load grids from file
	int32 gridSize = _engine->_hqrdepack->hqrGetallocEntry(&currentGrid, Resources::HQR_LBA_GRI_FILE, index);

	// load layouts from file
	_engine->_hqrdepack->hqrGetallocEntry(&currentBll, Resources::HQR_LBA_BLL_FILE, index);

	loadGridBricks(gridSize);

	createGridMask();

	numberOfBll = (*((uint32 *)currentBll) >> 2);

	createGridMap();

	return 1;
}

int32 Grid::initCellingGrid(int32 index) {
	uint8 *gridPtr;

	// load grids from file
	_engine->_hqrdepack->hqrGetallocEntry(&gridPtr, Resources::HQR_LBA_GRI_FILE, index + CELLING_GRIDS_START_INDEX);

	createCellingGridMap(gridPtr);

	if (gridPtr)
		free(gridPtr);

	_engine->_redraw->reqBgRedraw = 1;

	return 0;
}

void Grid::drawBrick(int32 index, int32 posX, int32 posY) {
	drawBrickSprite(index, posX, posY, brickTable[index], false);
}

void Grid::drawSprite(int32 index, int32 posX, int32 posY, uint8 *ptr) {
	drawBrickSprite(index, posX, posY, ptr, true);
}

// WARNING: Rewrite this function to have better performance
void Grid::drawBrickSprite(int32 index, int32 posX, int32 posY, uint8 *ptr, bool isSprite) {
	//unsigned char *ptr;
	uint8 *outPtr;

	if (isSprite)
		ptr = ptr + *((uint32 *)(ptr + index * 4));

	int32 left = posX + *(ptr + 2);
	int32 top = posY + *(ptr + 3);
	int32 right = *ptr + left - 1;
	int32 bottom = *(ptr + 1) + top - 1;

	ptr += 4;

	int32 x = left;
	int32 y = top;

	//if (left >= textWindowLeft-2 && top >= textWindowTop-2 && right <= textWindowRight-2 && bottom <= textWindowBottom-2) // crop
	{
		right++;
		bottom++;

		outPtr = _engine->frontVideoBuffer + _engine->screenLookupTable[top] + left;

		int32 offset = -((right - left) - SCREEN_WIDTH);

		for (int32 c1 = 0; c1 < bottom - top; c1++) {
			int32 vc3 = *(ptr++);
			for (int32 c2 = 0; c2 < vc3; c2++) {
				int32 temp = *(ptr++);
				int32 iteration = temp & 0x3F;
				if (temp & 0xC0) {
					iteration++;
					if (!(temp & 0x40)) {
						temp = *(ptr++);
						for (int32 i = 0; i < iteration; i++) {
							if (x >= _engine->_interface->textWindowLeft && x < _engine->_interface->textWindowRight && y >= _engine->_interface->textWindowTop && y < _engine->_interface->textWindowBottom)
								_engine->frontVideoBuffer[y * SCREEN_WIDTH + x] = temp;

							x++;
							outPtr++;
						}
					} else {
						for (int32 i = 0; i < iteration; i++) {
							if (x >= _engine->_interface->textWindowLeft && x < _engine->_interface->textWindowRight && y >= _engine->_interface->textWindowTop && y < _engine->_interface->textWindowBottom)
								_engine->frontVideoBuffer[y * SCREEN_WIDTH + x] = *ptr;

							x++;
							ptr++;
							outPtr++;
						}
					}
				} else {
					outPtr += iteration + 1;
					x += iteration + 1;
				}
			}
			outPtr += offset;
			x = left;
			y++;
		}
	}
}

uint8 *Grid::getBlockLibrary(int32 index) {
	int32 offset = *((uint32 *)(currentBll + 4 * index));
	return (uint8 *)(currentBll + offset);
}

void Grid::getBrickPos(int32 x, int32 y, int32 z) {
	brickPixelPosX = (x - z) * 24 + 288;              // x pos
	brickPixelPosY = ((x + z) * 12) - (y * 15) + 215; // y pos
}

void Grid::drawColumnGrid(int32 blockIdx, int32 brickBlockIdx, int32 x, int32 y, int32 z) {
	uint8 *blockPtr;
	uint16 brickIdx;
	uint8 brickShape;
	uint8 brickSound;
	int32 brickBuffIdx;
	BrickEntry *currBrickEntry;

	blockPtr = getBlockLibrary(blockIdx) + 3 + brickBlockIdx * 4;

	brickShape = *((uint8 *)(blockPtr));
	brickSound = *((uint8 *)(blockPtr + 1));
	brickIdx = *((uint16 *)(blockPtr + 2));

	if (!brickIdx)
		return;

	getBrickPos(x - newCameraX, y - newCameraY, z - newCameraZ);

	if (brickPixelPosX < -24)
		return;
	if (brickPixelPosX >= SCREEN_WIDTH)
		return;
	if (brickPixelPosY < -38)
		return;
	if (brickPixelPosY >= SCREEN_HEIGHT)
		return;

	// draw the background brick
	drawBrick(brickIdx - 1, brickPixelPosX, brickPixelPosY);

	brickBuffIdx = (brickPixelPosX + 24) / 24;

	if (brickInfoBuffer[brickBuffIdx] >= 150) {
		warning("\nGRID WARNING: brick buffer exceeded! \n");
		return;
	}

	currBrickEntry = &bricksDataBuffer[brickBuffIdx][brickInfoBuffer[brickBuffIdx]];

	currBrickEntry->x = x;
	currBrickEntry->y = y;
	currBrickEntry->z = z;
	currBrickEntry->posX = brickPixelPosX;
	currBrickEntry->posY = brickPixelPosY;
	currBrickEntry->index = brickIdx - 1;
	currBrickEntry->shape = brickShape;
	currBrickEntry->sound = brickSound;

	brickInfoBuffer[brickBuffIdx]++;
}

void Grid::redrawGrid() {
	blockMap *map = (blockMap *)blockBuffer;

	cameraX = newCameraX << 9;
	cameraY = newCameraY << 8;
	cameraZ = newCameraZ << 9;

	_engine->_renderer->projectPositionOnScreen(-cameraX, -cameraY, -cameraZ);

	_engine->_renderer->projPosXScreen = _engine->_renderer->projPosX;
	_engine->_renderer->projPosYScreen = _engine->_renderer->projPosY;

	for (int32 i = 0; i < 28; i++) {
		brickInfoBuffer[i] = 0;
	}

	if (_engine->_scene->changeRoomVar10 == 0)
		return;

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			for (int32 y = 0; y < GRID_SIZE_Y; y++) {
				const uint8 blockIdx = (*map)[z][x][y].blockIdx;
				if (blockIdx) {
					drawColumnGrid(blockIdx - 1, (*map)[z][x][y].brickBlockIdx, x, y, z);
				}
			}
		}
	}
}

int32 Grid::getBrickShape(int32 x, int32 y, int32 z) { // WorldColBrick
	uint8 blockIdx;
	uint8 *blockBufferPtr;

	blockBufferPtr = blockBuffer;

	_engine->_collision->collisionX = (x + 0x100) >> 9;
	_engine->_collision->collisionY = y >> 8;
	_engine->_collision->collisionZ = (z + 0x100) >> 9;

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= 64)
		return 0;

	if (_engine->_collision->collisionY <= -1)
		return 1;

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY > 24 || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= 64)
		return 0;

	blockBufferPtr += _engine->_collision->collisionX * 50;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ << 7) * 25;

	blockIdx = *blockBufferPtr;

	if (blockIdx) {
		uint8 *blockPtr;
		uint8 tmpBrickIdx;

		blockPtr = currentBll;

		blockPtr += *(uint32 *)(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;

		return *blockPtr;
	}
	return *(blockBufferPtr + 1);
}

int32 Grid::getBrickShapeFull(int32 x, int32 y, int32 z, int32 y2) {
	int32 newY, currY, i;
	uint8 blockIdx, brickShape;
	uint8 *blockBufferPtr;

	blockBufferPtr = blockBuffer;

	_engine->_collision->collisionX = (x + 0x100) >> 9;
	_engine->_collision->collisionY = y >> 8;
	_engine->_collision->collisionZ = (z + 0x100) >> 9;

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= 64)
		return 0;

	if (_engine->_collision->collisionY <= -1)
		return 1;

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY > 24 || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= 64)
		return 0;

	blockBufferPtr += _engine->_collision->collisionX * 50;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ << 7) * 25;

	blockIdx = *blockBufferPtr;

	if (blockIdx) {
		uint8 *blockPtr = currentBll;

		blockPtr += *(uint32 *)(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		uint8 tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;

		brickShape = *blockPtr;

		newY = (y2 + 255) >> 8;
		currY = _engine->_collision->collisionY;

		for (i = 0; i < newY; i++) {
			if (currY > 24) {
				return brickShape;
			}

			blockBufferPtr += 2;
			currY++;

			if (*(int16 *)(blockBufferPtr) != 0) {
				return 1;
			}
		}

		return brickShape;
	}
	brickShape = *(blockBufferPtr + 1);

	newY = (y2 + 255) >> 8;
	currY = _engine->_collision->collisionY;

	for (i = 0; i < newY; i++) {
		if (currY > 24) {
			return brickShape;
		}

		blockBufferPtr += 2;
		currY++;

		if (*(int16 *)(blockBufferPtr) != 0) {
			return 1;
		}
	}

	return 0;
}

int32 Grid::getBrickSoundType(int32 x, int32 y, int32 z) { // getPos2
	uint8 blockIdx;
	uint8 *blockBufferPtr;

	blockBufferPtr = blockBuffer;

	_engine->_collision->collisionX = (x + 0x100) >> 9;
	_engine->_collision->collisionY = y >> 8;
	_engine->_collision->collisionZ = (z + 0x100) >> 9;

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= 64)
		return 0;

	if (_engine->_collision->collisionY <= -1)
		return 1;

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY > 24 || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= 64)
		return 0;

	blockBufferPtr += _engine->_collision->collisionX * 50;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ << 7) * 25;

	blockIdx = *blockBufferPtr;

	if (blockIdx) {
		uint8 *blockPtr;
		uint8 tmpBrickIdx;

		blockPtr = currentBll;

		blockPtr += *(uint32 *)(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;
		blockPtr++;

		return *((int16 *)blockPtr);
	}

	return 0xF0;
}

} // namespace TwinE
