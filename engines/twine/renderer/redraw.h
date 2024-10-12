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

#ifndef TWINE_RENDERER_REDRAW_H
#define TWINE_RENDERER_REDRAW_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"
#include "twine/shared.h"

namespace TwinE {

// MAX_INCRUST_DISP
#define OVERLAY_MAX_ENTRIES 10

enum class OverlayType {
	koSprite = 0,        // INCRUST_SPRITE
	koNumber = 1,        // INCRUST_NUM
	koNumberRange = 2,   // INCRUST_CMPT
	koInventoryItem = 3, // INCRUST_OBJ
	koText = 4,          // INCRUST_TEXT
	koInventory = 5,     // lba2 (INCRUST_INVENTORY)
	koSysText = 6,       // lba2 (INCRUST_SYS_TEXT)
	koFlash = 7,         // lba2 (INCRUST_ECLAIR)
	koRain = 8,          // lba2 (INCRUST_PLUIE)
	koMax
};

// lba2
#define INCRUST_YCLIP (1 << 8)

enum class OverlayPosType {
	koNormal = 0,
	koFollowActor = 1
};

/** Overlay list structure */
struct OverlayListStruct {
	int16 num = 0; // sprite/3d model entry | number | number range
	int16 x = 0;
	int16 y = 0;
	OverlayType type = OverlayType::koSprite;
	int16 info = 0; // text = actor | total coins
	OverlayPosType move = OverlayPosType::koNormal;
	int16 timerEnd = 0; // life time in ticks - see toSeconds()
};

struct DrawListStruct {
	int16 z = 0; // depth sorting value
	uint32 type = 0;
	// NumObj was also used with mask of type and numObj - we are
	// not masking the value in numObj, but store the type in type
	uint16 numObj = 0;

	uint16 xw = 0;
	uint16 yw = 0;
	uint16 zw = 0;
	uint16 num = 0;

	inline bool operator==(const DrawListStruct& other) const {
		return z == other.z;
	}

	inline bool operator<(const DrawListStruct& other) const {
		return z < other.z;
	}
};

#define TYPE_OBJ_SHIFT (10)
#define TYPE_OBJ_FIRST (1 << TYPE_OBJ_SHIFT) // 1024
#define NUM_OBJ_MASK (TYPE_OBJ_FIRST - 1)

class TwinEEngine;
class Redraw {
private:
	TwinEEngine *_engine;
	enum DrawListType {
		DrawObject3D = (0 << TYPE_OBJ_SHIFT), // TYPE_OBJ_3D
		DrawFlagRed = (1 << TYPE_OBJ_SHIFT),
		DrawFlagYellow = (2 << TYPE_OBJ_SHIFT),
		DrawShadows = (3 << TYPE_OBJ_SHIFT), // TYPE_SHADOW
		DrawActorSprites = (4 << TYPE_OBJ_SHIFT), // TYPE_OBJ_SPRITE
		DrawZoneDec = (5 << TYPE_OBJ_SHIFT),
		DrawExtras = (6 << TYPE_OBJ_SHIFT), // TYPE_EXTRA
		DrawPrimitive = (7 << TYPE_OBJ_SHIFT)
	};

	Common::Rect _currentRedrawList[300];
	Common::Rect _nextRedrawList[300];

	int16 _overlayRotation = 0;

	/** Save last actor that bubble dialog icon */
	int32 _bubbleActor = -1;
	int32 _bubbleSpriteIndex;

	IVec3 _projPosScreen; // XpOrgw, YpOrgw

	// big font shadow text in the lower left corner
	Common::String _text;
	int32 _textDisappearTime = -1;

	/**
	 * Add a certain region to the current redraw list array
	 * @param redrawArea redraw the region
	 */
	void addRedrawCurrentArea(const Common::Rect &redrawArea);
	/**
	 * Move next regions to the current redraw list,
	 * setup the redraw areas for next display
	 */
	void moveNextAreas();
	void updateOverlayTypePosition(int16 x1, int16 y1, int16 x2, int16 y2);

	void processDrawListShadows(const DrawListStruct& drawCmd);
	void processDrawListActors(const DrawListStruct& drawCmd, bool bgRedraw);
	void processDrawListActorSprites(const DrawListStruct& drawCmd, bool bgRedraw);
	void processDrawListExtras(const DrawListStruct& drawCmd);

	int32 fillActorDrawingList(DrawListStruct *drawList, bool bgRedraw);
	int32 fillExtraDrawingList(DrawListStruct *drawList, int32 drawListPos);
	void correctZLevels(DrawListStruct *drawList, int32 drawListPos);
	void processDrawList(DrawListStruct *drawList, int32 drawListPos, bool bgRedraw);
	void renderOverlays();
	void renderText();

public:
	Redraw(TwinEEngine *engine);

	bool _flagMCGA = false;

	/** Request background redraw */
	bool _firstTime = false;

	/** Current number of redraw regions in the screen */
	int32 _currNumOfRedrawBox = 0; // fullRedrawVar8
	/** Number of redraw regions in the screen */
	int32 _numOfRedrawBox = 0;

	int _sceneryViewX = 0; // xmin
	int _sceneryViewY = 0; // ymin

	OverlayListStruct overlayList[OVERLAY_MAX_ENTRIES];

	void setRenderText(const Common::String &text);

	// InitIncrustDisp
	int32 addOverlay(OverlayType type, int16 info0, int16 x, int16 y, int16 info1, OverlayPosType posType, int16 lifeTime);
	void posObjIncrust(OverlayListStruct *ptrdisp, int32 num); // lba2

	/**
	 * Add a certain region to redraw list array
	 * @param left start width to redraw the region
	 * @param top start height to redraw the region
	 * @param right end width to redraw the region
	 * @param bottom end height to redraw the region
	 */
	void addRedrawArea(int32 left, int32 top, int32 right, int32 bottom); // AddPhysBox
	void addRedrawArea(const Common::Rect &rect); // AddPhysBox

	/**
	 * Flip currentRedrawList regions in the screen
	 * This only updates small areas in the screen so few CPU processor is used
	 */
	void flipRedrawAreas();

	/** Blit/Update all screen regions in the currentRedrawList */
	void blitBackgroundAreas();

	/**
	 * This is responsible for the entire game screen redraw
	 * @param bgRedraw true if we want to redraw background grid, false if we want to update certain screen areas
	 */
	void drawScene(bool bgRedraw);

	/** Draw dialogue sprite image */
	void drawBubble(int32 actorIdx);

	/**
	 * Sort drawing list struct ordered as the first objects appear in the top left corner of the screen
	 * @param list drawing list variable which contains information of the drawing objects
	 * @param listSize number of drawing objects in the list
	 */
	void sortDrawingList(DrawListStruct *list, int32 listSize) const;
};

} // namespace TwinE

#endif
