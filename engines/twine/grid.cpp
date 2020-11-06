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
#include "common/endian.h"
#include "common/memstream.h"
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

Grid::Grid(TwinEEngine *engine) : _engine(engine) {
	blockBufferSize = GRID_SIZE_X * GRID_SIZE_Z * GRID_SIZE_Y * 2 * sizeof(uint8);
	blockBuffer = (uint8 *)malloc(blockBufferSize);
}

Grid::~Grid() {
	free(blockBuffer);
	for (int32 i = 0; i < ARRAYSIZE(brickMaskTable); i++) {
		free(brickMaskTable[i]);
	}
	for (int32 i = 0; i < ARRAYSIZE(brickTable); i++) {
		free(brickTable[i]);
	}
}

void Grid::copyGridMask(int32 index, int32 x, int32 y, const uint8 *buffer) {
	uint8 *ptr = brickMaskTable[index];

	int32 left = x + *(ptr + 2);
	int32 top = y + *(ptr + 3);
	int32 right = *ptr + left - 1;
	int32 bottom = *(ptr + 1) + top - 1;

	if (left > _engine->_interface->textWindowRight || right < _engine->_interface->textWindowLeft || bottom < _engine->_interface->textWindowTop || top > _engine->_interface->textWindowBottom) {
		return;
	}

	ptr += 4;

	int32 absX = left;
	int32 absY = top;

	int32 vSize = (bottom - top) + 1;

	if (vSize <= 0) {
		return;
	}

	int32 offset = -((right - left) - SCREEN_WIDTH) - 1;

	right++;
	bottom++;

	// if line on top aren't in the blitting area...
	if (absY < _engine->_interface->textWindowTop) {
		int numOfLineToRemove = _engine->_interface->textWindowTop - absY;

		vSize -= numOfLineToRemove;
		if (vSize <= 0) {
			return;
		}

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
		if (vSize <= 0) {
			return;
		}
	}

	uint8 *outPtr = (uint8 *)_engine->frontVideoBuffer.getPixels() + _engine->screenLookupTable[absY] + left;
	const uint8 *inPtr = buffer + _engine->screenLookupTable[absY] + left;

	do {
		int32 vc3 = *(ptr++);

		do {
			int32 temp = *(ptr++); // skip size
			outPtr += temp;
			inPtr += temp;

			absX += temp;

			vc3--;
			if (!vc3) {
				break;
			}

			temp = *(ptr++); // copy size

			for (int32 j = 0; j < temp; j++) {
				if (absX >= _engine->_interface->textWindowLeft && absX <= _engine->_interface->textWindowRight) {
					*outPtr = *inPtr;
				}

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

void Grid::drawOverModelActor(int32 x, int32 y, int32 z) {
	const int32 copyBlockPhysLeft = ((_engine->_interface->textWindowLeft + 24) / 24) - 1;
	const int32 copyBlockPhysRight = ((_engine->_interface->textWindowRight + 24) / 24);

	for (int32 j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (int32 i = 0; i < brickInfoBuffer[j]; i++) {
			BrickEntry *currBrickEntry = &bricksDataBuffer[j][i];

			if (currBrickEntry->posY + 38 > _engine->_interface->textWindowTop && currBrickEntry->posY <= _engine->_interface->textWindowBottom && currBrickEntry->y >= y) {
				if (currBrickEntry->x + currBrickEntry->z > z + x) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, (uint8 *)_engine->workVideoBuffer.getPixels());
				}
			}
		}
	}
}

void Grid::drawOverSpriteActor(int32 x, int32 y, int32 z) {
	const int32 copyBlockPhysLeft = ((_engine->_interface->textWindowLeft + 24) / 24) - 1;
	const int32 copyBlockPhysRight = (_engine->_interface->textWindowRight + 24) / 24;

	for (int32 j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (int32 i = 0; i < brickInfoBuffer[j]; i++) {
			BrickEntry *currBrickEntry = &bricksDataBuffer[j][i];

			if (currBrickEntry->posY + 38 > _engine->_interface->textWindowTop && currBrickEntry->posY <= _engine->_interface->textWindowBottom && currBrickEntry->y >= y) {
				if ((currBrickEntry->x == x) && (currBrickEntry->z == z)) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, (uint8 *)_engine->workVideoBuffer.getPixels());
				}

				if ((currBrickEntry->x > x) || (currBrickEntry->z > z)) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, (uint8 *)_engine->workVideoBuffer.getPixels());
				}
			}
		}
	}
}

int Grid::processGridMask(const uint8 *buffer, uint8 *ptr) {
	const uint8 *ptrSave = ptr;
	int32 ebx = READ_UINT32(buffer); // brick flag
	buffer += 4;
	WRITE_LE_UINT32(ptr, (uint32)ebx);
	ptr += 4;

	uint8 bh = (ebx & 0x0000FF00) >> 8;

	const uint8 *esi = (const uint8 *)buffer;
	uint8 *edi = (uint8 *)ptr;

	uint8 iteration = 0;

	do {
		uint8 numOfBlock = 0;
		uint8 ah = 0;
		uint8 *ptr2 = edi;

		edi++;

		uint8 bl = *(esi++);

		if (*esi & 0xC0) { // the first time isn't skip. the skip size is 0 in that case
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

	return (int)(edi - ptrSave);
}

void Grid::createGridMask() {
	for (int32 b = 0; b < NUM_BRICKS; b++) {
		if (!brickUsageTable[b]) {
			continue;
		}
		if (brickMaskTable[b]) {
			free(brickMaskTable[b]);
		}
		brickMaskTable[b] = (uint8 *)malloc(brickSizeTable[b]);
		processGridMask(brickTable[b], brickMaskTable[b]);
	}
}

void Grid::getSpriteSize(int32 offset, int32 *width, int32 *height, const uint8 *spritePtr) {
	spritePtr += READ_LE_INT32(spritePtr + offset * 4);

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
	const uint8 *ptrToBllBits = currentGrid + (gridSize - 32);

	// for all bits under the 32bytes (256bits)
	for (uint32 i = 1; i < 256; i++) {
		uint8 currentBitByte = *(ptrToBllBits + (i / 8));
		uint8 currentBitMask = 1 << (7 - (i & 7));

		if (currentBitByte & currentBitMask) {
			uint32 currentBllOffset = READ_LE_UINT32(currentBll + currentBllEntryIdx);
			const uint8 *currentBllPtr = currentBll + currentBllOffset;

			uint32 bllSizeX = currentBllPtr[0];
			uint32 bllSizeY = currentBllPtr[1];
			uint32 bllSizeZ = currentBllPtr[2];

			uint32 bllSize = bllSizeX * bllSizeY * bllSizeZ;

			const uint8 *bllDataPtr = currentBllPtr + 5;

			for (uint32 j = 0; j < bllSize; j++) {
				uint32 brickIdx = READ_LE_INT16(bllDataPtr);

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
		if (!brickUsageTable[i]) {
			continue;
		}
		if (brickTable[i]) {
			free(brickTable[i]);
		}
		brickSizeTable[i] = HQR::getAllocEntry(&brickTable[i], Resources::HQR_LBA_BRK_FILE, i);
		if (brickSizeTable[i] == 0) {
			warning("Failed to load isometric brick index %i", i);
		}
	}

	return 1;
}

void Grid::createGridColumn(const uint8 *gridEntry, uint32 gridEntrySize, uint8 *dest, uint32 destSize) {
	Common::MemoryReadStream stream(gridEntry, gridEntrySize);
	Common::MemoryWriteStream outstream(dest, destSize);
	int32 brickCount = stream.readByte();

	do {
		const int32 flag = stream.readByte();
		const int32 blockCount = (flag & 0x3F) + 1;

		if (!(flag & 0xC0)) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(0);
			}
		} else if (flag & 0x40) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(stream.readUint16LE());
			}
		} else {
			int32 gridIdx = stream.readUint16LE();
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(gridIdx);
			}
		}
		assert(!outstream.err());
		assert(!stream.err());
	} while (--brickCount);
}

void Grid::createCellingGridColumn(const uint8 *gridEntry, uint32 gridEntrySize, uint8 *dest, uint32 destSize) {
	Common::MemoryReadStream stream(gridEntry, gridEntrySize);
	Common::SeekableMemoryWriteStream outstream(dest, destSize);
	int32 brickCount = stream.readByte();

	do {
		const int32 flag = stream.readByte();
		const int32 blockCount = (flag & 0x3F) + 1;

		if (!(flag & 0xC0)) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.seek(outstream.pos() + 2);
			}
		} else if (flag & 0x40) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(stream.readUint16LE());
			}
		} else {
			int32 gridIdx = stream.readUint16LE();
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(gridIdx);
			}
		}
		assert(!outstream.err());
		assert(!stream.err());
	} while (--brickCount);
}

void Grid::createGridMap() {
	int32 currOffset = 0;

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		int32 blockOffset = currOffset;
		int32 gridIdx = z << 6;

		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			int32 gridOffset = READ_LE_UINT16(currentGrid + 2 * (x + gridIdx));
			createGridColumn(currentGrid + gridOffset, currentGridSize - gridOffset, blockBuffer + blockOffset, blockBufferSize - blockOffset);
			blockOffset += 50;
		}
		currOffset += 3200;
	}
}

void Grid::createCellingGridMap(const uint8 *gridPtr, int32 gridPtrSize) {
	int32 currGridOffset = 0;
	int32 currOffset = 0;

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		int32 blockOffset = currOffset;
		const uint8 *tempGridPtr = gridPtr + currGridOffset;

		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			int gridOffset = READ_LE_UINT16(tempGridPtr);
			tempGridPtr += 2;
			createCellingGridColumn(gridPtr + gridOffset, gridPtrSize - gridOffset, blockBuffer + blockOffset, blockBufferSize - blockOffset);
			blockOffset += 50;
		}
		currGridOffset += 128;
		currOffset += 3200;
	}
}

bool Grid::initGrid(int32 index) {
	// load grids from file
	currentGridSize = HQR::getAllocEntry(&currentGrid, Resources::HQR_LBA_GRI_FILE, index);
	if (currentGridSize == 0) {
		warning("Failed to load grid index: %i", index);
		return false;
	}

	// load layouts from file
	HQR::getAllocEntry(&currentBll, Resources::HQR_LBA_BLL_FILE, index);

	loadGridBricks(currentGridSize);

	createGridMask();

	numberOfBll = READ_LE_INT32(currentBll) >> 2;

	createGridMap();

	return true;
}

bool Grid::initCellingGrid(int32 index) {
	uint8 *gridPtr = nullptr;

	// load grids from file
	const int realIndex = index + CELLING_GRIDS_START_INDEX;
	const int32 gridSize = HQR::getAllocEntry(&gridPtr, Resources::HQR_LBA_GRI_FILE, realIndex);
	if (gridSize == 0) {
		warning("Failed to load grid index %i", realIndex);
		return false;
	}

	createCellingGridMap(gridPtr, gridSize);
	free(gridPtr);
	_engine->_redraw->reqBgRedraw = true;
	return true;
}

void Grid::drawBrick(int32 index, int32 posX, int32 posY) {
	drawBrickSprite(index, posX, posY, brickTable[index], false);
}

void Grid::drawSprite(int32 index, int32 posX, int32 posY, const uint8 *ptr) {
	drawBrickSprite(index, posX, posY, ptr, true);
}

// WARNING: Rewrite this function to have better performance
void Grid::drawBrickSprite(int32 index, int32 posX, int32 posY, const uint8 *ptr, bool isSprite) {
	if (isSprite) {
		ptr = ptr + READ_LE_INT32(ptr + index * 4);
	}

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

		uint8 *outPtr = (uint8 *)_engine->frontVideoBuffer.getPixels() + _engine->screenLookupTable[top] + left;

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
							if (x >= _engine->_interface->textWindowLeft && x < _engine->_interface->textWindowRight && y >= _engine->_interface->textWindowTop && y < _engine->_interface->textWindowBottom) {
								*(uint8 *)_engine->frontVideoBuffer.getBasePtr(x, y) = temp;
							}

							x++;
							outPtr++;
						}
					} else {
						for (int32 i = 0; i < iteration; i++) {
							if (x >= _engine->_interface->textWindowLeft && x < _engine->_interface->textWindowRight && y >= _engine->_interface->textWindowTop && y < _engine->_interface->textWindowBottom) {
								*(uint8 *)_engine->frontVideoBuffer.getBasePtr(x, y) = *ptr;
							}

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

uint8* Grid::getBlockBuffer(int32 x, int32 y, int32 z) {
	const int32 tempX = (x + 0x100) >> 9;
	const int32 tempY = y >> 8;
	const int32 tempZ = (z + 0x100) >> 9;
	return blockBuffer + tempY * 2 + tempX * GRID_SIZE_Y * 2 + (tempZ << 6) * GRID_SIZE_Y * 2;
}

const uint8* Grid::getBlockBufferGround(int32 x, int32 y, int32 z, int16 &ground) const {
	_engine->_grid->updateCollisionCoordinates(x, y, z);
	const int32 tempX = _engine->_collision->collisionX;
	int32 tempY = _engine->_collision->collisionY;
	const int32 tempZ = _engine->_collision->collisionZ;
	const uint8 *ptr = blockBuffer + tempY * 2 + tempX * GRID_SIZE_Y * 2 + (tempZ << 6) * GRID_SIZE_Y * 2;

	while (tempY) {
		if (READ_LE_INT16(ptr)) { // found the ground
			break;
		}
		tempY--;
		ptr -= 2;
	}

	_engine->_collision->collisionY = tempY;
	ground = (int16)((tempY + 1) << 8);

	return ptr;
}

const uint8 *Grid::getBlockLibrary(int32 index) {
	const int32 offset = READ_LE_UINT32(currentBll + 4 * index);
	return (const uint8 *)(currentBll + offset);
}

void Grid::getBrickPos(int32 x, int32 y, int32 z) {
	brickPixelPosX = (x - z) * 24 + 288;              // x pos
	brickPixelPosY = ((x + z) * 12) - (y * 15) + 215; // y pos
}

void Grid::drawColumnGrid(int32 blockIdx, int32 brickBlockIdx, int32 x, int32 y, int32 z) {
	const uint8 *blockPtr = getBlockLibrary(blockIdx) + 3 + brickBlockIdx * 4;

	const uint8 brickShape = *((const uint8 *)(blockPtr + 0));
	const uint8 brickSound = *((const uint8 *)(blockPtr + 1));
	const uint16 brickIdx = READ_LE_UINT16(blockPtr + 2);
	if (!brickIdx) {
		return;
	}

	getBrickPos(x - newCameraX, y - newCameraY, z - newCameraZ);

	if (brickPixelPosX < -24) {
		return;
	}
	if (brickPixelPosX >= SCREEN_WIDTH) {
		return;
	}
	if (brickPixelPosY < -38) {
		return;
	}
	if (brickPixelPosY >= SCREEN_HEIGHT) {
		return;
	}

	// draw the background brick
	drawBrick(brickIdx - 1, brickPixelPosX, brickPixelPosY);

	int32 brickBuffIdx = (brickPixelPosX + 24) / 24;

	if (brickInfoBuffer[brickBuffIdx] >= 150) {
		warning("GRID: brick buffer exceeded");
		return;
	}

	BrickEntry *currBrickEntry = &bricksDataBuffer[brickBuffIdx][brickInfoBuffer[brickBuffIdx]];

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

	memset(brickInfoBuffer, 0, sizeof(brickInfoBuffer));

	if (_engine->_scene->changeRoomVar10 == 0) {
		return;
	}

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

int32 Grid::getBrickShape(int32 x, int32 y, int32 z) {
	updateCollisionCoordinates(x, y, z);

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= GRID_SIZE_X) {
		return 0;
	}

	if (_engine->_collision->collisionY <= -1) {
		return 1;
	}

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY >= GRID_SIZE_Y || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= GRID_SIZE_Z) {
		return 0;
	}

	uint8 *blockBufferPtr = blockBuffer;
	blockBufferPtr += _engine->_collision->collisionX * GRID_SIZE_Y * 2;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ << 7) * GRID_SIZE_Y;

	uint8 blockIdx = *blockBufferPtr;

	if (blockIdx) {
		const uint8 *blockPtr = currentBll;

		blockPtr += READ_LE_UINT32(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		const uint8 tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;

		return *blockPtr;
	}
	return *(blockBufferPtr + 1);
}

void Grid::updateCollisionCoordinates(int32 x, int32 y, int32 z) {
	_engine->_collision->collisionX = (x + 0x100) >> 9;
	_engine->_collision->collisionY = y >> 8;
	_engine->_collision->collisionZ = (z + 0x100) >> 9;
}

int32 Grid::getBrickShapeFull(int32 x, int32 y, int32 z, int32 y2) {
	updateCollisionCoordinates(x, y, z);

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= GRID_SIZE_X) {
		return 0;
	}

	if (_engine->_collision->collisionY <= -1) {
		return 1;
	}

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY >= GRID_SIZE_Y || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= GRID_SIZE_Z) {
		return 0;
	}

	uint8 *blockBufferPtr = blockBuffer;
	blockBufferPtr += _engine->_collision->collisionX * GRID_SIZE_Y * 2;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ << 7) * GRID_SIZE_Y;

	uint8 blockIdx = *blockBufferPtr;

	if (blockIdx) {
		const uint8 *blockPtr = currentBll;

		blockPtr += READ_LE_UINT32(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		uint8 tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;

		uint8 brickShape = *blockPtr;

		int32 newY = (y2 + 255) >> 8;
		int32 currY = _engine->_collision->collisionY;

		for (int32 i = 0; i < newY; i++) {
			if (currY >= GRID_SIZE_Y) {
				return brickShape;
			}

			blockBufferPtr += 2;
			currY++;

			if (READ_LE_INT16(blockBufferPtr) != 0) {
				return 1;
			}
		}

		return brickShape;
	}
	uint8 brickShape = *(blockBufferPtr + 1);

	int32 newY = (y2 + 255) >> 8;
	int32 currY = _engine->_collision->collisionY;

	for (int32 i = 0; i < newY; i++) {
		if (currY >= GRID_SIZE_Y) {
			return brickShape;
		}

		blockBufferPtr += 2;
		currY++;

		if (READ_LE_INT16(blockBufferPtr) != 0) {
			return 1;
		}
	}

	return 0;
}

int32 Grid::getBrickSoundType(int32 x, int32 y, int32 z) { // getPos2
	updateCollisionCoordinates(x, y, z);

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= GRID_SIZE_X) {
		return 0;
	}

	if (_engine->_collision->collisionY <= -1) {
		return 1;
	}

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY >= GRID_SIZE_Y || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= GRID_SIZE_Z) {
		return 0;
	}

	const uint8 *blockBufferPtr = blockBuffer;
	blockBufferPtr += _engine->_collision->collisionX * GRID_SIZE_Y * 2;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ << 7) * GRID_SIZE_Y;

	uint8 blockIdx = *blockBufferPtr;

	if (blockIdx) {
		const uint8 *blockPtr = currentBll;

		blockPtr += READ_LE_UINT32(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		uint8 tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;
		blockPtr++;

		return READ_LE_INT16(blockPtr);
	}

	return 0xF0;
}

} // namespace TwinE
