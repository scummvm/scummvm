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

#ifndef SHERLOCK_SCALPEL_INVENTORY_H
#define SHERLOCK_SCALPEL_INVENTORY_H

#include "sherlock/inventory.h"

namespace Sherlock {

namespace Scalpel {

class ScalpelInventory : public Inventory {
public:
	ScalpelInventory(SherlockEngine *vm);
	~ScalpelInventory();

	/**
	 * Put the game into inventory mode and open the interface window.
	 */
	void drawInventory(InvNewMode flag);

	/**
	 * Prints the line of inventory commands at the top of an inventory window with
	 * the correct highlighting
	 */
	void invCommands(bool slamIt);

	/**
	 * Set the highlighting color of a given inventory item
	 */
	void highlight(int index, byte color);

	/**
	 * Support method for refreshing the display of the inventory
	 */
	void refreshInv();

	/**
	 * Display the character's inventory. The slamIt parameter specifies:
	 */
	void putInv(InvSlamMode slamIt);

	/**
	 * Load the list of names the inventory items correspond to, if not already loaded,
	 * and then calls loadGraphics to load the associated graphics
	 */
	virtual void loadInv();
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
