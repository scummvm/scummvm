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

bool Scene7040::shouldLoadAlternatePaletteAfterItem0B() const {
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

} // End of namespace Hollywood
