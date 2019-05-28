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

#include "titanic/continue_save_dialog.h"
#include "titanic/support/movie_manager.h"
#include "titanic/titanic.h"
#include "common/error.h"
#include "common/str-array.h"
#include "graphics/screen.h"

namespace Titanic {

#define SAVEGAME_SLOTS_COUNT 5
#define RESTORE_X 346
#define RESTORE_Y 94
#define START_X 370
#define START_Y 276

CContinueSaveDialog::CContinueSaveDialog() {
	g_vm->_events->addTarget(this);
	_highlightedSlot = _selectedSlot = -999;
	_restoreState = _startState = -1;
	_mouseDown = false;
	_evilTwinShown = false;

	for (int idx = 0; idx < SAVEGAME_SLOTS_COUNT; ++idx) {
		Rect slotRect = getSlotBounds(idx);
		_slotNames[idx].setFontNumber(0);
		_slotNames[idx].setBounds(slotRect);
		_slotNames[idx].resize(3);
		_slotNames[idx].setMaxCharsPerLine(22);
		_slotNames[idx].setHasBorder(false);
		_slotNames[idx].setup();
	}
}

CContinueSaveDialog::~CContinueSaveDialog() {
	g_vm->_events->removeTarget();
}

void CContinueSaveDialog::addSavegame(int slot, const CString &name) {
	if (_saves.size() < SAVEGAME_SLOTS_COUNT) {
		_slotNames[_saves.size()].setText(name);
		_saves.push_back(SaveEntry(slot, name));
	}
}

Rect CContinueSaveDialog::getSlotBounds(int index) {
	return Rect(360, 164 + index * 19, 556, 180 + index * 19);
}

int CContinueSaveDialog::show() {
	// Load images for the dialog
	loadImages();

	// Render the view
	render();

	// Event loop waiting for selection
	while (!g_vm->shouldQuit() && _selectedSlot == -999) {
		g_vm->_events->pollEventsAndWait();

		if (g_vm->_loadSaveSlot != -1)
			_selectedSlot = g_vm->_loadSaveSlot;
	}
	if (g_vm->shouldQuit())
		_selectedSlot = -2;

	return _selectedSlot;
}

void CContinueSaveDialog::loadImages() {
	_backdrop.load("Bitmap/BACKDROP");
	_evilTwin.load("Bitmap/EVILTWIN");
	_restoreD.load("Bitmap/RESTORED");
	_restoreU.load("Bitmap/RESTOREU");
	_restoreF.load("Bitmap/RESTOREF");
	_startD.load("Bitmap/STARTD");
	_startU.load("Bitmap/STARTU");
	_startF.load("Bitmap/STARTF");
}

void CContinueSaveDialog::render() {
	Graphics::Screen &screen = *g_vm->_screen;
	screen.clear();
	screen.blitFrom(_backdrop, Common::Point(48, 22));
	CScreenManager::_screenManagerPtr->setSurfaceBounds(SURFACE_PRIMARY,
		Rect(48, 22, 48 + _backdrop.w, 22 + _backdrop.h));

	if (_evilTwinShown)
		screen.blitFrom(_evilTwin, Common::Point(78, 59));

	_restoreState = _startState = -1;
	renderButtons();
	renderSlots();
}

void CContinueSaveDialog::renderButtons() {
	Graphics::Screen &screen = *g_vm->_screen;
	Rect restoreRect(RESTORE_X, RESTORE_Y, RESTORE_X + _restoreU.w, RESTORE_Y + _restoreU.h);
	Rect startRect(START_X, START_Y, START_X + _startU.w, START_Y + _startU.h);

	// Determine the current state for the buttons
	int restoreState, startState;
	if (!restoreRect.contains(_mousePos))
		restoreState = 0;
	else
		restoreState = _mouseDown ? 1 : 2;

	if (!startRect.contains(_mousePos))
		startState = 0;
	else
		startState = _mouseDown ? 1 : 2;

	// Draw the start button
	if (startState != _startState) {
		_startState = startState;
		switch (_startState) {
		case 0:
			screen.blitFrom(_startU, Common::Point(START_X, START_Y));
			break;
		case 1:
			screen.blitFrom(_startD, Common::Point(START_X, START_Y));
			break;
		case 2:
			screen.blitFrom(_startF, Common::Point(START_X, START_Y));
			break;
		default:
			break;
		}
	}

	// Draw the restore button
	if (restoreState != _restoreState) {
		_restoreState = restoreState;
		switch (_restoreState) {
		case 0:
			screen.blitFrom(_restoreU, Common::Point(RESTORE_X, RESTORE_Y));
			break;
		case 1:
			screen.blitFrom(_restoreD, Common::Point(RESTORE_X, RESTORE_Y));
			break;
		case 2:
			screen.blitFrom(_restoreF, Common::Point(RESTORE_X, RESTORE_Y));
			break;
		default:
			break;
		}
	}
}

void CContinueSaveDialog::renderSlots() {
	for (int idx = 0; idx < (int)_saves.size(); ++idx) {
		byte rgb = (_highlightedSlot == idx) ? 255 : 0;
		_slotNames[idx].setColor(rgb, rgb, rgb);
		_slotNames[idx].setLineColor(0, rgb, rgb, rgb);
		_slotNames[idx].draw(CScreenManager::_screenManagerPtr);
	}
}

void CContinueSaveDialog::mouseMove(const Point &mousePos) {
	_mousePos = mousePos;
	renderButtons();
}

void CContinueSaveDialog::leftButtonDown(const Point &mousePos) {
	Rect eye1(188, 190, 192, 195), eye2(209, 192, 213, 197);

	if (g_vm->_events->isSpecialPressed(MK_SHIFT) &&
			(eye1.contains(mousePos) || eye2.contains(mousePos))) {
		// Show the Easter Egg "Evil Twin"
		_evilTwinShown = true;
		render();
	} else {
		// Standard mouse handling
		_mouseDown = true;
		mouseMove(mousePos);
	}
}

void CContinueSaveDialog::leftButtonUp(const Point &mousePos) {
	Rect restoreRect(RESTORE_X, RESTORE_Y, RESTORE_X + _restoreU.w, RESTORE_Y + _restoreU.h);
	Rect startRect(START_X, START_Y, START_X + _startU.w, START_Y + _startU.h);
	_mouseDown = false;

	if (_evilTwinShown) {
		_evilTwinShown = false;
		render();
		return;
	}

	if (restoreRect.contains(mousePos)) {
		// Flag to exit dialog and load highlighted slot. If no slot was
		// selected explicitly, then fall back on loading the first slot
		_selectedSlot = (_highlightedSlot == -999) ? _saves[0]._slot :
			_saves[_highlightedSlot]._slot;
	} else if (startRect.contains(mousePos)) {
		// Start a new game
		_selectedSlot = -1;
	} else {
		// Check whether a filled in slot was selected
		for (uint idx = 0; idx < _saves.size(); ++idx) {
			if (getSlotBounds(idx).contains(mousePos)) {
				_highlightedSlot = idx;
				render();
				break;
			}
		}
	}
}

void CContinueSaveDialog::keyDown(Common::KeyState keyState) {
	if (keyState.keycode == Common::KEYCODE_ESCAPE)
		_selectedSlot = EXIT_GAME;
}

} // End of namespace Titanic
