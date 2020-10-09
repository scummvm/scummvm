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
#include "engines/stark/services/settings.h"

#include "common/str.h"
#include "common/archive.h"

#include "video/bink_decoder.h"
#include "video/smk_decoder.h"

namespace Stark {

VisualSmacker::VisualSmacker(Gfx::Driver *gfx) :
		Visual(TYPE),
		_gfx(gfx),
		_surface(nullptr),
		_texture(nullptr),
		_decoder(nullptr),
		_position(0, 0),
		_originalWidth(0),
		_originalHeight(0),
		_overridenFramerate(-1) {
	_surfaceRenderer = _gfx->createSurfaceRenderer();
}

VisualSmacker::~VisualSmacker() {
	delete _texture;
	delete _decoder;
	delete _surfaceRenderer;
}

void VisualSmacker::loadSmacker(Common::SeekableReadStream *stream) {
	delete _texture;
	delete _decoder;

	_decoder = new Video::SmackerDecoder();
	_decoder->setSoundType(Audio::Mixer::kSFXSoundType);
	_decoder->loadStream(stream);

	init();
}

void VisualSmacker::loadBink(Common::SeekableReadStream *stream) {
	delete _texture;
	delete _decoder;

	_decoder = new Video::BinkDecoder();
	_decoder->setSoundType(Audio::Mixer::kSFXSoundType);
	_decoder->setDefaultHighColorFormat(Gfx::Driver::getRGBAPixelFormat());
	_decoder->loadStream(stream);

	init();
}

void VisualSmacker::init() {
	_originalWidth  = _decoder->getWidth();
	_originalHeight = _decoder->getHeight();

	rewind();

	_texture = _gfx->createTexture();
	_texture->setSamplingFilter(StarkSettings->getImageSamplingFilter());

	update();
}

void VisualSmacker::readOriginalSize(Common::SeekableReadStream *stream) {
	Video::SmackerDecoder smacker;
	smacker.loadStream(stream);

	_originalWidth  = smacker.getWidth();
	_originalHeight = smacker.getHeight();
}

void VisualSmacker::render(const Common::Point &position) {
	assert(_decoder->getCurFrame() >= 0);

	// The position argument contains the scroll offset
	_surfaceRenderer->render(_texture, _position - position, _originalWidth, _originalHeight);
}

void VisualSmacker::update() {
	if (_decoder->needsUpdate()) {
		_surface = _decoder->decodeNextFrame();
		const byte *palette = _decoder->getPalette();

		if (palette) {
			// Convert the surface to RGBA
			Graphics::Surface convertedSurface;
			convertedSurface.create(_surface->w, _surface->h, Gfx::Driver::getRGBAPixelFormat());

			for (int y = 0; y < _surface->h; y++) {
				const byte *srcRow = (const byte *)_surface->getBasePtr(0, y);
				byte *dstRow = (byte *)convertedSurface.getBasePtr(0, y);

				for (int x = 0; x < _surface->w; x++) {
					byte index = *srcRow++;

					byte r = palette[index * 3];
					byte g = palette[index * 3 + 1];
					byte b = palette[index * 3 + 2];

					if (r != 0 || g != 255 || b != 255) {
						*dstRow++ = r;
						*dstRow++ = g;
						*dstRow++ = b;
						*dstRow++ = 0xFF;
					} else {
						// Cyan is the transparent color
						*dstRow++ = 0;
						*dstRow++ = 0;
						*dstRow++ = 0;
						*dstRow++ = 0;
					}
				}
			}

			_texture->update(&convertedSurface);

			convertedSurface.free();
		} else {
			_texture->update(_surface);
		}
	}
}

bool VisualSmacker::isPointSolid(const Common::Point &point) const {
	if (!_decoder || !_surface) {
		return false;
	}

	Common::Point scaledPoint;
	scaledPoint.x = point.x * _surface->w / _originalWidth;
	scaledPoint.y = point.y * _surface->h / _originalHeight;
	scaledPoint.x = CLIP<uint16>(scaledPoint.x, 0, _surface->w);
	scaledPoint.y = CLIP<uint16>(scaledPoint.y, 0, _surface->h);

	const byte *ptr = (const byte *)_surface->getBasePtr(scaledPoint.x, scaledPoint.y);
	const byte *palette = _decoder->getPalette();
	if (palette) {
		byte r = palette[*ptr * 3];
		byte g = palette[*ptr * 3 + 1];
		byte b = palette[*ptr * 3 + 2];

		// Cyan is the transparent color
		return r != 0x00 || g != 0xFF || b != 0xFF;
	} else {
		// Maybe implement this method in some other way to avoid having to keep the surface in memory
		return *(ptr + 3) == 0xFF;
	}
}

int VisualSmacker::getFrameNumber() const {
	if (_decoder && _decoder->isPlaying()) {
		return _decoder->getCurFrame();
	}
	return -1;
}

bool VisualSmacker::isDone() {
	return getCurrentTime() >= getDuration();
}

int VisualSmacker::getWidth() const {
	return _originalWidth;
}

int VisualSmacker::getHeight() const {
	return _originalHeight;
}

uint32 VisualSmacker::getDuration() const {
	return (_decoder->getRate().getInverse() * _decoder->getDuration().msecs()).toInt();
}

void VisualSmacker::rewind() {
	_decoder->rewind();
	_decoder->start();

	if (_overridenFramerate != -1) {
		Common::Rational originalFrameRate;

		Video::SmackerDecoder *smacker = dynamic_cast<Video::SmackerDecoder *>(_decoder);
		if (smacker) {
			originalFrameRate = smacker->getFrameRate();
		}

		Video::BinkDecoder *bink = dynamic_cast<Video::BinkDecoder *>(_decoder);
		if (bink) {
			originalFrameRate = bink->getFrameRate();
		}

		Common::Rational playbackRate = _overridenFramerate / originalFrameRate;
		_decoder->setRate(playbackRate);
	}
}

uint32 VisualSmacker::getCurrentTime() const {
	return (_decoder->getRate().getInverse() * _decoder->getTime()).toInt();
}

void VisualSmacker::overrideFrameRate(int32 framerate) {
	_overridenFramerate = framerate;
}

void VisualSmacker::pause(bool pause) {
	_decoder->pauseVideo(pause);
}

} // End of namespace Stark
