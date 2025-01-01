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
#include "common/path.h"
#include "graphics/surface.h"

#include "tetraedge/te/te_scummvm_codec.h"

namespace Tetraedge {

TeScummvmCodec::TeScummvmCodec() : _loadedSurface(nullptr), _frameRate(0.0f),
_bottomBorder(0), _topBorder(0) {
}

TeScummvmCodec::~TeScummvmCodec() {
	if (_loadedSurface) {
		_loadedSurface->free();
		delete _loadedSurface;
	}
}

bool TeScummvmCodec::load(const TetraedgeFSNode &node) {
	Common::ScopedPtr<Common::SeekableReadStream> file(node.createReadStream());
	if (file && load(*file)) {
		_loadedPath = node.getPath();
		return true;
	}
	return false;
}

uint TeScummvmCodec::width() {
	if (_loadedSurface)
		return _loadedSurface->w;
	return 0;
}

uint TeScummvmCodec::height() {
	if (_loadedSurface)
		return _loadedSurface->h;
	return 0;
}

TeImage::Format TeScummvmCodec::imageFormat() {
	return TeImage::RGBA8;
}

bool TeScummvmCodec::update(uint i, TeImage &imgout) {
	if (!_loadedSurface)
		return false;

	if (!_loadedPath.empty())
		imgout.setAccessName(_loadedPath);

	if (imgout.w == _loadedSurface->w && imgout.h == _loadedSurface->h && imgout.format == _loadedSurface->format && !_bottomBorder && !_topBorder) {
		imgout.copyFrom(*_loadedSurface);
		return true;
	} else if (imgout.w == _loadedSurface->w && imgout.h == _loadedSurface->h
		&& !_bottomBorder && !_topBorder) {
		Graphics::PixelFormat destfmt = imgout.format;
		imgout.copyFrom(*_loadedSurface);
		imgout.convertToInPlace(destfmt);
		return true;
	}

	error("TODO: Implement TeScummvmCodec::update for different sizes");
}

void TeScummvmCodec::setLeftBorderSize(uint val) {
	error("left border not supported.");
}

void TeScummvmCodec::setRightBorderSize(uint val) {
	error("right border not supported.");
}

void TeScummvmCodec::setBottomBorderSize(uint val) {
	_bottomBorder = val;
}

void TeScummvmCodec::setTopBorderSize(uint val) {
	_topBorder = val;
}

} // end namespace Tetraedge
