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

#ifndef TWP_ROOM_H
#define TWP_ROOM_H

#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"
#include "math/vector2d.h"

namespace Twp {

class Layer {
public:
	Common::Array<Common::String> names;
	Math::Vector2d parallax;
	int zsort;
};

// Represents an area where an actor can or cannot walk
class Walkbox {
public:
	Walkbox(const Common::Array<Math::Vector2d> &polygon, bool visible = true);

public:
	Common::String name;

private:
	Common::Array<Math::Vector2d> _polygon;
	bool _visible;
};

enum Direction {
	dNone = 0,
	dRight = 1,
	dLeft = 2,
	dFront = 4,
	dBack = 8
};

enum ObjectType {
	otNone,
	otProp,
	otSpot,
	otTrigger
};

struct ObjectAnimation {
	Common::String name;
	Common::String sheet;
	Common::StringArray frames;
	Common::Array<ObjectAnimation> layers;
	Common::StringArray triggers;
	Common::Array<Math::Vector2d> offsets;
	bool loop;
	float fps;
	int flags;
	int frameIndex;
};

struct ScalingValue {
	float scale;
	int y;
};

struct Scaling {
	Common::Array<ScalingValue> values;
	Common::String trigger;
};

class Room;
class Object {
public:
	int getId();

public:
	HSQOBJECT table;
	Common::String name;
	Common::String sheet;
	Common::String key; // key used to identify this object by script
	int state;
	Math::Vector2d usePos;
	Direction useDir;
	Common::Rect hotspot;
	ObjectType objType;
	Room *room;
	Common::Array<ObjectAnimation> anims;
	bool temporary;
	bool touchable;
};

class Room {
public:
	void load(Common::SeekableReadStream &s);

	Object* createObject(const Common::String& sheet, const Common::Array<Common::String>& frames);
	Object *createTextObject(const Common::String& fontName, const Common::String& text, TextHAlignment hAlign = thLeft, TextVAlignment vAlign = tvCenter, float maxWidth = 0.0f);

	Math::Vector2d getScreenSize();
	Math::Vector2d roomToScreen(Math::Vector2d pos);

public:
	Common::String _name;              // Name of the room
	Common::String _sheet;             // Name of the spritesheet to use
	Math::Vector2d _roomSize;          // Size of the room
	int _fullscreen;                   // Indicates if a room is a closeup room (fullscreen=1) or not (fullscreen=2), just a guess
	int _height;                       // Height of the room (what else ?)
	Common::Array<Layer> _layers;      // Parallax layers of a room
	Common::Array<Walkbox> _walkboxes; // Represents the areas where an actor can or cannot walk
	Common::Array<Scaling> _scalings;  // Defines the scaling of the actor in the room
    Scaling _scaling;				  // Defines the scaling of the actor in the room
};

} // namespace Twp

#endif
