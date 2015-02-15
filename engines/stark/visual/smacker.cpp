/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/stark/visual/smacker.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

#include "common/str.h"
#include "common/archive.h"

#include "video/smk_decoder.h"

namespace Stark {

VisualSmacker::VisualSmacker(Gfx::Driver *gfx) :
		Visual(TYPE),
		_gfx(gfx),
		_texture(nullptr),
		_smacker(nullptr) {
}

VisualSmacker::~VisualSmacker() {
	delete _texture;
	delete _smacker;
}

void VisualSmacker::load(Common::SeekableReadStream *stream) {
	delete _texture;
	delete _smacker;

	_smacker = new Video::SmackerDecoder();
	_smacker->loadStream(stream);
	_smacker->start();

	_texture = _gfx->createTexture();

	update(0);
}

void VisualSmacker::render(const Common::Point &position) {
	_gfx->drawSurface(_texture, position);
}

void VisualSmacker::update(uint32 delta) {
	if (_smacker->endOfVideo()) {
		_smacker->rewind();
		_smacker->start();
	}

	if (_smacker->needsUpdate()) {
		const Graphics::Surface *surface = _smacker->decodeNextFrame();
		const byte *palette = _smacker->getPalette();

		// Convert the surface to RGBA
		Graphics::Surface *convertedSurface = new Graphics::Surface();
		convertedSurface->create(surface->w, surface->h, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

		for (int y = 0; y < surface->h; y++) {
			const byte *srcRow = (const byte *)surface->getBasePtr(0, y);
			uint32 *dstRow = (uint32 *)convertedSurface->getBasePtr(0, y);

			for (int x = 0; x < surface->w; x++) {
				byte index = *srcRow++;

				byte r = palette[index * 3];
				byte g = palette[index * 3 + 1];
				byte b = palette[index * 3 + 2];

				if (r != 0 || g != 255 || b != 255) {
					*dstRow++ = convertedSurface->format.RGBToColor(r, g, b);
				} else {
					// Cyan is the transparent color
					*dstRow++ = 0;
				}
			}
		}

		_texture->update(convertedSurface);

		convertedSurface->free();
		delete convertedSurface;

	}
}

bool VisualSmacker::isDone() {
	return _smacker->endOfVideo();
}

} // End of namespace Stark
