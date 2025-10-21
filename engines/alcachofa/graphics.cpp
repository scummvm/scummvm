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

#include "alcachofa/graphics.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/shape.h"
#include "alcachofa/global-ui.h"

#include "common/system.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/bufferedstream.h"
#include "image/tga.h"

using namespace Common;
using namespace Math;
using namespace Image;
using namespace Graphics;

namespace Alcachofa {

ITexture::ITexture(Point size) : _size(size) {
	if ((!isPowerOfTwo(size.x) || !isPowerOfTwo(size.y)) &&
		g_engine->renderer().requiresPoTTextures())
		warning("Created unsupported NPOT texture (%dx%d)", size.x, size.y);
}

void IDebugRenderer::debugShape(const Shape &shape, Color color) {
	constexpr uint kMaxPoints = 16;
	Vector2d points2d[kMaxPoints];
	for (auto polygon : shape) {
		// I don't think this will happen but let's be sure
		assert(polygon._points.size() <= kMaxPoints);
		for (uint i = 0; i < polygon._points.size(); i++) {
			const auto p3d = polygon._points[i];
			const auto p2d = g_engine->camera().transform3Dto2D(Vector3d(p3d.x, p3d.y, kBaseScale));
			points2d[i] = Vector2d(p2d.x(), p2d.y());
		}

		debugPolygon({ points2d, polygon._points.size() }, color);
	}
}

AnimationBase::AnimationBase(String fileName, AnimationFolder folder)
	: _fileName(reencode(fileName))
	, _folder(folder) {}

AnimationBase::~AnimationBase() {
	freeImages();
}

void AnimationBase::load() {
	if (_isLoaded)
		return;

	String fullPath;
	switch (_folder) {
	case AnimationFolder::Animations:
		fullPath = "Animaciones/";
		break;
	case AnimationFolder::Masks:
		fullPath = "Mascaras/";
		break;
	case AnimationFolder::Backgrounds:
		fullPath = "Fondos/";
		break;
	default:
		assert(false && "Invalid AnimationFolder");
		break;
	}
	if (_fileName.size() < 4 || scumm_strnicmp(_fileName.end() - 4, ".AN0", 4) != 0)
		_fileName += ".AN0";
	fullPath += _fileName;

	File file;
	if (!file.open(fullPath.c_str())) {
		// original fallback
		fullPath = "Mascaras/" + _fileName;
		if (!file.open(fullPath.c_str())) {
			loadMissingAnimation();
			return;
		}
	}
	// Reading the images is a major bottleneck in loading, buffering helps a lot with that
	ScopedPtr<SeekableReadStream> stream(wrapBufferedSeekableReadStream(&file, file.size(), DisposeAfterUse::NO));

	uint spriteCount = stream->readUint32LE();
	assert(spriteCount < kMaxSpriteIDs);
	_spriteBases.reserve(spriteCount);

	uint imageCount = stream->readUint32LE();
	_images.reserve(imageCount);
	_imageOffsets.reserve(imageCount);
	for (uint i = 0; i < imageCount; i++) {
		_images.push_back(readImage(*stream));
	}

	// an inconsistency, maybe a historical reason:
	// the sprite bases are also stored as fixed 256 array, but as sprite *indices*
	// have to be contiguous we do not need to do that ourselves.
	// but let's check in Debug to be sure
	for (uint i = 0; i < spriteCount; i++) {
		_spriteBases.push_back(stream->readUint32LE());
		assert(_spriteBases.back() < imageCount);
	}
#ifdef ALCACHOFA_DEBUG
	for (uint i = spriteCount; i < kMaxSpriteIDs; i++)
		assert(stream->readSint32LE() == 0);
#else
	stream->skip(sizeof(int32) * (kMaxSpriteIDs - spriteCount));
#endif

	for (uint i = 0; i < imageCount; i++)
		_imageOffsets.push_back(readPoint(*stream));
	for (uint i = 0; i < kMaxSpriteIDs; i++)
		_spriteIndexMapping[i] = stream->readSint32LE();

	uint frameCount = stream->readUint32LE();
	_frames.reserve(frameCount);
	_spriteOffsets.reserve(frameCount * spriteCount);
	_totalDuration = 0;
	for (uint i = 0; i < frameCount; i++) {
		for (uint j = 0; j < spriteCount; j++)
			_spriteOffsets.push_back(stream->readUint32LE());
		AnimationFrame frame;
		frame._center = readPoint(*stream);
		frame._offset = readPoint(*stream);
		frame._duration = stream->readUint32LE();
		_frames.push_back(frame);
		_totalDuration += frame._duration;
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
	_images.clear();
	_spriteOffsets.clear();
	_spriteBases.clear();
	_frames.clear();
	_imageOffsets.clear();
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

	const auto &palette = decoder.getPalette();
	auto target = new ManagedSurface();
	target->setPalette(palette.data(), 0, palette.size());
	target->convertFrom(*source, g_engine->renderer().getPixelFormat());
	return target;
}

void AnimationBase::loadMissingAnimation() {
	// only allow missing animations we know are faulty in the original game
	g_engine->game().missingAnimation(_fileName);

	// otherwise setup a functioning but empty animation
	_isLoaded = true;
	_totalDuration = 1;
	_spriteIndexMapping[0] = 0;
	_spriteOffsets.push_back(1);
	_spriteBases.push_back(0);
	_images.push_back(nullptr);
	_imageOffsets.push_back(Point());
	_frames.push_back({ Point(), Point(), 1 });
}

// unfortunately ScummVMs BLEND_NORMAL does not blend alpha
// but this also bad, let's find/discuss a better solution later
void AnimationBase::fullBlend(const ManagedSurface &source, ManagedSurface &destination, int offsetX, int offsetY) {
	// TODO: Support other pixel formats
	assert(source.format == Graphics::PixelFormat::createFormatRGBA32() ||
	       source.format == Graphics::PixelFormat::createFormatBGRA32());
	assert(destination.format == source.format);
	assert(offsetX >= 0 && offsetX + source.w <= destination.w);
	assert(offsetY >= 0 && offsetY + source.h <= destination.h);

	const byte *sourceLine = (const byte *)source.getPixels();
	byte *destinationLine = (byte *)destination.getPixels() + offsetY * destination.pitch + offsetX * 4;
	for (int y = 0; y < source.h; y++) {
		const byte *sourcePixel = sourceLine;
		byte *destPixel = destinationLine;
		for (int x = 0; x < source.w; x++) {
			byte alpha = sourcePixel[3];
			for (int i = 0; i < 3; i++)
				destPixel[i] = ((byte)(alpha * sourcePixel[i] / 255)) + ((byte)((255 - alpha) * destPixel[i] / 255));
			destPixel[3] = alpha + ((byte)((255 - alpha) * destPixel[3] / 255));
			sourcePixel += 4;
			destPixel += 4;
		}
		sourceLine += source.pitch;
		destinationLine += destination.pitch;
	}
}

Point AnimationBase::imageSize(int32 imageI) const {
	auto image = _images[imageI];
	return image == nullptr ? Point() : Point(image->w, image->h);
}

Animation::Animation(String fileName, AnimationFolder folder)
	: AnimationBase(fileName, folder) {}

void Animation::load() {
	if (_isLoaded)
		return;
	AnimationBase::load();
	Rect maxBounds = maxFrameBounds();
	int16 texWidth = maxBounds.width(), texHeight = maxBounds.height();
	if (g_engine->renderer().requiresPoTTextures()) {
		texWidth = nextHigher2(maxBounds.width());
		texHeight = nextHigher2(maxBounds.height());
	}
	_renderedSurface.create(texWidth, texHeight, g_engine->renderer().getPixelFormat());
	_renderedTexture = g_engine->renderer().createTexture(texWidth, texHeight, true);

	// We always create mipmaps, even for the backgrounds that usually do not scale much,
	// the exception to this is the thumbnails for the savestates.
	// If we need to reduce graphics memory usage in the future, we can change it right here
}

void Animation::freeImages() {
	if (!_isLoaded)
		return;
	AnimationBase::freeImages();
	_renderedSurface.free();
	_renderedTexture.reset(nullptr);
	_renderedFrameI = -1;
	_premultiplyAlpha = 100;
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
	return image == nullptr
		? Rect(imageI < 0 ? Point() : _imageOffsets[imageI], 2, 1)
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

Point Animation::totalFrameOffset(int32 frameI) const {
	const auto &frame = _frames[frameI];
	const auto bounds = frameBounds(frameI);
	return Point(
		bounds.left - frame._center.x + frame._offset.x,
		bounds.top - frame._center.y + frame._offset.y);
}

int32 Animation::frameAtTime(uint32 time) const {
	for (int32 i = 0; (uint)i < _frames.size(); i++) {
		if (time <= _frames[i]._duration)
			return i;
		time -= _frames[i]._duration;
	}
	return -1;
}

void Animation::overrideTexture(const ManagedSurface &surface) {
	int16 texWidth = surface.w, texHeight = surface.h;
	if (g_engine->renderer().requiresPoTTextures()) {
		texWidth = nextHigher2(texWidth);
		texHeight = nextHigher2(texHeight);
	}

	// In order to really use the overridden surface we have to override all
	// values used for calculating the output size
	_renderedFrameI = 0;
	_renderedPremultiplyAlpha = _premultiplyAlpha;
	_renderedSurface.free();
	_renderedSurface.w = texWidth;
	_renderedSurface.h = texHeight;
	_images[0]->free();
	_images[0]->w = surface.w;
	_images[0]->h = surface.h;

	if (_renderedTexture->size() != Point(texWidth, texHeight)) {
		_renderedTexture = Common::move(
			g_engine->renderer().createTexture(texWidth, texHeight, false));
	}
	if (surface.w == texWidth && surface.h == texHeight)
		_renderedTexture->update(surface);
	else {
		ManagedSurface tmpSurface(texWidth, texHeight, g_engine->renderer().getPixelFormat());
		tmpSurface.blitFrom(surface);
		_renderedTexture->update(tmpSurface);
	}
}

void Animation::prerenderFrame(int32 frameI) {
	assert(frameI >= 0 && (uint)frameI < frameCount());
	if (frameI == _renderedFrameI && _renderedPremultiplyAlpha == _premultiplyAlpha)
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
		fullBlend(*image, _renderedSurface, offsetX, offsetY);
	}

	// Here was some alpha premultiplication, but it only produces bugs so is ignored

	_renderedTexture->update(_renderedSurface);
	_renderedFrameI = frameI;
	_renderedPremultiplyAlpha = _premultiplyAlpha;
}

void Animation::outputRect2D(int32 frameI, float scale, Vector2d &topLeft, Vector2d &size) const {
	auto bounds = frameBounds(frameI);
	topLeft += as2D(totalFrameOffset(frameI)) * scale;
	size = Vector2d(bounds.width(), bounds.height()) * scale;
}

void Animation::draw2D(int32 frameI, Vector2d topLeft, float scale, BlendMode blendMode, Color color) {
	prerenderFrame(frameI);
	auto bounds = frameBounds(frameI);
	Vector2d texMin(0, 0);
	Vector2d texMax((float)bounds.width() / _renderedSurface.w, (float)bounds.height() / _renderedSurface.h);

	Vector2d size;
	outputRect2D(frameI, scale, topLeft, size);

	auto &renderer = g_engine->renderer();
	renderer.setTexture(_renderedTexture.get());
	renderer.setBlendMode(blendMode);
	renderer.quad(topLeft, size, color, Angle(), texMin, texMax);
}

void Animation::outputRect3D(int32 frameI, float scale, Vector3d &topLeft, Vector2d &size) const {
	auto bounds = frameBounds(frameI);
	topLeft += as3D(totalFrameOffset(frameI)) * scale;
	topLeft = g_engine->camera().transform3Dto2D(topLeft);
	size = Vector2d(bounds.width(), bounds.height()) * scale * topLeft.z();
}

void Animation::draw3D(int32 frameI, Vector3d topLeft, float scale, BlendMode blendMode, Color color) {
	prerenderFrame(frameI);
	auto bounds = frameBounds(frameI);
	Vector2d texMin(0, 0);
	Vector2d texMax((float)bounds.width() / _renderedSurface.w, (float)bounds.height() / _renderedSurface.h);

	Vector2d size;
	outputRect3D(frameI, scale, topLeft, size);
	const auto rotation = -g_engine->camera().rotation();

	auto &renderer = g_engine->renderer();
	renderer.setTexture(_renderedTexture.get());
	renderer.setBlendMode(blendMode);
	renderer.quad(as2D(topLeft), size, color, rotation, texMin, texMax);
}

void Animation::drawEffect(int32 frameI, Vector3d topLeft, Vector2d size, Vector2d texOffset, BlendMode blendMode) {
	prerenderFrame(frameI);
	auto bounds = frameBounds(frameI);
	Vector2d texMin(0, 0);
	Vector2d texMax((float)bounds.width() / _renderedSurface.w, (float)bounds.height() / _renderedSurface.h);

	topLeft += as3D(totalFrameOffset(frameI));
	topLeft = g_engine->camera().transform3Dto2D(topLeft);
	const auto rotation = -g_engine->camera().rotation();
	size(0, 0) *= bounds.width() * topLeft.z() / _renderedSurface.w;
	size(1, 0) *= bounds.height() * topLeft.z() / _renderedSurface.h;

	auto &renderer = g_engine->renderer();
	renderer.setTexture(_renderedTexture.get());
	renderer.setBlendMode(blendMode);
	renderer.quad(as2D(topLeft), size, kWhite, rotation, texMin + texOffset, texMax + texOffset);
}

Font::Font(String fileName) : AnimationBase(fileName) {}

void Font::load() {
	if (_isLoaded)
		return;
	AnimationBase::load();
	// We now render all frames into a 16x16 atlas and fill up to power of two size just because it is easy here
	// However in two out of three fonts the character 128 is massive, it looks like a bug
	// as we want easy regular-sized characters it is ignored

	Point cellSize;
	for (auto image : _images) {
		assert(image != nullptr); // no fake pictures in fonts please
		if (image == _images[128])
			continue;
		cellSize.x = MAX(cellSize.x, image->w);
		cellSize.y = MAX(cellSize.y, image->h);
	}

	_texMins.resize(_images.size());
	_texMaxs.resize(_images.size());
	ManagedSurface atlasSurface(nextHigher2(cellSize.x * 16), nextHigher2(cellSize.y * 16), g_engine->renderer().getPixelFormat());
	cellSize.x = atlasSurface.w / 16;
	cellSize.y = atlasSurface.h / 16;
	const float invWidth = 1.0f / atlasSurface.w;
	const float invHeight = 1.0f / atlasSurface.h;
	for (uint i = 0; i < _images.size(); i++) {
		if (i == 128) continue;

		int offsetX = (i % 16) * cellSize.x + (cellSize.x - _images[i]->w) / 2;
		int offsetY = (i / 16) * cellSize.y + (cellSize.y - _images[i]->h) / 2;
		fullBlend(*_images[i], atlasSurface, offsetX, offsetY);

		_texMins[i].setX(offsetX * invWidth);
		_texMins[i].setY(offsetY * invHeight);
		_texMaxs[i].setX((offsetX + _images[i]->w) * invWidth);
		_texMaxs[i].setY((offsetY + _images[i]->h) * invHeight);
	}
	_texture = g_engine->renderer().createTexture(atlasSurface.w, atlasSurface.h, false);
	_texture->update(atlasSurface);
	debugCN(1, kDebugGraphics, "Rendered font atlas %s at %dx%d", _fileName.c_str(), atlasSurface.w, atlasSurface.h);
}

void Font::freeImages() {
	if (!_isLoaded)
		return;
	AnimationBase::freeImages();
	_texture.reset();
	_texMins.clear();
	_texMaxs.clear();
}

void Font::drawCharacter(int32 imageI, Point centerPoint, Color color) {
	assert(imageI >= 0 && (uint)imageI < _images.size());
	Vector2d center = as2D(centerPoint + _imageOffsets[imageI]);
	Vector2d size(_images[imageI]->w, _images[imageI]->h);

	auto &renderer = g_engine->renderer();
	renderer.setTexture(_texture.get());
	renderer.setBlendMode(BlendMode::Tinted);
	renderer.quad(center, size, color, Angle(), _texMins[imageI], _texMaxs[imageI]);
}

Graphic::Graphic() {}

Graphic::Graphic(ReadStream &stream) {
	_topLeft.x = stream.readSint16LE();
	_topLeft.y = stream.readSint16LE();
	_scale = stream.readSint16LE();
	_order = stream.readSByte();
	auto animationName = readVarString(stream);
	if (!animationName.empty())
		setAnimation(animationName, AnimationFolder::Animations);
}

Graphic::Graphic(const Graphic &other)
	: _animation(other._animation)
	, _topLeft(other._topLeft)
	, _scale(other._scale)
	, _order(other._order)
	, _color(other._color)
	, _isPaused(other._isPaused)
	, _isLooping(other._isLooping)
	, _lastTime(other._lastTime)
	, _frameI(other._frameI)
	, _depthScale(other._depthScale) {}

Graphic &Graphic::operator= (const Graphic &other) {
	_ownedAnimation.reset();
	_animation = other._animation;
	_topLeft = other._topLeft;
	_scale = other._scale;
	_order = other._order;
	_color = other._color;
	_isPaused = other._isPaused;
	_isLooping = other._isLooping;
	_lastTime = other._lastTime;
	_frameI = other._frameI;
	_depthScale = other._depthScale;
	return *this;
}

void Graphic::loadResources() {
	if (_animation != nullptr)
		_animation->load();
}

void Graphic::freeResources() {
	if (_ownedAnimation == nullptr)
		_animation = nullptr;
	else {
		_ownedAnimation->freeImages();
		_animation = _ownedAnimation.get();
	}
}

void Graphic::update() {
	if (_animation == nullptr || _animation->frameCount() == 0)
		return;

	const uint32 totalDuration = _animation->totalDuration();
	uint32 curTime = _isPaused
		? _lastTime
		: g_engine->getMillis() - _lastTime;
	if (curTime > totalDuration) {
		if (_isLooping && totalDuration > 0)
			curTime %= totalDuration;
		else {
			pause();
			curTime = totalDuration ? totalDuration - 1 : 0;
			_lastTime = curTime;
		}
	}

	_frameI = totalDuration == 0 ? 0 : _animation->frameAtTime(curTime);
	assert(_frameI >= 0);
}

void Graphic::start(bool isLooping) {
	_isPaused = false;
	_isLooping = isLooping;
	_lastTime = g_engine->getMillis();
}

void Graphic::pause() {
	_isPaused = true;
	_isLooping = false;
	_lastTime = g_engine->getMillis() - _lastTime;
}

void Graphic::reset() {
	_frameI = 0;
	_lastTime = _isPaused ? 0 : g_engine->getMillis();
}

void Graphic::setAnimation(const Common::String &fileName, AnimationFolder folder) {
	_ownedAnimation.reset(new Animation(fileName, folder));
	_animation = _ownedAnimation.get();
}

void Graphic::setAnimation(Animation *animation) {
	_animation = animation;
}

void Graphic::syncGame(Serializer &serializer) {
	syncPoint(serializer, _topLeft);
	serializer.syncAsSint16LE(_scale);
	serializer.syncAsUint32LE(_lastTime);
	serializer.syncAsByte(_isPaused);
	serializer.syncAsByte(_isLooping);
	serializer.syncAsFloatLE(_depthScale);
}

static int8 shiftAndClampOrder(int8 order) {
	return MAX<int8>(0, MIN<int8>(kOrderCount - 1, order + kForegroundOrderCount));
}

IDrawRequest::IDrawRequest(int8 order)
	: _order(shiftAndClampOrder(order)) {}

AnimationDrawRequest::AnimationDrawRequest(Graphic &graphic, bool is3D, BlendMode blendMode, float lodBias)
	: IDrawRequest(graphic._order)
	, _is3D(is3D)
	, _animation(&graphic.animation())
	, _frameI(graphic._frameI)
	, _topLeft(graphic._topLeft.x, graphic._topLeft.y, graphic._scale)
	, _scale(graphic._scale * graphic._depthScale)
	, _color(graphic.color())
	, _blendMode(blendMode)
	, _lodBias(lodBias) {
	assert(_frameI >= 0 && (uint)_frameI < _animation->frameCount());
}

AnimationDrawRequest::AnimationDrawRequest(Animation *animation, int32 frameI, Vector2d center, int8 order)
	: IDrawRequest(order)
	, _is3D(false)
	, _animation(animation)
	, _frameI(frameI)
	, _topLeft(as3D(center))
	, _scale(kBaseScale)
	, _color(kWhite)
	, _blendMode(BlendMode::AdditiveAlpha)
	, _lodBias(0.0f) {
	assert(animation != nullptr && animation->isLoaded());
	assert(_frameI >= 0 && (uint)_frameI < _animation->frameCount());
}

void AnimationDrawRequest::draw() {
	g_engine->renderer().setLodBias(_lodBias);
	if (_is3D)
		_animation->draw3D(_frameI, _topLeft, _scale * kInvBaseScale, _blendMode, _color);
	else
		_animation->draw2D(_frameI, as2D(_topLeft), _scale * kInvBaseScale, _blendMode, _color);
}

SpecialEffectDrawRequest::SpecialEffectDrawRequest(Graphic &graphic, Point topLeft, Point bottomRight, Vector2d texOffset, BlendMode blendMode)
	: IDrawRequest(graphic._order)
	, _animation(&graphic.animation())
	, _frameI(graphic._frameI)
	, _topLeft(topLeft.x, topLeft.y, graphic._scale)
	, _size(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y)
	, _texOffset(texOffset)
	, _blendMode(blendMode) {
	assert(_frameI >= 0 && (uint)_frameI < _animation->frameCount());
}

void SpecialEffectDrawRequest::draw() {
	_animation->drawEffect(_frameI, _topLeft, _size, _texOffset, _blendMode);
}

static const byte *trimLeading(const byte *text, const byte *end) {
	while (*text && text < end && *text <= ' ')
		text++;
	return text;
}

static const byte *trimTrailing(const byte *text, const byte *begin, bool trimSpaces) {
	while (text != begin && (*text <= ' ') == trimSpaces)
		text--;
	return text;
}

static Point characterSize(const Font &font, byte ch) {
	if (ch <= ' ' || (uint)(ch - ' ') >= font.imageCount())
		ch = 0;
	else
		ch -= ' ';
	return font.imageSize(ch);
}

TextDrawRequest::TextDrawRequest(Font &font, const char *originalText, Point pos, int maxWidth, bool centered, Color color, int8 order)
	: IDrawRequest(order)
	, _font(font)
	, _color(color) {
	const int screenW = g_system->getWidth();
	const int screenH = g_system->getHeight();
	if (maxWidth < 0)
		maxWidth = screenW;

	// allocate on drawQueue to prevent having destruct it
	assert(originalText != nullptr);
	auto textLen = strlen(originalText);
	char *text = (char *)g_engine->drawQueue().allocator().allocateRaw(textLen + 1, 1);
	memcpy(text, originalText, textLen + 1);

	// split into trimmed lines
	uint lineCount = 0;
	const byte *itChar = (byte *)text, *itLine = (byte *)text, *textEnd = itChar + textLen + 1;
	int lineWidth = 0;
	while (true) {
		if (lineCount >= kMaxLines) {
			g_engine->game().tooManyDialogLines(lineCount, kMaxLines);
			break;
		}

		if (*itChar != '\r' && *itChar)
			lineWidth += characterSize(font, *itChar).x;
		if (lineWidth <= maxWidth && *itChar != '\r' && *itChar) {
			itChar++;
			continue;
		}
		// now we are in new-line territory

		if (*itChar > ' ')
			itChar = trimTrailing(itChar, itLine, false); // trim last word
		if (centered) {
			itChar = trimTrailing(itChar, itLine, true) + 1;
			itLine = trimLeading(itLine, itChar);
			_allLines[lineCount] = TextLine(itLine, itChar - itLine);
		} else
			_allLines[lineCount] = TextLine(itLine, itChar - itLine);
		itChar = trimLeading(itChar, textEnd);
		lineCount++;
		lineWidth = 0;
		itLine = itChar;

		if (!*itChar)
			break;
	}
	_lines = Span<TextLine>(_allLines, lineCount);
	_posX = Span<int>(_allPosX, lineCount);

	// calc line widths and max line width
	_width = 0;
	for (uint i = 0; i < lineCount; i++) {
		lineWidth = 0;
		for (auto ch : _lines[i]) {
			if (ch != '\r' && ch)
				lineWidth += characterSize(font, ch).x;
		}
		_posX[i] = lineWidth;
		_width = MAX(_width, lineWidth);
	}

	// setup line positions
	if (centered) {
		if (pos.x - _width / 2 < 0)
			pos.x = _width / 2 + 1;
		if (pos.x + _width / 2 >= screenW)
			pos.x = screenW - _width / 2 - 1;
		for (auto &linePosX : _posX)
			linePosX = pos.x - linePosX / 2;
	} else
		fill(_posX.begin(), _posX.end(), pos.x);

	// setup height and y position
	_height = (int)lineCount * (font.imageSize(0).y * 4 / 3);
	_posY = pos.y;
	if (centered)
		_posY -= _height / 2;
	if (_posY < 0)
		_posY = 0;
	if (_posY + _height >= screenH)
		_posY = screenH - _height;
}

void TextDrawRequest::draw() {
	const Point spaceSize = _font.imageSize(0);
	Point cursor(0, _posY);
	for (uint i = 0; i < _lines.size(); i++) {
		cursor.x = _posX[i];
		for (auto ch : _lines[i]) {
			const Point charSize = characterSize(_font, ch);
			if (ch > ' ' && (uint)(ch - ' ') < _font.imageCount())
				_font.drawCharacter(ch - ' ', Point(cursor.x, cursor.y), _color);
			cursor.x += charSize.x;
		}
		cursor.y += spaceSize.y * 4 / 3;
	}
}

FadeDrawRequest::FadeDrawRequest(FadeType type, float value, int8 order)
	: IDrawRequest(order)
	, _type(type)
	, _value(value) {}

void FadeDrawRequest::draw() {
	Color color;
	const byte valueAsByte = (byte)(_value * 255);
	switch (_type) {
	case FadeType::ToBlack:
		color = { 0, 0, 0, valueAsByte };
		g_engine->renderer().setBlendMode(BlendMode::AdditiveAlpha);
		break;
	case FadeType::ToWhite:
		color = { valueAsByte, valueAsByte, valueAsByte, valueAsByte };
		g_engine->renderer().setBlendMode(BlendMode::Additive);
		break;
	default:
		g_engine->game().unknownFadeType((int)_type);
		return;
	}
	g_engine->renderer().setTexture(nullptr);
	g_engine->renderer().quad(Vector2d(0, 0), as2D(Point(g_system->getWidth(), g_system->getHeight())), color);
}

struct FadeTask final : public Task {
	FadeTask(Process &process, FadeType fadeType,
		float from, float to,
		uint32 duration, EasingType easingType,
		int8 order,
		PermanentFadeAction permanentFadeAction)
		: Task(process)
		, _fadeType(fadeType)
		, _from(from)
		, _to(to)
		, _duration(duration)
		, _easingType(easingType)
		, _order(order)
		, _permanentFadeAction(permanentFadeAction) {}

	FadeTask(Process &process, Serializer &s)
		: Task(process) {
		FadeTask::syncGame(s);
	}

	TaskReturn run() override {
		TASK_BEGIN;
		if (_permanentFadeAction == PermanentFadeAction::UnsetFaded)
			g_engine->globalUI().isPermanentFaded() = false;
		_startTime = g_engine->getMillis();
		while (g_engine->getMillis() - _startTime < _duration) {
			draw((g_engine->getMillis() - _startTime) / (float)_duration);
			TASK_YIELD(1);
		}
		draw(1.0f); // so that during a loading lag the screen is completly black/white
		if (_permanentFadeAction == PermanentFadeAction::SetFaded)
			g_engine->globalUI().isPermanentFaded() = true;
		TASK_END;
	}

	void debugPrint() override {
		uint32 remaining = g_engine->getMillis() - _startTime <= _duration
			? _duration - (g_engine->getMillis() - _startTime)
			: 0;
		g_engine->console().debugPrintf("Fade (%d) from %.2f to %.2f with %ums remaining\n", (int)_fadeType, _from, _to, remaining);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		syncEnum(s, _fadeType);
		syncEnum(s, _easingType);
		syncEnum(s, _permanentFadeAction);
		s.syncAsFloatLE(_from);
		s.syncAsFloatLE(_to);
		s.syncAsUint32LE(_startTime);
		s.syncAsUint32LE(_duration);
		s.syncAsSByte(_order);
	}

	const char *taskName() const override;

private:
	void draw(float t) {
		g_engine->drawQueue().add<FadeDrawRequest>(_fadeType, _from + (_to - _from) * ease(t, _easingType), _order);
	}

	FadeType _fadeType = {};
	float _from = 0, _to = 0;
	uint32 _startTime = 0, _duration = 0;
	EasingType _easingType = {};
	int8 _order = 0;
	PermanentFadeAction _permanentFadeAction = {};
};
DECLARE_TASK(FadeTask)

Task *fade(Process &process, FadeType fadeType,
	float from, float to,
	int32 duration, EasingType easingType,
	int8 order,
	PermanentFadeAction permanentFadeAction) {
	if (duration <= 0)
		return new DelayTask(process, 0);
	if (!process.isActiveForPlayer())
		return new DelayTask(process, (uint32)duration);
	return new FadeTask(process, fadeType, from, to, duration, easingType, order, permanentFadeAction);
}

BorderDrawRequest::BorderDrawRequest(Rect rect, Color color)
	: IDrawRequest(-kForegroundOrderCount)
	, _rect(rect)
	, _color(color) {}

void BorderDrawRequest::draw() {
	auto &renderer = g_engine->renderer();
	renderer.setTexture(nullptr);
	renderer.setBlendMode(BlendMode::AdditiveAlpha);
	renderer.quad({ (float)_rect.left, (float)_rect.top }, { (float)_rect.width(), (float)_rect.height() }, _color);
}

DrawQueue::DrawQueue(IRenderer *renderer)
	: _renderer(renderer)
	, _allocator(1024) {
	assert(renderer != nullptr);
}

void DrawQueue::clear() {
	_allocator.deallocateAll();
	memset(_requestsPerOrderCount, 0, sizeof(_requestsPerOrderCount));
	memset(_lodBiasPerOrder, 0, sizeof(_lodBiasPerOrder));
}

void DrawQueue::addRequest(IDrawRequest *drawRequest) {
	assert(drawRequest != nullptr && drawRequest->order() >= 0 && drawRequest->order() < kOrderCount);
	auto order = drawRequest->order();
	if (_requestsPerOrderCount[order] < kMaxDrawRequestsPerOrder)
		_requestsPerOrder[order][_requestsPerOrderCount[order]++] = drawRequest;
	else
		g_engine->game().tooManyDrawRequests(order);
}

void DrawQueue::setLodBias(int8 orderFrom, int8 orderTo, float newLodBias) {
	orderFrom = shiftAndClampOrder(orderFrom);
	orderTo = shiftAndClampOrder(orderTo);
	if (orderFrom <= orderTo) {
		Common::fill(_lodBiasPerOrder + orderFrom, _lodBiasPerOrder + orderTo + 1, newLodBias);
	}
}

void DrawQueue::draw() {
	for (int8 order = kOrderCount - 1; order >= 0; order--) {
		_renderer->setLodBias(_lodBiasPerOrder[order]);
		for (uint8 requestI = 0; requestI < _requestsPerOrderCount[order]; requestI++) {
			_requestsPerOrder[order][requestI]->draw();
			_requestsPerOrder[order][requestI]->~IDrawRequest();
		}
	}
	_allocator.deallocateAll();
}

BumpAllocator::BumpAllocator(size_t pageSize) : _pageSize(pageSize) {
	allocatePage();
}

BumpAllocator::~BumpAllocator() {
	for (auto page : _pages)
		free(page);
}

void *BumpAllocator::allocateRaw(size_t size, size_t align) {
	assert(size <= _pageSize);
	uintptr_t page = (uintptr_t)_pages[_pageI];
	uintptr_t top = page + _used;
	top += align - 1;
	top -= top % align;
	if (page + _pageSize - top >= size) {
		_used = top + size - page;
		return (void *)top;
	}

	_used = 0;
	_pageI++;
	if (_pageI >= _pages.size())
		allocatePage();
	return allocateRaw(size, align);
}

void BumpAllocator::allocatePage() {
	auto page = malloc(_pageSize);
	if (page == nullptr)
		error("Out of memory in BumpAllocator");
	_pages.push_back(page);
}

void BumpAllocator::deallocateAll() {
	_pageI = 0;
	_used = 0;
}

}
