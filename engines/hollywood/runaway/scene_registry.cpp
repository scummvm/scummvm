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

#include "common/util.h"
#include "hollywood/runaway/scene_registry.h"
#include "hollywood/runaway/scene4030.h"
#include "hollywood/runaway/scene4050.h"
#include "hollywood/runaway/scene4060.h"
#include "hollywood/runaway/scene4070.h"
#include "hollywood/runaway/scene4090.h"
#include "hollywood/runaway/scene4100.h"
#include "hollywood/runaway/scene4140.h"
#include "hollywood/runaway/scene6014.h"
#include "hollywood/runaway/scene6080.h"
#include "hollywood/runaway/scene6090.h"
#include "hollywood/runaway/scene6130.h"
#include "hollywood/runaway/scene6180.h"
#include "hollywood/runaway/scene6190.h"
#include "hollywood/runaway/scene6200.h"
#include "hollywood/runaway/scene6210.h"
#include "hollywood/runaway/scene6220.h"
#include "hollywood/runaway/scene6240.h"
#include "hollywood/runaway/scene6260.h"
#include "hollywood/runaway/scene6263.h"
#include "hollywood/runaway/scene6270.h"
#include "hollywood/runaway/scene6290.h"
#include "hollywood/runaway/scene6350.h"
#include "hollywood/runaway/scene6370.h"

namespace Hollywood {
namespace {

typedef RunawayScene *(*SceneFactory)(RunawayAudio &, RunawayInventory &, const RunawayStrings &, RunawayGameType);

struct SceneDefinition {
	const RunawayRoom *room;
	SceneFactory create;
};

template<class Scene>
RunawayScene *createScene(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) {
	return new Scene(audio, inventory, strings, game);
}

const SceneDefinition kSceneDefinitions[] = {
	{&kRunawayScene2020, createScene<RunawayRoomScene>},
	{&kRunawayScene2060, createScene<RunawayRoomScene>},
	{&kRunawayScene2080, createScene<RunawayRoomScene>},
	{&kRunawayScene4030, createScene<Runaway::Scene4030>},
	{&kRunawayScene4050, createScene<Runaway::Scene4050>},
	{&kRunawayScene4060, createScene<Runaway::Scene4060>},
	{&kRunawayScene4070, createScene<Runaway::Scene4070>},
	{&kRunawayScene4090, createScene<Runaway::Scene4090>},
	{&kRunawayScene4100, createScene<Runaway::Scene4100>},
	{&kRunawayScene4140, createScene<Runaway::Scene4140>},
	{&kRunawayScene5010, createScene<RunawayRoomScene>},
	{&kRunawayScene5020, createScene<RunawayRoomScene>},
	{&kRunawayScene5050, createScene<RunawayRoomScene>},
	{&kRunawayScene5060, createScene<RunawayRoomScene>},
	{&kRunawayScene5070, createScene<RunawayRoomScene>},
	{&kRunawayScene5080, createScene<RunawayRoomScene>},
	{&kRunawayScene5100, createScene<RunawayRoomScene>},
	{&kRunawayScene5110, createScene<RunawayRoomScene>},
	{&kRunawayScene5130, createScene<RunawayRoomScene>},
	{&kRunawayScene5140, createScene<RunawayRoomScene>},
	{&kRunawayScene5160, createScene<RunawayRoomScene>},
	{&kRunawayScene5170, createScene<RunawayRoomScene>},
	{&kRunawayScene6014, createScene<Runaway::Scene6014>},
	{&kRunawayScene6080, createScene<Runaway::Scene6080>},
	{&kRunawayScene6090, createScene<Runaway::Scene6090>},
	{&kRunawayScene6130, createScene<Runaway::Scene6130>},
	{&kRunawayScene6180, createScene<Runaway::Scene6180>},
	{&kRunawayScene6190, createScene<Runaway::Scene6190>},
	{&kRunawayScene6200, createScene<Runaway::Scene6200>},
	{&kRunawayScene6210, createScene<Runaway::Scene6210>},
	{&kRunawayScene6220, createScene<Runaway::Scene6220>},
	{&kRunawayScene6240, createScene<Runaway::Scene6240>},
	{&kRunawayScene6260, createScene<Runaway::Scene6260>},
	{&kRunawayScene6263, createScene<Runaway::Scene6263>},
	{&kRunawayScene6270, createScene<Runaway::Scene6270>},
	{&kRunawayScene6290, createScene<Runaway::Scene6290>},
	{&kRunawayScene6350, createScene<Runaway::Scene6350>},
	{&kRunawayScene6370, createScene<Runaway::Scene6370>}
};

} // End of anonymous namespace

const RunawayRoom *findRunawayRoom(uint16 number) {
	if (number >= 2021 && number <= 2023) {
		number = kHospitalBedroom;
	}
	if (number == 4010 || (number >= 4061 && number <= 4065)) {
		number = kMuseumHall;
	} else if (number >= 4071 && number <= 4072) {
		number = kMuseumWilly;
	} else if (number == 4101) {
		number = kMuseumSecurity;
	} else if (number >= 4141 && number <= 4142) {
		number = kMuseumKeypad;
	} else if (number == 5021 || number == 5022 || number == 5061 || number == 5062 ||
			number == 5081 || number == 5101 || number == 5171) {
		number -= number % 10;
	} else if (number >= 6221 && number <= 6225) {
		number = 6220;
	} else if (number == 6015) {
		number = 6014;
	} else if (number == 6081 || number == 6082) {
		number = 6080;
	} else if (number == 6131) {
		number = 6130;
	} else if (number == 6181) {
		number = 6180;
	} else if (number == 6241) {
		number = 6240;
	} else if (number == 6201) {
		number = 6200;
	} else if (number == 6261 || number == 6262 || number == 6265) {
		number = 6260;
	} else if (number == 6264) {
		number = 6263;
	} else if (number == 6291) {
		number = 6290;
	}
	for (uint i = 0; i < ARRAYSIZE(kSceneDefinitions); ++i) {
		if (kSceneDefinitions[i].room->number == number) {
			return kSceneDefinitions[i].room;
		}
	}
	return nullptr;
}

RunawayScene *createRunawayRoomScene(const RunawayRoom &room, RunawayAudio &audio, RunawayInventory &inventory,
		const RunawayStrings &strings, RunawayGameType game) {
	for (uint i = 0; i < ARRAYSIZE(kSceneDefinitions); ++i) {
		if (kSceneDefinitions[i].room->number == room.number) {
			return kSceneDefinitions[i].create(audio, inventory, strings, game);
		}
	}
	return nullptr;
}

} // End of namespace Hollywood
