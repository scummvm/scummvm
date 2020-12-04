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

namespace Hadesch {

static const char *kBacchusHighlight = "AnimBacchusStatue";
static const char *kGuardLooking = "AnimGuardLooking";
static const char *kAnimMinosEating = "AnimMinosEating";
static const char *kStatues = "AnimStatueZeroPose";

enum {
	kMinosBackToIdleEvent = 14003,
	// 14005 is the end of statue animation that we handle as functor instead
	kInstructionMovieCompleted = 14011,
	kMinosHornedStatue = 14012,
	kMinosOtherItem = 14013,
	kMinosToss1 = 14014,
	kMinosToss2 = 14015,
	kMinosToss3 = 14016,
	kMinosToss4 = 14017,
	kMinosStatueTossed = 14018,
	kGuardGruntCleanup = 14020,
	kGuardNagCleanup = 14021,
	kGuardNag = 14022,
	kGuardPeriodic = 1014001,
	kMinosPeriodic = 1014002
};

enum {
	kBacchusZ = 200,
	kStatueOnTheTableZ = 300,
	kMinosZ = 500,
	kInstalledStatueZ = 500,
	kGuardZ = 600,
	kBackgroundZ = 10000
};

class MinosHandler : public Handler {
public:
	MinosHandler() {
		_guardIsBusy = false;
		_minosIsBusy = false;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (name == "Bacchus") {
			Common::Array<Common::String> videos;
			videos.push_back("SndBacchusStatueA");
			videos.push_back("SndBacchusStatueB");
			videos.push_back("SndBacchusStatueC");

			room->playStatueSMK(kBacchusStatue,
					    kBacchusHighlight,
					    kBacchusZ,
					    videos, 22, 39);
			return;
		}

		if (name == "Crete") {
			room->disableMouse();
			g_vm->moveToRoom(kCreteRoom);
			return;
		}

		if (name == "Daedalus") {
			room->disableMouse();
			g_vm->moveToRoom(kDaedalusRoom);
			return;
		}

		if (name == "Guard" && !_guardIsBusy) {
			_guardIsBusy = true;
			room->playAnimWithSFX("AnimGuardGrunt",
					      "SndGuardGrunt",
					      kGuardZ,
					      PlayAnimParams::keepLastFrame(),
					      kGuardGruntCleanup);
			room->stopAnim(kGuardLooking);
			return;
		}

		if (name == "Minos") {
			playMinosMovie("MovMinosBeGone", kMinosBackToIdleEvent, Common::Point(202, 229));
			return;
		}

		if (name == "Table") {
			playMinosMovie("MovMinosHavePiece", kMinosBackToIdleEvent, Common::Point(230, 227));
			return;
		}

		if (name == "Statue" && !_guardIsBusy) {
			_guardIsBusy = true;
			room->playVideo("MovGuardDontTouch", kGuardZ, 14004, Common::Point(432, 142));
			room->stopAnim(kGuardLooking);
			return;
		}
	}

	bool handleClickWithItem(const Common::String &name, InventoryItem item) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (name == "Minos") {
			if (item >= kHornlessStatue1 && item <= kHornedStatue) {
				playMinosMovie("MovMinosPutOnTable", kMinosBackToIdleEvent, Common::Point(218, 227));
				return true;
			}

			playMinosMovie("MovMinosBeGone", kMinosBackToIdleEvent, Common::Point(202, 229));
			return true;
		}

		if (name == "Table") {
			static const char *sounds[4] = {
				"R3160eA0",
				"R3160eB0",
				"R3160eC0",
				"R3160eD0",
			};
			room->playSFX(sounds[g_vm->getRnd().getRandomNumberRng(0, 3)]);
			if (item == kHornedStatue) {
				g_vm->getHeroBelt()->removeFromInventory(item);
//				room->selectFrame(kStatues, 0);
				playMinosMovie("MovMinosLoveTheHorns", kMinosHornedStatue, Common::Point(202, 178));
				persistent->_creteDaedalusRoomAvailable = true;
				return true;
			}
			if (item >= kHornlessStatue1 && item <= kHornedStatue) {
				g_vm->getHeroBelt()->removeFromInventory(item);
				static const int mapFrames[] = {
					2, 4, 1, 3
				};
				room->selectFrame(kStatues, kStatueOnTheTableZ, mapFrames[item - kHornlessStatue1]);
				playMinosMovie("MovMinosWhatTrash", kMinosToss1 + item - kHornlessStatue1, Common::Point(202, 225));
				persistent->_creteTriedHornless[item - kHornlessStatue1] = true;
				return true;
			}
			playMinosMovie("MovMinosWhatTrash", kMinosOtherItem, Common::Point(202, 225));
			return true;
		}
		return false;
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		switch (eventId) {
		case kMinosBackToIdleEvent:
			minosBackToIdle();
			break;
		case kMinosHornedStatue:
			room->setLayerEnabled(kStatues, false);
			persistent->_creteShowMerchant = false;
			persistent->_creteHadesPusnishesPainAndPanic = true;
			g_vm->moveToRoom(kCreteRoom);
			break;
		case kMinosOtherItem:
			playMinosMovie("MovMinosBeGone", kMinosBackToIdleEvent, Common::Point(202, 229));
			scheduleNagging();
			break;
		case kMinosToss1:
			room->setLayerEnabled(kStatues, false);
			playMinosMovie("MovMinosToss1", kMinosStatueTossed, Common::Point(0, 191));
			break;
		case kMinosToss2:
			room->setLayerEnabled(kStatues, false);
			playMinosMovie("MovMinosToss2", kMinosStatueTossed, Common::Point(0, 188));
			break;
		case kMinosToss3:
			room->setLayerEnabled(kStatues, false);
			playMinosMovie("MovMinosToss3", kMinosStatueTossed, Common::Point(0, 183));
			break;
		case kMinosToss4:
			room->setLayerEnabled(kStatues, false);
			playMinosMovie("MovMinosToss4", kMinosStatueTossed, Common::Point(0, 191));
			break;			
		case kMinosStatueTossed:
			playMinosMovie("MovMinosBeGone", kMinosBackToIdleEvent, Common::Point(202, 229));
			scheduleNagging();
			break;

		case kGuardGruntCleanup:
			_guardIsBusy = false;
			room->stopAnim("AnimGuardGrunt");
			room->selectFrame(kGuardLooking, kGuardZ, 0);
			break;
		case kGuardPeriodic:
			if (!_guardIsBusy)
				room->playAnimWithSFX(kGuardLooking,
						      "SndGuardLooking",
						      kGuardZ,
						      PlayAnimParams::keepLastFrame());
			g_vm->addTimer(kGuardPeriodic, g_vm->getRnd().getRandomNumberRng(5000, 10000));
			break;
		case kMinosPeriodic:
			if (!_minosIsBusy)
				room->playAnimWithSFX(kAnimMinosEating,
						      "SndMinosEating",
						      kMinosZ,
						      PlayAnimParams::keepLastFrame());
			g_vm->addTimer(kMinosPeriodic, g_vm->getRnd().getRandomNumberRng(5000, 10000));
			break;
		case kInstructionMovieCompleted:
			minosBackToIdle();
			break;

		case kGuardNagCleanup:
			room->selectFrame(kGuardLooking, kGuardZ, 0);
			_guardIsBusy = false;
			break;

		case kGuardNag:
			scheduleNagging();
			if (!_guardIsBusy && !_minosIsBusy) {
				_guardIsBusy = true;
				room->playVideo("MovGuardUMustGo", kGuardNagCleanup, kGuardZ, Common::Point(0, 142));
				room->stopAnim(kGuardLooking);
			}
			break;

		case 14004:
			_guardIsBusy = false;
			room->selectFrame(kGuardLooking, kGuardZ, 0);
			break;
		case 14006:
			_ambients.tick();
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		room->loadHotZones("Minos.HOT", false);
		room->addStaticLayer("Background", kBackgroundZ);
		room->playAnimLoop("AnimFountain", 250);
		room->enableHotzone("Bacchus");
		room->enableHotzone("Crete");
		g_vm->getHeroBelt()->setColour(HeroBelt::kWarm);

		if (persistent->_creteDaedalusRoomAvailable) {
			room->enableHotzone("Statue");
			room->enableHotzone("Daedalus");
		} else {
			room->enableHotzone("Minos");
			room->selectFrame(kAnimMinosEating, kMinosZ, 0);
			g_vm->addTimer(kMinosPeriodic, 10000);
		}

		for (int i = kHornlessStatue1; i <= kHornedStatue; i++)
			if (persistent->isInInventory((InventoryItem) i))
				room->enableHotzone("Table");

		room->enableHotzone("Guard");
		room->selectFrame(kGuardLooking, kGuardZ, 0);
		g_vm->addTimer(kGuardPeriodic, 5000);

		if (!persistent->_creteMinosInstructed) {
			playMinosMovie(
				"MovMinosInstructions", kInstructionMovieCompleted,
				Common::Point(210, 229));
			persistent->_creteMinosInstructed = true;
			persistent->_creteShowMerchant = true;
			persistent->_creteIntroMerchant = true;
			persistent->_creteShowHorned = true;
			persistent->_creteShowHornless1 = true;
			persistent->_creteShowHornless2 = true;
			persistent->_creteShowHornless3 = true;
			persistent->_creteShowHornless4 = true;
		}

		room->playMusic(persistent->isInInventory(kHornedStatue)
				? "HornedIntroMusic" : "NormalIntroMusic");

		if (persistent->_creteDaedalusRoomAvailable) {
			room->selectFrame(kStatues, kInstalledStatueZ, 0, Common::Point(37, -110));
		}

		TextTable miAmb = TextTable(
			Common::SharedPtr<Common::SeekableReadStream>(room->openFile("MiAmb.txt")), 6);
		_ambients.readTableFilePriamSFX(miAmb);
		g_vm->addTimer(14006, 100, -1);
		_ambients.firstFrame();
	}

private:
	void scheduleNagging() {
		g_vm->addTimer(kGuardNag, g_vm->getRnd().getRandomNumberRng(5000, 10000));
	}

	void playMinosMovie(const Common::String &name, int callback,
			    Common::Point offset) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->setLayerEnabled(kAnimMinosEating, false);
		room->playVideo(name, kMinosZ, callback, offset);
		room->disableMouse();
		_minosIsBusy = true;
	}

	void minosBackToIdle() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_minosIsBusy = false;
		room->enableMouse();
		room->selectFrame(kAnimMinosEating, kMinosZ, 0);
	}

	bool _guardIsBusy;
	bool _minosIsBusy;
  	AmbientAnimWeightedSet _ambients;
};

Common::SharedPtr<Hadesch::Handler> makeMinosHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new MinosHandler());
}

}
