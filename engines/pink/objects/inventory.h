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

#ifndef PINK_INVENTORY_H
#define PINK_INVENTORY_H

#include "common/rect.h"

#include "pink/utils.h"

namespace Pink {

class InventoryItem : public NamedObject {
public:
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	const Common::String &getCurrentOwner() const { return _currentOwner; }

	friend class InventoryMgr;
private:
	Common::String _initialOwner;
	Common::String _currentOwner;
};

class LeadActor;
class Actor;

class InventoryMgr : public Object {
public:
	InventoryMgr();
	~InventoryMgr() override;
	void deserialize(Archive &archive) override;
	void toConsole() const override;

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	void update();
	void onClick(Common::Point point);

	bool start(bool paused);

	void setLeadActor(LeadActor *lead) { _lead = lead; }
	InventoryItem *findInventoryItem(const Common::String &name);

	bool isPinkOwnsAnyItems();
	void setItemOwner(const Common::String &owner, InventoryItem *item);

	InventoryItem *getCurrentItem() { return _currentItem; }

	friend class Console;

private:
	void close();
	enum Direction {
		kLeft = 0,
		kRight = 1
	};
	void showNextItem(bool direction);

	LeadActor *_lead;
	Actor *_window;
	Actor *_itemActor;
	Actor *_rightArrow;
	Actor *_leftArrow;

	InventoryItem *_currentItem;
	Array<InventoryItem *> _items;

	enum State {
		kIdle = 0,
		kOpening = 1,
		kReady = 2,
		kClosing = 3
	} _state;
	bool _isClickedOnItem;
};

} // End of namespace Pink

#endif
