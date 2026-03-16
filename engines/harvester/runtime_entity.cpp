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

#include <math.h>

#include "harvester/runtime_entity.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/screen.h"
#include "harvester/detection.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static const char *const kCursorEntityName = "MOUSE";
static const char *const kCursorResourcePath = "1:/GRAPHIC/POINTERS/POINTERS.ABM";
static const float kCursorEntityZ = -100.0f;
static const int kCursorAnimationRate = 10;
static const int kFramesPerSequence = 10;
static const uint32 kAnimationClockDivisorMs = 10;
static const byte kTransparentPaletteIndex = 0;

static int roundToInt(float value) {
	return value >= 0.0f ? (int)floorf(value + 0.5f) : (int)ceilf(value - 0.5f);
}

static int scaleDimension(uint32 value, float scale) {
	return MAX<int>(roundToInt((float)value * scale), 1);
}

static void scaleIndexedBitmapNearest(const IndexedBitmap &source, IndexedBitmap &dest,
		int scaledWidth, int scaledHeight) {
	dest.width = (uint32)scaledWidth;
	dest.height = (uint32)scaledHeight;
	dest.pixels.resize((uint32)scaledWidth * (uint32)scaledHeight);

	for (int y = 0; y < scaledHeight; ++y) {
		const uint32 srcY = MIN<uint32>(((uint32)y * source.height) / (uint32)scaledHeight, source.height - 1);
		for (int x = 0; x < scaledWidth; ++x) {
			const uint32 srcX = MIN<uint32>(((uint32)x * source.width) / (uint32)scaledWidth, source.width - 1);
			dest.pixels[(uint32)y * (uint32)scaledWidth + (uint32)x] =
				source.pixels[srcY * source.width + srcX];
		}
	}
}

static uint32 getAnimationClockTicks() {
	// The original runtime entity animation timer is driven from a centisecond DOS clock.
	return g_system ? (g_system->getMillis() / kAnimationClockDivisorMs) : 0;
}

static void blitAnimationFrame(Graphics::Screen &screen, const Common::Array<AbmFrame> &frames, uint frameIndex,
		int x, int y) {
	if (frameIndex >= frames.size() || !frames[frameIndex].isValid())
		return;

	const AbmFrame &frame = frames[frameIndex];
	int destX = x;
	int destY = y;
	int srcX = 0;
	int srcY = 0;
	int width = (int)frame.width;
	int height = (int)frame.height;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = frame.pixels.data() + srcY * frame.width + srcX;
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, frame.width, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
}

static bool decodeAnimationFrame(const byte *source, uint32 sourceSize, bool compressed, Common::Array<byte> &dest) {
	if (!compressed) {
		if (sourceSize < dest.size())
			return false;

		memcpy(dest.data(), source, dest.size());
		return true;
	}

	uint32 srcOffset = 0;
	uint32 dstOffset = 0;
	while (srcOffset < sourceSize && dstOffset < dest.size()) {
		const byte control = source[srcOffset++];
		if ((control & 0x80) == 0) {
			const uint32 literalCount = MIN<uint32>(control,
				MIN<uint32>(sourceSize - srcOffset, dest.size() - dstOffset));
			memcpy(dest.data() + dstOffset, source + srcOffset, literalCount);
			srcOffset += literalCount;
			dstOffset += literalCount;
		} else {
			if (srcOffset >= sourceSize)
				return false;

			const uint32 repeatCount = MIN<uint32>(control & 0x7f, dest.size() - dstOffset);
			memset(dest.data() + dstOffset, source[srcOffset++], repeatCount);
			dstOffset += repeatCount;
		}
	}

	return dstOffset == dest.size();
}

} // End of anonymous namespace

static bool isRectangleOnlyHitClass(int classId) {
	return classId == kRuntimeEntityClassRectHotspot ||
		classId == kRuntimeEntityClassRectHotspot18 ||
		classId == kRuntimeEntityClassRectHotspot19;
}

bool RuntimeEntity::loadBitmapResource(ResourceManager &resources, const Common::String &path) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12) {
		warning("Harvester: unable to load runtime entity bitmap '%s'", path.c_str());
		return false;
	}

	AbmFrame frame;
	frame.width = READ_LE_UINT32(data.data());
	frame.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = frame.width * frame.height;
	if (frame.width == 0 || frame.height == 0 || data.size() < 12 + pixelCount) {
		warning("Harvester: invalid runtime bitmap '%s'", path.c_str());
		return false;
	}

	frame.pixels.resize(pixelCount);
	memcpy(frame.pixels.data(), data.data() + 12, pixelCount);

	_frames.clear();
	_frames.push_back(frame);
	_baseFrames = _frames;
	_resourcePath = path;
	_currentFrame = 0;
	_firstFrame = 0;
	_lastFrame = 0;
	_animationEnabled = false;
	_drawEnabled = true;
	_depthScale = 1.0f;
	updateBoundsFromCurrentFrame();
	_hitTestMode = kRuntimeEntityHitTestOpaquePixels;
	return true;
}

bool RuntimeEntity::loadAbmResource(ResourceManager &resources, const Common::String &path) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 8) {
		warning("Harvester: unable to load runtime entity animation '%s'", path.c_str());
		return false;
	}

	const uint32 frameCount = READ_LE_UINT32(data.data());
	uint32 offset = 8;
	_frames.resize(frameCount);

	for (uint32 i = 0; i < frameCount; ++i) {
		if (data.size() < offset + 25) {
			warning("Harvester: short runtime ABM header in '%s'", path.c_str());
			return false;
		}

		AbmFrame &frame = _frames[i];
		frame.xOffset = (int32)READ_LE_UINT32(data.data() + offset);
		frame.yOffset = (int32)READ_LE_UINT32(data.data() + offset + 4);
		frame.width = READ_LE_UINT32(data.data() + offset + 8);
		frame.height = READ_LE_UINT32(data.data() + offset + 12);
		const bool compressed = data[offset + 16] != 0;
		const uint32 sourceSize = READ_LE_UINT32(data.data() + offset + 17);
		const uint32 pixelCount = frame.width * frame.height;
		const uint32 payloadOffset = offset + 25;

		if (frame.width == 0 || frame.height == 0 || data.size() < payloadOffset + sourceSize) {
			warning("Harvester: invalid runtime ABM frame %u in '%s'", i, path.c_str());
			return false;
		}

		frame.pixels.resize(pixelCount);
		if (!decodeAnimationFrame(data.data() + payloadOffset, sourceSize, compressed, frame.pixels))
			return false;

		offset = payloadOffset + sourceSize;
	}

	_resourcePath = path;
	_baseFrames = _frames;
	_currentFrame = _frames.empty() ? -1 : 0;
	_firstFrame = _currentFrame;
	_lastFrame = _frames.empty() ? -1 : (int)_frames.size() - 1;
	_animationEnabled = !_frames.empty();
	_drawEnabled = true;
	_depthScale = 1.0f;
	updateBoundsFromCurrentFrame();
	_hitTestMode = kRuntimeEntityHitTestOpaquePixels;
	return true;
}

void RuntimeEntity::setPosition(int x, int y, float z) {
	_x = x;
	_y = y;
	_z = z;
}

void RuntimeEntity::setAnimationRate(int rate) {
	if (rate == 0)
		_animationTickInterval = 0;
	else
		_animationTickInterval = 100 / rate;

	if (rate != _animationRate) {
		_nextAnimationTick = 0;
		_animationRate = rate;
	}
}

void RuntimeEntity::setCurrentFrame(int frame) {
	if (_frames.empty())
		return;

	advanceAnimationFrame(frame);
}

void RuntimeEntity::setAnimationFrameRange(int firstFrame, int lastFrame, bool looping) {
	if (_frames.empty())
		return;

	firstFrame = CLIP<int>(firstFrame, 0, (int)_frames.size() - 1);
	lastFrame = CLIP<int>(lastFrame, 0, (int)_frames.size() - 1);
	if (lastFrame < firstFrame)
		SWAP(firstFrame, lastFrame);

	_animationSequence = -1;
	_looping = looping;
	_playBackwards = false;
	_firstFrame = firstFrame;
	_lastFrame = lastFrame;
	_animationEnabled = firstFrame != lastFrame;
	if (_currentFrame < _firstFrame || _currentFrame > _lastFrame)
		advanceAnimationFrame(_firstFrame);
	else
		updateBoundsFromCurrentFrame();
}

void RuntimeEntity::setAnimationSequence(int sequence) {
	if (_frames.empty() || sequence == _animationSequence)
		return;

	_animationSequence = sequence;
	_looping = true;
	_playBackwards = false;
	_animationEnabled = true;
	_firstFrame = MIN<int>(sequence * kFramesPerSequence, (int)_frames.size() - 1);
	_lastFrame = MIN<int>(_firstFrame + kFramesPerSequence - 1, (int)_frames.size() - 1);
	advanceAnimationFrame(_firstFrame);

	if (_classId == kRuntimeEntityClassCursor) {
		debugC(1, kDebugGeneral,
			"Harvester: cursor animation sequence=%d frames=%d..%d current=%d",
			_animationSequence, _firstFrame, _lastFrame, _currentFrame);
	}
}

void RuntimeEntity::configureHotspotBounds(int width, int height) {
	_frames.clear();
	_baseFrames.clear();
	_currentFrame = -1;
	_firstFrame = -1;
	_lastFrame = -1;
	_animationEnabled = false;
	_drawEnabled = false;
	_boundsWidth = MAX(width, 1);
	_boundsHeight = MAX(height, 1);
	_hitTestMode = kRuntimeEntityHitTestBounds;
	_depthScale = 1.0f;
}

bool RuntimeEntity::getCurrentFrameMetrics(int &width, int &height, int &xOffset, int &yOffset) const {
	if (_frames.empty() || _currentFrame < 0 || (uint)_currentFrame >= _frames.size())
		return false;

	const AbmFrame &frame = _frames[(uint)_currentFrame];
	width = (int)frame.width;
	height = (int)frame.height;
	xOffset = frame.xOffset;
	yOffset = frame.yOffset;
	return true;
}

void RuntimeEntity::setDepthScale(float scale) {
	if (_frames.empty())
		return;

	const float newScale = scale > 0.0f ? scale : 1.0f;
	if (fabsf(_depthScale - newScale) < 0.0001f)
		return;

	if (_baseFrames.empty())
		_baseFrames = _frames;

	_depthScale = newScale;
	rebuildScaledFrames();
}

bool RuntimeEntity::tickVisualState(uint32 now) {
	if (!_animationEnabled || _currentFrame < 0 || _animationTickInterval == 0)
		return false;
	if (now < _nextAnimationTick)
		return false;

	bool advanced = false;
	do {
		advanceAnimationFrame(_playBackwards ? -1 : -2);
		_nextAnimationTick = now + _animationTickInterval;
		advanced = true;
	} while (_animationTickInterval != 0 && now >= _nextAnimationTick);

	return advanced;
}

Common::Point RuntimeEntity::getDrawOrigin() const {
	if (!_frames.empty() && _currentFrame >= 0 && (uint)_currentFrame < _frames.size()) {
		const AbmFrame &frame = _frames[(uint)_currentFrame];
		int drawX = _x;
		int drawY = _y;
		if (_anchorMode == kRuntimeEntityAnchorCentered) {
			drawX -= (int)frame.width / 2;
			drawY -= (int)frame.height / 2;
		}

		return Common::Point(drawX + frame.xOffset, drawY + frame.yOffset);
	}

	int drawX = _x;
	int drawY = _y;
	if (_anchorMode == kRuntimeEntityAnchorCentered) {
		drawX -= _boundsWidth / 2;
		drawY -= _boundsHeight / 2;
	}

	return Common::Point(drawX, drawY);
}

Common::Rect RuntimeEntity::getScreenRect() const {
	return getFrameRect();
}

void RuntimeEntity::draw(Graphics::Screen &screen) const {
	if (!_visible || !_drawEnabled || _currentFrame < 0)
		return;

	const Common::Point drawOrigin = getDrawOrigin();
	blitAnimationFrame(screen, _frames, _currentFrame, drawOrigin.x, drawOrigin.y);
}

Common::Rect RuntimeEntity::getFrameRect() const {
	const Common::Point drawOrigin = getDrawOrigin();
	if (!_frames.empty() && _currentFrame >= 0 && (uint)_currentFrame < _frames.size()) {
		const AbmFrame &frame = _frames[(uint)_currentFrame];
		return Common::Rect(drawOrigin.x, drawOrigin.y,
			drawOrigin.x + frame.width, drawOrigin.y + frame.height);
	}

	return Common::Rect(drawOrigin.x, drawOrigin.y, drawOrigin.x + _boundsWidth, drawOrigin.y + _boundsHeight);
}

bool RuntimeEntity::hasOpaqueFrame() const {
	return !_frames.empty() && _currentFrame >= 0 && (uint)_currentFrame < _frames.size();
}

bool RuntimeEntity::isOpaqueAt(const Common::Point &point) const {
	if (!hasOpaqueFrame())
		return false;

	const Common::Rect bounds = getFrameRect();
	if (!bounds.contains(point))
		return false;

	const AbmFrame &frame = _frames[(uint)_currentFrame];
	const int relativeX = point.x - bounds.left;
	const int relativeY = point.y - bounds.top;
	if (relativeX < 0 || relativeY < 0 || relativeX >= (int)frame.width || relativeY >= (int)frame.height)
		return false;

	return frame.pixels[(uint)relativeY * frame.width + (uint)relativeX] != 0;
}

bool RuntimeEntity::hitTest(const Common::Point &point) const {
	if (!_visible)
		return false;
	if (_classId == kRuntimeEntityClassDisabledHotspot)
		return false;
	if (_classId == kRuntimeEntityClassBackground)
		return true;
	if (_hitTestMode == kRuntimeEntityHitTestNone)
		return false;

	const Common::Rect bounds = getFrameRect();
	if (!bounds.contains(point))
		return false;
	if (_hitTestMode != kRuntimeEntityHitTestOpaquePixels || isRectangleOnlyHitClass(_classId))
		return true;

	return isOpaqueAt(point);
}

bool RuntimeEntity::overlapsEntity(const RuntimeEntity &other) const {
	if (this == &other || !_visible || !other._visible)
		return false;
	if (!hasOpaqueFrame() || !other.hasOpaqueFrame())
		return false;
	if (_classId == kRuntimeEntityClassCursor || _classId == kRuntimeEntityClassBackground ||
		_classId == kRuntimeEntityClassRectHotspot || _classId == kRuntimeEntityClassRectHotspot19 ||
		other._classId == kRuntimeEntityClassCursor || other._classId == kRuntimeEntityClassBackground ||
		other._classId == kRuntimeEntityClassRectHotspot || other._classId == kRuntimeEntityClassRectHotspot19) {
		return false;
	}

	const float thisZMax = _z + _zExtent;
	const float otherZMax = other._z + other._zExtent;
	if (thisZMax < other._z || otherZMax < _z)
		return false;

	const Common::Rect thisRect = getFrameRect();
	const Common::Rect otherRect = other.getFrameRect();
	const int left = MAX(thisRect.left, otherRect.left);
	const int top = MAX(thisRect.top, otherRect.top);
	const int right = MIN(thisRect.right, otherRect.right);
	const int bottom = MIN(thisRect.bottom, otherRect.bottom);
	if (left >= right || top >= bottom)
		return false;

	for (int y = top; y < bottom; ++y) {
		for (int x = left; x < right; ++x) {
			const Common::Point point(x, y);
			if (isOpaqueAt(point) && other.isOpaqueAt(point))
				return true;
		}
	}

	return false;
}

bool RuntimeEntity::measureCurrentFrameTransparency(uint32 &framePixels, uint32 &transparentPixels,
		uint32 &preservedPixels) const {
	framePixels = 0;
	transparentPixels = 0;
	preservedPixels = 0;

	if (!hasOpaqueFrame())
		return false;

	const AbmFrame &frame = _frames[(uint)_currentFrame];
	framePixels = frame.width * frame.height;
	Common::Array<byte> opaqueMask;
	opaqueMask.resize(framePixels);
	for (uint32 i = 0; i < framePixels; ++i)
		opaqueMask[i] = frame.pixels[i] == kTransparentPaletteIndex ? 0 : 1;

	Common::Array<byte> probeSurface;
	probeSurface.resize(framePixels);
	memset(probeSurface.data(), 0x7f, probeSurface.size());
	Graphics::keyBlit(probeSurface.data(), opaqueMask.data(), frame.width, frame.width,
		frame.width, frame.height, 1, kTransparentPaletteIndex);

	for (uint32 i = 0; i < framePixels; ++i) {
		if (frame.pixels[i] == kTransparentPaletteIndex)
			++transparentPixels;
		if (probeSurface[i] == 0x7f)
			++preservedPixels;
	}

	return true;
}

void RuntimeEntity::advanceAnimationFrame(int directive) {
	if (_frames.empty())
		return;

	if (directive == -1) {
		--_currentFrame;
		if (_currentFrame >= _firstFrame)
			goto done;

		if (!_looping) {
			_currentFrame = _firstFrame;
			_animationEnabled = false;
			goto done;
		}

		if (!_pingPong) {
			_currentFrame = _lastFrame;
			goto done;
		}

		_playBackwards = false;
		_currentFrame = _firstFrame;
		goto done;
	}

	if (directive == -2) {
		++_currentFrame;
		if (_currentFrame <= _lastFrame)
			goto done;

		if (!_looping) {
			_currentFrame = _lastFrame;
			_animationEnabled = false;
			goto done;
		}

		if (_pingPong) {
			_playBackwards = true;
			_currentFrame = _lastFrame;
			goto done;
		}

		_currentFrame = _firstFrame;
		goto done;
	}

	_currentFrame = CLIP<int>(directive, 0, (int)_frames.size() - 1);

done:
	updateBoundsFromCurrentFrame();
}

void RuntimeEntity::updateBoundsFromCurrentFrame() {
	if (_frames.empty()) {
		_boundsWidth = 0;
		_boundsHeight = 0;
		return;
	}

	const int frameIndex = (_currentFrame >= 0 && (uint)_currentFrame < _frames.size()) ? _currentFrame : 0;
	_boundsWidth = (int)_frames[(uint)frameIndex].width;
	_boundsHeight = (int)_frames[(uint)frameIndex].height;
}

void RuntimeEntity::rebuildScaledFrames() {
	if (_baseFrames.empty()) {
		_frames.clear();
		updateBoundsFromCurrentFrame();
		return;
	}

	if (fabsf(_depthScale - 1.0f) < 0.0001f) {
		_frames = _baseFrames;
		updateBoundsFromCurrentFrame();
		return;
	}

	_frames.resize(_baseFrames.size());
	for (uint i = 0; i < _baseFrames.size(); ++i) {
		const AbmFrame &source = _baseFrames[i];
		AbmFrame &scaled = _frames[i];
		const int scaledWidth = scaleDimension(source.width, _depthScale);
		const int scaledHeight = scaleDimension(source.height, _depthScale);

		scaleIndexedBitmapNearest(source, scaled, scaledWidth, scaledHeight);
		scaled.xOffset = roundToInt((float)source.xOffset * _depthScale);
		scaled.yOffset = roundToInt((float)source.yOffset * _depthScale);
	}

	updateBoundsFromCurrentFrame();
}

RuntimeEntityManager::RuntimeEntityManager(ResourceManager &resources) : _resources(resources) {
}

RuntimeEntityManager::~RuntimeEntityManager() {
	clear();
}

void RuntimeEntityManager::clear() {
	clearSceneEntities();
	delete _cursorEntity;
	_cursorEntity = nullptr;
}

void RuntimeEntityManager::clearSceneEntities() {
	for (RuntimeEntity *entity : _sceneEntities)
		delete entity;
	_sceneEntities.clear();
}

RuntimeEntity *RuntimeEntityManager::spawnAbmEntityFromResource(const Common::String &name,
		const Common::String &resourcePath, int classId, const Common::Point &position, float z,
		int animationRate, bool looping, bool pingPong) {
	RuntimeEntity *entity = new RuntimeEntity();
	if (!entity->loadAbmResource(_resources, resourcePath)) {
		delete entity;
		return nullptr;
	}

	entity->setName(name);
	entity->setClassId(classId);
	entity->setPosition(position.x, position.y, z);
	entity->setLooping(looping);
	entity->setPingPong(pingPong);
	entity->setAnimationRate(animationRate);
	return entity;
}

RuntimeEntity *RuntimeEntityManager::spawnBitmapEntityFromResource(const Common::String &name,
		const Common::String &resourcePath, int classId, const Common::Point &position, float z) {
	RuntimeEntity *entity = new RuntimeEntity();
	if (!entity->loadBitmapResource(_resources, resourcePath)) {
		delete entity;
		return nullptr;
	}

	entity->setName(name);
	entity->setClassId(classId);
	entity->setPosition(position.x, position.y, z);
	return entity;
}

RuntimeEntity *RuntimeEntityManager::spawnCursorEntity(const Common::Point &position) {
	if (_cursorEntity)
		return _cursorEntity;

	_cursorEntity = spawnAbmEntityFromResource(kCursorEntityName, kCursorResourcePath,
		kRuntimeEntityClassCursor, position, kCursorEntityZ, kCursorAnimationRate, true, false);
	if (_cursorEntity)
		_cursorEntity->setAnimationSequence(0);
	if (_cursorEntity)
		_cursorEntity->setHitTestMode(kRuntimeEntityHitTestNone);
	if (_cursorEntity) {
		const uint32 animationInterval = _cursorEntity->getAnimationRate() == 0 ? 0 :
			(100U / (uint32)_cursorEntity->getAnimationRate());
		debugC(1, kDebugGeneral,
			"Harvester: spawned cursor entity rate=%d intervalTicks=%u clock=centiseconds frame=%d..%d pos=(%d,%d)",
			_cursorEntity->getAnimationRate(), animationInterval, _cursorEntity->getCurrentFrame(),
			_cursorEntity->getLastFrame(), position.x, position.y);
	}

	return _cursorEntity;
}

RuntimeEntity *RuntimeEntityManager::spawnSceneBitmapEntity(const Common::String &name,
		const Common::String &resourcePath, const Common::Point &position, float z) {
	RuntimeEntity *entity = spawnBitmapEntityFromResource(name, resourcePath, kRuntimeEntityClassObject,
		position, z);
	if (entity)
		insertSceneEntity(entity);
	return entity;
}

RuntimeEntity *RuntimeEntityManager::spawnSceneHotspotEntity(const Common::String &name,
		const Common::Rect &bounds, float z) {
	RuntimeEntity *entity = new RuntimeEntity();
	entity->setName(name);
	entity->setClassId(kRuntimeEntityClassObject);
	entity->setPosition(bounds.left, bounds.top, z);
	entity->configureHotspotBounds(bounds.width(), bounds.height());
	insertSceneEntity(entity);
	return entity;
}

RuntimeEntity *RuntimeEntityManager::spawnSceneAnimationEntity(const Common::String &name,
		const Common::String &resourcePath, const Common::Point &position, float z, int animationRate,
		bool active, bool visible, bool looping, bool playBackwards, bool pingPong) {
	RuntimeEntity *entity = spawnAbmEntityFromResource(name, resourcePath, kRuntimeEntityClassAnimation,
		position, z, animationRate, looping, pingPong);
	if (!entity)
		return nullptr;

	entity->setAnchorMode(kRuntimeEntityAnchorCentered);
	entity->setZExtent(3.0f);
	entity->setVisible(visible);
	entity->setPlayBackwards(playBackwards);
	entity->setHitTestMode(kRuntimeEntityHitTestNone);
	if (playBackwards)
		entity->setCurrentFrame(entity->getLastFrame());
	else
		entity->setCurrentFrame(0);

	if (!active && !visible)
		entity->setVisible(false);

	insertSceneEntity(entity);
	return entity;
}

RuntimeEntity *RuntimeEntityManager::spawnSceneActorEntity(const Common::String &name,
		const Common::String &resourcePath, const Common::Point &position, float z, int initialFrame) {
	RuntimeEntity *entity = spawnAbmEntityFromResource(name, resourcePath, kRuntimeEntityClassObject,
		position, z, 0, false, false);
	if (!entity)
		return nullptr;

	entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
	entity->setZExtent(3.0f);
	entity->setHitTestMode(kRuntimeEntityHitTestOpaquePixels);
	entity->setCurrentFrame(initialFrame);
	debugC(1, kDebugScene,
		"Harvester: spawned scene actor '%s' resource='%s' frames=0..%d initial_frame=%d animation_rate=%d sequence=%d pos=(%d,%d,z=%.2f)",
		name.c_str(), resourcePath.c_str(), entity->getLastFrame(), initialFrame,
		entity->getAnimationRate(), entity->getAnimationSequence(),
		position.x, position.y, (double)z);
	insertSceneEntity(entity);
	return entity;
}

void RuntimeEntityManager::hideCursor() {
	if (_cursorEntity)
		_cursorEntity->setVisible(false);
}

void RuntimeEntityManager::showCursor() {
	if (_cursorEntity)
		_cursorEntity->setVisible(true);
}

bool RuntimeEntityManager::tickSceneEntities() {
	const uint32 now = getAnimationClockTicks();
	bool changed = false;

	for (RuntimeEntity *entity : _sceneEntities)
		changed |= entity->tickVisualState(now);

	return changed;
}

bool RuntimeEntityManager::syncCursorEntityPosition(const Common::Point &position) {
	if (!_cursorEntity)
		return false;

	const bool moved = _cursorEntity->getX() != position.x || _cursorEntity->getY() != position.y;
	_cursorEntity->setPosition(position.x, position.y, kCursorEntityZ);
	return _cursorEntity->tickVisualState(getAnimationClockTicks()) || moved;
}

void RuntimeEntityManager::drawCursor(Graphics::Screen &screen) const {
	if (_cursorEntity)
		_cursorEntity->draw(screen);
}

void RuntimeEntityManager::drawSceneEntities(Graphics::Screen &screen) const {
	for (RuntimeEntity *entity : _sceneEntities)
		entity->draw(screen);
}

const RuntimeEntity *RuntimeEntityManager::findTopSceneEntityAt(const Common::Point &point, int classIdFilter) const {
	for (int i = (int)_sceneEntities.size() - 1; i >= 0; --i) {
		const RuntimeEntity *entity = _sceneEntities[(uint)i];
		if (classIdFilter >= 0 && entity->getClassId() != classIdFilter)
			continue;
		if (entity->hitTest(point))
			return entity;
	}

	return nullptr;
}

const RuntimeEntity *RuntimeEntityManager::findSceneEntityByName(const Common::String &name) const {
	for (const RuntimeEntity *entity : _sceneEntities) {
		if (entity->getName().equalsIgnoreCase(name))
			return entity;
	}

	return nullptr;
}

RuntimeEntity *RuntimeEntityManager::findSceneEntityByName(const Common::String &name) {
	for (RuntimeEntity *entity : _sceneEntities) {
		if (entity->getName().equalsIgnoreCase(name))
			return entity;
	}

	return nullptr;
}

RuntimeEntity *RuntimeEntityManager::detachSceneEntityByName(const Common::String &name) {
	for (uint i = 0; i < _sceneEntities.size(); ++i) {
		RuntimeEntity *entity = _sceneEntities[i];
		if (!entity->getName().equalsIgnoreCase(name))
			continue;

		_sceneEntities.remove_at(i);
		return entity;
	}

	return nullptr;
}

void RuntimeEntityManager::adoptSceneEntity(RuntimeEntity *entity) {
	if (!entity)
		return;

	insertSceneEntity(entity);
}

void RuntimeEntityManager::insertSceneEntity(RuntimeEntity *entity) {
	uint insertIndex = 0;
	while (insertIndex < _sceneEntities.size() && _sceneEntities[insertIndex]->getZ() >= entity->getZ())
		++insertIndex;
	_sceneEntities.insert_at(insertIndex, entity);
}

} // End of namespace Harvester
