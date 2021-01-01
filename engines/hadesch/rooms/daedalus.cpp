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

static const char *kDaedalusStillFrame = "daedalus still frame";
static const char *kDaedalusAmbient = "daedalus ambient";
static const char *kModelPiece = "model piece";
static const char *kLabyrinthWorkers = "labyrinth workers";

enum {
	kDaedalusTick = 13901,

	// TODO: Remove this once we have a possibility
	// to pass an argument to event handler.
	// Originally: "daedalus intro 1" -> 13002[1] -> "phil intro 1"
	// -> 13003[1] -> "daedalus intro 2" -> 13002[2]
	// -> "phil intro 2" -> 13003[2] -> "daedalus intro 3"
	kIntroStep1 = 1013001,
	kIntroStep2 = 1013002,
	kIntroStep3 = 1013003,
	kIntroStep4 = 1013004,
	kIntroStep5 = 1013005
};

enum {
	kBackgroundZ = 10000,
	kLabyrinthWorkersZ = 900,
	kDaedalusZ = 500,
	kModelPieceZ = 500,
	kPhilZ = 0
};

class DaedalusHandler : public Handler {
public:
	DaedalusHandler() {
		_daedalusIsBusy = false;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		if (name == "minos palace") {
			g_vm->moveToRoom(kMinosPalaceRoom);
			return;
		}

		if (name == "daedalus") {
			playDaedalusVideo("daedalus no materials", 13005, Common::Point(76, 0));
			return;
		}

		if (name == "wings") {
			playDaedalusVideo("daedalus wings", 4009, Common::Point(10, 56));
			return;
		}

		if (name == "labyrinth" && persistent->_quest != kCreteQuest) {
			room->disableMouse();
			room->playVideo("phil navigation help", 0, 13007, Common::Point(0, 216));
			return;
		}

		if (name == "brick wall") {
			daedalusWallMotion();
			return;
		}
	}

	bool handleClickWithItem(const Common::String &name, InventoryItem item) override {
		Persistent *persistent = g_vm->getPersistent();

		int labItem = -1;
		debug("Item is %d", item);
		switch (item) {
		case kStone:
			labItem = 0;
			break;
		case kBricks:
			labItem = 1;
			break;
		case kWood:
			labItem = 2;
			break;
		case kStraw:
			labItem = 3;
			break;
		default:
			labItem = -1;
			break;
		}
		
		if ((name == "daedalus" || name == "chute") && labItem < 0) {
			playDaedalusVideo("daedalus what to do with that", 13005, Common::Point(10, 40));
			return true;
		}

		if (name == "daedalus" && labItem >= 0) {
			playDaedalusVideo("daedalus put that in the chute", 4009, Common::Point(64, 48));
			return true;
		}

		if (name == "chute" && labItem >= 0) {
			bool hasAll = true;
			Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
			g_vm->getHeroBelt()->removeFromInventory(item);
			persistent->_daedalusLabItem[labItem] = true;
			for (int i = 0; i < 4; i++) {
				if (!persistent->_daedalusLabItem[i]) {
					hasAll = false;
					break;
				}
			}

			renderCheckMarks();

			room->playAnimWithSFX("dust cloud", "dust cloud sound", 850, PlayAnimParams::disappear());

			if (hasAll) 
				playDaedalusVideo("daedalus exclaims", 13008, Common::Point(0, 2));
			else {
				// Original goes to event 4009
				if (g_vm->getRnd().getRandomBit())
					playDaedalusVideo("daedalus congrats 1", 4009, Common::Point(70, 30));
				else
					playDaedalusVideo("daedalus congrats 2", 4009, Common::Point(68, 32));
			}

			return true;
		}

		return false;
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		switch(eventId) {
		case kIntroStep1:
			room->playVideo("phil intro 1", kPhilZ, kIntroStep2,
					Common::Point(0, 216));
			room->selectFrame(kDaedalusAmbient, kDaedalusZ, 0);
			break;
		case kIntroStep2:
			playDaedalusVideo("daedalus intro 2", kIntroStep3,
					  Common::Point(76, 55));
			break;
		case kIntroStep3:
			room->playVideo("phil intro 2", kPhilZ, kIntroStep4,
					Common::Point(0, 216));
			room->selectFrame(kDaedalusStillFrame, kDaedalusZ, 0);
			break;

		case kIntroStep4:
			playDaedalusVideo("daedalus intro 3", kIntroStep5,
					  Common::Point(76, 60));
			break;
		case 13004:
			room->stopAnim("daedalus note");
			room->stopAnim("daedalus note text male");
			room->stopAnim("daedalus note text female");
			break;
		case 13005:
			g_vm->addTimer(13006, 5000, 1);
			// Fallthrough
		case kIntroStep5:
		case 4009:
			daedalusBecomesIdle();
			room->enableMouse();
			break;
		case 13006:
			if (!room->isMouseEnabled()) {
				g_vm->addTimer(13006, 5000, 1);
				break;
			}
			room->disableMouse();
			room->playVideo("phil coerces", 0, 13007, Common::Point(0, 216));
			break;
		case 13007:
			room->enableMouse();
			break;
		case 13008:
			room->enableMouse();
			room->selectFrame("daedalus exclaims still", kDaedalusZ,0);
			// TODO: for now we skip arcade sequence until it's implemented
			//			g_vm->moveToRoom(kMinotaurPuzzle);
			g_vm->moveToRoom(kQuiz);
			break;
		case 13011: {
			// TODO: use right algorithm
			int roarNum = g_vm->getRnd().getRandomNumberRng(1, 5);
			room->playSFX(Common::String::format("ambient minotaur roar %d", roarNum), 13012);
			break;
		}
		case 13012:
			g_vm->addTimer(13011, g_vm->getRnd().getRandomNumberRng(5000, 10000));
			break;
		case kDaedalusTick:
			if (_daedalusIsBusy)
				break;
			_daedalusIsBusy = true;
			switch (g_vm->getRnd().getRandomNumberRng(1, 6)) {
			case 1:
				daedalusWallMotion();
				break;
			case 2:
			case 3:
			case 4:
				room->playAnim(kDaedalusAmbient, kDaedalusZ, PlayAnimParams::keepLastFrame().partial(0, 21), 13904);
				break;
			case 5:
			case 6:
				room->playAnim(kDaedalusAmbient, kDaedalusZ, PlayAnimParams::keepLastFrame().partial(23, 30), 13903);
				break;
			}
			break;
		case 13902:
			room->playAnim(kDaedalusAmbient, kDaedalusZ, PlayAnimParams::keepLastFrame().partial(35, -1), 13904);
			room->playAnimWithSFX(kLabyrinthWorkers, "labyrinth workers sound", kLabyrinthWorkersZ,
					      PlayAnimParams::keepLastFrame());
			break;
		case 13903:
			room->playAnim(kDaedalusAmbient, kDaedalusZ, PlayAnimParams::keepLastFrame().partial(57, -1), 13904);
			break;
		case 13904:
			_daedalusIsBusy = false;
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		room->loadHotZones("Daedalus.HOT", false);
		room->addStaticLayer("background", kBackgroundZ);
		room->addStaticLayer("chute label", 850);
		g_vm->getHeroBelt()->setColour(HeroBelt::kWarm);

		if (quest == kCreteQuest) {
			room->addStaticLayer("wings", 900);
			room->addStaticLayer("check list", 800);
			room->addStaticLayer("check list text", 799);
			room->enableHotzone("brick wall");
			room->enableHotzone("chute");
			room->enableHotzone("wings");
			room->enableHotzone("daedalus");
			room->selectFrame(kLabyrinthWorkers, kLabyrinthWorkersZ, 0);
			room->selectFrame(kDaedalusAmbient, kDaedalusZ, 0);
			g_vm->addTimer(13011, g_vm->getRnd().getRandomNumberRng(5000, 10000));
			g_vm->addTimer(kDaedalusTick, g_vm->getRnd().getRandomNumberRng(5000, 10000), -1);
		} else {
			room->enableHotzone("labyrinth");
			if (!persistent->_daedalusShowedNote) {
				persistent->_daedalusShowedNote = true;
				room->selectFrame("daedalus note", 800, 0);
				room->selectFrame(persistent->_gender == kMale ? "daedalus note text male"
						  : "daedalus note text female", 799, 0);
				room->playSpeech(persistent->_gender == kMale ?
						 TranscribedSound::make("daedalus note vo male", "Dear hero, now that we've brought peace to the people of Crete, I've used the wings that I've built for myself and my son Icarus to escape. I'm forever grateful for your help. Your friend, Daedalus") :
						 TranscribedSound::make("daedalus note vo female", "Dear heroine, now that we've brought peace to the people of Crete, I've used the wings that I've built for myself and my son Icarus to escape. I'm forever grateful for your help. Your friend, Daedalus. Au revoir. Salaam. Good bye."),
						 13004);
			}
		}

		renderCheckMarks();

		room->enableHotzone("minos palace");

		if (quest == kCreteQuest
		    && !persistent->isRoomVisited(kDaedalusRoom)) {
			persistent->_creteIntroAtlantisBoat = true;
			persistent->_creteShowAtlantisBoat = true;
			persistent->_creteIntroAtlantisWood = true;
			persistent->_troyPlayAttack = true;
			playDaedalusVideo("daedalus intro 1", kIntroStep1,
					  Common::Point(50, 35));
			room->playMusicLoop("theme music 1");
		} else if (quest == kCreteQuest &&
			   (persistent->_daedalusLabItem[0] || persistent->isInInventory(kStone)) &&
			   (persistent->_daedalusLabItem[1] || persistent->isInInventory(kBricks)) &&
			   (persistent->_daedalusLabItem[2] || persistent->isInInventory(kWood)) &&
			   (persistent->_daedalusLabItem[3] || persistent->isInInventory(kStraw))) {
			room->playMusicLoop("theme music 2");
		} else {
			room->playMusicLoop("R4010eA0");
		}
		AmbientAnim("mouse", "mouse sound", 900, 5000, 10000, AmbientAnim::KEEP_LOOP, Common::Point(0, 0),
			    AmbientAnim::PAN_ANY).start();
	}
private:
	void playDaedalusVideo(const Common::String &name, int callback, const Common::Point &offset) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		_daedalusIsBusy = true;

		room->stopAnim(kDaedalusStillFrame);
		room->stopAnim(kDaedalusAmbient);
		room->selectFrame(kModelPiece, kModelPieceZ, 0);
		room->disableMouse();
		room->playVideo(name, kDaedalusZ, callback, offset);
	}

	void daedalusBecomesIdle() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		room->selectFrame(kDaedalusAmbient, kDaedalusZ, 0);
		room->stopAnim(kModelPiece);
		_daedalusIsBusy = false;
	}

	void renderCheckMarks() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		for (int i = 0; i < 4; i++) {
			Common::String layer = Common::String::format("check mark %d", i + 1);
			if (persistent->_daedalusLabItem[i]) {
				room->selectFrame(layer, 798, 0);
			} else {
				room->stopAnim(layer);
			}
		}
	}

	void daedalusWallMotion() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnim(kDaedalusAmbient, kDaedalusZ, PlayAnimParams::keepLastFrame().partial(0, 34), 13902);
		room->playSFX("daedalus ambient sound");
		_daedalusIsBusy = true;
	}

	bool _daedalusIsBusy;
};

Common::SharedPtr<Hadesch::Handler> makeDaedalusHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new DaedalusHandler());
}

}
