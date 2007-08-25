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

#ifndef PARALLACTION_INVENTORY_H
#define PARALLACTION_INVENTORY_H



namespace Parallaction {


struct InventoryItem {
	uint32		_id;            // object name (lowest 16 bits are always zero)
	uint16		_index;			// index to frame in objs file
};

#define INVENTORYITEM_PITCH 		32
#define INVENTORYITEM_WIDTH 		24
#define INVENTORYITEM_HEIGHT		24

#define MAKE_INVENTORY_ID(x) (((x) & 0xFFFF) << 16)


extern InventoryItem _inventory[];

void initInventory();
void destroyInventory();
void openInventory();
void closeInventory();
void cleanInventory();
void addInventoryItem(uint16 item);

int16 getInventoryItemIndex(int16 pos);
void highlightInventoryItem(int16 pos, byte color);


} // namespace Parallaction

#endif
