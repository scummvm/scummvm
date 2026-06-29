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

#include "hollywood/scenes/playable/scene7050.h"

namespace Hollywood {

Scene7050::Scene7050(HollywoodEngine *vm) :
		SuePlayableScene(vm, "scene7050", 0x0a1, 0x158, 2, 0xfd, 0xfb) {
}

const char *Scene7050::resourceArchiveName() const {
	return "RESOURCE.G05";
}

uint Scene7050::sceneInitialRequiredChunkCount() const {
	return 12;
}

uint Scene7050::sceneArenaFirstChunk() const {
	return 5;
}

uint Scene7050::sceneArenaLastChunk() const {
	return 11;
}

uint Scene7050::sceneStageIndex() const {
	return 705;
}

const char *Scene7050::sceneDebugName() const {
	return "Scene 7050";
}

uint16 Scene7050::sceneViewportXOffset() const {
	return 0x68;
}

bool Scene7050::usesSingleSecondaryActorComposite() const {
	return true;
}

bool Scene7050::isMainFlowStateInScene(uint16 stateId) const {
	return stateId == 0x1b8a;
}

bool Scene7050::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 306:
	case 307:
		return true;
	case 301: // Mirar puerta (look at door)
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar/Abrir puerta (use/open door)
		handleG05ActionSlot01ReturnToG04();
		return true;
	case 303: // Hablar con empleado del guardarropa (talk to cloakroom attendant)
		runG05DialogueMenuRow98();
		applySceneStateToHotspotsAndPatches(0);
		return true;
	case 304: // Mirar empleado del guardarropa (look at cloakroom attendant)
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Mirar trapo (look at rag)
		beginSecondarySpeechLine(3, 0);
		return true;
	case 308: // Mirar caja/Charlie (look at box/Charlie)
		beginSecondarySpeechLine(6, 0);
		return true;
	case 311: // Coger trapo (take rag)
		handleG05ActionSlot10PickupItem10();
		return true;
	default:
		return false;
	}
}

} // End of namespace Hollywood
