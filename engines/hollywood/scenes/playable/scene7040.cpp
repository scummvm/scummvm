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

namespace Hollywood {

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
