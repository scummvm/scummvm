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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_DISPNODE_H
#define SAGA2_DISPNODE_H

#include "saga2/objects.h"
#include "saga2/speldefs.h"

namespace Saga2 {

enum nodeType {
	nodeTypeObject = 0,
	nodeTypeEffect,
};


class DisplayNode {
	friend class    DisplayNodeList;
	friend class    SpellDisplayList;
	friend class    GameObject;
	friend ObjectID pickObject(const Point16 &mouse, TilePoint &objPos);

private:
	DisplayNode             *nextDisplayed;         // pointer to next in display list
	int16                   sortDepth;              // for sorting by depth
	GameObject              *object;                // the object to display
	Point16                 screenCoords;           // screen coordinates
	Rect16                  hitBox;                 // hitbox for clicking this item
	uint8                   flags;                  // various flags

	enum {
		displayIndicator = (1 << 0),
	};

public:
	nodeType                type;
	Effectron               *efx;

	DisplayNode();

	void drawObject(void);
	void drawEffect(void);
	void updateObject(const int32 deltaTime);
	void updateEffect(const int32 deltaTime);
	TilePoint SpellPos(void) {
		if (efx) return efx->current;
		return Nowhere;
	}

	void *operator new (size_t s) {
		return RNewPtr(s, NULL, "Display Node");
	}
	void operator delete (void *m) {
		RDisposePtr(m);
	}

};

/* ============================================================================ *
   Object Display List
 * ============================================================================ */

//  This class is used to form a list of objects to display on
//  the screen.

const int           maxDisplayed = 100;

class DisplayNodeList {
	friend ObjectID pickObject(const Point16 &mouse, TilePoint &objPos);

public:
	uint16              count;                  // number of entries in list
	DisplayNode         *displayList;       // table of displayed objects
	static DisplayNode  *head;              // head of list

	DisplayNodeList(uint16 newSize) {
		displayList = (DisplayNode *)TALLOC(sizeof(DisplayNode) * newSize, memDispNode);
		init(newSize);
		count = 0;
	}
	DisplayNodeList() {
		displayList = (DisplayNode *)TALLOC(sizeof(DisplayNode) * maxDisplayed, memDispNode);
		init(maxDisplayed);
		count = 0;
	}
	~DisplayNodeList() {
		delete [] displayList;
	}

	void reset(void) {
		count = 0;
		head = NULL;
	}

	void  init(uint16 s);
	void  buildObjects(bool fromScratch);
	void  buildEffects(bool fromScratch);
	void  draw(void);
	void  updateOStates(const int32 deltaTime);
	void  updateEStates(const int32 deltaTime);
	bool  dissipated(void);

private:
	int16           sortDepth;              // for sorting by depth
	void *operator new (size_t s) {
		return RNewPtr(s, NULL, "Display List");
	}
	void operator delete (void *m) {
		RDisposePtr(m);
	}
};

/* ============================================================================ *
   Function prototypes
 * ============================================================================ */

//  Enable or disable the center actor indicator
void setCenterActorIndicator(bool enabled);

} // end of namespace Saga2

#endif
