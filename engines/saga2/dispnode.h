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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_DISPNODE_H
#define SAGA2_DISPNODE_H

#include "saga2/idtypes.h"
#include "saga2/rect.h"

namespace Saga2 {

class GameObject;
struct TilePoint;
struct StaticTilePoint;

class Effectron;

enum nodeType {
	nodeTypeObject = 0,
	nodeTypeEffect
};


class DisplayNode {
	friend class    DisplayNodeList;
	friend class    SpellDisplayList;
	friend class    GameObject;
	friend ObjectID pickObject(const StaticPoint32 &mouse, StaticTilePoint &objPos);

private:
	DisplayNode             *nextDisplayed;         // pointer to next in display list
	int16                   sortDepth;              // for sorting by depth
	GameObject              *object;                // the object to display
	Point16                 screenCoords;           // screen coordinates
	Rect16                  hitBox;                 // hitbox for clicking this item
	uint8                   flags;                  // various flags

	enum {
		displayIndicator = (1 << 0)
	};

public:
	nodeType                type;
	Effectron               *efx;

	DisplayNode();

	void drawObject();
	void drawEffect();
	void updateObject(const int32 deltaTime);
	void updateEffect(const int32 deltaTime);
	TilePoint SpellPos();
};

/* ============================================================================ *
   Object Display List
 * ============================================================================ */

//  This class is used to form a list of objects to display on
//  the screen.

const int           maxDisplayed = 100;

class DisplayNodeList {
	friend ObjectID pickObject(const StaticPoint32 &mouse, StaticTilePoint &objPos);

public:
	uint16              count;                  // number of entries in list
	DisplayNode         *displayList;       // table of displayed objects
	static DisplayNode  *head;              // head of list

	DisplayNodeList(uint16 newSize) {
		displayList = (DisplayNode *)malloc(sizeof(DisplayNode) * newSize);
		init(newSize);
		count = 0;
	}
	DisplayNodeList() {
		displayList = (DisplayNode *)malloc(sizeof(DisplayNode) * maxDisplayed);
		init(maxDisplayed);
		count = 0;
	}
	~DisplayNodeList() {
		free(displayList);
	}

	void reset() {
		count = 0;
		head = NULL;
	}

	void  init(uint16 s);
	void  buildObjects(bool fromScratch);
	void  buildEffects(bool fromScratch);
	void  draw();
	void  updateOStates(const int32 deltaTime);
	void  updateEStates(const int32 deltaTime);
	bool  dissipated();
};

/* ============================================================================ *
   Function prototypes
 * ============================================================================ */

//  Enable or disable the center actor indicator
void setCenterActorIndicator(bool enabled);

} // end of namespace Saga2

#endif
