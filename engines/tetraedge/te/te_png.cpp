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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "image/png.h"
#include "graphics/surface.h"

#include "tetraedge/te/te_png.h"

namespace Tetraedge {

TePng::TePng(const Common::String &extn) : _height(0) {
	if (extn == "png#anim") {
		_nbFrames = 8;
		_frameRate = 8.0f;
	} else {
		_nbFrames = 1;
	}
}

TePng::~TePng() {
}

/*static*/
bool TePng::matchExtension(const Common::String &extn) {
	return extn == "png" || extn == "png#anim";
}

bool TePng::load(const TetraedgeFSNode &node) {
	if (!TeScummvmCodec::load(node))
		return false;

	_height = _loadedSurface->h / _nbFrames;

	return true;
}

bool TePng::load(Common::SeekableReadStream &stream) {
	if (_loadedSurface)
		delete _loadedSurface;
	_loadedSurface = nullptr;

	Image::PNGDecoder png;
	if (!png.loadStream(stream))
		return false;

	_loadedSurface = png.getSurface()->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	_height = _loadedSurface->h;

	return true;
}

bool TePng::update(uint i, TeImage &imgout) {
	if (_nbFrames == 1)
		return TeScummvmCodec::update(i, imgout);

	uint relFrame = i % _nbFrames;
	if (imgout.w == _loadedSurface->w && imgout.h == _height) {
		Common::Rect srcRect;
		srcRect.left = 0;
		srcRect.right = _loadedSurface->w;
		srcRect.top = relFrame * _height;
		srcRect.bottom = (relFrame + 1) * _height;
		imgout.blitFrom(*_loadedSurface, srcRect, Common::Point());
		return true;
	}

	error("TODO: Update animated png for different size");
}

// TODO: should this return true if last frame was 7?
bool TePng::isAtEnd() {
	return false;
}

TeImage::Format TePng::imageFormat() {
	if (_loadedSurface) {
		if (_loadedSurface->format == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24))
			return TeImage::RGBA8;
		else if (_loadedSurface->format == Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0))
			return TeImage::RGB8;
	}
	return TeImage::INVALID;
}

} // end namespace Tetraedge
