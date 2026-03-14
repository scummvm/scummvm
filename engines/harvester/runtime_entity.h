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
#include "harvester/startup_art.h"

namespace Graphics {
class Screen;
}

namespace Harvester {

class ResourceManager;

enum RuntimeEntityClass {
	kRuntimeEntityClassObject = 0,
	kRuntimeEntityClassAnimation = 1,
	kRuntimeEntityClassCursor = 2
};

class RuntimeEntity {
public:
	bool loadBitmapResource(ResourceManager &resources, const Common::String &path);
	bool loadAbmResource(ResourceManager &resources, const Common::String &path);

	void setName(const Common::String &name) { _name = name; }
	const Common::String &getName() const { return _name; }

	void setClassId(int classId) { _classId = classId; }
	int getClassId() const { return _classId; }

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
	void setCurrentFrame(int frame);
	int getCurrentFrame() const { return _currentFrame; }
	int getLastFrame() const { return _lastFrame; }
	void setAnimationSequence(int sequence);
	int getAnimationSequence() const { return _animationSequence; }

	bool hasFrames() const { return !_frames.empty(); }
	bool tickVisualState(uint32 now);
	void draw(Graphics::Screen &screen) const;

private:
	void advanceAnimationFrame(int directive);

	Common::String _name;
	Common::String _resourcePath;
	Common::Array<AbmFrame> _frames;
	int _classId = 0;
	int _x = 0;
	int _y = 0;
	float _z = 0.0f;
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
	bool _visible = true;
};

class RuntimeEntityManager {
public:
	explicit RuntimeEntityManager(ResourceManager &resources);
	~RuntimeEntityManager();

	void clear();
	void clearSceneEntities();
	RuntimeEntity *spawnAbmEntityFromResource(const Common::String &name, const Common::String &resourcePath,
		int classId, const Common::Point &position, float z, int animationRate, bool looping, bool pingPong);
	RuntimeEntity *spawnBitmapEntityFromResource(const Common::String &name, const Common::String &resourcePath,
		int classId, const Common::Point &position, float z);
	RuntimeEntity *spawnCursorEntity(const Common::Point &position);
	RuntimeEntity *spawnSceneBitmapEntity(const Common::String &name, const Common::String &resourcePath,
		const Common::Point &position, float z);
	RuntimeEntity *spawnSceneAnimationEntity(const Common::String &name, const Common::String &resourcePath,
		const Common::Point &position, float z, int animationRate, bool active, bool visible, bool looping,
		bool playBackwards, bool pingPong, int initialFrame);
	RuntimeEntity *getCursorEntity() const { return _cursorEntity; }
	void hideCursor();
	void showCursor();
	bool tickSceneEntities();
	bool syncCursorEntityPosition(const Common::Point &position);
	void drawSceneEntities(Graphics::Screen &screen) const;
	void drawCursor(Graphics::Screen &screen) const;

private:
	ResourceManager &_resources;
	Common::Array<RuntimeEntity *> _sceneEntities;
	RuntimeEntity *_cursorEntity = nullptr;
};

} // End of namespace Harvester

#endif // HARVESTER_RUNTIME_ENTITY_H
