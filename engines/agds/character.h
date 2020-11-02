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
#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"

namespace Common	{ class SeekableReadStream; }
namespace Graphics	{ struct Surface; }

namespace AGDS {

class AGDSEngine;
class Process;
class Object;
typedef Common::SharedPtr<Object> ObjectPtr;
class Animation;

class Character {
	AGDSEngine *	_engine;
	ObjectPtr		_object;
	Animation *		_animation;
	Common::String 	_name;
	Common::Point	_pos;
	Common::Point	_dst;
	bool _enabled;
	bool _visible;
	int _phase;
	int _frames;
	int _direction;
	int _movementDirections;

	struct AnimationDescription {
		struct Frame {
			int x, y;
			int w, h;
		};

		Common::String 			filename;
		Common::Array<Frame> 	frames;
	};
	Common::Array<AnimationDescription> _animations;

public:
	Character(AGDSEngine * engine, const Common::String & name, const ObjectPtr & object):
		_engine(engine), _name(name), _object(object), _animation(nullptr), _enabled(true), _visible(true), _phase(-1), _frames(0), _direction(-1), _movementDirections(0) {
	}

	const Common::String & name() const {
		return _name;
	}

	const ObjectPtr & object() const {
		return _object;
	}

	void load(Common::SeekableReadStream* stream);

	void enable(bool enabled = true) {
		_enabled = enabled;
	}

	void visible(bool visible) {
		_visible = visible;
	}

	void animate(Common::Point pos, int frames, int speed);

	void stop() {
		_phase = 0;
		_frames = 0;
	}

	int getPhase() const {
		return _phase;
	}

	void position(Common::Point pos) {
		_pos = pos;
	}

	Common::Point position() const {
		return _pos;
	}

	void moveTo(Common::Point dst, int frames);

	void direction(int dir);

	int direction() const {
		return _direction;
	}

	void paint(Graphics::Surface & backbuffer);

	int getDirectionForMovement(int dx, int dy);
};


} // End of namespace AGDS

#endif /* AGDS_CHARACTER_H */
