/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "engines/grim/actor.h"
#include "engines/grim/grim.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/smush/smush.h"
#include "engines/grim/imuse/imuse.h"

namespace Grim {

int g_winX1, g_winY1, g_winX2, g_winY2;

Actor::Actor(const char *actorName) :
		_name(actorName), _setName(""), _talkColor(255, 255, 255), _pos(0, 0, 0),
		// Some actors don't set walk and turn rates, so we default the
		// _turnRate so Doug at the cat races can turn and we set the
		// _walkRate so Glottis at the demon beaver entrance can walk
		_pitch(0), _yaw(0), _roll(0), _walkRate(1.0f), _turnRate(100.0f),
		_reflectionAngle(80),
		_visible(true), _lipSync(NULL), _turning(false), _walking(false),
		_restCostume(NULL), _restChore(-1),
		_walkCostume(NULL), _walkChore(-1), _walkedLast(false), _walkedCur(false),
		_turnCostume(NULL), _leftTurnChore(-1), _rightTurnChore(-1),
		_lastTurnDir(0), _currTurnDir(0),
		_mumbleCostume(NULL), _mumbleChore(-1), _sayLineText(NULL) {
	g_grim->registerActor(this);
	_lookingMode = false;
	_constrain = false;
	_talkSoundName = "";
	_activeShadowSlot = -1;
	_shadowArray = new Shadow[5];
	_winX1 = _winY1 = 1000;
	_winX2 = _winY2 = -1000;

	for (int i = 0; i < 5; i++) {
		_shadowArray[i].active = false;
		_shadowArray[i].dontNegate = false;
		_shadowArray[i].shadowMask = NULL;
	}

	for (int i = 0; i < 10; i++) {
		_talkCostume[i] = NULL;
		_talkChore[i] = -1;
	}
}

Actor::~Actor() {
	clearShadowPlanes();
	delete[] _shadowArray;
}

void Actor::saveState(SaveGame *savedState) {
	int32 size;

	// store actor name
	size = strlen(name());
	savedState->writeLESint32(size);
	savedState->write(name(), size);

}

void Actor::setYaw(float yawParam) {
	// While the program correctly handle yaw angles outside
	// of the range [0, 360), proper convention is to roll
	// these values over correctly
	if (yawParam >= 360.0)
		_yaw = yawParam - 360;
	else if (yawParam < 0.0)
		_yaw = yawParam + 360;
	else
		_yaw = yawParam;
}

void Actor::setRot(float pitchParam, float yawParam, float rollParam) {
	_pitch = pitchParam;
	setYaw(yawParam);
	_roll = rollParam;
}

void Actor::turnTo(float pitchParam, float yawParam, float rollParam) {
	_pitch = pitchParam;
	_roll = rollParam;
	if (_yaw != yawParam) {
		_turning = true;
		_destYaw = yawParam;
	} else
		_turning = false;
}

void Actor::walkTo(Graphics::Vector3d p) {
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
	float dist = g_grim->perSecond(_walkRate);
	float yaw_rad = _yaw * (LOCAL_PI / 180), pitch_rad = _pitch * (LOCAL_PI / 180);
	//float yaw;
	Graphics::Vector3d forwardVec(-sin(yaw_rad) * cos(pitch_rad),
		cos(yaw_rad) * cos(pitch_rad), sin(pitch_rad));
	Graphics::Vector3d destPos = _pos + forwardVec * dist;

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

	g_grim->currScene()->findClosestSector(_pos, &currSector, &_pos);
	if (!currSector) { // Shouldn't happen...
		_pos += forwardVec * dist;
		_walkedCur = true;
		return;
	}

	while (currSector) {
		prevSector = currSector;
		Graphics::Vector3d puckVec = currSector->projectToPuckVector(forwardVec);
		puckVec /= puckVec.magnitude();
		currSector->getExitInfo(_pos, puckVec, &ei);
		float exitDist = (ei.exitPoint - _pos).magnitude();
		if (dist < exitDist) {
			_pos += puckVec * dist;
			_walkedCur = true;
			return;
		}
		_pos = ei.exitPoint;
		dist -= exitDist;
		if (exitDist > 0.0001)
			_walkedCur = true;

		// Check for an adjacent sector which can continue
		// the path
		currSector = g_grim->currScene()->findPointSector(ei.exitPoint + (float)0.0001 * puckVec, 0x1000);
		if (currSector == prevSector)
			break;
	}

	ei.angleWithEdge *= (float)(180 / LOCAL_PI);
	int turnDir = 1;
	if (ei.angleWithEdge > 90) {
		ei.angleWithEdge = 180 - ei.angleWithEdge;
		ei.edgeDir = -ei.edgeDir;
		turnDir = -1;
	}
	if (ei.angleWithEdge > _reflectionAngle)
		return;

	ei.angleWithEdge += (float)0.1;
	float turnAmt = g_grim->perSecond(_turnRate);
	if (turnAmt > ei.angleWithEdge)
		turnAmt = ei.angleWithEdge;
	setYaw(_yaw + turnAmt * turnDir);
}

Graphics::Vector3d Actor::puckVector() const {
	float yaw_rad = _yaw * (LOCAL_PI / 180);
	Graphics::Vector3d forwardVec(-sin(yaw_rad), cos(yaw_rad), 0);

	Sector *sector = g_grim->currScene()->findPointSector(_pos, 0x1000);
	if (!sector)
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
		error("Unexpectedly got only one turn chore");
}

void Actor::setTalkChore(int index, int chore, Costume *cost) {
	if (index < 1 || index > 10)
		error("Got talk chore index out of range (%d)", index);

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
	float delta = g_grim->perSecond(_turnRate) * dir;
	setYaw(_yaw + delta);
	_currTurnDir = dir;
}

float Actor::angleTo(const Actor &a) const {
	float yaw_rad = _yaw * (LOCAL_PI / 180);
	Graphics::Vector3d forwardVec(-sin(yaw_rad), cos(yaw_rad), 0);
	Graphics::Vector3d delta = a.pos() - _pos;
	delta.z() = 0;

	return angle(forwardVec, delta) * (180 / LOCAL_PI);
}

float Actor::yawTo(Graphics::Vector3d p) const {
	Graphics::Vector3d dpos = p - _pos;

	if (dpos.x() == 0 && dpos.y() == 0)
		return 0;
	else
		return atan2(-dpos.x(), dpos.y()) * (180 / LOCAL_PI);
}

void Actor::sayLine(const char *msg, const char *msgId) {
	assert(msg);
	assert(msgId);

	Common::String textName(msgId);
	textName += ".txt";

	if (msgId[0] == 0) {
		error("Actor::sayLine: No message ID for text");
		return;
	}

	// During Fullscreen movies SayLine is called for text display only
	// However, normal SMUSH movies may call SayLine, for example:
	// When Domino yells at Manny (a SMUSH movie) he does it with
	// a SayLine request rather than as part of the movie!
	if (!g_smush->isPlaying() || g_grim->getMode() == ENGINE_MODE_NORMAL) {
		Common::String soundName = msgId;
		Common::String soundLip = msgId;
		soundName += ".wav";
		soundLip += ".lip";

		if (_talkSoundName == soundName)
			return;

		if (g_imuse->getSoundStatus(_talkSoundName.c_str()) || msg[0] == 0)
			shutUp();

		_talkSoundName = soundName;
		g_imuse->startVoice(_talkSoundName.c_str());
		if (g_grim->currScene()) {
			g_grim->currScene()->setSoundPosition(_talkSoundName.c_str(), pos());
		}

		// If the actor is clearly not visible then don't try to play the lip sync
		if (visible()) {
			// Sometimes actors speak offscreen before they, including their
			// talk chores are initialized.
			// For example, when reading the work order (a LIP file exists for no reason).
			// Also, some lip sync files have no entries
			// In these cases, revert to using the mumble chore.
			_lipSync = g_resourceloader->loadLipSync(soundLip.c_str());
			// If there's no lip sync file then load the mumble chore if it exists
			// (the mumble chore doesn't exist with the cat races announcer)
			if (!_lipSync && _mumbleChore != -1)
				_mumbleCostume->playChoreLooping(_mumbleChore);

			_talkAnim = -1;
		}
	}

	if (_sayLineText) {
		g_grim->killTextObject(_sayLineText);
		_sayLineText = NULL;
	}

	if (!sayLineDefaults.font)
		return;

	_sayLineText = new TextObject(false);
	_sayLineText->setDefaults(&sayLineDefaults);
	_sayLineText->setText((char *)msg);
	_sayLineText->setFGColor(&_talkColor);
	if (g_grim->getMode() == ENGINE_MODE_SMUSH) {
		_sayLineText->setX(640 / 2);
		_sayLineText->setY(420);
	} else {
		if (_winX1 == 1000 || _winX2 == -1000 || _winY2 == -1000) {
			_sayLineText->setX(640 / 2);
			_sayLineText->setY(0);
		} else {
			_sayLineText->setX((_winX1 + _winX2) / 2);
			_sayLineText->setY(_winY1);
		}
	}
	_sayLineText->createBitmap();
	g_grim->registerTextObject(_sayLineText);
}

bool Actor::talking() {
	// If there's no sound file then we're obviously not talking
	if (strlen(_talkSoundName.c_str()) == 0)
		return false;

	return g_imuse->getSoundStatus(_talkSoundName.c_str());
}

void Actor::shutUp() {
	// While the call to stop the sound is usually made by the game,
	// we also need to handle when the user terminates the dialog.
	// Some warning messages will occur when the user terminates the
	// actor dialog but the game will continue alright.
	if (_talkSoundName != "") {
		g_imuse->stopSound(_talkSoundName.c_str());
		_talkSoundName = "";
	}
	if (_lipSync) {
		if (_talkAnim != -1 && _talkChore[_talkAnim] >= 0)
			_talkCostume[_talkAnim]->stopChore(_talkChore[_talkAnim]);
		_lipSync = NULL;
	} else if (_mumbleChore >= 0) {
		_mumbleCostume->stopChore(_mumbleChore);
	}

	if (_sayLineText) {
		g_grim->killTextObject(_sayLineText);
		_sayLineText = NULL;
	}
}

void Actor::pushCostume(const char *n) {
	Costume *newCost = g_resourceloader->loadCostume(n, currentCostume());

	newCost->setColormap(NULL);
	_costumeStack.push_back(newCost);
}

void Actor::setColormap(const char *map) {
	if (!_costumeStack.empty()) {
		Costume *cost = _costumeStack.back();
		cost->setColormap((char *) map);
	} else {
		warning("Actor::setColormap: No costumes");
	}
}

void Actor::setCostume(const char *n) {
	if (!_costumeStack.empty())
		popCostume();

	pushCostume(n);
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
		Costume *newCost;
		if (_costumeStack.empty())
			newCost = NULL;
		else
			newCost = _costumeStack.back();
		if (!newCost) {
			if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
				printf("Popped (freed) the last costume for an actor.\n");
		}
	} else {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Attempted to pop (free) a costume when the stack is empty!");
	}
}

void Actor::clearCostumes() {
	// Make sure to destroy costume copies in reverse order
	while (!_costumeStack.empty())
		popCostume();
}

void Actor::setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw) {
	if (!_costumeStack.empty()) {
		_costumeStack.back()->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
	}
}

Costume *Actor::findCostume(const char *n) {
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); i++)
		if (strcasecmp((*i)->filename(), n) == 0)
			return *i;

	return NULL;
}

void Actor::update() {
	// Snap actor to walkboxes if following them.  This might be
	// necessary for example after activating/deactivating
	// walkboxes, etc.
	if (_constrain && !_walking) {
		g_grim->currScene()->findClosestSector(_pos, NULL, &_pos);
	}

	if (_turning) {
		float turnAmt = g_grim->perSecond(_turnRate);
		float dyaw = _destYaw - _yaw;
		while (dyaw > 180)
			dyaw -= 360;
		while (dyaw < -180)
			dyaw += 360;
		// If the actor won't turn because the rate is set to zero then
		// have the actor turn all the way to the destination yaw.
		// Without this some actors will lock the interface on changing
		// scenes, this affects the Bone Wagon in particular.
		if (turnAmt == 0 || turnAmt >= fabs(dyaw)) {
			setYaw(_destYaw);
			_turning = false;
		}
		else if (dyaw > 0)
			setYaw(_yaw + turnAmt);
		else
			setYaw(_yaw -= turnAmt);
		_currTurnDir = (dyaw > 0 ? 1 : -1);
	}

	if (_walking) {
		Graphics::Vector3d dir = _destPos - _pos;
		float dist = dir.magnitude();

		if (dist > 0)
			dir /= dist;

		float walkAmt = g_grim->perSecond(_walkRate);

		if (walkAmt >= dist) {
			_pos = _destPos;
			_walking = false;
// It seems that we need to allow an already active turning motion to
// continue or else turning actors away from barriers won't work right
//			_turning = false;
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

	// Update lip syncing
	if (_lipSync) {
		int posSound;

		// While getPosIn60HzTicks will return "-1" to indicate that the
		// sound is no longer playing, it is more appropriate to check first
		if (g_imuse->getSoundStatus(_talkSoundName.c_str()))
			posSound = g_imuse->getPosIn60HzTicks(_talkSoundName.c_str());
		else
			posSound = -1;
		if (posSound != -1) {
			int anim = _lipSync->getAnim(posSound);
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

	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); i++) {
		(*i)->setPosRotate(_pos, _pitch, _yaw, _roll);
		(*i)->update();
	}

	if (_lookingMode) {
		/*float lookAtAmt = */g_grim->perSecond(_lookAtRate);
	}
}

void Actor::draw() {
	g_winX1 = g_winY1 = 1000;
	g_winX2 = g_winY2 = -1000;

	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); i++)
		(*i)->setupTextures();

	if (!g_driver->isHardwareAccelerated() && g_grim->getFlagRefreshShadowMask()) {
		for (int l = 0; l < 5; l++) {
			if (!_shadowArray[l].active)
				continue;
			g_driver->setShadow(&_shadowArray[l]);
			g_driver->drawShadowPlanes();
			g_driver->setShadow(NULL);
		}
	}

	if (!_costumeStack.empty()) {
		Costume *costume = _costumeStack.back();
		if (!g_driver->isHardwareAccelerated()) {
			for (int l = 0; l < 5; l++) {
				if (!_shadowArray[l].active)
					continue;
				g_driver->setShadow(&_shadowArray[l]);
				g_driver->setShadowMode();
				g_driver->startActorDraw(_pos, _yaw, _pitch, _roll);
				costume->draw();
				g_driver->finishActorDraw();
				g_driver->clearShadowMode();
				g_driver->setShadow(NULL);
			}
			// normal draw actor
			g_driver->startActorDraw(_pos, _yaw, _pitch, _roll);
			costume->draw();
			g_driver->finishActorDraw();
		} else {
			// normal draw actor
			g_driver->startActorDraw(_pos, _yaw, _pitch, _roll);
			costume->draw();
			g_driver->finishActorDraw();

			for (int l = 0; l < 5; l++) {
				if (!_shadowArray[l].active)
					continue;
				g_driver->setShadow(&_shadowArray[l]);
				g_driver->setShadowMode();
				g_driver->drawShadowPlanes();
				g_driver->startActorDraw(_pos, _yaw, _pitch, _roll);
				costume->draw();
				g_driver->finishActorDraw();
				g_driver->clearShadowMode();
				g_driver->setShadow(NULL);
			}
		}
	}
	_winX1 = g_winX1;
	_winX2 = g_winX2;
	_winY1 = g_winY1;
	_winY2 = g_winY2;
}

// "Undraw objects" (handle objects for actors that may not be on screen)
void Actor::undraw(bool /*visible*/) {
	if (!talking() || !g_imuse->isVoicePlaying())
		shutUp();
}

#define strmatch(src, dst)     (strlen(src) == strlen(dst) && strcmp(src, dst) == 0)

void Actor::setShadowPlane(const char *n) {
	assert(_activeShadowSlot != -1);

	_shadowArray[_activeShadowSlot].name = n;
}

void Actor::addShadowPlane(const char *n) {
	assert(_activeShadowSlot != -1);

	int numSectors = g_grim->currScene()->getSectorCount();

	for (int i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->currScene()->getSectorBase(i);
		if (strmatch(sector->name(), n)) {
			_shadowArray[_activeShadowSlot].planeList.push_back(sector);
			g_grim->flagRefreshShadowMask(true);
			return;
		}
	}
}

void Actor::setActiveShadow(int shadowId) {
	assert(shadowId >= 0 && shadowId <= 4);

	_activeShadowSlot = shadowId;
	_shadowArray[_activeShadowSlot].active = true;
}

void Actor::setShadowValid(int valid) {
	if (valid == -1)
		_shadowArray[_activeShadowSlot].dontNegate = true;
	else
		_shadowArray[_activeShadowSlot].dontNegate = false;
}

void Actor::setActivateShadow(int shadowId, bool state) {
	assert(shadowId >= 0 && shadowId <= 4);

	_shadowArray[shadowId].active = state;
}

void Actor::setShadowPoint(Graphics::Vector3d p) {
	assert(_activeShadowSlot != -1);

	_shadowArray[_activeShadowSlot].pos = p;
}

void Actor::clearShadowPlanes() {
	for (int i = 0; i < 5; i++) {
		Shadow *shadow = &_shadowArray[i];
		while (!shadow->planeList.empty()) {
			shadow->planeList.pop_back();
		}
		delete[] shadow->shadowMask;
		shadow->shadowMask = NULL;
		shadow->active = false;
		shadow->dontNegate = false;
	}
}

} // end of namespace Grim
