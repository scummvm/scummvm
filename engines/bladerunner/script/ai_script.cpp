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

#include "bladerunner/actor.h"
#include "bladerunner/bladerunner.h"

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

	_AIScripts[kActorMcCoy] = new AIScriptMcCoy(_vm);                     //  0
	_AIScripts[kActorSteele] = new AIScriptSteele(_vm);                   //  1
	_AIScripts[kActorGordo] = new AIScriptGordo(_vm);                     //  2
	_AIScripts[kActorDektora] = new AIScriptDektora(_vm);                 //  3
	_AIScripts[kActorGuzza] = new AIScriptGuzza(_vm);                     //  4
	_AIScripts[kActorClovis] = new AIScriptClovis(_vm);                   //  5
	_AIScripts[kActorLucy] = new AIScriptLucy(_vm);                       //  6
	_AIScripts[kActorIzo] = new AIScriptIzo(_vm);                         //  7
	_AIScripts[kActorSadik] = new AIScriptSadik(_vm);                     //  8
	_AIScripts[kActorCrazylegs] = new AIScriptCrazylegs(_vm);             //  9
	_AIScripts[kActorLuther] = new AIScriptLuther(_vm);                   // 10
	_AIScripts[kActorGrigorian] = new AIScriptGrigorian(_vm);             // 11
	_AIScripts[kActorTransient] = new AIScriptTransient(_vm);             // 12
	_AIScripts[kActorLance] = new AIScriptLance(_vm);                     // 13
	_AIScripts[kActorBulletBob] = new AIScriptBulletBob(_vm);             // 14
	_AIScripts[kActorRunciter] = new AIScriptRunciter(_vm);               // 15
	_AIScripts[kActorInsectDealer] = new AIScriptInsectDealer(_vm);       // 16
	_AIScripts[kActorTyrellGuard] = new AIScriptTyrellGuard(_vm);         // 17
	_AIScripts[kActorEarlyQ] = new AIScriptEarlyQ(_vm);                   // 18
	_AIScripts[kActorZuben] = new AIScriptZuben(_vm);                     // 19
	_AIScripts[kActorHasan] = new AIScriptHasan(_vm);                     // 20
	_AIScripts[kActorMarcus] = new AIScriptMarcus(_vm);                   // 21
	_AIScripts[kActorMia] = new AIScriptMia(_vm);                         // 22
	_AIScripts[kActorOfficerLeary] = new AIScriptOfficerLeary(_vm);       // 23
	_AIScripts[kActorOfficerGrayford] = new AIScriptOfficerGrayford(_vm); // 24
	_AIScripts[kActorHanoi] = new AIScriptHanoi(_vm);                     // 25
	_AIScripts[kActorBaker] = new AIScriptBaker(_vm);                     // 26
	_AIScripts[kActorDeskClerk] = new AIScriptDeskClerk(_vm);             // 27
	_AIScripts[kActorHowieLee] = new AIScriptHowieLee(_vm);               // 28
	_AIScripts[kActorFishDealer] = new AIScriptFishDealer(_vm);           // 29
	_AIScripts[kActorKlein] = new AIScriptKlein(_vm);                     // 30
	_AIScripts[kActorMurray] = new AIScriptMurray(_vm);                   // 31
	_AIScripts[kActorHawkersBarkeep] = new AIScriptHawkersBarkeep(_vm);   // 32
	_AIScripts[kActorHolloway] = new AIScriptHolloway(_vm);               // 33
	_AIScripts[kActorSergeantWalls] = new AIScriptSergeantWalls(_vm);     // 34
	_AIScripts[kActorMoraji] = new AIScriptMoraji(_vm);                   // 35
	_AIScripts[kActorTheBard] = new AIScriptTheBard(_vm);                 // 36
	_AIScripts[kActorPhotographer] = new AIScriptPhotographer(_vm);       // 37
	_AIScripts[kActorDispatcher] = new AIScriptDispatcher(_vm);           // 38
	_AIScripts[kActorAnsweringMachine] = new AIScriptAnsweringMachine(_vm);// 39
	_AIScripts[kActorRajif] = new AIScriptRajif(_vm);                     // 40
	_AIScripts[kActorGovernorKolvig] = new AIScriptGovernorKolvig(_vm);   // 41
	_AIScripts[kActorEarlyQBartender] = new AIScriptEarlyQBartender(_vm); // 42
	_AIScripts[kActorHawkersParrot] = new AIScriptHawkersParrot(_vm);     // 43
	_AIScripts[kActorTaffyPatron] = new AIScriptTaffyPatron(_vm);         // 44
	_AIScripts[kActorLockupGuard] = new AIScriptLockupGuard(_vm);         // 45
	_AIScripts[kActorTeenager] = new AIScriptTeenager(_vm);               // 46
	_AIScripts[kActorHysteriaPatron1] = new AIScriptHysteriaPatron1(_vm); // 47
	_AIScripts[kActorHysteriaPatron2] = new AIScriptHysteriaPatron2(_vm); // 48
	_AIScripts[kActorHysteriaPatron3] = new AIScriptHysteriaPatron3(_vm); // 49
	_AIScripts[kActorShoeshineMan] = new AIScriptShoeshineMan(_vm);       // 50
	_AIScripts[kActorTyrell] = new AIScriptTyrell(_vm);                   // 51
	_AIScripts[kActorChew] = new AIScriptChew(_vm);                       // 52
	_AIScripts[kActorGaff] = new AIScriptGaff(_vm);                       // 53
	_AIScripts[kActorBryant] = new AIScriptBryant(_vm);                   // 54
	_AIScripts[kActorTaffy] = new AIScriptTaffy(_vm);                     // 55
	_AIScripts[kActorSebastian] = new AIScriptSebastian(_vm);             // 56
	_AIScripts[kActorRachael] = new AIScriptRachael(_vm);                 // 57
	_AIScripts[kActorGeneralDoll] = new AIScriptGeneralDoll(_vm);         // 58
	_AIScripts[kActorIsabella] = new AIScriptIsabella(_vm);               // 59
	_AIScripts[kActorBlimpGuy] = new AIScriptBlimpGuy(_vm);               // 60
	_AIScripts[kActorNewscaster] = new AIScriptNewscaster(_vm);           // 61
	_AIScripts[kActorLeon] = new AIScriptLeon(_vm);                       // 62
	_AIScripts[kActorMaleAnnouncer] = new AIScriptMaleAnnouncer(_vm);     // 63
	_AIScripts[kActorFreeSlotA] = new AIScriptFreeSlotA(_vm);             // 64
	_AIScripts[kActorFreeSlotB] = new AIScriptFreeSlotB(_vm);             // 65
	_AIScripts[kActorMaggie] = new AIScriptMaggie(_vm);                   // 66
	_AIScripts[kActorGenwalkerA] = new AIScriptGenericWalkerA(_vm);       // 67
	_AIScripts[kActorGenwalkerB] = new AIScriptGenericWalkerB(_vm);       // 68
	_AIScripts[kActorGenwalkerC] = new AIScriptGenericWalkerC(_vm);       // 69
	_AIScripts[kActorMutant1] = new AIScriptMutant1(_vm);                 // 70
	_AIScripts[kActorMutant2] = new AIScriptMutant2(_vm);                 // 71
	_AIScripts[kActorMutant3] = new AIScriptMutant3(_vm);                 // 72
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

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->TimerExpired(timer);
	}
	--_inScriptCounter;
}

void AIScripts::completedMovementTrack(int actor) {
	if (actor >= _actorCount) {
		return;
	}

	if (!_vm->_actors[actor]->inCombat()) {
		++_inScriptCounter;
		if (_AIScripts[actor]) {
			_AIScripts[actor]->CompletedMovementTrack();
		}
		--_inScriptCounter;
	}
}

void AIScripts::receivedClue(int actor, int clueId, int fromActorId) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->ReceivedClue(clueId, fromActorId);
	}
	--_inScriptCounter;
}

void AIScripts::clickedByPlayer(int actor) {
	if (actor >= _actorCount) {
		return;
	}


	if (_vm->_actors[actor]->inCombat()) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->ClickedByPlayer();
	}
	--_inScriptCounter;
}

void AIScripts::enteredSet(int actor, int setId) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->EnteredSet(setId);
	}
	--_inScriptCounter;
}

void AIScripts::otherAgentEnteredThisSet(int actor, int otherActorId) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->OtherAgentEnteredThisSet(otherActorId);
	}
	--_inScriptCounter;
}

void AIScripts::otherAgentExitedThisSet(int actor, int otherActorId) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->OtherAgentExitedThisSet(otherActorId);
	}
	--_inScriptCounter;
}

void AIScripts::otherAgentEnteredCombatMode(int actorId, int otherActorId, int combatMode) {
	assert(actorId < _actorCount);
	++_inScriptCounter;
	if (_AIScripts[actorId]) {
		_AIScripts[actorId]->OtherAgentEnteredCombatMode(otherActorId, combatMode);
	}
	--_inScriptCounter;
}

void AIScripts::shotAtAndMissed(int actorId) {
	assert(actorId < _actorCount);
	++_inScriptCounter;
	if (_AIScripts[actorId]) {
		_AIScripts[actorId]->ShotAtAndMissed();
	}
	--_inScriptCounter;
}

bool AIScripts::shotAtAndHit(int actorId) {
	assert(actorId < _actorCount);
	bool result = true;
	++_inScriptCounter;
	if (_AIScripts[actorId]) {
		result = _AIScripts[actorId]->ShotAtAndHit();
	}
	--_inScriptCounter;
	return result;
}

void AIScripts::retired(int actor, int retiredByActorId) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->Retired(retiredByActorId);
	}
	--_inScriptCounter;
}

void AIScripts::goalChanged(int actor, int currentGoalNumber, int newGoalNumber) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->GoalChanged(currentGoalNumber, newGoalNumber);
	}
	--_inScriptCounter;
}

bool AIScripts::reachedMovementTrackWaypoint(int actor, int waypointId) {
	if (actor >= _actorCount) {
		return false;
	}

	bool result = false;
	if (!_vm->_actors[actor]->inCombat()) {
		++_inScriptCounter;
		if (_AIScripts[actor]) {
			result = _AIScripts[actor]->ReachedMovementTrackWaypoint(waypointId);
		}
		--_inScriptCounter;
	}
	return result;
}

void AIScripts::updateAnimation(int actor, int *animation, int *frame) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->UpdateAnimation(animation, frame);
	}
	--_inScriptCounter;
}

void AIScripts::changeAnimationMode(int actor, int mode) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->ChangeAnimationMode(mode);
	}
	--_inScriptCounter;
}

void AIScripts::fledCombat(int actor) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->FledCombat();
	}
	--_inScriptCounter;
}

void AIScripts::setAnimationState(int actor, int animationState, int animationFrame, int animationStateNext, int animationNext) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->SetAnimationState(animationState, animationFrame, animationStateNext, animationNext);
	}
	--_inScriptCounter;
}


void AIScripts::queryAnimationState(int actor, int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	if (actor >= _actorCount) {
		return;
	}

	++_inScriptCounter;
	if (_AIScripts[actor]) {
		_AIScripts[actor]->QueryAnimationState(animationState, animationFrame, animationStateNext, animationNext);
	}
	--_inScriptCounter;
}


} // End of namespace BladeRunner
