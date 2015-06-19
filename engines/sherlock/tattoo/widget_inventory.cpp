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

#include "sherlock/tattoo/widget_inventory.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define INVENTORY_XSIZE 70			// Width of the box that surrounds inventory items
#define INVENTORY_YSIZE 70			// Width of the box that surrounds inventory items
#define NUM_INVENTORY_SHOWN 8		// Number of Inventory Items Shown
#define BUTTON_XSIZE 15				// Button width

WidgetInventory::WidgetInventory(SherlockEngine *vm) : WidgetBase(vm) {
	_invMode = 0;
	_invVerbMode = 0;
	_invSelect = _oldInvSelect = 0;
	_selector = _oldSelector = 0;
	_dialogTimer = -1;
}

void WidgetInventory::load(int mode) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Common::Point mousePos = events.mousePos();

	if (mode != 0)
		_invMode = mode;
	_invVerbMode = 0;
	_invSelect = _oldInvSelect = -1;
	_selector = _oldSelector = -1;
	_dialogTimer = -1;

	if (mode == 0) {
		banishWindow();
	} else {
		_bounds = Common::Rect((INVENTORY_XSIZE + 3) * NUM_INVENTORY_SHOWN / 2 + BUTTON_XSIZE + 6,
			(INVENTORY_YSIZE + 3) * 2 + 3);
		_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);
	}

	// Ensure menu will be on-screen
	checkMenuPosition();

	// Load the inventory data
	inv.loadInv();

	// Redraw the inventory menu on the widget surface
	_surface.create(_bounds.width(), _bounds.height());
	_surface.fill(TRANSPARENCY);

	// Draw the window background and then the inventory on top of it
	makeInfoArea();
	//putInv(0);
}

void WidgetInventory::loadInv() {
	// TODO
}


} // End of namespace Tattoo

} // End of namespace Sherlock
