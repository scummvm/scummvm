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

Image::~Image() {
	if (_assetReference == 0) {
		// If we're just referencing another asset's bitmap,
		// don't delete that bitmap.
		delete _bitmap;
	}
	_bitmap = nullptr;
}

void Image::readParameter(Chunk &chunk, AssetHeaderSectionType paramType) {
	switch (paramType) {
	case kAssetHeaderChunkReference:
		_chunkReference = chunk.readTypedChunkReference();
		break;

	case kAssetHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kAssetHeaderLoadType:
		_loadType = chunk.readTypedByte();
		break;

	case kAssetHeaderDissolveFactor:
		_dissolveFactor = chunk.readTypedDouble();
		break;

	case kAssetHeaderX:
		_xOffset = chunk.readTypedUint16();
		break;

	case kAssetHeaderY:
		_yOffset = chunk.readTypedUint16();
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue Image::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kSpatialShowMethod: {
		assert(args.empty());
		spatialShow();
		return returnValue;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		spatialHide();
		return returnValue;
	}

	case kSetDissolveFactorMethod: {
		warning("STUB: setDissolveFactor");
		assert(args.size() == 1);
		_dissolveFactor = args[0].asFloat();
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

void Image::redraw(Common::Rect &rect) {
	if (!_isVisible) {
		return;
	}

	Common::Point leftTop = getLeftTop();
	Common::Rect bbox(leftTop, _bitmap->width(), _bitmap->height());
	Common::Rect areaToRedraw = bbox.findIntersectingRect(rect);
	if (!areaToRedraw.isEmpty()) {
		Common::Point originOnScreen(areaToRedraw.left, areaToRedraw.top);
		areaToRedraw.translate(-leftTop.x, -leftTop.y);
		areaToRedraw.clip(Common::Rect(0, 0, _bitmap->width(), _bitmap->height()));
		g_engine->_screen->simpleBlitFrom(_bitmap->_surface, areaToRedraw, originOnScreen);
	}
}

void Image::spatialShow() {
	_isVisible = true;
	Common::Rect bbox(getLeftTop(), _bitmap->width(), _bitmap->height());
	g_engine->_dirtyRects.push_back(bbox);
}

void Image::spatialHide() {
	_isVisible = false;
	Common::Rect bbox(getLeftTop(), _bitmap->width(), _bitmap->height());
	g_engine->_dirtyRects.push_back(bbox);
}

Common::Point Image::getLeftTop() {
	return Common::Point(_xOffset + _boundingBox.left, _yOffset + _boundingBox.top);
}

void Image::readChunk(Chunk &chunk) {
	BitmapHeader *bitmapHeader = new BitmapHeader(chunk);
	_bitmap = new Bitmap(chunk, bitmapHeader);
}

} // End of namespace MediaStation
