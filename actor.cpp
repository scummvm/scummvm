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

#include "stdafx.h"
#include "actor.h"
#include "engine.h"
#include "costume.h"
#include "lipsynch.h"
#include "localize.h"
#include "driver.h"
#include "smush.h"

#include "mixer/mixer.h"

#include "imuse/imuse.h"

#include <cmath>
#include <cstring>

Actor::Actor(const char *name) :
		_name(name), _talkColor(255, 255, 255), _pos(0, 0, 0),
		_pitch(0), _yaw(0), _roll(0), _walkRate(0), _turnRate(0),
		_reflectionAngle(80),
		_visible(true), _lipSynch(NULL), _turning(false), _walking(false),
		_restCostume(NULL), _restChore(-1),
		_walkCostume(NULL), _walkChore(-1), _walkedLast(false), _walkedCur(false),
		_turnCostume(NULL), _leftTurnChore(-1), _rightTurnChore(-1),
		_lastTurnDir(0), _currTurnDir(0),
		_mumbleCostume(NULL), _mumbleChore(-1), _sayLineText(NULL) {
	g_engine->registerActor(this);
	_lookingMode = false;
	_constrain = false;
	_talkSoundName = "";

	strcpy(_colormap, DEFAULT_COLORMAP);
	for (int i = 0; i < 10; i++) {
		_talkCostume[i] = NULL;
		_talkChore[i] = -1;
	}
}

void Actor::turnTo(float pitch, float yaw, float roll) {
	_pitch = pitch;
	_roll = roll;
	if (_yaw != yaw) {
		_turning = true;
		_destYaw = yaw;
	} else
		_turning = false;
}

void Actor::walkTo(Vector3d p) {
	// For now, this is just the ignoring-boxes version (which afaict
	// isn't even in the original).  This will eventually need a
	// following-boxes version also.
	if (p == _pos)
		_walking = false;
	else {
		_walking = true;
		_destPos = p;

		if (p.x() != _pos.x() || p.y() != _pos.y())
			turnTo(_pitch, yawTo(p), _roll);
	}
}

bool Actor::isWalking() const {
	return _walkedLast || _walkedCur || _walking;
}

bool Actor::isTurning() const {
	if (_turning)
		return true;

	if (_lastTurnDir != 0 || _currTurnDir != 0)
		return true;

	return false;
}

void Actor::walkForward() {
	float dist = g_engine->perSecond(_walkRate);
	float yaw_rad = _yaw * (M_PI / 180), pitch_rad = _pitch * (M_PI / 180);
	Vector3d forwardVec(-std::sin(yaw_rad) * std::cos(pitch_rad),
		std::cos(yaw_rad) * std::cos(pitch_rad),
		std::sin(pitch_rad));
	Vector3d destPos = _pos + forwardVec * dist;

	if (! _constrain) {
		_pos += forwardVec * dist;
		_walkedCur = true;
		return;
	}

	if (dist < 0) {
		dist = -dist;
		forwardVec = -forwardVec;
	}

	Sector *currSector = NULL, *prevSector = NULL;
	Sector::ExitInfo ei;

	g_engine->currScene()->findClosestSector(_pos, &currSector, &_pos);
	if (currSector == NULL) { // Shouldn't happen...
		_pos += forwardVec * dist;
		_walkedCur = true;
		return;
	}

	while (currSector != NULL) {
		prevSector = currSector;
		Vector3d puckVector = currSector->projectToPuckVector(forwardVec);
		puckVector /= puckVector.magnitude();
		currSector->getExitInfo(_pos, puckVector, &ei);
		float exitDist = (ei.exitPoint - _pos).magnitude();
		if (dist < exitDist) {
			_pos += puckVector * dist;
			_walkedCur = true;
			return;
		}
		_pos = ei.exitPoint;
		dist -= exitDist;
		if (exitDist > 0.0001)
			_walkedCur = true;

		// Check for an adjacent sector which can continue
		// the path
		currSector = g_engine->currScene()->findPointSector(ei.exitPoint + (float)0.0001 * puckVector, 0x1000);
		if (currSector == prevSector)
			break;
	}

	ei.angleWithEdge *= (float)(180.0 / M_PI);
	int turnDir = 1;
	if (ei.angleWithEdge > 90) {
		ei.angleWithEdge = 180 - ei.angleWithEdge;
		ei.edgeDir = -ei.edgeDir;
		turnDir = -1;
	}
	if (ei.angleWithEdge > _reflectionAngle)
		return;

	ei.angleWithEdge += (float)0.1;
	float turnAmt = g_engine->perSecond(_turnRate);
	if (turnAmt > ei.angleWithEdge)
		turnAmt = ei.angleWithEdge;
	_yaw += turnAmt * turnDir;
}

Vector3d Actor::puckVector() const {
	float yaw_rad = _yaw * (M_PI / 180);
	Vector3d forwardVec(-std::sin(yaw_rad), std::cos(yaw_rad), 0);

	Sector *sector = g_engine->currScene()->findPointSector(_pos, 0x1000);
	if (sector == NULL)
		return forwardVec;
	else
		return sector->projectToPuckVector(forwardVec);
}

void Actor::setRestChore(int chore, Costume *cost) {
	if (_restCostume == cost && _restChore == chore)
		return;

	if (_restChore >= 0)
		_restCostume->stopChore(_restChore);

	_restCostume = cost;
	_restChore = chore;

	if (_restChore >= 0)
		_restCostume->playChoreLooping(_restChore);
}

void Actor::setWalkChore(int chore, Costume *cost) {
	if (_walkCostume == cost && _walkChore == chore)
		return;

	if (_walkChore >= 0)
		_walkCostume->stopChore(_walkChore);

	_walkCostume = cost;
	_walkChore = chore;
}

void Actor::setTurnChores(int left_chore, int right_chore, Costume *cost) {
	if (_turnCostume == cost && _leftTurnChore == left_chore &&
	    _rightTurnChore == right_chore)
		return;

	if (_leftTurnChore >= 0) {
		_turnCostume->stopChore(_leftTurnChore);
		_turnCostume->stopChore(_rightTurnChore);
	}

	_turnCostume = cost;
	_leftTurnChore = left_chore;
	_rightTurnChore = right_chore;

	if ((left_chore >= 0 && right_chore < 0) || (left_chore < 0 && right_chore >= 0))
		error("Unexpectedly got only one turn chore\n");
}

void Actor::setTalkChore(int index, int chore, Costume *cost) {
	if (index < 1 || index > 10)
		error("Got talk chore index out of range (%d)\n", index);

	index--;

	if (_talkCostume[index] == cost && _talkChore[index] == chore)
		return;

	if (_talkChore[index] >= 0)
		_talkCostume[index]->stopChore(_talkChore[index]);

	_talkCostume[index] = cost;
	_talkChore[index] = chore;
}

void Actor::setMumbleChore(int chore, Costume *cost) {
	if (_mumbleChore >= 0)
		_mumbleCostume->stopChore(_mumbleChore);

	_mumbleCostume = cost;
	_mumbleChore = chore;
}

void Actor::turn(int dir) {
	float delta = g_engine->perSecond(_turnRate) * dir;
	_yaw += delta;
	_currTurnDir = dir;
}

float Actor::angleTo(const Actor &a) const {
	float yaw_rad = _yaw * (M_PI / 180);
	Vector3d forwardVec(-std::sin(yaw_rad), std::cos(yaw_rad), 0);
	Vector3d delta = a.pos() - _pos;
	delta.z() = 0;

	return angle(forwardVec, delta) * (180 / M_PI);
}

float Actor::yawTo(Vector3d p) const {
	Vector3d dpos = p - _pos;

	if (dpos.x() == 0 && dpos.y() == 0)
		return 0;
	else
		return std::atan2(-dpos.x(), dpos.y()) * (180 / M_PI);
}

void Actor::sayLine(const char *msg, const char *msgId) {
	assert(msg);
	assert(msgId);

	std::string textName = msgId;
	textName += ".txt";

	if (msg[0] != '/')
		warning("Actor::sayLine: Invalid source message (should be an ID)!");

	if (msgId[0] == 0) {
		error("Actor::sayLine: No message ID for text!");
		return;
	}

	// During movies, SayLine is called for text display only
	if (!g_smush->isPlaying()) {
		
		std::string soundName = msgId;
		std::string soundLip = msgId;
		soundName += ".wav";
		soundLip += ".lip";

		if (_talkSoundName == soundName)
			return;

		if (g_imuse->getSoundStatus(_talkSoundName.c_str()))
			shutUp();

		_talkSoundName = soundName;
		g_imuse->startVoice(_talkSoundName.c_str());
		if (g_engine->currScene()) {
			g_engine->currScene()->setSoundPosition(_talkSoundName.c_str(), pos());
		}

		// If the actor is clearly not visible then don't try to play the lip synch
		if (visible()) {
			// Sometimes actors speak offscreen before they, including their
			// talk chores are initialized.
			// For example, when reading the work order (a LIP file exists for no reason).
			// Also, some lip synch files have no entries
			// In these cases, revert to using the mumble chore.
			_lipSynch = g_resourceloader->loadLipSynch(soundLip.c_str());

			_talkAnim = -1;
		}
	}

	if (_sayLineText) {
		g_engine->killTextObject(_sayLineText);
		delete _sayLineText;
		_sayLineText = NULL;
	}

	_sayLineText = new TextObject();
	_sayLineText->setDefaults(&sayLineDefaults);
	_sayLineText->setText((char *)msg);
	_sayLineText->setFGColor(&_talkColor);
	// if the actor isn't visible render their text at the bottom
	// of the screen
	if (!visible() || !inSet(g_engine->currScene()->name())) {
		_sayLineText->setX(640 / 2);
		_sayLineText->setY(440);
	} else {
		// render at the top for active actors for now
		_sayLineText->setX(640 / 2);
		_sayLineText->setY(0);
	}
	_sayLineText->createBitmap();
	g_engine->registerTextObject(_sayLineText);
}

bool Actor::talking() {
	return g_imuse->getSoundStatus(_talkSoundName.c_str());
}

void Actor::shutUp() {
	g_imuse->stopSound(_talkSoundName.c_str());
	_talkSoundName = "";
	if (_lipSynch != NULL) {
		if ((_talkAnim != -1) && (_talkChore[_talkAnim] >= 0))
			_talkCostume[_talkAnim]->stopChore(_talkChore[_talkAnim]);
		_lipSynch = NULL;
	} else if (_mumbleChore >= 0) {
		_mumbleCostume->stopChore(_mumbleChore);
	}

	if (_sayLineText != NULL) {
		g_engine->killTextObject(_sayLineText);
		delete _sayLineText;
		_sayLineText = NULL;
	}
}

void Actor::pushCostume(const char *name) {
	Costume *newCost = g_resourceloader->loadCostume(name, currentCostume());
	
	newCost->setColormap(_colormap);
	_costumeStack.push_back(newCost);
}

void Actor::setCostume(const char *name) {
	if (!_costumeStack.empty())
		popCostume();

	pushCostume(name);
}

void Actor::popCostume() {
	if (!_costumeStack.empty()) {
		freeCostumeChore(_costumeStack.back(), _restCostume, _restChore);
		freeCostumeChore(_costumeStack.back(), _walkCostume, _walkChore);

		if (_turnCostume == _costumeStack.back()) {
			_turnCostume = NULL;
			_leftTurnChore = -1;
			_rightTurnChore = -1;
		}

		freeCostumeChore(_costumeStack.back(), _mumbleCostume, _mumbleChore);
		for (int i = 0; i < 10; i++)
			freeCostumeChore(_costumeStack.back(), _talkCostume[i], _talkChore[i]);
		delete _costumeStack.back();
		_costumeStack.pop_back();
	}
}

void Actor::clearCostumes() {
	// Make sure to destroy costume copies in reverse order
	while (!_costumeStack.empty())
		popCostume();
}

void Actor::setHead( int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw ) {
	if (!_costumeStack.empty()) {
		_costumeStack.back()->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
	}
}

Costume *Actor::findCostume(const char *name) {
	for (std::list<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); i++)
		if (std::strcmp((*i)->filename(), name) == 0)
			return *i;

	return NULL;
}

void Actor::update() {
	// Snap actor to walkboxes if following them.  This might be
	// necessary for example after activating/deactivating
	// walkboxes, etc.
	if (_constrain && !_walking) {
		g_engine->currScene()->findClosestSector(_pos, NULL, &_pos);
	}

	if (_turning) {
		float turnAmt = g_engine->perSecond(_turnRate);
		float dyaw = _destYaw - _yaw;
		while (dyaw > 180)
			dyaw -= 360;
		while (dyaw < -180)
			dyaw += 360;
		if (turnAmt >= std::abs(dyaw)) {
			_yaw = _destYaw;
			_turning = false;
		}
		else if (dyaw > 0)
			_yaw += turnAmt;
		else
			_yaw -= turnAmt;
		_currTurnDir = (dyaw > 0 ? 1 : -1);
	}

	if (_walking) {
		Vector3d dir = _destPos - _pos;
		float dist = dir.magnitude();

		if (dist > 0)
			dir /= dist;

		float walkAmt = g_engine->perSecond(_walkRate);

		if (walkAmt >= dist) {
			_pos = _destPos;
			_walking = false;
			_turning = false;
		} else
			_pos += dir * walkAmt;

		_walkedCur = true;
	}

	// The rest chore might have been stopped because of a
	// StopActorChore(nil).  Restart it if so.
	if (_restChore >= 0 && _restCostume->isChoring(_restChore, false) < 0)
		_restCostume->playChoreLooping(_restChore);

	if (_walkChore >= 0) {
		if (_walkedCur) {
			if (_walkCostume->isChoring(_walkChore, false) < 0)
				_walkCostume->playChoreLooping(_walkChore);
		} else {
			if (_walkCostume->isChoring(_walkChore, false) >= 0)
				_walkCostume->stopChore(_walkChore);
		}
	}

	if (_leftTurnChore >= 0) {
		if (_walkedCur)
			_currTurnDir = 0;
		if (_lastTurnDir != 0 && _lastTurnDir != _currTurnDir)
			_turnCostume->stopChore(getTurnChore(_lastTurnDir));
		if (_currTurnDir != 0 && _currTurnDir != _lastTurnDir)
			_turnCostume->playChore(getTurnChore(_currTurnDir));
	} else
		_currTurnDir = 0;

	_walkedLast = _walkedCur;
	_walkedCur = false;
	_lastTurnDir = _currTurnDir;
	_currTurnDir = 0;

	// Update lip synching
	if (_lipSynch != NULL) {
		int posSound = g_imuse->getPosIn60HzTicks(_talkSoundName.c_str());
		if (posSound != -1) {
			int anim = _lipSynch->getAnim(posSound);
			if (_talkAnim != anim) {
				if (_talkAnim != -1 && _talkChore[_talkAnim] >= 0)
					_talkCostume[_talkAnim]->stopChore(_talkChore[_talkAnim]);
				if (anim != -1) {
					_talkAnim = anim;
					if (_talkChore[_talkAnim] >= 0) {
						_talkCostume[_talkAnim]->playChoreLooping(_talkChore[_talkAnim]);
					}
				}
			}
		}
	}

	for (std::list<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); i++) {
		(*i)->setPosRotate(_pos, _pitch, _yaw, _roll);
		(*i)->update();
	}

	if (_lookingMode) {
		/*float lookAtAmt = */g_engine->perSecond(_lookAtRate);
	}
}

void Actor::draw() {
	for (std::list<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); i++)
		(*i)->setupTextures();

	if (!_costumeStack.empty()) {
		g_driver->startActorDraw(_pos, _yaw, _pitch, _roll);
		_costumeStack.back()->draw();
		g_driver->finishActorDraw();
	}
}

// "Undraw objects" (handle objects for actors that may not be on screen)
void Actor::undraw(bool visible) {
	if (!talking() || !g_imuse->isVoicePlaying())
		shutUp();
}
