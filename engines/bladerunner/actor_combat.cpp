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

#include "bladerunner/actor_combat.h"

#include "bladerunner/actor.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/combat.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_info.h"
#include "bladerunner/movement_track.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/ai_script.h"
#include "bladerunner/set.h"
#include "bladerunner/settings.h"

namespace BladeRunner {

ActorCombat::ActorCombat(BladeRunnerEngine *vm) {
	_vm = vm;
	reset();
}

ActorCombat::~ActorCombat() {
}

void ActorCombat::setup() {
	reset();
}

void ActorCombat::combatOn(int actorId, int initialState, bool rangedAttackFlag, int enemyId, int waypointType, int fleeRatio, int coverRatio, int attackRatio, int damage, int range, bool unstoppable) {
	_actorId = actorId;
	_state = initialState;
	_rangedAttack = rangedAttackFlag;
	_enemyId = enemyId;
	_waypointType = waypointType;
	_damage = damage;
	_fleeRatioConst = fleeRatio;
	_coverRatioConst = coverRatio;
	_attackRatioConst = attackRatio;
	_fleeRatio = fleeRatio;
	_coverRatio = coverRatio;
	_attackRatio = attackRatio;
	_active = true;
	if (_rangedAttack) {
		_range = range;
	} else {
		_range = 300;
	}
	_unstoppable = unstoppable;

	Actor *actor = _vm->_actors[_actorId];

	_actorPosition = actor->getXYZ();
	_enemyPosition = _vm->_actors[_enemyId]->getXYZ();

	actor->_movementTrack->flush();
	actor->stopWalking(false);

	if (_enemyId == kActorMcCoy) {
		actor->setTarget(true);
	}

	_actorHp = actor->getCurrentHP();

	_coversWaypointCount = 0;
	for (int i = 0; i < (int)_vm->_gameInfo->getCoverWaypointCount(); ++i) {
		if (_vm->_combat->_coverWaypoints[i].type == waypointType && _vm->_combat->_coverWaypoints[i].setId == actor->getSetId()) {
			++_coversWaypointCount;
		}
	}
	if (_coversWaypointCount == 0) {
		_coverRatioConst = 0;
		_coverRatio = 0;
	}

	_fleeWaypointsCount = 0;
	for (int i = 0; i < (int)_vm->_gameInfo->getFleeWaypointCount(); ++i) {
		if (_vm->_combat->_fleeWaypoints[i].type == waypointType && _vm->_combat->_fleeWaypoints[i].setId == actor->getSetId()) {
			++_fleeWaypointsCount;
		}
	}
	if (_fleeWaypointsCount == 0) {
		_fleeRatioConst = 0;
		_fleeRatio = 0;
	}
}

void ActorCombat::combatOff() {
	_active = false;
	reset();
}

void ActorCombat::tick() {
	static int processingCounter = 0;

	if (!_active || processingCounter > 0) {
		return;
	}

	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	if (actor->getSetId() != enemy->getSetId()) {
		actor->combatModeOff();
		return;
	}

	++processingCounter;

	_actorPosition = actor->getXYZ();
	_enemyPosition = enemy->getXYZ();

	if (_attackRatioConst >= 0) {
		_attackRatio = _attackRatioConst;
	} else {
		_attackRatio = calculateAttackRatio();
	}

	if (_vm->_combat->findCoverWaypoint(_waypointType, _actorId, _enemyId) != -1) {
		if (_coverRatioConst >= 0) {
			_coverRatio = _coverRatioConst;
		} else {
			_coverRatio = calculateCoverRatio();
		}
	} else {
		_coverRatio = 0;
	}

	if (_fleeRatioConst >= 0) {
		_fleeRatio = _fleeRatioConst;
	} else {
		_fleeRatio = calculateFleeRatio();
	}

	float dist = actor->distanceFromActor(_enemyId);
	int oldState = _state;

	if (_attackRatio < _fleeRatio || _attackRatio < _coverRatio) {
		if (_coverRatio >= _fleeRatio && _coverRatio >= _attackRatio) {
			_state = kActorCombatStateCover;
		} else {
			_state = kActorCombatStateFlee;
		}
	} else {
		if (_rangedAttack) {
			if (dist > _range) {
				_state = kActorCombatStateApproachRangedAttack;
			} else {
				if (actor->isObstacleBetween(_enemyPosition)) {
					_state = kActorCombatStateUncover;
				} else {
					_state = kActorCombatStateRangedAttack;
				}
			}
		} else {
			if (dist > 36.0f) {
				_state = kActorCombatStateApproachCloseAttack;
			} else {
				_state = kActorCombatStateCloseAttack;
			}
		}
	}

	if (enemy->isRetired()) {
		_state = kActorCombatStateIdle;
	}

	if (actor->getAnimationMode() == kAnimationModeHit || actor->getAnimationMode() == kAnimationModeCombatHit) {
		_state = kActorCombatStateIdle;
	} else {
		if (_state != oldState) {
			actor->stopWalking(false);
		}
	}
	switch (_state) {
	case kActorCombatStateCover:
		cover();
		break;
	case kActorCombatStateApproachCloseAttack:
		approachToCloseAttack();
		break;
	case kActorCombatStateUncover:
		uncover();
		break;
	case kActorCombatStateAim:
		aim();
		break;
	case kActorCombatStateRangedAttack:
		rangedAttack();
		break;
	case kActorCombatStateCloseAttack:
		closeAttack();
		break;
	case kActorCombatStateFlee:
		flee();
		break;
	case kActorCombatStateApproachRangedAttack:
		approachToRangedAttack();
		break;
	default:
		break;
	}
	--processingCounter;
}

void ActorCombat::hitAttempt() {
	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	if (_enemyId == kActorMcCoy && !_vm->playerHasControl() && !_unstoppable) {
		return;
	}

	if (actor->isRetired()) {
		return;
	}

	int attackCoefficient = 0;
	if (_rangedAttack) {
		attackCoefficient = _rangedAttack ? getCoefficientRangedAttack() : 0;
	} else {
		attackCoefficient = getCoefficientCloseAttack();
	}

	if (attackCoefficient == 0) {
		return;
	}

	int random = _vm->_rnd.getRandomNumberRng(1, 100);

	if (random <= attackCoefficient) {
		if (enemy->isWalking()) {
			enemy->stopWalking(true);
		}

		int sentenceId = _vm->_rnd.getRandomNumberRng(0, 1) ? 9000 : 9005;
		if (enemy->inCombat()) {
			enemy->changeAnimationMode(22, false);
		} else {
			enemy->changeAnimationMode(21, false);
		}

		int damage = 0;
		if (_rangedAttack) {
			damage = getDamageRangedAttack(random, attackCoefficient);
		} else {
			damage = getDamageCloseAttack(random, attackCoefficient);
		}

		int enemyHp = MAX(enemy->getCurrentHP() - damage, 0);
		enemy->setCurrentHP(enemyHp);

		if (enemyHp <= 0) {
			if (!enemy->isRetired()) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
				// make sure the dead enemy won't pick a pending movement track and re-spawn
				enemy->_movementTrack->flush();
#endif
				if (enemy->inCombat()) {
					enemy->changeAnimationMode(kAnimationModeCombatDie, false);
				} else {
					enemy->changeAnimationMode(kAnimationModeDie, false);
				}
				sentenceId = 9020;
			}
			enemy->retire(true, 6, 3, _actorId);
		}

		if (_enemyId == kActorMcCoy) {
			sentenceId += 900;
		}

		_vm->_audioSpeech->playSpeechLine(_enemyId, sentenceId, 75, enemy->soundPan(), 99);
	}
}

void ActorCombat::save(SaveFileWriteStream &f) {
	f.writeInt(_actorId);
	f.writeBool(_active);
	f.writeInt(_state);
	f.writeBool(_rangedAttack);
	f.writeInt(_enemyId);
	f.writeInt(_waypointType);
	f.writeInt(_damage);
	f.writeInt(_fleeRatio);
	f.writeInt(_coverRatio);
	f.writeInt(_attackRatio);
	f.writeInt(_fleeRatioConst);
	f.writeInt(_coverRatioConst);
	f.writeInt(_attackRatioConst);
	f.writeInt(_range);
	f.writeInt(_unstoppable);
	f.writeInt(_actorHp);
	f.writeInt(_fleeingTowards);
	f.writeVector3(_actorPosition);
	f.writeVector3(_enemyPosition);
	f.writeInt(_coversWaypointCount);
	f.writeInt(_fleeWaypointsCount);
}

void ActorCombat::load(SaveFileReadStream &f) {
	_actorId = f.readInt();
	_active = f.readBool();
	_state = f.readInt();
	_rangedAttack = f.readBool();
	_enemyId = f.readInt();
	_waypointType = f.readInt();
	_damage = f.readInt();
	_fleeRatio = f.readInt();
	_coverRatio = f.readInt();
	_attackRatio = f.readInt();
	_fleeRatioConst = f.readInt();
	_coverRatioConst = f.readInt();
	_attackRatioConst = f.readInt();
	_range = f.readInt();
	_unstoppable = f.readInt();
	_actorHp = f.readInt();
	_fleeingTowards = f.readInt();
	_actorPosition = f.readVector3();
	_enemyPosition = f.readVector3();
	_coversWaypointCount = f.readInt();
	_fleeWaypointsCount = f.readInt();
}

void ActorCombat::reset() {
	_active              = false;
	_actorId             = -1;
	_state               = -1;
	_rangedAttack        = false;
	_enemyId             = -1;
	_waypointType        = -1;
	_damage              = 0;
	_fleeRatio           = -1;
	_coverRatio          = -1;
	_attackRatio         = -1;
	_fleeRatioConst      = -1;
	_coverRatioConst     = -1;
	_attackRatioConst    = -1;
	_actorHp             = 0;
	_range               = 300;
	_unstoppable         = false;
	_actorPosition       = Vector3(0.0f, 0.0f, 0.0f);
	_enemyPosition       = Vector3(0.0f, 0.0f, 0.0f);
	_coversWaypointCount = 0;
	_fleeWaypointsCount  = 0;
	_fleeingTowards      = -1;
}

void ActorCombat::cover() {
	Actor *actor = _vm->_actors[_actorId];

	if (actor->isWalking()) {
		return;
	}

	if (actor->isObstacleBetween(_enemyPosition)) {
		faceEnemy();
		return;
	}

	int coverWaypointId = _vm->_combat->findCoverWaypoint(_waypointType, _actorId, _enemyId);
	if (coverWaypointId == -1) {
		_state = kActorCombatStateIdle;
	} else {
		actor->asyncWalkToXYZ(_vm->_combat->_coverWaypoints[coverWaypointId].position, 0, true, 0);
	}
}

void ActorCombat::approachToCloseAttack() {
	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	float dist = actor->distanceFromActor(_enemyId);
	if (dist > 36.0f) {
		if (!actor->isWalking() || enemy->isWalking()) {
			Vector3 target;
			if (findClosestPositionToEnemy(target)) {
				actor->asyncWalkToXYZ(target, 0, dist >= 240.0f, 0);
			} else {
				_state = kActorCombatStateCover;
			}
		}
	} else {
		if (actor->isWalking()) {
			actor->stopWalking(false);
		}
		faceEnemy();
		_state = kActorCombatStateCloseAttack;
	}
}

void ActorCombat::approachToRangedAttack() {
	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	float dist = actor->distanceFromActor(_enemyId);
	if (dist > _range) {
		if (!actor->isWalking() || enemy->isWalking()) {
			Vector3 target;
			if (findClosestPositionToEnemy(target)) {
				actor->asyncWalkToXYZ(target, 0, dist >= 240.0f, 0);
			} else {
				_state = kActorCombatStateCover;
			}
		}
	} else {
		if (actor->isWalking()) {
			actor->stopWalking(false);
		}
		faceEnemy();
		_state = kActorCombatStateRangedAttack;
	}
}

void ActorCombat::uncover() {
	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	if (actor->isObstacleBetween(_enemyPosition)) {
		actor->asyncWalkToXYZ(enemy->getXYZ(), 16, false, 0);
	} else {
		if (actor->isWalking()) {
			actor->stopWalking(false);
		}
		faceEnemy();
	}
}

void ActorCombat::aim() {
	Actor *actor = _vm->_actors[_actorId];

	if (actor->isObstacleBetween(_enemyPosition)) {
		if (actor->getAnimationMode() != kAnimationModeCombatIdle) {
			actor->changeAnimationMode(kAnimationModeCombatIdle, false);
		}
	} else {
		faceEnemy();
		if (actor->getAnimationMode() != kAnimationModeCombatAim) {
			actor->changeAnimationMode(kAnimationModeCombatAim, false);
		}
	}
}

void ActorCombat::rangedAttack() {
	Actor *actor = _vm->_actors[_actorId];

	if (actor->isObstacleBetween(_enemyPosition) || (actor->distanceFromActor(_enemyId) > _range)) {
		_state = kActorCombatStateApproachRangedAttack;
	} else {
		faceEnemy();
		if (actor->getAnimationMode() != kAnimationModeCombatAttack) {
			if (_enemyId != kActorMcCoy || _vm->playerHasControl() || _unstoppable) {
				actor->changeAnimationMode(kAnimationModeCombatAttack, false);
			}
		}
	}
}

void ActorCombat::closeAttack() {
	Actor *actor = _vm->_actors[_actorId];

	if (actor->isObstacleBetween(_enemyPosition) || (actor->distanceFromActor(_enemyId) > 36.0f)) {
		_state = kActorCombatStateApproachCloseAttack;
	} else {
		faceEnemy();
		if (actor->getAnimationMode() != kAnimationModeCombatAttack) {
			if (_enemyId != kActorMcCoy || _vm->playerHasControl() || _unstoppable) {
				actor->changeAnimationMode(kAnimationModeCombatAttack, false);
			}
		}
	}
}

void ActorCombat::flee() {
	Actor *actor = _vm->_actors[_actorId];

	if (_fleeingTowards != -1 && actor->isWalking()) {
		Vector3 fleeWaypointPosition = _vm->_combat->_fleeWaypoints[_fleeingTowards].position;
		if (distance(_actorPosition, fleeWaypointPosition) <= 12.0f) {
			_vm->_aiScripts->fledCombat(_actorId/*, _enemyId*/);
			actor->setSetId(kSetFreeSlotG);
			actor->combatModeOff();
			_fleeingTowards = -1;
		}
	} else {
		int fleeWaypointId = _vm->_combat->findFleeWaypoint(actor->getSetId(), _enemyId, _actorPosition);
		if (fleeWaypointId == -1) {
			_state = kActorCombatStateIdle;
		} else {
			Vector3 fleeWaypointPosition = _vm->_combat->_fleeWaypoints[fleeWaypointId].position;
			actor->asyncWalkToXYZ(fleeWaypointPosition, 0, true, 0);
			_fleeingTowards = fleeWaypointId;
		}
	}
}

void ActorCombat::faceEnemy() {
	_vm->_actors[_actorId]->setFacing(angle_1024(_actorPosition.x, _actorPosition.z, _enemyPosition.x, _enemyPosition.z), false);
}

int ActorCombat::getCoefficientCloseAttack() const{
	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	float distance = actor->distanceFromActor(_enemyId);

	if (distance > 36.0f) {
		return 0;
	}

	int aggressiveness = 0;
	if (enemy->isRunning()) {
		aggressiveness = 11;
	} else if (enemy->isMoving()) {
		aggressiveness = 22;
	} else {
		aggressiveness = 33;
	}

	aggressiveness += actor->getCombatAggressiveness() / 3;

	int angle = abs(actor->angleTo(_enemyPosition));

	if (angle > 128) {
		return false;
	}

	return aggressiveness + (abs(angle - 128) / 3.7f);
}

int ActorCombat::getCoefficientRangedAttack() const {
	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	if (actor->isObstacleBetween(_enemyPosition)) {
		return 0;
	}

	int distance = MIN(actor->distanceFromActor(_enemyId), 900.0f);

	int aggressiveness = 0;
	if (enemy->isRunning()) {
		aggressiveness = 10;
	} else if (enemy->isMoving()) {
		aggressiveness = 20;
	} else {
		aggressiveness = 30;
	}

	aggressiveness += actor->getCombatAggressiveness() / 5;
	return aggressiveness + abs((distance / 30) - 30) + actor->getIntelligence() / 5;
}

int ActorCombat::getDamageCloseAttack(int min, int max) const {
	if (_enemyId == kActorMcCoy && _vm->_settings->getDifficulty() == kGameDifficultyEasy) {
		return _damage / 2;
	}
	if (_enemyId == kActorMcCoy && _vm->_settings->getDifficulty() == kGameDifficultyHard) {
		return _damage;
	}
	return ((MIN(max - min, 30) * 100.0f / 60.0f) + 50) * _damage / 100;
}

int ActorCombat::getDamageRangedAttack(int min, int max) const {
	if (_enemyId == kActorMcCoy && _vm->_settings->getDifficulty() == kGameDifficultyEasy) {
		return _damage / 2;
	}
	if (_enemyId == kActorMcCoy && _vm->_settings->getDifficulty() == kGameDifficultyHard) {
		return _damage;
	}
	return ((MIN(max - min, 30) * 100.0f / 60.0f) + 50) * _damage / 100;
}

int ActorCombat::calculateAttackRatio() const {
	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	int aggressivenessFactor = actor->getCombatAggressiveness();
	int actorHpFactor        = actor->getCurrentHP();
	int enemyHpFactor        = 100 - enemy->getCurrentHP();
	int combatFactor         = enemy->inCombat() ? 0 : 100;
	int angleFactor          = (100 * abs(enemy->angleTo(_actorPosition))) / 512;
	int distanceFactor       = 2 * (50 - MIN(actor->distanceFromActor(_enemyId) / 12.0f, 50.0f));

	if (_rangedAttack) {
		return
			angleFactor          * 0.25f +
			combatFactor         * 0.05f +
			enemyHpFactor        * 0.20f +
			actorHpFactor        * 0.10f +
			aggressivenessFactor * 0.40f;
	} else {
		return
			distanceFactor       * 0.20f +
			angleFactor          * 0.10f +
			combatFactor         * 0.10f +
			enemyHpFactor        * 0.15f +
			actorHpFactor        * 0.15f +
			aggressivenessFactor * 0.30f;
	}
}

int ActorCombat::calculateCoverRatio() const {
	if (_coversWaypointCount == 0) {
		return 0;
	}

	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	int angleFactor          = 100 - (100 * abs(enemy->angleTo(_actorPosition))) / 512;
	int actorHpFactor        = 100 - actor->getCurrentHP();
	int enemyHpFactor        = enemy->getCurrentHP();
	int aggressivenessFactor = 100 - actor->getCombatAggressiveness();
	int distanceFactor       = 2 * MIN(actor->distanceFromActor(_enemyId) / 12.0f, 50.0f);

	if (_rangedAttack) {
		return
			angleFactor          * 0.40f +
			enemyHpFactor        * 0.05f +
			actorHpFactor        * 0.15f +
			aggressivenessFactor * 0.50f;
	} else {
		return
			distanceFactor       * 0.25f +
			angleFactor          * 0.20f +
			enemyHpFactor        * 0.05f +
			actorHpFactor        * 0.10f +
			aggressivenessFactor * 0.50f;
	}
}

int ActorCombat::calculateFleeRatio() const {
	if (_fleeWaypointsCount == 0) {
		return 0;
	}

	Actor *actor = _vm->_actors[_actorId];
	Actor *enemy = _vm->_actors[_enemyId];

	int aggressivenessFactor = 100 - actor->getCombatAggressiveness();
	int actorHpFactor        = 100 - actor->getCurrentHP();
	int combatFactor         = enemy->inCombat() ? 100 : 0;

	return
		combatFactor * 0.2f +
		actorHpFactor * 0.4f +
		aggressivenessFactor * 0.4f;
}

bool ActorCombat::findClosestPositionToEnemy(Vector3 &output) const {
	output = Vector3();

	Vector3 offsets[] = {
		Vector3(  0.0f, 0.0f, -28.0f),
		Vector3( 28.0f, 0.0f,   0.0f),
		Vector3(  0.0f, 0.0f,  28.0f),
		Vector3(-28.0f, 0.0f,   0.0f)
	};

	float min = -1.0f;

	for (int i = 0; i < 4; ++i) {
		Vector3 test = _enemyPosition + offsets[i];
		float dist = distance(_actorPosition, test);
		if ( min == -1.0f || dist < min) {
			if (!_vm->_sceneObjects->existsOnXZ(_actorId + kSceneObjectOffsetActors, test.x, test.z, true, true) && _vm->_scene->_set->findWalkbox(test.x, test.z) >= 0) {
				output = test;
				min = dist;
			}
		}
	}

	return min >= 0.0f;
}

} // End of namespace BladeRunner
