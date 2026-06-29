/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hollywood/scenes/playable/scene7040.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene7040State7041 = 0x1b81;
const uint16 kScene7040State7042 = 0x1b82;
const byte kScene7040Entry7040Facing = 1;
const int kScene7040Entry7040StartX = 100;
const int kScene7040Entry7040StartY = 0x1b1;
const int kScene7040Entry7040FirstTargetX = 0x14a;
const int kScene7040Entry7040FirstTargetY = 0x139;
const int kScene7040Entry7040RepeatTargetX = 0x16f;
const int kScene7040Entry7040RepeatTargetY = 0x177;
const byte kScene7040Entry7041Facing = 4;
const int kScene7040Entry7041StartX = 600;
const int kScene7040Entry7041StartY = 0x132;
const byte kScene7040Entry7042Facing = 4;
const int kScene7040Entry7042StartX = 0x322;
const int kScene7040Entry7042StartY = 0x1c9;
const int kScene7040Entry7042TargetX = 0x29e;
const int kScene7040Entry7042TargetY = 0x1cc;
const uint32 kScene7040Chunk11FrameMillis = 75;
const byte kScene7040DialogueStageId = 0x62;
const byte kScene7040DialoguePrimaryRow = 99;
const uint16 kScene7040DialoguePrimaryCenterX = 0x1c2;
const uint16 kScene7040DialoguePrimaryTopY = 0x73;
const byte kScene7040DialoguePrimaryRed = 0x3f;
const byte kScene7040DialoguePrimaryGreen = 0x32;
const byte kScene7040DialoguePrimaryBlue = 0x0c;

Scene7040::Scene7040(HollywoodEngine *vm) :
		SuePlayableScene(vm, "scene7040", 0x14a, 0x139, 1, 0xfd, 0xfb) {
}

const char *Scene7040::resourceArchiveName() const {
	return "RESOURCE.G04";
}

uint Scene7040::sceneInitialRequiredChunkCount() const {
	return 20;
}

uint Scene7040::sceneArenaFirstChunk() const {
	return 5;
}

uint Scene7040::sceneArenaLastChunk() const {
	return 18;
}

uint Scene7040::sceneStageIndex() const {
	return 704;
}

const char *Scene7040::sceneDebugName() const {
	return "Scene 7040";
}

uint16 Scene7040::sceneViewportXOffset() const {
	return 0xc8;
}

bool Scene7040::shouldLoadPaletteAfterFrankensteinNote() const {
	return true;
}

bool Scene7040::shouldConvertSavedFramebufferFF() const {
	return true;
}

bool Scene7040::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

bool Scene7040::usesG04PathRouteSpecialCase() const {
	return true;
}

bool Scene7040::isMainFlowStateInScene(uint16 stateId) const {
	return stateId >= 0x1b80 && stateId <= 0x1b89;
}

bool Scene7040::hasCustomEntrySequence() const {
	return true;
}

void Scene7040::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene7040State7041) {
		_soundBank0.playSample(4, 100);
		runEntryPath(kScene7040Entry7041StartX, kScene7040Entry7041StartY, kScene7040Entry7041Facing,
			kScene7040Entry7041StartX, kScene7040Entry7041StartY);
		return;
	}

	if (state.mainFlowStateId == kScene7040State7042) {
		runEntryPath(kScene7040Entry7042StartX, kScene7040Entry7042StartY, kScene7040Entry7042Facing,
			kScene7040Entry7042TargetX, kScene7040Entry7042TargetY);
		return;
	}

	const bool seenJosephGuestListGreeting = state.seenJosephGuestListGreeting;
	const int targetX = seenJosephGuestListGreeting ?
		kScene7040Entry7040RepeatTargetX : kScene7040Entry7040FirstTargetX;
	const int targetY = seenJosephGuestListGreeting ?
		kScene7040Entry7040RepeatTargetY : kScene7040Entry7040FirstTargetY;
	runEntryPath(kScene7040Entry7040StartX, kScene7040Entry7040StartY,
		kScene7040Entry7040Facing, targetX, targetY);
	if (!seenJosephGuestListGreeting && !shouldStopJosephGuestListGreeting())
		runJosephGuestListGreeting();
}

void Scene7040::runJosephGuestListGreeting() {
	beginSecondarySpeechLine(kScene7040DialogueStageId, 6);
	if (shouldStopJosephGuestListGreeting())
		return;
	_preItemIdleState = 2;
	waitPreItemIdleSequence();
	if (shouldStopJosephGuestListGreeting())
		return;
	_preItemIdleState = 3;
	beginPrimarySpeechLine(kScene7040DialoguePrimaryRow, 6, kScene7040DialoguePrimaryCenterX,
		kScene7040DialoguePrimaryTopY, kScene7040DialoguePrimaryRed, kScene7040DialoguePrimaryGreen,
		kScene7040DialoguePrimaryBlue);
	if (shouldStopJosephGuestListGreeting())
		return;
	beginSecondarySpeechLine(kScene7040DialogueStageId, 7);
	if (shouldStopJosephGuestListGreeting())
		return;
	beginPrimarySpeechLine(kScene7040DialoguePrimaryRow, 7, kScene7040DialoguePrimaryCenterX,
		kScene7040DialoguePrimaryTopY, kScene7040DialoguePrimaryRed, kScene7040DialoguePrimaryGreen,
		kScene7040DialoguePrimaryBlue);
	if (shouldStopJosephGuestListGreeting())
		return;
	_chunk11FrameIndex = 0;
	_preItemIdleState = 0;
	_vm->gameState().seenJosephGuestListGreeting = true;
}

void Scene7040::waitPreItemIdleSequence() {
	for (uint step = 0; _preItemIdleState != 0 && step < 8 && !Engine::shouldQuit(); ++step) {
		if (waitSceneMillis(kScene7040Chunk11FrameMillis))
			break;
	}
}

bool Scene7040::shouldStopJosephGuestListGreeting() {
	if (!_skipRequested && !Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		return false;

	_chunk11FrameIndex = 0;
	_preItemIdleState = 0;
	return true;
}

bool Scene7040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a terraza (go to terrace)
		handleActionSlot00ReturnToG03();
		return true;
	case 302: // Mirar puerta (look at door)
		handleActionSlot01ProgressSpeech();
		return true;
	case 303: // Usar/Abrir puerta (use/open door)
		handleActionSlot02MajorHotspotAction();
		return true;
	case 304: // Ir a escalera (go to stairs)
		handleActionSlot03TransitionToState7060();
		return true;
	case 305: // Mirar escalera (look at stairs)
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar puerta (look at door)
		handleActionSlot05ExitProgressSpeech();
		return true;
	case 307: // Usar/Abrir puerta (use/open door)
		handleActionSlot06TransitionToG05();
		return true;
	case 308: // Mirar estatua (look at statue)
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar florero (look at vase)
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Coger libreta (take notebook)
		handleActionSlot09PickupItem0FThenExit();
		return true;
	case 311: // Usar libreta (use notebook)
		handleActionSlot10CommonSpeech();
		return true;
	case 312: // Mirar libreta (look at notebook)
		handleActionHandler312ProgressSpeech();
		return true;
	case 313: // Hablar con Joseph (talk to Joseph)
		handleActionHandler313ConversationGate();
		return true;
	case 314: // Mirar Joseph (look at Joseph)
		handleActionHandler314FrankensteinNoteSpeech();
		return true;
	case 315: // Coger florero (take vase)
		handleActionHandler315PickupItem0C();
		return true;
	default:
		return false;
	}
}

} // End of namespace Hollywood
