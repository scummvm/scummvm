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

#include "mediastation/actors/canvas.h"
#include "mediastation/actors/image.h"
#include "mediastation/actors/stage.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

void CanvasActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChannelIdent:
		// The original seems to read this and then throws it away!
		_channelIdent = chunk.readTypedChannelIdent();
		break;

	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderTransparency:
	case kActorHeaderCanvasTransparency:
		_hasTransparency = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderDissolveFactor:
		_dissolveFactor = chunk.readTypedDouble();
		break;

	case kActorHeaderX:
		_offset.x = chunk.readTypedGraphicUnit();
		break;

	case kActorHeaderY:
		_offset.y = chunk.readTypedGraphicUnit();
		break;

	case kActorHeaderLoadType:
		// The original seems to read this and then throws it away!
		chunk.readByte();
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue CanvasActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kSpatialShowMethod:
		ARGCOUNTCHECK(0);
		setVisibility(true);
		break;

	case kSpatialHideMethod:
		ARGCOUNTCHECK(0);
		setVisibility(false);
		break;

	case kCanvasClearToTransparencyMethod:
		ARGCOUNTCHECK(0);
		clearToTransparency();
		break;

	case kCanvasStampImageMethod: {
		ARGCOUNTCHECK(3);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		Common::Point stampPosition(x, y);
		uint actorId = args[2].asActorId();
		stampImage(stampPosition, actorId);
		break;
	}

	case kCanvasCopyScreenToMethod: {
		ARGCOUNTCHECK(2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		Common::Point dest(x, y);
		copyScreenTo(dest);
		break;
	}

	case kCanvasClearToPaletteMethod: {
		ARGCOUNTCHECK(1);
		uint colorIndex = static_cast<uint>(args[0].asFloat());
		clearToPalette(colorIndex);
		break;
	}

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}
	return returnValue;
}

void CanvasActor::loadIsComplete() {
	ImageInfo imageInfo;
	if (_hasTransparency) {
		imageInfo._compressionType = kUncompressedTransparentBitmap;
	} else {
		imageInfo._compressionType = kUncompressedBitmap;
	}

	imageInfo._dimensions = Common::Point(getBbox().width(), getBbox().height());
	imageInfo._stride = getBbox().width();

	_image = Common::SharedPtr<ImageAsset>(new ImageAsset);
	_image->bitmap = new PixMapImage(imageInfo);
	if (_hasTransparency) {
		fillCanvas(0x00);
	} else {
		fillCanvas(0xFE);
	}

	SpatialEntity::loadIsComplete();
}

void CanvasActor::setVisibility(bool visibility) {
	if (visibility != _isVisible) {
		_isVisible = visibility;
		invalidateLocalBounds();
	}
}

void CanvasActor::fillCanvas(uint paletteIndex) {
	if (_image != nullptr && _image->bitmap != nullptr) {
		Graphics::ManagedSurface &surface = _image->bitmap->_image;
		surface.fillRect(Common::Rect(0, 0, surface.w, surface.h), paletteIndex);
	}
}

void CanvasActor::clearToTransparency() {
	fillCanvas(0);
	invalidateLocalBounds();
}

void CanvasActor::stampImage(const Common::Point &dest, uint actorId) {
	// Set up the display context to draw to the canvas's image surface.
	if (_image != nullptr) {
		_displayContext._destImage = &_image->bitmap->_image;
		if (_displayContext._destImage == nullptr) {
			_displayContext.deleteClips();
		} else {
			_displayContext.verifyClipSize();
		}
	}

	// Although this method is named stampImage, it can actually stamp other spatial entities too.
	debugC(5, kDebugGraphics, "[%s] %s: %s at (%d, %d)",
		debugName(), __func__, g_engine->formatActorName(actorId).c_str(), dest.x, dest.y);
	SpatialEntity *imageToStamp = g_engine->getImtGod()->getSpatialEntityById(actorId);
	Common::Point imageToStampOriginalBoundsOrigin = imageToStamp->getBbox().origin();
	imageToStamp->moveTo(dest.x, dest.y);

	// The idea is to draw the entity in the proper place on the canvas (with a manufactured dirty region
	// so the draw actually happens), then restore the entity to its original place.
	Region dirtyRegion;
	Common::Rect imageToStampBounds = imageToStamp->getBbox();
	dirtyRegion.addRect(imageToStampBounds);
	if (_displayContext._destImage != nullptr) {
		_displayContext.addClip();
	}
	_displayContext.setClipTo(dirtyRegion);
	imageToStamp->draw(_displayContext);

	// Clean up display context if we're drawing to our own image.
	_displayContext.emptyCurrentClip();
	if (_image != nullptr && &_image->bitmap->_image == _displayContext._destImage) {
		_displayContext._destImage = nullptr;
		_displayContext.deleteClips();
	}

	imageToStamp->moveTo(imageToStampOriginalBoundsOrigin.x, imageToStampOriginalBoundsOrigin.y);
	invalidateLocalBounds();
}

void CanvasActor::copyScreenTo(const Common::Point &dest) {
	warning("[%s] %s: STUB", debugName(), __func__);
}

void CanvasActor::clearToPalette(uint colorIndex) {
	fillCanvas(colorIndex);
	invalidateLocalBounds();
}

void CanvasActor::draw(DisplayContext &displayContext) {
	if (_image != nullptr) {
		Common::Point drawPosition = getBbox().origin() + _offset;
		g_engine->getDisplayManager()->imageBlit(drawPosition, _image->bitmap, _dissolveFactor, &displayContext, nullptr, true);
	}
}

} // End of namespace MediaStation
