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
	Common::String _name;

private:
	Common::Array<Math::Vector2d> _polygon;
	bool _visible;
};

struct ScalingValue {
	float scale;
	int y;
};

struct Scaling {
	Common::Array<ScalingValue> values;
	Common::String trigger;
};

struct Light {
    Color color;
    Math::Vector2d pos;
    float brightness;     // light brightness 1.0f...100.f
    float coneDirection;  // cone direction 0...360.f
    float coneAngle;      // cone angle 0...360.f
    float coneFalloff;    // cone falloff 0.f...1.0f
    float cutOffRadius;   // cutoff raduus
    float halfRadius;     // cone half radius 0.0f...1.0f
    bool on;
    int id;
};

struct Lights {
	int _numLights;			// Number of lights
	Light _lights[50];
	Color _ambientLight;	// Ambient light color
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
	Common::String _name;               	// Name of the room
	Common::String _sheet;              	// Name of the spritesheet to use
	Math::Vector2d _roomSize;           	// Size of the room
	int _fullscreen;                    	// Indicates if a room is a closeup room (fullscreen=1) or not (fullscreen=2), just a guess
	int _height;                        	// Height of the room (what else ?)
	Common::Array<Layer *> _layers;     	// Parallax layers of a room
	Common::Array<Walkbox> _walkboxes;  	// Represents the areas where an actor can or cannot walk
	Common::Array<Scaling> _scalings;   	// Defines the scaling of the actor in the room
	Scaling _scaling;                   	// Defines the scaling of the actor in the room
	HSQOBJECT _table;						// Squirrel table representing this room
	bool _entering;							// Indicates whether or not an actor is entering this room
	Lights _lights;							// Lights of the room
	Common::Array<Walkbox> _mergedPolygon;
	Common::Array<Object*> _triggers;		// Triggers currently enabled in the room
	bool _pseudo;
};

} // namespace Twp

#endif
