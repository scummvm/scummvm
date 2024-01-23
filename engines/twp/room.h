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
#include "twp/motor.h"
#include "twp/scenegraph.h"
#include "twp/graph.h"

#define FULLSCREENCLOSEUP	1
#define FULLSCREENROOM		2

namespace Twp {

enum class RoomEffect {
	None = 0,
	Sepia = 1,
	Ega = 2,
	Vhs = 3,
	Ghost = 4,
	BlackAndWhite = 5
};

class Node;
class Object;

class Layer {
public:
	Layer(const Common::String &name, Math::Vector2d parallax, int zsort);
	Layer(const Common::StringArray &names, Math::Vector2d parallax, int zsort);

public:
	Common::Array<Common::String> _names;
	Common::Array<Object *> _objects;
	Math::Vector2d _parallax;
	int _zsort = 0;
	Node *_node = nullptr;
};

struct ScalingValue {
	float scale;
	int y;
};

struct Scaling {
	Common::Array<ScalingValue> values;
	Common::String trigger;

	float getScaling(float yPos);
};

struct Light {
	Color color;
	Math::Vector2d pos;
	float brightness;    // light brightness 1.0f...100.f
	float coneDirection; // cone direction 0...360.f
	float coneAngle;     // cone angle 0...360.f
	float coneFalloff;   // cone falloff 0.f...1.0f
	float cutOffRadius;  // cutoff radius
	float halfRadius;    // cone half radius 0.0f...1.0f
	bool on;
	int id;
};

struct Lights {
	int _numLights; // Number of lights
	Light _lights[50];
	Color _ambientLight; // Ambient light color
};

struct ScalingTrigger {
	ScalingTrigger(Object * obj, Scaling* scaling);

	Object * _obj = nullptr;
	Scaling* _scaling = nullptr;
};

class PathFinder;
class Scene;
class Room {
public:
	Room(const Common::String &name, HSQOBJECT &table);
	~Room();

	void load(Common::SeekableReadStream &s);

	void update(float elapsedSec);

	Object *createObject(const Common::String &sheet, const Common::Array<Common::String> &frames);
	Object *createTextObject(const Common::String &fontName, const Common::String &text, TextHAlignment hAlign = thLeft, TextVAlignment vAlign = tvCenter, float maxWidth = 0.0f);

	Math::Vector2d getScreenSize();

	Layer *layer(int zsort);
	Object *getObj(const Common::String &key);

	Light *createLight(Color color, Math::Vector2d pos);
	float getScaling(float yPos);
	void objectParallaxLayer(Object *obj, int zsort);
	void setOverlay(Color color);
	Color getOverlay() const;

	void walkboxHidden(const Common::String &name, bool hidden);
	Common::Array<Vector2i> calculatePath(Vector2i frm, Vector2i to);

public:
	Common::String _name;              // Name of the room
	Common::String _sheet;             // Name of the spritesheet to use
	Math::Vector2d _roomSize;          // Size of the room
	int _fullscreen = 0;               // Indicates if a room is a closeup room (fullscreen=1) or not (fullscreen=2), just a guess
	int _height = 0;                   // Height of the room (what else ?)
	Common::Array<Layer *> _layers;    // Parallax layers of a room
	Common::Array<Walkbox> _walkboxes; // Represents the areas where an actor can or cannot walk
	Common::Array<Walkbox> _mergedPolygon;
	Common::Array<Scaling> _scalings;  // Defines the scaling of the actor in the room
	Scaling _scaling;                  // Defines the scaling of the actor in the room
	HSQOBJECT _table;                  // Squirrel table representing this room
	bool _entering = false;            // Indicates whether or not an actor is entering this room
	Lights _lights;                    // Lights of the room
	Common::Array<Object *> _triggers; // Triggers currently enabled in the room
	Common::Array<ScalingTrigger> _scalingTriggers; // Scaling Triggers of the room
	bool _pseudo = false;
	Common::Array<Object *> _objects;
	Scene *_scene = nullptr;
	OverlayNode _overlayNode;	// Represents an overlay
	RoomEffect _effect = RoomEffect::None;
	Motor* _overlayTo = nullptr;
	PathFinder _pathFinder;
};

} // namespace Twp

#endif
