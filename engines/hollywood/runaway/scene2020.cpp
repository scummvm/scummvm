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

#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene2020.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// RESOURCE.B02; approach points and action IDs from the retail room at 0x444aa0.
const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1265, 546, 1, {301, 346}, {0, 1}, {1324, 530}, 5}, // 1: Bathroom door.
	{1075, 534, 7, {302, 303}, {0, 0}, {1040, 463}, 3, true}, // 2: Gina.
	{846, 453, 1, {304, 305}, {1, 1}, {902, 421}}, // 3: Glass.
	{858, 453, 1, {306, 307}, {1, 1}, {902, 421}}, // 4: Package of pills.
	{622, 457, 7, {308, 309}, {0, 1}, {620, 436}}, // 5: Sheet.
	{417, 444, 7, {310, 311}, {0, 0}, {401, 432}}, // 6: Locker.
	{417, 444, 7, {312, 313}, {0, 1}, {401, 432}}, // 7: Gina's bag.
	{265, 456, 7, {314, 332}, {1, 1}, {237, 438}}, // 8: Window.
	{690, 540, 1, {315, 316}, {1, 1}, {748, 517}}, // 9: Gina's medical chart.
	{453, 455, 1, {317, 318}, {1, 1}, {650, 420}}, // 10: Chart.
	{687, 448, 7, {319, 320}, {0, 1}, {650, 420}}, // 11: Fake patient.
	{459, 473, 7, {321, 322}, {0, 0}, {459, 473}}, // 12: Fire sprinkler.
	{846, 453, 7, {323, 324}, {0, 0}, {982, 412}}, // 13: Buzzer.
	{712, 445, 3, {323, 324}, {0, 0}, {666, 407}}, // 14: Buzzer.
	{265, 456, 7, {325, 326}, {0, 0}, {237, 438}}, // 15: Radiator.
	{713, 533, 1, {327, 328}, {0, 0}, {1046, 489}}, // 16: Poster.
	{454, 455, 1, {329, 328}, {0, 0}, {696, 424}}, // 17: Poster.
	{1265, 546, 1, {330, 331}, {1, 0}, {1324, 530}}, // 18: Emergency exit map.
	{404, 456, 1, {344, 345}, {0, 0}, {650, 420}}, // 19: Bed.
	// IDs 20-22 hold alternate bed/chart shapes and names in the original.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{313, 532, 5, {351, 352}, {-1, 0}, {40, 599}, 7}, // 23: Hallway door.
};

// Item IDs 1-19, then hotspot IDs in kHotspots order. Zero uses kItemActions.
const byte kItemResponses[19][19] = {
	{ 3, 17,  3,  3,  3,  5,  5, 19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 1: Wallet.
	{ 3, 17,  3,  3,  3, 17,  5, 19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 2: Car keys.
	{ 3, 17,  3,  3,  3,  5,  5, 19,  5,  5,  4,  5,  5,  5,  5,  5,  5,  5,  5}, // 3: Order form.
	{ 3, 17,  3,  3,  3,  5,  5, 19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 4: Amber.
	{ 3, 15,  3,  3,  3,  0,  5, 19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 5: Gina's bag.
	{ 3, 17,  3,  3,  3,  5,  5, 19,  0,  0,  5,  5,  3,  3,  5,  5,  5,  5,  1}, // 6: Gina's chart.
	{ 3, 15,  3,  3,  3,  1,  1, 19,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 31}, // 7: Sheet.
	{ 3, 17,  3,  3,  3,  1,  1, 19,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1}, // 8: Pills.
	{29, 15, 29, 29, 29,  5, 29, 19, 29, 29, 29,  0, 29, 29, 29, 29, 29, 29, 29}, // 9: Matches.
	{ 3,  0,  3,  3,  3,  5,  5, 19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, 17}, // 10: Wig.
	{ 5, 17,  3,  3, 15,  5,  5, 19, 17, 17,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 11: Marker.
	{ 5, 17, 17,  3,  3,  5,  5, 19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 12: Alcohol.
	{17,  0,  3,  3,  3,  5,  5, 19, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17}, // 13: Glass.
	{ 5, 17,  3,  3,  3,  5,  5, 19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  0}, // 14: Pillows.
	{ 5,  0, 15,  3,  3,  5,  5, 19,  1,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 15: Syringe.
	{ 5, 17,  3,  3,  3,  5,  5, 19,  0,  0,  5,  5,  5,  5,  5,  5,  5,  5, 24}, // 16: Blank chart.
	{ 5, 17,  3,  3,  3,  5,  5, 19,  3,  3,  5,  5,  3,  3,  5,  3,  3,  3,  0}, // 17: Head.
	{ 5, 15,  3,  3, 15,  5,  5, 19,  5,  5,  5,  0,  5,  5,  5,  5,  5,  5,  5}, // 18: Spray.
	{ 3, 17,  3,  3,  3,  5,  5, 19,  3,  3,  5,  5,  3,  3,  5,  5,  5,  5,  5}, // 19: Medical reference book.
};

const RunawayResponse kResponses[] = {
	{kHospitalBedroom, 301, 1, 0}, // Look at the bathroom door.
	{kHospitalBedroom, 304, 4, 0}, // Look at the glass.
	{kHospitalBedroom, 306, 5, 0}, // Look at the sleeping pills.
	{kHospitalBedroom, 308, 6, 0}, // Look at the sheet.
	{kHospitalBedroom, 310, 7, 0}, // Look at the locker.
	{kHospitalBedroom, 311, 8, 0}, // Open the locker.
	{kHospitalBedroom, 312, 9, 0}, // Look at Gina's bag.
	{kHospitalBedroom, 320, 16, 0}, // Use the fake patient.
	{kHospitalBedroom, 321, 17, 0}, // Look at the fire sprinkler.
	{kHospitalBedroom, 322, 18, 0}, // Use the fire sprinkler.
	{kHospitalBedroom, 323, 19, 0}, // Look at a nurse buzzer.
	{kHospitalBedroom, 325, 21, 0}, // Look at the radiator.
	{kHospitalBedroom, 326, 22, 0}, // Use the radiator.
	{kHospitalBedroom, 327, 23, 0}, // Look at the AIDS poster.
	{kHospitalBedroom, 328, 24, 0}, // Take a poster.
	{kHospitalBedroom, 329, 25, 0}, // Look at the organ donor poster.
	{kHospitalBedroom, 331, 27, 0}, // Take the emergency exit map.
	{kHospitalBedroom, 333, 29, 0}, // Use Gina's bag with the locker.
	{kHospitalBedroom, 344, 39, 0}, // Look at the empty bed.
	{kHospitalBedroom, 345, 40, 0}, // Use the empty bed.
};

const RunawayItemAction kItemActions[] = {
	{kHospitalBedroom, 5, 6, 333, false}, // Use Gina's bag with the locker.
	{kHospitalBedroom, 6, 9, 334, false}, // Put Gina's chart back on her bed.
	{kHospitalBedroom, 6, 10, 335}, // Put Gina's chart on the decoy's bed.
	{kHospitalBedroom, 9, 12, 343}, // Use the matches at the sprinkler.
	{kHospitalBedroom, 10, 2, 336, false}, // Use the wig with Gina.
	{kHospitalBedroom, 13, 2, 349}, // Use the glass with Gina.
	{kHospitalBedroom, 14, 19, 341, false}, // Use the pillows with the empty bed.
	{kHospitalBedroom, 15, 2, 337, false}, // Use the syringe with Gina.
	{kHospitalBedroom, 16, 9, 339}, // Replace Gina's medical chart.
	{kHospitalBedroom, 16, 10, 338, false}, // Put the blank medical chart on the other bed.
	{kHospitalBedroom, 17, 19, 340}, // Use the head with the empty bed.
	{kHospitalBedroom, 18, 12, 342}, // Use the flammable spray at the sprinkler.
};

const byte kPeekHallwayFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 19, 19, 19, 19, 18, 17, 16, 15, 14, 20, 21, 22, 23, 24,
	24, 24, 24, 24, 24, 23, 22, 21, 20, 14, 13, 12, 11, 10, 9, 8,
	7, 6, 5, 4, 3, 2, 1, 0
};

// Retail chapter-one film scripts, from 0x43c450..0x4403cb.
const RunawaySequence::Command kGinaStory[] = {
	{RunawaySequence::kOpen, 9},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kPlay, 0, 28},
	{RunawaySequence::kTalk, 83, 0, 1, 0},
	{RunawaySequence::kTalk, 83, 1, 3, 0},
	{RunawaySequence::kTalk, 83, 2, 1, 0},
	{RunawaySequence::kTalk, 83, 3, 3, 0},
	{RunawaySequence::kTalk, 83, 4, 1, 0},
	{RunawaySequence::kTalk, 83, 5, 5, 0},
	{RunawaySequence::kSpeech, 83, 6, 0},
	{RunawaySequence::kPlay, 65, 105, RunawaySequence::kIdleDuringSpeech},
	{RunawaySequence::kMusic, 29, 255, 0},
	{RunawaySequence::kPlay, 106, 214},
	{RunawaySequence::kEffect, 25, 102, 1, 1},
	{RunawaySequence::kTalk, 83, 7, 7, 0},
	{RunawaySequence::kPlay, 222, 254},
	{RunawaySequence::kMusic, 25, 255, 1},
	{RunawaySequence::kTalk, 83, 8, 9, 0},
	{RunawaySequence::kTalk, 83, 9, 11, 0},
	{RunawaySequence::kTalk, 83, 10, 9, 0},
	{RunawaySequence::kTalk, 83, 11, 11, 0},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kPlay, 1608, 1632},
	{RunawaySequence::kSpeech, 83, 12, 0},
	{RunawaySequence::kPlay, 1633, 1687, RunawaySequence::kIdleDuringSpeech},
	{RunawaySequence::kTalk, 83, 13, 5, 0},
	{RunawaySequence::kMusic, 26, 255, 0},
	{RunawaySequence::kTalk, 83, 14, 5, 1},
	{RunawaySequence::kTalk, 83, 15, 5, 0},
	{RunawaySequence::kSpeech, 83, 16, 0},
	{RunawaySequence::kPlay, 1632, 1687, RunawaySequence::kIdleDuringSpeech},
	{RunawaySequence::kMusic, 27, 255, 1},
	{RunawaySequence::kPlay, 1688, 1713},
	{RunawaySequence::kTalk, 83, 17, 15, 0},
	{RunawaySequence::kPlay, 302, 324},
	{RunawaySequence::kTalk, 83, 18, 13, 0},
	{RunawaySequence::kEffect, 22, 255, 0, 0},
	{RunawaySequence::kPlay, 336, 350},
	{RunawaySequence::kSpeech, 83, 19, 22},
	{RunawaySequence::kPlay, 351, 990},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 83, 23, 15, 0},
	{RunawaySequence::kPlay, 1003, 1025},
	{RunawaySequence::kTalk, 83, 24, 17, 0},
	{RunawaySequence::kPlay, 1038, 1117},
	{RunawaySequence::kTalk, 83, 25, 19, 0},
	{RunawaySequence::kEffect, 23, 255, 0, 0},
	{RunawaySequence::kPlay, 1129, 1146},
	{RunawaySequence::kSpeech, 83, 26, 0},
	{RunawaySequence::kPlay, 1147, 1208},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 83, 27, 21, 0},
	{RunawaySequence::kTalk, 83, 28, 23, 0},
	{RunawaySequence::kEffect, 24, 255, 0, 0},
	{RunawaySequence::kPlay, 1232, 1270},
	{RunawaySequence::kSpeech, 83, 29, 0},
	{RunawaySequence::kPlay, 1271, 1300},
	{RunawaySequence::kSpeech, 83, 30, 0},
	{RunawaySequence::kMusic, 24, 255, 1},
	{RunawaySequence::kPlay, 1301, 1360},
	{RunawaySequence::kSpeech, 83, 31, 0},
	{RunawaySequence::kPlay, 1361, 1560},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kSpeech, 83, 32, 0},
	{RunawaySequence::kPlay, 1561, 1590},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 83, 33, 25, 0},
	{RunawaySequence::kTalk, 83, 34, 27, 0},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kLedgeCrossing[] = {
	{RunawaySequence::kOpen, 1},
	{RunawaySequence::kMusic, 28, 255, 1},
	{RunawaySequence::kEffect, 19, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 151},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kEffect, 21, 255, 1, 1},
	{RunawaySequence::kTalk, 70, 0, 1, 0},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kEffect, 20, 255, 0, 0},
	{RunawaySequence::kPlay, 163, 315},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kBuildDecoy[] = {
	{RunawaySequence::kOpen, 3},
	{RunawaySequence::kEffect, 26, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 75},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kHospitalAmbush[] = {
	{RunawaySequence::kOpen, 5},
	{RunawaySequence::kMusic, 21, 255, 0},
	{RunawaySequence::kEffect, 11, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 204},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kTalk, 81, 0, 0, 0},
	{RunawaySequence::kPlay, 204, 310},
	{RunawaySequence::kEffect, 12, 255, 0, 0},
	{RunawaySequence::kPlay, 311, 318},
	{RunawaySequence::kDelay, 2000},
	{RunawaySequence::kPlay, 318, 388},
	{RunawaySequence::kTalk, 81, 1, 0, 0},
	{RunawaySequence::kMusic, 22, 255, 0},
	{RunawaySequence::kPlay, 388, 412},
	{RunawaySequence::kEffect, 13, 255, 0, 0},
	{RunawaySequence::kPlay, 413, 496},
	{RunawaySequence::kSpeech, 81, 2, 0},
	{RunawaySequence::kPlay, 497, 531},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kEffect, 15, 255, 0, 0},
	{RunawaySequence::kWaitEffect, 3000},
	{RunawaySequence::kEffect, 14, 255, 0, 0},
	{RunawaySequence::kDelay, 1000},
	{RunawaySequence::kSpeech, 81, 3, 0},
	{RunawaySequence::kPlay, 531, 590},
	{RunawaySequence::kSpeech, 81, 4, 0},
	{RunawaySequence::kPlay, 591, 679},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kHospitalEscape[] = {
	{RunawaySequence::kOpen, 6},
	{RunawaySequence::kMusic, 23, 255, 0},
	{RunawaySequence::kEffect, 39, 204, 0, 0},
	{RunawaySequence::kPlay, 0, 118},
	{RunawaySequence::kSpeech, 82, 0, 0},
	{RunawaySequence::kPlay, 119, 160},
	{RunawaySequence::kSpeech, 82, 1, 0},
	{RunawaySequence::kPlay, 161, 210},
	{RunawaySequence::kSpeech, 82, 2, 0},
	{RunawaySequence::kPlay, 211, 248},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kMusic, 24, 255, 1},
	{RunawaySequence::kSpeech, 82, 3, 5},
	{RunawaySequence::kPlay, 249, 649, RunawaySequence::kLoopDuringSpeech},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kEffect, 40, 255, 0, 0},
	{RunawaySequence::kPlay, 650, 950},
	{RunawaySequence::kOpen, 2},
	{RunawaySequence::kMusic, 18, 255, 1},
	{RunawaySequence::kEffect, 30, 3, 1, 1},
	{RunawaySequence::kEffect, 16, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 100},
	{RunawaySequence::kSpeech, 80, 0, 0},
	{RunawaySequence::kSpeech, 80, 2, 0},
	{RunawaySequence::kPlay, 101, 400},
	{RunawaySequence::kEffect, 18, 51, 0, 0},
	{RunawaySequence::kPlay, 401, 719, RunawaySequence::kUntilSpeechEnds},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kPlay, 875, 932},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kTalk, 80, 3, 1, 0},
	{RunawaySequence::kPlay, 820, 863},
	{RunawaySequence::kTalk, 80, 4, 3, 0},
	{RunawaySequence::kTalk, 80, 5, 1, 0},
	{RunawaySequence::kTalk, 80, 6, 3, 0},
	{RunawaySequence::kTalk, 80, 7, 1, 0},
	{RunawaySequence::kEffect, 30, 3, 1, 1},
	{RunawaySequence::kSpeech, 80, 8, 11},
	{RunawaySequence::kPlay, 200, 719, RunawaySequence::kLoopDuringSpeech},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kTalk, 80, 12, 3, 0},
	{RunawaySequence::kTalk, 80, 13, 3, 1},
	{RunawaySequence::kEffect, 30, 3, 1, 1},
	{RunawaySequence::kEffect, 17, 255, 0, 0},
	{RunawaySequence::kSpeech, 80, 14, 0},
	{RunawaySequence::kPlay, 720, 819},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kOpen, 7},
	{RunawaySequence::kMusic, 31, 255, 1},
	{RunawaySequence::kPlay, 0, 36},
	{RunawaySequence::kTalk, 84, 1, 0, 0},
	{RunawaySequence::kTalk, 84, 2, 0, 0},
	{RunawaySequence::kTalk, 84, 3, 0, 0},
	{RunawaySequence::kTalk, 84, 4, 0, 0},
	{RunawaySequence::kTalk, 84, 5, 0, 0},
	{RunawaySequence::kTalk, 84, 6, 0, 0},
	{RunawaySequence::kTalk, 84, 7, 0, 0},
	{RunawaySequence::kEffect, 44, 153, 0, 0},
	{RunawaySequence::kPlay, 37, 40},
	{RunawaySequence::kSpeech, 84, 8, 0},
	{RunawaySequence::kPlay, 41, 236},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 84, 9, 1, 0},
	{RunawaySequence::kTalk, 84, 10, 3, 0},
	{RunawaySequence::kTalk, 84, 11, 5, 0},
	{RunawaySequence::kTalk, 84, 12, 3, 0},
	{RunawaySequence::kTalk, 84, 13, 5, 0},
	{RunawaySequence::kTalk, 84, 14, 3, 0},
	{RunawaySequence::kTalk, 84, 15, 3, 1},
	{RunawaySequence::kTalk, 84, 16, 5, 0},
	{RunawaySequence::kPlay, 272, 284},
	{RunawaySequence::kSpeech, 84, 17, 0},
	{RunawaySequence::kPlay, 285, 371},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kDelay, 1000},
	{RunawaySequence::kEnd},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene2020 = {
	kHospitalBedroom, 1520, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {60, 60, 80}, {80, 80, 100}},
	Common::Point(1075, 534),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene2020::Scene2020(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene2020::loadCustomResources() {
	return loadForeground(0, 12) && loadForeground(1, 14) && loadForeground(2, 11, true) &&
		loadForeground(3, 15) && _chartAnimation.load(_archive, 5, 1) &&
		_ginaAnimation.load(_archive, 22, 19) && _chartOverlay.load(_archive, 17, 1) &&
		_archive.readVariableChunk(20, _monitorPatches[0]) && _archive.readVariableChunk(21, _monitorPatches[1]);
}

byte Scene2020::defaultItemAction(byte item, byte id) const {
	if (!item || item > ARRAYSIZE(kItemResponses) || !id || id > ARRAYSIZE(kItemResponses[0])) {
		return 1;
	}
	return kItemResponses[item - 1][id - 1];
}

bool Scene2020::applyCustomBackgroundPatches() {
	if ((_hospital.glassTaken && !applyPatch(8)) || (_hospital.pillsTaken && !applyPatch(9)) ||
			(_hospital.sheetTaken && !applyPatch(7)) || (_hospital.bagTaken && !applyPatch(6))) {
		return false;
	}
	return (!_hospital.decoy || applyPatch(13)) && applyPatch(11) && applyRunawayPatch(_monitorPatches[1], _background) &&
		applyRunawayPatch(_monitorPatches[0], _background);
}

void Scene2020::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		// Zones 5/6 are reserved for scripted movement, not normal walking.
		if (_maps[i] > 4) {
			_maps[i] = 0;
		}
		byte &id = _maps[256 + i];
		if ((_hospital.glassTaken && id == 3) || (_hospital.pillsTaken && id == 4) ||
				(_hospital.bagTaken && id == 7) || (id == 11 && !_hospital.decoy)) {
			id = 0;
		} else if (_hospital.decoy && (id == 20 || id == 21)) {
			id = 11;
		} else if (id == 5 || id == 21) {
			id = _hospital.sheetTaken ? 19 : 5;
		} else if (id == 20) {
			id = 19;
		}
	}
}

RunawayRoomScene::ActionResult Scene2020::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Look at Gina.
		say(2, _random.getRandomNumber(2) + (_hospital.attacked ? 3 : 0));
		return kHandled;
	case 303: // Talk to Gina.
		say(3, _random.getRandomBit() + (_hospital.attacked ? 2 : 0));
		return kHandled;
	case 305: // Take the glass.
		_hospital.glassTaken = 1;
		_inventory.add(13);
		say(62, 0, true);
		return kBackgroundChanged;
	case 307: // Take the sleeping pills.
		_hospital.pillsTaken = 1;
		_inventory.add(8);
		say(57, 0, true);
		return kBackgroundChanged;
	case 309: // Take the sheet.
		_hospital.sheetTaken = 1;
		_inventory.add(7);
		say(18, _random.getRandomNumber(2), true);
		return kBackgroundChanged;
	case 313: // Take Gina's bag.
		_hospital.bagTaken = 1;
		_inventory.add(5);
		say(9, 1);
		return kBackgroundChanged;
	case 330: // Read the emergency exit map.
		if (!_hospital.exitMapRead) {
			say(26);
		}
		say(26, _hospital.window ? 2 : 1);
		_hospital.exitMapRead = 1;
		return kHandled;
	case 332: // Climb through the window.
		if (!_hospital.exitMapRead) {
			say(28);
			return kHandled;
		}
		if (!_hospital.window) {
			say(28, 1);
		}
		_hospital.window = MIN<int>(2, _hospital.window + 1);
		beginRoomAnimation(action, 4, 42);
		return kHandled;
	case 314: // Look out of the window.
		say(10, _hospital.window >= 2);
		return kHandled;
	case 315: // Look at Gina's medical chart.
	case 316: // Take Gina's medical chart.
		say(action == 315 ? 11 : 12, _hospital.chartReplaced);
		return kHandled;
	case 317: // Look at the other medical chart.
	case 318: // Take the other medical chart.
		say(action == 317 ? 13 : 14, _hospital.chartMoved);
		return kHandled;
	case 319: // Look at the fake patient.
		say(15, _hospital.attacked);
		return kHandled;
	case 324: // Press a nurse buzzer.
		say(20, _hospital.attacked);
		return kHandled;
	case 334: // Put Gina's chart back on her bed.
		say(30);
		return kHandled;
	case 335: // Put Gina's chart on the decoy's bed.
		if (!_hospital.decoy) {
			say(31);
			return kHandled;
		}
		beginRoomAnimation(action, 16, 29);
		return kHandled;
	case 336: // Use the wig with Gina.
		say(32);
		return kHandled;
	case 337: // Use the syringe with Gina.
		if (_hospital.attacked) {
			say(33);
		} else {
			say(4, 0, true);
		}
		return kHandled;
	case 338: // Put the blank medical chart on the other bed.
		if (_inventory.chartWritten()) {
			say(14, _random.getRandomNumber(2), true);
		} else {
			say(34);
		}
		return kHandled;
	case 339: { // Replace Gina's medical chart.
		if (!_inventory.chartWritten()) {
			say(34);
			return kHandled;
		}
		beginRoomAnimation(action, 19, 35);
		_actionFrames.push_back(33);
		for (int frame = 32; frame >= 0; --frame) {
			_actionFrames.push_back(frame);
		}
		return kHandled;
	}
	case 340: // Use the head with the empty bed.
	case 341: // Use the pillows with the empty bed.
		if (!_inventory.contains(17)) {
			say(36, 5);
		} else if (!_inventory.headWig()) {
			say(36, action == 340 ? 2 : 6);
		} else if (!_inventory.contains(14)) {
			say(36);
		} else if (!_inventory.contains(7)) {
			say(36, 1);
		} else {
			say(36, 3);
			_roomAction = 340;
			_actionStage = 0;
		}
		return kHandled;
	case 342: // Use the flammable spray at the sprinkler.
		if (!_hospital.attacked || !_hospital.medicineRead) {
			if (_inventory.contains(9)) {
				say(37, 1);
			} else {
				say(1, 2, true);
			}
		} else if (!_inventory.contains(9)) {
			say(37);
		} else {
			say(37, 2);
			_roomAction = action;
		}
		return kHandled;
	case 343: // Use the matches at the sprinkler.
		say(_hospital.attacked && _hospital.medicineRead ? 38 : 37, _hospital.attacked && _hospital.medicineRead ? 0 : 1);
		return kHandled;
	case 346: // Go to the bathroom.
		leaveHospitalRoom(kHospitalBathroom, Common::Point(0, 596), 1);
		return kHandled;
	case 349: // Use the glass with Gina.
		if (_hospital.attacked) {
			say(41, _inventory.water() != 0);
		} else {
			say(14, _random.getRandomNumber(2), true);
		}
		return kHandled;
	case 351: // Peek through the hallway door.
		walkTo(Common::Point(313, 532), 5, true);
		_roomAction = action;
		_actionStage = 0;
		return kHandled;
	case 352: // Try to leave through the hallway door.
		say(42, _hospital.attacked ? 2 : _random.getRandomBit());
		return kHandled;
	default:
		return kUnhandled;
	}
}

float Scene2020::actorHeight() const {
	// The original switches perspective planes at x = 864 (0x443be0).
	return _position.x < 864 ? _position.y * 1.26388884f - 302.069458f :
		_position.y * 0.44444445f + 188.777771f;
}

void Scene2020::start(uint32 now) {
	_now = _lastUpdate = now;
	_idleHead.reset(now);
	for (byte item = 1; item <= 4; ++item) {
		_inventory.add(item);
	}
	_inventory.takeNewItem();
	_facing = 7;
	startHospitalFilm(kGinaStoryFilm);
	resume();
}

bool Scene2020::busy() const {
	return RunawayRoomScene::busy() || _entryStep;
}

void Scene2020::setCameraX(int x) {
	_camera.x = CLIP<int>(x, 156, MAX<int>(0, _background.w - kRunawayScreenWidth));
}

byte Scene2020::initialActorFacing() const {
	return 7;
}

void Scene2020::resumeAmbientAudio(bool entering) {
	RunawayRoomScene::resumeAmbientAudio(entering);
	_audio.playEffect(30, 3, 1, true);
}

void Scene2020::runCustomEntrySequence() {
	if (_inventory.water() == 1) {
		_inventory.setWater(2);
	}
	if (_entryScene == 2022) {
		static const byte frames[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
		beginRoomAnimation(_entryScene, 23, 13, frames, ARRAYSIZE(frames));
	} else if (_entryScene == 2023) {
		startHospitalFilm(kAmbushFilm);
	} else {
		_audio.playEffect(48);
	}
}

void Scene2020::advanceCustomGameplayLoop(uint32 delta) {
	if (_entryStep && !speaking() && !_walkPath.active()) {
		switch (_entryStep++) {
		case 1: // Reflect on Gina's story.
			_facing = 4;
			say(0, 1);
			break;
		case 2: // Approach Gina.
			walkTo(Common::Point(1075, 599));
			break;
		case 3: // Decide to protect Gina.
			say(0, 2);
			break;
		case 4: // Finish the hospital introduction.
			_facing = 3;
			say(0, 3);
			_entryStep = 0;
			break;
		case 5: // React to the attack on the decoy.
			_facing = 4;
			say(81, 6);
			_entryStep = 0;
			break;
		case 6: // Inspect the completed decoy.
			say(36, 4);
			_entryStep = 0;
			break;
		case 7: // Report what Brian saw in the hallway.
			say(42, _hospital.attacked ? 5 : 4);
			_entryStep = 0;
			break;
		default:
			_entryStep = 0;
			break;
		}
	}
	if (!speaking() && !_walkPath.active()) {
		if (_roomAction == 340) {
			if (!_actionStage) {
				walkTo(Common::Point(531, 416), 3, true);
				_actionStage = 1;
			} else {
				startHospitalFilm(kDecoyFilm);
			}
		} else if (_roomAction == 342) {
			startHospitalFilm(kEscapeFilm);
		} else if (_roomAction == 351 && _actionFrames.empty()) {
			if (!_actionStage) {
				say(42, 3);
				_actionStage = 1;
			} else if (_actionStage == 1) {
				walkTo(Common::Point(40, 599), 5, true);
				_actionStage = 2;
			} else {
				beginRoomAnimation(351, 10, 25, kPeekHallwayFrames, ARRAYSIZE(kPeekHallwayFrames));
			}
		}
	}
	if (_now - _ginaTick >= 80) {
		_ginaTick = _now;
		if ((_ginaFrame > 0 && _ginaFrame < 18) || !_random.getRandomNumber(49)) {
			_ginaFrame += _ginaDirection;
			if (_ginaFrame == 18 || _ginaFrame == 0) {
				_ginaDirection = -_ginaDirection;
			}
		}
	}
	// The bedside indicator alternates after 17 and 9 animation ticks.
	applyRunawayPatch(_monitorPatches[(_now / 80) % 26 >= 17], _background);
}

void Scene2020::adaptGogFilm(const RunawaySequence::Command *commands) {
	typedef RunawaySequence S;
	_gogFilmCommands.clear();
	uint movie = 0, part = 1;
	for (uint i = 0; commands[i].operation != S::kEnd; ++i) {
		S::Command command = commands[i];
		if (command.operation == S::kOpen) {
			movie = command.a;
			part = 1;
		}
		if (movie == 2 && command.operation == S::kPlay) {
			if (command.a >= 820) {
				continue; // These CD shots are separate stills in the GOG edition.
			}
			uint targetPart = command.a >= 720 ? 4 : command.a >= 200 ? 3 : 1;
			if (command.a == 101) {
				_gogFilmCommands.push_back(S::Command(S::kPlay, 101, 199));
				targetPart = 3;
				command.a = 200;
			}
			if (part != targetPart) {
				_gogFilmCommands.push_back(S::Command(S::kOpen, movie, targetPart));
				part = targetPart;
			}
			const uint offset = part == 4 ? 720 : part == 3 ? 200 : 0;
			command.a -= offset;
			command.b -= offset;
		} else if (movie == 2 && command.operation == S::kTalk && command.c) {
			_gogFilmCommands.push_back(S::Command(S::kStill, command.c == 1 ? 0 : 1, 1));
			command.c = command.c == 1 ? 3 : 1;
		} else if (movie == 7 && command.operation == S::kTalk && command.b >= 9) {
			static const uint16 stills[] = {236, 237, 238, 239, 240, 241, 241, 242};
			_gogFilmCommands.push_back(S::Command(S::kStill, stills[command.b - 9]));
		} else if (movie == 9 && command.operation == S::kPlay) {
			uint targetPart = 1;
			if (command.a == 1608) {
				targetPart = 2;
				command.a = 0;
				command.b = 23;
			} else if (command.a == 1632 || command.a == 1633) {
				targetPart = 3;
				command.a = 0;
				command.b = 54;
			} else if (command.a == 1688) {
				targetPart = 4;
				command.a = 0;
				command.b = 24;
			} else if (command.a == 65) {
				command.a = 66;
			} else if (command.a == 106) {
				command.b = 215;
			} else if (command.a >= 222 && command.a <= 1147) {
				++command.a;
				++command.b;
			}
			if (part != targetPart) {
				_gogFilmCommands.push_back(S::Command(S::kOpen, movie, targetPart));
				part = targetPart;
			}
		} else if (movie == 9 && command.operation == S::kTalk) {
			static const uint16 stills[] = {
				29, 42, 43, 44, 45, 53, 0, 216, 255, 257, 258, 259, 0, 260, 260, 260,
				0, 302, 325, 0, 0, 0, 0, 991, 1026, 1118, 0, 1209, 1222, 0, 0, 0, 0, 1591, 1598
			};
			if (part != 1) {
				_gogFilmCommands.push_back(S::Command(S::kOpen, movie));
				part = 1;
			}
			_gogFilmCommands.push_back(S::Command(S::kStill, stills[command.b]));
		}
		_gogFilmCommands.push_back(command);
	}
	_gogFilmCommands.push_back(S::Command(S::kEnd));
}

void Scene2020::startHospitalFilm(HospitalFilm film) {
	const RunawaySequence::Command *commands = nullptr;
	switch (film) {
	case kGinaStoryFilm:
		commands = kGinaStory;
		break;
	case kLedgeFilm:
		commands = kLedgeCrossing;
		break;
	case kDecoyFilm:
		commands = kBuildDecoy;
		break;
	case kAmbushFilm:
		commands = kHospitalAmbush;
		break;
	case kEscapeFilm:
		commands = kHospitalEscape;
		break;
	}
	_hospitalFilm = film;
	_roomAction = 0;
	_actionFrames.clear();
	_caption.clear();
	if (Common::File::exists("DATAVC00.000")) {
		adaptGogFilm(commands);
		commands = _gogFilmCommands.data();
	}
	_sequence.start(commands, _text, _now);
}

void Scene2020::finishCustomFilm() {
	switch (_hospitalFilm) {
	case kGinaStoryFilm:
		_entryStep = 1;
		say(0);
		break;
	case kLedgeFilm:
		_transition.scene = kHospitalStoreroom;
		_transition.position = Common::Point(419, 546);
		_transition.facing = 3;
		break;
	case kDecoyFilm:
		_hospital.decoy = 1;
		_inventory.remove(7);
		_inventory.remove(14);
		_inventory.remove(17);
		if (!rebuildBackground()) {
			error("Cannot build Runaway hospital decoy");
		}
		updateMaps();
		walkTo(Common::Point(404, 456), 1, true);
		_entryStep = 6;
		break;
	case kAmbushFilm:
		_entryStep = 5;
		say(81, 5);
		break;
	case kEscapeFilm:
		_chapterComplete = true;
		_audio.stopMusic();
		return;
	}
	_audio.playMusic(11, false);
	_audio.playEffect(30, 3, 1, true);
}

void Scene2020::finishCustomAnimation(uint16 action) {
	switch (action) {
	case 332: // Start the ledge crossing.
		startHospitalFilm(kLedgeFilm);
		return;
	case 2022: // Finish climbing into Gina's room.
		_facing = 3;
		return;
	case 335: // Finish moving Gina's chart to the decoy's bed.
		_hospital.chartMoved = 1;
		_inventory.remove(6);
		say(31, 1);
		break;
	case 339: // Finish replacing Gina's chart.
		_hospital.chartReplaced = 1;
		_inventory.remove(16);
		_inventory.add(6);
		say(35);
		break;
	case 351: // Return from peeking through the hallway door.
		walkTo(Common::Point(313, 532), 4, true);
		_entryStep = 7;
		return;
	default:
		return;
	}
	if (!rebuildBackground()) {
		error("Cannot update Runaway hospital chart");
	}
	updateMaps();
}

void Scene2020::handleAnimationFrameHook() {
	if ((_roomAction == 335 && _actionStep == 15) || (_roomAction == 339 && _actionStep == 47)) {
		_audio.playEffect(46);
	}
	if (_roomAction == 339 && _actionStep == 16) {
		_audio.playEffect(45);
	}
	if (_roomAction == 339 && _actionStep == 1 && !applyPatch(18)) {
		error("Cannot clear Runaway hospital chart");
	}
	if (_roomAction == 332 && (_actionStep == 21 || _actionStep == 35 || _actionStep == 41)) {
		const byte volume = CLIP<int>((int)(actorHeight() * 255 / 455), 0, 255);
		_audio.playFootstep(1, _random.getRandomBit(), volume, _position.x);
	}
}

Common::U32String Scene2020::hotspotName(byte id) const {
	if (id == 9 && _hospital.chartReplaced) {
		return _strings.text(RunawayStrings::kFakeChart);
	}
	if (id == 10 && _hospital.chartMoved) {
		return _strings.itemName(1, 6);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene2020::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawRoomComposite(viewport, false);
}

void Scene2020::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	drawRoomComposite(viewport, true);
}

void Scene2020::drawRoomComposite(Graphics::ManagedSurface &viewport, bool foreground) {
	if (!foreground && _hospital.chartMoved) {
		_chartAnimation.draw(viewport, 0, _camera.x, 0);
	}
	// Gina and the near bed occlude Brian only when he is behind them.
	if (foreground == (_position.y < 503)) {
		_ginaAnimation.draw(viewport, _ginaFrame, _camera.x, 0);
		drawForeground(viewport, 2);
		if (_roomAction != 339 || !_actionStep) {
			_chartOverlay.draw(viewport, 0, _camera.x, 0);
		}
	}
	if (foreground) {
		if (_position.x < 600 && _position.y < 434) {
			drawForeground(viewport, _hospital.decoy ? 3 : 1);
		}
		if (_position.x > 1000) {
			drawForeground(viewport, 0);
		}
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
