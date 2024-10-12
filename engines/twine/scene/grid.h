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

#ifndef TWINE_SCENE_GRID_H
#define TWINE_SCENE_GRID_H

#define WATER_BRICK (0xF1)

#include "common/scummsys.h"
#include "twine/parser/blocklibrary.h"
#include "twine/parser/sprite.h"
#include "twine/shared.h"

namespace Graphics {
class ManagedSurface;
}

namespace TwinE {

class ActorStruct;

/** Block fragment entry */
struct BlockEntry {
	/** Block library index */
	uint8 blockIdx = 0;
	/** Brick index inside the block library */
	uint8 brickBlockIdx = 0;
};
/** Brick entry data */
struct BrickEntry {
	/** Brick X position in screen */
	int16 x = 0; //z
	/** Brick Y position in screen */
	int16 y = 0;
	/** Brick Z position in screen */
	int16 z = 0; // x
	/** Brick pixel X position */
	int16 posX = 0;
	/** Brick pixel Y position */
	int16 posY = 0;
	/** Brick index */
	int16 index = 0;
	/** Brick shape type */
	uint8 shape = 0;
	/** Brick sound type */
	uint8 sound = 0;
};

/** Total number of bricks allowed in the game */
#define NUM_BRICKS 9000

/** Grid X size */
#define SIZE_CUBE_X 64
/** Grid Y size */
#define SIZE_CUBE_Y 25
/** Grid Z size */
#define SIZE_CUBE_Z SIZE_CUBE_X

#define ISO_SCALE 512
#define SIZE_BRICK_XZ 512
#define SIZE_BRICK_Y 256
#define DEMI_BRICK_XZ 256
#define DEMI_BRICK_Y 128
// short max 32767 0x7FFF
//           32256 0x7E00
//           32000 0x7D00
#define SCENE_SIZE_MAX (SIZE_BRICK_XZ * (SIZE_CUBE_X - 1))
// short min -32768
#define SCENE_SIZE_MIN (-SIZE_BRICK_XZ * SIZE_CUBE_X)
#define SCENE_SIZE_HALF (SIZE_BRICK_XZ * SIZE_CUBE_X / 2)
#define SCENE_SIZE_HALFF (SIZE_BRICK_XZ * SIZE_CUBE_X / 2.0f)

#define MAXBRICKS 150

class TwinEEngine;

class Grid {
private:
	TwinEEngine *_engine;

	/**
	 * Draw a specific brick in the grid column according with the block index
	 * @param blockIdx block library index
	 * @param brickBlockIdx brick index inside the block
	 * @param x column x position
	 * @param y column y position
	 * @param z column z position
	 */
	void drawColumnGrid(int32 blockIdx, int32 brickBlockIdx, int32 x, int32 y, int32 z);
	/**
	 * Get brick position in the screen
	 * @param x column x position in the current camera
	 * @param y column y position in the current camera
	 * @param z column z position in the current camera
	 */
	void getBrickPos(int32 x, int32 y, int32 z, int32 &_brickPixelPosX, int32 &_brickPixelPosY) const;
	/**
	 * Create celling grid map from celling grid to block library buffer
	 * @param gridPtr celling grid buffer pointer
	 */
	void createCellingGridMap(const uint8 *gridPtr, int32 gridPtrSize);
	/**
	 * Create grid Y column in block buffer
	 * @param gridEntry current grid index
	 * @param dest destination block buffer
	 */
	void createCellingGridColumn(const uint8 *gridEntry, uint32 gridEntrySize, uint8 *dest, uint32 destSize);
	/**
	 * Create grid Y column in block buffer
	 * @param gridEntry current grid index
	 * @param dest destination block buffer
	 */
	void decompColumn(const uint8 *gridEntry, uint32 gridEntrySize, uint8 *dest, uint32 destSize);
	/**
	 * Load grid bricks according with block librarie usage
	 */
	void loadGridBricks();
	/** Create grid masks to allow display actors over the bricks */
	void createGridMask();
	/**
	 * Process brick masks to allow actors to display over the bricks
	 * @param buffer brick pointer buffer
	 * @param ptr brick mask pointer buffer
	 */
	void processGridMask(const uint8 *buffer, uint8 *ptr);
	/**
	 * Copy grid mask to allow actors to display over the bricks
	 * @param index current brick index
	 * @param x grid X coordinate
	 * @param y grid Y coordinate
	 * @param buffer work video buffer
	 */
	void copyMask(int32 index, int32 x, int32 y, const Graphics::ManagedSurface &buffer);

	/** Table with all loaded bricks */
	uint8 *_brickTable[NUM_BRICKS]{nullptr};
	/** Table with all loaded bricks masks */
	uint8 *_brickMaskTable[NUM_BRICKS]{nullptr};
	/** Table with all loaded bricks sizes */
	uint32 _brickSizeTable[NUM_BRICKS]{0};
	/** Table with all loaded bricks usage */
	uint8 _brickUsageTable[NUM_BRICKS]{0};

	/** Current grid pointer */
	int32 _currentGridSize = 0;
	uint8 *_currentGrid = nullptr;
	/** Current block library */
	BlockLibraryData _currentBlockLibrary;

	/** Brick data buffer */
	BrickEntry *_bricksDataBuffer = nullptr;
	/** Brick info buffer */
	int16 *_brickInfoBuffer = nullptr;
	int32 _brickInfoBufferSize = 0;

	/** Celling grid brick block buffer */
	int32 _blockBufferSize = 0;
	uint8 *_bufCube = nullptr;

	const BrickEntry* getBrickEntry(int32 j, int32 i) const;

	const IVec3 &updateCollisionCoordinates(int32 x, int32 y, int32 z);

	BlockEntry getBlockEntry(int32 xmap, int32 ymap, int32 zmap) const;

	bool shouldCheckWaterCol(int32 actorIdx) const;
public:
	Grid(TwinEEngine *engine);
	~Grid();

	void init(int32 w, int32 h);

	/**
	 * search down until either ground is found or lower border of the cube is reached
	 */
	const uint8 *getBlockBufferGround(const IVec3 &pos, int32 &ground);

	/** New grid camera x, y and z coordinates */
	IVec3 _startCube; // StartXCube, StartYCube, StartZCube

	/** Current grid camera x, y and z coordinates */
	IVec3 _worldCube; // WorldXCube WorldYCube

	/** Flag to know if the engine is using celling grids */
	int16 _useCellingGrid = 0;
	/** Current celling grid index */
	int16 _cellingGridIdx = 0;

	/**
	 * Draw 3D actor over bricks
	 * @param x actor.x coordinate
	 * @param y actor.y coordinate
	 * @param z actor.z coordinate
	 */
	void drawOverBrick(int32 x, int32 y, int32 z);

	/**
	 * Draw sprite actor over bricks
	 * @param x actor.x coordinate
	 * @param y actor.y coordinate
	 * @param z actor.z coordinate
	 */
	void drawOverBrick3(int32 x, int32 y, int32 z);

	/**
	 * Get sprite width and height sizes
	 * @param offset sprite pointer offset
	 * @param width sprite width size
	 * @param height sprite height size
	 * @param spritePtr sprite buffer pointer
	 */
	void getSpriteSize(int32 offset, int32 *width, int32 *height, const uint8 *spritePtr);

	/** recenter screen on followed actor automatically */
	void centerScreenOnActor();
	void centerOnActor(const ActorStruct* actor);

	/**
	 * Draw brick sprite in the screen
	 * @param index brick index to draw
	 * @param posX brick X position to draw
	 * @param posY brick Y position to draw
	 */
	bool drawBrick(int32 index, int32 posX, int32 posY);

	/**
	 * Draw sprite in the screen
	 * @param index sprite index to draw
	 * @param posX sprite X position to draw
	 * @param posY sprite Y position to draw
	 * @param ptr sprite buffer pointer to draw
	 */
	bool drawSprite(int32 index, int32 posX, int32 posY, const uint8 *spritePtr);
	bool drawSprite(int32 posX, int32 posY, const SpriteData &ptr, int spriteIndex = 0);

	/**
	 * Draw sprite or bricks in the screen according with the type
	 * @param posX sprite X position to draw
	 * @param posY sprite Y position to draw
	 * @param ptr sprite buffer pointer to draw
	 * @param isSprite allows to identify if the sprite to display is brick or a single sprite
	 */
	bool drawBrickSprite(int32 posX, int32 posY, const uint8 *spritePtr, bool isSprite);

	/**
	 * Get block library
	 * @param blockIdx block library index
	 * @return pointer to the current block index
	 */
	const BlockData *getBlockLibrary(int32 blockIdx) const;
	const BlockDataEntry* getAdrBlock(int32 blockIdx, int32 tmpBrickIdx) const;

	/** Create grid map from current grid to block library buffer */
	void copyMapToCube();

	/**
	 * Initialize grid (background scenearios)
	 * @param index grid index number
	 */
	bool initGrid(int32 index);

	/**
	 * Initialize celling grid (background scenearios)
	 * @param index grid index number
	 */
	bool initCellingGrid(int32 index);

	/** Redraw grid background */
	void redrawGrid();

	ShapeType worldColBrick(int32 x, int32 y, int32 z);

	ShapeType worldColBrickFull(int32 x, int32 y, int32 z, int32 y2, int32 actorIdx);

	uint8 worldCodeBrick(int32 x, int32 y, int32 z);

	inline ShapeType worldColBrick(const IVec3 &pos) {
		return worldColBrick(pos.x, pos.y, pos.z);
	}

	inline ShapeType worldColBrickFull(const IVec3 &pos, int32 y2, int32 actorIdx) {
		return worldColBrickFull(pos.x, pos.y, pos.z, y2, actorIdx);
	}
};

} // namespace TwinE

#endif
