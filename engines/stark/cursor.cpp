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
#include "engines/stark/visual/image.h"
#include "engines/stark/resources/object.h"
namespace Stark {

Cursor::Cursor(Gfx::Driver *gfx) :
		_gfx(gfx),
		_cursorImage(nullptr),
		_mouseText(nullptr) {
}

Cursor::~Cursor() {
	delete _mouseText;
}

void Cursor::init() {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;

	// TODO: This is just a quick solution to get anything drawn.
	_cursorImage = staticProvider->getCursorImage(0);
}

void Cursor::setMousePosition(Common::Point pos) {
	_mousePos = pos;
}

void Cursor::render() {
	_gfx->setScreenViewport(true); // The cursor is drawn unscaled

	_cursorImage->render(_mousePos);
	if (_mouseText) {
		// TODO: Should probably query the image for the width of the cursor
		_gfx->drawSurface(_mouseText, Common::Point(_mousePos.x + 20, _mousePos.y));
	}
}

Common::Point Cursor::getMousePosition() const {
	// The rest of the engine expects 640x480 coordinates
	return _gfx->scalePoint(_mousePos);
}

void Cursor::handleMouseOver(Gfx::RenderEntryArray renderEntries) {
	Gfx::RenderEntryArray::iterator element = renderEntries.begin();
	Gfx::RenderEntry *mouseOverEntry = nullptr;
	// We need this scaled. (Optionally, if we want to scale the cursor, we can move the scaling to the setMousePosition-function)
	Common::Point mousePos = getMousePosition();
	while (element != renderEntries.end()) {
		if ((*element)->containsPoint(mousePos)) {
			if (!mouseOverEntry) {
				mouseOverEntry = *element;
			// This assumes that lower sort keys are more important than higher sortkeys.
			} else if (Gfx::RenderEntry::compare(*element, mouseOverEntry)) {
				mouseOverEntry = *element;
			}
		}
		++element;
	}
	if (mouseOverEntry) {
		_mouseText = _gfx->createTextureFromString(mouseOverEntry->getOwner()->getName(), 0xFFFF0000);
	} else {
		delete _mouseText;
		_mouseText = nullptr;
	}
}

} // End of namespace Stark
