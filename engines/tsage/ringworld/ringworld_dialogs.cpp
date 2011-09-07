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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/translation.h"

#include "gui/dialog.h"
#include "gui/widget.h"

#include "tsage/tsage.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"
#include "tsage/ringworld/ringworld_dialogs.h"
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------*/

#define BUTTON_WIDTH 28
#define BUTTON_HEIGHT 29

RightClickButton::RightClickButton(int buttonIndex, int xp, int yp) : GfxButton() {
	_buttonIndex = buttonIndex;
	this->_bounds.left = xp;
	this->_bounds.top = yp;
	this->_bounds.setWidth(BUTTON_WIDTH);
	this->_bounds.setHeight(BUTTON_HEIGHT);
	_savedButton = NULL;
}

void RightClickButton::highlight() {
	if (_savedButton) {
		// Button was previously highlighted, so de-highlight by restoring saved area
		_globals->gfxManager().copyFrom(*_savedButton, _bounds.left, _bounds.top);
		delete _savedButton;
		_savedButton = NULL;
	} else {
		// Highlight button by getting the needed highlighted image resource
		_savedButton = Surface_getArea(_globals->gfxManager().getSurface(), _bounds);

		uint size;
		byte *imgData = _resourceManager->getSubResource(7, 2, _buttonIndex, &size);

		GfxSurface btnSelected = surfaceFromRes(imgData);
		_globals->gfxManager().copyFrom(btnSelected, _bounds.left, _bounds.top);

		DEALLOCATE(imgData);
	}
}

/*--------------------------------------------------------------------------*/

/**
 * This dialog implements the right-click dialog
 */
RightClickDialog::RightClickDialog() : GfxDialog(),
		_walkButton(1, 48, 12), _lookButton(2, 31, 29), _useButton(3, 65, 29),
		_talkButton(4, 14, 47), _inventoryButton(5, 48, 47), _optionsButton(6, 83, 47) {
	Rect rectArea, dialogRect;

	// Set the palette and change the cursor
	_gfxManager.setDialogPalette();
	_globals->_events.setCursor(CURSOR_ARROW);

	// Get the dialog image
	_surface = surfaceFromRes(7, 1, 1);

	// Set the dialog position
	dialogRect.resize(_surface, 0, 0, 100);
	dialogRect.center(_globals->_events._mousePos.x, _globals->_events._mousePos.y);

	// Ensure the dialog will be entirely on-screen
	Rect screenRect = _globals->gfxManager()._bounds;
	screenRect.collapse(4, 4);
	dialogRect.contain(screenRect);

	_bounds = dialogRect;
	_gfxManager._bounds = _bounds;

	_highlightedButton = NULL;
	_selectedAction = -1;
}

RightClickDialog::~RightClickDialog() {
}

RightClickButton *RightClickDialog::findButton(const Common::Point &pt) {
	RightClickButton *btnList[] = {  &_walkButton, &_lookButton, &_useButton, &_talkButton, &_inventoryButton, &_optionsButton };

	for (int i = 0; i < 6; ++i) {
		btnList[i]->_owner = this;

		if (btnList[i]->_bounds.contains(pt))
			return btnList[i];
	}

	return NULL;
}

void RightClickDialog::draw() {
	// Save the covered background area
	_savedArea = Surface_getArea(_globals->_gfxManagerInstance.getSurface(), _bounds);

	// Draw the dialog image
	_globals->gfxManager().copyFrom(_surface, _bounds.left, _bounds.top);
}

bool RightClickDialog::process(Event &event) {
	switch (event.eventType) {
	case EVENT_MOUSE_MOVE: {
		// Check whether a button is highlighted
		RightClickButton *btn = findButton(event.mousePos);

		if (btn != _highlightedButton) {
			// De-highlight any previously selected button
			if (_highlightedButton) {
				_highlightedButton->highlight();
				_highlightedButton = NULL;
			}
			if (btn) {
				// Highlight the new button
				btn->highlight();
				_highlightedButton = btn;
			}
		}
		event.handled = true;
		return true;
	}

	case EVENT_BUTTON_DOWN:
		// If a button is highlighted, then flag the selected button index
		if (_highlightedButton)
			_selectedAction = _highlightedButton->_buttonIndex;
		else
			_selectedAction = _lookButton._buttonIndex;
		event.handled = true;
		return true;

	default:
		break;
	}

	return false;
}

void RightClickDialog::execute() {
	// Draw the dialog
	draw();

	// Dialog event handler loop
	_gfxManager.activate();

	while (!_vm->shouldQuit() && (_selectedAction == -1)) {
		Event evt;
		while (_globals->_events.getEvent(evt, EVENT_MOUSE_MOVE | EVENT_BUTTON_DOWN)) {
			evt.mousePos.x -= _bounds.left;
			evt.mousePos.y -= _bounds.top;

			process(evt);
		}

		g_system->delayMillis(10);
		g_system->updateScreen();
	}

	// Execute the specified action
	switch (_selectedAction) {
	case 1:
		// Look action
		_globals->_events.setCursor(CURSOR_LOOK);
		break;
	case 2:
		// Walk action
		_globals->_events.setCursor(CURSOR_WALK);
		break;
	case 3:
		// Use cursor
		_globals->_events.setCursor(CURSOR_USE);
		break;
	case 4:
		// Talk cursor
		_globals->_events.setCursor(CURSOR_TALK);
		break;
	case 5:
		// Inventory dialog
		InventoryDialog::show();
		break;
	case 6:
		// Dialog options
		OptionsDialog::show();
		break;
	}

	_gfxManager.deactivate();
}

} // End of namespace Ringworld

} // End of namespace TsAGE
