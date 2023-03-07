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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/ui/ornaments.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"

namespace Nancy {
namespace UI {

void ViewportOrnaments::init() {
	Common::Rect viewportBounds;
	Common::SeekableReadStream *viewChunk = g_nancy->getBootChunkStream("VIEW");
	viewChunk->seek(0);
	readRect(*viewChunk, _screenPosition);
	readRect(*viewChunk, viewportBounds);

	Graphics::ManagedSurface &object0 = g_nancy->_graphicsManager->_object0;

	_drawSurface.create(viewportBounds.width(), viewportBounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());

	uint8 palette[256 * 3];
	object0.grabPalette(palette, 0, 256);
	_drawSurface.setPalette(palette, 0, 256);

	// All values for the viewport ornaments are hardcoded and not stored in a chunk
	Common::Rect src[6] = {
		{ 0, 0, 31, 110 },
		{ 49, 0, 81, 110 },
		{ 33, 24, 45, 37 },
		{ 33, 69, 46, 82 },
		{ 33, 0, 43, 22 },
		{ 33, 39, 40, 59 }
	};

	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	setTransparent(true);

	// Top left
	_drawSurface.blitFrom(object0, src[0], Common::Point(0, 0));
	// Top right
	_drawSurface.blitFrom(object0, src[1], Common::Point(viewportBounds.right - src[1].width(), 0));
	// Bottom left
	_drawSurface.blitFrom(object0, src[2], Common::Point(0, viewportBounds.bottom - src[2].height()));
	// Bottom right
	_drawSurface.blitFrom(object0, src[3], Common::Point(viewportBounds.right - src[3].width(), viewportBounds.bottom - src[3].height()));
	// Middle left
	_drawSurface.blitFrom(object0, src[4], Common::Point(0, 204));
	// Middle right
	_drawSurface.blitFrom(object0, src[5], Common::Point(viewportBounds.right - src[5].width(), 205));


	RenderObject::init();
}

void TextboxOrnaments::init() {
	_screenPosition = g_nancy->_textboxScreenPosition;
	Common::Rect textboxBounds = _screenPosition;
	textboxBounds.moveTo(0, 0);

	Graphics::ManagedSurface &object0 = g_nancy->_graphicsManager->_object0;

	_drawSurface.create(textboxBounds.width(), textboxBounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());

	uint8 palette[256 * 3];
	object0.grabPalette(palette, 0, 256);
	_drawSurface.setPalette(palette, 0, 256);

	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	setTransparent(true);

	// Values for textbox ornaments are stored in the TBOX chunk
	Common::Rect src[14];
	Common::Rect dest[14];

	Common::SeekableReadStream *tboxChunk = g_nancy->getBootChunkStream("TBOX");
	tboxChunk->seek(0x3E);

	for (uint i = 0; i < 14; ++i) {
		readRect(*tboxChunk, src[i]);
	}

	for (uint i = 0; i < 14; ++i) {
		readRect(*tboxChunk, dest[i]);
	}

	for (uint i = 0; i < 14; ++i) {
		_drawSurface.blitFrom(object0, src[i], Common::Point(dest[i].left - _screenPosition.left, dest[i].top - _screenPosition.top));
	}

	RenderObject::init();
}

} // End of namespace UI
} // End of namespace Nancy
