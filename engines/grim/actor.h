/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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

#include "engines/grim/object.h"
#include "graphics/vector3d.h"

namespace Grim {

class TextObject;
class Sector;
class Costume;
class LipSync;
class Font;
class Color;

typedef Common::List<Sector *> SectorListType;

extern int g_winX1, g_winY1, g_winX2, g_winY2;

struct Shadow {
	Common::String name;
	Graphics::Vector3d pos;
	SectorListType planeList;
	byte *shadowMask;
	int shadowMaskSize;
	bool active;
	bool dontNegate;
};

class Actor : public Object {
public:
	Actor(const Common::String &name);
	Actor();
	~Actor();

	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	const Common::String &getName() const { return _name; }

	void setTalkColor(Color *c) { _talkColor = c; }
	Color *getTalkColor() const { return _talkColor; }
	void setPos(Graphics::Vector3d position);
	Graphics::Vector3d getPos() const { return _pos; }
	void walkTo(const Graphics::Vector3d &p);
	void stopWalking() { _walking = false; }
	bool isWalking() const;
	void setRot(float pitch, float yaw, float roll);
	void turnTo(float pitch, float yaw, float roll);
	bool isTurning() const;
	float getPitch() const { return _pitch; }
	float getYaw() const { return _yaw; }
	float getRoll() const { return _roll; }
	void setVisibility(bool val) { _visible = val; }
	bool isVisible() const { return _visible; }
	void setScale(float scale) { _scale = scale; }
	// The set should change immediately, otherwise a very rapid set change
	// for an actor will be recognized incorrectly and the actor will be lost.
	void putInSet(const Common::String &setName) { _setName = setName; }
	void setTurnRate(float rate) { _turnRate = rate; }
	float getTurnRate() const { return _turnRate; }
	void setWalkRate(float rate) { _walkRate = rate; }
	float getWalkRate() const { return _walkRate; }
	void setLooking(bool lookingMode) { _lookingMode = lookingMode; }

	float getAngleTo(const Actor &a) const;
	float getYawTo(Graphics::Vector3d p) const;

	bool isInSet(const Common::String &setName) const;
	void walkForward();
	void setRunning(bool running) { _running = running; }
	void setReflection(float angle) { _reflectionAngle = angle; }
	Graphics::Vector3d getPuckVector() const;
	void turn(int dir);

	void sayLine(const char *msg, const char *msgId, bool background);
	// When we clean all text objects we don't want the actors to clean their
	// objects again since they're already freed
	void lineCleanup() { _sayLineText = NULL; }
	void shutUp();
	bool isTalking();

	void setRestChore(int choreNumber, Costume *cost);
	int getRestChore() const { return _restChore; }
	void setWalkChore(int choreNumber, Costume *cost);
	void setTurnChores(int left_chore, int right_chore, Costume *cost);
	void setTalkChore(int index, int choreNumber, Costume *cost);
	int getTalkChore(int index) const { return _talkChore[index]; }
	Costume* getTalkCostume(int index) const { return _talkCostume[index]; }
	void setMumbleChore(int choreNumber, Costume *cost);

	void setColormap(const char *map);
	void pushCostume(const char *name);
	void setCostume(const char *name);
	void popCostume();
	void clearCostumes();
	Costume *getCurrentCostume() {
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
	void setShadowPoint(Graphics::Vector3d pos);
	void setShadowPlane(const char *name);
	void addShadowPlane(const char *name);
	void clearShadowPlanes();
	void setShadowValid(int);
	void setActivateShadow(int, bool);

	void setConstrain(bool constrain) {
		_constrain = constrain;
	}
	void update();
	void draw();
	void undraw(bool);

	bool isLookAtVectorZero() {
		return _lookAtVector.isZero();
	}
	void setLookAtVectorZero() {
		_lookAtVector.set(0.f, 0.f, 0.f);
	}
	void setLookAtVector(Graphics::Vector3d vector) {
		_lookAtVector = vector;
	}
	Graphics::Vector3d getLookAtVector() {
		return _lookAtVector;
	}
	void setLookAtRate(float rate) {
		_lookAtRate = rate;
	}
	float getLookAtRate() {
		return _lookAtRate;
	}
	void setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw);

	bool _toClean;

private:
	enum Footstep {
		LeftWalk = 10,
		RightWalk = 15,
		LeftRun = 20,
		RightRun = 25,
		LeftTurn = 30,
		RightTurn = 35
	};

	void costumeMarkerCallback(Footstep step);
	void updateWalk();

	Common::String _name;
	Common::String _setName;    // The actual current set

	Color *_talkColor;
	Graphics::Vector3d _pos;
	float _pitch, _yaw, _roll;
	float _walkRate, _turnRate;

	bool _constrain;	// Constrain to walkboxes
	float _reflectionAngle;	// Maximum angle to turn by at walls
	bool _visible;
	float _scale;
	bool _lookingMode;
	Common::String _talkSoundName;
	ObjectPtr<LipSync> _lipSync;
	Common::List<Costume *> _costumeStack;

	// Variables for gradual turning
	bool _turning;
	float _destYaw;

	// Variables for walking to a point
	bool _walking;
	Graphics::Vector3d _destPos;

	// chores
	Costume *_restCostume;
	int _restChore;

	Costume *_walkCostume;
	int _walkChore;
	bool _walkedLast, _walkedCur;
	bool _lastWasLeft;
	int _lastStepTime;
	bool _running;

	Costume *_turnCostume;
	int _leftTurnChore, _rightTurnChore;
	int _lastTurnDir, _currTurnDir;

	Costume *_talkCostume[10];
	int _talkChore[10];
	int _talkAnim;

	Costume *_mumbleCostume;
	int _mumbleChore;

	Shadow *_shadowArray;
	int _activeShadowSlot;

	static ObjectPtr<Font> _sayLineFont;
	TextObject *_sayLineText;

	// Validate a yaw angle then set it appropriately
	void setYaw(float yaw);

	int getTurnChore(int dir) {
		return (dir > 0 ? _rightTurnChore : _leftTurnChore);
	}

	void freeCostumeChore(Costume *toFree, Costume *&cost, int &chore);

	// lookAt
	Graphics::Vector3d _lookAtVector;
	float _lookAtRate;

	int _winX1, _winY1, _winX2, _winY2;

	// struct used for path finding
	struct PathNode {
		Sector *sect;
		PathNode *parent;
		Graphics::Vector3d pos;
		float dist;
		float cost;
	};
	Common::List<Graphics::Vector3d> _path;

	friend class GrimEngine;
};

} // end of namespace Grim

#endif
