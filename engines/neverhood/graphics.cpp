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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/graphics.h"
#include "neverhood/resource.h"
#include "neverhood/screen.h"

namespace Neverhood {

BaseSurface::BaseSurface(NeverhoodEngine *vm, int priority, int16 width, int16 height)
	: _vm(vm), _priority(priority), _visible(true) {
	
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = width;
	_drawRect.height = height;
	// TODO: Check if _sysRect is needed at all in the reimplementation...
	_sysRect.x = 0;
	_sysRect.y = 0;
	_sysRect.width = (width + 3) & 0xFFFC; // align by 4 bytes
	_sysRect.height = height;
	_clipRect.x1 = 0;
	_clipRect.y1 = 0;
	_clipRect.x2 = 640;
	_clipRect.y2 = 480;
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

BaseSurface::~BaseSurface() {
	delete _surface;
}

void BaseSurface::draw() {
	debug("BaseSurface::draw()");
	if (_surface && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		// TODO: _sysRect alternate drawing code (is that used?)
		_vm->_screen->drawSurface2(_surface, _drawRect, _clipRect);
	}
}

void BaseSurface::addDirtyRect() {
	// TODO
}

void BaseSurface::clear() {
	_surface->fillRect(Common::Rect(0, 0, _surface->w, _surface->h), 0);
}

void BaseSurface::drawSpriteResource(SpriteResource &spriteResource) {
	if (spriteResource.getDimensions().width <= _drawRect.width && 
		spriteResource.getDimensions().height <= _drawRect.height) {
		clear();
		spriteResource.draw((byte*)_surface->pixels, _surface->pitch, false, false);
	}
}

// Misc

void parseBitmapResource(byte *sprite, bool *rle, NDimensions *dimensions, NPoint *position, byte **palette, byte **pixels) {

	uint16 flags;
	
	flags = READ_LE_UINT16(sprite);
	sprite += 2;

	if (rle)
		*rle = flags & 1;

	if (flags & 2) {
		if (dimensions) {
			dimensions->width = READ_LE_UINT16(sprite);
			dimensions->height = READ_LE_UINT16(sprite + 2);
		}
		sprite += 4;
	} else if (dimensions) {
		dimensions->width = 1;
		dimensions->height = 1;
	}

	if (flags & 4) {
		if (position) {
			position->x = READ_LE_UINT16(sprite);
			position->y = READ_LE_UINT16(sprite + 2);
		}
		sprite += 4;
	} else if (position) {
		position->x = 0;
		position->y = 0;
	}

	if (flags & 8) {
		if (palette)
			*palette = sprite;
		sprite += 1024;
	} else if (palette)
		*palette = NULL;

	if (flags & 0x10) {
		if (pixels)
			*pixels = sprite;
	} else if (pixels)
		*pixels = NULL;

}

void unpackSpriteRle(byte *source, int width, int height, byte *dest, int destPitch, bool flipX, bool flipY) {

	// TODO: Flip

	int16 rows, chunks;
	int16 skip, copy;

	rows = READ_LE_UINT16(source);
	chunks = READ_LE_UINT16(source + 2);
	source += 4;

	do {
		if (chunks == 0) {
			dest += rows * destPitch;
		} else {
			while (rows-- > 0) {
				uint16 rowChunks = chunks;
				while (rowChunks-- > 0) {
					skip = READ_LE_UINT16(source);
					copy = READ_LE_UINT16(source + 2);
					source += 4;
					memcpy(dest + skip, source, copy);
					source += copy;
				}
				dest += destPitch;
			}
		}
		rows = READ_LE_UINT16(source);
		chunks = READ_LE_UINT16(source + 2);
		source += 4;
	} while (rows > 0);

}

void unpackSpriteNormal(byte *source, int width, int height, byte *dest, int destPitch, bool flipX, bool flipY) {

	// TODO: Flip

	int sourcePitch = (width + 3) & 0xFFFC;

	while (height-- > 0) {
		memcpy(dest, source, width);
		source += sourcePitch;
		dest += destPitch;
	}

}

} // End of namespace Neverhood
