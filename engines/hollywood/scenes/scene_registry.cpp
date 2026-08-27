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

#include "hollywood/scenes/scene_registry.h"

#include "hollywood/scenes/intro/scene1000.h"
#include "hollywood/scenes/intro/scene9090.h"
#include "hollywood/scenes/intro/scene9100.h"
#include "hollywood/scenes/intro/scene9130.h"
#include "hollywood/scenes/intro/scene9140.h"
#include "hollywood/scenes/intro/scene9150.h"
#include "hollywood/scenes/intro/scene9160.h"
#include "hollywood/scenes/intro/scene9170.h"
#include "hollywood/scenes/intro/scene9180.h"
#include "hollywood/scenes/playable/scene1010.h"
#include "hollywood/scenes/playable/scene1020.h"
#include "hollywood/scenes/playable/scene1030.h"
#include "hollywood/scenes/playable/scene1040.h"
#include "hollywood/scenes/playable/scene1050.h"
#include "hollywood/scenes/playable/scene1060.h"
#include "hollywood/scenes/playable/scene1070.h"
#include "hollywood/scenes/playable/scene1080.h"
#include "hollywood/scenes/playable/scene1090.h"
#include "hollywood/scenes/playable/scene2000.h"
#include "hollywood/scenes/playable/scene2010.h"
#include "hollywood/scenes/playable/scene2020.h"
#include "hollywood/scenes/playable/scene2030.h"
#include "hollywood/scenes/playable/scene2040.h"
#include "hollywood/scenes/playable/scene2050.h"
#include "hollywood/scenes/playable/scene2060.h"
#include "hollywood/scenes/playable/scene2070.h"
#include "hollywood/scenes/playable/scene2080.h"
#include "hollywood/scenes/playable/scene2090.h"
#include "hollywood/scenes/playable/scene2100.h"
#include "hollywood/scenes/playable/scene2110.h"
#include "hollywood/scenes/playable/scene3000.h"
#include "hollywood/scenes/playable/scene3010.h"
#include "hollywood/scenes/playable/scene3020.h"
#include "hollywood/scenes/playable/scene3030.h"
#include "hollywood/scenes/playable/scene3040.h"
#include "hollywood/scenes/playable/scene3050.h"
#include "hollywood/scenes/playable/scene3060.h"
#include "hollywood/scenes/playable/scene3070.h"
#include "hollywood/scenes/playable/scene3080.h"
#include "hollywood/scenes/playable/scene3090.h"
#include "hollywood/scenes/playable/scene3100.h"
#include "hollywood/scenes/playable/scene3110.h"
#include "hollywood/scenes/playable/scene4000.h"
#include "hollywood/scenes/playable/scene4010.h"
#include "hollywood/scenes/playable/scene4020.h"
#include "hollywood/scenes/playable/scene4030.h"
#include "hollywood/scenes/playable/scene4040.h"
#include "hollywood/scenes/playable/scene4050.h"
#include "hollywood/scenes/playable/scene4060.h"
#include "hollywood/scenes/playable/scene4070.h"
#include "hollywood/scenes/playable/scene4080.h"
#include "hollywood/scenes/playable/scene4090.h"
#include "hollywood/scenes/playable/scene4100.h"
#include "hollywood/scenes/playable/scene4110.h"
#include "hollywood/scenes/playable/scene5000.h"
#include "hollywood/scenes/playable/scene5010.h"
#include "hollywood/scenes/playable/scene5020.h"
#include "hollywood/scenes/playable/scene5030.h"
#include "hollywood/scenes/playable/scene5040.h"
#include "hollywood/scenes/playable/scene5050.h"
#include "hollywood/scenes/playable/scene5060.h"
#include "hollywood/scenes/playable/scene5070.h"
#include "hollywood/scenes/playable/scene5080.h"
#include "hollywood/scenes/playable/scene5090.h"
#include "hollywood/scenes/playable/scene5100.h"
#include "hollywood/scenes/playable/scene5110.h"
#include "hollywood/scenes/playable/scene5120.h"
#include "hollywood/scenes/playable/scene5130.h"
#include "hollywood/scenes/playable/scene6000.h"
#include "hollywood/scenes/playable/scene6010.h"
#include "hollywood/scenes/playable/scene6020.h"
#include "hollywood/scenes/playable/scene6030.h"
#include "hollywood/scenes/playable/scene6040.h"
#include "hollywood/scenes/playable/scene6050.h"
#include "hollywood/scenes/playable/scene6060.h"
#include "hollywood/scenes/playable/scene6070.h"
#include "hollywood/scenes/playable/scene6080.h"
#include "hollywood/scenes/playable/scene6090.h"
#include "hollywood/scenes/playable/scene6100.h"
#include "hollywood/scenes/playable/scene7000.h"
#include "hollywood/scenes/playable/scene7010.h"
#include "hollywood/scenes/playable/scene7020.h"
#include "hollywood/scenes/playable/scene7030.h"
#include "hollywood/scenes/playable/scene7040.h"
#include "hollywood/scenes/playable/scene7050.h"
#include "hollywood/scenes/playable/scene7060.h"
#include "hollywood/scenes/playable/scene7070.h"
#include "hollywood/scenes/playable/scene7080.h"
#include "hollywood/scenes/playable/scene7090.h"
#include "hollywood/scenes/playable/scene7100.h"
#include "hollywood/scenes/playable/scene8000.h"
#include "hollywood/scenes/playable/scene8010.h"
#include "hollywood/scenes/playable/scene8020.h"

namespace Hollywood {

typedef bool (*SceneRunner)(HollywoodEngine *vm);

struct SceneDefinition {
	uint16 firstState;
	uint16 lastState;
	uint16 bootState;
	SceneRunner run;
};

template<class Scene>
bool playScene(HollywoodEngine *vm) {
	Scene scene(vm);
	return scene.play();
}

bool playScene9101(HollywoodEngine *vm) {
	Scene9100 scene(vm);
	return scene.playDialogueBranch();
}

#define SCENE(id, lastState) { id, lastState, id, &playScene<Scene##id> }
#define BOOT_SCENE(id, lastState, bootState) { id, lastState, bootState, &playScene<Scene##id> }

// A scene number is its first state; bootState records direct-boot exceptions.
const SceneDefinition kSceneDefinitions[] = {
	SCENE(1000, 1000),
	BOOT_SCENE(1010, 0x03fb, 0x03f4),
	SCENE(1020, 0x0405),
	SCENE(1030, 0x0407),
	SCENE(1040, 0x0419),
	SCENE(1050, 0x0423),
	SCENE(1060, 0x042d),
	SCENE(1070, 0x0437),
	SCENE(1080, 0x0441),
	SCENE(1090, 0x044b),
	SCENE(2000, 2000),
	SCENE(2010, 0x07dc),
	SCENE(2020, 0x07ed),
	SCENE(2030, 0x07f7),
	SCENE(2040, 0x0801),
	SCENE(2050, 0x0803),
	BOOT_SCENE(2060, 0x0811, 0x080e),
	SCENE(2070, 0x0817),
	SCENE(2080, 0x0821),
	SCENE(2090, 0x082b),
	SCENE(2100, 0x083d),
	SCENE(2110, 0x083f),
	SCENE(3000, 3000),
	SCENE(3010, 0x0bcb),
	SCENE(3020, 0x0bd5),
	SCENE(3030, 0x0bdf),
	SCENE(3040, 3040),
	SCENE(3050, 0x0bf3),
	SCENE(3060, 0x0bf5),
	SCENE(3070, 0x0c07),
	SCENE(3080, 0x0c11),
	SCENE(3090, 0x0c1b),
	SCENE(3100, 0x0c25),
	SCENE(3110, 0x0c27),
	SCENE(4000, 4000),
	SCENE(4010, 0x0fb3),
	SCENE(4020, 0x0fb5),
	SCENE(4030, 0x0fc7),
	SCENE(4040, 0x0fd1),
	SCENE(4050, 0x0fdb),
	SCENE(4060, 0x0fe5),
	SCENE(4070, 0x0fef),
	SCENE(4080, 4080),
	SCENE(4090, 0x1003),
	SCENE(4100, 0x100d),
	SCENE(4110, 0x1017),
	SCENE(5000, 5000),
	SCENE(5010, 0x139b),
	SCENE(5020, 5020),
	SCENE(5030, 0x13af),
	SCENE(5040, 0x13b9),
	SCENE(5050, 0x13c3),
	SCENE(5060, 0x13cd),
	SCENE(5070, 5070),
	SCENE(5080, 5080),
	SCENE(5090, 0x13eb),
	SCENE(5100, 0x13f5),
	SCENE(5110, 0x13ff),
	SCENE(5120, 0x1409),
	SCENE(5130, 0x1413),
	SCENE(6000, 6000),
	SCENE(6010, 0x1783),
	SCENE(6020, 0x178d),
	SCENE(6030, 0x1797),
	SCENE(6040, 0x17a1),
	SCENE(6050, 0x17ab),
	SCENE(6060, 0x17ad),
	SCENE(6070, 0x17bf),
	SCENE(6080, 0x17c9),
	SCENE(6090, 6090),
	SCENE(6100, 6100),
	SCENE(7000, 7000),
	SCENE(7010, 0x1b6b),
	SCENE(7020, 7020),
	SCENE(7030, 0x1b7f),
	SCENE(7040, 0x1b89),
	SCENE(7050, 7050),
	SCENE(7060, 0x1b9d),
	SCENE(7070, 0x1ba0),
	SCENE(7080, 0x1bb1),
	SCENE(7090, 0x1bbb),
	SCENE(7100, 0x1bc5),
	SCENE(8000, 8000),
	SCENE(8010, 0x1f4b),
	SCENE(8020, 8020),
	SCENE(9090, 9090),
	{ 9101, 9101, 9101, &playScene9101 },
	SCENE(9130, 9130),
	SCENE(9140, 9140),
	SCENE(9150, 9150),
	SCENE(9160, 9160),
	SCENE(9170, 9170),
	SCENE(9180, 9180)
};

#undef BOOT_SCENE
#undef SCENE

const SceneDefinition *findSceneDefinition(uint16 stateId) {
	for (uint i = 0; i < ARRAYSIZE(kSceneDefinitions); ++i) {
		const SceneDefinition &definition = kSceneDefinitions[i];
		if (stateId >= definition.firstState && stateId <= definition.lastState)
			return &definition;
	}

	return nullptr;
}

bool isImplementedGameplayState(int stateId) {
	if (stateId < 0 || stateId > 0xffff)
		return false;

	return stateId == 0xffff || findSceneDefinition((uint16)stateId) != nullptr;
}

bool isGameplayStateInScene(uint16 sceneStateId, uint16 stateId) {
	const SceneDefinition *definition = findSceneDefinition(sceneStateId);
	return definition && definition == findSceneDefinition(stateId);
}

int gameplaySceneNumberForState(uint16 stateId) {
	const SceneDefinition *definition = findSceneDefinition(stateId);
	return definition ? definition->firstState : -1;
}

int gameplayStateForBootParam(int bootParam) {
	for (uint i = 0; i < ARRAYSIZE(kSceneDefinitions); ++i) {
		const SceneDefinition &definition = kSceneDefinitions[i];
		if (bootParam == definition.firstState)
			return definition.bootState;
	}

	return bootParam;
}

GameplaySceneResult runGameplayScene(HollywoodEngine *vm, uint16 stateId) {
	const SceneDefinition *definition = findSceneDefinition(stateId);
	if (!definition)
		return kGameplaySceneNotFound;

	return definition->run(vm) ? kGameplaySceneCompleted : kGameplaySceneFailed;
}

} // End of namespace Hollywood
