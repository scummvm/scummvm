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

#include "engines/myst3/archive.h"
#include "engines/myst3/cursor.h"
#include "engines/myst3/gfx.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/scene.h"
#include "engines/myst3/state.h"

#include "graphics/surface.h"

#include "image/bmp.h"

namespace Myst3 {

struct CursorData {
	uint32 nodeID;
	uint16 hotspotX;
	uint16 hotspotY;
	float transparency;
	float transparencyXbox;
};

static const CursorData availableCursors[] = {
	{ 1000,  8,  8, 0.25f, 0.00f }, // Default cursor
	{ 1001,  8,  8, 0.50f, 0.50f }, // On top of inventory item
	{ 1002,  8,  8, 0.50f, 0.50f }, // Drag cursor
	{ 1003,  1,  5, 0.50f, 0.50f },
	{ 1004, 14,  5, 0.50f, 0.50f },
	{ 1005, 16, 14, 0.50f, 0.50f },
	{ 1006, 16, 14, 0.50f, 0.50f },
	{ 1007,  8,  8, 0.55f, 0.55f },
	{ 1000,  8,  8, 0.25f, 0.00f }, // Default cursor
	{ 1001,  8,  8, 0.50f, 0.50f },
	{ 1011, 16, 16, 0.50f, 0.50f },
	{ 1000,  6,  1, 0.50f, 0.50f },
	{ 1000,  8,  8, 0.00f, 0.25f }  // Invisible cursor
};

Cursor::Cursor(Myst3Engine *vm) :
	_vm(vm),
	_position(vm->_scene->getCenter()),
	_hideLevel(0),
	_lockedAtCenter(false) {

	// The cursor is manually scaled
	_scaled = false;
	_isConstrainedToWindow = false;

	// Load available cursors
	loadAvailableCursors();

	// Set default cursor
	changeCursor(8);
}

void Cursor::loadAvailableCursors() {
	assert(_textures.empty());

	// Load available cursors
	for (uint i = 0; i < ARRAYSIZE(availableCursors); i++) {
		// Check if a cursor sharing the same texture has already been loaded
		if (_textures.contains(availableCursors[i].nodeID)) continue;

		// Load the cursor bitmap
		ResourceDescription cursorDesc = _vm->getFileDescription("GLOB", availableCursors[i].nodeID, 0, Archive::kRawData);
		if (!cursorDesc.isValid())
			error("Cursor %d does not exist", availableCursors[i].nodeID);

		Common::SeekableReadStream *bmpStream = cursorDesc.getData();

		Image::BitmapDecoder bitmapDecoder;
		if (!bitmapDecoder.loadStream(*bmpStream))
			error("Could not decode Myst III bitmap");
		const Graphics::Surface *surfaceBGRA = bitmapDecoder.getSurface();
		Graphics::Surface *surfaceRGBA = surfaceBGRA->convertTo(Texture::getRGBAPixelFormat());

		delete bmpStream;

		// Apply the colorkey for transparency
		for (uint y = 0; y < surfaceRGBA->h; y++) {
			byte *pixels = (byte *)(surfaceRGBA->getBasePtr(0, y));
			for (uint x = 0; x < surfaceRGBA->w; x++) {
				byte *r = pixels + 0;
				byte *g = pixels + 1;
				byte *b = pixels + 2;
				byte *a = pixels + 3;

				if (*r == 0 && *g == 0xFF && *b == 0 && *a == 0xFF) {
					*g = 0;
					*a = 0;
				}

				pixels += 4;
			}
		}

		// Create and store the texture
		_textures.setVal(availableCursors[i].nodeID, _vm->_gfx->createTexture(surfaceRGBA));

		surfaceRGBA->free();
		delete surfaceRGBA;
	}
}

Cursor::~Cursor() {
	// Free cursors textures
	for (TextureMap::iterator it = _textures.begin(); it != _textures.end(); it++) {
		_vm->_gfx->freeTexture(it->_value);
	}
}

void Cursor::changeCursor(uint32 index) {
	if (index >= ARRAYSIZE(availableCursors))
		return;

	if (_vm->getPlatform() == Common::kPlatformXbox) {
		// The cursor is hidden when it is not hovering hotspots
		if ((index == 0 || index == 8) && _vm->_state->getViewType() != kCube)
			index = 12;
	}

	_currentCursorID = index;
}

float Cursor::getTransparencyForId(uint32 cursorId) {
	assert(cursorId < ARRAYSIZE(availableCursors));
	if (_vm->getPlatform() == Common::kPlatformXbox) {
		return availableCursors[cursorId].transparencyXbox;
	} else {
		return availableCursors[cursorId].transparency;
	}
}

void Cursor::lockPosition(bool lock) {
	if (_lockedAtCenter == lock)
		return;

	_lockedAtCenter = lock;

	g_system->lockMouse(lock);

	Common::Point center = _vm->_scene->getCenter();
	if (_lockedAtCenter) {
		// Locking, just move the cursor at the center of the screen
		_position = center;
	} else {
		// Unlocking, warp the actual mouse position to the cursor
		g_system->warpMouse(center.x, center.y);
	}
}

void Cursor::updatePosition(const Common::Point &mouse) {
	if (!_lockedAtCenter) {
		_position = mouse;
	} else {
		_position = _vm->_scene->getCenter();
	}
}

Common::Point Cursor::getPosition(bool scaled) {
	if (scaled) {
		Common::Rect viewport = _vm->_gfx->viewport();

		// The rest of the engine expects 640x480 coordinates
		Common::Point scaledPosition = _position;
		scaledPosition.x -= viewport.left;
		scaledPosition.y -= viewport.top;
		scaledPosition.x = CLIP<int16>(scaledPosition.x, 0, viewport.width());
		scaledPosition.y = CLIP<int16>(scaledPosition.y, 0, viewport.height());
		scaledPosition.x *= Renderer::kOriginalWidth / (float) viewport.width();
		scaledPosition.y *= Renderer::kOriginalHeight / (float) viewport.height();

		return scaledPosition;
	} else {
		return _position;
	}
}

void Cursor::draw() {
	assert(_currentCursorID < ARRAYSIZE(availableCursors));

	const CursorData &cursor = availableCursors[_currentCursorID];

	Texture *texture = _textures[cursor.nodeID];
	if (!texture) {
		error("No texture for cursor with id %d", cursor.nodeID);
	}

	// Rect where to draw the cursor
	Common::Rect viewport = _vm->_gfx->viewport();
	float scale = MIN(
			viewport.width()  / (float) Renderer::kOriginalWidth,
			viewport.height() / (float) Renderer::kOriginalHeight
	);

	Common::Rect screenRect = Common::Rect(texture->width * scale, texture->height * scale);
	screenRect.translate(_position.x - cursor.hotspotX * scale, _position.y - cursor.hotspotY * scale);

	// Texture rect
	Common::Rect textureRect = Common::Rect(texture->width, texture->height);

	float transparency = 1.0f;

	int32 varTransparency = _vm->_state->getCursorTransparency();
	if (_lockedAtCenter || varTransparency == 0) {
		if (varTransparency >= 0)
			transparency = varTransparency / 100.0f;
		else
			transparency = getTransparencyForId(_currentCursorID);
	}

	_vm->_gfx->drawTexturedRect2D(screenRect, textureRect, texture, transparency);
}

void Cursor::setVisible(bool show) {
	if (show)
		_hideLevel = MAX<int32>(0, --_hideLevel);
	else
		_hideLevel++;
}

bool Cursor::isVisible() {
	return !_hideLevel && !_vm->_state->getCursorHidden() && !_vm->_state->getCursorLocked();
}

void Cursor::getDirection(float &pitch, float &heading) {
	if (_lockedAtCenter) {
		pitch = _vm->_state->getLookAtPitch();
		heading = _vm->_state->getLookAtHeading();
	} else {
		_vm->_scene->screenPosToDirection(_position, pitch, heading);
	}
}

} // End of namespace Myst3
