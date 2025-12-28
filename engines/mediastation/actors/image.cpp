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
		_loadType = chunk.readTypedByte();
		break;

	case kActorHeaderX:
		_xOffset = chunk.readTypedUint16();
		break;

	case kActorHeaderY:
		_yOffset = chunk.readTypedUint16();
		break;

	case kActorHeaderActorReference: {
		_actorReference = chunk.readTypedUint16();
		Actor *referencedActor = g_engine->getActorById(_actorReference);
		if (referencedActor == nullptr) {
			error("%s: Referenced actor %d doesn't exist or has not been read yet in this title", __func__, _actorReference);
		}
		if (referencedActor->type() != kActorTypeImage) {
			error("%s: Type mismatch of referenced actor %d", __func__, _actorReference);
		}
		ImageActor *referencedImage = static_cast<ImageActor *>(referencedActor);
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
		assert(args.empty());
		spatialShow();
		return returnValue;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		spatialHide();
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
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
	BitmapHeader *bitmapHeader = new BitmapHeader(chunk);
	_asset->bitmap = new Bitmap(chunk, bitmapHeader);
}

} // End of namespace MediaStation
