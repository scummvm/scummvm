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

#include "twine/scene/grid.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "twine/menu/interface.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/collision.h"
#include "twine/scene/scene.h"
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
	free(currentGrid);
	free(currentBll);
}

void Grid::copyGridMask(int32 index, int32 x, int32 y, const Graphics::ManagedSurface &buffer) {
	uint8 *ptr = brickMaskTable[index];

	int32 left = x + *(ptr + 2);
	int32 top = y + *(ptr + 3);
	int32 right = *ptr + left - 1;
	int32 bottom = *(ptr + 1) + top - 1;

	if (left > _engine->_interface->textWindow.right || right < _engine->_interface->textWindow.left || bottom < _engine->_interface->textWindow.top || top > _engine->_interface->textWindow.bottom) {
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
	if (absY < _engine->_interface->textWindow.top) {
		int numOfLineToRemove = _engine->_interface->textWindow.top - absY;

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
	if (absY + vSize - 1 > _engine->_interface->textWindow.bottom) {
		vSize = _engine->_interface->textWindow.bottom - absY + 1;
		if (vSize <= 0) {
			return;
		}
	}

	uint8 *outPtr = (uint8 *)_engine->frontVideoBuffer.getBasePtr(left, absY);
	const uint8 *inPtr = (const uint8 *)buffer.getBasePtr(left, absY);

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
				if (absX >= _engine->_interface->textWindow.left && absX <= _engine->_interface->textWindow.right) {
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
	const int32 copyBlockPhysLeft = ((_engine->_interface->textWindow.left + 24) / 24) - 1;
	const int32 copyBlockPhysRight = ((_engine->_interface->textWindow.right + 24) / 24);

	for (int32 j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (int32 i = 0; i < brickInfoBuffer[j]; i++) {
			const BrickEntry *currBrickEntry = &bricksDataBuffer[j][i];

			if (currBrickEntry->posY + 38 > _engine->_interface->textWindow.top && currBrickEntry->posY <= _engine->_interface->textWindow.bottom && currBrickEntry->y >= y) {
				if (currBrickEntry->x + currBrickEntry->z > z + x) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->workVideoBuffer);
				}
			}
		}
	}
}

void Grid::drawOverSpriteActor(int32 x, int32 y, int32 z) {
	const int32 copyBlockPhysLeft = ((_engine->_interface->textWindow.left + 24) / 24) - 1;
	const int32 copyBlockPhysRight = (_engine->_interface->textWindow.right + 24) / 24;

	for (int32 j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (int32 i = 0; i < brickInfoBuffer[j]; i++) {
			BrickEntry *currBrickEntry = &bricksDataBuffer[j][i];

			if (currBrickEntry->posY + 38 > _engine->_interface->textWindow.top && currBrickEntry->posY <= _engine->_interface->textWindow.bottom && currBrickEntry->y >= y) {
				if (currBrickEntry->x == x && currBrickEntry->z == z) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->workVideoBuffer);
				}

				if (currBrickEntry->x > x || currBrickEntry->z > z) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->workVideoBuffer);
				}
			}
		}
	}
}

void Grid::processGridMask(const uint8 *buffer, uint8 *ptr) {
	const uint8 width = *buffer++;
	uint8 height = *buffer++;
	const uint8 offsetX = *buffer++;
	const uint8 offsetY = *buffer++;
	const int32 maxY = offsetY + height;

	*ptr++ = width;
	*ptr++ = height;
	*ptr++ = offsetX;
	*ptr++ = offsetY;

	uint8 *targetPtrPos = ptr;

	for (int32 y = offsetY; y < maxY; ++y) {
		uint8 numOfBlock = 0;
		uint8 opaquePixels = 0;
		uint8 *numOfBlockTargetPtr = targetPtrPos;

		targetPtrPos++;

		const uint8 numRuns = *buffer++;

		// the first time isn't skip. the skip size is 0 in that case
		if (bits(*buffer, 6, 2) != 0) {
			*targetPtrPos++ = 0;
			numOfBlock++;
		}

		for (uint8 run = 0; run < numRuns; ++run) {
			const uint8 runSpec = *buffer++;
			const uint8 runLength = bits(runSpec, 0, 6) + 1;
			const uint8 type = bits(runSpec, 6, 2);
			if (type == 2) {
				opaquePixels += runLength;
				buffer++;
			} else if (type == 1) {
				opaquePixels += runLength;
				buffer += runLength;
			} else { // skip (type 3)
				if (opaquePixels) {
					*targetPtrPos++ = opaquePixels; // write down the number of pixel passed so far
					numOfBlock++;
					opaquePixels = 0;
				}
				*targetPtrPos++ = runLength; //write skip
				numOfBlock++;
			}
		}

		if (opaquePixels) {
			*targetPtrPos++ = opaquePixels;
			numOfBlock++;

			opaquePixels = 0;
		}

		*numOfBlockTargetPtr = numOfBlock;
	}
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

void Grid::loadGridBricks() {
	uint32 firstBrick = 60000;
	uint32 lastBrick = 0;
	uint32 currentBllEntryIdx = 0;

	memset(brickSizeTable, 0, sizeof(brickSizeTable));
	memset(brickUsageTable, 0, sizeof(brickUsageTable));

	// get block libraries usage bits
	const uint8 *ptrToBllBits = currentGrid + (currentGridSize - 32);

	// for all bits under the 32bytes (256bits)
	for (uint32 i = 1; i < 256; i++) {
		uint8 currentBitByte = *(ptrToBllBits + (i / 8));
		uint8 currentBitMask = 1 << (7 - (i & 7));

		if (currentBitByte & currentBitMask) {
			uint32 currentBllOffset = READ_LE_UINT32(currentBll + currentBllEntryIdx);
			const uint8 *currentBllPtr = currentBll + currentBllOffset;

			uint32 bllSizeX = *currentBllPtr++;
			uint32 bllSizeY = *currentBllPtr++;
			uint32 bllSizeZ = *currentBllPtr++;

			uint32 bllSize = bllSizeX * bllSizeY * bllSizeZ;

			for (uint32 j = 0; j < bllSize; j++) {
				/* const uint8 type = * */currentBllPtr++;
				/* const uint8 shape = * */currentBllPtr++;
				uint32 brickIdx = READ_LE_INT16(currentBllPtr);
				currentBllPtr += 2;

				if (brickIdx) {
					brickIdx--;

					if (brickIdx <= firstBrick) {
						firstBrick = brickIdx;
					}

					if (brickIdx > lastBrick) {
						lastBrick = brickIdx;
					}

					brickUsageTable[brickIdx] = 1;
				}
			}
		}
		currentBllEntryIdx += 4;
	}

	for (uint32 i = firstBrick; i <= lastBrick; i++) {
		if (!brickUsageTable[i]) {
			free(brickTable[i]);
			brickTable[i] = nullptr;
			continue;
		}
		brickSizeTable[i] = HQR::getAllocEntry(&brickTable[i], Resources::HQR_LBA_BRK_FILE, i);
		if (brickSizeTable[i] == 0) {
			warning("Failed to load isometric brick index %i", i);
		}
	}
}

void Grid::createGridColumn(const uint8 *gridEntry, uint32 gridEntrySize, uint8 *dest, uint32 destSize) {
	Common::MemoryReadStream stream(gridEntry, gridEntrySize);
	Common::MemoryWriteStream outstream(dest, destSize);
	int32 brickCount = stream.readByte();

	do {
		const int32 flag = stream.readByte();
		const int32 blockCount = bits(flag, 0, 6) + 1;
		const int32 type = bits(flag, 6, 2);
		if (type == 0) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(0);
			}
		} else if (type == 1) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(stream.readUint16LE());
			}
		} else {
			const int32 gridIdx = stream.readUint16LE();
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
		const int32 blockCount = bits(flag, 0, 6) + 1;
		const int32 type = bits(flag, 6, 2);

		if (type == 0) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.seek(outstream.pos() + 2);
			}
		} else if (type == 1) {
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(stream.readUint16LE());
			}
		} else {
			const int32 gridIdx = stream.readUint16LE();
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
		const int32 gridIdx = z * GRID_SIZE_X;

		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			const int32 gridOffset = READ_LE_UINT16(currentGrid + 2 * (x + gridIdx));
			createGridColumn(currentGrid + gridOffset, currentGridSize - gridOffset, blockBuffer + blockOffset, blockBufferSize - blockOffset);
			blockOffset += 2 * GRID_SIZE_Y;
		}
		currOffset += GRID_SIZE_X * (2 * GRID_SIZE_Y);
	}
}

void Grid::createCellingGridMap(const uint8 *gridPtr, int32 gridPtrSize) {
	int32 currGridOffset = 0;
	int32 currOffset = 0;

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		int32 blockOffset = currOffset;
		const uint8 *tempGridPtr = gridPtr + currGridOffset;

		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			const int gridOffset = READ_LE_UINT16(tempGridPtr);
			tempGridPtr += 2;
			createCellingGridColumn(gridPtr + gridOffset, gridPtrSize - gridOffset, blockBuffer + blockOffset, blockBufferSize - blockOffset);
			blockOffset += 2 * GRID_SIZE_Y;
		}
		currGridOffset += GRID_SIZE_X+ GRID_SIZE_Z;
		currOffset += GRID_SIZE_X * (2 * GRID_SIZE_Y);
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
	if (HQR::getAllocEntry(&currentBll, Resources::HQR_LBA_BLL_FILE, index) == 0) {
		warning("Failed to load block library index: %i", index);
		return false;
	}

	loadGridBricks();

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
	ptr = ptr + READ_LE_INT32(ptr + index * 4);
	drawBrickSprite(index, posX, posY, ptr, true);
}

void Grid::drawSprite(int32 posX, int32 posY, const SpriteData &ptr) {
	const int32 left = posX + ptr.offsetX();
	if (left > _engine->_interface->textWindow.right) {
		return;
	}
	const int32 right = ptr.surface().w + left;
	if (right < _engine->_interface->textWindow.left) {
		return;
	}
	const int32 top = posY + ptr.offsetY();
	if (top > _engine->_interface->textWindow.bottom) {
		return;
	}
	const int32 bottom = ptr.surface().h + top;
	if (bottom < _engine->_interface->textWindow.top) {
		return;
	}

	_engine->frontVideoBuffer.transBlitFrom(ptr.surface(), Common::Point(left, top));
}

// WARNING: Rewrite this function to have better performance
void Grid::drawBrickSprite(int32 index, int32 posX, int32 posY, const uint8 *ptr, bool isSprite) {
	const int32 left = posX + *(ptr + 2);
	if (left > _engine->_interface->textWindow.right) {
		return;
	}
	const int32 right = *ptr + left;
	if (right < _engine->_interface->textWindow.left) {
		return;
	}
	const int32 top = posY + *(ptr + 3);
	if (top > _engine->_interface->textWindow.bottom) {
		return;
	}
	const int32 bottom = (int32)*(ptr + 1) + top;
	if (bottom < _engine->_interface->textWindow.top) {
		return;
	}
	const int32 maxY = MIN(bottom, (int32)_engine->_interface->textWindow.bottom);

	ptr += 4;

	int32 x = left;

	//if (left >= textWindowLeft-2 && top >= textWindowTop-2 && right <= textWindowRight-2 && bottom <= textWindowBottom-2) // crop
	{
		for (int32 y = top; y < maxY; ++y) {
			const uint8 rleAmount = *ptr++;
			for (int32 run = 0; run < rleAmount; ++run) {
				const uint8 rleMask = *ptr++;
				const uint8 iterations = bits(rleMask, 0, 6) + 1;
				const uint8 type = bits(rleMask, 6, 2);
				if (type == 0) {
					x += iterations;
					continue;
				}
				if (y < _engine->_interface->textWindow.top || x >= _engine->_interface->textWindow.right || x + iterations < _engine->_interface->textWindow.left) {
					if (type == 1) {
						ptr += iterations;
					} else {
						++ptr;
					}
					x += iterations;
					continue;
				}
				if (type == 1) {
					uint8 *out = (uint8 *)_engine->frontVideoBuffer.getBasePtr(x, y);
					for (uint8 i = 0; i < iterations; i++) {
						if (x >= _engine->_interface->textWindow.left && x < _engine->_interface->textWindow.right) {
							*out = *ptr;
						}

						++out;
						++x;
						++ptr;
					}
				} else {
					const uint8 pixel = *ptr++;
					uint8 *out = (uint8 *)_engine->frontVideoBuffer.getBasePtr(x, y);
					for (uint8 i = 0; i < iterations; i++) {
						if (x >= _engine->_interface->textWindow.left && x < _engine->_interface->textWindow.right) {
							*out = pixel;
						}

						++out;
						++x;
					}
				}
			}
			x = left;
		}
	}
}

uint8 *Grid::getBlockBuffer(int32 x, int32 y, int32 z) {
	const int32 tempX = (x + BRICK_HEIGHT) / BRICK_SIZE;
	const int32 tempY = y / BRICK_HEIGHT;
	const int32 tempZ = (z + BRICK_HEIGHT) / BRICK_SIZE;
	return blockBuffer + tempY * 2 + tempX * GRID_SIZE_Y * 2 + (tempZ * GRID_SIZE_X) * GRID_SIZE_Y * 2;
}

const uint8 *Grid::getBlockBufferGround(int32 x, int32 y, int32 z, int16 &ground) const {
	_engine->_grid->updateCollisionCoordinates(x, y, z);
	const int32 tempX = _engine->_collision->collisionX;
	int32 tempY = _engine->_collision->collisionY;
	const int32 tempZ = _engine->_collision->collisionZ;
	const uint8 *ptr = blockBuffer + tempY * 2 + tempX * GRID_SIZE_Y * 2 + (tempZ * GRID_SIZE_X) * GRID_SIZE_Y * 2;

	while (tempY) {
		if (READ_LE_INT16(ptr)) { // found the ground
			break;
		}
		tempY--;
		ptr -= 2;
	}

	_engine->_collision->collisionY = tempY;
	ground = (int16)((tempY + 1) * BRICK_HEIGHT);

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

	if (brickInfoBuffer[brickBuffIdx] >= MAXBRICKS) {
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

	cameraX = newCameraX * BRICK_SIZE;
	cameraY = newCameraY * BRICK_HEIGHT;
	cameraZ = newCameraZ * BRICK_SIZE;

	_engine->_renderer->projectPositionOnScreen(-cameraX, -cameraY, -cameraZ);

	_engine->_renderer->projPosXScreen = _engine->_renderer->projPosX;
	_engine->_renderer->projPosYScreen = _engine->_renderer->projPosY;

	memset(brickInfoBuffer, 0, sizeof(brickInfoBuffer));

	if (!_engine->_scene->changeRoomVar10) {
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

ShapeType Grid::getBrickShape(int32 x, int32 y, int32 z) {
	updateCollisionCoordinates(x, y, z);

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= GRID_SIZE_X) {
		return ShapeType::kNone;
	}

	if (_engine->_collision->collisionY <= -1) {
		return ShapeType::kSolid;
	}

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY >= GRID_SIZE_Y || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= GRID_SIZE_Z) {
		return ShapeType::kNone;
	}

	uint8 *blockBufferPtr = blockBuffer;
	blockBufferPtr += _engine->_collision->collisionX * GRID_SIZE_Y * 2;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ * GRID_SIZE_X * 2) * GRID_SIZE_Y;

	uint8 blockIdx = *blockBufferPtr;

	if (blockIdx) {
		const uint8 *blockPtr = currentBll;

		blockPtr += READ_LE_UINT32(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		const uint8 tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;

		return (ShapeType)*blockPtr;
	}
	return (ShapeType) * (blockBufferPtr + 1);
}

void Grid::updateCollisionCoordinates(int32 x, int32 y, int32 z) {
	_engine->_collision->collisionX = (x + BRICK_HEIGHT) / BRICK_SIZE;
	_engine->_collision->collisionY = y / BRICK_HEIGHT;
	_engine->_collision->collisionZ = (z + BRICK_HEIGHT) / BRICK_SIZE;
}

ShapeType Grid::getBrickShapeFull(int32 x, int32 y, int32 z, int32 y2) {
	updateCollisionCoordinates(x, y, z);

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= GRID_SIZE_X) {
		return ShapeType::kNone;
	}

	if (_engine->_collision->collisionY <= -1) {
		return ShapeType::kSolid;
	}

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY >= GRID_SIZE_Y || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= GRID_SIZE_Z) {
		return ShapeType::kNone;
	}

	uint8 *blockBufferPtr = blockBuffer;
	blockBufferPtr += _engine->_collision->collisionX * GRID_SIZE_Y * 2;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ * GRID_SIZE_X * 2) * GRID_SIZE_Y;

	uint8 blockIdx = *blockBufferPtr;

	if (blockIdx) {
		const uint8 *blockPtr = currentBll;

		blockPtr += READ_LE_UINT32(blockPtr + blockIdx * 4 - 4);
		blockPtr += 3;

		const uint8 tmpBrickIdx = *(blockBufferPtr + 1);
		blockPtr = blockPtr + tmpBrickIdx * 4;

		const ShapeType brickShape = (ShapeType)*blockPtr;

		const int32 newY = (y2 + (BRICK_HEIGHT - 1)) / BRICK_HEIGHT;
		int32 currY = _engine->_collision->collisionY;

		for (int32 i = 0; i < newY; i++) {
			if (currY >= GRID_SIZE_Y) {
				return brickShape;
			}

			blockBufferPtr += 2;
			currY++;

			if (READ_LE_INT16(blockBufferPtr) != 0) {
				return ShapeType::kSolid;
			}
		}

		return brickShape;
	}
	const ShapeType brickShape = (ShapeType) * (blockBufferPtr + 1);

	const int32 newY = (y2 + (BRICK_HEIGHT - 1)) / BRICK_HEIGHT;
	int32 currY = _engine->_collision->collisionY;

	for (int32 i = 0; i < newY; i++) {
		if (currY >= GRID_SIZE_Y) {
			return brickShape;
		}

		blockBufferPtr += 2;
		currY++;

		if (READ_LE_INT16(blockBufferPtr) != 0) {
			return ShapeType::kSolid;
		}
	}

	return ShapeType::kNone;
}

int32 Grid::getBrickSoundType(int32 x, int32 y, int32 z) { // getPos2
	updateCollisionCoordinates(x, y, z);

	if (_engine->_collision->collisionX < 0 || _engine->_collision->collisionX >= GRID_SIZE_X) {
		return 0; // none
	}

	if (_engine->_collision->collisionY <= -1) {
		return 1; // solid
	}

	if (_engine->_collision->collisionY < 0 || _engine->_collision->collisionY >= GRID_SIZE_Y || _engine->_collision->collisionZ < 0 || _engine->_collision->collisionZ >= GRID_SIZE_Z) {
		return 0; // none
	}

	const uint8 *blockBufferPtr = blockBuffer;
	blockBufferPtr += _engine->_collision->collisionX * GRID_SIZE_Y * 2;
	blockBufferPtr += _engine->_collision->collisionY * 2;
	blockBufferPtr += (_engine->_collision->collisionZ * GRID_SIZE_X * 2) * GRID_SIZE_Y;

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

	return 240;
}

} // namespace TwinE
