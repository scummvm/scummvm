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

#include "bladerunner/script/ai_script.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"

namespace BladeRunner {

AIScripts::AIScripts(BladeRunnerEngine *vm, int actorCount) {
	_vm = vm;
	_inScriptCounter = 0;
	_actorCount = actorCount;
	_actorUpdating = new bool[actorCount];
	_AIScripts = new AIScriptBase*[actorCount];
	for (int i = 0; i < actorCount; ++i) {
		_AIScripts[i] = nullptr;
		_actorUpdating[i] = false;
	}

	_AIScripts[kActorMcCoy] = new AIScriptMcCoy(_vm);
	_AIScripts[kActorGrigorian] = new AIScriptGrigorian(_vm);
	_AIScripts[kActorTransient] = new AIScriptTransient(_vm);
	_AIScripts[kActorLance] = new AIScriptLance(_vm);
	_AIScripts[kActorRunciter] = new AIScriptRunciter(_vm);
	_AIScripts[kActorZuben] = new AIScriptZuben(_vm);
	_AIScripts[kActorOfficerLeary] = new AIScriptOfficerLeary(_vm);
	_AIScripts[kActorHowieLee] = new AIScriptHowieLee(_vm);
	_AIScripts[kActorKlein] = new AIScriptKlein(_vm);
	_AIScripts[kActorSergeantWalls] = new AIScriptSergeantWalls(_vm);
	_AIScripts[kActorHysteriaPatron1] = new AIScriptHysteriaPatron1(_vm);
	_AIScripts[kActorHysteriaPatron2] = new AIScriptHysteriaPatron2(_vm);
	_AIScripts[kActorHysteriaPatron3] = new AIScriptHysteriaPatron3(_vm);
	_AIScripts[kActorShoeshineMan] = new AIScriptShoeshineMan(_vm);
	_AIScripts[kActorGaff] = new AIScriptGaff(_vm);
	_AIScripts[kActorBryant] = new AIScriptBryant(_vm);
	_AIScripts[kActorNewscaster] = new AIScriptNewscaster(_vm);
	_AIScripts[kActorLeon] = new AIScriptLeon(_vm);
	_AIScripts[kActorMaleAnnouncer] = new AIScriptMaleAnnouncer(_vm);
	_AIScripts[kActorMaggie] = new AIScriptMaggie(_vm);
	_AIScripts[kActorGenwalkerA] = new AIScriptGenericWalkerA(_vm);
	_AIScripts[kActorGenwalkerB] = new AIScriptGenericWalkerB(_vm);
	_AIScripts[kActorGenwalkerC] = new AIScriptGenericWalkerC(_vm);
}

AIScripts::~AIScripts() {
	for (int i = 0; i < _actorCount; ++i) {
		delete _AIScripts[i];
		_AIScripts[i] = nullptr;
	}
	delete[] _AIScripts;
	delete[] _actorUpdating;
}

void AIScripts::initialize(int actor) {
	if (actor >= _actorCount) {
		return;
	}

	if (_AIScripts[actor]) {
		_AIScripts[actor]->Initialize();
	}
}

void AIScripts::update(int actor) {
	if (actor >= _actorCount) {
		return;
	}

	if (!_actorUpdating[actor]) {
		_actorUpdating[actor] = true;
		++_inScriptCounter;
		if (_AIScripts[actor]) {
			_AIScripts[actor]->Update();
		}
		--_inScriptCounter;
		_actorUpdating[actor] = false;
	}
}

void AIScripts::timerExpired(int actor, int timer) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->TimerExpired(timer);
	}
	_inScriptCounter--;
}

void AIScripts::completedMovementTrack(int actor) {
	if (actor >= _actorCount) {
		return;
	}

	if (!_vm->_actors[actor]->inCombat()) {
		_inScriptCounter++;
		if (_AIScripts[actor]) {
			_AIScripts[actor]->CompletedMovementTrack();
		}
		_inScriptCounter--;
	}
}

void AIScripts::receivedClue(int actor, int clueId, int fromActorId) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->ReceivedClue(clueId, fromActorId);
	}
	_inScriptCounter--;
}

void AIScripts::clickedByPlayer(int actor) {
	if (actor >= _actorCount) {
		return;
	}


	if(_vm->_actors[actor]->inCombat()) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->ClickedByPlayer();
	}
	_inScriptCounter--;
}

void AIScripts::enteredScene(int actor, int setId) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->EnteredScene(setId);
	}
	_inScriptCounter--;
}

void AIScripts::otherAgentEnteredThisScene(int actor, int otherActorId) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->OtherAgentEnteredThisScene(otherActorId);
	}
	_inScriptCounter--;
}

void AIScripts::otherAgentExitedThisScene(int actor, int otherActorId) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->OtherAgentExitedThisScene(otherActorId);
	}
	_inScriptCounter--;
}

void AIScripts::otherAgentEnteredCombatMode(int actorId, int otherActorId, int combatMode) {
	assert(actorId < _actorCount);
	_inScriptCounter++;
	if (_AIScripts[actorId]) {
		_AIScripts[actorId]->OtherAgentEnteredCombatMode(otherActorId, combatMode);
	}
	_inScriptCounter--;
}

void AIScripts::shotAtAndMissed(int actorId) {
	assert(actorId < _actorCount);
	_inScriptCounter++;
	if (_AIScripts[actorId]) {
		_AIScripts[actorId]->ShotAtAndMissed();
	}
	_inScriptCounter--;
}

bool AIScripts::shotAtAndHit(int actorId) {
	assert(actorId < _actorCount);
	bool result = true;
	_inScriptCounter++;
	if (_AIScripts[actorId]) {
		result = _AIScripts[actorId]->ShotAtAndHit();
	}
	_inScriptCounter--;
	return result;
}

void AIScripts::retired(int actor, int retiredByActorId) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->Retired(retiredByActorId);
	}
	_inScriptCounter--;
}

void AIScripts::goalChanged(int actor, int currentGoalNumber, int newGoalNumber) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->GoalChanged(currentGoalNumber, newGoalNumber);
	}
	_inScriptCounter--;
}

bool AIScripts::reachedMovementTrackWaypoint(int actor, int waypointId) {
	if (actor >= _actorCount) {
		return false;
	}

	bool result = false;
	if (!_vm->_actors[actor]->inCombat()) {
		_inScriptCounter++;
		if (_AIScripts[actor]) {
			result = _AIScripts[actor]->ReachedMovementTrackWaypoint(waypointId);
		}
		_inScriptCounter--;
	}
	return result;
}

void AIScripts::updateAnimation(int actor, int *animation, int *frame) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->UpdateAnimation(animation, frame);
	}
	_inScriptCounter--;
}

void AIScripts::changeAnimationMode(int actor, int mode) {
	if (actor >= _actorCount) {
		return;
	}

	_inScriptCounter++;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->ChangeAnimationMode(mode);
	}
	_inScriptCounter--;
}

} // End of namespace BladeRunner
