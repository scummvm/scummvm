/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#include "bladerunner/combat.h"

#include "bladerunner/actor.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_info.h"
#include "bladerunner/movement_track.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/settings.h"

namespace BladeRunner {

Combat::Combat(BladeRunnerEngine *vm) {
	_vm = vm;

	_coverWaypoints.resize(_vm->_gameInfo->getCoverWaypointCount());
	_fleeWaypoints.resize(_vm->_gameInfo->getFleeWaypointCount());

	reset();
}

Combat::~Combat() {
}

void Combat::reset() {
	_active = false;
	_enabled = true;

	_ammoDamage[0] = 10;
	_ammoDamage[1] = 20;
	_ammoDamage[2] = 30;

	for (int i = 0; i < kSoundCount; ++i) {
		_hitSoundId[i] = -1;
		_missSoundId[i] = -1;
	}
}

void Combat::activate() {
	if (_enabled) {
		_vm->_playerActor->combatModeOn(-1, true, -1, -1, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, _vm->_combat->_ammoDamage[_vm->_settings->getAmmoType()], 0, false);
		_active = true;
	}
}

void Combat::deactivate() {
	if (_enabled) {
		_vm->_playerActor->combatModeOff();
		_active = false;
	}
}

void Combat::change() {
	if (!_vm->_playerActor->mustReachWalkDestination() && _enabled) {
		if (_active) {
			deactivate();
		} else {
			activate();
		}
	}
}

bool Combat::isActive() const{
	return _active;
}

void Combat::enable() {
	_enabled = true;
}

void Combat::disable() {
	_enabled = false;
}

void Combat::setHitSound(int ammoType, int column, int soundId) {
	_hitSoundId[(kSoundCount/_vm->_settings->getAmmoTypesCount()) * ammoType + column] = soundId;
}

void Combat::setMissSound(int ammoType, int column, int soundId) {
	_missSoundId[(kSoundCount/_vm->_settings->getAmmoTypesCount()) * ammoType + column] = soundId;
}

int Combat::getHitSound() const {
	return _hitSoundId[(kSoundCount/_vm->_settings->getAmmoTypesCount()) * _vm->_settings->getAmmoType() + _vm->_rnd.getRandomNumber(2)];
}

int Combat::getMissSound() const {
	return _missSoundId[(kSoundCount/_vm->_settings->getAmmoTypesCount()) * _vm->_settings->getAmmoType() + _vm->_rnd.getRandomNumber(2)];
}

void Combat::shoot(int actorId, Vector3 &to, int screenX) {
	Actor *actor = _vm->_actors[actorId];

	if (actor->isRetired()) {
		return;
	}

	int sentenceId = -1;

	/*
	Distance from center as a percentage:
	                            screenX - abs(right + left) / 2
	distanceFromCenter = 100 *  -------------------------------
	                                 abs(right - left) / 2
	*/
	const Common::Rect &rect = actor->getScreenRectangle();
	int distanceFromCenter = CLIP(100 * (screenX - abs((rect.right + rect.left) / 2)) / abs((rect.right - rect.left) / 2), 0, 100);

	int damage = (100 - distanceFromCenter) * _ammoDamage[_vm->_settings->getAmmoType()] / 100;

	int hp = MAX(actor->getCurrentHP() - damage, 0);

	actor->setCurrentHP(hp);

	bool setDamageAnimation = true;
	if (actor->isWalking() == 1 && !actor->getFlagDamageAnimIfMoving()) {
		setDamageAnimation = false;
	}
	if (actor->_movementTrack->hasNext() && !actor->_movementTrack->isPaused()) {
		setDamageAnimation = false;
	}
	if (setDamageAnimation) {
		if (actor->isWalking()) {
			actor->stopWalking(false);
		}
		if (actor->getAnimationMode() != kAnimationModeHit && actor->getAnimationMode() != kAnimationModeCombatHit) {
			actor->changeAnimationMode(kAnimationModeHit, false);
			sentenceId = _vm->_rnd.getRandomNumberRng(0, 1) ? 9000 : 9005;
		}
	}

	if (hp <= 0) {
		actor->setTarget(false);
		if (actor->inCombat()) {
			actor->combatModeOff();
		}
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// make sure the dead enemy won't pick a pending movement track and re-spawn
		actor->_movementTrack->flush();
#endif
		actor->stopWalking(false);
		actor->changeAnimationMode(kAnimationModeDie, false);

		actor->retire(true, 72, 36, kActorMcCoy);
		actor->setAtXYZ(actor->getXYZ(), actor->getFacing(), true, false, true);
		_vm->_sceneObjects->setRetired(actorId + kSceneObjectOffsetActors, true);

		sentenceId = 9020; // Bug or intended? This sentence id (death rattle) won't be used in this case since combat mode is set to off above. Probably intended, in order to use the rattle in a case by case (?)
	}

	if (sentenceId >= 0 && actor->inCombat()) {
		_vm->_audioSpeech->playSpeechLine(actorId, sentenceId, 75, 0, 99);
	}
}

int Combat::findFleeWaypoint(int setId, int enemyId, const Vector3& position) const {
	float min = -1.0f;
	int result = -1;
	for (int i = 0; i < (int)_fleeWaypoints.size(); ++i) {
		if (setId == _fleeWaypoints[i].setId) {
			float dist = distance(position, _fleeWaypoints[i].position);
			if (result == -1 || dist < min) {
				result = i;
				min = dist;
			}
		}
	}
	return result;
}

int Combat::findCoverWaypoint(int waypointType, int actorId, int enemyId) const {
	Actor *actor = _vm->_actors[actorId];
	Actor *enemy = _vm->_actors[enemyId];
	int result = -1;
	float min = -1.0f;
	for (int i = 0; i < (int)_coverWaypoints.size(); ++i) {
		if (waypointType == _coverWaypoints[i].type && actor->getSetId() == _coverWaypoints[i].setId) {
			if (_vm->_sceneObjects->isObstacleBetween(_coverWaypoints[i].position, enemy->getXYZ(), enemyId)) {
				float dist = distance(_coverWaypoints[i].position, actor->getXYZ());
				if (result == -1 || dist < min) {
					result = i;
					min = dist;
				}
			}
		}
	}
	return result;
}

void Combat::save(SaveFileWriteStream &f) {
	f.writeBool(_active);
	f.writeBool(_enabled);
	for (int i = 0; i != kSoundCount; ++i) {
		f.writeInt(_hitSoundId[i]);
	}
	for (int i = 0; i != kSoundCount; ++i) {
		f.writeInt(_missSoundId[i]);
	}
}

void Combat::load(SaveFileReadStream &f) {
	_active = f.readBool();
	_enabled = f.readBool();
	for (int i = 0; i != kSoundCount; ++i) {
		_hitSoundId[i] = f.readInt();
	}
	for (int i = 0; i != kSoundCount; ++i) {
		_missSoundId[i] = f.readInt();
	}
}

} // End of namespace BladeRunner
