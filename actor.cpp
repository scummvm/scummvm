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
#include "localize.h"
#include <cmath>
#include <cstring>
#include "driver_gl.h"

Actor::Actor(const char *name) :
		name_(name), talkColor_(255, 255, 255), pos_(0, 0, 0),
		pitch_(0), yaw_(0), roll_(0), walkRate_(0), turnRate_(0),
		visible_(true), talkSound_(NULL), turning_(false), walking_(false),
		restCostume_(NULL), restChore_(-1),
		walkCostume_(NULL), walkChore_(-1), walkedLast_(false), walkedCur_(-1),
		turnCostume_(NULL), leftTurnChore_(-1), rightTurnChore_(-1),
		lastTurnDir_(0), currTurnDir_(0),
		mumbleCostume_(NULL), mumbleChore_(-1) {
	Engine::instance()->registerActor(this);
	lookingMode_ = false;
	constrain_ = false;

	for (int i = 0; i < 10; i++) {
		talkCostume_[i] = NULL;
		talkChore_[i] = -1;
	}
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

bool Actor::isWalking() const {
	return walkedLast_ || walkedCur_ || walking_;
}

bool Actor::isTurning() const {
	if (turning_)
		return true;
	if (lastTurnDir_ != 0 || currTurnDir_ != 0)
		return true;
	return false;
}

void Actor::walkForward() {
	float dist = Engine::instance()->perSecond(walkRate_);
	float yaw_rad = yaw_ * (M_PI / 180), pitch_rad = pitch_ * (M_PI / 180);
	Vector3d forwardVec(-std::sin(yaw_rad) * std::cos(pitch_rad),
		std::cos(yaw_rad) * std::cos(pitch_rad),
		std::sin(pitch_rad));
	Vector3d destPos = pos_ + forwardVec * dist;

	if (! constrain_) {
		pos_ = destPos;
		walkedCur_ = true;
	}
	else {
		Sector *sector = Engine::instance()->currScene()->findPointSector(destPos, 0x1000);
		if (sector != NULL) {
			pos_ = sector->projectToPlane(destPos);
			walkedCur_ = true;
		}
	}
}

Vector3d Actor::puckVector() const {
	float yaw_rad = yaw_ * (M_PI / 180);
	Vector3d forwardVec(-std::sin(yaw_rad), std::cos(yaw_rad), 0);

	Sector *sector = Engine::instance()->currScene()->findPointSector(pos_, 0x1000);
	if (sector == NULL)
		return forwardVec;
	else
		return sector->projectToPuckVector(forwardVec);
}

void Actor::setRestChore(int chore, Costume *cost) {
	if (restCostume_ == cost && restChore_ == chore)
		return;
	if (restChore_ >= 0)
		restCostume_->stopChore(restChore_);
	restCostume_ = cost;
	restChore_ = chore;
	restCostume_->playChoreLooping(restChore_);
}

void Actor::setWalkChore(int chore, Costume *cost) {
	if (walkCostume_ == cost && walkChore_ == chore)
		return;
	if (walkChore_ >= 0)
		walkCostume_->stopChore(walkChore_);
	walkCostume_ = cost;
	walkChore_ = chore;
}

void Actor::setTurnChores(int left_chore, int right_chore, Costume *cost) {
	if (turnCostume_ == cost && leftTurnChore_ == left_chore &&
	    rightTurnChore_ == right_chore)
		return;
	if (leftTurnChore_ >= 0) {
		turnCostume_->stopChore(leftTurnChore_);
		turnCostume_->stopChore(rightTurnChore_);
	}
	turnCostume_ = cost;
	leftTurnChore_ = left_chore;
	rightTurnChore_ = right_chore;

	if ((left_chore >= 0 && right_chore < 0) ||
	    (left_chore < 0 && right_chore >= 0))
		error("Unexpectedly got only one turn chore\n");
}

void Actor::setTalkChore(int index, int chore, Costume *cost) {
	if (index < 1 || index > 10)
		error("Got talk chore index out of range (%d)\n", index);
	index--;
	if (talkCostume_[index] == cost && talkChore_[index] == chore)
		return;
	if (talkChore_[index] >= 0)
		talkCostume_[index]->stopChore(talkChore_[index]);
	talkCostume_[index] = cost;
	talkChore_[index] = chore;
}

void Actor::setMumbleChore(int chore, Costume *cost) {
	if (mumbleChore_ >= 0)
		mumbleCostume_->stopChore(mumbleChore_);
	mumbleCostume_ = cost;
	mumbleChore_ = chore;
}

void Actor::turn(int dir) {
	float delta = Engine::instance()->perSecond(turnRate_) * dir;
	yaw_ += delta;
	currTurnDir_ = dir;
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
	std::string msgText = Localizer::instance()->localize(secondSlash + 1);
 	std::string msgId(msg + 1, secondSlash);
	talkSound_ = ResourceLoader::instance()->loadSound((msgId + ".wav").c_str());
	if (talkSound_ != NULL) {
		Mixer::instance()->playVoice(talkSound_);
		if (mumbleChore_ >= 0)
			mumbleCostume_->playChoreLooping(mumbleChore_);
	}
}

bool Actor::talking() {
	return (talkSound_ != NULL && ! talkSound_->done());
}

void Actor::shutUp() {
	if (talkSound_) {
		Mixer::instance()->stopVoice(talkSound_);
		talkSound_ = NULL;
		if (mumbleChore_ >= 0)
			mumbleCostume_->stopChore(mumbleChore_);
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
		freeCostumeChore(costumeStack_.back(), restCostume_, restChore_);
		freeCostumeChore(costumeStack_.back(), walkCostume_, walkChore_);
		if (turnCostume_ == costumeStack_.back()) {
			turnCostume_ = NULL;
			leftTurnChore_ = -1;
			rightTurnChore_ = -1;
		}
		freeCostumeChore(costumeStack_.back(), mumbleCostume_, mumbleChore_);
		for (int i = 0; i < 10; i++)
			freeCostumeChore(costumeStack_.back(), talkCostume_[i], talkChore_[i]);
		delete costumeStack_.back();
		costumeStack_.pop_back();
	}
}

void Actor::clearCostumes() {
	// Make sure to destroy costume copies in reverse order
	while (! costumeStack_.empty())
		popCostume();
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
		currTurnDir_ = (dyaw > 0 ? 1 : -1);
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

		walkedCur_ = true;
	}

	// The rest chore might have been stopped because of a
	// StopActorChore(nil).  Restart it if so.
	if (restChore_ >= 0 && restCostume_->isChoring(restChore_, false) < 0)
		restCostume_->playChoreLooping(restChore_);

	if (walkChore_ >= 0) {
		if (walkedCur_) {
			if (walkCostume_->isChoring(walkChore_, false) < 0)
				walkCostume_->playChoreLooping(walkChore_);
		}
		else {
			if (walkCostume_->isChoring(walkChore_, false) >= 0)
				walkCostume_->stopChore(walkChore_);
		}
	}

	if (leftTurnChore_ >= 0) {
		if (walkedCur_)
			currTurnDir_ = 0;
		if (lastTurnDir_ != 0 && lastTurnDir_ != currTurnDir_)
			turnCostume_->stopChore(getTurnChore(lastTurnDir_));
		if (currTurnDir_ != 0 && currTurnDir_ != lastTurnDir_)
			turnCostume_->playChoreLooping(getTurnChore(currTurnDir_));
	}
	else
		currTurnDir_ = 0;
	walkedLast_ = walkedCur_;
	walkedCur_ = false;
	lastTurnDir_ = currTurnDir_;
	currTurnDir_ = 0;

	if (talkSound_ != NULL && talkSound_->done()) {
		talkSound_ = NULL;
		if (mumbleChore_ >= 0)
			mumbleCostume_->stopChore(mumbleChore_);
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
	for (std::list<Costume *>::iterator i = costumeStack_.begin();
	     i != costumeStack_.end(); i++)
		(*i)->setupTextures();

	if (! costumeStack_.empty()) {
		g_driver->startActorDraw(pos_, yaw_, pitch_, roll_);
		costumeStack_.back()->draw();
		g_driver->finishActorDraw();
		}
	}
