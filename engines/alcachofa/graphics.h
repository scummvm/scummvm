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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common/ptr.h"
#include "common/stream.h"
#include "common/serializer.h"
#include "common/rect.h"

namespace Alcachofa {

enum class CursorType {
	Normal,
	LookAt,
	Use,
	GoTo,
	LeaveUp,
	LeaveRight,
	LeaveDown,
	LeaveLeft
};

enum class Direction {
	Up,
	Down,
	Left,
	Right
};

constexpr const int32 kDirectionCount = 4;

enum class AnimationFolder {
	Animations,
	Maskaras,
	Fondos
};

class Animation {
public:
	Animation(Common::String fileName, AnimationFolder folder = AnimationFolder::Animations);

private:
	Common::String _fileName;
	AnimationFolder _folder;
};

class Graphic {
public:
	Graphic();
	Graphic(Common::ReadStream &stream);

	inline int8 order() const { return _order; }

	void start(bool looping);
	void stop();
	void serializeSave(Common::Serializer &serializer);

public:
	Common::SharedPtr<Animation> _animation;
	Common::Point _center;
	int16 _scale = 300;
	int8 _order = 0;

private:
	bool _isPaused = true,
		_isLooping = true;
	uint32 _lastTime = 0;
	float _camAcceleration = 1.0f;
};

class IGraphics {
public:
	virtual ~IGraphics() = default;
};

}

#endif
