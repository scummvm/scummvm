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
#define INVENTORY_YSIZE 70			// Height of the box that surrounds inventory items
#define NUM_INVENTORY_SHOWN 8		// Number of Inventory Items Shown
#define BUTTON_SIZE 15				// Button width/height

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
		_bounds = Common::Rect((INVENTORY_XSIZE + 3) * NUM_INVENTORY_SHOWN / 2 + BUTTON_SIZE + 6,
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
	drawInventory();
}

void WidgetInventory::drawInventory() {
	Inventory &inv = *_vm->_inventory;

	// TODO: Refactor _invIndexinto this widget class
	for (int idx= 0, itemId = inv._invIndex; idx < NUM_INVENTORY_SHOWN; ++idx) {
		// Figure out the drawing position
		Common::Point pt(3 + (INVENTORY_XSIZE + 3) * (idx % (NUM_INVENTORY_SHOWN / 2)),
			3 + (INVENTORY_YSIZE + 3) * idx / (NUM_INVENTORY_SHOWN / 2));

		// Draw the box to serve as the background for the item
		_surface.hLine(pt.x + 1, pt.y, pt.x + INVENTORY_XSIZE - 2, TRANSPARENCY);
		_surface.fillRect(Common::Rect(pt.x, pt.y + 1, pt.x + INVENTORY_XSIZE, pt.y + INVENTORY_YSIZE - 1), TRANSPARENCY);
		_surface.hLine(pt.x + 1, pt.y + INVENTORY_YSIZE - 1, pt.x + INVENTORY_XSIZE - 2, TRANSPARENCY);

		// Draw the item
		if (itemId < inv._holdings) {
			ImageFrame &img = (*inv._invShapes[idx])[0];
			_surface.transBlitFrom(img, Common::Point(pt.x + (INVENTORY_XSIZE - img._width) / 2,
				pt.y + (INVENTORY_YSIZE - img._height) / 2));
		}
	}

	drawScrollBar();
}

void WidgetInventory::drawScrollBar() {
	Inventory &inv = *_vm->_inventory;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	bool raised;

	// Fill the area with transparency
	Common::Rect r(BUTTON_SIZE, _bounds.height() - 6);
	r.moveTo(_bounds.width() - BUTTON_SIZE - 3, 3);
	_surface.fillRect(r, TRANSPARENCY);

	raised = ui._scrollHighlight != 1;
	_surface.fillRect(Common::Rect(r.left + 2, r.top + 2, r.right - 2, r.top + BUTTON_SIZE - 2), INFO_MIDDLE);
	drawDialogRect(Common::Rect(r.left, r.top, r.left + BUTTON_SIZE, r.top + BUTTON_SIZE), raised);

	raised = ui._scrollHighlight != 5;
	_surface.fillRect(Common::Rect(r.left + 2, r.bottom - BUTTON_SIZE + 2, r.right - 2, r.bottom - 2), INFO_MIDDLE);
	drawDialogRect(Common::Rect(r.left, r.bottom - BUTTON_SIZE, r.right, r.bottom), raised);

	// Draw the arrows on the scroll buttons
	byte color = inv._invIndex? INFO_BOTTOM + 2 : INFO_BOTTOM;
	_surface.hLine(r.right / 2, r.top - 2 + BUTTON_SIZE / 2, r.right / 2, color);
	_surface.fillRect(Common::Rect(r.right / 2 - 1, r.top - 1 + BUTTON_SIZE / 2,
		r.right / 2 + 1, r.top - 1 + BUTTON_SIZE / 2), color);
	_surface.fillRect(Common::Rect(r.right / 2 - 2, r.top + BUTTON_SIZE / 2,
		r.right / 2 + 2, r.top + BUTTON_SIZE / 2), color);
	_surface.fillRect(Common::Rect(r.right / 2 - 3, r.top + 1 + BUTTON_SIZE / 2,
		r.right / 2 + 3, r.top + 1 + BUTTON_SIZE / 2), color);

	color = (inv._invIndex + NUM_INVENTORY_SHOWN) < inv._holdings ? INFO_BOTTOM + 2 : INFO_BOTTOM;
	_surface.fillRect(Common::Rect(r.right / 2 - 3, r.bottom - 1 - BUTTON_SIZE + BUTTON_SIZE / 2,
		r.right / 2 + 3, r.bottom - 1 - BUTTON_SIZE + BUTTON_SIZE / 2), color);
	_surface.fillRect(Common::Rect(r.right / 2 - 2, r.bottom - 1 - BUTTON_SIZE + 1 + BUTTON_SIZE / 2,
		r.right / 2 + 2, r.bottom - 1 - BUTTON_SIZE + 1 + BUTTON_SIZE / 2), color);
	_surface.fillRect(Common::Rect(r.right / 2 - 1, r.bottom - 1 - BUTTON_SIZE + 2 + BUTTON_SIZE / 2,
		r.right / 2 + 1, r.bottom - 1 - BUTTON_SIZE + 2 + BUTTON_SIZE / 2), color);
	_surface.fillRect(Common::Rect(r.right / 2, r.bottom - 1 - BUTTON_SIZE + 3 + BUTTON_SIZE / 2,
		r.right / 2, r.bottom - 1 - BUTTON_SIZE + 3 + BUTTON_SIZE / 2), color);

	// Draw the scroll position bar
	int idx= inv._holdings;
	if (idx% (NUM_INVENTORY_SHOWN / 2))
		idx= (idx + (NUM_INVENTORY_SHOWN / 2)) / (NUM_INVENTORY_SHOWN / 2)*(NUM_INVENTORY_SHOWN / 2);
	int barHeight = NUM_INVENTORY_SHOWN * (_bounds.height() - BUTTON_SIZE * 2) / idx;
	barHeight = CLIP(barHeight, BUTTON_SIZE, _bounds.height() - BUTTON_SIZE * 2);

	int barY = (idx<= NUM_INVENTORY_SHOWN) ? r.top + BUTTON_SIZE :
		(r.height() - BUTTON_SIZE * 2 - barHeight) * FIXED_INT_MULTIPLIER / (idx- NUM_INVENTORY_SHOWN)
			* inv._invIndex / FIXED_INT_MULTIPLIER + r.top + BUTTON_SIZE;
	_surface.fillRect(Common::Rect(r.left + 2, barY + 2, r.right - 2, barY + barHeight - 3), INFO_MIDDLE);
	drawDialogRect(Common::Rect(r.left, barY, r.right, barY + barHeight), true);
}

void WidgetInventory::drawDialogRect(const Common::Rect &r, bool raised) {
	switch (raised) {
	case true:
		// Draw Left
		_surface.vLine(r.left, r.top, r.bottom - 1, INFO_TOP);
		_surface.vLine(r.left + 1, r.top, r.bottom - 2, INFO_TOP);
		// Draw Top
		_surface.hLine(r.left + 2, r.top, r.right - 1, INFO_TOP);
		_surface.hLine(r.left + 2, r.top + 1, r.right - 2, INFO_TOP);
		// Draw Right
		_surface.vLine(r.right - 1, r.top + 1,r.bottom - 1, INFO_BOTTOM);
		_surface.vLine(r.right - 2, r.top + 2, r.bottom - 1, INFO_BOTTOM);
		// Draw Bottom
		_surface.hLine(r.left + 1, r.bottom - 1, r.right - 3, INFO_BOTTOM);
		_surface.hLine(r.left + 2, r.bottom - 2, r.right - 3, INFO_BOTTOM);
		break;

	case false:
		// Draw Left
		_surface.vLine(r.left, r.top, r.bottom - 1, INFO_BOTTOM);
		_surface.vLine(r.left + 1, r.top, r.bottom - 2, INFO_BOTTOM);
		// Draw Top
		_surface.hLine(r.left + 2, r.top, r.right - 1, INFO_BOTTOM);
		_surface.hLine(r.left + 2, r.top + 1, r.right - 2, INFO_BOTTOM);
		// Draw Right
		_surface.vLine(r.right - 1, r.top + 1, r.bottom - 1, INFO_TOP);
		_surface.vLine(r.right - 2, r.top + 2, r.bottom - 1, INFO_TOP);
		// Draw Bottom
		_surface.hLine(r.left + 1, r.bottom - 1, r.right - 3, INFO_TOP);
		_surface.hLine(r.left + 2, r.bottom - 2, r.right - 3, INFO_TOP);
		break;
	}
}


} // End of namespace Tattoo

} // End of namespace Sherlock
