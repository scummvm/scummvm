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

static const char *kDamagedWall = "t1010ob0";
static const char *kBricksImage = "g0120oB0";
static const char *kHephaestusHighlight = "t2330ba0";
static const char *kHeraHighlight = "t2300ba0";
static const char *kOdysseusIdle = "t2150ba0";
static const char *kOdysseusWithMessage = "t2140bb0";
static const char *kOdysseusFateOfGreece = "t2150bb0";
static const char *kPrisoner = "t2130ba0";
static const char *kKeyAndDecreeImage = "g0150ob0";
static const char *kKeyAndDecreePopup = "t2010of0";
static const char *kMenelausImage = "t2070ba0";
static const char *kHelenImage = "t1230ba0";

// TODO: fill this
static const TranscribedSound trClickTranscript[] = {
	{ nullptr, nullptr }
};

enum {
	kAnimationCompleted = 10011,
	kPrisonerVideoCompleteted = 1010001,
	kKeyPlaced = 1010002,
	kMenelausAnimCompleteted = 1010003,
	kBgSoldiersAnimCompleteted = 1010004,
	kHelenAnimCompleteted = 1010005,
	kCatacombAnimCompleteted = 1010006,
	kPlayOutro = 1010007,
	kHorseCounter = 1010008
};

enum {
	kBackgroundZ = 10000,
	kDamagedWallZ = 1101,
	kMenelausZ = 1000,
	kHelenZ = 500,
	kSoldier1Z = 101,
	kSolsier2Z = 101,
	kOdysseusZ = 131,
	kPrisonerZ = 151,
	kSoldier3Z = 161,

	kPopupZ = 0
};

#define PLACEHOLDER_MIN_INTERVAL 7000
#define PLACEHOLDER_MAX_INTERVAL 12000

class TroyHandler : public Handler {
public:
	TroyHandler() {
		_philUseSecondInsistance = false;
		_prisonerCounter = 0;
		_bgSoldierCount = 0;
		_prisonerTouched = false;
		_philWarnedFishing = false;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;

		if (name == "Argo") {
			if (quest == kTroyQuest && !_philWarnedFishing) {
				room->disableMouse();
				room->playVideo("t1030ba0", 0, 10015, Common::Point(0, 216));
				return;
			}
			g_vm->moveToRoom(kArgoRoom);
			return;
		}

		if (name == "Bricks") {
			room->stopAnim(kBricksImage);
			g_vm->getHeroBelt()->placeToInventory(kBricks, 10069);
			room->disableHotzone("Bricks");
			room->disableMouse();
			room->playVideo("t1270ma0", 0);
			return;
		}

		if (name == "Hephaestus' Statue") {
			Common::Array<Common::String> videos;
			videos.push_back("t2330na0");
			videos.push_back("t2330nb0");
			videos.push_back("t2330nc0");

			room->playStatueSMK(kHephaestusStatue,
					    kHephaestusHighlight,
					    201,
					    videos,
					    16, 29,
					    kOffsetRightRoom);
			return;
		}

		if (name == "Hera's Statue") {
			Common::Array<Common::String> videos;
			videos.push_back("t2300na0");
			if (quest > kTroyQuest || (quest == kTroyQuest && persistent->_troyIsDefeated)) {
				videos.push_back("t2320na0");
			} else if (quest == kTroyQuest) {
				videos.push_back("t2310na0");
				videos.push_back("t2310nb0");
				videos.push_back("t2310nc0");
			}

			room->playStatueSMK(kHeraStatue,
					    kHeraHighlight,
					    101,
					    videos,
					    17, 34,
					    kOffsetRightRoom);
			return;
		}

		if (name == "Background2") {
			if (_philUseSecondInsistance)
				room->playVideo("t2140bi0", 0, 10072, Common::Point(640, 216));
			else
				room->playVideo("t2150bc0", 0, 10071, Common::Point(640, 216));
			_philUseSecondInsistance = true;
			room->disableMouse();
			return;
		}

		if (name == "Odysseus' Scroll") {
			room->disableHotzone("Odysseus' Scroll");
			room->disableHotzone("Background2");
			room->playSFX("T2150eA1", 10049);
			hideOdysseus();
			showIdleOdysseus();
			_philUseSecondInsistance = false;
			room->selectFrame("t2010oe0", 101, 0, Common::Point(695, 0));
			g_vm->cancelTimer(10047);
			return;
		}

		if (name == "Scroll PopUp") {
			room->cancelVideo();
			room->disableHotzone("Scroll PopUp");
			room->disableHotzone("Background2");
			room->playSFX("T2150eB0");
			room->stopAnim("t2010oe0");
			room->disableMouse();
			g_vm->getHeroBelt()->placeToInventory(kMessage, 10052);
			return;
		}

		if (name == "Crazy Soldier 1") {
			room->stopAnim(soldier1());
			_trClick.playNext("CampBurntOutSoldier", kAnimationCompleted);
			_soldier1Ambient.pause();
			return;
		}

		if (name == "Crazy Soldier 2") {
			room->stopAnim(soldier2());
			_trClick.playNext("CampCrazySoldier", kAnimationCompleted);
			_soldier2Ambient.pause();
			return;
		}

		if (name == "Crazy Soldier 3") {
			room->stopAnim(soldier3());
			_trClick.playNext("CampShellShockedSoldier", kAnimationCompleted);
			_soldier3Ambient.pause();
			return;
		}

		// Alternate Soldier * use TrClick.txt in the original
		if (name == "Alternate Soldier 1") {
			_soldier1Ambient.play(false);
			return;
		}
		if (name == "Alternate Soldier 2") {
			_soldier2Ambient.play(false);
			return;
		}
		if (name == "Alternate Soldier 3") {
			_soldier3Ambient.play(false);
			return;
		}

		if (name == "Prisoner") {
			room->stopAnim(kPrisoner);
			room->disableMouse();
			_prisonerAmbient.pause();
			_trClick.playChosen("Prisoner", _prisonerCounter, kPrisonerVideoCompleteted);
			_prisonerTouched = true;
			return;
		}

		if (name == "Menelaus' Tent") {
			_menelausTent1Ambient.hide();
			room->disableMouse();
			_trClick.playNext(quest == kCreteQuest ? "MenelausTentBeforeNote"
					  : "MenelausTentAfterNote", kMenelausAnimCompleteted);
			return;
		}

		if (name == "Helen") {
			_helenAmbient.hide();
			room->disableMouse();
			_trClick.playNext("Helen", kHelenAnimCompleteted);
			return;
		}

		// TODO: is targetCounter persistent?
		if (name == "Background Soldiers") {
			room->stopAnim("t2250ba0");
			_bgSoldiersAmbient.pause();
			room->disableMouse();
			int targetCounter = _bgSoldierCount;
			if (quest == kCreteQuest) {
				if (targetCounter < 2) {
					_trClick.playChosen("GroupSoldiersCrete", targetCounter, kBgSoldiersAnimCompleteted);
					_bgSoldierCount++;
					return;
				} else
					targetCounter -= 2;
			}
			if (quest == kMedusaQuest) {
				if (targetCounter < 2) {
					_trClick.playChosen("GroupSoldiersMedusa", targetCounter, kBgSoldiersAnimCompleteted);
					_bgSoldierCount++;
					return;
				} else
					targetCounter -= 2;
			}
			if (quest == kTroyQuest) {
				if (targetCounter < 9) {
					_trClick.playChosen("GroupSoldiersBeforeTroy", targetCounter, kBgSoldiersAnimCompleteted);
					_bgSoldierCount++;
					return;
				} else
					targetCounter -= 9;
			}

			if (quest > kTroyQuest) {
				if (targetCounter < 6) {
					_trClick.playChosen("GroupSoldiersAfterTroy", targetCounter, kBgSoldiersAnimCompleteted);
					_bgSoldierCount++;
					return;
				} else
					targetCounter -= 6;
			}

			_trClick.playChosen("GroupSoldiersAll", targetCounter, kBgSoldiersAnimCompleteted);
			_bgSoldierCount = 0;
			return;
		}

		if (name == "Key And Decree") {
			room->disableMouse();
			persistent->_troyKeyAndDecreeState = Persistent::KEY_AND_DECREE_TAKEN;
			room->disableHotzone("Key And Decree");
			room->stopAnim(kKeyAndDecreeImage);
			room->selectFrame(kKeyAndDecreePopup, kPopupZ, 0, kOffsetRightRoom);
			g_vm->addTimer(10057, 2000, 1);
			return;
		}

		if (name == "Catacomb") {
			if (quest == kCreteQuest) {
				room->disableMouse();
				room->playVideo("t1280ba0", 0, 10017, Common::Point(0, 217));
				return;
			}

			if (persistent->_troyCatacombCounter == 0 || persistent->_troyCatacombCounter == 1) {
				room->stopAnim(persistent->_troyCatacombCounter == 1 ? "t1280bc0" : "t1280bb0");
				_trClick.playChosen("Catacomb", persistent->_troyCatacombCounter, kCatacombAnimCompleteted);
				persistent->_troyCatacombCounter++;
				return;
			}

			if (quest > kTroyQuest) {
				room->playVideo("T1280BD0", 0, 10016, Common::Point(0, 216));
				return;
			}

			room->enableHotzone("Background");
			room->enableHotzone("Catacomb PopUp");
			room->selectFrame("t1010oe0", 106, 0);
			room->selectFrame("t1290bb0", 105, persistent->_troyCatacombsUnlocked ? 12 : 0);
			room->enableHotzone(persistent->_troyCatacombsUnlocked ? "Link To Catacombs" : "Catacomb PopUp Grate");

			// TODO: unschedule 10023
			return;
		}

		if (name == "Background") {
			room->disableHotzone("Background");
			room->disableHotzone("Catacomb PopUp");
			room->stopAnim("t1010oe0");
			room->stopAnim("t1290bb0");
			room->disableHotzone("Link To Catacombs");
			room->disableHotzone("Catacomb PopUp Grate");
			// TODO: schedule 10023
			return;
		}

		if (name == "Link To Catacombs") {
			room->disableMouse();
			g_vm->moveToRoom(kCatacombsRoom);
			return;
		}

		if (name == "Catacomb PopUp Grate") {
			room->disableMouse();
			room->playVideo(
				!persistent->isInInventory(kKey) && _prisonerTouched ? "t1290ba0"
				: "t1290bd0", 0, 10020,
				Common::Point(0, 216));
			return;
		}
	}

	bool handleClickWithItem(const Common::String &name, InventoryItem item) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (name == "Catacomb PopUp Grate") {
			if (item == kKey) {
				room->disableMouse();
				g_vm->getHeroBelt()->removeFromInventory(kKey);
				room->playAnimWithSFX("t1290bb0", "t1290xa0", 105, PlayAnimParams::keepLastFrame(),
						      10060);
				persistent->_troyCatacombsUnlocked = true;
				room->disableHotzone("Catacomb PopUp Grate");
				room->enableHotzone("Link To Catacombs");
				return true;
			}

			if (!_prisonerTouched || !persistent->isInInventory(kKey)) {
				room->disableMouse();
				room->playVideo(
					_prisonerTouched ? "t1290ba0" : "t1290bd0", 0, 10020,
					Common::Point(0, 216));
				return true;
			}

			return false;
		}

		return false;
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		switch(eventId) {
		case 2803:
			_leftAmbients.tick();
			break;
		case kAnimationCompleted:
			room->stopAnim(kHephaestusHighlight);
			room->stopAnim(kHeraHighlight);
			soldiersDisplay();
			room->enableMouse();
			_soldier1Ambient.unpause();
			_soldier2Ambient.unpause();
			_soldier3Ambient.unpause();
			break;
		case kCatacombAnimCompleteted:
			room->enableMouse();
			showCatacombStones();
			break;
		case 10015:
			_philWarnedFishing = true;
			room->enableMouse();
			break;
		case 10013:
		case 10017:
		case 10018:
		case 10019:
		case 10020:
		case 10060:
			room->enableMouse();
			break;
		case 10022:
			// Attack on castle
			room->playAnimWithSFX("t1250bb0",
					      "t1250eb0", 131,
					      PlayAnimParams::disappear(),
					      10027);
			break;
		case 10027:
			room->selectFrame(kDamagedWall, kDamagedWallZ, 0);
			showBricks();
			room->enableMouse();
			break;
		case 10044:
			g_vm->addTimer(10045, 500, 1);
			break;
		case 10045:
			hideOdysseus();
			room->playVideo("t2140ba0", kOdysseusZ, 10046, Common::Point(649, 17));
			break;
		case 10046:
			room->playAnimLoop(kOdysseusWithMessage, kOdysseusZ, Common::Point(800, 0));
			room->setLayerParallax(kOdysseusWithMessage, -160);
			g_vm->addTimer(10047, 20000, -1);
			room->enableHotzone("Odysseus' Scroll");
			room->enableHotzone("Background2");
			room->enableMouse();
			room->setPannable(false);
			break;
		case 10047:
			if (room->isMouseEnabled()) {
				room->stopAnim(kOdysseusWithMessage);
				room->playVideo("t2140bc0", kOdysseusZ, 10048, Common::Point(650, 17));
			} 
			break;
		case 10048:
			room->playAnimLoop(kOdysseusWithMessage, kOdysseusZ, Common::Point(800, 0));
			break;
		case 10049:
			room->enableHotzone("Scroll PopUp");
			room->enableHotzone("Background2");
			room->playVideo("t2150xb0", 0, 10050);
			break;
			// 10050 is cleanup
		case 10052:
			hideOdysseus();
			room->playVideo(kOdysseusFateOfGreece, kOdysseusZ, 10053, Common::Point(640, 8));
			break;
		case 10053:
			room->enableMouse();
			room->setPannable(true);
			break;
		case 10055:
			hideOdysseus();
			room->playVideo("T2340BB0", kOdysseusZ, 10056, Common::Point(649, 18));
			break;
		case 10056:
			showIdleOdysseus();
			// TODO: for now we skip arcade sequence until it's implemented
			if (0) {
				g_vm->moveToRoom(kTrojanHorsePuzzle);
			} else {
				persistent->_troyPlayFinish = true;
				g_vm->moveToRoom(kTroyRoom);
			}
			break;
		case 10057:
			room->playSpeech(
				TranscribedSound::make("T2240wA0", "Official orders from king Priam: messenger is granted permissions to leave the city walls"), 10058);
			break;
		case 10058:
			room->enableMouse();
			room->stopAnim(kKeyAndDecreePopup);
			g_vm->getHeroBelt()->placeToInventory(kKey, kKeyPlaced);
			break;
		case kKeyPlaced:
			g_vm->getHeroBelt()->placeToInventory(kDecree);
			break;
		case kHorseCounter:
			_horseCounter--;
			if (_horseCounter != 0)
				break;
			/* Fallthrough */
		case 10063:
			room->playSFX("t1350ec0", 10064);
			break;
		case 10064:
			room->playAnimKeepLastFrame("t1350bb0", 501, 10065);
			room->playSFX("t1350ed0", 10066);
			break;
		case 10065:
			room->playAnim("t1350bb0", 501, PlayAnimParams::loop().partial(8, 11));
			break;
		case 10066:
			g_vm->moveToRoom(kQuiz);
			break;
		case 10069:
			// TODO: check this
			room->playVideo("T1270BA0", 0, 10070, Common::Point(0, 216));
			break;
		case 10070:
		case 10071:
		case 10072:
			room->enableMouse();
			break;
		case kPrisonerVideoCompleteted:
			room->enableMouse();
			_prisonerAmbient.unpauseAndFirstFrame();
			if (_prisonerCounter == 2) {
				room->selectFrame(kKeyAndDecreeImage, kPrisonerZ, 0, kOffsetRightRoom);
				room->enableHotzone("Key And Decree");
				persistent->_troyKeyAndDecreeState = Persistent::KEY_AND_DECREE_THROWN;
			}
			if (_prisonerCounter < 7)
				_prisonerCounter++;
			break;
		case kMenelausAnimCompleteted:
			_menelausTent1Ambient.unpauseAndFirstFrame();
			room->enableMouse();
			break;
		case kHelenAnimCompleteted:
			_helenAmbient.unpauseAndFirstFrame();
			room->enableMouse();
			break;
		case kBgSoldiersAnimCompleteted:
			_bgSoldiersAmbient.unpauseAndFirstFrame();
			room->enableMouse();
			break;
		case kPlayOutro:
			_menelausTent1Ambient.hide();
			_trClick.playNext("MenelausClosingStatement", 10055);
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		Common::RandomSource &rnd = g_vm->getRnd();
		room->loadHotZones("troy.HOT", false);
		room->addStaticLayer("t1010pa0", kBackgroundZ);
		room->setPannable(true);
		g_vm->getHeroBelt()->setColour(HeroBelt::kWarm);
		room->enableHotzone("Argo");
		room->enableHotzone("Hephaestus' Statue");
		room->enableHotzone("Hera's Statue");
		room->enableHotzone("Catacomb");
		room->playAnimLoop("T1110BA0", 501);

		_trClick.readTable(room, "TrClick.txt", trClickTranscript);

		if (persistent->_troyWallDamaged && quest == kCreteQuest) {
			room->selectFrame(kDamagedWall, kDamagedWallZ, 0);
		}

		if (persistent->_troyShowBricks && quest == kCreteQuest) {
			showBricks();
		}

		if (persistent->_troyPlayAttack && quest == kCreteQuest) {
			room->disableMouse();
			persistent->_troyPlayAttack = false;
			persistent->_troyShowBricks = true;
			persistent->_troyWallDamaged = true;
			room->playVideo("t1060ba0", 0, 10022,
					   Common::Point(0, 201));
		}

		room->enableHotzone("Background Soldiers");
		room->selectFrame("t2250ba0", 601, 0, kOffsetRightRoom);
		_bgSoldiersAmbient = AmbientAnim("t2250ba0", "", 601, PLACEHOLDER_MIN_INTERVAL,
						 PLACEHOLDER_MAX_INTERVAL, AmbientAnim::KEEP_LOOP, kOffsetRightRoom,
						 AmbientAnim::PAN_RIGHT);
		_bgSoldiersAmbient.start();

		if (!persistent->_troyPlayedOdysseus && quest == kTroyQuest) {
		    room->panRightAnim(10044);
		    room->disableMouse();
		    showIdleOdysseus();
		}

		if (quest <= kTroyQuest) {
			_soldier1IsCrazy = rnd.getRandomBit();
			_soldier2IsCrazy = rnd.getRandomBit();
			_soldier3IsCrazy = rnd.getRandomBit();
			room->addStaticLayer("t1010pd0", 121, Common::Point(712, 187));
			room->setLayerParallax("t1010pd0", -160);
			soldiersDisplay();

			// TODO: check times
			_soldier1Ambient =
				AmbientAnim(soldier1(), soldier1Sound(),
					    kSoldier1Z, PLACEHOLDER_MIN_INTERVAL, PLACEHOLDER_MAX_INTERVAL,
					    AmbientAnim::KEEP_LOOP,
					    Common::Point(980, 0),
					    AmbientAnim::PAN_RIGHT);
			_soldier1Ambient.start();
			_soldier2Ambient = AmbientAnim(soldier2(), soldier2Sound(), kSolsier2Z, PLACEHOLDER_MIN_INTERVAL,
						       PLACEHOLDER_MAX_INTERVAL, AmbientAnim::KEEP_LOOP, kOffsetRightRoom,
						       AmbientAnim::PAN_RIGHT);
			_soldier2Ambient.start();
			_soldier3Ambient = AmbientAnim(soldier3(), soldier3Sound(), kSoldier3Z, PLACEHOLDER_MIN_INTERVAL,
						       PLACEHOLDER_MAX_INTERVAL, AmbientAnim::KEEP_LOOP, kOffsetRightRoom,
						       AmbientAnim::PAN_RIGHT);
			_soldier3Ambient.start();

			room->enableHotzone(_soldier1IsCrazy ? "Crazy Soldier 1" : "Alternate Soldier 1");
			room->enableHotzone(_soldier2IsCrazy ? "Crazy Soldier 2" : "Alternate Soldier 2");
			room->enableHotzone(_soldier3IsCrazy ? "Crazy Soldier 3" : "Alternate Soldier 3");
			room->enableHotzone("Menelaus' Tent");

			_menelausTent1Ambient = AmbientAnim(kMenelausImage, "", kMenelausZ, PLACEHOLDER_MIN_INTERVAL,
							    PLACEHOLDER_MAX_INTERVAL, AmbientAnim::KEEP_LOOP, kOffsetRightRoom,
							    AmbientAnim::PAN_RIGHT);
			_menelausTent1Ambient.start();
			room->selectFrame(kMenelausImage, kMenelausZ, 0, kOffsetRightRoom);

			Common::Array<AmbientAnim::AmbientDesc> tent2Ambients;
			tent2Ambients.push_back(AmbientAnim::AmbientDesc("t2070bb0", ""));
			tent2Ambients.push_back(AmbientAnim::AmbientDesc("t2070bm0", ""));
			tent2Ambients.push_back(AmbientAnim::AmbientDesc("t2070bn0", ""));
			tent2Ambients.push_back(AmbientAnim::AmbientDesc("t2070bo0", ""));

			_menelausTent2Ambient = AmbientAnim(
				tent2Ambients, kMenelausZ, PLACEHOLDER_MIN_INTERVAL,
				PLACEHOLDER_MAX_INTERVAL, AmbientAnim::KEEP_LOOP, kOffsetRightRoom,
				AmbientAnim::PAN_RIGHT);
			_menelausTent2Ambient.start();
			room->selectFrame("t2070bb0", kMenelausZ, 0, kOffsetRightRoom);

			room->selectFrame(Common::String::format("t2010o%c0",
								 rnd.getRandomNumberRng('g', 'i')),
					  141, 0, kOffsetRightRoom);

			if (!persistent->_troyMessageIsDelivered) {
				room->enableHotzone("Helen");
				_helenAmbient = AmbientAnim(
					kHelenImage, "", kHelenZ, PLACEHOLDER_MIN_INTERVAL,
					PLACEHOLDER_MAX_INTERVAL, AmbientAnim::KEEP_LOOP,
					Common::Point(0, 0),
					AmbientAnim::PAN_LEFT);
				_helenAmbient.start();
				room->selectFrame(kHelenImage, kHelenZ, 0);
			}
		}

		if (quest == kTroyQuest) {
			room->enableHotzone("Prisoner");
			if (!persistent->_troyPlayedOdysseus) {
				persistent->_troyPlayedOdysseus = true;
			}

			room->selectFrame(kPrisoner, kPrisonerZ, 0, kOffsetRightRoom);
			_prisonerAmbient = AmbientAnim(kPrisoner, "t2130ea0", kPrisonerZ,
						       PLACEHOLDER_MIN_INTERVAL, PLACEHOLDER_MAX_INTERVAL,
						       AmbientAnim::KEEP_LOOP, kOffsetRightRoom,
						       AmbientAnim::PAN_RIGHT);
			_prisonerAmbient.start();

			if (persistent->_troyKeyAndDecreeState == Persistent::KEY_AND_DECREE_THROWN) {
				room->selectFrame(kKeyAndDecreeImage, kPrisonerZ, 0, kOffsetRightRoom);
				room->enableHotzone("Key And Decree");
			}

			if (persistent->_troyKeyAndDecreeState >= Persistent::KEY_AND_DECREE_THROWN) {
				_prisonerCounter = 7;
			}
		}

		room->playMusicLoop(persistent->_troyMessageIsDelivered || quest > kTroyQuest ? "t1010eb0" : "t1010ea0");

		if (quest <= kTroyQuest && !persistent->_troyPlayFinish) {
			TextTable trLftAmb = TextTable(
				Common::SharedPtr<Common::SeekableReadStream>(room->openFile("TrLftAmb.txt")), 9);
			_leftAmbients.readTableFileSFX(trLftAmb, AmbientAnim::PAN_LEFT);
			g_vm->addTimer(2803, 10000, -1);
			_leftAmbients.firstFrame();
		}

		room->playAnimLoop("t2055bb0", 501, kOffsetRightRoom);
		room->playAnimLoop("t2055bc0", 501, kOffsetRightRoom);
		room->playAnimLoop("t2055bd0", 501, kOffsetRightRoom);
		room->playAnimLoop("t2055be0", 501, kOffsetRightRoom);

		showCatacombStones();

		if (persistent->_previousRoomId == kCatacombsRoom) {
			room->disableMouse();
			room->playVideo(selectReturnFromCatacombs(), 0, 10013, Common::Point(0, 216));
		}

		if (persistent->_previousRoomId == kPriamRoom
		    && !persistent->_troyMessageIsDelivered
		    && quest == kTroyQuest) {
			room->playVideo("t1290bc0", 0, 10018,
					Common::Point(0, 216));
		}

		if (persistent->_previousRoomId == kPriamRoom
		    && persistent->_troyMessageIsDelivered
		    && !persistent->_troyPlayedOdysseusCongrats
		    && quest == kTroyQuest) {
			persistent->_troyPlayedOdysseusCongrats = true;
			room->panRightAnim(kPlayOutro);
			room->disableMouse();
			showIdleOdysseus();
		}

		if (persistent->_troyPlayFinish && quest == kTroyQuest) {
			persistent->_troyPlayFinish = false;
			room->disableMouse();
			_horseCounter = 2;
			room->playMusic("T1350mA0", kHorseCounter);
			room->playVideo("t1350ba0", 501, kHorseCounter, Common::Point(288, 211));

			room->playAnimLoop("t1090ba0", 501);
			room->playAnimLoop("t1090bb0", 501);
		}
	}

private:
	Common::String selectReturnFromCatacombs() const {
		Persistent *persistent = g_vm->getPersistent();
		if (persistent->_catacombLastLevel == 2) {
			return "t1310bg0";
		}

		switch (g_vm->getRnd().getRandomNumberRng(0, 4)) {
		case 0:
			return "t1310ba0";
		case 1:
			return persistent->_gender == kMale ? "t1310bb0" : "t1310bc0";
		case 2:
			return "t1310bd0";
		case 3:
			return persistent->_gender == kMale ? "t1310be0" : "t1310bf0";
		case 4:
		default:
			return "t1310bh0";
		}
	}
	void showCatacombStones() const {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (persistent->_troyMessageIsDelivered || persistent->_quest > kTroyQuest) {
			room->selectFrame("t1010og0", 115, 0);
			return;
		}
		room->selectFrame("t1280bc0", 212, (persistent->_troyCatacombCounter == 2) ? 4 : 0);
		room->selectFrame("t1280bb0", 211, (persistent->_troyCatacombCounter >= 1) ? 3 : 0);

		if (persistent->_troyCatacombsUnlocked) {
			room->stopAnim("t1010of0");
		} else {
			room->selectFrame("t1010of0", 421, 0);
		}
	}

	void soldiersDisplay() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->selectFrame(soldier1(), kSoldier1Z, 0, Common::Point(980, 0));
		room->setLayerParallax(soldier1(), -340);
		room->selectFrame(soldier2(), kSolsier2Z, 0, kOffsetRightRoom);
		room->selectFrame(soldier3(), kSoldier3Z, 0, kOffsetRightRoom);
	}

	void showIdleOdysseus() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnimLoop(kOdysseusIdle, kOdysseusZ, Common::Point(800, 0));
		room->setLayerParallax(kOdysseusIdle, -160);
	}

	void hideOdysseus() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->stopAnim(kOdysseusWithMessage);
		room->stopAnim(kOdysseusIdle);
	}

	void showBricks() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->selectFrame(kBricksImage, 121, 0);
		room->enableHotzone("Bricks");
	}

	Common::String soldier1() {
		return _soldier1IsCrazy ? "t2080ba0" : "t2120ba0";
	}

	Common::String soldier2() {
		return _soldier2IsCrazy ? "t2090ba0" : "t2120bb0";
	}

	Common::String soldier3() {
		return _soldier3IsCrazy ? "t2100ba0" : "t2120bc0";
	}

	Common::String soldier1Sound() {
		return _soldier1IsCrazy ? "t2080ea0" : "t2120ea0";
	}

	Common::String soldier2Sound() {
		return _soldier2IsCrazy ? "t2090ea0" : "t2120eb0";
	}

	Common::String soldier3Sound() {
		return _soldier3IsCrazy ? "t2100ea0" : "t2120ec0";
	}

	bool _philUseSecondInsistance;
	bool _philWarnedFishing;
	bool _soldier1IsCrazy;
	bool _soldier2IsCrazy;
	bool _soldier3IsCrazy;
	bool _prisonerTouched;

	// TODO: read ambients from file
	AmbientAnim _soldier1Ambient;
	AmbientAnim _soldier2Ambient;
	AmbientAnim _soldier3Ambient;
	AmbientAnim _prisonerAmbient;
	AmbientAnim _menelausTent1Ambient;
	AmbientAnim _menelausTent2Ambient;
	AmbientAnim _bgSoldiersAmbient;
	AmbientAnim _helenAmbient;

	AmbientAnimWeightedSet _leftAmbients;
	AnimClickables _trClick;
	int _prisonerCounter;
	int _bgSoldierCount;
	int _horseCounter;
};

Common::SharedPtr<Hadesch::Handler> makeTroyHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new TroyHandler());
}

}
