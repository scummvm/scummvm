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
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define INVENTORY_XSIZE 70			// Width of the box that surrounds inventory items
#define INVENTORY_YSIZE 70			// Height of the box that surrounds inventory items
#define MAX_INV_COMMANDS 10			// Maximum elements in dialog

WidgetInventoryTooltip::WidgetInventoryTooltip(SherlockEngine *vm, WidgetInventory *owner) : 
		WidgetTooltipBase(vm), _owner(owner) {
}

void WidgetInventoryTooltip::setText(const Common::String &str) {
	// If no text specified, erase any previously displayed tooltip and free it's surface
	if (str.empty()) {
		erase();
		_surface.free();
		return;
	}

	int width = _surface.stringWidth(str) + 2;
	int height = 0;
	Common::String line1 = str, line2;

	// See if we need to split it into two lines
	if (width > 150) {
		// Yes, we do
		const char *s = str.c_str();
		const char *space = nullptr;
		int dif = 10000;

		while (*s) {
			s = strchr(s, ' ');

			if (!s) {
				if (!space) {
					height = _surface.stringHeight(str) + 2;
				} else {
					line1 = Common::String(str.c_str(), space);
					line2 = Common::String(space + 1);
					height = _surface.stringHeight(line1) + _surface.stringHeight(line2) + 4;
				}
				break;
			} else {
				line1 = Common::String(str.c_str(), s);
				line2 = Common::String(s + 1);
				int width1 = _surface.stringWidth(line1);
				int width2 = _surface.stringWidth(line2);

				if (ABS(width1 - width2) < dif) {
					// Found a split point that results in less overall width
					space = s;
					dif = ABS(width1 - width2);
					width = MAX(width1, width2);
				}

				s++;
			}
		}
	} else {
		height = _surface.stringHeight(str) + 2;
	}

	// Allocate a fresh surface for the new string
	_bounds = Common::Rect(width, height);
	_surface.create(width, height);
	_surface.fill(TRANSPARENCY);

	if (line2.empty()) {
		_surface.writeFancyString(str, Common::Point(0, 0), BLACK, INFO_TOP);
	} else {
		int xp, yp;
		
		xp = (_bounds.width() - _surface.stringWidth(line1) - 2) / 2;
		_surface.writeFancyString(line1, Common::Point(xp, 0), BLACK, INFO_TOP);

		xp = (_bounds.width() - _surface.stringWidth(line2) - 2) / 2;
		yp = _surface.stringHeight(line2) + 2;
		_surface.writeFancyString(line2, Common::Point(xp, yp), BLACK, INFO_TOP);
	}
}

void WidgetInventoryTooltip::handleEvents() {	
	Events &events = *_vm->_events;
	FixedText &fixedText = *_vm->_fixedText;
	Inventory &inv = *_vm->_inventory;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Scene &scene = *_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
	Common::String str;
	int select = -1, oldSelect = 999;
	Common::String strWith = fixedText.getText(kFixedText_With);
	Common::String strUse = fixedText.getText(kFixedText_Use);

	// If there's a floating graphic for a selected inventory item, update it's bounds
	if (_owner->_invVerbMode == 2 || _owner->_invVerbMode == 3) {
		_oldInvGraphicBounds = _invGraphicBounds;

		// Set the New position of the graphic
		int xp = CLIP(mousePos.x - _invGraphicBounds.width() / 2, 0, SHERLOCK_SCREEN_WIDTH - _invGraphicBounds.width());
		int yp = CLIP(mousePos.y - _invGraphicBounds.height() / 2, 0, SHERLOCK_SCREEN_HEIGHT - _invGraphicBounds.height());

		_invGraphicBounds.moveTo(xp, yp);
	}

	// If we are using an inventory item on an object in the room, display the appropriate text above the mouse cursor
	if (_owner->_invVerbMode == 3) {
		select = ui._bgFound;
		oldSelect = ui._oldBgFound;
	
		if (select != -1 && (select != oldSelect || (select != -1 && _surface.empty()))) {
			// See if we're pointing at a shape or a sprite
			if (select < 1000) {
				Object &obj = scene._bgShapes[select];

				if (!obj._description.empty() && !obj._description.hasPrefix(" ")) {
					if (_vm->getLanguage() == Common::GR_GRE) {

						if (!_owner->_swapItems)
							str = Common::String::format("%s %s %s %s", ui._action.c_str(), obj._description.c_str(), 
								inv[_owner->_invSelect]._name.c_str(), _owner->_invVerb.c_str());
						else
							str = Common::String::format("%s %s %s %s", ui._action.c_str(), inv[_owner->_invSelect]._name.c_str(), 
								obj._description.c_str(), _owner->_invVerb.c_str());
					} else {
						if (_owner->_swapItems)
							str = Common::String::format("%s %s %s %s", _owner->_invVerb.c_str(), obj._description.c_str(), ui._action.c_str(), 
								inv[_owner->_invSelect]._name.c_str());
						else
							str = Common::String::format("%s %s %s %s", _owner->_invVerb.c_str(), inv[_owner->_invSelect]._name.c_str(), 
								ui._action.c_str(), obj._description.c_str());
					}
				}
			} else {
				Person &person = people[ui._bgFound - 1000];

				if (!person._description.empty() && !person._description.hasPrefix(" ")) {
					if (_vm->getLanguage() == Common::GR_GRE) {
						if (!_owner->_swapItems)
							str = Common::String::format("%s %s %s %s", ui._action.c_str(), person._description.c_str(),
								inv[_owner->_invSelect]._name.c_str(), _owner->_invVerb.c_str());
						else
							str = Common::String::format("%s %s %s %s", ui._action.c_str(), inv[_owner->_invSelect]._name.c_str(),
								person._description.c_str(), _owner->_invVerb.c_str());
					} else {

						if (_owner->_swapItems)
							str = Common::String::format("%s %s %s %s", _owner->_invVerb.c_str(), person._description.c_str(),
								ui._action.c_str(), inv[_owner->_invSelect]._name.c_str());
						else
							str = Common::String::format("%s %s %s %s", _owner->_invVerb.c_str(),
								inv[_owner->_invSelect]._name.c_str(), ui._action.c_str(), person._description.c_str());
					}
				}
			}
		}
	} else {
		Common::Rect r = _owner->_bounds;
		r.grow(-3);

		if (r.contains(mousePos)) {
			select = (mousePos.x - r.left) / (INVENTORY_XSIZE + 3) + NUM_INVENTORY_SHOWN / 2 *
				((mousePos.y - r.top) / (INVENTORY_YSIZE + 3)) + inv._invIndex;

			if (select >= inv._holdings) {
				select = -1;
			} else {
				oldSelect = _owner->_invSelect;
				
				if (select != _owner->_invSelect || _surface.empty()) {

					if (_owner->_invMode == 1) {
						// See if we were pointing at a shapre or sprite
						if (ui._activeObj < 1000) {
							Object &obj = scene._bgShapes[ui._activeObj];

							if (!obj._description.empty() && !obj._description.hasPrefix(" "))
								str = Common::String::format("%s %s %s %s", strUse.c_str(), inv[select]._name.c_str(),
									strWith.c_str(), obj._description.c_str());
						} else {
							Person &person = people[ui._activeObj - 1000];

							if (!person._description.empty() && !person._description.hasPrefix(" "))
								str = Common::String::format("%s %s %s %s", strUse.c_str(), inv[select]._name.c_str(),
									strWith.c_str(), person._description.c_str());
						}
					} else {
						if (_owner->_invVerbMode == 2)
							str = Common::String::format("%s %s %s %s", strUse.c_str(), inv[_owner->_invSelect]._name.c_str(),
								strWith.c_str(), inv[select]._name.c_str());
						else
							str = inv[select]._description.c_str();
					}
				}
			}
		}
	}

	// See if they are pointing at a different inventory object and we need to
	// change the graphics of the Text Tag
	if (select != oldSelect || (select != -1 && _surface.empty())) {
		// Set the text
		setText(str);

		if (_owner->_invVerbMode != 3)
			_owner->_invSelect = select;
		else
			ui._oldBgFound = select;
	} else if (select == -1 && oldSelect != -1) {
		setText(Common::String());
		return;
	}

	// Update the position of the tooltip
	int xs = CLIP(mousePos.x - _bounds.width() / 2, 0, SHERLOCK_SCREEN_WIDTH - _bounds.width());
	int ys = CLIP(mousePos.y - _bounds.height(), 0, SHERLOCK_SCREEN_HEIGHT - _bounds.height());
	_bounds.moveTo(xs, ys);
}

/*----------------------------------------------------------------*/


WidgetInventory::WidgetInventory(SherlockEngine *vm) : WidgetBase(vm), _tooltipWidget(vm, this) {
	_invMode = 0;
	_invVerbMode = 0;
	_invSelect = _oldInvSelect = -1;
	_selector = _oldSelector = -1;
	_invVerbSelect = _oldInvVerbSelect = -1;
	_dialogTimer = -1;
	_scrollHighlight = 0;
	_swapItems = false;
}

void WidgetInventory::load(int mode) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Common::Point mousePos = events.mousePos();

	if (mode == 3) {
		mode = 2;
		mousePos = Common::Point(SHERLOCK_SCREEN_WIDTH / 2, SHERLOCK_SCREEN_HEIGHT / 2);
	}

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
	makeInfoArea(_surface);
	drawBars();
	drawInventory();
}

void WidgetInventory::drawBars() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;
	int x;

	_surface.hLine(3, INVENTORY_YSIZE + 3, _bounds.width() - 4, INFO_TOP);
	_surface.hLine(3, INVENTORY_YSIZE + 4, _bounds.width() - 4, INFO_MIDDLE);
	_surface.hLine(3, INVENTORY_YSIZE + 5, _bounds.width() - 4, INFO_BOTTOM);
	_surface.transBlitFrom(images[4], Common::Point(0, INVENTORY_YSIZE + 2));

	for (int idx = 1; idx <= NUM_INVENTORY_SHOWN / 2; ++idx) {
		x = idx * (INVENTORY_XSIZE + 3);

		_surface.vLine(x, 3, _bounds.height() - 4, INFO_TOP);
		_surface.vLine(x + 1, 3, _bounds.height() - 4, INFO_MIDDLE);
		_surface.vLine(x + 2, 3, _bounds.height() - 4, INFO_BOTTOM);

		_surface.transBlitFrom(images[6], Common::Point(x - 1, 1));
		_surface.transBlitFrom(images[7], Common::Point(x - 1, _bounds.height() - 4));
		_surface.transBlitFrom(images[6], Common::Point(x - 1, INVENTORY_YSIZE + 5));
		_surface.transBlitFrom(images[7], Common::Point(x - 1, INVENTORY_YSIZE + 2));
	}

	_surface.hLine(x + 2, INVENTORY_YSIZE + 2, INVENTORY_YSIZE + 8, INFO_BOTTOM);
}

void WidgetInventory::drawInventory() {
	Inventory &inv = *_vm->_inventory;

	// TODO: Refactor _invIndex into this widget class
	for (int idx = 0, itemId = inv._invIndex; idx < NUM_INVENTORY_SHOWN; ++idx, ++itemId) {
		// Figure out the drawing position
		Common::Point pt(3 + (INVENTORY_XSIZE + 3) * (idx % (NUM_INVENTORY_SHOWN / 2)),
			3 + (INVENTORY_YSIZE + 3) * (idx / (NUM_INVENTORY_SHOWN / 2)));

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
	ui.drawDialogRect(_surface, Common::Rect(r.left, r.top, r.left + BUTTON_SIZE, r.top + BUTTON_SIZE), raised);

	raised = ui._scrollHighlight != 5;
	_surface.fillRect(Common::Rect(r.left + 2, r.bottom - BUTTON_SIZE + 2, r.right - 2, r.bottom - 2), INFO_MIDDLE);
	ui.drawDialogRect(_surface, Common::Rect(r.left, r.bottom - BUTTON_SIZE, r.right, r.bottom), raised);

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
	int idx = inv._holdings;
	if (idx % (NUM_INVENTORY_SHOWN / 2))
		idx = (idx + (NUM_INVENTORY_SHOWN / 2)) / (NUM_INVENTORY_SHOWN / 2) * (NUM_INVENTORY_SHOWN / 2);
	int barHeight = NUM_INVENTORY_SHOWN * (_bounds.height() - BUTTON_SIZE * 2) / idx;
	barHeight = CLIP(barHeight, BUTTON_SIZE, _bounds.height() - BUTTON_SIZE * 2);

	int barY = (idx<= NUM_INVENTORY_SHOWN) ? r.top + BUTTON_SIZE :
		(r.height() - BUTTON_SIZE * 2 - barHeight) * FIXED_INT_MULTIPLIER / (idx- NUM_INVENTORY_SHOWN)
			* inv._invIndex / FIXED_INT_MULTIPLIER + r.top + BUTTON_SIZE;
	_surface.fillRect(Common::Rect(r.left + 2, barY + 2, r.right - 2, barY + barHeight - 3), INFO_MIDDLE);
	ui.drawDialogRect(_surface, Common::Rect(r.left, barY, r.right, barY + barHeight), true);
}

void WidgetInventory::handleEvents() {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	People &people = *_vm->_people;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	if (_invVerbMode == 1)
		checkTabbingKeys(MAX_INV_COMMANDS);
	else if (_invVerbMode == 0)
		checkInvTabbingKeys();

	if (_invVerbMode != 1)
		_tooltipWidget.handleEvents();

	// Flag is they started pressing outside of the menu
	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	if (_invVerbMode != 3)
		highlightControls();

	// See if they released a mouse button button
	if (events._released || events._rightReleased || ui._keyState.keycode == Common::KEYCODE_ESCAPE) {
		_dialogTimer = -1;
		_scrollHighlight = 0;

		// See if they have a Verb List open for an Inventry Item
		if (_invVerbMode == 1) {
			// An inventory item's Verb List is open

			// See if they want to close the menu (by clicking outside the menu)
			Common::Rect innerBounds = _bounds;
			innerBounds.grow(-3);
			
			if (_outsideMenu && !innerBounds.contains(mousePos)) {
				banishWindow();
				_invVerbMode = 0;
			} else if (innerBounds.contains(mousePos)) {
				_outsideMenu = false;

				// Check if they are trying to solve the Foolscap puzzle, or looking at the completed puzzle
				bool doHangman = !inv[_invSelect]._name.compareToIgnoreCase(FIXED(Inv6)) &&
					!_inventCommands[_invVerbSelect].compareToIgnoreCase(FIXED(Solve));
				doHangman |= (!inv[_invSelect]._name.compareToIgnoreCase(FIXED(Inv6)) || !inv[_invSelect]._name.compareToIgnoreCase(FIXED(Inv7)))
					&& _inventCommands[_invVerbSelect].compareToIgnoreCase(FIXED(Look)) && vm.readFlags(299);

				if (doHangman) {
					// Close the entire Inventory and return to Standard Mode
					banishWindow();
					_invVerbMode = 0;

					_tooltipWidget.banishWindow();
					banishWindow();
					inv.freeInv();

					events.clearEvents();
					events.setCursor(ARROW);
					ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;

					scene.doBgAnim();
					vm.doHangManPuzzle();
				} else if (_invVerbSelect == 0) {
					// They have released the mouse on the Look Verb command, so Look at the inventory item
					ui._invLookFlag = true;
					inv.freeInv();
					ui._windowOpen = false;
					ui._lookPos = mousePos;
					ui.printObjectDesc(inv[_invSelect]._examine, true);
				} else {
					// Clear the window
					banishWindow();
					_invVerbMode = 3;
					ui._oldBgFound = -1;

					// See if the selected Verb with the selected Iventory Item, is to be used by itself
					if (!_inventCommands[_invVerbSelect].compareToIgnoreCase(inv[_invSelect]._verb._verb) ||
						!inv[_invSelect]._verb._target.compareToIgnoreCase("*SELF")) {
						inv.freeInv();
						
						ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
						events.clearEvents();
						ui.checkAction(inv[_invSelect]._verb, 2000);
					} else {
						_invVerb = _inventCommands[_invVerbSelect];
					}
				}

				// If we are still in Inventory Mode, setup the graphic to float in front of the mouse cursor
				if (ui._menuMode == INV_MODE) {
					ImageFrame &imgFrame = (*inv._invShapes[_invSelect - inv._invIndex])[0];
					_invGraphicBounds = Common::Rect(imgFrame._width, imgFrame._height);
					_invGraphicBounds.moveTo(mousePos.x - _invGraphicBounds.width() / 2,
						mousePos.y - _invGraphicBounds.height() / 2);

					// Constrain it to the screen
					if (_invGraphicBounds.left < 0)
						_invGraphicBounds.moveTo(0, _invGraphicBounds.top);
					if (_invGraphicBounds.top < 0)
						_invGraphicBounds.moveTo(_invGraphicBounds.left, 0);
					if (_invGraphicBounds.right > SHERLOCK_SCREEN_WIDTH)
						_invGraphicBounds.moveTo(SHERLOCK_SCREEN_WIDTH - _invGraphicBounds.width(), _invGraphicBounds.top);
					if (_invGraphicBounds.bottom > SHERLOCK_SCREEN_HEIGHT)
						_invGraphicBounds.moveTo(_invGraphicBounds.left, SHERLOCK_SCREEN_HEIGHT - _invGraphicBounds.height());

					// Make a copy of the inventory image
					_invGraphic.create(imgFrame._width, imgFrame._height);
					_invGraphic.blitFrom(imgFrame, Common::Point(0, 0));
				}
			}
		} else if (_invVerbMode == 3) {
			// Selecting object after inventory verb has been selected
			_tooltipWidget.banishWindow();
			_invGraphic.free();
			inv.freeInv();

			ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
			events.clearEvents();

			if (ui._keyState.keycode != Common::KEYCODE_ESCAPE) {
				// If user pointed at an item, use the selected inventory item with this item
				bool found = false;
				if (ui._bgFound != -1) {
					if (ui._personFound) {
						for (int idx = 0; idx < 2; ++idx) {
							if (!people[ui._bgFound - 1000]._use[idx]._verb.compareToIgnoreCase(_invVerb) &&
								!people[ui._bgFound - 1000]._use[idx]._target.compareToIgnoreCase(_invTarget)) {
								ui.checkAction(people[ui._bgFound - 1000]._use[idx], ui._bgFound);
								found = true;
							}
						}
					} else {
						for (int idx = 0; idx < 6; ++idx) {
							if (!ui._bgShape->_use[idx]._verb.compareToIgnoreCase(_invVerb) &&
									!ui._bgShape->_use[idx]._target.compareToIgnoreCase(_invTarget)) {
								ui.checkAction(ui._bgShape->_use[idx], ui._bgFound);
								found = true;
							}
						}
					}
				}

				if (!found)
					ui.putMessage("%s", FIXED(NoEffect).c_str());
			}
		} else if ((_outsideMenu && !_bounds.contains(mousePos)) || ui._keyState.keycode == Common::KEYCODE_ESCAPE) {
			// Want to close the window (clicked outside of it). So close the window and return to Standard 
			banishWindow();
			inv.freeInv();

			events.clearEvents();
			events.setCursor(ARROW);
			banishWindow();
			ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
		} else if (_bounds.contains(mousePos)) {
			// Mouse button was released inside the inventory window
			_outsideMenu = false;
		
			//  See if they are pointing at one of the inventory items
			if (_invSelect != -1) {
				// See if they are in Use Obj with Inv. Mode (they right clicked on an item 
				// in the room and selected "Use with Inv.")
				if (_invMode == 1) {
					_tooltipWidget.banishWindow();
					banishWindow();

					// See if the item in the room that they started with was a person
					bool found = false;
					if (ui._activeObj >= 1000) {
						// Object was a person, activate anything in his two verb fields
						for (int idx = 0; idx < 2; ++idx) {
							if (!people[ui._activeObj - 1000]._use[idx]._target.compareToIgnoreCase(inv[_invSelect]._name)) {
								ui.checkAction(people[ui._activeObj - 1000]._use[idx], ui._activeObj);
								found = true;
							}
						}
					} else {
						// Object was a regular object, activate anything in its verb fields
						for (int idx = 0; idx < 6; ++idx) {
							if (!scene._bgShapes[ui._activeObj]._use[idx]._target.compareToIgnoreCase(inv[_invSelect]._name)) {
								ui.checkAction(scene._bgShapes[ui._activeObj]._use[idx], ui._activeObj);
								found = true;
							}
						}
					}
					if (!found)
						ui.putMessage("%s",FIXED(NoEffect).c_str());

				} else {
					// See if they right clicked on an item
					if (events._rightReleased) {
						_invVerbMode = 1;
						_oldInvVerbSelect = -1;
						_tooltipWidget.banishWindow();

						// Keep track of the name of the inventory object so we can check it against the target fields 
						// of verbs when we activate it
						_invTarget = inv[_invSelect]._name;

						// Make the Verb List for this Inventory Item
						_inventCommands.clear();
						_inventCommands.push_back(FIXED(Look));

						// Default the Action word to "with"
						_action = _vm->getLanguage() == Common::GR_GRE ? "" : FIXED(With);
						_swapItems = false;

						// Search all the bgshapes for any matching Target Fields
						for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
							Object &obj = scene._bgShapes[idx];

							if (obj._type != INVALID && obj._type != HIDDEN) {
								for (int useNum = 0; useNum < 6; ++useNum) {
									if (obj._use[useNum]._verb.hasPrefix("*") && 
											!obj._use[useNum]._target.compareToIgnoreCase(inv[_invSelect]._name)) {
										// Make sure the Verb is not already in the list
										bool found1 = false;
										for (uint cmdNum = 0; cmdNum < _inventCommands.size() && !found1; ++cmdNum) {
											if (!_inventCommands[cmdNum].compareToIgnoreCase(obj._use[useNum]._verb))
												found1 = true;
										}

										if (!found1) {
											_inventCommands.push_back(obj._use[useNum]._verb);

											// Check for any Special Action commands
											for (int nameNum = 0; nameNum < 4; ++nameNum) {
												if (!scumm_strnicmp(obj._use[useNum]._names[nameNum].c_str(), "*V", 2)) {
													if (!scumm_strnicmp(obj._use[useNum]._names[nameNum].c_str(), "*VSWAP", 6))
														_swapItems = true;
													else
														_action = Common::String(obj._use[useNum]._names[nameNum].c_str() + 2);
												}
											}
										}
									}
								}
							}
						}

						// Search the NPCs for matches as well
						for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
							for (int useNum = 0; useNum < 2; ++useNum) {
								if (!people[idx]._use[useNum]._target.compareToIgnoreCase(inv[_invSelect]._name) &&
										!people[idx]._use[useNum]._verb.empty() && !people[idx]._use[useNum]._verb.hasPrefix(" ")) {
									bool found1 = false;
									for (uint cmdNum = 0; cmdNum < _inventCommands.size() && !found1; ++cmdNum) {
										if (!_inventCommands[cmdNum].compareToIgnoreCase(people[idx]._use[cmdNum]._verb))
											found1 = true;
									}

									if (!found1)
										_inventCommands.push_back(people[idx]._use[useNum]._verb);
								}
							}
						}

						// Finally see if the item itself has a verb
						if (!inv[_invSelect]._verb._verb.empty()) {
							// Don't add "Solve" to the Foolscap if it's already been "Solved"
							if (inv[_invSelect]._verb._verb.compareToIgnoreCase(FIXED(Solve)) || !vm.readFlags(299))
								_inventCommands.push_back(inv[_invSelect]._verb._verb);
						}

						// Now find the widest command in the _inventCommands array
						int width = 0;
						for (uint idx = 0; idx < _inventCommands.size(); ++idx)
							width = MAX(width, _surface.stringWidth(_inventCommands[idx]));

						// Set up bounds for the menu
						_menuBounds = Common::Rect(width + _surface.widestChar() * 2 + 6,
							(_surface.fontHeight() + 7) * _inventCommands.size() + 3);
						_menuBounds.moveTo(mousePos.x + _menuBounds.width() / 2, mousePos.y + _menuBounds.height() / 2);

						// Create the surface
						_menuSurface.create(_menuBounds.width(), _menuBounds.height());
						_surface.fill(TRANSPARENCY);
						makeInfoArea(_menuSurface);

						// Draw the Verb commands and the lines separating them
						ImageFile &images = *ui._interfaceImages;
						for (int idx = 0; idx < (int)_inventCommands.size(); ++idx) {
							_menuSurface.writeString(_inventCommands[idx], Common::Point((_menuBounds.width() -
								_menuSurface.stringWidth(_inventCommands[idx])) / 2, (_menuSurface.fontHeight() + 7) * idx + 5), INFO_TOP);

							if (idx < (int)_inventCommands.size()- 1) {
								_menuSurface.vLine(3, (_menuSurface.fontHeight() + 7) * (idx + 1), _menuBounds.right - 4, INFO_TOP);
								_menuSurface.vLine(3, (_menuSurface.fontHeight() + 7) * (idx + 1) + 1, _menuBounds.right - 4, INFO_MIDDLE);
								_menuSurface.vLine(3, (_menuSurface.fontHeight() + 7) * (idx + 1) + 2, _menuBounds.right - 4, INFO_BOTTOM);
								
								_menuSurface.transBlitFrom(images[4], Common::Point(0, (_menuSurface.fontHeight() + 7) * (idx + 1)));
								_menuSurface.transBlitFrom(images[5], Common::Point(_menuBounds.width() - images[5]._width,
									(_menuSurface.fontHeight() + 7) * (idx + 1) - 1));
							}
						}
					} else {		
						// They left clicked on an inventory item, so Look at it

						// Check if they are looking at the solved Foolscap
						if ((!inv[_invSelect]._name.compareToIgnoreCase(FIXED(Inv6)) || !inv[_invSelect]._name.compareToIgnoreCase(FIXED(Inv7)))
								&& vm.readFlags(299)) {
							banishWindow();
							_tooltipWidget.erase();

							_invVerbMode = 0;
							inv.freeInv();
							
							events.clearEvents();
							events.setCursor(ARROW);
							ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;

							scene.doBgAnim();
							vm.doHangManPuzzle();
						} else {
							ui._invLookFlag = true;
							inv.freeInv();

							_tooltipWidget.banishWindow();
							ui._windowOpen = false;
							ui._lookPos = mousePos;
							ui.printObjectDesc(inv[_invSelect]._examine, true);
						}
					}
				}
			}
		}
	}
}

void WidgetInventory::checkInvTabbingKeys() {
}

void WidgetInventory::highlightControls() {
	// TODO
}

void WidgetInventory::banishWindow() {
	WidgetBase::banishWindow();

	_menuSurface.free();
	_menuBounds = _oldMenuBounds = Common::Rect(0, 0, 0, 0);
}

void WidgetInventory::draw() {
	WidgetBase::draw();
	_tooltipWidget.draw();
}

void WidgetInventory::erase() {
	WidgetBase::erase();
	_tooltipWidget.erase();
}

} // End of namespace Tattoo

} // End of namespace Sherlock
