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

#include "mediastation/mediastation.h"
#include "mediastation/assets/image.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Image::Image(AssetHeader *header) : Asset(header) {
	if (header->_startup == kAssetStartupActive) {
		_isActive = true;
	}
}

Image::~Image() {
	delete _bitmap;
	_bitmap = nullptr;
}

Operand Image::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kSpatialShowMethod: {
		assert(args.empty());
		spatialShow();
		return Operand();
		break;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		spatialHide();
		return Operand();
		break;
	}

	case kSetDissolveFactorMethod: {
		assert(args.size() == 1);
		warning("Image::callMethod(): setDissolveFactor not implemented yet");
		return Operand();
	}


	default: {
		error("Image::callMethod(): Got unimplemented method ID %d", methodId);
	}
	}
}

void Image::redraw(Common::Rect &rect) {
	if (!_isActive) {
		return;
	}

	Common::Point leftTop = getLeftTop();
	Common::Rect bbox(leftTop, _bitmap->width(), _bitmap->height());
	Common::Rect areaToRedraw = bbox.findIntersectingRect(rect);
	if (!areaToRedraw.isEmpty()) {
		Common::Point originOnScreen(areaToRedraw.left, areaToRedraw.top);
		areaToRedraw.translate(-leftTop.x, -leftTop.y);
		g_engine->_screen->simpleBlitFrom(_bitmap->_surface, areaToRedraw, originOnScreen);
	}
}

void Image::spatialShow() {
	_isActive = true;
	g_engine->addPlayingAsset(this);
	Common::Rect bbox(getLeftTop(), _bitmap->width(), _bitmap->height());
	g_engine->_dirtyRects.push_back(bbox);
}

void Image::spatialHide() {
	_isActive = false;
	Common::Rect bbox(getLeftTop(), _bitmap->width(), _bitmap->height());
	g_engine->_dirtyRects.push_back(bbox);
}

Common::Point Image::getLeftTop() {
	return Common::Point(_header->_x + _header->_boundingBox->left, _header->_y + _header->_boundingBox->top);
}

void Image::readChunk(Chunk &chunk) {
	BitmapHeader *bitmapHeader = new BitmapHeader(chunk);
	_bitmap = new Bitmap(chunk, bitmapHeader);
}

} // End of namespace MediaStation
