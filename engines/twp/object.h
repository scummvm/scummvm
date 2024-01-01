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

enum UseFlag {
	ufNone,
	ufUseWith,
	ufUseOn,
	ufUseIn,
	ufGiveTo
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

class Trigger {
public:
	virtual ~Trigger() {}
	virtual void trig() = 0;
};

struct VerbId {
	int id = 0;
};

class Object;
struct Sentence {
	VerbId verb;
	Object *noun1 = nullptr;
	Object *noun2 = nullptr;
	bool enabled = false;
};

class Anim;
class Room;
class Motor;
class Node;
class Layer;

struct TalkingState {
	Object *_obj;
	Color _color;

	void say(const Common::StringArray &texts, Object *obj);
};

struct LockFacing {
	Facing key;
	Facing value;
};

class Object {
public:
	Object();
	Object(HSQOBJECT o, const Common::String &key);
	~Object();

	static Object *createActor();

	Common::String getname() const;
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

	bool touchable();
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
	bool inInventory();
	void removeInventory(Object *obj);
	void removeInventory();

	int getFlags();
	UseFlag useFlag();

	bool contains(Math::Vector2d pos);
	void setRoom(Room *room);
	void delObject();
	void stopObjectMotors();
	void dependentOn(Object *dependentObj, int state);

	Common::String getAnimName(const Common::String &key);
	void setHeadIndex(int head);
	void setAnimationNames(const Common::String &head, const Common::String &stand, const Common::String &walk, const Common::String &reach);
	Common::String getReachAnim();

	bool isWalking();
	void stopWalking();
	void blinkRate(float min, float max);
	void setCostume(const Common::String &name, const Common::String &sheet);
	void stand();

	void update(float elapsedSec);

	void setAlphaTo(Motor *alphaTo);
	void setRotateTo(Motor *rotateTo);
	void setMoveTo(Motor *moveTo);
	void setWalkTo(Motor *walkTo);
	Motor *getWalkTo() const { return _walkTo; }
	void walk(Math::Vector2d pos, int facing = 0);
	void walk(Object* obj);

	void setTalking(Motor *talking);
	void setBlink(Motor *blink);
	void setTurnTo(Motor *turnTo);
	void setShakeTo(Motor *shakeTo);
	void setJiggleTo(Motor *jiggleTo);

	Motor *getTalking() { return _talking; }
	void stopTalking();
	void say(const Common::StringArray &texts, Color color);

	void pickupObject(Object *obj);

	void execVerb();
	void turn(Facing facing);
	void turn(Object* obj);

private:
	Common::String suffix() const;
	// Plays an animation specified by the state
	bool playCore(const Common::String &state, bool loop = false, bool instant = false);
	int flags();

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
	Room *_room = nullptr;
	Common::Array<ObjectAnimation> _anims;
	bool _temporary = false;
	bool _touchable = false;
	Node *_node = nullptr;
	Node *_sayNode = nullptr;
	Anim *_nodeAnim = nullptr;
	Layer *_layer = nullptr;
	Common::StringArray _hiddenLayers;
	Common::String _animName;
	int _animFlags = 0;
	bool _animLoop = false;
	Common::Array<LockFacing> _facingMap;
	Facing _facing = FACE_FRONT;
	int _facingLockValue = 0;
	float _fps = 0.f;
	Common::HashMap<int, Trigger *> _triggers;
	Math::Vector2d _talkOffset;
	Math::Vector2d _walkSpeed;
	bool _triggerActive = false;
	bool _useWalkboxes = false;
	float _volume = 1.f;
	Color _talkColor;
	Common::HashMap<Common::String, Common::String> _animNames;
	bool _lit = false;
	Object *_owner = nullptr;
	Common::Array<Object *> _inventory;
	int _inventoryOffset = 0;
	Common::StringArray _icons;
	int _iconFps = 0;
	int _iconIndex = 0;
	float _iconElapsed = 0.f;
	HSQOBJECT _enter, _leave;
	int _dependentState = 0;
	Object *_dependentObj = nullptr;
	float _popElapsed = 0.f;
	int _popCount = 0;
	Sentence _exec;

private:
	Motor *_alphaTo = nullptr;
	Motor *_rotateTo = nullptr;
	Motor *_moveTo = nullptr;
	Motor *_walkTo = nullptr;
	Motor *_talking = nullptr;
	Motor *_blink = nullptr;
	Motor *_turnTo = nullptr;
	Motor *_shakeTo = nullptr;
	Motor *_jiggleTo = nullptr;
	TalkingState _talkingState;
};

} // namespace Twp

#endif
