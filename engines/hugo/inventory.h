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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_INVENTORY_H
#define HUGO_INVENTORY_H
namespace Hugo {

/**
* Actions for Process_inventory()
*/
enum invact_t {kInventoryActionInit, kInventoryActionLeft, kInventoryActionRight, kInventoryActionGet};

class InventoryHandler {
public:
	InventoryHandler(HugoEngine *vm);

	int16 processInventory(invact_t action, ...);
	void runInventory();

private:
	HugoEngine *_vm;

	static const int kStepDy = 8;                   // Pixels per step movement

	void constructInventory(int16 imageTotNumb, int displayNumb, bool scrollFl, int16 firstObjId);
};

} // End of namespace Hugo

#endif // HUGO_INVENTORY_H
