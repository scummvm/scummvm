/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_ACTOR_H
#define GRIM_ACTOR_H

#include "engines/grim/pool.h"
#include "engines/grim/object.h"
#include "engines/grim/color.h"
#include "math/vector3d.h"
#include "math/angle.h"
#include "math/quat.h"

namespace Grim {

class TextObject;
class Sector;
class Costume;
class LipSync;
class Font;
class Set;
class Material;
struct SetShadow;
struct Joint;
class EMIModel;

struct Plane {
	Common::String setName;
	Sector *sector;
};

typedef Common::List<Plane> SectorListType;

#define MAX_SHADOWS 8

struct Shadow {
	Shadow();

	Common::String name;
	Math::Vector3d pos;
	SectorListType planeList;
	byte *shadowMask;
	int shadowMaskSize;
	bool active;
	bool dontNegate;
	Color color;
	void *userData;
};

/**
 * @class Actor
 *
 * @short Actor represents a 3D character on screen.
 */
class Actor : public PoolObject<Actor> {
public:
	enum CollisionMode {
		CollisionOff = 0,
		CollisionBox = 1,
		CollisionSphere = 2
	};

	enum AlphaMode {
		AlphaOff = -1,
		AlphaReplace = 2,
		AlphaModulate = 3 // Seems to be unused
	};

	enum LightMode {
		LightStatic = 0,
		LightFastDyn = 1,
		LightNormDyn = 2,
		LightNone = 3
	};

	/**
	 * Builds an actor setting up only the minimal variables.
	 */
	Actor();
	/**
	 * Destroys the actor.
	 * The actor is automatically removed from the GrimEngine instance.
	 */
	~Actor();

	static int32 getStaticTag() { return MKTAG('A', 'C', 'T', 'R'); }

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
	 * Sets the name of the actor.
	 *
	 * @param name The name.
	 */
	void setName(const Common::String &name) { _name = name; }

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
	inline Math::Vector3d getDestPos() const { return _destPos; }

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
	 * @param snap If true tells the actor to increate its turn speed.
	 * @see getPitch
	 * @see getYaw
	 * @see getRoll
	 * @see setRot
	 * @see turn
	 * @see isTurning
	 */
	void turnTo(const Math::Angle &pitch, const Math::Angle &yaw, const Math::Angle &roll, bool snap = false);
	/**
	 * Turn the actor towards a point in space.
	 * The effect is not immediate, the actor will slowly rotate
	 * to the destination orientation.
	 *
	 * @param pos The position the actor should turn to.
	 * @param snap If true tells the actor to increate its turn speed.
	 * @see turnTo
	 * @see setRot
	 */
	void turnTo(const Math::Vector3d &pos, bool snap = false);
	/**
	 * Turn the actor towards a point in space, by an amount defined by the turn rate.
	 * This function does not make an actor point at the given position, but it makes
	 * it rotate towards that.
	 *
	 * @param pos The position the actor should turn to.
	 * @return true if the actor has reached the desired orientation
	 * @see turnTo
	 * @see setRot
	 */
	bool singleTurnTo(const Math::Vector3d &pos);
	/**
	 * Returns true if the actor is turning.
	 *
	 * @see turnTo
	 */
	bool isTurning() const;
	/**
	 * Stops the actor from turning
	 */
	void stopTurning();
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
	 * Turn the actor towards a point in space.
	 * The effect is immediate.
	 *
	 * @param pos The position the actor should turn to.
	 * @see turnTo
	 * @see setRot
	 */
	void setRot(const Math::Vector3d &pos);
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
	Math::Angle  getYawTo(const Actor *actor) const;
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
	float getScale() const { return _scale; }

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
	 * For engine internal use only, do not expose via lua API.
	 *
	 * @param setName The name of the set.
	 */
	bool isDrawableInSet(const Common::String &setName) const;
	/**
	 * Returns true if the actor is in the given set.
	 * Can be exposed via lua API.
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
	void sayLine(const char *msgId, bool background, float x, float y);
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
	void stopAllChores(bool ignoreLoopingChores = false);
	void setColormap(const char *map);
	void pushCostume(const char *name);
	void setCostume(const char *name);
	void popCostume();
	void clearCostumes();
	Costume *getCurrentCostume() const;
	void setLocalAlphaMode(unsigned int vertex, AlphaMode alphamode);
	void setLocalAlpha(unsigned int vertex, float alpha);
	bool hasLocalAlpha() const;
	float getLocalAlpha(unsigned int vertex) const;
	Costume *findCostume(const Common::String &name);
	int getCostumeStackDepth() const {
		return _costumeStack.size();
	}
	const Common::List<Costume *> &getCostumes() const { return _costumeStack; }

	void setActiveShadow(int shadowId);
	void setShadowPoint(const Math::Vector3d &pos);
	void setShadowColor(const Color &color);
	void setShadowPlane(const char *name);
	void addShadowPlane(const char *name);
	void clearShadowPlanes();
	void clearShadowPlane(int i);
	void setShadowValid(int);
	void setActivateShadow(int, bool);

	void setFollowBoxes(bool follow);
	bool hasFollowedBoxes() const { return _hasFollowedBoxes; }
	void update(uint frameTime);
	/**
	 * Check if the actor is still talking. If it is returns true, otherwise false.
	 */
	bool updateTalk(uint frameTime);
	void draw();

	bool isLookAtVectorZero() {
		return _lookAtVector.isZero();
	}
	void setLookAtVectorZero() {
		_lookAtVector.set(0.f, 0.f, 0.f);
		_lookAtActor = 0;
	}
	void setLookAtVector(const Math::Vector3d &vector) {
		_lookAtVector = vector;
		_lookAtActor = 0;
	}
	Math::Vector3d getLookAtVector() {
		return _lookAtVector;
	}
	void setLookAtActor(Actor *other) { _lookAtActor = other->getId(); }
	void setLookAtRate(float rate);
	float getLookAtRate() const;
	void setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw);
	void setHead(const char *joint, const Math::Vector3d &offset);
	void setHeadLimits(float yawRange, float maxPitch, float minPitch);

	void setCollisionMode(CollisionMode mode);
	void setCollisionScale(float scale);

	bool handleCollisionWith(Actor *actor, CollisionMode mode, Math::Vector3d *vec) const;

	static void saveStaticState(SaveGame *state);
	static void restoreStaticState(SaveGame *state);

	bool isAttached() const { return _attachedActor != 0; }
	Math::Vector3d getWorldPos() const;
	void attachToActor(Actor *other, const char *joint);
	void detach();
	Math::Quaternion getRotationQuat() const;
	const Math::Matrix4 getFinalMatrix() const;
	Math::Vector3d getHeadPos() const;

	void setInOverworld(bool inOverworld) { _inOverworld = inOverworld; }
	bool isInOverworld() const { return _inOverworld; }

	float getGlobalAlpha() const { return _globalAlpha; }
	AlphaMode getAlphaMode() const { return _alphaMode; }
	float getEffectiveAlpha() const { return _alphaMode != AlphaOff ? _globalAlpha : 1.f; }
	void setGlobalAlpha(float alpha, const Common::String &mesh);
	void setAlphaMode(AlphaMode mode, const Common::String &mesh);

	int getSortOrder() const;
	void setSortOrder(const int order);
	int getEffectiveSortOrder() const;

	void activateShadow(bool active, const char *shadowName);
	void activateShadow(bool active, SetShadow *shadow);

	void drawToCleanBuffer();

	bool isTalkingForeground() const;

	LightMode getLightMode() const { return _lightMode; }
	void setLightMode(LightMode lightMode) { _lightMode = lightMode; }

	ObjectPtr<Material> loadMaterial(const Common::String &name, bool clamp);
	ObjectPtr<Material> findMaterial(const Common::String &name);

	void getBBoxInfo(Math::Vector3d &bboxPos, Math::Vector3d &bboxSize) const;

private:
	void costumeMarkerCallback(int marker);
	void collisionHandlerCallback(Actor *other) const;
	void updateWalk();
	void addShadowPlane(const char *n, Set *scene, int shadowId);
	bool shouldDrawShadow(int shadowId);
	void stopTalking();
	bool stopMumbleChore();
	void drawCostume(Costume *costume);
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
	void calculateOrientation(const Math::Vector3d &pos, Math::Angle *pitch, Math::Angle *yaw, Math::Angle *roll);

	bool getSphereInfo(bool adjustZ, float &size, Math::Vector3d &pos) const;
	EMIModel *findModelWithMesh(const Common::String &mesh);

	Common::String _name;
	Common::String _setName;    // The actual current set

	Color _talkColor;
	Math::Vector3d _pos;
	Math::Angle _pitch, _yaw, _roll;
	float _walkRate, _turnRate;

	bool _followBoxes;  // Constrain to walkboxes
	bool _hasFollowedBoxes;
	float _reflectionAngle; // Maximum angle to turn by at walls
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
	bool _singleTurning;
	// NOTE: The movement direction is separate from the direction
	// the actor's model is facing. The model's direction is gradually
	// updated to match the movement direction. This produces a smooth
	// turning animation while still allowing the actor to move in a
	// new direction immediately after reflecting off a wall.
	Math::Angle _moveYaw;
	Math::Angle _movePitch;
	Math::Angle _moveRoll;
	// This is used to increase momentarily the turn rate when needed
	float _turnRateMultiplier;

	// Variables for walking to a point
	bool _walking;
	Math::Vector3d _destPos;

	//chores
	class ActionChore {
	public:
		ActionChore();
		ActionChore(Costume *cost, int chore);

		void play(bool fade = false, unsigned int time = fadeTime);
		void playLooping(bool fade = false, unsigned int time = fadeTime);
		void stop(bool fade = false, unsigned int time = fadeTime);
		void setLastFrame();

		inline bool isValid() const { return _chore > -1 && _costume != nullptr; }
		bool isPlaying() const;
		inline bool equals(const Costume *cost, int chore) const {
			return (_costume == cost && _chore == chore);
		}

		void saveState(SaveGame *state) const;
		void restoreState(SaveGame *state, Actor *actor);

		Costume *_costume;
		int _chore;

		static const unsigned int fadeTime;
		static const unsigned int talkFadeTime;
	};
	ActionChore _restChore;

	ActionChore _walkChore;
	bool _walkedLast, _walkedCur;
	bool _running;

	ActionChore _leftTurnChore, _rightTurnChore;
	int _lastTurnDir, _currTurnDir;

	ActionChore _talkChore[10];
	int _talkAnim;

	ActionChore _mumbleChore;

	Shadow *_shadowArray;
	int _activeShadowSlot;

	static ObjectPtr<Font> _sayLineFont;
	int _sayLineText;
	bool _mustPlaceText;

	ActionChore *getTurnChore(int dir) {
		return (dir > 0 ? &_leftTurnChore : &_rightTurnChore);
	}

	void freeCostume(Costume *costume);
	void freeCostumeChore(const Costume *toFree, ActionChore *chore);

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
	int _talkDelay;
	int _attachedActor;
	int _lookAtActor;
	Common::String _attachedJoint;
	AlphaMode _alphaMode;
	float _globalAlpha;

	bool _inOverworld;

	int _sortOrder;
	int _sectorSortOrder;
	bool _useParentSortOrder;

	bool _fakeUnbound;
	bool _drawnToClean;

	LightMode _lightMode;

	Common::List<ObjectPtr<Material> > _materials;

	// Highest vertex used in EMI
	const static unsigned int MAX_LOCAL_ALPHA_VERTICES = 48;
	Common::Array<float> _localAlpha;
	Common::Array<int> _localAlphaMode;
};

} // end of namespace Grim

#endif
