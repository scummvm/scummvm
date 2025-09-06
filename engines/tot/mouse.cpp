/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "graphics/cursorman.h"

#include "tot/mouse.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

MouseManager::MouseManager() {
	_mouseArea = Common::Rect(0, 0, 320, 200);
	loadMasks();
	mouseX = 160;
	mouseY = 100;
	mouseClickX = mouseX;
	mouseClickY = mouseY;
	mouseMaskIndex = 1;
}

MouseManager::~MouseManager() {
}

void MouseManager::drawMask(int idx) {
	CursorMan.replaceCursor(_mouseMasks[idx].mask, 15, 15, 0, 0, 0);
}

void MouseManager::animateMouseIfNeeded() {
	if (g_engine->_chrono->_gameTick) {
		setMouseMask(_currentMouseMask);
		if (_currentMouseMask < 7) {
			// sync this with frame time
			_currentMouseMask++;
		} else
			_currentMouseMask = 0;
		CursorMan.showMouse(true);
	}
}

void MouseManager::hide() {
	CursorMan.showMouse(false);
}

void MouseManager::show() {
	CursorMan.showMouse(true);
}

void MouseManager::setMouseArea(Common::Rect rect) {
	_mouseArea = rect;
}

void MouseManager::warpMouse(Common::Point p) {
	p.x = CLIP<int16>(p.x, _mouseArea.left, _mouseArea.right);
	p.y = CLIP<int16>(p.y, _mouseArea.top, _mouseArea.bottom);

	g_system->warpMouse(p.x, p.y);
}

void MouseManager::printPos(int x, int y, int screenPosX, int screenPosY) {
	g_engine->_graphics->restoreBackgroundArea(screenPosX, screenPosY, screenPosX + 100, screenPosY + 10);
	g_engine->_graphics->euroText(Common::String::format("MousePos: %d, %d", x + 7, y + 7), screenPosX, screenPosY, Graphics::kTextAlignLeft);
}

Common::Point MouseManager::getClickCoordsWithinGrid() {
	int correctedMouseX = (mouseClickX + 7) / kXGridCount;
	int correctedMouseY = (mouseClickY + 7) / kYGridCount;
	return Common::Point(correctedMouseX, correctedMouseY);
}

Common::Point MouseManager::getMouseCoordsWithinGrid() {
	int correctedMouseX = (mouseX + 7) / kXGridCount;
	int correctedMouseY = (mouseY + 7) / kYGridCount;
	return Common::Point(correctedMouseX, correctedMouseY);
}

void MouseManager::warpMouse(int mask, int x, int y) {
	setMouseMask(_currentMouseMask);
	g_system->warpMouse(x, y);
}

void MouseManager::loadMasks() {
	Common::File mouseMaskFile;
	if (!mouseMaskFile.open(Common::Path("RATON.ACA")))
		showError(317);

	int numMouseMasks = mouseMaskFile.readByte();
	int mouseMaskSize = mouseMaskFile.readUint16LE();

	for (int i = 0; i < numMouseMasks; i++) {
		_mouseMasks[i].width = mouseMaskFile.readUint16LE();
		_mouseMasks[i].height = mouseMaskFile.readUint16LE();
		_mouseMasks[i].mask = (byte *)malloc(mouseMaskSize - 4);
		mouseMaskFile.read(_mouseMasks[i].mask, mouseMaskSize - 4);
	}
	mouseMaskFile.close();
}

void MouseManager::setMouseMask(int numMask) {
	drawMask(numMask);
	CursorMan.showMouse(true);
}

} // End of namespace Tot
