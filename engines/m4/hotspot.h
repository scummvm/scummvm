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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef M4_HOTSPOT_H
#define M4_HOTSPOT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/util.h"

/*
	TODO:
		- check if hotspot_duplicate is needed
	NOTES:
		- hotspot_add_dynamic unused in Orion Burger
*/

//???: should String be used instead of char* here?

namespace M4 {

class HotSpot {
	friend class HotSpotList;//just for debugging, to be removed later
public:

	HotSpot(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);
	~HotSpot();

	void setVocab(const char *value);
	void setVocabID(int32 value) { _vocabID = value; }
	void setVerb(const char *value);
	void setVerbID(int32 value) { _verbID = value; }
	void setPrep(const char *value);
	void setSprite(const char *value);
	void setActive(bool value) { _active = value; }
	void setCursor(uint8 value) { _cursor = value; }
	void setRect(int x1, int y1, int x2, int y2);
	void setFeet(int x, int y);
	void setFacing(uint8 facing) { _facing = facing; }
	void setArticle(int8 v) { _articleNumber = v; }
	char *getVocab() const { return _vocab; }
	int32 getVocabID() { return _vocabID; }
	char *getVerb() const { return _verb; }
	int32 getVerbID() { return _verbID; }
	char *getPrep() const { return _prep; }
	char *getSprite() const { return _sprite; }
	uint8 getCursor() const { return _cursor; }
	bool getActive() const { return _active; }
	uint8 getFacing() const { return _facing; }
	int getFeetX() { return _feetX; }
	int getFeetY() { return _feetY; }
	int8 getArticle() const { return _articleNumber; }
	Common::Rect getRect() const;
	int getIndex() const { return _index; }
	void setIndex(int index) { _index = index; }

	int32 area() const { return (_rect.width() - 1) * (_rect.height() - 1); }
	bool pointInside(int x, int y) { return _rect.contains(x, y); }

private:
	char *_vocab, *_verb, *_prep, *_sprite;
	Common::Rect _rect;
	bool _active;
	int _feetX, _feetY;
	uint8 _facing, _cursor;
	int _index;

	// Unused in Orion Burger, used in MADS games
	uint8 _syntax;
	int32 _vocabID, _verbID;
	int8 _articleNumber;

	//TODO: check if this is actually needed by the game
	int16 _hash;
};

class HotSpotList {
public:
	HotSpotList();
	~HotSpotList();

	int add(HotSpot *hotspot, bool head = false);
	HotSpot *get(int index) { return _hotspots[index]; }
	HotSpot &operator[](int idx) { return *get(idx); }
	int size() const { return _hotspots.size(); }
	void remove(HotSpot *hotspot);
	void unlinkItem(HotSpot *hotspot);
	void clear();
	HotSpot *findByXY(int x, int y);
	void setActive(const char *name, bool active);
	void setActiveXY(const char *name, int x, int y, bool active);

	void dump();

	void loadHotSpots(Common::SeekableReadStream* hotspotStream, int hotspotCount);

	uint32 readHotSpotInteger(Common::SeekableReadStream* hotspotStream);
private:
	typedef Common::Array<HotSpot*> HotSpotArray;
	HotSpotArray _hotspots;
};


} // End of namespace M4


#endif
