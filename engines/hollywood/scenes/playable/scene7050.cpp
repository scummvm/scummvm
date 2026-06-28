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

} // End of namespace Hollywood
