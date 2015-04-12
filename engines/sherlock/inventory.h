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

#ifndef SHERLOCK_INVENTORY_H
#define SHERLOCK_INVENTORY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str-array.h"
#include "sherlock/objects.h"
#include "sherlock/resources.h"

namespace Sherlock {

#define MAX_VISIBLE_INVENTORY 6

enum InvMode {
	INVMODE_EXIT = 0,
	INVMODE_LOOK = 1,
	INVMODE_USE = 2,
	INVMODE_GIVE = 3,
	INVMODE_FIRST = 4,
	INVMODE_PREVIOUS = 5,
	INVMODE_NEXT = 6,
	INVMODE_LAST = 7,
	INVMODE_INVALID = 8,
	INVMODE_USE55 = 255
};

struct InventoryItem {
	int _requiredFlag;
	Common::String _name;
	Common::String _description;;
	Common::String _examine;
	int _lookFlag;

	InventoryItem() : _requiredFlag(0), _lookFlag(0) {}
	InventoryItem(int requiredFlag, const Common::String &name,
		const Common::String &description, const Common::String &examine);
};

class Inventory : public Common::Array<InventoryItem> {
private:
	SherlockEngine *_vm;

	void copyToInventory(Object &obj);
public:
	ImageFile *_invShapes[MAX_VISIBLE_INVENTORY];
	Common::StringArray _names;
	bool _invGraphicsLoaded;
	InvMode _invMode;
	int _invIndex;
	int _holdings;
	void freeGraphics();
	int _oldFlag;
	int _invFlag;
public:
	Inventory(SherlockEngine *vm);
	~Inventory();

	void freeInv();

	void loadInv();

	void loadGraphics();

	int findInv(const Common::String &name);

	void putInv(int slamIt);

	void drawInventory(int flag);

	void invCommands(bool slamIt);

	void highlight(int index, byte color);

	void doInvJF();

	int putNameInInventory(const Common::String &name);
	int putItemInInventory(Object &obj);

	int deleteItemFromInventory(const Common::String &name);
};

} // End of namespace Sherlock

#endif
