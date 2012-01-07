/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/cursor.h"
#include "engines/myst3/directorysubentry.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/scene.h"

#include "graphics/surface.h"
#include "graphics/imagedec.h"

namespace Myst3 {

struct CursorData {
	uint32 nodeID;
	uint16 hotspotX;
	uint16 hotspotY;
	Graphics::Surface *surface;
	double transparency;
};

static CursorData availableCursors[13] = {
		{ 1000,  8,  8, 0, 0.25 },
		{ 1001,  8,  8, 0, 0.5  },
		{ 1002,  8,  8, 0, 0.5  },
		{ 1003,  1,  5, 0, 0.5  },
		{ 1004, 14,  5, 0, 0.5  },
		{ 1005, 16, 14, 0, 0.5  },
		{ 1006, 16, 14, 0, 0.5  },
		{ 1007,  8,  8, 0, 0.55 },
		{ 1000,  8,  8, 0, 0.25 },
		{ 1001,  8,  8, 0, 0.5  },
		{ 1011, 16, 16, 0, 0.5  },
		{ 1000,  6,  1, 0, 0.5  },
		{    0,  0,  0, 0, 0    }
};

Cursor::Cursor(Myst3Engine *vm) :
	_vm(vm),
	_position(320, 210),
	_lockedAtCenter(false) {

	// Load available cursors
	loadAvailableCursors();

	// Generate texture
	generateTexture();

	// Set default cursor
	changeCursor(8);
}

void Cursor::loadAvailableCursors() {
	// Load available cursors
	for (uint i = 0; availableCursors[i].nodeID; i++) {
		const DirectorySubEntry *cursorDesc = _vm->getFileDescription("GLOB", availableCursors[i].nodeID, 0, DirectorySubEntry::kCursor);

		if (!cursorDesc)
			error("Cursor %d does not exist", availableCursors[i].nodeID);

		Common::MemoryReadStream *bmpStream = cursorDesc->getData();
		availableCursors[i].surface = Graphics::ImageDecoder::loadFile(*bmpStream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

		// Apply the colorkey for transparency
		for (uint u = 0; u < availableCursors[i].surface->w; u++) {
			for (uint v = 0; v < availableCursors[i].surface->h; v++) {
				uint32 *pixel = (uint32*)(availableCursors[i].surface->getBasePtr(u, v));
				if (*pixel == 0xFF00FF00)
					*pixel = 0x0000FF00;

			}
		}

		delete bmpStream;
	}
}

void Cursor::generateTexture() {
	glGenTextures(1, &_textureId);
	glBindTexture(GL_TEXTURE_2D, _textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureSize, _textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Cursor::~Cursor() {
	// Free available cursors
	for (uint i = 0; availableCursors[i].nodeID; i++) {
		if (availableCursors[i].surface) {
			availableCursors[i].surface->free();
			delete availableCursors[i].surface;
			availableCursors[i].surface = 0;
		}
	}

	// Delete texture
	glDeleteTextures(1, &_textureId);
}

void Cursor::changeCursor(uint32 index) {
	assert(index >= 0 && index <= 12);

	if (_currentCursorID != index) {
		_currentCursorID = index;
		uploadTexture();
	}
}

void Cursor::lockPosition(bool lock) {
	_lockedAtCenter = lock;

	if (_lockedAtCenter) {
		_position.x = 320;
		_position.y = 210;
	}
}

void Cursor::updatePosition(Common::Point &mouse) {
	if (!_lockedAtCenter) {
		_position += mouse;

		_position.x = CLIP<int16>(_position.x, 0, Scene::_originalWidth);
		_position.y = CLIP<int16>(_position.y, 0, Scene::_originalHeight);
	}
}

void Cursor::uploadTexture() {
	Graphics::Surface *bitmap = availableCursors[_currentCursorID].surface;

	glBindTexture(GL_TEXTURE_2D, _textureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmap->w, bitmap->h, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->pixels);
}

void Cursor::draw()
{
	CursorData &cursor = availableCursors[_currentCursorID];
	// Size of the cursor
	const float w = cursor.surface->w;
	const float h = cursor.surface->h;
	// Used fragment of texture
	const float u = w / (float)(_textureSize);
	const float v = h / (float)(_textureSize);

	const float left = _position.x - cursor.hotspotX;
	const float top = _position.y - cursor.hotspotY;

	float transparency;
	if (_lockedAtCenter)
		transparency = cursor.transparency;
	else
		transparency = 1.0f;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, transparency);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, _textureId);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, v); glVertex3f( left + 0, top + h, 1.0f);
		glTexCoord2f(u, v); glVertex3f( left + w, top + h, 1.0f);
		glTexCoord2f(0, 0); glVertex3f( left + 0, top + 0, 1.0f);
		glTexCoord2f(u, 0); glVertex3f( left + w, top + 0, 1.0f);
	glEnd();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

} /* namespace Myst3 */
