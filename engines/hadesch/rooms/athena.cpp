/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "hadesch/ambient.h"
#include "common/translation.h"

namespace Hadesch {
static const char *kAthenaAnim = "c8060ba0";
static const char *kLights = "c8110bb0";

// Keep in order
enum {
	kWonPuzzle = -2,
	kNowhere = -1,
	kBubo = 0,
	kArtist = 1,
	kOrator = 2,
	kScholar = 3,
	kWarrior = 4
};
static const int kNumPuzzleElements = 5;
static const struct {
	const char *hotname;
	int lights[2];
	int rays[2];
} puzzleElements[kNumPuzzleElements] = {
	{"Bubo",  { kArtist, kOrator}, {2, 1}},
	{"Artist", { kNowhere, kOrator}, {6, 5}},
	{"Orator", { kArtist, kScholar}, {3, 4}},
	{"Scholar", { kNowhere, kWarrior}, {8, 9}},
	{"Warrior", { kScholar, kWonPuzzle}, {7, 10}}
};

enum {
	kBackgroundZ = 10000,
	kLightsZ = 201
};

enum {
	kPhilForgettingEnd = 23009,
	kPhilHardwareFinished = 23012,
	kIntroFinished = 1023001
};

class AthenaHandler : public Handler {
public:
	AthenaHandler() {
		_playAreYouForgetting = true;
		_playAthenaTempleHardware = true;
		_isPuzzleWon = false;
		_hintTimerLength = 20000;
		memset(_puzzleState, 0, sizeof (_puzzleState));
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		if (name == "Seriphos") {
			if (quest == kMedusaQuest) {
				if (persistent->_athenaPuzzleSolved &&
				    (!persistent->_athenaSwordTaken
				     || !persistent->_athenaShieldTaken)
				    && persistent->_hintsAreEnabled
				    && _playAreYouForgetting) {
					_playAreYouForgetting = false;
					room->disableMouse();
					room->playVideo("c8020ba0", 0,
							kPhilForgettingEnd,
							Common::Point(0, 216));
					return;
				} else if (persistent->_athenaPuzzleSolved &&
					   persistent->_athenaSwordTaken &&
					   persistent->_athenaShieldTaken &&
					   !persistent->_athenaPlayedPainAndPanic
					) {
					persistent->_athenaPlayedPainAndPanic = true;
					room->disableMouse();
					room->fadeOut(1000, 23019);
					return;
				} else if (persistent->_athenaPuzzleSolved &&
					   !_playAthenaTempleHardware
					   && persistent->_hintsAreEnabled) {
					_playAthenaTempleHardware = false;
					room->disableMouse();
					room->playVideo("c8160ba0", 0,
							kPhilHardwareFinished,
							Common::Point(0, 216));
					return;
				}
			}
			g_vm->moveToRoom(kSeriphosRoom);
			return;
		}

		if (name == "Athena") {
			Common::Array<Common::String> videos;
			if (quest == kMedusaQuest && !persistent->_athenaPuzzleSolved) {
				videos.push_back(persistent->_gender == kMale ? "c8060wa0" : "c8060wb0");
			} else {
				videos.push_back("c8060wc0");
				videos.push_back("c8060wd0");
			}

			room->playStatueSMK(kAthenaStatue,
					    kAthenaAnim,
					    1101,
					    videos, 26, 42);
			return;
		}

		for (unsigned i = 0; i < kNumPuzzleElements; i++)
			if (name == puzzleElements[i].hotname) {
				handlePuzzleClick(i);
				return;
			}

		if (name == "Sword") {
			persistent->_athenaSwordTaken = true;
			g_vm->getHeroBelt()->placeToInventory(kSword);
			room->stopAnim("c8130bf0");
			room->disableHotzone("Sword");
			room->disableMouse();
			room->playSpeech(TranscribedSound::make(
					     "c8140wa0",
					     "The magic sword will never leave Perseus' hand, "
					     "so he can successfully cut off Medusa's head"),
					 23026);
			return;
		}

		if (name == "Shield") {
			persistent->_athenaShieldTaken = true;
			g_vm->getHeroBelt()->placeToInventory(kShield);
			room->stopAnim("c8130be0");
			room->disableHotzone("Shield");
			room->disableMouse();
			room->playSpeech(TranscribedSound::make(
					     "c8150wa0",
					     "Medusa can only turn Perseus to stone if he looks directly at her. "
					     "He'll use this shield to block her gaze"),
					 23027);
			return;
		}

		if (name == "Athena's Sword") {
			room->disableMouse();
			room->playAnimLoop("c8010oc0", 2101);
			room->playVideo("c8080wa0", 0, 23043);
			room->playSFX("C8080eA1");
			return;
		}

		if (name == "Athena's Shield") {
			room->disableMouse();
			room->playAnimLoop("c8010ob0", 2101);
			room->playVideo("c8070wa0", 0, 23044);
			room->playSFX("C8080eA1");
			return;
		}
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		switch(eventId) {
		case kIntroFinished:
			room->stopAnim(kAthenaAnim);
			room->playAnim("c8110ba0", 0, PlayAnimParams::disappear(), 23035);
			room->enableMouse();
			_hintTimerLength = 20000;
			rescheduleHintTimer();
			break;
		case kPhilForgettingEnd:
		case kPhilHardwareFinished:
		case 23026:
		case 23027:
			room->enableMouse();
			break;
		case 23007:
			handleEvent(23010);
			_hintTimerLength = 40000;
			rescheduleHintTimer(true);
			break;
		case 23008:
			room->playAnim("c8140ba0", 1101, PlayAnimParams::disappear(), 23015);
			room->playAnim("c8150ba0", 1101, PlayAnimParams::disappear(), 23016);
			room->playMusic("c8130ma0", 23020);
			room->playSFX("c8130eb0");
			room->playSFX("c8130ec0");
			break;
		case 23010:
			if (persistent->_hintsAreEnabled && room->isMouseEnabled()) {
				room->disableMouse();
				room->playVideo("c8170ba0", 0, 23011, Common::Point(0, 216));
			}
			break;
		case 23011:
			room->enableMouse();
			break;
		case 23015:
			room->playAnimKeepLastFrame("c8130bf0", 1101, 23017);
			room->playSFX("c8130ef0");
			break;
		case 23016:
			room->playAnimKeepLastFrame("c8130be0", 1101, 23018);
			room->playSFX("c8130ee0");
			break;
		case 23017:
			room->enableHotzone("Sword");
			room->enableMouse();
			break;
		case 23018:
			room->enableHotzone("Shield");
			room->enableMouse();
			break;
		case 23019:
			room->resetFade();
			room->disableHeroBelt();
			room->resetLayers();
			room->addStaticLayer("c8180pa0", 9000);
			room->playSFX("g0261ma0", 23031);
			room->playSpeech(persistent->_gender == kMale
					? TranscribedSound::make(
					    "c8180wa0",
					    "Oh no. Why did I shine that light in Athena's temple. "
					    "I was just trying to see what the hero was doing")
					 : TranscribedSound::make(
					     "c8180wb0",
					     "Oh no. Why did I shine that light in Athena's temple. "
					     "I was just trying to see what the heroine was doing"),
					 23029);
			break;
		case 23029:
			room->playSpeech(TranscribedSound::make("c8180wc0", "Well now you can see what I'm doing: tomato heads"), 23030);
			break;
		case 23030:
			room->playVideo("c8180ba0", 0, 23032);
			break;
		case 23031:
			break;
		case 23032:
			room->selectFrame("c8180bb0", 101, 0);
			g_vm->moveToRoom(kSeriphosRoom);
			break;
		// TODO: lighting up of the beam: 23035/23036 are for lighting up
		case 23035:
		case 23036:
			room->selectFrame(LayerId(kLights, 0, "source"), kLightsZ, 0);
			/* Fallthrough */
		case 23037:
			room->playAnimLoop("c8110bc0", 211);
			room->enableHotzone("Athena's Sword");
			room->enableHotzone("Athena's Shield");
			for (unsigned i = 0; i < kNumPuzzleElements; i++)
				room->enableHotzone(puzzleElements[i].hotname);
			break;
		case 23020:
			room->playAnim("c8130bd0", 0, PlayAnimParams::disappear(), 23041);
			/*Fallthrough */
		case 23038:
			g_vm->addTimer(23040, 640);
			g_vm->addTimer(23039, 40, 15);
			break;
		case 23039:
			//TODO progressive beams
			break;
		case 23040:
			for (unsigned i = 0; i < 12; i++)
				room->stopAnim(LayerId(kLights, i, "internal"));
			room->stopAnim(LayerId(kLights, 0, "source"));
			room->stopAnim("c8110bc0");
			break;
		case 23043:
			room->stopAnim("c8010oc0");
			room->enableMouse();
			break;
		case 23044:
			room->stopAnim("c8010ob0");
			room->enableMouse();
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		room->loadHotZones("Athena.HOT", false);
		room->addStaticLayer("c8010pa0", kBackgroundZ);
		room->addStaticLayer("c8010ta0", 601);
		room->enableHotzone("Athena");
		room->enableHotzone("Seriphos");

		if (quest == kMedusaQuest && !persistent->_athenaPuzzleSolved) {
			persistent->_athenaIntroPlayed = true;
			room->disableMouse();
			room->playVideo(persistent->_gender == kMale ? "c8040wa0" : "c8040wb0",
					1101, kIntroFinished);
			room->playAnim(kAthenaAnim, 1101, PlayAnimParams::loop());
			room->playMusic("c8040ma0", 23013);
		}

		if (!persistent->_athenaShieldTaken) {
			if (persistent->_athenaPuzzleSolved) {
				room->selectFrame("c8130be0", 1101, 4);
				room->enableHotzone("Shield");
			} else {
				room->selectFrame("c8150ba0", 1101, 0);
			}
		}

		if (!persistent->_athenaSwordTaken) {
			if (persistent->_athenaPuzzleSolved) {
				room->selectFrame("c8130bf0", 1101, 7);
				room->enableHotzone("Sword");
			} else {
				room->selectFrame("c8140ba0", 1101, 0);
			}
		}

		room->playAnimLoop("c8030ba0", 201);
		g_vm->getHeroBelt()->setColour(HeroBelt::kCool);
	}
private:
	void rescheduleHintTimer(bool isInHandler = false) {
		Persistent *persistent = g_vm->getPersistent();

		if (!isInHandler)
			g_vm->cancelTimer(23007);
		if (!persistent->_athenaPuzzleSolved)
			g_vm->addTimer(23007, _hintTimerLength);
	}

	void handlePuzzleClick(int num) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		rescheduleHintTimer();

		_puzzleState[num] = (_puzzleState[num] + 1) % 3;
		memset(_isPuzzleLit, 0, sizeof (_isPuzzleLit));
		_isPuzzleLit[0] = true;
		bool done = false;
		while (!done) {
			done = true;
			for (unsigned i = 0; i < kNumPuzzleElements; i++)
				if (_puzzleState[i] != 0 && _isPuzzleLit[i]
				    && puzzleElements[i].lights[_puzzleState[i] - 1] >= 0
				    && !_isPuzzleLit[puzzleElements[i].lights[_puzzleState[i] - 1]]) {
					_isPuzzleLit[puzzleElements[i].lights[_puzzleState[i] - 1]] = true;
					done = false;
				}
		}
		for (unsigned i = 0; i < kNumPuzzleElements; i++)
			if (_puzzleState[i] != 0 && _isPuzzleLit[i]
			    && puzzleElements[i].lights[_puzzleState[i] - 1] == kWonPuzzle) {
				_isPuzzleWon = true;
				break;
			}
		for (unsigned i = 0; i < kNumPuzzleElements; i++)
			if (!_isPuzzleLit[i])
				_puzzleState[i] = 0;
		for (unsigned i = 0; i < 11; i++)
			room->stopAnim(LayerId(kLights, i, "internal"));
		for (unsigned i = 0; i < kNumPuzzleElements; i++)
			if (_puzzleState[i] != 0 && _isPuzzleLit[i]) {
				int ray = puzzleElements[i].rays[_puzzleState[i] - 1];
				if (ray <= 0)
					continue;
				room->selectFrame(LayerId(kLights, ray, "internal"), kLightsZ, ray);
			}

		if (_isPuzzleLit[kArtist] && _puzzleState[kArtist] == 1)
			room->playAnimLoop("c8120bg0", 601);
		else
			room->stopAnim("c8120bg0");
		if (_isPuzzleLit[kScholar] && _puzzleState[kScholar] == 1)
			room->playAnimLoop("c8120bg1", 601);
		else
			room->stopAnim("c8120bg1");

		if (_isPuzzleLit[num] && _puzzleState[num])
			room->playSFX("c8120ea0");

		if (_isPuzzleWon) {
			room->selectFrame(LayerId(kLights, 11, "internal"), kLightsZ, 11);
			room->playSFX("C8130eA0");
			g_vm->addTimer(23008, 1000);
			room->disableHotzone("Athena's Sword");
			room->disableHotzone("Athena's Shield");
			for (unsigned i = 0; i < kNumPuzzleElements; i++)
				room->disableHotzone(puzzleElements[i].hotname);
			room->disableMouse();
			persistent->_athenaPuzzleSolved = true;
		}

		room->playMusicLoop(
			persistent->_quest != kMedusaQuest || persistent->_athenaPuzzleSolved
			? "c8010ea0" : "c8110ea0");
	}
	bool _playAreYouForgetting;
	bool _playAthenaTempleHardware;
	bool _isPuzzleLit[kNumPuzzleElements];
	int _puzzleState[kNumPuzzleElements];
	bool _isPuzzleWon;
	int _hintTimerLength;
};

Common::SharedPtr<Hadesch::Handler> makeAthenaHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new AthenaHandler());
}

}
