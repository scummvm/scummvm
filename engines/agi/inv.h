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

#ifndef AGI_INV_H
#define AGI_INV_H

namespace Agi {

struct InventoryEntry {
	uint16 objectNr;
	int16  row;
	int16  column;
	const char *name;
};
typedef Common::Array<InventoryEntry> InventoryArray;

class InventoryMgr {
private:
	GfxMgr *_gfx;
	TextMgr *_text;
	AgiEngine *_vm;
	SystemUI *_systemUI;

	InventoryArray _array;
	int16 _activeItemNr;

public:
	InventoryMgr(AgiEngine *agi, GfxMgr *gfx, TextMgr *text, SystemUI *systemUI);
	~InventoryMgr();

	void getPlayerInventory();
	void drawAll();
	void drawItem(int16 itemNr);
	void show();

	void keyPress(uint16 newKey);
	void changeActiveItem(int16 direction);
};

} // End of namespace Agi

#endif /* AGI_INV_H */
