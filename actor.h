// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef ACTOR_H
#define ACTOR_H

#include "color.h"
#include "vector3d.h"
#include "resource.h"
#include "font.h"

#include <string>
#include <list>

#define DEFAULT_COLORMAP "item.cmp"

class Costume;
class LipSynch;
class TextObject;

class Actor {
public:
	Actor(const char *name);

	const char *name() const { return _name.c_str(); }

	void setTalkColor(const Color& c) { _talkColor = c; }
	Color talkColor() const { return _talkColor; }
	void setPos(Vector3d pos) { _pos = pos; }
	Vector3d pos() const { return _pos; }
	void walkTo(Vector3d p);
	bool isWalking() const;
	void setRot(float pitch, float yaw, float roll) {
		_pitch = pitch; _yaw = yaw; _roll = roll;
	}
	void turnTo(float pitch, float yaw, float roll);
	bool isTurning() const;
	float pitch() const { return _pitch; }
	float yaw() const { return _yaw; }
	float roll() const { return _roll; }
	void setVisibility(bool val) { _visible = val; }
	bool visible() const { return _visible; }
	void putInSet(const char *name) { _setName = name; }
	void setTurnRate(float rate) { _turnRate = rate; }
	float turnRate() const { return _turnRate; }
	void setWalkRate(float rate) { _walkRate = rate; }
	float walkRate() const { return _walkRate; }
	void setLooking(bool lookingMode) { _lookingMode = lookingMode; }

	float angleTo(const Actor &a) const;
	float yawTo(Vector3d p) const;

	bool inSet(const char *name) const {
		return _setName == name;
	}
	void walkForward();
	void setReflection(float angle) { _reflectionAngle = angle; }
	Vector3d puckVector() const;
	void turn(int dir);

	void sayLine(const char *msg, const char *msgId);
	// When we clean all text objects we don't want the actors to clean their
	// objects again since they're already freed
	void lineCleanup() { _sayLineText = NULL; }
	void shutUp();
	bool talking();

	void setRestChore(int choreNumber, Costume *cost);
	void setWalkChore(int choreNumber, Costume *cost);
	void setTurnChores(int left_chore, int right_chore, Costume *cost);
	void setTalkChore(int index, int choreNumber, Costume *cost);
	void setMumbleChore(int choreNumber, Costume *cost);

	void setColormap(const char *map) {
		strcpy(_colormap, map);
	}
	void pushCostume(const char *name);
	void setCostume(const char *name);
	void popCostume();
	void clearCostumes();
	Costume *currentCostume() {
		if (_costumeStack.empty())
			return NULL;
		else
			return _costumeStack.back();
	}
	Costume *findCostume(const char *name);
	int costumeStackDepth() const {
		return _costumeStack.size();
	}
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
		_lookAtVector.set( 0.f, 0.f, 0.f );
	}
	void setLookAtVector(Vector3d vector) {
		_lookAtVector = vector;
	}
	void setLookAtRate(float rate) {
		_lookAtRate = rate;
	}
	float lookAtRate() {
		return _lookAtRate;
	}
	void setHead( int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw);

private:
	std::string _name;
	std::string _setName;
	Color _talkColor;
	Vector3d _pos;
	float _pitch, _yaw, _roll;
	float _walkRate, _turnRate;
	char _colormap[256];

	bool _constrain;	// Constrain to walkboxes
	float _reflectionAngle;	// Maximum angle to turn by at walls
	bool _visible;
	bool _lookingMode;
	std::string _talkSoundName;
	ResPtr<LipSynch> _lipSynch;
	std::list<Costume *> _costumeStack;

	// Variables for gradual turning
	bool _turning;
	float _destYaw;

	// Variables for walking to a point
	bool _walking;
	Vector3d _destPos;

	// chores
	Costume *_restCostume;
	int _restChore;

	Costume *_walkCostume;
	int _walkChore;
	bool _walkedLast, _walkedCur;

	Costume *_turnCostume;
	int _leftTurnChore, _rightTurnChore;
	int _lastTurnDir, _currTurnDir;

	Costume *_talkCostume[10];
	int _talkChore[10];
	int _talkAnim;

	Costume *_mumbleCostume;
	int _mumbleChore;

	static Font *_sayLineFont;
	TextObject *_sayLineText;

	int getTurnChore(int dir) {
		return (dir > 0 ? _rightTurnChore : _leftTurnChore);
	}

	void freeCostumeChore(Costume *toFree, Costume *&cost, int &chore) {
		if (cost == toFree) {
			cost = NULL;
			chore = -1;
		}
	}

	// lookAt
	Vector3d _lookAtVector;
	float _lookAtRate;

	friend class Engine;
};

#endif
