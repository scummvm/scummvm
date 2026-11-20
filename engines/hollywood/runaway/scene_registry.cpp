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
#include "hollywood/runaway/scene2020.h"
#include "hollywood/runaway/scene2060.h"
#include "hollywood/runaway/scene2080.h"
#include "hollywood/runaway/scene4030.h"
#include "hollywood/runaway/scene4050.h"
#include "hollywood/runaway/scene4060.h"
#include "hollywood/runaway/scene4070.h"
#include "hollywood/runaway/scene4090.h"
#include "hollywood/runaway/scene4100.h"
#include "hollywood/runaway/scene4140.h"
#include "hollywood/runaway/scene5010.h"
#include "hollywood/runaway/scene5020.h"
#include "hollywood/runaway/scene5050.h"
#include "hollywood/runaway/scene5060.h"
#include "hollywood/runaway/scene5070.h"
#include "hollywood/runaway/scene5080.h"
#include "hollywood/runaway/scene5100.h"
#include "hollywood/runaway/scene5110.h"
#include "hollywood/runaway/scene5130.h"
#include "hollywood/runaway/scene5140.h"
#include "hollywood/runaway/scene5160.h"
#include "hollywood/runaway/scene5170.h"
#include "hollywood/runaway/scene6030.h"
#include "hollywood/runaway/scene6080.h"
#include "hollywood/runaway/scene6090.h"
#include "hollywood/runaway/scene6130.h"
#include "hollywood/runaway/scene6180.h"
#include "hollywood/runaway/scene6190.h"
#include "hollywood/runaway/scene6200.h"
#include "hollywood/runaway/scene6210.h"
#include "hollywood/runaway/scene6220.h"
#include "hollywood/runaway/scene6226.h"
#include "hollywood/runaway/scene6240.h"
#include "hollywood/runaway/scene6250.h"
#include "hollywood/runaway/scene6260.h"
#include "hollywood/runaway/scene6263.h"
#include "hollywood/runaway/scene6270.h"
#include "hollywood/runaway/scene6280.h"
#include "hollywood/runaway/scene6290.h"
#include "hollywood/runaway/scene6300.h"
#include "hollywood/runaway/scene6350.h"
#include "hollywood/runaway/scene6370.h"
#include "hollywood/runaway/scene6390.h"
#include "hollywood/runaway/scene7010.h"
#include "hollywood/runaway/scene7020.h"
#include "hollywood/runaway/scene7030.h"
#include "hollywood/runaway/scene7040.h"
#include "hollywood/runaway/scene7050.h"
#include "hollywood/runaway/scene8090.h"
#include "hollywood/runaway/scene8100.h"
#include "hollywood/runaway/scene8130.h"
#include "hollywood/runaway/scene8180.h"
#include "hollywood/runaway/scene8190.h"
#include "hollywood/runaway/scene8200.h"
#include "hollywood/runaway/scene8210.h"
#include "hollywood/runaway/scene8220.h"
#include "hollywood/runaway/scene8226.h"
#include "hollywood/runaway/scene8230.h"
#include "hollywood/runaway/scene8240.h"
#include "hollywood/runaway/scene8260.h"
#include "hollywood/runaway/scene8263.h"
#include "hollywood/runaway/scene8290.h"
#include "hollywood/runaway/scene8300.h"
#include "hollywood/runaway/scene8350.h"
#include "hollywood/runaway/scene8370.h"
#include "hollywood/runaway/scene8380.h"
#include "hollywood/runaway/scene8400.h"
#include "hollywood/runaway/scene8410.h"
#include "hollywood/runaway/scene8420.h"
#include "hollywood/runaway/scene9010.h"
#include "hollywood/runaway/scene9020.h"
#include "hollywood/runaway/scene9030.h"

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
	{&kRunawayScene2020, createScene<Runaway::Scene2020>},
	{&kRunawayScene2060, createScene<Runaway::Scene2060>},
	{&kRunawayScene2080, createScene<Runaway::Scene2080>},
	{&kRunawayScene4030, createScene<Runaway::Scene4030>},
	{&kRunawayScene4050, createScene<Runaway::Scene4050>},
	{&kRunawayScene4060, createScene<Runaway::Scene4060>},
	{&kRunawayScene4070, createScene<Runaway::Scene4070>},
	{&kRunawayScene4090, createScene<Runaway::Scene4090>},
	{&kRunawayScene4100, createScene<Runaway::Scene4100>},
	{&kRunawayScene4140, createScene<Runaway::Scene4140>},
	{&kRunawayScene5010, createScene<Runaway::Scene5010>},
	{&kRunawayScene5020, createScene<Runaway::Scene5020>},
	{&kRunawayScene5050, createScene<Runaway::Scene5050>},
	{&kRunawayScene5060, createScene<Runaway::Scene5060>},
	{&kRunawayScene5070, createScene<Runaway::Scene5070>},
	{&kRunawayScene5080, createScene<Runaway::Scene5080>},
	{&kRunawayScene5100, createScene<Runaway::Scene5100>},
	{&kRunawayScene5110, createScene<Runaway::Scene5110>},
	{&kRunawayScene5130, createScene<Runaway::Scene5130>},
	{&kRunawayScene5140, createScene<Runaway::Scene5140>},
	{&kRunawayScene5160, createScene<Runaway::Scene5160>},
	{&kRunawayScene5170, createScene<Runaway::Scene5170>},
	{&kRunawayScene6030, createScene<Runaway::Scene6030>},
	{&kRunawayScene6080, createScene<Runaway::Scene6080>},
	{&kRunawayScene6090, createScene<Runaway::Scene6090>},
	{&kRunawayScene6130, createScene<Runaway::Scene6130>},
	{&kRunawayScene6180, createScene<Runaway::Scene6180>},
	{&kRunawayScene6190, createScene<Runaway::Scene6190>},
	{&kRunawayScene6200, createScene<Runaway::Scene6200>},
	{&kRunawayScene6210, createScene<Runaway::Scene6210>},
	{&kRunawayScene6220, createScene<Runaway::Scene6220>},
	{&kRunawayScene6226, createScene<Runaway::Scene6226>},
	{&kRunawayScene6240, createScene<Runaway::Scene6240>},
	{&kRunawayScene6250, createScene<Runaway::Scene6250>},
	{&kRunawayScene6260, createScene<Runaway::Scene6260>},
	{&kRunawayScene6263, createScene<Runaway::Scene6263>},
	{&kRunawayScene6270, createScene<Runaway::Scene6270>},
	{&kRunawayScene6280, createScene<Runaway::Scene6280>},
	{&kRunawayScene6290, createScene<Runaway::Scene6290>},
	{&kRunawayScene6300, createScene<Runaway::Scene6300>},
	{&kRunawayScene6350, createScene<Runaway::Scene6350>},
	{&kRunawayScene6370, createScene<Runaway::Scene6370>},
	{&kRunawayScene6390, createScene<Runaway::Scene6390>},
	{&kRunawayScene7010, createScene<Runaway::Scene7010>},
	{&kRunawayScene7020, createScene<Runaway::Scene7020>},
	{&kRunawayScene7030, createScene<Runaway::Scene7030>},
	{&kRunawayScene7040, createScene<Runaway::Scene7040>},
	{&kRunawayScene7050, createScene<Runaway::Scene7050>},
	{&kRunawayScene8090, createScene<Runaway::Scene8090>},
	{&kRunawayScene8100, createScene<Runaway::Scene8100>},
	{&kRunawayScene8130, createScene<Runaway::Scene8130>},
	{&kRunawayScene8180, createScene<Runaway::Scene8180>},
	{&kRunawayScene8190, createScene<Runaway::Scene8190>},
	{&kRunawayScene8200, createScene<Runaway::Scene8200>},
	{&kRunawayScene8210, createScene<Runaway::Scene8210>},
	{&kRunawayScene8220, createScene<Runaway::Scene8220>},
	{&kRunawayScene8226, createScene<Runaway::Scene8226>},
	{&kRunawayScene8230, createScene<Runaway::Scene8230>},
	{&kRunawayScene8240, createScene<Runaway::Scene8240>},
	{&kRunawayScene8260, createScene<Runaway::Scene8260>},
	{&kRunawayScene8263, createScene<Runaway::Scene8263>},
	{&kRunawayScene8290, createScene<Runaway::Scene8290>},
	{&kRunawayScene8300, createScene<Runaway::Scene8300>},
	{&kRunawayScene8350, createScene<Runaway::Scene8350>},
	{&kRunawayScene8370, createScene<Runaway::Scene8370>},
	{&kRunawayScene8380, createScene<Runaway::Scene8380>},
	{&kRunawayScene8400, createScene<Runaway::Scene8400>},
	{&kRunawayScene8410, createScene<Runaway::Scene8410>},
	{&kRunawayScene8420, createScene<Runaway::Scene8420>},
	{&kRunawayScene9010, createScene<Runaway::Scene9010>},
	{&kRunawayScene9020, createScene<Runaway::Scene9020>},
	{&kRunawayScene9030, createScene<Runaway::Scene9030>}
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
	} else if (number == 6227 || number == 6228) {
		number = 6226;
	} else if (number == 6031 || number == 6014 || number == 6015) {
		// Preserve the shaft scene numbers used by early chapter-four saves.
		number = 6030;
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
	} else if (number >= 6301 && number <= 6303) {
		number = 6300;
	} else if (number == 6191) {
		number = 6190;
	} else if (number == 6251 || number == 6252) {
		number = 6250;
	} else if (number == 6391) {
		number = 6390;
	} else if (number == 6291) {
		number = 6290;
	} else if (number == 7021 || number == 7022) {
		number = 7020;
	} else if (number == 7031) {
		number = 7030;
	} else if (number >= 8091 && number <= 8094) {
		number = 8090;
	} else if (number >= 8101 && number <= 8103) {
		number = 8100;
	} else if (number == 8131 || number == 8132) {
		number = 8130;
	} else if (number == 8181 || number == 8182) {
		number = 8180;
	} else if (number == 8191) {
		number = 8190;
	} else if (number == 8201) {
		number = 8200;
	} else if ((number >= 8221 && number <= 8225) || number == 8229) {
		number = 8220;
	} else if (number == 8227 || number == 8228) {
		number = 8226;
	} else if (number == 8231 || number == 8232) {
		number = 8230;
	} else if (number >= 8241 && number <= 8243) {
		number = 8240;
	} else if (number == 8261 || number == 8262 || number == 8265) {
		number = 8260;
	} else if (number == 8264) {
		number = 8263;
	} else if (number >= 8301 && number <= 8303) {
		number = 8300;
	} else if (number >= 8371 && number <= 8375) {
		number = 8370;
	} else if (number >= 8381 && number <= 8385) {
		number = 8380;
	} else if (number >= 8401 && number <= 8403) {
		number = 8400;
	} else if (number == 8421 || number == 8422) {
		number = 8420;
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
