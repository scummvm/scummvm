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

#ifndef MADS_INVENTORY_H
#define MADS_INVENTORY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/serializer.h"

namespace MADS {

enum {
	PLAYER_INVENTORY = 2, NOWHERE = 1
};

class MADSEngine;

#define MAX_VOCAB 5
#define MAX_QUALITIES 4

class InventoryObject {
public:
	int _descId;
	int _roomNumber;
	int _article;
	int _vocabCount;
	int _qualitiesCount;
	int syntax;

	struct {
		int _vocabId;
		VerbType _verbType;
		PrepType _prepType;
	} _vocabList[MAX_VOCAB];

	int _qualityId[MAX_QUALITIES];
	int _qualityValue[MAX_QUALITIES];

	/**
	 * Synchronizes the data for a given object
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Returns true if the given object has the specified quality
	 */
	bool hasQuality(int qualityId) const;

	/**
	 * Sets the quality value for a given quality Id
	 */
	void setQuality(int qualityId, int qualityValue);

	/**
	 * Gets the quality value for a given quality Id
	 */
	int getQuality(int qualityId) const;
};

class InventoryObjects : public Common::Array<InventoryObject> {
private:
	MADSEngine *_vm;

public:
	SynchronizedList _inventoryList;

	/**
	 * Constructor
	 */
	InventoryObjects(MADSEngine *vm) : _vm(vm) {}

	/**
	 * Loads the game's object list
	 */
	void load();

	/**
	* Synchronize the objects list in a savegame
	*/
	void synchronize(Common::Serializer &s);

	/**
	 * Returns the inventory item from the player's inventory
	 */
	InventoryObject &getItem(int itemIndex) {
		return (*this)[_inventoryList[itemIndex]];
	}

	/**
	 * Sets an item's scene number
	 */
	void setRoom(int objectId, int sceneNumber);

	/**
	 * Returns true if a given object is in the player's current scene
	 */
	bool isInRoom(int objectId) const;

	/**
	 * Returns true if a given object is in the player's inventory
	 */
	bool isInInventory(int objectId) const;

	/**
	* Removes the specified object from the player's inventory
	*/
	void addToInventory(int objectId);

	/**
	 * Removes the specified object to the player's inventory
	 * @param objectId	Object to remove
	 * @param newScene	Specifies the new scene to set the item to
	 */
	void removeFromInventory(int objectId, int newScene);

	int getIdFromDesc(int objectId);
};

} // End of namespace MADS

#endif /* MADS_INVENTORY_H */
