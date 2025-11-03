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

#ifndef CHARACTER_H
#define CHARACTER_H

#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
class ReadStream;
class WriteStream;
} // namespace Common
namespace Graphics {
struct Surface;
class ManagedSurface;
} // namespace Graphics

namespace AGDS {

class AGDSEngine;
class Object;
using ObjectPtr = Common::SharedPtr<Object>;
class Animation;
using AnimationPtr = Common::SharedPtr<Animation>;

class Character {
	using FogPtr = Common::ScopedPtr<Graphics::ManagedSurface>;
	AGDSEngine *_engine;
	ObjectPtr _object;
	AnimationPtr _animation;
	FogPtr _fog;
	bool _jokes;
	Common::String _name;
	Common::String _processName;
	Common::Point _pos;
	Common::Point _animationPos;
	bool _enabled;
	bool _visible;
	bool _stopped;
	bool _shown;
	int _phase;
	int _frames;
	int _direction;
	int _jokesDirection;
	int _movementDirections;
	int _fogMinZ, _fogMaxZ;

	struct AnimationDescription {
		struct Frame {
			int x, y;
			uint w, h;
		};

		Common::String filename;
		Common::Array<Frame> frames;
	};
	Common::HashMap<uint, AnimationDescription> _animations;
	const AnimationDescription *_description;

	bool animate(int direction, int speed, bool jokes);
	Common::Point animationPosition() const;

public:
	Character(AGDSEngine *engine, const Common::String &name);
	~Character();

	void associate(const Common::String &name);

	const AnimationDescription *animationDescription(uint index) const {
		auto it = _animations.find(index);
		return it != _animations.end() ? &it->_value : nullptr;
	}

	const Common::String &name() const {
		return _name;
	}

	const ObjectPtr &object() const {
		return _object;
	}

	void load(Common::SeekableReadStream &stream);
	void loadState(Common::ReadStream &stream);
	void saveState(Common::WriteStream &stream) const;

	void enable(bool enabled = true) {
		_enabled = enabled;
	}

	void visible(bool visible);
	bool visible() const {
		return _visible && _shown;
	}

	bool active() const {
		return _enabled && _visible;
	}

	bool animate(Common::Point pos, int direction, int speed);

	void stop();
	void leave(const Common::String &processName);

	int phase() const {
		return _jokes ? _phase : -1;
	}
	void phase(int phase) {
		_phase = phase;
	}

	void position(Common::Point pos) {
		_pos = pos;
	}

	Common::Point position() const {
		return _pos;
	}
	bool pointIn(Common::Point pos) const;

	void notifyProcess(const Common::String &processName);
	bool moveTo(const Common::String &processName, Common::Point dst, int direction);
	void pointTo(const Common::String &processName, Common::Point dst);

	bool direction(int dir);

	int direction() const {
		return _jokes ? _jokesDirection : _direction;
	}

	void tick(bool reactivate);
	void paint(Graphics::Surface &backbuffer, Common::Point pos) const;

	int getDirectionForMovement(Common::Point delta);

	int z() const;

	void reset();
	void setFog(Graphics::ManagedSurface *surface, int minZ, int maxZ);
};

} // End of namespace AGDS

#endif /* AGDS_CHARACTER_H */
