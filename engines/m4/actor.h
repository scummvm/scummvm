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

#ifndef M4_ACTOR_H
#define M4_ACTOR_H

#include "common/array.h"

#include "m4/m4.h"
#include "m4/scene.h"
#include "m4/graphics.h"
#include "m4/assets.h"

namespace M4 {

struct InventoryObject {
	const char* name;
	int32 scene;
	int32 icon;
};

enum inventoryObjectFlags {
	UNKNOWN_OBJECT = 997,
	BACKPACK = 998,
	NOWHERE = 999
};

enum WalkerDirection {
	kFacingNorth		= 1,	// has shadow
	kFacingNorthEast	= 2,	// has shadow
	kFacingEast			= 3,	// has shadow
	kFacingSouthEast	= 4,	// has shadow
	kFacingSouth		= 5,	// has shadow
	// 6 is unused
	kFacingSouthAlt		= 7,	// no shadow
	kFacingSouthWest	= 8,	// no shadow
	kFacingWest			= 9		// no shadow
};

class Actor {
public:
	Actor(MadsM4Engine *vm);
	~Actor();
	void placeWalkerSpriteAt(int spriteNum, int x, int y);
	void setWalkerScaling(int scaling) { _scaling = scaling; }
	int getWalkerScaling() { return _scaling; }
	void setWalkerDirection(uint8 direction) { _direction = direction; }
	uint8 getWalkerDirection() { return _direction; }
	void setWalkerPalette();
	int getWalkerWidth();
	int getWalkerHeight();
private:
	MadsM4Engine *_vm;
	int _scaling;
	uint8 _direction;
	Common::Array<SpriteAsset*> _walkerSprites;

	void loadWalkers();
	void loadWalkerDirection(uint8 direction);
	void unloadWalkers();
};

// TODO: perhaps the inventory and its view could be merged?
// TODO: the original game capitalizes all inventory object names
// internally, which we do as well, but perhaps we could make sure
// that all object names are parsed with the same case and avoid
// case-insensitive string comparing through scumm_stricmp, using
// the normal strcmp method instead
class Inventory {
public:
	Inventory(MadsM4Engine *vm);
	~Inventory();
	void clear();
	void registerObject(char* name, int32 scene, int32 icon);
	void moveObject(char* name, int32 scene);
	void giveToPlayer(char* name) { moveObject(name, BACKPACK); }
	void addToBackpack(uint32 objectIndex);
	void removeFromBackpack(uint32 objectIndex);
	bool isInBackpack(char* name) { return (getScene(name) == BACKPACK); }
	bool isInScene(char* name, int32 scene) { return (getScene(name) == scene); }
	bool isInCurrentScene(char* name);
	int getScene(char* name);
	int getIcon(char* name);
	int getIndex(char* name);
	int getTotalItems() { return _inventory.size(); }

private:
	MadsM4Engine *_vm;
	Common::Array<InventoryObject *> _inventory;
};

} // End of namespace M4


#endif
