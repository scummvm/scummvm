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

#include "harvester/runtime_entity.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/screen.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static const char *const kCursorEntityName = "MOUSE";
static const char *const kCursorResourcePath = "1:/GRAPHIC/POINTERS/POINTERS.ABM";
static const float kCursorEntityZ = -100.0f;
static const int kCursorAnimationRate = 10;
static const int kFramesPerSequence = 10;

static void blitAnimationFrame(Graphics::Screen &screen, const Common::Array<AbmFrame> &frames, uint frameIndex,
		int x, int y) {
	if (frameIndex >= frames.size() || !frames[frameIndex].isValid())
		return;

	const AbmFrame &frame = frames[frameIndex];
	screen.copyRectToSurface(frame.pixels.data(), frame.width, x + frame.xOffset, y + frame.yOffset,
		frame.width, frame.height);
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
	_resourcePath = path;
	_currentFrame = 0;
	_firstFrame = 0;
	_lastFrame = 0;
	_animationEnabled = false;
	_drawEnabled = true;
	_boundsWidth = frame.width;
	_boundsHeight = frame.height;
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
	_currentFrame = _frames.empty() ? -1 : 0;
	_firstFrame = _currentFrame;
	_lastFrame = _frames.empty() ? -1 : (int)_frames.size() - 1;
	_animationEnabled = !_frames.empty();
	_drawEnabled = true;
	_boundsWidth = _frames.empty() ? 0 : _frames[0].width;
	_boundsHeight = _frames.empty() ? 0 : _frames[0].height;
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
}

void RuntimeEntity::configureHotspotBounds(int width, int height) {
	_frames.clear();
	_currentFrame = -1;
	_firstFrame = -1;
	_lastFrame = -1;
	_animationEnabled = false;
	_drawEnabled = false;
	_boundsWidth = MAX(width, 1);
	_boundsHeight = MAX(height, 1);
	_hitTestMode = kRuntimeEntityHitTestBounds;
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

void RuntimeEntity::draw(Graphics::Screen &screen) const {
	if (!_visible || !_drawEnabled || _currentFrame < 0)
		return;

	blitAnimationFrame(screen, _frames, _currentFrame, _x, _y);
}

Common::Rect RuntimeEntity::getFrameRect() const {
	if (!_frames.empty() && _currentFrame >= 0 && (uint)_currentFrame < _frames.size()) {
		const AbmFrame &frame = _frames[(uint)_currentFrame];
		return Common::Rect(_x + frame.xOffset, _y + frame.yOffset,
			_x + frame.xOffset + frame.width, _y + frame.yOffset + frame.height);
	}

	return Common::Rect(_x, _y, _x + _boundsWidth, _y + _boundsHeight);
}

bool RuntimeEntity::hitTest(const Common::Point &point) const {
	if (_hitTestMode == kRuntimeEntityHitTestNone)
		return false;

	const Common::Rect bounds = getFrameRect();
	if (!bounds.contains(point))
		return false;
	if (_hitTestMode != kRuntimeEntityHitTestOpaquePixels)
		return true;
	if (_frames.empty() || _currentFrame < 0 || (uint)_currentFrame >= _frames.size())
		return false;

	const AbmFrame &frame = _frames[(uint)_currentFrame];
	const int relativeX = point.x - bounds.left;
	const int relativeY = point.y - bounds.top;
	if (relativeX < 0 || relativeY < 0 || relativeX >= (int)frame.width || relativeY >= (int)frame.height)
		return false;

	return frame.pixels[(uint)relativeY * frame.width + (uint)relativeX] != 0;
}

void RuntimeEntity::advanceAnimationFrame(int directive) {
	if (_frames.empty())
		return;

	if (directive == -1) {
		--_currentFrame;
		if (_currentFrame >= _firstFrame)
			return;

		if (!_looping) {
			_currentFrame = _firstFrame;
			if (_classId == kRuntimeEntityClassAnimation)
				_animationEnabled = false;
			return;
		}

		if (!_pingPong) {
			_currentFrame = _lastFrame;
			return;
		}

		_playBackwards = false;
		_currentFrame = _firstFrame;
		return;
	}

	if (directive == -2) {
		++_currentFrame;
		if (_currentFrame <= _lastFrame)
			return;

		if (!_looping) {
			_currentFrame = _lastFrame;
			if (_classId == kRuntimeEntityClassAnimation)
				_animationEnabled = false;
			return;
		}

		if (_pingPong) {
			_playBackwards = true;
			_currentFrame = _lastFrame;
			return;
		}

		_currentFrame = _firstFrame;
		return;
	}

	_currentFrame = CLIP<int>(directive, 0, (int)_frames.size() - 1);
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

void RuntimeEntityManager::hideCursor() {
	if (_cursorEntity)
		_cursorEntity->setVisible(false);
}

void RuntimeEntityManager::showCursor() {
	if (_cursorEntity)
		_cursorEntity->setVisible(true);
}

bool RuntimeEntityManager::tickSceneEntities() {
	const uint32 now = g_system->getMillis();
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
	return _cursorEntity->tickVisualState(g_system->getMillis()) || moved;
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

void RuntimeEntityManager::insertSceneEntity(RuntimeEntity *entity) {
	uint insertIndex = 0;
	while (insertIndex < _sceneEntities.size() && _sceneEntities[insertIndex]->getZ() >= entity->getZ())
		++insertIndex;
	_sceneEntities.insert_at(insertIndex, entity);
}

} // End of namespace Harvester
