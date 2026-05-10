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
#include "mediastation/actors/image.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

ImageAsset::~ImageAsset() {
	delete bitmap;
	bitmap = nullptr;
}

ImageActor::~ImageActor() {
	unregisterWithStreamManager();
}

void ImageActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		_asset = Common::SharedPtr<ImageAsset>(new ImageAsset);
		break;

	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderLoadType:
		_decompressInPlace = chunk.readTypedByte();
		break;

	case kActorHeaderX:
		_xOffset = chunk.readTypedUint16();
		break;

	case kActorHeaderY:
		_yOffset = chunk.readTypedUint16();
		break;

	case kActorHeaderActorReference: {
		_actorReference = chunk.readTypedUint16();
		ImageActor *referencedImage = static_cast<ImageActor *>(g_engine->getImtGod()->getActorByIdAndType(_actorReference, kActorTypeImage));
		_asset = referencedImage->_asset;
		break;
	}

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue ImageActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kSpatialShowMethod: {
		ARGCOUNTCHECK(0);
		spatialShow();
		break;
	}

	case kSpatialHideMethod: {
		ARGCOUNTCHECK(0);
		spatialHide();
		break;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
	return returnValue;
}

void ImageActor::draw(DisplayContext &displayContext) {
	if (_isVisible) {
		Common::Point origin = getBbox().origin();
		g_engine->getDisplayManager()->imageBlit(origin, _asset->bitmap, _dissolveFactor, &displayContext);
	}
}

void ImageActor::spatialShow() {
	_isVisible = true;
	invalidateLocalBounds();
}

void ImageActor::spatialHide() {
	_isVisible = false;
	invalidateLocalBounds();
}

Common::Rect ImageActor::getBbox() const {
	Common::Point origin(_xOffset + _boundingBox.left, _yOffset + _boundingBox.top);
	Common::Rect bbox(origin, _asset->bitmap->width(), _asset->bitmap->height());
	return bbox;
}

void ImageActor::readChunk(Chunk &chunk) {
	ImageInfo bitmapHeader = ImageInfo(chunk);
	_asset->bitmap = new PixMapImage(chunk, bitmapHeader, _decompressInPlace);
}

} // End of namespace MediaStation
