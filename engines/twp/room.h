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
#include "twp/squirrel/squirrel.h"
#include "twp/font.h"

namespace Twp {

class Node;
class Object;

class Layer {
public:
	Common::Array<Common::String> _names;
	Common::Array<Object *> _objects;
	Math::Vector2d _parallax;
	int _zsort;
	Node *_node;
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

class Anim;
class Room;
class Node;

enum Facing {
	FACE_RIGHT = 1,
	FACE_LEFT = 2,
	FACE_FRONT = 4,
	FACE_BACK = 8
};

class Trigger {
public:
	virtual ~Trigger() {}
	virtual void trig() = 0;
};

class Object {
public:
	Object();

	int getId();

	void setState(int state, bool instant = false);
	void play(int state, bool loop = false, bool instant = false);
	// Plays an animation specified by the `state`.
	void play(const Common::String &state, bool loop = false, bool instant = false);
	void showLayer(const Common::String &layer, bool visible);
	Facing getFacing() const;
	void trig(const Common::String &name);

private:
	Common::String suffix() const;
	// Plays an animation specified by the state
	bool playCore(const Common::String &state, bool loop = false, bool instant = false);

public:
	HSQOBJECT _table;
	Common::String _name;
	Common::String _sheet;
	Common::String _key; // key used to identify this object by script
	int _state;
	Math::Vector2d _usePos;
	Direction _useDir;
	Common::Rect _hotspot;
	ObjectType _objType;
	Room *_room;
	Common::Array<ObjectAnimation> _anims;
	bool _temporary;
	bool _touchable;
	Node *_node;
	Anim *_nodeAnim;
	Layer *_layer;
	Common::StringArray _hiddenLayers;
	Common::String _animName;
	int _animFlags;
	bool _animLoop;
	Common::HashMap<Facing, Facing, Common::Hash<int> > _facingMap;
	Facing _facing;
	int _facingLockValue;
	float _fps;
	Common::HashMap<int, Trigger*> _triggers;
	Math::Vector2d _talkOffset;
};

class Room {
public:
	void load(Common::SeekableReadStream &s);

	Object *createObject(const Common::String &sheet, const Common::Array<Common::String> &frames);
	Object *createTextObject(const Common::String &fontName, const Common::String &text, TextHAlignment hAlign = thLeft, TextVAlignment vAlign = tvCenter, float maxWidth = 0.0f);

	Math::Vector2d getScreenSize();
	Math::Vector2d roomToScreen(Math::Vector2d pos);

	Layer *layer(int zsort);

public:
	Common::String _name;              // Name of the room
	Common::String _sheet;             // Name of the spritesheet to use
	Math::Vector2d _roomSize;          // Size of the room
	int _fullscreen;                   // Indicates if a room is a closeup room (fullscreen=1) or not (fullscreen=2), just a guess
	int _height;                       // Height of the room (what else ?)
	Common::Array<Layer *> _layers;    // Parallax layers of a room
	Common::Array<Walkbox> _walkboxes; // Represents the areas where an actor can or cannot walk
	Common::Array<Scaling> _scalings;  // Defines the scaling of the actor in the room
	Scaling _scaling;                  // Defines the scaling of the actor in the room
};

} // namespace Twp

#endif
