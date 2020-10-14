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

#ifndef TWINE_GRID_H
#define TWINE_GRID_H

#include "common/scummsys.h"
#include "twine/actor.h"

namespace TwinE {

enum ShapeType {
	kNone = 0,
	kSolid = 1,
	kStairsTopLeft = 2,
	kStairsTopRight = 3,
	kStairsBottomLeft = 4,
	kStairsBottomRight = 5,
	kDoubleSideStairsTop1 = 6,
	kDoubleSideStairsBottom1 = 7,
	kDoubleSideStairsLeft1 = 8,
	kDoubleSideStairsRight1 = 9,
	kDoubleSideStairsTop2 = 10,
	kDoubleSideStairsBottom2 = 11,
	kDoubleSideStairsLeft2 = 12,
	kDoubleSideStairsRight2 = 13,
	kFlatBottom1 = 14,
	kFlatBottom2 = 15
};

/** Block fragment entry */
struct BlockEntry {
	/** Block library index */
	uint8 blockIdx = 0;
	/** Brick index inside the block library */
	uint8 brickBlockIdx = 0;
};
/** Brick entry data */
typedef struct BrickEntry {
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
} BrickEntry;

/** Total number of bricks allowed in the game */
#define NUM_BRICKS 9000

/** Total number of bricks allowed in the game */
#define CELLING_GRIDS_START_INDEX 120

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
	void getBrickPos(int32 x, int32 y, int32 z);
	/**
	 * Create celling grid map from celling grid to block library buffer
	 * @param gridPtr celling grid buffer pointer
	 */
	void createCellingGridMap(uint8 *gridPtr);
	/**
	 * Create grid Y column in block buffer
	 * @param gridEntry current grid index
	 * @param dest destination block buffer
	 */
	void createCellingGridColumn(uint8 *gridEntry, uint8 *dest);
	/**
	 * Create grid Y column in block buffer
	 * @param gridEntry current grid index
	 * @param dest destination block buffer
	 */
	void createGridColumn(uint8 *gridEntry, uint8 *dest);
	/**
	 * Load grid bricks according with block librarie usage
	 * @param gridSize size of the current grid
	 * @return true if everything went ok
	 */
	int32 loadGridBricks(int32 gridSize);
	/** Create grid masks to allow display actors over the bricks */
	void createGridMask();
	/**
	 * Process brick masks to allow actors to display over the bricks
	 * @param buffer brick pointer buffer
	 * @param ptr brick mask pointer buffer
	 */
	int processGridMask(uint8 *buffer, uint8 *ptr);
	/**
	 * Copy grid mask to allow actors to display over the bricks
	 * @param index current brick index
	 * @param x grid X coordinate
	 * @param y grid Y coordinate
	 * @param buffer work video buffer
	 */
	void copyGridMask(int32 index, int32 x, int32 y, uint8 *buffer);

public:
	Grid(TwinEEngine *engine);

	/** Table with all loaded bricks */
	uint8 *brickTable[NUM_BRICKS]{nullptr};
	/** Table with all loaded bricks masks */
	uint8 *brickMaskTable[NUM_BRICKS]{nullptr};
	/** Table with all loaded bricks sizes */
	uint32 brickSizeTable[NUM_BRICKS]{0};
	/** Table with all loaded bricks usage */
	uint8 brickUsageTable[NUM_BRICKS]{0};

	/** Current grid pointer */
	uint8 *currentGrid = nullptr;
	/** Current block library pointer */
	uint8 *currentBll = nullptr;
	/** Number of block libraries */
	int32 numberOfBll = 0;

	/** Grid block entry types */
	typedef struct BlockEntry blockMap[64][64][25];

	/** Brick data buffer */
	BrickEntry bricksDataBuffer[28][150];
	/** Brick info buffer */
	int16 brickInfoBuffer[28]{0};

	/** Current brick pixel X position */
	int32 brickPixelPosX = 0;
	/** Current brick pixel Y position */
	int32 brickPixelPosY = 0;

	/** New grid camera X coordinates */
	int32 newCameraX = 0;
	/** New grid camera Y coordinates */
	int32 newCameraY = 0;
	/** New grid camera Z coordinates */
	int32 newCameraZ = 0;

	/** Current grid camera X coordinates */
	int32 cameraX = 0;
	/** Current grid camera Y coordinates */
	int32 cameraY = 0;
	/** Current grid camera Z coordinates */
	int32 cameraZ = 0;

	/** Celling grid brick block buffer */
	uint8 *blockBuffer = nullptr;

	/** Flag to know if the engine is using celling grids */
	int16 useCellingGrid = 0; // useAnotherGrm
	/** Current celling grid index */
	int16 cellingGridIdx = 0; // currentGrid2

	/**
	 * Draw 3D actor over bricks
	 * @param X actor.x coordinate
	 * @param Y actor.y coordinate
	 * @param Z actor.z coordinate
	 */
	void drawOverModelActor(int32 X, int32 Y, int32 Z);

	/**
	 * Draw sprite actor over bricks
	 * @param X actor.x coordinate
	 * @param Y actor.y coordinate
	 * @param Z actor.z coordinate
	 */
	void drawOverSpriteActor(int32 X, int32 Y, int32 Z);

	/**
	 * Get sprite width and height sizes
	 * @param offset sprite pointer offset
	 * @param width sprite width size
	 * @param height sprite height size
	 * @param spritePtr sprite buffer pointer
	 */
	void getSpriteSize(int32 offset, int32 *width, int32 *height, uint8 *spritePtr);

	/**
	 * Draw brick sprite in the screen
	 * @param index brick index to draw
	 * @param posX brick X position to draw
	 * @param posY brick Y position to draw
	 */
	void drawBrick(int32 index, int32 posX, int32 posY);

	/**
	 * Draw sprite in the screen
	 * @param index sprite index to draw
	 * @param posX sprite X position to draw
	 * @param posY sprite Y position to draw
	 * @param ptr sprite buffer pointer to draw
	 */
	void drawSprite(int32 index, int32 posX, int32 posY, uint8 *spritePtr);

	/**
	 * Draw sprite or bricks in the screen according with the type
	 * @param index sprite index to draw
	 * @param posX sprite X position to draw
	 * @param posY sprite Y position to draw
	 * @param ptr sprite buffer pointer to draw
	 * @param isSprite allows to identify if the sprite to display is brick or a single sprite
	 */
	void drawBrickSprite(int32 index, int32 posX, int32 posY, uint8 *spritePtr, bool isSprite);

	/**
	 * Get block library
	 * @param index block library index
	 * @return pointer to the current block index
	 */
	uint8 *getBlockLibrary(int32 index);

	/** Create grid map from current grid to block library buffer */
	void createGridMap();

	/**
	 * Initialize grid (background scenearios)
	 * @param index grid index number
	 */
	int32 initGrid(int32 index);

	/**
	 * Initialize celling grid (background scenearios)
	 * @param index grid index number
	 */
	int32 initCellingGrid(int32 index);

	/** Redraw grid background */
	void redrawGrid();

	int32 getBrickShape(int32 x, int32 y, int32 z);

	int32 getBrickShapeFull(int32 x, int32 y, int32 z, int32 y2);

	int32 getBrickSoundType(int32 x, int32 y, int32 z);
};

} // namespace TwinE

#endif
