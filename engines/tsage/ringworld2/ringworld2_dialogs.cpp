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
#include "tsage/ringworld2/ringworld2_dialogs.h"

namespace TsAGE {

namespace Ringworld2 {

/**
 * This dialog implements the right-click dialog
 */
RightClickDialog::RightClickDialog() : GfxDialog() {
	// Setup button positions
	_btnList[0] = Common::Point(48, 12);
	_btnList[1] = Common::Point(31, 29);
	_btnList[2] = Common::Point(65, 29);
	_btnList[3] = Common::Point(14, 47);
	_btnList[4] = Common::Point(48, 47);
	_btnList[5] = Common::Point(83, 47);

	// Set the palette and change the cursor
	BF_GLOBALS._events.setCursor(CURSOR_ARROW);

	setPalette();

	// Get the dialog image and selected button images
	if (R2_GLOBALS._sceneManager._sceneNumber == 2900) {
		_surface = surfaceFromRes(2902, 1, 1);
		_btnImages.setVisage(2902, 2);
	} else {
		_surface = surfaceFromRes(1, 1, 1);
		_btnImages.setVisage(1, 2);
	}

	// Set the dialog position
	Rect dialogRect;
	dialogRect.resize(_surface, 0, 0, 100);
	dialogRect.center(g_globals->_events._mousePos.x, g_globals->_events._mousePos.y);

	// Ensure the dialog will be entirely on-screen
	Rect screenRect = g_globals->gfxManager()._bounds;
	screenRect.collapse(4, 4);
	dialogRect.contain(screenRect);

	_bounds = dialogRect;
	_gfxManager._bounds = _bounds;

	_highlightedAction = -1;
	_selectedAction = -1;
}

RightClickDialog::~RightClickDialog() {
}

void RightClickDialog::draw() {
	// Save the covered background area
	_savedArea = Surface_getArea(g_globals->_gfxManagerInstance.getSurface(), _bounds);

	// Draw the dialog image
	g_globals->gfxManager().copyFrom(_surface, _bounds.left, _bounds.top);
}

bool RightClickDialog::process(Event &event) {
	switch (event.eventType) {
	case EVENT_MOUSE_MOVE: {
		// Check whether a button is highlighted
		int buttonIndex;
		for (buttonIndex = 6; buttonIndex >= 0; --buttonIndex) {
			Rect tempRect(0, 0, 28, 29);
			tempRect.moveTo(_btnList[buttonIndex].x, _btnList[buttonIndex].y);

			if (tempRect.contains(event.mousePos))
				break;
		}
		// If selection has changed, handle it
		if (buttonIndex != _highlightedAction) {
			if (_highlightedAction != -1) {
				// Another button was previously selected, so restore dialog
				_gfxManager.copyFrom(_surface, 0, 0);
			}

			if (buttonIndex != -1) {
				// Draw newly selected button
				GfxSurface btn = _btnImages.getFrame(buttonIndex + 1);
				_gfxManager.copyFrom(btn, _btnList[buttonIndex].x, _btnList[buttonIndex].y);
			}

			_highlightedAction = buttonIndex;
		}

		event.handled = true;
		return true;
	}

	case EVENT_BUTTON_DOWN:
		// Specify the selected action
		_selectedAction = (_highlightedAction == -1) ? 999 : _highlightedAction;
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

	while (!g_vm->shouldQuit() && (_selectedAction == -1)) {
		Event evt;
		while (g_globals->_events.getEvent(evt, EVENT_MOUSE_MOVE | EVENT_BUTTON_DOWN)) {
			evt.mousePos.x -= _bounds.left;
			evt.mousePos.y -= _bounds.top;

			process(evt);
		}

		g_system->delayMillis(10);
		g_system->updateScreen();
	}

	// Execute the specified action
	CursorType cursorNum = CURSOR_NONE;
	switch (_selectedAction) {
	case 0:
		// Look action
		cursorNum = CURSOR_LOOK;
		break;
	case 1:
		// Walk action
		cursorNum = CURSOR_WALK;
		break;
	case 2:
		// Use action
		cursorNum = CURSOR_USE;
		break;
	case 3:
		// Talk action
		cursorNum = CURSOR_TALK;
		break;
	case 4:
		// Change player
		CharacterDialog::show();
		break;
	case 5:
		// Options dialog
		break;
	}

	if (cursorNum != CURSOR_NONE)
		BF_GLOBALS._events.setCursor(cursorNum);

	_gfxManager.deactivate();
}

/*--------------------------------------------------------------------------*/

void CharacterDialog::show() {
	CharacterDialog *dlg = new CharacterDialog();
	dlg->draw();

	/*GfxButton *btn = */dlg->execute(&dlg->_btnCancel);


	dlg->remove();
	delete dlg;
}

CharacterDialog::CharacterDialog() {
	// Set the element text
	_msgTitle.set(CHAR_TITLE, 140, ALIGN_LEFT);
	_btnQuinn.setText(CHAR_QUINN_MSG);
	_btnSeeker.setText(CHAR_SEEKER_MSG);
	_btnMiranda.setText(CHAR_MIRANDA_MSG);
	_btnCancel.setText(CHAR_CANCEL_MSG);

	// Set position of the elements
	_msgTitle._bounds.moveTo(5, 5);
	_btnQuinn._bounds.moveTo(25, _msgTitle._bounds.bottom + 1);
	_btnSeeker._bounds.moveTo(25, _btnQuinn._bounds.bottom + 1);
	_btnMiranda._bounds.moveTo(25, _btnSeeker._bounds.bottom + 1);
	_btnCancel._bounds.moveTo(25, _btnMiranda._bounds.bottom + 1);

	// Add the items to the dialog
	addElements(&_msgTitle, &_btnQuinn, &_btnSeeker, &_btnMiranda, &_btnCancel, NULL);

	// Set the dialog size and position
	frame();
	_bounds.collapse(-6, -6);
	setCenter(160, 100);
}

} // End of namespace Ringworld2

} // End of namespace TsAGE
