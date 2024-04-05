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

#ifndef TWP_OBJECT_H
#define TWP_OBJECT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "math/vector2d.h"
#include "twp/squirrel/squirrel.h"
#include "twp/ids.h"
#include "twp/gfx.h"
#include "twp/util.h"
#include "twp/objectanimation.h"

#define STAND_ANIMNAME "stand"
#define HEAD_ANIMNAME "head"
#define WALK_ANIMNAME "walk"
#define REACH_ANIMNAME "reach"

namespace Twp {

enum ObjectType {
	otNone,
	otProp,
	otSpot,
	otTrigger
};

enum Direction {
	dNone = 0,
	dRight = 1,
	dLeft = 2,
	dFront = 4,
	dBack = 8
};

enum class Facing {
	FACE_RIGHT = 1,
	FACE_LEFT = 2,
	FACE_FRONT = 4,
	FACE_BACK = 8
};

enum class UseFlag {
	ufNone,
	ufUseWith,
	ufUseOn,
	ufUseIn,
	ufGiveTo
};

class Trigger {
public:
	virtual ~Trigger() {}
	virtual void trig() = 0;
};

struct VerbId {
	int id = VERB_WALKTO;
};

class Object;
struct Sentence {
	VerbId verb;
	Common::SharedPtr<Object> noun1;
	Common::SharedPtr<Object> noun2;
	bool enabled = false;
};

class Anim;
class Room;
class Motor;
class Node;
class Layer;
class Blink;
class TextNode;

struct TalkingState {
	Common::SharedPtr<Object> _obj;
	Color _color;

	void say(const Common::StringArray &texts, Common::SharedPtr<Object> obj);
};

struct LockFacing {
	Facing key;
	Facing value;
};

struct ObjectIcons {
	int fps = 0;
	Common::StringArray icons;
};

class Object {
public:
	Object();
	Object(HSQOBJECT o, const Common::String &key);
	~Object();

	static Common::SharedPtr<Object> createActor();

	Common::String getName() const;
	int getId() const;

	// Changes the `state` of an object, although this can just be a internal state,
	//
	// it is typically used to change the object's image as it moves from it's current state to another.
	// Behind the scenes, states as just simple ints. State0, State1, etc.
	// Symbols like `CLOSED` and `OPEN` and just pre-defined to be 0 or 1.
	// State 0 is assumed to be the natural state of the object, which is why `OPEN` is 1 and `CLOSED` is 0 and not the other way around.
	// This can be a little confusing at first.
	// If the state of an object has multiple frames, then the animation is played when changing state, such has opening the clock.
	// `GONE` is a unique in that setting an object to `GONE` both sets its graphical state to 1, and makes it untouchable.
	// Once an object is set to `GONE`, if you want to make it visible and touchable again, you have to set both:
	//
	// .. code-block:: Squirrel
	// objectState(coin, HERE)
	// objectTouchable(coin, YES)
	void setState(int state, bool instant = false);
	int getState() const { return _state; }

	bool isTouchable();
	void setTouchable(bool value);

	void play(int state, bool loop = false, bool instant = false);
	// Plays an animation specified by the `state`.
	void play(const Common::String &state, bool loop = false, bool instant = false);

	void showLayer(const Common::String &layer, bool visible);
	Facing getFacing() const;
	void lockFacing(int facing);
	void lockFacing(Facing left, Facing right, Facing front, Facing back);
	void resetLockFacing();
	void trig(const Common::String &name);

	void setPop(int count);
	int getPop() const { return _popCount; }
	float popScale() const;
	float getScale();

	int defaultVerbId();
	void setFacing(Facing facing);

	Math::Vector2d getUsePos();
	Facing getDoorFacing();

	void setIcon(int fps, const Common::StringArray &icons);
	void setIcon(const Common::String &icon);
	Common::String getIcon();
	ObjectIcons getIcons() const;
	bool inInventory();
	void removeInventory(Common::SharedPtr<Object> obj);

	int getFlags();
	UseFlag useFlag();

	bool contains(const Math::Vector2d &pos);
	static void setRoom(Common::SharedPtr<Object> object, Common::SharedPtr<Room> room);
	void stopObjectMotors();
	void dependentOn(Common::SharedPtr<Object> dependentObj, int state);

	Common::String getAnimName(const Common::String &key);
	void setHeadIndex(int head);
	void setAnimationNames(const Common::String &head, const Common::String &stand, const Common::String &walk, const Common::String &reach);
	Common::String getReachAnim();

	bool isWalking();
	void stopWalking();
	static void blinkRate(Common::SharedPtr<Object>, float min, float max);
	void setCostume(const Common::String &name, const Common::String &sheet);
	void stand();

	void update(float elapsedSec);

	void setAlphaTo(Common::SharedPtr<Motor> alphaTo);
	void setRotateTo(Common::SharedPtr<Motor> rotateTo);
	void setMoveTo(Common::SharedPtr<Motor> moveTo);
	void setReach(Common::SharedPtr<Motor> reach);
	Common::SharedPtr<Motor> getWalkTo() { return _walkTo; }
	Common::SharedPtr<Motor> getReach() { return _reach; }
	static void walk(Common::SharedPtr<Object> obj, const Math::Vector2d &pos, int facing = 0);
	static void walk(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj);

	void setTalking(Common::SharedPtr<Motor> talking);
	void setShakeTo(Common::SharedPtr<Motor> shakeTo);
	void setScaleTo(Common::SharedPtr<Motor> scaleTo);

	Common::SharedPtr<Motor> getTalking() { return _talking; }
	void stopTalking();
	static void say(Common::SharedPtr<Object> obj, const Common::StringArray &texts, const Color &color);

	static void pickupObject(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj);

	static void execVerb(Common::SharedPtr<Object> obj);
	void turn(Facing facing);
	static void turn(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj);
	void jiggle(float amount);

	void inventoryScrollUp();
	void inventoryScrollDown();

private:
	Common::String suffix() const;
	// Plays an animation specified by the state
	bool playCore(const Common::String &state, bool loop = false, bool instant = false);

public:
	HSQOBJECT _table;
	Common::String _name;
	Common::String _parent;
	Common::String _sheet; // Spritesheet to use when a sprite is displayed in the room: "raw" means raw texture, empty string means use room texture
	Common::String _key;   // key used to identify this object by script
	Common::String _costumeName, _costumeSheet;
	int _state = -1;
	Math::Vector2d _usePos; // use position
	Direction _useDir = dNone;
	Common::Rect _hotspot;
	ObjectType _objType = otNone;
	Common::SharedPtr<Room> _room;
	Common::Array<ObjectAnimation> _anims;
	bool _temporary = false;
	Common::SharedPtr<Node> _node;
	Common::SharedPtr<TextNode> _sayNode;
	Common::SharedPtr<Anim> _nodeAnim;
	Common::SharedPtr<Layer> _layer;
	Common::StringArray _hiddenLayers;
	Common::String _animName;
	int _animFlags = 0;
	bool _animLoop = false;
	Common::Array<LockFacing> _facingMap;
	Facing _facing = Facing::FACE_FRONT;
	int _facingLockValue = 0;
	float _fps = 0.f;
	Common::HashMap<int, Common::SharedPtr<Trigger> > _triggers;
	Math::Vector2d _talkOffset;
	Math::Vector2d _walkSpeed;
	bool _triggerActive = false;
	bool _useWalkboxes = false;
	float _volume = 1.f;
	Color _talkColor;
	Common::HashMap<Common::String, Common::String> _animNames;
	bool _lit = false;
	Common::SharedPtr<Object> _owner;
	Common::Array<Common::SharedPtr<Object> > _inventory;
	int _inventoryOffset = 0;
	int _iconIndex = 0;
	float _iconElapsed = 0.f;
	HSQOBJECT _enter, _leave;
	int _dependentState = 0;
	Common::SharedPtr<Object> _dependentObj;
	float _popElapsed = 0.f;
	int _popCount = 0;
	Sentence _exec;
	int _sound = 0;
	bool _jiggle = false;

private:
	Common::SharedPtr<Motor> _alphaTo;
	Common::SharedPtr<Motor> _rotateTo;
	Common::SharedPtr<Motor> _moveTo;
	Common::SharedPtr<Motor> _walkTo;
	Common::SharedPtr<Motor> _reach;
	Common::SharedPtr<Motor> _talking;
	Common::SharedPtr<Blink> _blink;
	Common::SharedPtr<Motor> _turnTo;
	Common::SharedPtr<Motor> _shakeTo;
	Common::SharedPtr<Motor> _jiggleTo;
	Common::SharedPtr<Motor> _scaleTo;
	TalkingState _talkingState;
};

} // namespace Twp

#endif
