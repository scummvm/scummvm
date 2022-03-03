/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGDS_CHARACTER_H
#define AGDS_CHARACTER_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/rect.h"

namespace Common	{ class SeekableReadStream; class ReadStream; class WriteStream; }
namespace Graphics	{ struct Surface; }

namespace AGDS {

class AGDSEngine;
class Object;
typedef Common::SharedPtr<Object> ObjectPtr;
class Animation;

class Character {
	AGDSEngine *	_engine;
	ObjectPtr		_object;
	Animation *		_animation;
	bool			_jokes;
	Common::String 	_name;
	Common::String	_processName;
	Common::Point	_pos;
	Common::Point	_animationPos;
	bool _enabled;
	bool _visible;
	bool _stopped;
	int _phase;
	int _frames;
	int _direction;
	int _jokesDirection;
	int _movementDirections;

	struct AnimationDescription {
		struct Frame {
			int x, y;
			int w, h;
		};

		Common::String 			filename;
		Common::Array<Frame> 	frames;
	};
	Common::HashMap<int, AnimationDescription> _animations;
	const AnimationDescription * _description;

public:
	Character(AGDSEngine * engine, const Common::String & name):
		_engine(engine), _name(name), _object(), _animation(nullptr), _jokes(false),
		_enabled(true), _visible(true), _stopped(false),
		_phase(-1), _frames(0), _direction(-1), _movementDirections(0) {
	}

	void associate(const Common::String &name);

	const AnimationDescription * animationDescription(uint index) const {
		auto it = _animations.find(index);
		return it != _animations.end()? &it->_value: nullptr;
	}

	const Common::String & name() const {
		return _name;
	}

	const ObjectPtr & object() const {
		return _object;
	}

	void load(Common::SeekableReadStream* stream);
	void loadState(Common::ReadStream* stream);
	void saveState(Common::WriteStream* stream) const;

	void enable(bool enabled = true) {
		_enabled = enabled;
	}

	void visible(bool visible) {
		_visible = visible;
	}

	void animate(const Common::String &processName, Common::Point pos, int direction, int speed);
	void animate(int direction, int speed, bool jokes);

	void stop();

	int getPhase() const {
		return _jokes? _phase: -1;
	}

	void position(Common::Point pos) {
		_pos = pos;
	}

	Common::Point position() const {
		return _pos;
	}

	void moveTo(const Common::String &processName, Common::Point dst, int direction);

	void direction(int dir);

	int direction() const {
		return _jokes? _jokesDirection: _direction;
	}

	void tick();
	void paint(Graphics::Surface & backbuffer, Common::Point pos) const;

	int getDirectionForMovement(Common::Point delta);

	int z() const;
};


} // End of namespace AGDS

#endif /* AGDS_CHARACTER_H */
