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

#include "engines/stark/visual/smacker.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
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
		_surface(nullptr),
		_texture(nullptr),
		_smacker(nullptr),
		_position(0, 0),
		_overridenFramerate(-1) {
	_surfaceRenderer = _gfx->createSurfaceRenderer();
}

VisualSmacker::~VisualSmacker() {
	delete _texture;
	delete _smacker;
	delete _surfaceRenderer;
}

void VisualSmacker::load(Common::SeekableReadStream *stream) {
	delete _texture;
	delete _smacker;

	_smacker = new Video::SmackerDecoder();
	_smacker->loadStream(stream);
	rewind();

	_texture = _gfx->createTexture();

	update();
}

void VisualSmacker::render(const Common::Point &position) {
	// The position argument contains the scroll offset
	_surfaceRenderer->render(_texture, _position - position);
}

void VisualSmacker::update() {
	if (_smacker->needsUpdate()) {
		_surface = _smacker->decodeNextFrame();
		const byte *palette = _smacker->getPalette();

		// Convert the surface to RGBA
		Graphics::Surface *convertedSurface = new Graphics::Surface();
		convertedSurface->create(_surface->w, _surface->h, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

		for (int y = 0; y < _surface->h; y++) {
			const byte *srcRow = (const byte *)_surface->getBasePtr(0, y);
			uint32 *dstRow = (uint32 *)convertedSurface->getBasePtr(0, y);

			for (int x = 0; x < _surface->w; x++) {
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

bool VisualSmacker::isPointSolid(const Common::Point &point) const {
	if (!_smacker || !_surface) {
		return false;
	}

	const byte *ptr = (const byte *)_surface->getBasePtr(point.x, point.y);
	const byte *palette = _smacker->getPalette();

	byte r = palette[*ptr * 3];
	byte g = palette[*ptr * 3 + 1];
	byte b = palette[*ptr * 3 + 2];

	// Cyan is the transparent color
	return r != 0x00 || g != 0xFF || b != 0xFF;
}

int VisualSmacker::getFrameNumber() const {
	if (_smacker && _smacker->isPlaying()) {
		return _smacker->getCurFrame();
	}
	return -1;
}

bool VisualSmacker::isDone() {
	return getCurrentTime() >= getDuration();
}

int VisualSmacker::getWidth() const {
	return _smacker->getWidth();
}

int VisualSmacker::getHeight() const {
	return _smacker->getHeight();
}

uint32 VisualSmacker::getDuration() const {
	return (_smacker->getRate().getInverse() * _smacker->getDuration().msecs()).toInt();
}

void VisualSmacker::rewind() {
	_smacker->rewind();
	_smacker->start();

	if (_overridenFramerate != -1) {
		Common::Rational originalFrameRate = _smacker->getFrameRate();
		Common::Rational playbackRate = _overridenFramerate / originalFrameRate;
		_smacker->setRate(playbackRate);
	}
}

uint32 VisualSmacker::getCurrentTime() const {
	return (_smacker->getRate().getInverse() * _smacker->getTime()).toInt();
}

void VisualSmacker::overrideFrameRate(int32 framerate) {
	_overridenFramerate = framerate;
}

void VisualSmacker::pause(bool pause) {
	_smacker->pauseVideo(pause);
}

} // End of namespace Stark
