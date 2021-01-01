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

#ifndef TWINE_REDRAW_H
#define TWINE_REDRAW_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace TwinE {

#define OVERLAY_MAX_ENTRIES 10

enum class OverlayType {
	koSprite = 0,
	koNumber = 1,
	koNumberRange = 2,
	koInventoryItem = 3,
	koText = 4
};

enum class OverlayPosType {
	koNormal = 0,
	koFollowActor = 1
};

/** Overlay list structure */
struct OverlayListStruct {
	OverlayType type = OverlayType::koSprite;
	int16 info0 = 0; // sprite/3d model entry | number | number range
	int16 x = 0;
	int16 y = 0;
	int16 info1 = 0; // text = actor | total coins
	OverlayPosType posType = OverlayPosType::koNormal;
	int16 lifeTime = 0;
};

class TwinEEngine;
class Redraw {
private:
	TwinEEngine *_engine;
	enum DrawListType {
		DrawActorSprites = 0x1000,
		DrawExtras = 0x1800,
		DrawShadows = 0xC00
	};

	struct DrawListStruct {
		int16 posValue = 0;
		uint32 type = 0;
		uint16 actorIdx = 0;
		uint16 x = 0;
		uint16 y = 0;
		uint16 z = 0;
		uint16 offset = 0;
		uint16 field_C = 0;
		uint16 field_E = 0;
		uint16 field_10 = 0;
	};

	/** Draw list array to grab the necessary */
	DrawListStruct drawList[150];

	Common::Rect currentRedrawList[300];
	Common::Rect nextRedrawList[300];

	int16 overlayRotation = 0;
	/**
	 * Add a certain region to the current redraw list array
	 * @param redrawArea redraw the region
	 */
	void addRedrawCurrentArea(const Common::Rect &redrawArea);
	/** Move next regions to the current redraw list */
	void moveNextAreas();
	/**
	 * Sort drawing list struct ordered as the first objects appear in the top left corner of the screen
	 * @param list drawing list variable which contains information of the drawing objects
	 * @param listSize number of drawing objects in the list
	 */
	void sortDrawingList(DrawListStruct *list, int32 listSize);
	void updateOverlayTypePosition(int16 x1, int16 y1, int16 x2, int16 y2);

	void processDrawListShadows(const DrawListStruct& drawCmd);
	void processDrawListActors(const DrawListStruct& drawCmd, bool bgRedraw);
	void processDrawListActorSprites(const DrawListStruct& drawCmd, bool bgRedraw);
	void processDrawListExtras(const DrawListStruct& drawCmd);

	int32 fillActorDrawingList(bool bgRedraw);
	int32 fillExtraDrawingList(int32 drawListPos);
	void processDrawList(int32 drawListPos, bool bgRedraw);
	void renderOverlays();

public:
	Redraw(TwinEEngine *engine) : _engine(engine) {}

	/** Auxiliar object render position on screen */
	Common::Rect renderRect { 0, 0, 0, 0 };

	bool drawInGameTransBox = false;

	/** Request background redraw */
	bool reqBgRedraw = false;

	/** Current number of redraw regions in the screen */
	int32 currNumOfRedrawBox = 0; // fullRedrawVar8
	/** Number of redraw regions in the screen */
	int32 numOfRedrawBox = 0;

	/** Save last actor that bubble dialog icon */
	int32 bubbleActor = -1;
	int32 bubbleSpriteIndex = 0;

	OverlayListStruct overlayList[OVERLAY_MAX_ENTRIES];

	void addOverlay(OverlayType type, int16 info0, int16 x, int16 y, int16 info1, OverlayPosType posType, int16 lifeTime);

	/**
	 * Add a certain region to redraw list array
	 * @param left start width to redraw the region
	 * @param top start height to redraw the region
	 * @param right end width to redraw the region
	 * @param bottom end height to redraw the region
	 */
	void addRedrawArea(int32 left, int32 top, int32 right, int32 bottom);
	void addRedrawArea(const Common::Rect &rect);

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
	void redrawEngineActions(bool bgRedraw);

	/** Draw dialogue sprite image */
	void drawBubble(int32 actorIdx);

	void zoomScreenScale();
};

} // namespace TwinE

#endif
