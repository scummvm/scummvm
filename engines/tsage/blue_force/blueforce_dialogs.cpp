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
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

namespace BlueForce {

/**
 * This dialog implements the right-click dialog
 */
RightClickDialog::RightClickDialog() : GfxDialog() {
	// Setup button areas
	_rectList1[0] = Rect(7, 50, 41, 67);
	_rectList1[1] = Rect(13, 27, 50, 50);
	_rectList1[2] = Rect(49, 27, 84, 50);
	_rectList1[3] = Rect(56, 50, 90, 67);
	_rectList1[4] = Rect(26, 68, 69, 99);

	_rectList3[0] = Rect(12, 49, 27, 64);
	_rectList3[1] = Rect(27, 31, 42, 46);
	_rectList3[2] = Rect(56, 31, 71, 46);
	_rectList3[3] = Rect(72, 50, 87, 65);
	_rectList3[4] = Rect(41, 81, 56, 96);

	// Set the palette and change the cursor
	GfxSurface cursor = surfaceFromRes(1, 5, 9);
	BF_GLOBALS._events.setCursor(cursor);

	setPalette();

	// Get the dialog image
	_surface = surfaceFromRes(1, 1, 1);

	// Set the dialog position
	Rect dialogRect;
	dialogRect.resize(_surface, 0, 0, 100);
	dialogRect.center(_globals->_events._mousePos.x, _globals->_events._mousePos.y);

	// Ensure the dialog will be entirely on-screen
	Rect screenRect = _globals->gfxManager()._bounds;
	screenRect.collapse(4, 4);
	dialogRect.contain(screenRect);

	// Load selected button images
	_btnImages.setVisage(1, 2);

	_bounds = dialogRect;
	_gfxManager._bounds = _bounds;

	_highlightedAction = -1;
	_selectedAction = -1;
}

RightClickDialog::~RightClickDialog() {
}

void RightClickDialog::draw() {
	// Save the covered background area
	_savedArea = Surface_getArea(_globals->_gfxManagerInstance.getSurface(), _bounds);

	// Draw the dialog image
	_globals->gfxManager().copyFrom(_surface, _bounds.left, _bounds.top);

	// Pre-process rect lists
	for (int idx = 0; idx < 5; ++idx) {
		_rectList2[idx] = _rectList1[idx];
		_rectList4[idx] = _rectList3[idx];

		_rectList2[idx].translate(_bounds.left, _bounds.top);
		_rectList4[idx].translate(_bounds.left, _bounds.top);
	}
}

bool RightClickDialog::process(Event &event) {
	switch (event.eventType) {
	case EVENT_MOUSE_MOVE: {
		// Check whether a button is highlighted
		int buttonIndex = 0;
		while ((buttonIndex < 5) && !_rectList1[buttonIndex].contains(event.mousePos))
			++buttonIndex;
		if (buttonIndex == 5)
			buttonIndex = -1;

		// If selection has changed, handle it
		if (buttonIndex != _highlightedAction) {
			if (_highlightedAction != -1) {
				// Another button was previously selected, so restore dialog
				_gfxManager.copyFrom(_surface, 0, 0);
			}

			if (buttonIndex != -1) {
				// Draw newly selected button
				GfxSurface btn = _btnImages.getFrame(buttonIndex + 1);
				_gfxManager.copyFrom(btn, _rectList3[buttonIndex].left, _rectList3[buttonIndex].top);
			}

			_highlightedAction = buttonIndex;
		}

		event.handled = true;
		return true;
	}

	case EVENT_BUTTON_DOWN:
		// Specify the selected action
		_selectedAction = (_highlightedAction == -1) ? 5 : _highlightedAction;
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
	CursorType cursorNum = CURSOR_NONE;
	switch (_selectedAction) {
	case 0:
		// Walk action
		cursorNum = BF_GLOBALS._player._canWalk ? CURSOR_WALK : CURSOR_USE;
		break;
	case 1:
		// Use action
		cursorNum = CURSOR_USE;
		break;
	case 2:
		// Look action
		cursorNum = CURSOR_LOOK;
		break;
	case 3:
		// Talk action
		cursorNum = CURSOR_TALK;
		break;
	case 4:
		// Options dialog
		break;
	}

	if (cursorNum != CURSOR_NONE)
		BF_GLOBALS._events.setCursor(cursorNum);

	_gfxManager.deactivate();
}

/*--------------------------------------------------------------------------*/

AmmoBeltDialog::AmmoBeltDialog() : GfxDialog() {
	_cursorNum = BF_GLOBALS._events.getCursor();
	_inDialog = -1;
	_closeFlag = false;

	// Get the dialog image
	_surface = surfaceFromRes(9, 5, 2);

	// Set the dialog position
	_dialogRect.resize(_surface, 0, 0, 100);
	_dialogRect.center(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	_bounds = _dialogRect;
	_gfxManager._bounds = _bounds;
	_savedArea = NULL;

	// Set up area rects
	_gunRect.set(0, 0, 82, 48);
	_clip1Rect.set(90, 6, _bounds.width(), 39);
	_clip2Rect.set(90, 40, _bounds.width(), _bounds.height());
	_loadedRect.set(50, 40, 60, 50);
}

AmmoBeltDialog::~AmmoBeltDialog() {
	BF_GLOBALS._events.setCursor(_cursorNum);
}

void AmmoBeltDialog::execute() {
	// Draw the dialog
	draw();

	// Dialog event handler loop
	_gfxManager.activate();

	while (!_vm->shouldQuit() && !_closeFlag) {
		Event evt;
		while (_globals->_events.getEvent(evt, EVENT_MOUSE_MOVE | EVENT_BUTTON_DOWN)) {
			evt.mousePos.x -= _bounds.left;
			evt.mousePos.y -= _bounds.top;

			process(evt);
		}

		g_system->delayMillis(10);
		g_system->updateScreen();
	}

	_gfxManager.deactivate();
}

bool AmmoBeltDialog::process(Event &event) {
	switch (event.eventType) {
	case EVENT_MOUSE_MOVE: {
		// Handle updating cursor depending on whether cursor is in dialog or not
		int inDialog = Rect(0, 0, _bounds.width(), _bounds.height()).contains(event.mousePos);
		if (inDialog != _inDialog) {
			// Update cursor
			BF_GLOBALS._events.setCursor(inDialog ? CURSOR_USE : CURSOR_EXIT);
			_inDialog = inDialog;
		}
		return true;
	}

	case EVENT_BUTTON_DOWN:
		if (!_inDialog)
			// Clicked outside dialog, so flag to close it
			_closeFlag = true;
		else {
			int v = (BF_GLOBALS.getFlag(fGunLoaded) ? 1 : 0) * (BF_GLOBALS.getFlag(fLoadedSpare) ? 2 : 1);

			// Handle first clip
			if ((v != 1) && _clip1Rect.contains(event.mousePos)) {
				if (BF_GLOBALS.getFlag(fGunLoaded)) {
					event.mousePos.x = event.mousePos.y = 0;
				}

				BF_GLOBALS.setFlag(fGunLoaded);
				BF_GLOBALS.clearFlag(fLoadedSpare);
			}

			// Handle second clip
			if ((v != 2) && _clip2Rect.contains(event.mousePos)) {
				if (BF_GLOBALS.getFlag(fGunLoaded)) {
					event.mousePos.x = event.mousePos.y = 0;
				}

				BF_GLOBALS.setFlag(fGunLoaded);
				BF_GLOBALS.setFlag(fLoadedSpare);
			}

			if (_gunRect.contains(event.mousePos) && BF_GLOBALS.getFlag(fGunLoaded)) {
				BF_GLOBALS.clearFlag(fGunLoaded);
				v = (BF_GLOBALS.getFlag(fGunLoaded) ? 1 : 0) * (BF_GLOBALS.getFlag(fLoadedSpare) ? 2 : 1);

				if (v != 2)
					BF_GLOBALS.clearFlag(fLoadedSpare);
			}

			draw();
		}

		return true;

	case EVENT_KEYPRESS:
		if ((event.kbd.keycode == Common::KEYCODE_ESCAPE) || (event.kbd.keycode == Common::KEYCODE_RETURN)) {
			// Escape pressed, so flag to close dialog
			_closeFlag = true;
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}
						   
void AmmoBeltDialog::draw() {
	Rect bounds = _bounds;

	if (!_savedArea) {
		// Save the covered background area
		_savedArea = Surface_getArea(_globals->_gfxManagerInstance.getSurface(), _bounds);
	} else {
		bounds.moveTo(0, 0);
	}

	// Draw the dialog image
	_globals->gfxManager().copyFrom(_surface, bounds.left, bounds.top);

	// Setup clip flags
	bool clip1 = true, clip2 = true;
	bool gunLoaded = BF_GLOBALS.getFlag(fGunLoaded);
	if (gunLoaded) {
		// A clip is currently loaded. Hide the appropriate clip
		if (BF_GLOBALS.getFlag(fLoadedSpare))
			clip2 = false;
		else
			clip1 = false;
	}

	// Draw the first clip if necessary
	if (clip1) {
		GfxSurface clipSurface = surfaceFromRes(9, 6, BF_GLOBALS._clip1Bullets);
		_clip1Rect.resize(clipSurface, _clip1Rect.left, _clip1Rect.top, 100);
		_globals->gfxManager().copyFrom(clipSurface, bounds.left + _clip1Rect.left, 
			bounds.top + _clip1Rect.top);
	}

	// Draw the second clip if necessary
	if (clip2) {
		GfxSurface clipSurface = surfaceFromRes(9, 6, BF_GLOBALS._clip2Bullets);
		_clip2Rect.resize(clipSurface, _clip2Rect.left, _clip2Rect.top, 100);
		_globals->gfxManager().copyFrom(clipSurface, bounds.left + _clip2Rect.left, 
			bounds.top + _clip2Rect.top);
	}

	// If a clip is loaded, draw the 'loaded' portion of the gun
	if (gunLoaded) {
		GfxSurface loadedSurface = surfaceFromRes(9, 7, 1);
		_loadedRect.resize(loadedSurface, _loadedRect.left, _loadedRect.top, 100);
		_globals->gfxManager().copyFrom(loadedSurface, bounds.left + _loadedRect.left, 
			bounds.top + _loadedRect.top);
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
