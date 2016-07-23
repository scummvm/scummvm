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
#include "titanic/titanic.h"

namespace Titanic {

CContinueSaveDialog::CContinueSaveDialog() {
	g_vm->_events->addTarget(this);
	_highlightedSlot = _selectedSlot = -999;
}

CContinueSaveDialog::~CContinueSaveDialog() {
	g_vm->_events->removeTarget();
}

void CContinueSaveDialog::addSavegame(int slot, const CString &name) {
	_saves.push_back(SaveEntry(slot, name));
}

int CContinueSaveDialog::show() {
	// Load images for the dialog
	loadImages();

	// Render the view
	render();

	// Event loop waiting for selection
	while (!g_vm->shouldQuit() && _selectedSlot == -999) {
		g_vm->_events->pollEventsAndWait();
	}

	return _selectedSlot;
}

void CContinueSaveDialog::loadImages() {
	_backdrop.load("Bitmap/BACKDROP");
	_evilTwin.load("Bitmap/EVILTWIN");
}

void CContinueSaveDialog::render() {
	Graphics::Screen &screen = *g_vm->_screen;
	screen.clear();
	screen.blitFrom(_backdrop, Common::Point(48, 22));
}

void CContinueSaveDialog::leftButtonDown(const Point &mousePos) {

}

void CContinueSaveDialog::leftButtonUp(const Point &mousePos) {

}

void CContinueSaveDialog::keyDown(Common::KeyState keyState) {
	if (keyState.keycode == Common::KEYCODE_ESCAPE)
		_selectedSlot = EXIT_GAME;
}

} // End of namespace Titanic
