// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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
#include <string>
#include <list>

class Sound;
class Costume;

class Actor {
public:
	Actor(const char *name);

	const char *name() const { return name_.c_str(); }

	void setTalkColor(const Color& c) { talkColor_ = c; }
	Color talkColor() const { return talkColor_; }
	void setPos(Vector3d pos) { pos_ = pos; }
	Vector3d pos() const { return pos_; }
	void walkTo(Vector3d p);
	bool isWalking() const { return walking_; }
	void setRot(float pitch, float yaw, float roll) {
		pitch_ = pitch; yaw_ = yaw; roll_ = roll;
	}
	void turnTo(float pitch, float yaw, float roll);
	bool isTurning() const { return turning_; }
	float pitch() const { return pitch_; }
	float yaw() const { return yaw_; }
	float roll() const { return roll_; }
	void setVisibility(bool val) { visible_ = val; }
	bool visible() const { return visible_; }
	void putInSet(const char *name) { setName_ = name; }
	void setTurnRate(float rate) { turnRate_ = rate; }
	float turnRate() const { return turnRate_; }
	void setWalkRate(float rate) { walkRate_ = rate; }
	float walkRate() const { return walkRate_; }
	void setLooking(bool lookingMode) { lookingMode_ = lookingMode; }

	float angleTo(const Actor &a) const;
	float yawTo(Vector3d p) const;

	bool validBoxVector(Vector3d forwardVec, float dist);

	bool inSet(const char *name) const {
		return setName_ == name;
	}
	void walkForward();
	void turn(int dir);

	void sayLine(const char *msg);
	void shutUp();
	bool talking();

	void setWalkChore(int choreNumber) { walkChore_ = choreNumber; }

	void pushCostume(const char *name);
	void setCostume(const char *name);
	void popCostume();
	void clearCostumes();
	Costume *currentCostume() {
		if (costumeStack_.empty())
			return NULL;
		else
			return costumeStack_.back();
	}
	Costume *findCostume(const char *name);
	int costumeStackDepth() const {
		return costumeStack_.size();
	}

	void update();
	void draw();

	bool isLookAtVectorZero() {
		return lookAtVector_.isZero();
	}
	void setLookAtVectorZero() {
		lookAtVector_.set( 0.f, 0.f, 0.f );
	}
	void setLookAtVector( Vector3d vector ) {
		lookAtVector_ = vector;
	}
	void setLookAtRate( float rate ) {
		lookAtRate_ = rate;
	}
	float lookAtRate() {
		return(lookAtRate_);
	}
	void setHead( int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw);

private:
	std::string name_;
	std::string setName_;
	Color talkColor_;
	Vector3d pos_;
	float pitch_, yaw_, roll_;
	float walkRate_, turnRate_;

	bool visible_;
	bool lookingMode_;
	ResPtr<Sound> talkSound_;
	std::list<Costume *> costumeStack_;

	// Variables for gradual turning
	bool turning_;
	float destYaw_;

	// Variables for walking to a point
	bool walking_;
	Vector3d destPos_;

	// chores
	int walkChore_;

	// lookAt
	Vector3d lookAtVector_;
	float lookAtRate_;

	friend class Engine;
};

#endif
