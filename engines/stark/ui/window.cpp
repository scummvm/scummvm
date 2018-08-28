/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/window.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/services/services.h"
#include "engines/stark/ui/cursor.h"

namespace Stark {

Window::Window(Gfx::Driver *gfx, Cursor *cursor) :
		_gfx(gfx),
		_cursor(cursor),
		_visible(false) {
}

Window::~Window() {
}

void Window::handleGameLoop() {
	if (!_visible) {
		return;
	}

	onGameLoop();
}

void Window::render() {
	if (!_visible) {
		return;
	}

	_gfx->setViewport(_position);

	onRender();
}

Graphics::Surface *Window::getScreenshot() const {
	if (!_visible) {
		return nullptr;
	}

	_gfx->setViewport(_position);
	return _gfx->getViewportScreenshot();
}

bool Window::isMouseInside() const {
	if (!_visible) {
		return false;
	}

	Common::Point mousePos = _cursor->getMousePosition();
	return _position.contains(mousePos);
}

bool Window::isVisible() const {
	return _visible;
}

Common::Point Window::getRelativeMousePosition() const {
	Common::Point mousePos = _cursor->getMousePosition();
	return mousePos - Common::Point(_position.left, _position.top);
}

void Window::handleMouseMove() {
	if (!_visible) {
		return;
	}

	if (isMouseInside()) {
		onMouseMove(getRelativeMousePosition());
	}
}

void Window::handleClick() {
	if (!_visible) {
		return;
	}

	if (isMouseInside()) {
		onClick(getRelativeMousePosition());
	}
}

void Window::handleRightClick() {
	if (!_visible) {
		return;
	}

	if (isMouseInside()) {
		onRightClick(getRelativeMousePosition());
	}
}

void Window::handleDoubleClick() {
	if (!_visible) {
		return;
	}

	if (isMouseInside()) {
		onDoubleClick(getRelativeMousePosition());
	}
}

} // End of namespace Stark
