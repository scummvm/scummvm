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

#ifndef SAGA2_GRABINFO_H
#define SAGA2_GRABINFO_H

namespace Saga2 {
/* ===================================================================== *
   class  GrabInfo
   this will need to be transferred to grabinfo.h
 * ===================================================================== */

class GrabInfo {

public:

	enum Intent {
		kIntNone,
		kIntWalkTo,
		kIntPickUp,
		kIntOpen,
		kIntDrop,
		kIntUse,
		kIntAttack,
		kIntCast,               // for spells

		kIntIntentCounts        // dummy to count enum
	};

private:

	enum {
		kBufSize = 60
	};

protected:

	// bitmaps for pointer
	gPixelMap   _pointerMap;
	Point16     _pointerOffset;          // mouse ptr hotspot

	Location    _from;                   // where the item was last

	ObjectID    _grabId;                 // which object picked by mouse
	GameObject  *_grabObj;               // object being dragged
	Intent      _intention;              // pickup state
	bool        _intentDoable;           // is intention doable?
	// (i.e. display red X cursor)
	bool        _displayGauge;           // indicates whether or not to show
	// the gauge
	int16       _gaugeNumerator,         // values to be displayed on the
	            _gaugeDenominator;       // gauge

	int16       _moveCount;              // number of items being moved in cursor

	char        _textBuf[kBufSize];

	// internal grab commonality
	void setIcon();
	void clearIcon();

	// set cursor image based on 'intention' and 'intentDoable'
	void setCursor();

public:
	// there will probably be only one GrabInfo, created globally
	GrabInfo();

	~GrabInfo();

	void    selectImage(uint8 index);

	// set the move count based on val and whether the object is
	// mergeable or not.
	void    setMoveCount(int16 val);
	int16   getMoveCount() {
		return _moveCount;
	}

	// put object into mouse ptr
	void    grabObject(ObjectID objid,  Intent in = kIntDrop, int16 count = 1);
	void    grabObject(GameObject *obj, Intent in = kIntDrop, int16 count = 1);

	void    copyObject(ObjectID objid,  Intent in = kIntDrop, int16 count = 1);
	void    copyObject(GameObject *obj, Intent in = kIntDrop, int16 count = 1);

	// non-destructive reads of the state
	uint8       getIntent()       {
		return _intention;
	}
	bool        getDoable()       {
		return _intentDoable;
	}

	// changes to GrabInfo state
	uint8       setIntent(uint8 in);
	void        setDoable(bool doable) {
		if (doable != _intentDoable) {
			_intentDoable = doable;
			setCursor();
		}
	}

	GameObject  *getObject()  {
		return _grabObj;
	}
	ObjectID    getObjectId() {
		return _grabId;
	}

	// free the cursor
	void placeObject(const Location &loc);
	void replaceObject();

	//  request a change to the mouse cursor text
	void setText(const char *txt);

	//  request a change to the mouse gauge
	void setGauge(int16 numerator, int16 denominator);

	//  clear the mouse gauge
	void clearGauge();
};

} // end of namespace Saga2

#endif
