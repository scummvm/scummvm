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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ILLUSIONS_BBDOU_BBDOU_INVENTORY_H
#define ILLUSIONS_BBDOU_BBDOU_INVENTORY_H

#include "illusions/specialcode.h"
#include "common/array.h"
#include "common/rect.h"

namespace Illusions {

class IllusionsEngine_BBDOU;
class BbdouSpecialCode;
class Control;
struct TriggerFunction;

struct InventoryItem {
	uint32 _objectId;
	uint32 _sequenceId;
	bool _assigned;
	bool _flag;
	int _timesPresent;
	int _fieldE;
	InventoryItem(uint32 objectId, uint32 sequenceId);
};

struct InventorySlot {
	uint32 _namedPointId;
	uint32 _objectId;
	InventoryItem *_inventoryItem;
	InventorySlot(uint32 namedPointId);
};

class InventoryBag {
public:
	InventoryBag(IllusionsEngine_BBDOU *vm, uint32 sceneId);
	~InventoryBag();
	void registerInventorySlot(uint32 namedPointId);
	bool addInventoryItem(InventoryItem *inventoryItem, InventorySlot *inventorySlot);
	void removeInventoryItem(InventoryItem *inventoryItem);
	bool hasInventoryItem(uint32 objectId);
	void buildItems();
	void clear();
	InventorySlot *getInventorySlot(uint32 objectId);
	InventorySlot *findClosestSlot(Common::Point putPos, int index);
protected:
public:
	typedef Common::Array<InventorySlot*> InventorySlots;
	typedef InventorySlots::iterator InventorySlotsIterator;
	IllusionsEngine_BBDOU *_vm;
	uint32 _sceneId;
	InventorySlots _inventorySlots;
	bool _isActive;
	int _fieldA;
};

class BbdouInventory {
public:
	BbdouInventory(IllusionsEngine_BBDOU *vm, BbdouSpecialCode *bbdou);
	~BbdouInventory();
	void registerInventoryBag(uint32 sceneId);
	void registerInventoryItem(uint32 objectId, uint32 sequenceId);
	void registerInventorySlot(uint32 namedPointId);
	void addInventoryItem(uint32 objectId);
	void removeInventoryItem(uint32 objectId);
	bool hasInventoryItem(uint32 objectId);
	void open();
	void close();
	InventoryBag *getInventoryBag(uint32 sceneId);
	InventoryItem *getInventoryItem(uint32 objectId);
	void refresh();
	void buildItems(InventoryBag *inventoryBag);
	void clear();
	void cause0x1B0001(TriggerFunction *triggerFunction, uint32 callingThreadId);
	void cause0x1B0002(TriggerFunction *triggerFunction, uint32 callingThreadId);
	void putBackInventoryItem(uint32 objectId, Common::Point cursorPosition);
protected:
	typedef Common::Array<InventoryItem*> InventoryItems;
	typedef InventoryItems::iterator InventoryItemsIterator;
	IllusionsEngine_BBDOU *_vm;
	BbdouSpecialCode *_bbdou;
	Common::Array<InventoryBag*> _inventoryBags;
	InventoryItems _inventoryItems;
	uint32 _activeBagSceneId;
	uint32 _activeInventorySceneId;
	int _index;
	//field_12 dw
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_INVENTORY_H
