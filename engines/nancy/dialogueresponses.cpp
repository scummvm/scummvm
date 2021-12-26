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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/dialogueresponses.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {

struct ResponseInFile {
	uint address;
	uint scene;
};

static const ResponseInFile nancy1ResponsesDaryl[] = {
	{ 0xB2820, 0x7C },
	{ 0xB27E4, 0x7F },
	{ 0xB279C, 0x81 },
	{ 0xB2730, 0x83 },
	{ 0xB26D4, 0x84 },
	{ 0xB25CC, 0x86 },
	{ 0xB2534, 0x8B },
	{ 0xB24D0, 0x8D },
	{ 0xB2438, 0x8F },
	{ 0xB239C, 0x90 },
	{ 0xB2300, 0x91 },
	{ 0xB228C, 0x92 },
	{ 0xB21D0, 0x96 },
	{ 0xB2170, 0x97 },
	{ 0xB20D0, 0x9C },
	{ 0xB2074, 0x93 },
	{ 0xB2038, 0x94 },
	{ 0xB1FE0, 0x95 }
};

static const ResponseInFile nancy1ResponsesConnie[] = {
	{ 0xB2BC4, 0xE9 },
	{ 0xB2B6C, 0xEA },
	{ 0xB2B34, 0xEB },
	{ 0xB2AF4, 0xEC },
	{ 0xB2A9C, 0xED },
	{ 0xB29E0, 0xEE },
	{ 0xB26D4, 0xEF },
	{ 0xB2948, 0xF0 },
	{ 0xB28A8, 0xF5 },
	{ 0xB2864, 0xE7 }
};

static const ResponseInFile nancy1ResponsesHal[] = {
	{ 0xB2EB0, 0x1B3 },
	{ 0xB27E4, 0x1B5 },
	{ 0xB2E54, 0x1B6 },
	{ 0xB2E0C, 0x1B7 },
	{ 0xB2DB4, 0x1B9 },
	{ 0xB2D28, 0x1BA },
	{ 0xB26D4, 0x1BB },
	{ 0xB2CA8, 0x1BC },
	{ 0xB2C0C, 0x1BE }
};

static const ResponseInFile nancy1ResponsesHulk[] = {
	{ 0xB3144, 0x14D },
	{ 0xB2B34, 0x150 },
	{ 0xB30B8, 0x153 },
	{ 0xB2E0C, 0x154 },
	{ 0xB306C, 0x155 },
	{ 0xB26D4, 0x156 },
	{ 0xB3008, 0x157 },
	{ 0xB2F90, 0x158 },
	{ 0xB2EF0, 0x159 }
};

#define ADD_RESPONSE(x) responses.push_back(PlayerResponse()); responses.back().sceneChange.sceneID = x.scene; responses.back().sceneChange.doNotStartSound = true; addresses.push_back(x.address);

const Common::Array<PlayerResponse> getResponsesNancy1(uint characterID) {
	Common::Array<PlayerResponse> responses;
	Common::Array<uint> addresses;
	State::Scene &scene = NancySceneState;

	switch (characterID) {
	case 0: // Daryl
		if (	scene.getEventFlag(0x1D, kTrue) &&
				scene.getEventFlag(0x39, kFalse)) {
			ADD_RESPONSE(nancy1ResponsesDaryl[0])
		}

		if (	scene.getEventFlag(0x13, kTrue) &&
				scene.getEventFlag(0x37, kFalse)) {
			ADD_RESPONSE(nancy1ResponsesDaryl[1])
		}

		if (	scene.getEventFlag(0xB, kTrue) &&
				scene.getEventFlag(0x38, kFalse)) {
			ADD_RESPONSE(nancy1ResponsesDaryl[2])
		}

		if (	scene.getEventFlag(0, kTrue) &&
				scene.getEventFlag(1, kFalse) &&
				scene.getEventFlag(0x68, kFalse)) {
			ADD_RESPONSE(nancy1ResponsesDaryl[3])
		}

		

		break;
	case 1: // Connie
		
		break;
	case 2: // Hal
		
		break;
	case 3: // Hulk
		
		break;
	}

	return responses;
}

const Common::Array<PlayerResponse> getResponses(const uint characterID) {
	switch (g_nancy->getGameType()) {
	case kGameTypeNancy1:
		return getResponsesNancy1(characterID);
	default:
		return Common::Array<PlayerResponse>();	
	}
}

} // End of namespace Nancy
