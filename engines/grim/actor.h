/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_ACTOR_H
#define GRIM_ACTOR_H

#include "engines/grim/pool.h"
#include "engines/grim/object.h"
#include "engines/grim/color.h"
#include "math/vector3d.h"
#include "math/angle.h"

namespace Grim {

class TextObject;
class Sector;
class Costume;
class LipSync;
class Font;
class Set;
class Joint;

struct Plane {
	Common::String setName;
	Sector *sector;
};

typedef Common::List<Plane> SectorListType;

#define MAX_SHADOWS 5

struct Shadow {
	Common::String name;
	Math::Vector3d pos;
	SectorListType planeList;
	byte *shadowMask;
	int shadowMaskSize;
	bool active;
	bool dontNegate;
};

/**
 * @class Actor
 *
 * @short Actor represents a 3D character on screen.
 */
class Actor : public PoolObject<Actor, MKTAG('A', 'C', 'T', 'R')> {
public:
	enum CollisionMode {
		CollisionOff = 0,
		CollisionBox = 1,
		CollisionSphere = 2
	};

	/**
	 * Initializes an actor with the given name.
	 *
	 * @param name The name of the actor.
	 */
	Actor(const Common::String &name);
	/**
	 * Builds an actor setting up only the minimal variables.
	 */
	Actor();
	/**
	 * Destroys the actor.
	 * The actor is automatically removed from the GrimEngine instance.
	 */
	~Actor();

	/**
	 * Saves the actor state.
	 *
	 * @param savedState The save state to which save the actor's state.
	 */
	void saveState(SaveGame *savedState) const;
	/**
	 * Restores the actor state.
	 *
	 * @param savedState The saved state from which the actor will be restored.
	 */
	bool restoreState(SaveGame *savedState);

	/**
	 * Returns the name of the actor.
	 */
	const Common::String &getName() const { return _name; }

	/**
	 * Sets the color of the subtitles of the actor.
	 *
	 * @param color The color.
	 * @see getTalkColor
	 */
	void setTalkColor(const Color &color) { _talkColor = color; }
	/**
	 * Returns the color of the subtitles of the actor.
	 *
	 * @see setTalkColor
	 */
	Color getTalkColor() const { return _talkColor; }

	/**
	 * Sets the position of the actor on the 3D scene.
	 *
	 * @param position The position.
	 * @see getPos
	 */
	void setPos(const Math::Vector3d &position);
	/**
	 * Returns the position of the actor on the 3D scene.
	 *
	 * @see setPos
	 */
	Math::Vector3d getPos() const { return _pos; }

	/**
	 * Tells the actor to go to the wanted position.
	 * If the actor follows the walkboxes it will find the best
	 * route to go there, otherwise it will walk on a straight line.
	 *
	 * @param position The destination position.
	 * @see stopWalking
	 * @see isWalking
	 */
	void walkTo(const Math::Vector3d &position);
	/**
	 * Stops immediately the actor's walk.
	 *
	 * @see walkTo
	 * @see isWalking
	 */
	void stopWalking() { _walking = false; }
	/**
	 * Returns true if the actor is walking to a position.
	 *
	 * @see walkTo
	 * @see stopWalking
	 */
	bool isWalking() const;

	/**
	 * Sets the rotation of thes actor in the 3D scene.
	 * The effect is not immediate, the actor will slowly rotate
	 * to the destination orientation.
	 *
	 * @param pitch The rotation of the x axis
	 * @param yaw The rotation of the z axis
	 * @param roll The rotation of the y axis
	 * @see getPitch
	 * @see getYaw
	 * @see getRoll
	 * @see setRot
	 * @see turn
	 * @see isTurning
	 */
	void turnTo(const Math::Angle &pitch, const Math::Angle &yaw, const Math::Angle &roll);
	/**
	 * Returns true if the actor is turning.
	 *
	 * @see turnTo
	 */
	 bool isTurning() const;
	/**
	 * Sets the rotation of the actor in the 3D scene.
	 * The effect is immediate.
	 *
	 * @param pitch The rotation of the x axis
	 * @param yaw The rotation of the z axis
	 * @param roll The rotation of the y axis
	 * @see getPitch
	 * @see getYaw
	 * @see getRoll
	 * @see turnTo
	 * @see turn
	 * @see isTurning
	 */
	void setRot(const Math::Angle &pitch, const Math::Angle &yaw, const Math::Angle &roll);
	/**
	 * Turns the actor by the given parameter on the z axis.
	 * The actual movement depends also on the turn rate.
	 *
	 * @param dir The quantity of the movement.
	 * @see getPitch
	 * @see getYaw
	 * @see getRoll
	 * @see setRot
	 * @see turnTo
	 * @see isTurning
	 */
	void turn(int dir);
	/**
	 * Returns the pitch of the actor, which is the rotation
	 * on the x axis.
	 *
	 * @see getYaw
	 * @see getRoll
	 * @see setRot
	 * @see turnTo
	 * @see isTurning
	 */
	Math::Angle getPitch() const { return _pitch; }
	/**
	 * Returns the yaw of the actor, which is the rotation
	 * on the z axis.
	 *
	 * @see getPitch
	 * @see getRoll
	 * @see setRot
	 * @see turnTo
	 * @see isTurning
	 */
	Math::Angle getYaw() const { return _yaw; }
	/**
	 * Returns the roll of the actor, which is the rotation
	 * on the y axis.
	 *
	 * @see getPitch
	 * @see getYaw
	 * @see setRot
	 * @see turnTo
	 * @see isTurning
	 */
	Math::Angle getRoll() const { return _roll; }

	/**
	 * Calculates and returns the angle between the direction the
	 * actor is facing and the direction towards another actor.
	 *
	 * @param actor The actor to look at.
	 */
	Math::Angle  getYawTo(Actor *actor) const;
	/**
	 * Calculates and returns the angle between the direction the
	 * actor is facing and the direction towards a point.
	 *
	 * @param actor The point to look at.
	 */
	Math::Angle  getYawTo(const Math::Vector3d &p) const;

	/**
	 * Sets the actor visibility.
	 *
	 * @param val The value: true if visible, false otherwise.
	 * @see isVisible
	 */
	void setVisibility(bool val) { _visible = val; }
	/**
	 * Returns true if the actor is visible.
	 *
	 * @see setVisibility
	 */
	bool isVisible() const { return _visible; }

	/**
	 * Sets the scale of the actor.
	 * A value of 1 is the natural size.
	 *
	 * @param scale The scale of the actor.
	 */
	void setScale(float scale) { _scale = scale; }

	/**
	 * Sets the time scale of the actor, used to calculate the
	 * speed of its animations.
	 * A value of 1 is the normal speed.
	 *
	 * @param scale The time scale.
	 * @see getTimeScale
	 */
	void setTimeScale(float scale) { _timeScale = scale; }
	/**
	 * Returns the time scale of the actor.
	 *
	 * @see setTimeScale
	 */
	float getTimeScale() const { return _timeScale; }

	/**
	 * Puts the actor in a set.
	 *
	 * @param setName The name of the set.
	 */
	void putInSet(const Common::String &setName);
	/**
	 * Returns true if the actor is in the given set.
	 *
	 * @param setName The name of the set.
	 */
	bool isInSet(const Common::String &setName) const;

	/**
	 * Sets the rate of the turning.
	 *
	 * @param rat The wanted rate.
	 * @see getTurnRate
	 */
	void setTurnRate(float rate) { _turnRate = rate; }
	/**
	 * Returns the turn rate.
	 *
	 * @see setTurnRate
	 */
	float getTurnRate() const { return _turnRate; }
	/**
	 * Sets the rate of the walk movement.
	 *
	 * @param rate The wanted rate.
	 * @see getWalkRate
	 */
	void setWalkRate(float rate) { _walkRate = rate; }
	/**
	 * Returns the walk rate of the actor.
	 *
	 * @see setWalkRate
	 */
	float getWalkRate() const { return _walkRate; }

	void setLooking(bool lookingMode) { _lookingMode = lookingMode; }

	/**
	 * Makes the actor move forward, the lenght of the movement based
	 * on the walk rate.
	 * If it is following boxes it will not go into not walkable areas.
	 *
	 * @see walkTo
	 */
	void walkForward();
	void moveTo(const Math::Vector3d &pos);
	/**
	 * Used to tell the actor if it is running or not.
	 *
	 * @param running The value: true if it is running.
	 */
	void setRunning(bool running) { _running = running; }
	void setReflection(float angle) { _reflectionAngle = angle; }
	/**
	 * Returns a vector representing the direction the actor
	 * is facing.
	 */
	Math::Vector3d getPuckVector() const;

	void setPuckOrient(bool orient);

	/**
	 * Makes the actor say the given line.
	 * It will show a subtitle and/or play the voice, depending
	 * on the speech mode set in the GrimEngine instance.
	 *
	 * @param msgId The id of the message to say.
	 * @param background ?? actual meaning unknown yet.
	 * @see isTalking
	 * @see shutUp
	 */
	void sayLine(const char *msgId, bool background);
	// When we clean all text objects we don't want the actors to clean their
	// objects again since they're already freed
	void lineCleanup();
	/**
	 * Makes the actor discard any subtitle and voice.
	 *
	 * @see sayLine
	 * @see isTalking
	 */
	void shutUp();
	/**
	 * Returns true if the actor is saying something.
	 *
	 * @see sayLine
	 * @see shutUp
	 */
	bool isTalking();

	void setRestChore(int choreNumber, Costume *cost);
	int getRestChore() const;
	void setWalkChore(int choreNumber, Costume *cost);
	void setTurnChores(int left_chore, int right_chore, Costume *cost);
	void setTalkChore(int index, int choreNumber, Costume *cost);
	int getTalkChore(int index) const;
	Costume *getTalkCostume(int index) const;
	void setMumbleChore(int choreNumber, Costume *cost);

	void setColormap(const char *map);
	void pushCostume(const char *name);
	void setCostume(const char *name);
	void popCostume();
	void clearCostumes();
	Costume *getCurrentCostume() const {
		if (_costumeStack.empty())
			return NULL;
		else
			return _costumeStack.back();
	}
	Costume *findCostume(const Common::String &name);
	int getCostumeStackDepth() const {
		return _costumeStack.size();
	}

	void setActiveShadow(int shadowId);
	void setShadowPoint(const Math::Vector3d &pos);
	void setShadowPlane(const char *name);
	void addShadowPlane(const char *name);
	void clearShadowPlanes();
	void setShadowValid(int);
	void setActivateShadow(int, bool);

	void setConstrain(bool constrain) {
		_constrain = constrain;
	}
	void update(uint frameTime);
	/**
	 * Check if the actor is still talking. If it is returns true, otherwise false.
	 */
	bool updateTalk();
	void draw();

	bool isLookAtVectorZero() {
		return _lookAtVector.isZero();
	}
	void setLookAtVectorZero() {
		_lookAtVector.set(0.f, 0.f, 0.f);
	}
	void setLookAtVector(const Math::Vector3d &vector) {
		_lookAtVector = vector;
	}
	Math::Vector3d getLookAtVector() {
		return _lookAtVector;
	}
	void setLookAtRate(float rate);
	float getLookAtRate() const;
	void setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw);

	void setCollisionMode(CollisionMode mode);
	void setCollisionScale(float scale);

	bool handleCollisionWith(Actor *actor, CollisionMode mode, Math::Vector3d *vec) const;

	static void saveStaticState(SaveGame *state);
	static void restoreStaticState(SaveGame *state);

	bool isAttached() const { return _attachedActor != NULL; }
	Math::Vector3d getWorldPos() const;
	void attachToActor(Actor *other, const char *joint);
	void detach();

private:
	void costumeMarkerCallback(int marker);
	void collisionHandlerCallback(Actor *other) const;
	void updateWalk();
	void addShadowPlane(const char *n, Set *scene, int shadowId);
	bool shouldDrawShadow(int shadowId);
	void stopTalking();
	bool stopMumbleChore();
	/**
	 * Given a start point and a destination this function returns a position
	 * that doesn't collide with any actor.
	 */
	Math::Vector3d handleCollisionTo(const Math::Vector3d &from, const Math::Vector3d &pos) const;
	/**
	 * Check if the line from pos to dest collides with this actor's bounding
	 * box, and if yes return a point that, together with pos, defines a line
	 * tangent with the bounding box.
	 */
	Math::Vector3d getTangentPos(const Math::Vector3d &pos, const Math::Vector3d &dest) const;

	Math::Vector3d getSimplePuckVector() const;

	Common::String _name;
	Common::String _setName;    // The actual current set

	Color _talkColor;
	Math::Vector3d _pos;
	Math::Angle _pitch, _yaw, _roll;
	float _walkRate, _turnRate;

	bool _constrain;	// Constrain to walkboxes
	float _reflectionAngle;	// Maximum angle to turn by at walls
	bool _visible;
	float _scale;
	float _timeScale;
	bool _lookingMode;
	Common::String _talkSoundName;
	bool _talking;
	bool _backgroundTalk;
	ObjectPtr<LipSync> _lipSync;
	Common::List<Costume *> _costumeStack;

	// Variables for gradual turning
	bool _turning;
	Math::Angle _destYaw;

	// Variables for walking to a point
	bool _walking;
	Math::Vector3d _destPos;

	// chores
	class Chore {
	public:
		Chore();
		Chore(Costume *cost, int chore);

		void play(bool fade = false, unsigned int time = fadeTime);
		void playLooping(bool fade = false, unsigned int time = fadeTime);
		void stop(bool fade = false, unsigned int time = fadeTime);
		void setLastFrame();

		inline bool isValid() const { return _chore > -1; }
		bool isPlaying() const;
		inline bool equals(Costume *cost, int chore) const {
			return (_costume == cost && _chore == chore);
		}

		void saveState(SaveGame *state) const;
		void restoreState(SaveGame *state, Actor *actor);


		Costume *_costume;
		int _chore;

		static const unsigned int fadeTime;
	};
	Chore _restChore;

	Chore _walkChore;
	bool _walkedLast, _walkedCur;
	bool _running;

	Chore _leftTurnChore, _rightTurnChore;
	int _lastTurnDir, _currTurnDir;

	Chore _talkChore[10];
	int _talkAnim;

	Chore _mumbleChore;

	Shadow *_shadowArray;
	int _activeShadowSlot;

	static ObjectPtr<Font> _sayLineFont;
	int _sayLineText;
	bool _mustPlaceText;

	// Validate a yaw angle then set it appropriately
	void setYaw(const Math::Angle &yaw);

	Chore *getTurnChore(int dir) {
		return (dir > 0 ? &_rightTurnChore : &_leftTurnChore);
	}

	void freeCostumeChore(Costume *toFree, Chore *chore);

	// lookAt
	Math::Vector3d _lookAtVector;

	// struct used for path finding
	struct PathNode {
		Sector *sect;
		PathNode *parent;
		Math::Vector3d pos;
		float dist;
		float cost;
	};
	Common::List<Math::Vector3d> _path;

	CollisionMode _collisionMode;
	float _collisionScale;

	bool _puckOrient;

	static bool _isTalkingBackground;
	Actor *_attachedActor;
	Common::String _attachedJoint;

	friend class GrimEngine;
};

} // end of namespace Grim

#endif
