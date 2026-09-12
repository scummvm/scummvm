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

#ifndef HOLLYWOOD_SCENES_SCENE_REGISTRY_H
#define HOLLYWOOD_SCENES_SCENE_REGISTRY_H

#include "common/scummsys.h"

namespace Hollywood {

class HollywoodEngine;

enum GameplaySceneResult {
	kGameplaySceneNotFound,
	kGameplaySceneCompleted,
	kGameplaySceneFailed
};

// State validation, direct boot mapping, and runtime dispatch share one scene table.
bool isImplementedGameplayState(int stateId);
bool isGameplayStateInScene(uint16 sceneStateId, uint16 stateId);
int gameplaySceneNumberForState(uint16 stateId);
int gameplayStateForBootParam(int bootParam);
GameplaySceneResult runGameplayScene(HollywoodEngine *vm, uint16 stateId);

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_SCENE_REGISTRY_H
