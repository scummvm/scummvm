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

#include "stdafx.h"
#include "actor.h"
#include "engine.h"
#include "costume.h"
#include "sound.h"
#include <cmath>
#include <cstring>
#include "driver_gl.h"

Actor::Actor(const char *name) :
		name_(name), talkColor_(255, 255, 255), pos_(0, 0, 0),
		pitch_(0), yaw_(0), roll_(0), walkRate_(0), turnRate_(0),
		visible_(true), talkSound_(NULL), turning_(false), walking_(false), walkChore_(-1) {
	Engine::instance()->registerActor(this);
	lookingMode_ = false;
}

void Actor::turnTo(float pitch, float yaw, float roll) {
	pitch_ = pitch;
	roll_ = roll;
	if (yaw_ != yaw) {
		turning_ = true;
		destYaw_ = yaw;
	} else
		turning_ = false;
}

void Actor::walkTo(Vector3d p) {
	// For now, this is just the ignoring-boxes version (which afaict
	// isn't even in the original).  This will eventually need a
	// following-boxes version also.
	if (p == pos_)
		walking_ = false;
	else {
		walking_ = true;
		destPos_ = p;

		if (p.x() != pos_.x() || p.y() != pos_.y())
			turnTo(pitch_, yawTo(p), roll_);
	}
}

bool Actor::validBoxVector(Vector3d forwardVec, float dist) {
	//TODO: Obey Actor 'constrain' flags. Verify if any other sector flags allow walking
	//Possibly use a box-aware TraceLine function instead of just checking dest point
	Vector3d tempVec = pos_ + (forwardVec * dist);
	int numSectors = Engine::instance()->currScene()->getSectorCount();

	for (int i = 0; i < numSectors; i++) {
		Sector *sector = Engine::instance()->currScene()->getSectorBase(i);
		if ((sector->type() & 0x1000) && sector->visible()) {
			if (sector->isPointInSector(tempVec))
				return true;
		}
	}
	return false;
}

void Actor::walkForward() {
	float dist = Engine::instance()->perSecond(walkRate_);
	float yaw_rad = yaw_ * (M_PI / 180), pitch_rad = pitch_ * (M_PI / 180);
	Vector3d forwardVec(-std::sin(yaw_rad) * std::cos(pitch_rad),
		std::cos(yaw_rad) * std::cos(pitch_rad),
		std::sin(pitch_rad));
	if (validBoxVector(forwardVec, dist)) 
		pos_ += forwardVec * dist;
}

void Actor::turn(int dir) {
	float delta = Engine::instance()->perSecond(turnRate_) * dir;
	yaw_ += delta;
}

float Actor::angleTo(const Actor &a) const {
	float yaw_rad = yaw_ * (M_PI / 180);
	Vector3d forwardVec(-std::sin(yaw_rad), std::cos(yaw_rad), 0);
	Vector3d delta = a.pos() - pos_;
	delta.z() = 0;
	return angle(forwardVec, delta) * (180 / M_PI);
}

float Actor::yawTo(Vector3d p) const {
	Vector3d dpos = p - pos_;
	if (dpos.x() == 0 && dpos.y() == 0)
		return 0;
	else
		return std::atan2(-dpos.x(), dpos.y()) * (180 / M_PI);
}

void Actor::sayLine(const char *msg) {
	// For now, just play the appropriate sound if found.  Eventually,
	// this needs to handle possibly displaying text, starting up
	// appropriate talking chores, etc.

	// Find the message identifier
	if (msg[0] != '/')
		return;
	const char *secondSlash = std::strchr(msg + 1, '/');
	if (secondSlash == NULL)
		return;
	if (talkSound_) // Only once line at a time, please :)
		shutUp();
	std::string msgId(msg + 1, secondSlash);
	talkSound_ = ResourceLoader::instance()->loadSound((msgId + ".wav").c_str());
	if (talkSound_ != NULL)
		Mixer::instance()->playVoice(talkSound_);

	//FIXME: Ender - Disabled until I work out why the wrong Chores play
	//	if (!costumeStack_.empty()) {
	//	printf("Requesting talk chore\n");
	//	costumeStack_.back()->playTalkChores();
	//}
	}

bool Actor::talking() {
	if (talkSound_ == NULL)
		return false;
	if (talkSound_->done()) {
		//FIXME: Ender - Disabled until I work out why the wrong Chores play
		//if (!costumeStack_.empty())
		//costumeStack_.back()->stopTalkChores();
		talkSound_ = NULL;
		return false;
	}
	return true;
}

void Actor::shutUp() {
	if (talkSound_) {
		Mixer::instance()->stopVoice(talkSound_);
		talkSound_ = NULL;
	}
}

void Actor::pushCostume(const char *name) {
	Costume *newCost = ResourceLoader::instance()->
		loadCostume(name, currentCostume());
	costumeStack_.push_back(newCost);
}

void Actor::setCostume(const char *name) {
	if (! costumeStack_.empty())
		popCostume();
	pushCostume(name);
}

void Actor::popCostume() {
	if (! costumeStack_.empty()) {
		delete costumeStack_.back();
		costumeStack_.pop_back();
	}
}

void Actor::clearCostumes() {
	// Make sure to destroy costume copies in reverse order
	while (! costumeStack_.empty()) {
		delete costumeStack_.back();
		costumeStack_.pop_back();
	}
}

void Actor::setHead( int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw ) {
	if (!costumeStack_.empty()) {
		costumeStack_.back()->setHead( joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
	}
}

Costume *Actor::findCostume(const char *name) {
	for (std::list<Costume *>::iterator i = costumeStack_.begin();
			i != costumeStack_.end(); i++)
		if (std::strcmp((*i)->filename(), name) == 0)
			return *i;
	return NULL;
}

void Actor::update() {
	if (turning_) {
		float turnAmt = Engine::instance()->perSecond(turnRate_);
		float dyaw = destYaw_ - yaw_;
		while (dyaw > 180)
			dyaw -= 360;
		while (dyaw < -180)
			dyaw += 360;
		if (turnAmt >= std::abs(dyaw)) {
			yaw_ = destYaw_;
			turning_ = false;
		}
		else if (dyaw > 0)
			yaw_ += turnAmt;
		else
			yaw_ -= turnAmt;
	}

	if (walking_) {
		Vector3d dir = destPos_ - pos_;
		float dist = dir.magnitude();

		if (dist > 0)
			dir /= dist;

		float walkAmt = Engine::instance()->perSecond(walkRate_);

		if (walkAmt >= dist) {
			pos_ = destPos_;
			walking_ = false;
			turning_ = false;
			}
		else
			pos_ += dir * walkAmt;
	}

	for (std::list<Costume *>::iterator i = costumeStack_.begin();
		i != costumeStack_.end(); i++) {
		(*i)->setPosRotate( pos_, pitch_, yaw_, roll_ );
		(*i)->update();
	}

	if (lookingMode_) {
		float lookAtAmt = Engine::instance()->perSecond(lookAtRate_);
	}
}

void Actor::draw() {
	if (! costumeStack_.empty()) {
		g_driver->startActorDraw(pos_, yaw_, pitch_, roll_);
		costumeStack_.back()->draw();
		g_driver->finishActorDraw();
		}
	}
