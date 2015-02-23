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

#include "engines/stark/cursor.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/visual/image.h"
#include "engines/stark/resources/object.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/item.h"
namespace Stark {

Cursor::Cursor(Gfx::Driver *gfx) :
		_gfx(gfx),
		_cursorImage(nullptr),
		_mouseText(nullptr),
		_currentCursorType(kNone),
		_currentHint("") {
}

Cursor::~Cursor() {
	delete _mouseText;
}

void Cursor::init() {
	setCursorType(kDefault);
}

void Cursor::setCursorType(CursorType type) {
	if (type == _currentCursorType) {
		return;
	}
	_currentCursorType = type;
	if (type == kNone) {
		_cursorImage = nullptr;
		return;
	}
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;

	// TODO: This is just a quick solution to get anything drawn.
	_cursorImage = staticProvider->getCursorImage(_currentCursorType);
}

void Cursor::setMousePosition(Common::Point pos) {
	_mousePos = _gfx->getScreenPosBounded(pos);
}

void Cursor::render() {
	_gfx->setScreenViewport(true); // The cursor is drawn unscaled
	if (_cursorImage) {
		_cursorImage->render(_mousePos);
	}
	if (_mouseText) {
		// TODO: Should probably query the image for the width of the cursor
		_gfx->drawSurface(_mouseText, Common::Point(_mousePos.x + 20, _mousePos.y));
	}
}

Common::Point Cursor::getMousePosition() const {
	// The rest of the engine expects 640x480 coordinates
	return _gfx->scalePoint(_mousePos);
}

void Cursor::setMouseHint(const Common::String &hint) {
	if (hint != _currentHint) {
		delete _mouseText;
		if (hint != "") {
			_mouseText = _gfx->createTextureFromString(hint, 0xFFFF0000);
		} else {
			_mouseText = nullptr;
		}
		_currentHint = hint;
	}
}

} // End of namespace Stark
