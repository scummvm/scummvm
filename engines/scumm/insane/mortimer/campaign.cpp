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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/insane/mortimer/campaign.h"

#include "common/util.h"

namespace Scumm {

const MortimerRegion kMortimerRegions[] = {
	{"RESOURCE/SAVANNA", 'S',
		{"RHI", "BAB", "CHE", "ELE", "GIR", "LIO", "OXP"},
		{"Rhinoceros", "Baboon", "Cheetah", "Elephant", "Giraffe", "Lion", "Oxpecker"},
		"RESOURCE/SAVANNA/SAVANNA1.SAN", "RESOURCE/DATA/SAV_SONG.TRS"},
	{"RESOURCE/FOREST", 'F',
		{"GRI", "BEA", "BAL", "FLY", "GOA", "MOO", "OPO"},
		{"Grizzly Bear", "Beaver", "Bald Eagle", "Flying Squirrel", "Rocky Mountain Goat", "Moose", "Opossum"},
		"RESOURCE/FOREST/FOREST1.SAN", "RESOURCE/DATA/FOR_SONG.TRS"},
	{"RESOURCE/ICE", 'I',
		{"WOL", "HUM", "KIL", "NAR", "POL", "TER", "WAL"},
		{"Gray Wolf", "Humpback Whale", "Killer Whale", "Narwhal", "Polar Bear", "Arctic Tern", "Walrus"},
		"RESOURCE/ICE/ICE1.SAN", "RESOURCE/DATA/ICE_SONG.TRS"},
	{"RESOURCE/DESERT", 'D',
		{"COY", "ELF", "GIL", "HOR", "LBA", "RAT", "ROA"},
		{"Coyote", "Elf Owl", "Gila Monster", "Horned Lizard", "Long-Tongued Bat", "Rattlesnake", "Roadrunner"},
		"RESOURCE/DESERT/DES_01.SAN", "RESOURCE/DATA/DES_SONG.TRS"}
};

const MortimerRegion &mortimerRegion(uint region) {
	assert(region < ARRAYSIZE(kMortimerRegions));
	return kMortimerRegions[region];
}

uint mortimerGuessCost(uint region, uint difficulty) {
	const uint costs[3][4] = {{7, 9, 12, 15}, {12, 15, 18, 25}, {15, 18, 25, 30}};
	assert(region < 4 && difficulty < 3);
	return costs[difficulty][region];
}

int mortimerGuessAllowance(uint difficulty) {
	const int guesses[] = {-1, 3, 1};
	assert(difficulty < ARRAYSIZE(guesses));
	return guesses[difficulty];
}

const MortimerCampaign::Scene kMortimerScenes[] = {
	{24, 5, MortimerCampaign::kMovie, "RESOURCE/BEGIN/GOLDGUY.SAN", "RESOURCE/DATA/BEGIN.TRS", true},
	{25, 5, MortimerCampaign::kMovie, "RESOURCE/BEGIN/INTRO1_H.SAN", "RESOURCE/DATA/BEGIN.TRS", true},
	{26, 5, MortimerCampaign::kMovie, "RESOURCE/BEGIN/INTRO2_H.SAN", "RESOURCE/DATA/BEGIN.TRS", true},
	{27, 5, MortimerCampaign::kMovie, "RESOURCE/BEGIN/INTRO3_H.SAN", "RESOURCE/DATA/BEGIN.TRS", true},
	{28, 5, MortimerCampaign::kMovie, "RESOURCE/BEGIN/INTRO4_H.SAN", nullptr, true},
	{29, 5, MortimerCampaign::kMovie, "RESOURCE/BEGIN/INTRO5H%c.SAN", "RESOURCE/DATA/BEGIN.TRS", false},
	{30, 5, MortimerCampaign::kMovie, "RESOURCE/BEGIN/TITLE.SAN", "RESOURCE/DATA/BEGIN.TRS", true},
	{0, 0, MortimerCampaign::kFlight, "RESOURCE/SAVANNA/SAVANNA1.SAN", "RESOURCE/DATA/MORTIMER.TRS", true},
	{12, 0, MortimerCampaign::kInterview, "RESOURCE/SAVANNA/SGATE.SAN", "RESOURCE/DATA/SAV_SONG.TRS", true},
	{17, 0, MortimerCampaign::kInterview, nullptr, nullptr, true},
	{4, 0, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/MORTIMER.TRS", true},
	{8, 0, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/SAV_SONG.TRS", true},
	{31, 0, MortimerCampaign::kMovie, "RESOURCE/ENDING/MID_1.SAN", "RESOURCE/DATA/MIDTEXT.TRS", true},
	{1, 1, MortimerCampaign::kFlight, "RESOURCE/FOREST/FOREST1.SAN", "RESOURCE/DATA/MORTIMER.TRS", true},
	{13, 1, MortimerCampaign::kInterview, "RESOURCE/FOREST/FGATE.SAN", "RESOURCE/DATA/FOR_SONG.TRS", true},
	{18, 1, MortimerCampaign::kInterview, nullptr, nullptr, true},
	{5, 1, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/MORTIMER.TRS", true},
	{9, 1, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/FOR_SONG.TRS", true},
	{32, 1, MortimerCampaign::kMovie, "RESOURCE/ENDING/MID_2.SAN", "RESOURCE/DATA/MIDTEXT.TRS", true},
	{2, 2, MortimerCampaign::kFlight, "RESOURCE/ICE/ICE1.SAN", "RESOURCE/DATA/MORTIMER.TRS", true},
	{14, 2, MortimerCampaign::kInterview, "RESOURCE/ICE/IGATE.SAN", "RESOURCE/DATA/ICE_SONG.TRS", true},
	{19, 2, MortimerCampaign::kInterview, nullptr, nullptr, true},
	{6, 2, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/MORTIMER.TRS", true},
	{10, 2, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/ICE_SONG.TRS", true},
	{33, 2, MortimerCampaign::kMovie, "RESOURCE/ENDING/MID_3.SAN", "RESOURCE/DATA/MIDTEXT.TRS", true},
	{3, 3, MortimerCampaign::kFlight, "RESOURCE/DESERT/DES_01.SAN", "RESOURCE/DATA/MORTIMER.TRS", true},
	{15, 3, MortimerCampaign::kInterview, "RESOURCE/DESERT/DGATE.SAN", "RESOURCE/DATA/DES_SONG.TRS", true},
	{20, 3, MortimerCampaign::kInterview, nullptr, nullptr, true},
	{7, 3, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/MORTIMER.TRS", true},
	{11, 3, MortimerCampaign::kInterview, nullptr, "RESOURCE/DATA/DES_SONG.TRS", true},
	{34, 2, MortimerCampaign::kMovie, "RESOURCE/ENDING/END_1.SAN", "RESOURCE/DATA/ENDTEXT.TRS", true},
	{35, 2, MortimerCampaign::kMovie, "RESOURCE/ENDING/END_2H%c.SAN", "RESOURCE/DATA/ENDTEXT.TRS", true},
	{21, 4, MortimerCampaign::kCastle, "RESOURCE/CASTLE/CS_ENTRY.SAN", "RESOURCE/DATA/CAS_SONG.TRS", true},
	{16, 4, MortimerCampaign::kCastleRiddle, "RESOURCE/CASTLE/CS_GATE.SAN", "RESOURCE/DATA/ENDTEXT.TRS", true},
	{22, 4, MortimerCampaign::kCastleRiddle, nullptr, "RESOURCE/DATA/CAS_SONG.TRS", true},
	{23, 4, MortimerCampaign::kCastleRiddle, "RESOURCE/CASTLE/CASTLE_I.NUT", "RESOURCE/DATA/CAS_SONG.TRS", true},
	{36, 6, MortimerCampaign::kMovie, "RESOURCE/ENDING/END_7.SAN", "RESOURCE/DATA/ENDTEXT.TRS", true},
	{37, 6, MortimerCampaign::kMovie, "RESOURCE/ENDING/END_8H%c.SAN", "RESOURCE/DATA/ENDTEXT.TRS", true},
	{38, 6, MortimerCampaign::kMovie, "RESOURCE/ENDING/END_9.SAN", "RESOURCE/DATA/ENDTEXT.TRS", true},
	{39, 6, MortimerCampaign::kCredits, "RESOURCE/ENDING/CREDITS.SAN", "RESOURCE/DATA/HISCORE.TRS", true},
};

MortimerCampaign::MortimerCampaign() : _index(0) {
}

bool MortimerCampaign::start(int sceneId) {
	for (uint i = 0; i < ARRAYSIZE(kMortimerScenes); ++i) {
		if (kMortimerScenes[i].id == sceneId) {
			_index = i;
			return true;
		}
	}
	return false;
}

bool MortimerCampaign::advance() {
	if (!finished())
		++_index;
	return !finished();
}

bool MortimerCampaign::returnToFlight() {
	return !finished() && scene().region < 4 && start(scene().region);
}

const MortimerCampaign::Scene &MortimerCampaign::scene() const {
	assert(!finished());
	return kMortimerScenes[_index];
}

Common::String MortimerCampaign::movieName(bool girl) const {
	if (!scene().movie)
		return Common::String();
	return Common::String::format(scene().movie, girl ? 'G' : 'B');
}

bool MortimerCampaign::finished() const {
	return _index >= ARRAYSIZE(kMortimerScenes);
}

} // End of namespace Scumm
