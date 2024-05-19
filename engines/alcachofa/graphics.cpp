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

#include "graphics.h"
#include "stream-helper.h"
#include "alcachofa.h"

#include "common/system.h"
#include "common/file.h"
#include "common/substream.h"
#include "image/tga.h"

using namespace Common;
using namespace Math;
using namespace Image;
using namespace Graphics;

namespace Alcachofa {

ITexture::ITexture(Point size) : _size(size) {}

AnimationBase::AnimationBase(String fileName, AnimationFolder folder)
	: _fileName(move(fileName))
	, _folder(folder) {
}

AnimationBase::~AnimationBase() {
	freeImages();
}

void AnimationBase::load() {
	if (_isLoaded)
		return;

	Common::String fullPath;
	switch (_folder) {
	case AnimationFolder::Animations: fullPath = "Animaciones/"; break;
	case AnimationFolder::Masks: fullPath = "Mascaras/"; break;
	case AnimationFolder::Backgrounds: fullPath = "Fondos/"; break;
	default: assert(false && "Invalid AnimationFolder");
	}
	if (_fileName.size() < 4 || scumm_strnicmp(_fileName.end() - 4, ".AN0", 4) != 0)
		_fileName += ".AN0";
	fullPath += _fileName;
	Common::File file;
	if (!file.open(fullPath.c_str())) {
		// original fallback
		fullPath = "Mascaras/" + _fileName;
		if (!file.open(fullPath.c_str()))
			error("Could not open animation %s", _fileName.c_str());
	}

	uint spriteCount = file.readUint32LE();
	assert(spriteCount < kMaxSpriteIDs);
	_spriteBases.reserve(spriteCount);

	uint imageCount = file.readUint32LE();
	_images.reserve(imageCount);
	_imageOffsets.reserve(imageCount);
	for (uint i = 0; i < imageCount; i++) {
		_images.push_back(readImage(file));
	}

	// an inconsistency, maybe a historical reason:
	// the sprite bases are also stored as fixed 256 array, but as sprite *indices*
	// have to be contiguous we do not need to do that ourselves.
	// but let's check in Debug to be sure
	for (uint i = 0; i < spriteCount; i++) {
		_spriteBases.push_back(file.readUint32LE());
		assert(_spriteBases.back() < imageCount);
	}
#ifdef _DEBUG
	for (uint i = spriteCount; i < kMaxSpriteIDs; i++)
		assert(file.readSint32LE() == 0);
#else
	file.skip(sizeof(int32) * (kMaxSpriteIDs - spriteCount));
#endif

	for (uint i = 0; i < imageCount; i++)
		_imageOffsets.push_back(readPoint(file));
	for (uint i = 0; i < kMaxSpriteIDs; i++)
		_spriteIndexMapping[i] = file.readSint32LE();

	uint frameCount = file.readUint32LE();
	_frames.reserve(frameCount);
	_spriteOffsets.reserve(frameCount * spriteCount);
	for (uint i = 0; i < frameCount; i++) {
		for (uint j = 0; j < spriteCount; j++)
			_spriteOffsets.push_back(file.readUint32LE());
		AnimationFrame frame;
		frame._center = readPoint(file);
		frame._offset = readPoint(file);
		frame._duration = file.readUint32LE();
		_frames.push_back(frame);
	}

	_isLoaded = true;
}

void AnimationBase::freeImages() {
	if (!_isLoaded)
		return;
	for (auto *image : _images) {
		if (image != nullptr)
			delete image;
	}
	_isLoaded = false;
}

ManagedSurface *AnimationBase::readImage(SeekableReadStream &stream) const {
	SeekableSubReadStream subStream(&stream, stream.pos(), stream.size());
	TGADecoder decoder;
	if (!decoder.loadStream(subStream))
		error("Failed to load TGA from animation %s", _fileName.c_str());

	// The length of the image is unknown but TGADecoder does not read
	// the end marker, so let's search for it.
	static const char *kExpectedMarker = "TRUEVISION-XFILE.";
	static const uint kMarkerLength = 18;
	char buffer[kMarkerLength] = { 0 };
	char *potentialStart = buffer + kMarkerLength;
	do {
		uint nextRead = potentialStart - buffer;
		if (potentialStart < buffer + kMarkerLength)
			memmove(buffer, potentialStart, kMarkerLength - nextRead);
		if (stream.read(buffer + kMarkerLength - nextRead, nextRead) != nextRead)
			error("Unexpected end-of-file in animation %s", _fileName.c_str());
		potentialStart = find(buffer + 1, buffer + kMarkerLength, kExpectedMarker[0]);
	} while (strncmp(buffer, kExpectedMarker, kMarkerLength) != 0);

	// instead of not storing unused frame images the animation contains
	// transparent 2x1 images. Let's just ignore them.
	auto source = decoder.getSurface();
	if (source->w == 2 && source->h == 1)
		return nullptr;

	auto target = source->convertTo(BlendBlit::getSupportedPixelFormat(), decoder.getPalette(), decoder.getPaletteColorCount());	
	return new ManagedSurface(target);
}

Animation::Animation(String fileName, AnimationFolder folder)
	: AnimationBase(fileName, folder) {
}

void Animation::load() {
	if (_isLoaded)
		return;
	AnimationBase::load();
	const auto withMipmaps = _folder != AnimationFolder::Backgrounds;
	Rect maxBounds = maxFrameBounds();
	_renderedSurface.create(maxBounds.width(), maxBounds.height(), BlendBlit::getSupportedPixelFormat());
	_renderedTexture = g_engine->renderer().createTexture(maxBounds.width(), maxBounds.height(), withMipmaps);
}

int32 Animation::imageIndex(int32 frameI, int32 spriteId) const {
	assert(frameI >= 0 && (uint)frameI < frameCount());
	assert(spriteId >= 0 && (uint)spriteId < spriteCount());
	int32 spriteIndex = _spriteIndexMapping[spriteId];
	int32 offset = _spriteOffsets[frameI * spriteCount() + spriteIndex];
	return offset <= 0 ? -1
		: offset + _spriteBases[spriteIndex] - 1;
}

Rect Animation::spriteBounds(int32 frameI, int32 spriteId) const {
	int32 imageI = imageIndex(frameI, spriteId);
	auto image = imageI < 0 ? nullptr : _images[imageI];
	return image == nullptr ? Rect()
		: Rect(_imageOffsets[imageI], image->w, image->h);
}

Rect Animation::frameBounds(int32 frameI) const {
	if (spriteCount() == 0)
		return Rect();
	Rect bounds = spriteBounds(frameI, 0);
	for (uint spriteI = 1; spriteI < spriteCount(); spriteI++)
		bounds.extend(spriteBounds(frameI, spriteI));
	return bounds;
}

Rect Animation::maxFrameBounds() const {
	if (frameCount() == 0)
		return Rect();
	Rect bounds = frameBounds(0);
	for (uint frameI = 1; frameI < frameCount(); frameI++)
		bounds.extend(frameBounds(frameI));
	return bounds;
}

void Animation::prerenderFrame(int32 frameI) {
	assert(frameI >= 0 && (uint)frameI < frameCount());
	if (frameI == _renderedFrameI)
		return;
	auto bounds = frameBounds(frameI);
	_renderedSurface.clear();
	for (uint spriteI = 0; spriteI < spriteCount(); spriteI++) {
		int32 imageI = imageIndex(frameI, spriteI);
		auto image = imageI < 0 ? nullptr : _images[imageI];
		if (image == nullptr)
			continue;
		int offsetX = _imageOffsets[imageI].x - bounds.left;
		int offsetY = _imageOffsets[imageI].y - bounds.top;
		image->blendBlitTo(_renderedSurface, offsetX, offsetY);
	}

	if (_premultiplyAlpha != 100) {
		byte *itPixel = (byte*)_renderedSurface.getPixels();
		uint componentCount = _renderedSurface.w * _renderedSurface.h * 4;
		for (uint32 i = 0; i < componentCount; i++, itPixel++)
			*itPixel = *itPixel * _premultiplyAlpha / 100;
	}

	_renderedTexture->update(_renderedSurface);
	_renderedFrameI = frameI;
}

void Animation::draw2D(int32 frameI, Vector2d center, float scale, Angle rotation, BlendMode blendMode, Color color) {
	prerenderFrame(frameI);
	auto bounds = frameBounds(frameI);
	Vector2d texMin(0, 0);
	Vector2d texMax((float)bounds.width() / _renderedSurface.w, (float)bounds.height() / _renderedSurface.h);

	Vector2d size(bounds.width(), bounds.height());
	Vector2d offset(
		bounds.left - _frames[frameI]._center.x + _frames[frameI]._offset.x,
		bounds.top - _frames[frameI]._center.y + _frames[frameI]._offset.y);
	center += offset * scale;
	size *= scale;

	auto &renderer = g_engine->renderer();
	renderer.setTexture(_renderedTexture.get());
	//renderer.setTexture(nullptr);
	renderer.setBlendMode(blendMode);
	renderer.quad(center, size, color, rotation, texMin, texMax);
}

Graphic::Graphic() {
}

Graphic::Graphic(ReadStream &stream) {
	_center.x = stream.readSint16LE();
	_center.y = stream.readSint16LE();
	_scale = stream.readSint16LE();
	_order = stream.readSByte();
	auto animationName = readVarString(stream);
	_animation.reset(new Animation(std::move(animationName)));
}

void Graphic::start(bool isLooping) {
	_isPaused = false;
	_isLooping = isLooping;
	_lastTime = g_system->getMillis();
}

void Graphic::stop() {
	_isPaused = true;
	_isLooping = false;
	_lastTime = g_system->getMillis() - _lastTime;
}

void Graphic::serializeSave(Serializer &serializer) {
	syncPoint(serializer, _center);
	serializer.syncAsSint16LE(_scale);
	serializer.syncAsUint32LE(_lastTime);
	serializer.syncAsByte(_isPaused);
	serializer.syncAsByte(_isLooping);
	serializer.syncAsFloatLE(_camAcceleration);
}

}
