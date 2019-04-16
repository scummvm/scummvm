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

#include "engines/stark/visual/image.h"

#include "graphics/surface.h"
#include "image/png.h"

#include "engines/stark/formats/xmg.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"

namespace Stark {

VisualImageXMG::VisualImageXMG(Gfx::Driver *gfx) :
		Visual(TYPE),
		_gfx(gfx),
		_texture(nullptr),
		_surface(nullptr),
		_originalWidth(0),
		_originalHeight(0) {
	_surfaceRenderer = _gfx->createSurfaceRenderer();
}

VisualImageXMG::~VisualImageXMG() {
	if (_surface) {
		_surface->free();
	}
	delete _surface;
	delete _texture;
	delete _surfaceRenderer;
}

void VisualImageXMG::setHotSpot(const Common::Point &hotspot) {
	_hotspot = hotspot;
}

void VisualImageXMG::load(Common::ReadStream *stream) {
	assert(!_surface && !_texture);

	// Decode the XMG
	_surface = Formats::XMGDecoder::decode(stream);
	_texture = _gfx->createTexture(_surface);
	_texture->setSamplingFilter(StarkSettings->getImageSamplingFilter());

	_originalWidth  = _surface->w;
	_originalHeight = _surface->h;
}

void VisualImageXMG::readOriginalSize(Common::ReadStream *stream) {
	Formats::XMGDecoder::readSize(stream, _originalWidth, _originalHeight);
}

bool VisualImageXMG::loadPNG(Common::SeekableReadStream *stream) {
	assert(!_surface && !_texture);

	// Decode the XMG
	Image::PNGDecoder pngDecoder;
	if (!pngDecoder.loadStream(*stream)) {
		return false;
	}

	if (pngDecoder.getPalette()) {
		warning("Indexed colors PNG images are not supported");
		return false;
	}

	if (StarkSettings->shouldPreMultiplyReplacementPNGs()) {
		// We can do alpha pre-multiplication when loading for
		// convenience when testing modded graphics.
		_surface = multiplyColorWithAlpha(pngDecoder.getSurface());
	} else {
		_surface = pngDecoder.getSurface()->convertTo(Gfx::Driver::getRGBAPixelFormat());
	}

	_texture = _gfx->createTexture(_surface);
	_texture->setSamplingFilter(StarkSettings->getImageSamplingFilter());

	return true;
}

Graphics::Surface *VisualImageXMG::multiplyColorWithAlpha(const Graphics::Surface *source) {
	assert(source->format == Gfx::Driver::getRGBAPixelFormat());

	Graphics::Surface *dest = new Graphics::Surface();
	dest->create(source->w, source->h, Gfx::Driver::getRGBAPixelFormat());

	for (uint y = 0; y < source->h; y++) {
		const uint8 *src = (const uint8 *) source->getBasePtr(0, y);
		uint8 *dst = (uint8 *) dest->getBasePtr(0, y);

		for (uint x = 0; x < source->w; x++) {
			uint8 a, r, g, b;
			r = *src++;
			g = *src++;
			b = *src++;
			a = *src++;

			if (a != 0xFF) {
				r = (int) r * a / 255;
				g = (int) g * a / 255;
				b = (int) b * a / 255;
			}

			*dst++ = r;
			*dst++ = g;
			*dst++ = b;
			*dst++ = a;
		}
	}

	return dest;
}

void VisualImageXMG::render(const Common::Point &position, bool useOffset) {
	render(position, useOffset, true);
}

void VisualImageXMG::render(const Common::Point &position, bool useOffset, bool unscaled) {
	Common::Point drawPos = useOffset ? position - _hotspot : position;

	if (!unscaled) {
		uint width = _gfx->scaleWidthOriginalToCurrent(_originalWidth);
		uint height = _gfx->scaleHeightOriginalToCurrent(_originalHeight);
		_surfaceRenderer->render(_texture, drawPos, width, height);
	} else {
		_surfaceRenderer->render(_texture, drawPos, _originalWidth, _originalHeight);
	}
}

void VisualImageXMG::setFadeLevel(float fadeLevel) {
	_surfaceRenderer->setFadeLevel(fadeLevel);
}

bool VisualImageXMG::isPointSolid(const Common::Point &point) const {
	assert(_surface);

	if (_originalWidth < 32 || _originalHeight < 32) {
		return true; // Small images are always solid
	}

	Common::Point scaledPoint;
	scaledPoint.x = point.x * _surface->w / _originalWidth;
	scaledPoint.y = point.y * _surface->h / _originalHeight;
	scaledPoint.x = CLIP<uint16>(scaledPoint.x, 0, _surface->w);
	scaledPoint.y = CLIP<uint16>(scaledPoint.y, 0, _surface->h);

	// Maybe implement this method in some other way to avoid having to keep the surface in memory
	const byte *ptr = (const byte *) _surface->getBasePtr(scaledPoint.x, scaledPoint.y);
	return *(ptr + 3) == 0xFF;
}

int VisualImageXMG::getWidth() const {
	return _originalWidth;
}

int VisualImageXMG::getHeight() const {
	return _originalHeight;
}

const Graphics::Surface *VisualImageXMG::getSurface() const {
	assert(_surface);
	return _surface;
}

} // End of namespace Stark
