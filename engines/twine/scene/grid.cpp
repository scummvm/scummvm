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
#include "twine/debugger/debug_grid.h"
#include "twine/menu/interface.h"
#include "twine/parser/blocklibrary.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/collision.h"
#include "twine/scene/scene.h"
#include "twine/twine.h"

#define CELLING_GRIDS_START_INDEX 120

namespace TwinE {

Grid::Grid(TwinEEngine *engine) : _engine(engine) {
	_blockBufferSize = GRID_SIZE_X * GRID_SIZE_Z * GRID_SIZE_Y * sizeof(BlockEntry);
	_blockBuffer = (uint8 *)malloc(_blockBufferSize);
}

Grid::~Grid() {
	free(_blockBuffer);
	for (int32 i = 0; i < ARRAYSIZE(_brickMaskTable); i++) {
		free(_brickMaskTable[i]);
	}
	for (int32 i = 0; i < ARRAYSIZE(_brickTable); i++) {
		free(_brickTable[i]);
	}
	free(_currentGrid);
	free(_brickInfoBuffer);
	free(_bricksDataBuffer);
}

void Grid::init(int32 w, int32 h) {
	const int32 numbrickentries = (1 + (w + 24) / 24);
	const size_t brickDataBufferSize = numbrickentries * MAXBRICKS * sizeof(BrickEntry);
	_bricksDataBuffer = (BrickEntry *)malloc(brickDataBufferSize);
	_brickInfoBufferSize = numbrickentries * sizeof(int16);
	_brickInfoBuffer = (int16 *)malloc(_brickInfoBufferSize);
}

void Grid::copyGridMask(int32 index, int32 x, int32 y, const Graphics::ManagedSurface &buffer) {
	uint8 *ptr = _brickMaskTable[index];

	int32 left = x + *(ptr + 2);
	int32 top = y + *(ptr + 3);
	int32 right = *ptr + left - 1;
	int32 bottom = *(ptr + 1) + top - 1;

	if (left > _engine->_interface->_clip.right || right < _engine->_interface->_clip.left || bottom < _engine->_interface->_clip.top || top > _engine->_interface->_clip.bottom) {
		return;
	}

	ptr += 4;

	int32 absX = left;
	int32 absY = top;

	int32 vSize = (bottom - top) + 1;

	if (vSize <= 0) {
		return;
	}

	int32 offset = -((right - left) - _engine->width()) - 1;

	right++;
	bottom++;

	// if line on top aren't in the blitting area...
	if (absY < _engine->_interface->_clip.top) {
		int numOfLineToRemove = _engine->_interface->_clip.top - absY;

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
	if (absY + vSize - 1 > _engine->_interface->_clip.bottom) {
		vSize = _engine->_interface->_clip.bottom - absY + 1;
		if (vSize <= 0) {
			return;
		}
	}

	uint8 *outPtr = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(left, absY);
	const uint8 *inPtr = (const uint8 *)buffer.getBasePtr(left, absY);

	do {
		int32 height = *(ptr++);

		do {
			int32 width = *(ptr++); // skip size
			outPtr += width;
			inPtr += width;

			absX += width;

			height--;
			if (!height) {
				break;
			}

			width = *(ptr++); // copy size

			for (int32 j = 0; j < width; j++) {
				if (absX >= _engine->_interface->_clip.left && absX <= _engine->_interface->_clip.right) {
					*outPtr = *inPtr;
				}

				absX++;
				outPtr++;
				inPtr++;
			}
		} while (--height);

		absX = left;

		outPtr += offset;
		inPtr += offset;
	} while (--vSize);
}

const BrickEntry* Grid::getBrickEntry(int32 j, int32 i) const {
	return &_bricksDataBuffer[j * MAXBRICKS + i];
}

void Grid::drawOverModelActor(int32 x, int32 y, int32 z) {
	const int32 copyBlockPhysLeft = ((_engine->_interface->_clip.left + 24) / 24) - 1;
	const int32 copyBlockPhysRight = ((_engine->_interface->_clip.right + 24) / 24);

	for (int32 j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (int32 i = 0; i < _brickInfoBuffer[j]; i++) {
			const BrickEntry *currBrickEntry = getBrickEntry(j, i);

			if (currBrickEntry->posY + 38 > _engine->_interface->_clip.top && currBrickEntry->posY <= _engine->_interface->_clip.bottom && currBrickEntry->y >= y) {
				if (currBrickEntry->x + currBrickEntry->z > z + x) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->_workVideoBuffer);
				}
			}
		}
	}
}

void Grid::drawOverSpriteActor(int32 x, int32 y, int32 z) {
	const int32 copyBlockPhysLeft = ((_engine->_interface->_clip.left + 24) / 24) - 1;
	const int32 copyBlockPhysRight = (_engine->_interface->_clip.right + 24) / 24;

	for (int32 j = copyBlockPhysLeft; j <= copyBlockPhysRight; j++) {
		for (int32 i = 0; i < _brickInfoBuffer[j]; i++) {
			const BrickEntry *currBrickEntry = getBrickEntry(j, i);

			if (currBrickEntry->posY + 38 > _engine->_interface->_clip.top && currBrickEntry->posY <= _engine->_interface->_clip.bottom && currBrickEntry->y >= y) {
				if (currBrickEntry->x == x && currBrickEntry->z == z) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->_workVideoBuffer);
				}

				if (currBrickEntry->x > x || currBrickEntry->z > z) {
					copyGridMask(currBrickEntry->index, (j * 24) - 24, currBrickEntry->posY, _engine->_workVideoBuffer);
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
		if (!_brickUsageTable[b]) {
			continue;
		}
		if (_brickMaskTable[b]) {
			free(_brickMaskTable[b]);
		}
		_brickMaskTable[b] = (uint8 *)malloc(_brickSizeTable[b]);
		processGridMask(_brickTable[b], _brickMaskTable[b]);
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
	uint32 currentBllEntryIdx = 1;

	memset(_brickSizeTable, 0, sizeof(_brickSizeTable));
	memset(_brickUsageTable, 0, sizeof(_brickUsageTable));

	// get block libraries usage bits
	const uint8 *ptrToBllBits = _currentGrid + (_currentGridSize - 32);

	// for all bits under the 32bytes (256bits)
	for (uint32 i = 1; i < 256; i++) {
		const uint8 currentBitByte = *(ptrToBllBits + (i / 8));
		const uint8 currentBitMask = 1 << (7 - (i & 7));

		if (currentBitByte & currentBitMask) {
			const BlockData *currentBllPtr = getBlockLibrary(currentBllEntryIdx);
			for (const BlockDataEntry &entry : currentBllPtr->entries) {
				uint16 brickIdx = entry.brickIdx;
				if (!brickIdx) {
					continue;
				}
				brickIdx--;
				if (brickIdx <= firstBrick) {
					firstBrick = brickIdx;
				}

				if (brickIdx > lastBrick) {
					lastBrick = brickIdx;
				}

				_brickUsageTable[brickIdx] = 1;
			}
		}
		++currentBllEntryIdx;
	}

	for (uint32 i = firstBrick; i <= lastBrick; i++) {
		if (!_brickUsageTable[i]) {
			free(_brickTable[i]);
			_brickTable[i] = nullptr;
			continue;
		}
		_brickSizeTable[i] = HQR::getAllocEntry(&_brickTable[i], Resources::HQR_LBA_BRK_FILE, i);
		if (_brickSizeTable[i] == 0) {
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
			const uint16 gridIdx = stream.readUint16LE();
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
			const uint16 gridIdx = stream.readUint16LE();
			for (int32 i = 0; i < blockCount; i++) {
				outstream.writeUint16LE(gridIdx);
			}
		}
		assert(!outstream.err());
		assert(!stream.err());
	} while (--brickCount);
}

void Grid::createGridMap() {
	int32 blockOffset = 0;

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		const int32 gridIdx = z * GRID_SIZE_X;

		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			const int32 gridOffset = READ_LE_UINT16(_currentGrid + 2 * (x + gridIdx));
			createGridColumn(_currentGrid + gridOffset, _currentGridSize - gridOffset, _blockBuffer + blockOffset, _blockBufferSize - blockOffset);
			blockOffset += 2 * GRID_SIZE_Y;
		}
	}
}

void Grid::createCellingGridMap(const uint8 *gridPtr, int32 gridPtrSize) {
	int32 currGridOffset = 0;
	int32 blockOffset = 0;

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		const uint8 *tempGridPtr = gridPtr + currGridOffset;

		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			const int gridOffset = READ_LE_UINT16(tempGridPtr);
			tempGridPtr += 2;
			createCellingGridColumn(gridPtr + gridOffset, gridPtrSize - gridOffset, _blockBuffer + blockOffset, _blockBufferSize - blockOffset);
			blockOffset += 2 * GRID_SIZE_Y;
		}
		currGridOffset += GRID_SIZE_X + GRID_SIZE_Z;
	}
}

bool Grid::initGrid(int32 index) {
	// load grids from file
	_currentGridSize = HQR::getAllocEntry(&_currentGrid, Resources::HQR_LBA_GRI_FILE, index);
	if (_currentGridSize == 0) {
		warning("Failed to load grid index: %i", index);
		return false;
	}

	// load layouts from file
	if (!_currentBlockLibrary.loadFromHQR(Resources::HQR_LBA_BLL_FILE, index, _engine->isLBA1())) {
		warning("Failed to load block library index: %i", index);
		return false;
	}

	loadGridBricks();

	createGridMask();

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
	_engine->_redraw->_reqBgRedraw = true;
	return true;
}

bool Grid::drawBrick(int32 index, int32 posX, int32 posY) {
	return drawBrickSprite(index, posX, posY, _brickTable[index], false);
}

bool Grid::drawSprite(int32 index, int32 posX, int32 posY, const uint8 *ptr) {
	ptr = ptr + READ_LE_INT32(ptr + index * 4);
	return drawBrickSprite(index, posX, posY, ptr, true);
}

bool Grid::drawSprite(int32 posX, int32 posY, const SpriteData &ptr, int spriteIndex) {
	const int32 left = posX + ptr.offsetX(spriteIndex);
	if (left > _engine->_interface->_clip.right) {
		return false;
	}
	const int32 right = ptr.surface(spriteIndex).w + left;
	if (right < _engine->_interface->_clip.left) {
		return false;
	}
	const int32 top = posY + ptr.offsetY(spriteIndex);
	if (top > _engine->_interface->_clip.bottom) {
		return false;
	}
	const int32 bottom = ptr.surface(spriteIndex).h + top;
	if (bottom < _engine->_interface->_clip.top) {
		return false;
	}

	const Common::Point pos(left, top);
	_engine->_frontVideoBuffer.transBlitFrom(ptr.surface(spriteIndex), pos);
	return true;
}

// WARNING: Rewrite this function to have better performance
bool Grid::drawBrickSprite(int32 index, int32 posX, int32 posY, const uint8 *ptr, bool isSprite) {
	if (!_engine->_interface->_clip.isValidRect()) {
		return false;
	}

	const int32 left = posX + *(ptr + 2);
	if (left > _engine->_interface->_clip.right) {
		return false;
	}
	const int32 right = *ptr + left;
	if (right < _engine->_interface->_clip.left) {
		return false;
	}
	const int32 top = posY + *(ptr + 3);
	if (top > _engine->_interface->_clip.bottom) {
		return false;
	}
	const int32 bottom = (int32)*(ptr + 1) + top;
	if (bottom < _engine->_interface->_clip.top) {
		return false;
	}
	const int32 maxY = MIN(bottom, (int32)_engine->_interface->_clip.bottom);

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
				if (y < _engine->_interface->_clip.top || x >= _engine->_interface->_clip.right || x + iterations < _engine->_interface->_clip.left) {
					if (type == 1) {
						ptr += iterations;
					} else {
						++ptr;
					}
					x += iterations;
					continue;
				}
				if (type == 1) {
					uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(x, y);
					for (uint8 i = 0; i < iterations; i++) {
						if (x >= _engine->_interface->_clip.left && x < _engine->_interface->_clip.right) {
							*out = *ptr;
						}

						++out;
						++x;
						++ptr;
					}
				} else {
					const uint8 pixel = *ptr++;
					uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(x, y);
					for (uint8 i = 0; i < iterations; i++) {
						if (x >= _engine->_interface->_clip.left && x < _engine->_interface->_clip.right) {
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

	Common::Rect rect(left, top, right, bottom);
	_engine->_frontVideoBuffer.addDirtyRect(rect);

	return true;
}

const uint8 *Grid::getBlockBufferGround(const IVec3 &pos, int32 &ground) {
	const IVec3 &collision = updateCollisionCoordinates(pos.x, pos.y, pos.z);
	const uint8 *ptr = _blockBuffer
					   + collision.y * sizeof(int16)
					   + collision.x * GRID_SIZE_Y * sizeof(int16)
					   + collision.z * GRID_SIZE_X * GRID_SIZE_Y * sizeof(int16);

	int32 collisionY = collision.y;
	while (collisionY) {
		if (READ_LE_INT16(ptr)) { // found the ground
			break;
		}
		collisionY--;
		ptr -= sizeof(int16);
	}

	_engine->_collision->_collision.y = collisionY;
	ground = (int16)((collisionY + 1) * BRICK_HEIGHT);

	return ptr;
}

const BlockDataEntry* Grid::getBlockPointer(int32 blockIdx, int32 brickIdx) const {
	const BlockData *blockPtr = getBlockLibrary(blockIdx);
	return &blockPtr->entries[brickIdx];
}

const BlockData *Grid::getBlockLibrary(int32 blockIdx) const {
	return _currentBlockLibrary.getLayout(blockIdx - 1);
}

void Grid::getBrickPos(int32 x, int32 y, int32 z) {
	_brickPixelPosX = (x - z) * 24 + _engine->width() / 2 - GRID_SIZE_X / 2;
	_brickPixelPosY = ((x + z) * 12) - (y * 15) + _engine->height() / 2 - GRID_SIZE_Y;
}

void Grid::drawColumnGrid(int32 blockIdx, int32 brickBlockIdx, int32 x, int32 y, int32 z) {
	const BlockDataEntry *blockPtr = getBlockPointer(blockIdx, brickBlockIdx);
	const uint8 brickShape = blockPtr->brickShape;
	const uint8 brickSound = blockPtr->brickType;
	const uint16 brickIdx = blockPtr->brickIdx;
	if (!brickIdx) {
		return;
	}

	getBrickPos(x - _newCamera.x, y - _newCamera.y, z - _newCamera.z);

	if (_brickPixelPosX < -24) {
		return;
	}
	if (_brickPixelPosX >= _engine->width()) {
		return;
	}
	if (_brickPixelPosY < -38) {
		return;
	}
	if (_brickPixelPosY >= _engine->height()) {
		return;
	}

	// draw the background brick
	drawBrick(brickIdx - 1, _brickPixelPosX, _brickPixelPosY);

	int32 brickBuffIdx = (_brickPixelPosX + 24) / 24;

	if (_brickInfoBuffer[brickBuffIdx] >= MAXBRICKS) {
		warning("GRID: brick buffer exceeded");
		return;
	}

	BrickEntry *currBrickEntry = &_bricksDataBuffer[brickBuffIdx * MAXBRICKS + _brickInfoBuffer[brickBuffIdx]];

	currBrickEntry->x = x;
	currBrickEntry->y = y;
	currBrickEntry->z = z;
	currBrickEntry->posX = _brickPixelPosX;
	currBrickEntry->posY = _brickPixelPosY;
	currBrickEntry->index = brickIdx - 1;
	currBrickEntry->shape = brickShape;
	currBrickEntry->sound = brickSound;

	_brickInfoBuffer[brickBuffIdx]++;
}

void Grid::redrawGrid() {
	_camera.x = _newCamera.x * BRICK_SIZE;
	_camera.y = _newCamera.y * BRICK_HEIGHT;
	_camera.z = _newCamera.z * BRICK_SIZE;

	memset(_brickInfoBuffer, 0, _brickInfoBufferSize);

	if (!_engine->_scene->_enableGridTileRendering) {
		return;
	}

	for (int32 z = 0; z < GRID_SIZE_Z; z++) {
		for (int32 x = 0; x < GRID_SIZE_X; x++) {
			for (int32 y = 0; y < GRID_SIZE_Y; y++) {
				const BlockEntry entry = getBlockEntry(x, y, z);
				if (entry.blockIdx) {
					drawColumnGrid(entry.blockIdx, entry.brickBlockIdx, x, y, z);
				}
			}
		}
	}
}

BlockEntry Grid::getBlockEntry(int32 x, int32 y, int32 z) const {
	const uint8 *blockBufferPtr = _blockBuffer;
	blockBufferPtr += x * GRID_SIZE_Y * 2;
	blockBufferPtr += y * 2;
	blockBufferPtr += (z * GRID_SIZE_X * 2) * GRID_SIZE_Y;

	BlockEntry entry;
	entry.blockIdx = *blockBufferPtr;
	entry.brickBlockIdx = *(blockBufferPtr + 1);
	return entry;
}

ShapeType Grid::getBrickShape(int32 x, int32 y, int32 z) {
	const IVec3 &collision = updateCollisionCoordinates(x, y, z);

	if (collision.x < 0 || collision.x >= GRID_SIZE_X) {
		return ShapeType::kNone;
	}

	if (collision.y <= -1) {
		return ShapeType::kSolid;
	}

	if (collision.y < 0 || collision.y >= GRID_SIZE_Y || collision.z < 0 || collision.z >= GRID_SIZE_Z) {
		return ShapeType::kNone;
	}

	const BlockEntry entry = getBlockEntry(collision.x, collision.y, collision.z);
	if (entry.blockIdx) {
		const BlockDataEntry *blockPtr = getBlockPointer(entry.blockIdx, entry.brickBlockIdx);
		return (ShapeType)blockPtr->brickShape;
	}
	return (ShapeType)entry.brickBlockIdx;
}

const IVec3 &Grid::updateCollisionCoordinates(int32 x, int32 y, int32 z) {
	_engine->_collision->_collision.x = (x + BRICK_HEIGHT) / BRICK_SIZE;
	_engine->_collision->_collision.y = y / BRICK_HEIGHT;
	_engine->_collision->_collision.z = (z + BRICK_HEIGHT) / BRICK_SIZE;
	return _engine->_collision->_collision;
}

ShapeType Grid::getBrickShapeFull(int32 x, int32 y, int32 z, int32 y2) {
	const IVec3 &collision = updateCollisionCoordinates(x, y, z);

	if (collision.x < 0 || collision.x >= GRID_SIZE_X) {
		return ShapeType::kNone;
	}

	if (collision.y <= -1) {
		return ShapeType::kSolid;
	}

	if (collision.y < 0 || collision.y >= GRID_SIZE_Y || collision.z < 0 || collision.z >= GRID_SIZE_Z) {
		return ShapeType::kNone;
	}

	uint8 *blockBufferPtr = _blockBuffer;
	blockBufferPtr += collision.x * GRID_SIZE_Y * 2;
	blockBufferPtr += collision.y * 2;
	blockBufferPtr += (collision.z * GRID_SIZE_X * 2) * GRID_SIZE_Y;

	uint8 blockIdx = *blockBufferPtr;

	if (blockIdx) {
		const uint8 tmpBrickIdx = *(blockBufferPtr + 1);
		const BlockDataEntry *blockPtr = getBlockPointer(blockIdx, tmpBrickIdx);
		const ShapeType brickShape = (ShapeType)blockPtr->brickShape;

		const int32 newY = (y2 + (BRICK_HEIGHT - 1)) / BRICK_HEIGHT;
		int32 currY = collision.y;

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
	int32 currY = collision.y;

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

uint8 Grid::getBrickSoundType(int32 x, int32 y, int32 z) {
	const IVec3 &collision = updateCollisionCoordinates(x, y, z);

	if (collision.x < 0 || collision.x >= GRID_SIZE_X) {
		return 0; // none
	}

	if (collision.y <= -1) {
		return 1; // solid
	}

	if (collision.y < 0 || collision.y >= GRID_SIZE_Y || collision.z < 0 || collision.z >= GRID_SIZE_Z) {
		return 0; // none
	}

	const BlockEntry entry = getBlockEntry(collision.x, collision.y, collision.z);
	if (entry.blockIdx) {
		const BlockDataEntry *blockPtr = getBlockPointer(entry.blockIdx, entry.brickBlockIdx);
		return blockPtr->brickType;
	}

	return 0xF0U;
}

void Grid::centerOnActor(const ActorStruct* actor) {
	_newCamera.x = (actor->_pos.x + BRICK_HEIGHT) / BRICK_SIZE;
	_newCamera.y = (actor->_pos.y + BRICK_HEIGHT) / BRICK_HEIGHT;
	_newCamera.z = (actor->_pos.z + BRICK_HEIGHT) / BRICK_SIZE;
	_engine->_redraw->_reqBgRedraw = true;
}

void Grid::centerScreenOnActor() {
	if (_engine->_disableScreenRecenter) {
		return;
	}
	if (_engine->_debugGrid->_useFreeCamera) {
		return;
	}

	ActorStruct *actor = _engine->_scene->getActor(_engine->_scene->_currentlyFollowedActor);
	_engine->_renderer->projectPositionOnScreen(actor->_pos.x - (_newCamera.x * BRICK_SIZE),
	                                   actor->_pos.y - (_newCamera.y * BRICK_HEIGHT),
	                                   actor->_pos.z - (_newCamera.z * BRICK_SIZE));
	// TODO: these border values should get scaled for higher resolutions
	if (_engine->_renderer->_projPos.x < 80 || _engine->_renderer->_projPos.x >= _engine->width() - 60 || _engine->_renderer->_projPos.y < 80 || _engine->_renderer->_projPos.y >= _engine->height() - 50) {
		_newCamera.x = ((actor->_pos.x + BRICK_HEIGHT) / BRICK_SIZE) + (((actor->_pos.x + BRICK_HEIGHT) / BRICK_SIZE) - _newCamera.x) / 2;
		_newCamera.y = actor->_pos.y / BRICK_HEIGHT;
		_newCamera.z = ((actor->_pos.z + BRICK_HEIGHT) / BRICK_SIZE) + (((actor->_pos.z + BRICK_HEIGHT) / BRICK_SIZE) - _newCamera.z) / 2;

		if (_newCamera.x >= GRID_SIZE_X) {
			_newCamera.x = GRID_SIZE_X - 1;
		}

		if (_newCamera.z >= GRID_SIZE_Z) {
			_newCamera.z = GRID_SIZE_Z - 1;
		}

		_engine->_redraw->_reqBgRedraw = true;
	}
}

} // namespace TwinE
