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

#ifndef HARVESTER_RUNTIME_ENTITY_H
#define HARVESTER_RUNTIME_ENTITY_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "harvester/art.h"

namespace Graphics {
class Screen;
}

namespace Harvester {

class ResourceManager;

enum RuntimeEntityClass {
	kRuntimeEntityClassObject = 0,
	kRuntimeEntityClassAnimation = 1,
	kRuntimeEntityClassCursor = 2,
	kRuntimeEntityClassBackground = 3,
	kRuntimeEntityClassNpc = 4,
	kRuntimeEntityClassPlayer = 5,
	kRuntimeEntityClassMonster = 6,
	kRuntimeEntityClassRectHotspot = 0x15,
	kRuntimeEntityClassDisabledHotspot = 0x16,
	kRuntimeEntityClassTimer = 0x17,
	kRuntimeEntityClassRectHotspot18 = 0x18,
	kRuntimeEntityClassRectHotspot19 = 0x19
};

enum RuntimeEntityHitTestMode {
	kRuntimeEntityHitTestNone = 0,
	kRuntimeEntityHitTestBounds = 1,
	kRuntimeEntityHitTestOpaquePixels = 2
};

enum RuntimeEntityAnchorMode {
	kRuntimeEntityAnchorTopLeft = 0,
	kRuntimeEntityAnchorCentered = 1
};

class Entity {
public:
	bool loadBitmapResource(ResourceManager &resources, const Common::String &path);
	bool loadAbmResource(ResourceManager &resources, const Common::String &path);

	void setName(const Common::String &name) { _name = name; }
	const Common::String &getName() const { return _name; }

	void setClassId(int classId) { _classId = classId; }
	int getClassId() const { return _classId; }
	void setAnchorMode(RuntimeEntityAnchorMode anchorMode);
	void setZExtent(float zExtent) { _zExtent = zExtent; }
	float getZExtent() const { return _zExtent; }

	void setLooping(bool looping) { _looping = looping; }
	void setPingPong(bool pingPong) { _pingPong = pingPong; }
	void setPlayBackwards(bool playBackwards) { _playBackwards = playBackwards; }
	void setVisible(bool visible) { _visible = visible; }
	bool isVisible() const { return _visible; }

	void setPosition(int x, int y, float z);
	int getX() const { return _x; }
	int getY() const { return _y; }
	float getZ() const { return _z; }

	void setAnimationRate(int rate);
	int getAnimationRate() const { return _animationRate; }
	void setAnimationEnabled(bool enabled);
	bool isAnimationEnabled() const { return _animationEnabled; }
	void setCurrentFrame(int frame);
	int getCurrentFrame() const { return _currentFrame; }
	int getLastFrame() const { return _lastFrame; }
	uint getFrameCount() const { return _frames.size(); }
	bool didAnimationAdvanceLastTick() const { return _animationAdvancedLastTick; }
	void setAnimationFrameRange(int firstFrame, int lastFrame, bool looping);
	void setAnimationSequence(int sequence);
	int getAnimationSequence() const { return _animationSequence; }
	void configureHotspotBounds(int width, int height);
	void setHitTestMode(RuntimeEntityHitTestMode mode) { _hitTestMode = mode; }
	int getBoundsWidth() const { return _boundsWidth; }
	int getBoundsHeight() const { return _boundsHeight; }
	bool getCurrentFrameMetrics(int &width, int &height, int &xOffset, int &yOffset) const;
	bool hasOpaqueFramesInRange(int firstFrame, int lastFrame) const;
	void setDepthScale(float scale);
	float getDepthScale() const { return _depthScale; }
	Common::Rect getScreenRect() const;
	void configureTimerCountdown(int initialValue, int currentValue, bool enabled, bool looping, bool global);
	bool tickTimerState(uint32 now, Common::Array<Common::String> &expiredTimerNames);
	void pauseTimerCountdown(uint32 now);
	void resumeTimerCountdown(uint32 now);
	void setTimerEnabled(bool enabled);
	bool isTimerEnabled() const { return _timerEnabled; }
	bool isTimerLooping() const { return _timerLooping; }
	bool isTimerGlobal() const { return _timerGlobal; }
	int getTimerInitialValue() const { return _timerInitialValue; }
	int getTimerCurrentValue() const { return _timerCurrentValue; }

	bool hasFrames() const { return !_frames.empty(); }
	bool tickVisualState(uint32 now);
	void draw(Graphics::Screen &screen) const;
	bool hitTest(const Common::Point &point) const;
	bool overlapsEntity(const Entity &other) const;
	bool measureCurrentFrameTransparency(uint32 &framePixels, uint32 &transparentPixels,
		uint32 &preservedPixels) const;

private:
	Common::Point getDrawOrigin() const;
	Common::Rect getFrameRect() const;
	bool hasOpaqueFrame() const;
	bool isOpaqueAt(const Common::Point &point) const;
	void advanceAnimationFrame(int directive);
	void updateBoundsFromCurrentFrame();
	void updateScreenBaseFromCurrentFrame();
	void rebuildScaledFrames();

	Common::String _name;
	Common::String _resourcePath;
	Common::Array<AbmFrame> _frames;
	Common::Array<AbmFrame> _baseFrames;
	int _classId = 0;
	int _x = 0;
	int _y = 0;
	float _z = 0.0f;
	int _screenBaseX = 0;
	int _screenBaseY = 0;
	int _currentFrame = -1;
	int _firstFrame = -1;
	int _lastFrame = -1;
	int _animationRate = 0;
	uint32 _animationTickInterval = 0;
	uint32 _nextAnimationTick = 0;
	int _animationSequence = -1;
	bool _looping = true;
	bool _pingPong = false;
	bool _playBackwards = false;
	bool _animationEnabled = true;
	bool _animationAdvancedLastTick = false;
	bool _visible = true;
	bool _drawEnabled = true;
	int _boundsWidth = 0;
	int _boundsHeight = 0;
	RuntimeEntityHitTestMode _hitTestMode = kRuntimeEntityHitTestNone;
	RuntimeEntityAnchorMode _anchorMode = kRuntimeEntityAnchorTopLeft;
	float _zExtent = 0.0f;
	float _depthScale = 1.0f;
	int _timerInitialValue = 0;
	int _timerCurrentValue = 0;
	uint32 _timerStartTick = 0;
	uint32 _timerNextFireTick = 0;
	uint32 _timerPauseTick = 0;
	bool _timerEnabled = false;
	bool _timerLooping = false;
	bool _timerGlobal = false;
	bool _timerPaused = false;
};

class EntityManager {
public:
	explicit EntityManager(ResourceManager &resources);
	~EntityManager();

	void clear();
	void clearSceneEntities(bool preserveGlobalTimers = false);
	Entity *spawnAbmEntityFromResource(const Common::String &name, const Common::String &resourcePath,
		int classId, const Common::Point &position, float z, int animationRate, bool looping, bool pingPong);
	Entity *spawnBitmapEntityFromResource(const Common::String &name, const Common::String &resourcePath,
		int classId, const Common::Point &position, float z);
	Entity *spawnCursorEntity(const Common::Point &position);
	Entity *spawnSceneBitmapEntity(const Common::String &name, const Common::String &resourcePath,
		const Common::Point &position, float z);
	Entity *spawnSceneHotspotEntity(const Common::String &name, const Common::Rect &bounds, float z);
	Entity *spawnSceneAnimationEntity(const Common::String &name, const Common::String &resourcePath,
		const Common::Point &position, float z, int animationRate, bool active, bool visible, bool looping,
		bool playBackwards, bool pingPong, int initialFrame = -1);
	Entity *spawnSceneActorEntity(const Common::String &name, const Common::String &resourcePath,
		const Common::Point &position, float z, int initialFrame);
	Entity *spawnSceneTimerEntity(const Common::String &name, int initialValue, int currentValue,
		bool enabled, bool looping, bool global);
	void configureSceneTimerEntity(Entity &entity, int initialValue, int currentValue,
		bool enabled, bool looping, bool global);
	Entity *getCursorEntity() const { return _cursorEntity; }
	void hideCursor();
	void showCursor();
	void pauseTimerCountdowns();
	void resumeTimerCountdowns();
	bool takeExpiredTimerNames(Common::Array<Common::String> &expiredTimerNames);
	bool tickSceneEntities();
	bool syncCursorEntityPosition(const Common::Point &position);
	void drawSceneEntities(Graphics::Screen &screen) const;
	void drawCursor(Graphics::Screen &screen) const;
	const Entity *findTopSceneEntityAt(const Common::Point &point, int classIdFilter = -1) const;
	int findSceneEntityDrawIndexByName(const Common::String &name) const;
	const Entity *findSceneEntityByName(const Common::String &name) const;
	Entity *findSceneEntityByName(const Common::String &name);
	Entity *detachSceneEntityByName(const Common::String &name);
	void adoptSceneEntity(Entity *entity);
	void reinsertSceneEntity(Entity *entity);

private:
	void insertSceneEntity(Entity *entity);

	ResourceManager &_resources;
	Common::Array<Entity *> _sceneEntities;
	Common::Array<Common::String> _expiredTimerNames;
	Entity *_cursorEntity = nullptr;
	int _timerPauseDepth = 0;
};

} // End of namespace Harvester

#endif // HARVESTER_RUNTIME_ENTITY_H
