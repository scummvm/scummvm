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

#ifndef SAGA2_GRABINFO_H
#define SAGA2_GRABINFO_H

namespace Saga2 {
/* ===================================================================== *
   class  GrabInfo
   this will need to be transfered to grabinfo.h
 * ===================================================================== */

class GrabInfo {

public:

	enum Intent {
		None,
		WalkTo,
		PickUp,
		Open,
		Drop,
		Use,
		Attack,
		Cast,               // for spells

		IntentCounts        // dummy to count enum
	};

private:

	enum {
		bufSize = 60
	};

protected:

	// bitmaps for pointer
	gPixelMap   pointerMap;
	Point16     pointerOffset;          // mouse ptr hotspot

	Location    from;                   // where the item was last

	ObjectID    grabId;                 // which object picked by mouse
	GameObject  *grabObj;               // object being dragged
	Intent      intention;              // pickup state
	bool        intentDoable;           // is intention doable?
	// (i.e. display red X cursor)
	bool        displayGauge;           // indicates wether or not to show
	// the gauge
	int16       gaugeNumerator,         // values to be displayed on the
	            gaugeDenominator;       // gauge

	int16       moveCount;              // number of items being moved in cursor

	char        textBuf[bufSize];

	// internal grab commonality
	void setIcon(void);
	void clearIcon(void);

	// set cursor image based on 'intention' and 'intentDoable'
	void setCursor(void);

public:
	// there will probably be only one GrabInfo, created globally
	GrabInfo(void);

	~GrabInfo(void);

	void    selectImage(uint8 index);

	// set the move count based on val and whether the object is
	// mergeable or not.
	void    setMoveCount(int16 val);
	int16   getMoveCount(void) {
		return moveCount;
	}

	// put object into mouse ptr
	void    grabObject(ObjectID objid,  Intent in = Drop, int16 count = 1);
	void    grabObject(GameObject *obj, Intent in = Drop, int16 count = 1);

	void    copyObject(ObjectID objid,  Intent in = Drop, int16 count = 1);
	void    copyObject(GameObject *obj, Intent in = Drop, int16 count = 1);

	// non-destructive reads of the state
	uint8       getIntent(void)       {
		return intention;
	}
	bool        getDoable(void)       {
		return intentDoable;
	}

	// changes to GrabInfo state
	uint8       setIntent(uint8 in);
	void        setDoable(bool doable) {
		if (doable != intentDoable) {
			intentDoable = doable;
			setCursor();
		}
	}

	GameObject  *getObject(void)  {
		return grabObj;
	}
	ObjectID    getObjectId(void) {
		return grabId;
	}

	// free the cursor
	void placeObject(const Location &loc);
	void replaceObject(void);

	//  request a change to the mouse cursor text
	void setText(const char *txt);

	//  request a change to the mouse gauge
	void setGauge(int16 numerator, int16 denominator);

	//  clear the mouse gauge
	void clearGauge(void);
};

} // end of namespace Saga2

#endif
