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

static const char *kTalusImage = "r1100bb0";
static const char *kTalusImageWithShip = "r1100bc0";  
static const char *kTalusMovie = "r1100ba0";
static const char *kTalusHotzone = "Talus";
static const char *kTavernImage = "r2190ba0";
static const char *kTavernHotzone = "Tavern";
static const char *kPoseidonHighlight = "r1230ba0";
static const char *kZeusHighlight = "r1240ba0";
static const char *kHermesHighlight = "r2320ba0";
static const char *kHornless1 = "g0170ob0";
static const char *kHornless2 = "g0170oe0";
static const char *kHornless3 = "g0170oh0";
static const char *kHornless4 = "g0170ok0";
static const char *kHorned = "g1800ob0";
static const char *kHornedHotzone = "HornedStatue";
static const char *kHornless1Hotzone = "HornlessStatue1";
static const char *kHornless2Hotzone = "HornlessStatue2";
static const char *kHornless3Hotzone = "HornlessStatue3";
static const char *kHornless4Hotzone = "HornlessStatue4";
static const char *kMerchantAnim = "r2130ba0";
static const char *kOneManBandAnim = "r2040ba1";
static const char *kOneManBandHotZone = "OneManBand";
static const char *kAtlantisOpening = "r1210bb0";
static const char *kAtlantisLargeDisk = "r1210be0";
static const char *kAtlantisMediumDisk = "r1210bf0";
static const char *kAtlantisSmallDisk = "r1210bg0";
static const char *kAtlantisDiskBackground = "r1210os0";

static const int vaseSol[] = {2, 3, 2, 3};
static const char *vaseSound[] = {
	"r1220ec0",
	"r1220ed0",
	"r1220ee0",
	"r1220ef0"
};

static const char *vaseSegment[] = {
	"r1220bc0",
	"r1220bd0",
	"r1220be0",
	"r1220bf0"	
};

enum {
	kBackgroundZ = 10000,
	kTalusZ = 6000,
	kTavernImageZ = 5000,
	kOneManBandZ = 1600,
	kMerchantStandZ = 1205,
	kMerchantZ = 1200,
	kAtlantisDiskBackgroundZ = 1100,
	kAtlantisLargeDiskZ = 1000,
	kAtlantisMediumDiskZ = 900,
	kAtlantisSmallDiskZ = 800,
	kAtlantisOpeningZ = 700
};

enum {
	kIntroMerchantVideoFinished = 12107,
	kTakenHorned = 12115,
	kTakenHornless = 12118,
	kMerchantIdleTimer = 12121,
	kMerchantIdleAnimCleanup = 12122,
	kTalueMovieCompleted = 12129,
	// 12137 is the end of statue animation that we handle as functor instead
	kAtlantisDoorOpens = 12307,
	kTavernMovieCompleted = 1012001,
	kIntroMerchantPanFinished = 1012002,
	kAtlantisBoatIntro2Finished = 1012003,
	kTakenWood = 1012005,
	kSoundVaseSegment1Finished = 1012006,
	kSoundVaseSegment2Finished = 1012007,
	kSoundVaseSegment3Finished = 1012008,
	kSoundVaseSegment4Finished = 1012009,
	kCoinGiven = 1012010,
	kSandalsPlaced = 1012011
};

static const PrePoint strongBoxSideDotOffsets[] = {
	{1060, 278},
	{1090, 310},
	{1060, 339},
	{1032, 305}
};

static const PrePoint strongBoxTileOffsets[] = {
	{1005, 274},
	{1035, 276},
	{1064, 278},
	{1093, 280},
	{1005, 304},
	{1035, 306},
	{1064, 308},
	{1093, 310},
	{1035, 246},
	{1064, 248},
	{1035, 337},
	{1064, 339}
};

static const int baseFrame[4][8] = {
	{  1,  3,  4,  6,  1,  3,  4,  6 },
	{  7,  9, 10, 12, 13, 15, 16, 18 },
	{ 19, 21, 22, 24, 25, 27, 28, 30 },
	{ 31, 33, 34, 36, 31, 33, 34, 36 }
};

static const int kTileMovementTime = 500;

class StrongBoxTile {
public:
	enum Letter {
		kLetterZ = 1,
		kLetterE,
		kLetterU,
		kLetterS
	};
	enum Orientation {
		kOrientation0 = 0,
		kOrientation90 = 90,
		kOrientation180 = 180,
		kOrientation270 = 270
	};

	void rotate() {
		_orientation = (Orientation) ((_orientation + 90) % 360);
	}

	void show() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		int orientation = _orientation / 45;
		if (_nextPosition != -1 && g_vm->getCurrentTime() > kTileMovementTime + _movementStartTime) {
			_position = _nextPosition;
			_nextPosition = -1;
		}
		Common::Point pos = strongBoxTileOffsets[_position].get();
		if (_nextPosition != -1) {
			double frac = (g_vm->getCurrentTime() - _movementStartTime + 0.0) / kTileMovementTime;
			pos = strongBoxTileOffsets[_position].get() * (1-frac) + strongBoxTileOffsets[_nextPosition].get() * frac;
		}
		int zVal = 500;
		switch (_rotationPhase) {
		case 0:
		case 2:
			pos += Common::Point(-5, 4);
			zVal = 300;
			break;
		case 1:
			orientation++;
			switch (_position) {
			case 1:
				pos = Common::Point(1046, 277);
				break;
			case 2:
				pos = Common::Point(1065, 298);
				break;
			case 5:
				pos = Common::Point(1025, 297);
				break;
			case 6:
				pos = Common::Point(1045, 318);
				break;
			}
			zVal = 300;
			break;
		}
		int frame = baseFrame[_letter - kLetterZ][orientation] - 1;
		if (_position < 4 && _nextPosition == -1 && _rotationPhase < 0)
			frame++;
		room->selectFrame(LayerId("r2010om0", _position, "pos"),
				  zVal, frame, pos);
	}

	void setRotationPhase(int phase) {
		_rotationPhase = phase;
	}

	bool isMoving() {
		return _nextPosition != -1 || _rotationPhase != -1;
	}

	int getPosition() {
		return _position;
	}

	Letter getLetter() {
		return _letter;
	}

	Orientation getOrientation() {
		return _orientation;
	}

	void setPosition(int position) {
		_position = position;
	}

	void moveTo(int pos) {
		_nextPosition = pos;
		_movementStartTime = g_vm->getCurrentTime();
	}

	StrongBoxTile() {
		_letter = kLetterZ;
		_position = 0;
		_orientation = kOrientation0;
		_nextPosition = -1;
		_rotationPhase = -1;
	}

	StrongBoxTile(Letter letter, Orientation orientation, int position) {
		_letter = letter;
		_position = position;
		_orientation = orientation;
		_nextPosition = -1;
		_rotationPhase = -1;
	}
private:
	Letter _letter;
	int _position;
	int _nextPosition;
	int _movementStartTime;
	int _rotationPhase;
	Orientation _orientation;
};

static const struct {
	StrongBoxTile::Letter letter;
	StrongBoxTile::Orientation orientation;
} initialLetters[12] = {
	{StrongBoxTile::kLetterZ, StrongBoxTile::kOrientation90},
	{StrongBoxTile::kLetterE, StrongBoxTile::kOrientation90},
	{StrongBoxTile::kLetterU, StrongBoxTile::kOrientation90},
	{StrongBoxTile::kLetterS, StrongBoxTile::kOrientation90},
	{StrongBoxTile::kLetterE, StrongBoxTile::kOrientation0},
	{StrongBoxTile::kLetterZ, StrongBoxTile::kOrientation0},
	{StrongBoxTile::kLetterU, StrongBoxTile::kOrientation270},
	{StrongBoxTile::kLetterS, StrongBoxTile::kOrientation90},
	{StrongBoxTile::kLetterU, StrongBoxTile::kOrientation90},
	{StrongBoxTile::kLetterE, StrongBoxTile::kOrientation90},
	{StrongBoxTile::kLetterZ, StrongBoxTile::kOrientation0},
	{StrongBoxTile::kLetterS, StrongBoxTile::kOrientation0}
};

class CreteHandler : public Handler {
public:
	CreteHandler() {
		_tavernCounter = 0;
		_oneManBandCounter = 0;
		_merchantIsBusy = false;
		memset(_vaseBusy, 0, sizeof(_vaseBusy));
		for (int i =0; i < 12; i++)
			_strongBoxTiles[i] = StrongBoxTile(initialLetters[i].letter, initialLetters[i].orientation, i);
		_strongBoxPopup = false;
		_fadingHades = false;
		_fadingHadesStartTime = 0;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (name == kTalusHotzone) {
			room->playVideo(kTalusMovie, kTalusZ, kTalueMovieCompleted,
					Common::Point(54, 29));
			room->setLayerEnabled(kTalusImageWithShip, false);
			return;
		}

		if (name == "MinosPalace") {
			room->disableMouse();
			g_vm->moveToRoom(kMinosPalaceRoom);
			return;
		}

		if (name == "Poseidon") {
			Common::Array<Common::String> videos;
			videos.push_back("r1230na0");
			videos.push_back("r1230nb0");
			videos.push_back("r1230nc0");

			room->playStatueSMK(kPoseidonStatue,
					    kPoseidonHighlight,
					    5000,
					    videos, 16, 39);
			return;
		}

		if (name == "Zeus") {
			Common::Array<Common::String> videos;
			videos.push_back("r1240wa0");
			videos.push_back("r1240wb0");

			room->playStatueSMK(kZeusStatue, kZeusHighlight, 5000,
					    videos, 23, 35);
			return;
		}

		if (name == "Hermes") {
			Common::Array<Common::String> videos;
			videos.push_back("r2320na0");
			videos.push_back("r2320nb0");

			room->playStatueSMK(kHermesStatue,
					    kHermesHighlight,
					    4000,
					    videos, 22, 39, kOffsetRightRoom);
			return;
		}

		if (name == kTavernHotzone) {
			room->playAnimLoop(kTavernImage, 5000, kOffsetRightRoom);
			room->playVideo(_tavernTalks[_tavernCounter], 5000,
					   kTavernMovieCompleted,
					   kOffsetRightRoom);
			_tavernCounter = (_tavernCounter + 1) % _tavernTalks.size();
			return;
		}

		if (name == "Argo") {
			g_vm->moveToRoom(kArgoRoom);
			return;
		}

		if (name == kHornedHotzone) {
			g_vm->getHeroBelt()->placeToInventory(kHornedStatue, kTakenHornless);
			room->setLayerEnabled(kHorned, false);
			persistent->_creteShowHorned = false;
			_merchantIsBusy = true;
			room->disableHotzone(kHornedHotzone);
			room->disableMouse();
			return;
		}

		if (name == kHornless1Hotzone) {
			g_vm->getHeroBelt()->placeToInventory(kHornlessStatue1, kTakenHorned);
			room->setLayerEnabled(kHornless1, false);
			persistent->_creteShowHornless1 = false;
			room->disableHotzone(kHornless1Hotzone);
			_merchantIsBusy = true;
			room->disableMouse();
			return;
		}

		if (name == kHornless2Hotzone) {
			g_vm->getHeroBelt()->placeToInventory(kHornlessStatue2, kTakenHorned);
			room->setLayerEnabled(kHornless2, false);
			persistent->_creteShowHornless2 = false;
			room->disableHotzone(kHornless2Hotzone);
			_merchantIsBusy = true;
			room->disableMouse();
			return;
		}

		if (name == kHornless3Hotzone) {
			g_vm->getHeroBelt()->placeToInventory(kHornlessStatue3, kTakenHorned);
			room->setLayerEnabled(kHornless3, false);
			persistent->_creteShowHornless3 = false;
			room->disableHotzone(kHornless3Hotzone);
			_merchantIsBusy = true;
			room->disableMouse();
			return;
		}

		if (name == kHornless4Hotzone) {
			g_vm->getHeroBelt()->placeToInventory(kHornlessStatue4, kTakenHorned);
			room->setLayerEnabled(kHornless4, false);
			persistent->_creteShowHornless4 = false;
			room->disableHotzone(kHornless4Hotzone);
			_merchantIsBusy = true;
			room->disableMouse();
			return;
		}

		if (name == kOneManBandHotZone) {
			room->playVideo(
				Common::String::format(
					"r2040b%c0", 'a' + (_oneManBandCounter % 3)),
				1600,
				12135, Common::Point(730, 183));
			room->stopAnim(kOneManBandAnim);
			_oneManBandCounter++;
			return;
		}

		if (name == "AtlantisBoat") {
			room->pushHotZones("Door.HOT");
			room->disableHotzone("wood");
			room->selectFrame("r1010ob0", 1200, 0);
			room->selectFrame(kAtlantisDiskBackground, kAtlantisDiskBackgroundZ, 0);
			for (unsigned i = 0; i < 3; i++)
				_atlantisBoatPosition[i] = g_vm->getRnd().getRandomNumberRng(1, 7);
			renderAtlantisDisks();
			if (persistent->_creteIntroAtlantisWood) {
				room->disableMouse();
				room->playVideo("R1210BA0", 600, kAtlantisBoatIntro2Finished);
				persistent->_creteIntroAtlantisWood = false;
			}
			return;
				
		}

		if (name == "SmallDisk") {
			advanceAtlantisDisk(2);
			return;
		}

		if (name == "MediumDisk") {
			advanceAtlantisDisk(1);
			return;
		}

		if (name == "LargeDisk") {
			advanceAtlantisDisk(0);
			return;
		}

		if (name == "wood") {
			room->selectFrame(kAtlantisOpening, kAtlantisOpeningZ, 9);
			g_vm->getHeroBelt()->placeToInventory(kWood, kTakenWood);
			room->popHotZones();
			room->disableHotzone("AtlantisBoat");
			persistent->_creteShowAtlantisBoat = false;
			return;
		}

		if (name == "Background") {
			room->popHotZones();
			room->stopAnim("r1010ob0");
			hideStrongBox();
			hideAtlantisPopupOverlays();
			showMiniStrongBox();
			return;
		}

		for (int i = 0; i < 4; i++) {
			if (name == Common::String::format("VaseSegment%d", i + 1) && !_vaseBusy[i]) {
				_vasePos[i]++;
				if (i == 1 || i == 3)
					_vasePos[i] %= 6;
				else
					_vasePos[i] %= 4;
				_vaseBusy[i] = true;
				renderVase();
				room->playSFX(vaseSound[i], kSoundVaseSegment1Finished + i);
				return;
			}
		}

		if (name == "sandals") {
			g_vm->getHeroBelt()->placeToInventory(kSandals, kSandalsPlaced);
			room->selectFrame("r1220ba0", 500, 0);
			room->disableMouse();
			persistent->_creteSandalsState = Persistent::SANDALS_TAKEN;
			return;
		}

		if (name == "AlchemistStand") {
			_alchemistAmbient.play(false);
			return;
		}

		if (name == "StrongBoxClosed"
			|| name == "StrongBoxOpen"
			|| name == "StrongBoxOpenPotion"
			|| name == "StrongBoxOpenNoPotion") {
			showStrongBox();
			return;
		}

		if (name == "Latch") {
			room->disableMouse();
			persistent->_creteStrongBoxState = Persistent::BOX_OPEN;
			room->playAnimWithSFX("r2230bb0", "g0082ea0", 1000,
					      PlayAnimParams::keepLastFrame().partial(1, 3),
					      12402, kOffsetRightRoom);
			return;
		}

		if (name == "ButtonN" && !strongBoxIsBusy()) {
			strongBoxMoveTiles(1, 8, 9, 2);
			return;
		}

		if (name == "ButtonE" && !strongBoxIsBusy()) {
			strongBoxMoveTiles(2, 3, 7, 6);
			return;
		}

		if (name == "ButtonW" && !strongBoxIsBusy()) {
			strongBoxMoveTiles(1, 5, 4, 0);
			return;
		}

		if (name == "ButtonS" && !strongBoxIsBusy()) {
			strongBoxMoveTiles(5, 6, 11, 10);
			return;
		}

		if (name == "ButtonC" && !strongBoxIsBusy()) {
			g_vm->addTimer(12409, 250);
			room->playSFX("r2230ed0", 12412);
			for (int i = 0; i < 12; i++) {
				int pos = _strongBoxTiles[i].getPosition();
				if (pos == 1 || pos == 2 || pos == 5 || pos == 6) {
					_strongBoxTiles[i].setRotationPhase(0);
				}
			}
			return;
		}

		if (name == "Potion") {
			room->disableHotzone("Potion");
			g_vm->getHeroBelt()->placeToInventory(kCoin, kCoinGiven);
			persistent->_creteStrongBoxState = Persistent::BOX_OPEN_NO_POTION;
			room->selectFrame("r2230bf0", 300, -1, kOffsetRightRoom);
			return;
		}
		/*
TODO:
 MNSH: Merchant
*/
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		switch (eventId) {
		case kTakenWood:
			room->stopAnim(kAtlantisOpening);
			room->stopAnim("r1010ob0");
			room->stopAnim(kAtlantisDiskBackground);
			room->enableMouse();
			break;
		case kTalueMovieCompleted:
			room->setLayerEnabled(kTalusImageWithShip, true);
			break;
		case kTavernMovieCompleted:
			room->selectFrame(kTavernImage, kTavernImageZ, 0, kOffsetRightRoom);
			break;
		case kMerchantIdleTimer:
			if (!persistent->_creteShowMerchant
			    || !persistent->_creteShowHornless4 || _merchantIsBusy)
				break;
			room->setLayerEnabled(kHornless4, false);
			room->disableHotzone(kHornless4Hotzone);
			room->playAnim(kMerchantAnim, kMerchantZ,
				       PlayAnimParams::disappear().partial(1, -2),
				       kMerchantIdleAnimCleanup);
			break;
		case kMerchantIdleAnimCleanup:
			room->selectFrame(kMerchantAnim, kMerchantZ, 0);
			room->setLayerEnabled(kHornless4, persistent->_creteShowHornless4);
			room->setHotzoneEnabled(kHornless4Hotzone, persistent->_creteShowHornless4 && persistent->_creteTriedHornless[2]);
			break;
		case kIntroMerchantPanFinished:
			room->setLayerEnabled(kMerchantAnim, false);
			room->playVideo("R2200BA0", kMerchantZ,
					kIntroMerchantVideoFinished,
					Common::Point(308, 99));
			break;
		// TODO: replay of R2200BB0 and R2200BC0
		case kIntroMerchantVideoFinished:
			room->disableMouse();
			room->playVideo("R2200BB0", kMerchantZ,
					12108,
					Common::Point(304, 113));
			_merchantIsBusy = true;
			break;
		case 12108:
			room->disableMouse();
			room->playVideo("R2200BC0", kMerchantZ,
					12109,
					Common::Point(304, 110));
			_merchantIsBusy = true;
			break;
		case 12109:
			room->selectFrame(kMerchantAnim, kMerchantZ, 0);
			room->enableMouse();
			_merchantIsBusy = false;
			break;
		case kTakenHorned:
			room->setLayerEnabled(kMerchantAnim, false);
			room->playVideo("r2210ba0", kMerchantZ, 12117,
					Common::Point(344, 111));
			room->disableMouse();
			break;
		case 12117:
			room->playVideo("r2210bb0", kMerchantZ, 12119,
				Common::Point(308, 112));
			break;
		case kTakenHornless:
			room->setLayerEnabled(kMerchantAnim, false);
			room->playVideo("r2240ba0", kMerchantZ, 12119,
				Common::Point(314, 91));
			_merchantIsBusy = true;
			break;
		case 12119:
			room->selectFrame(kMerchantAnim, kMerchantZ, 0);
			if (!persistent->_cretePlayedEyeGhostTown) {
				room->playVideo("r2210bc0", 1000, 12120,
						   Common::Point(0, 216));
				persistent->_cretePlayedEyeGhostTown = true;
			} else {
				room->enableMouse();
				_merchantIsBusy = false;
			}
			break;
		case 12120:
			room->selectFrame(kMerchantAnim, kMerchantZ, 0);
			room->enableMouse();
			_merchantIsBusy = false;
			break;
		case 12128:
		case 12134:
		case 12143:
		case 12146:
			room->enableMouse();
			break;
		case 12135:
			room->playAnimLoop(kOneManBandAnim, kOneManBandZ, kOffsetRightRoom);
			break;
		case 12142:
			if (persistent->_quest == kMedusaQuest && !persistent->_cretePlayedPhilAlchemist) {
				persistent->_cretePlayedPhilAlchemist = true;
				room->playVideo("r2220bc0", 1000, 12143, Common::Point(640, 216));
				room->disableMouse();
			}
			if (persistent->_quest == kRescuePhilQuest && !persistent->_cretePlayedZeusCheckOutThatBox && persistent->_hintsAreEnabled) {
				g_vm->addTimer(12144, 5000, -1);
			}
			break;
		case 12144:
			if (!room->isMouseEnabled() || persistent->_cretePlayedZeusCheckOutThatBox)
				break;
			persistent->_cretePlayedZeusCheckOutThatBox = true;
			room->disableMouse();
			room->playAnimWithSpeech("r2230ba0",
						TranscribedSound::make("r2230wa0",
								 "Hey, why don't you check out that box? Maybe it has something that can help"),
						4000,
						PlayAnimParams::keepLastFrame(),
						12145,
						kOffsetRightRoom);
			break;
		case kAtlantisBoatIntro2Finished:
			room->enableMouse();
			break;
		case kAtlantisDoorOpens:
			room->disableHotzone("LargeDisk");
			room->disableHotzone("MediumDisk");
			room->disableHotzone("SmallDisk");
			room->disableHotzone("Background");
			room->enableHotzone("wood");
			room->playVideo("r1210bd0", 600, 12308);
			break;
		case 12308:
			room->enableMouse();
			break;
		case kSoundVaseSegment1Finished:
		case kSoundVaseSegment2Finished:
		case kSoundVaseSegment3Finished:
		case kSoundVaseSegment4Finished: {
			int vase = eventId - kSoundVaseSegment1Finished;
			bool won = true;
			_vaseBusy[vase] = false;
			for (int i = 0; i < 4; i++) {
				if (_vaseBusy[i])
					won = false;
			}

			for (int i = 0; i < 4; i++) {
				if (_vasePos[i] != vaseSol[i])
					won = false;
			}

			if (!won)
				break;

			for (int i = 0; i < 4; i++)
				room->stopAnim(vaseSegment[i]);

			for (int i = 0; i < 4; i++)
				room->disableHotzone(Common::String::format("VaseSegment%d", i + 1));

			room->playAnim("r1220ba0", 500,
				       PlayAnimParams::disappear().partial(0, 17), 12206);
			room->playVideo("r1220mb0", 0);
			room->playSFX("r1220ea0");
			room->disableMouse();
			break;
		}
		case 12206:
			showSandals();
			persistent->_creteSandalsState = Persistent::SANDALS_SOLVED;
			room->enableMouse();
			break;
		case 12303:
		case 12304:
		case 12305:
			if (_atlantisBoatPosition[0] == 0
			    && _atlantisBoatPosition[1] == 0
			    && _atlantisBoatPosition[2] == 0) {
				handleEvent(12306);
			}
			break;
		case 12306:
			room->disableMouse();
			hideAtlantisPopupOverlays();
			room->playAnimWithSFX(kAtlantisOpening, "r1210eb0", kAtlantisOpeningZ,
						PlayAnimParams::keepLastFrame().partial(0, 8),
						kAtlantisDoorOpens);
			break;
		case 12402:
			room->enableMouse();
			redrawStrongBox();
			// Fallthrough
		case 12403:
			room->disableMouse();
			room->playAnimWithSpeech("r2230ba0",
						 TranscribedSound::make("r2230wb0", "I think those buttons control the symbols"),
						 4000,
						 PlayAnimParams::keepLastFrame(), 12404,
						 kOffsetRightRoom);
			break;
		case 12404:
		case 12405:
			if (persistent->_hintsAreEnabled)
				g_vm->addTimer(12406, 5000);
			// Fallthrough
		case 12407:
		case 12408:
		case 12145:
			room->playAnim("r2230ba0", 4000, PlayAnimParams::disappear().backwards(), -1,
				       kOffsetRightRoom);
			room->enableMouse();
			break;
		case 12406:
			if (!room->isMouseEnabled())
				break;
			room->disableMouse();
			room->playAnimWithSpeech("r2230ba0",
						 TranscribedSound::make("r2230wc0", "Try to spell my name"),
						 4000,
						 PlayAnimParams::keepLastFrame(), 12407,
						 kOffsetRightRoom);
			break;
		case 12409:
			for (int i = 0; i < 12; i++) {
				int pos = _strongBoxTiles[i].getPosition();
				if (pos == 1 || pos == 2 || pos == 5 || pos == 6) {
					_strongBoxTiles[i].setRotationPhase(1);
				}
			}
			g_vm->addTimer(12410, 250);
			break;
		case 12410:
			for (int i = 0; i < 12; i++) {
				int pos = _strongBoxTiles[i].getPosition();
				if (pos == 1 || pos == 2 || pos == 5 || pos == 6) {
					_strongBoxTiles[i].setRotationPhase(2);
					_strongBoxTiles[i].rotate();
					switch (pos) {
					case 1:
						_strongBoxTiles[i].setPosition(2);
						break;
					case 2:
						_strongBoxTiles[i].setPosition(6);
						break;
					case 5:
						_strongBoxTiles[i].setPosition(1);
						break;
					case 6:
						_strongBoxTiles[i].setPosition(5);
						break;
					}
				}
			}
			g_vm->addTimer(12411, 250);
			break;
		case 12411:
			for (int i = 0; i < 12; i++) {
				_strongBoxTiles[i].setRotationPhase(-1);
			}
			redrawStrongBox();
			break;
		case 12414:
			room->playVideo("v4190ma0", 0, 12415);
			break;
		case 12204:
		case 12415:
			room->enableMouse();
			break;
		case kCoinGiven:
			g_vm->getHeroBelt()->placeToInventory(kPotion);
			break;
		case 12124:
			_fadingHades = true;
			_fadingHadesStartTime = g_vm->getCurrentTime();
			break;
		case 12125:
			room->stopAnim("r2035ba0");
			room->playVideo("R2035BE0", 1200, 12127);
			break;
		case 12127:
			g_vm->moveToRoom(kDaedalusRoom);
			break;
		case kSandalsPlaced:
			room->playVideo("r1220bb0", 0, 12204);
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;

		if (persistent->_creteHadesPusnishesPainAndPanic) {
			room->disableHeroBelt();
			room->disableMouse();
			room->addStaticLayer("r2035pa0", kBackgroundZ); // background
			room->selectFrame("r2035ba0", 1200, 0);

			// Originally event 12123
			room->playSpeech(TranscribedSound::make(
						      "r2035wa0",
						      "Seems the only thing I can count on you two for is target practice"), 12124);
			persistent->_creteHadesPusnishesPainAndPanic = false;
			return;
		}
		
		room->loadHotZones("Crete.HOT", false);
		room->addStaticLayer("r1010pa0", kBackgroundZ); // background
		g_vm->getHeroBelt()->setColour(HeroBelt::kWarm);

		if (quest != kMedusaQuest && quest != kRescuePhilQuest) {
			room->playAnimLoop("r2010oe0", 7000, kOffsetRightRoom);
			room->playAnimLoop("r2010tb0", 6500, kOffsetRightRoom);
		}
		room->playAnimLoop("r2010ta0", 7600, kOffsetRightRoom);
		room->playAnimLoop("r1120ba0", 5500);
		room->playAnimLoop("r1160ba0", 5000);
		room->playAnimLoop("r1170ba0", 5000);
		
		room->selectFrame(kTavernImage, kTavernImageZ, 0, kOffsetRightRoom);
		room->enableMouse();
		room->setPannable(true);
		room->enableHotzone(kTavernHotzone);
		room->enableHotzone("Argo");
		room->enableHotzone("Hermes");
		room->enableHotzone("Zeus");
		room->enableHotzone("Poseidon");
		room->enableHotzone("MinosPalace");
		if (quest != kMedusaQuest && quest != kRescuePhilQuest) {
			room->enableHotzone("AlchemistStand");
		}

		room->setUserPanCallback(-1, -1, 12140, 12142);
		
		if (quest == kMedusaQuest && !persistent->_creteAlchemistExploded) {
			persistent->_creteAlchemistExploded = true;
			room->disableMouse();
			room->playAnimWithSFX("r1190ba0", "r1190ea0", 1005, PlayAnimParams::disappear(), 12128);
		}

		if (quest == kRescuePhilQuest || quest == kMedusaQuest) {
			room->selectFrame("r2010op0", 7500, 0, kOffsetRightRoom);
		}

		if (quest == kMedusaQuest) {
			room->playAnimWithSFX("r2220bb0", "r2220eb0", 4500, PlayAnimParams::loop(), -1, kOffsetRightRoom);
		}

		showMiniStrongBox();

		switch (quest) {
		case kCreteQuest:
			_tavernTalks.push_back("r2250xa0");
			_tavernTalks.push_back("r2250xb0");
			_tavernTalks.push_back("r2250xc0");
			_tavernTalks.push_back("r2250xd0");
			_tavernTalks.push_back("r2250xe0");
			_tavernTalks.push_back("r2250xf0");
			_tavernTalks.push_back("r2220xa0");
			_tavernTalks.push_back("r2220xb0");
			_tavernTalks.push_back("r2220xc0");
			_tavernTalks.push_back("r2220xd0");
			_tavernTalks.push_back("r2220xe0");
			_tavernTalks.push_back("r2220xf0");
			break;
		case kTroyQuest:
			_tavernTalks.push_back("r2260xa0");
			_tavernTalks.push_back("r2260xb0");
			_tavernTalks.push_back("r2195xa0");
			_tavernTalks.push_back("r2195xb0");
			_tavernTalks.push_back("r2290xf0");
			_tavernTalks.push_back("r2220xa0");
			_tavernTalks.push_back("r2220xb0");
			_tavernTalks.push_back("r2220xc0");
			_tavernTalks.push_back("r2220xd0");
			_tavernTalks.push_back("r2220xe0");
			_tavernTalks.push_back("r2220xf0");
			break;
		case kMedusaQuest:
			_tavernTalks.push_back("r2270xa0");
			_tavernTalks.push_back("r2270xb0");
			_tavernTalks.push_back("r2290xg0");
			_tavernTalks.push_back("r2290xh0");
			_tavernTalks.push_back("r2290xi0");
			_tavernTalks.push_back("r2290xj0");
			_tavernTalks.push_back("r2290xk0");
			_tavernTalks.push_back("r2290xl0");
			break;
		case kRescuePhilQuest:
			_tavernTalks.push_back("r2280xa0");
			_tavernTalks.push_back("r2280xc0");
			_tavernTalks.push_back("r2280xd0");
			_tavernTalks.push_back("r2290xg0");
			_tavernTalks.push_back("r2290xh0");
			_tavernTalks.push_back("r2290xi0");
			_tavernTalks.push_back("r2290xj0");
			_tavernTalks.push_back("r2290xk0");
			_tavernTalks.push_back("r2290xl0");
			break;

		// To silence warning
		case kNoQuest:
		case kEndGame:
		case kNumQuests:
			break;
		}
		_tavernTalks.push_back("r2290xa0");
		_tavernTalks.push_back("r2290xb0");
		_tavernTalks.push_back("r2290xc0");
		_tavernTalks.push_back("r2290xd0");
		_tavernTalks.push_back("r2290xe0");

		int bg1_variant = -1;
		int bg2_variant = -1;

		bool showOiBoat = false;

		if (!persistent->_creteShowMerchant && (quest != kMedusaQuest || !persistent->_medisleShowFates)) {
			bg1_variant = g_vm->getRnd().getRandomNumberRng(0, 2);
			bg2_variant = g_vm->getRnd().getRandomNumberRng(0, 2);
			debug("BG variants %d and %d", bg1_variant, bg2_variant);
		}

		if (randomBool()) {
			showOiBoat = true;
			room->addStaticLayer("r1010oi0", 4000);
		}
		if (randomBool())
			room->addStaticLayer("r1010oj0", 4000);
		if (randomBool())
			room->addStaticLayer("r1010ok0", 4000);
		if (randomBool())
			room->addStaticLayer("r1010ol0", 4000);
		if (randomBool())
			room->addStaticLayer("r1010on0", 4001);
		if (randomBool())
			room->addStaticLayer("r1010oo0", 4000);
		if (randomBool())
			room->addStaticLayer("r1010op0", 4100);
		if (randomBool())
			room->addStaticLayer("r1010or0", 6500);

		if (quest != kCreteQuest)
			persistent->_creteShowAtlantisBoat = false;

		if (persistent->_creteShowAtlantisBoat) {
			room->addStaticLayer("r1010od0", 3900);
			room->addStaticLayer("r1010ta0", 3800);
			room->enableHotzone("AtlantisBoat");
		}

		room->playAnimLoop("r1010om0", 3000);

		int minotaurPosition = -1;
		int toughGuyPosition = -1;
		int womanSmellPosition = -1;
		if (quest == kCreteQuest && !persistent->_creteShowMerchant && !persistent->_creteShowAtlantisBoat)
			minotaurPosition = g_vm->getRnd().getRandomNumberRng(0, 3);

		bool showCat = false;
		bool showStatueMan = false;
		bool showOldMan = false;
		bool showBlondBoy = false;
		bool showMotherAndKidSmall = false;
		bool showMotherAndKid = false;
		bool showManSelling = false;
		bool showDrawinWithoutWoman = false;
		bool showWomanDrawing = false;
		bool showWomanGraffiti = false;
		bool showGirlAndNut = false;
		bool showOneManBand = false;
		int ambientComposite = g_vm->getRnd().getRandomNumberRng(1, 8);
		debug("Ambients from Composite %d selected. \n", ambientComposite);
		switch (ambientComposite) {
		case 1:
			showGirlAndNut = true;
			if (quest != kCreteQuest && quest != kTroyQuest) {
				toughGuyPosition = 1;
				break;
			}
			if (minotaurPosition == 1) {
				if (randomBool())
					toughGuyPosition = 0;
				break;

			}
			if (persistent->_creteShowMerchant) {
				showOneManBand = true;
				showWomanGraffiti = true;
				break;
			}
			if (randomBool()) {
				toughGuyPosition = 0;
				showOneManBand = true;
				showWomanGraffiti = true;
			}
			else
				toughGuyPosition = 1;
			break;
		case 2:
			womanSmellPosition = g_vm->getRnd().getRandomNumberRng(0, 2);
			showWomanDrawing = true;
			showManSelling = true;
			if (quest != kMedusaQuest && quest != kRescuePhilQuest)
				showMotherAndKid = true;
			showBlondBoy = true;
			break;
		case 3:
			if (quest != kRescuePhilQuest && bg1_variant != 0 && bg1_variant != 1)
				showCat = true;
			showOldMan = true;
			if (!persistent->_creteShowMerchant) {
				showManSelling = true;
				toughGuyPosition = 2;
			}
			showWomanGraffiti = true;
			showWomanDrawing = true;
			break;
		case 4:
			if (quest != kRescuePhilQuest && bg1_variant != 0 && bg1_variant != 1)
				showCat = true;
			showWomanDrawing = true;
			toughGuyPosition = 2;
			showOneManBand = true;
			break;
		case 5:
			if (quest != kCreteQuest && quest != kTroyQuest)
				toughGuyPosition = 1;
			else
				toughGuyPosition = g_vm->getRnd().getRandomBit();
			womanSmellPosition = g_vm->getRnd().getRandomBit();
			break;
		case 6:
			showWomanGraffiti = true;
			showWomanDrawing = true;
			if (quest != kCreteQuest && quest != kTroyQuest)
				toughGuyPosition = 1;
			else 
				toughGuyPosition = g_vm->getRnd().getRandomBit();
			break;
		case 7:
			womanSmellPosition = g_vm->getRnd().getRandomNumberRng(0, 2);
			if (womanSmellPosition == 2 && (quest != kCreteQuest && quest != kTroyQuest))
				womanSmellPosition = -1;
			if (quest == kMedusaQuest)
				showStatueMan = true;
			showMotherAndKidSmall = true;
			showWomanGraffiti = true;
			break;
		case 8:
			if (quest != kCreteQuest && quest != kTroyQuest) {
				toughGuyPosition = 1;
				showDrawinWithoutWoman = true;
				break;
			}

			if (!persistent->_creteShowMerchant && randomBool()) {
				toughGuyPosition = 2;
				showBlondBoy = true;
			}
			else
				toughGuyPosition = 1;
			showOldMan = true;
			showDrawinWithoutWoman = true;
			break;
		}

		int birdsState = g_vm->getRnd().getRandomNumberRng(0, 2);
		int dolphinPosition = -1;
		if (!persistent->_creteShowAtlantisBoat)
			dolphinPosition = g_vm->getRnd().getRandomNumberRng(0, showOiBoat ? 1 : 2);
		if (quest != kMedusaQuest && quest != kRescuePhilQuest)
			_alchemistAmbient = ambient("r2220ba0", "r2220ec0", 5500, 15, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom, true);

		// Disable conflicting images
		if (minotaurPosition == 1
		    && (toughGuyPosition == 1 || toughGuyPosition == 2))
			toughGuyPosition = -1;
		if (minotaurPosition == 1) {
			showWomanGraffiti = false;
			showManSelling = false;
			showOneManBand = false;
		}
		if (persistent->_creteShowMerchant &&
		    (toughGuyPosition == 0 || toughGuyPosition == 1))
			toughGuyPosition = -1;
		if (womanSmellPosition == 2)
			showMotherAndKid = false;
		if (persistent->_creteShowMerchant)
			showCat = false;
		if (quest != kCreteQuest && quest != kTroyQuest)
			showOldMan = false;

		debug("toughGuyPosition = %d, minotaurPosition = %d, dolphinPosition = %d",
		      toughGuyPosition, minotaurPosition, dolphinPosition);
		if (showWomanGraffiti)
			ambient("r2370ba0", "r2370ea0", 1700, 15, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
		switch (toughGuyPosition) {
		case 0:
			ambient("r2340ba0", "r2340ea0", 1400, 15, 25, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
			break;
		case 1:
			ambient("r2340bb0", "r2340eb0", 1400, 15, 25, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
			break;
		case 2:
			ambient("r2340be0", "r2340ee0", 1600, 15, 25, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
			break;
		}
		if (showOneManBand) {
			room->playAnimLoop(kOneManBandAnim, kOneManBandZ, kOffsetRightRoom);
			room->enableHotzone(kOneManBandHotZone);
		}
		if (showGirlAndNut)
			ambient("r2140ba0", "r2140ea0", 1500, 15, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
		if (showWomanDrawing)
			ambient("r2110ba0", "r2110ea0", 1600, 10, 30, AmbientAnim::PAN_LEFT);
		if (showDrawinWithoutWoman)
			room->addStaticLayer("r2110bb0", 1600);
		if (showManSelling)
			ambient("r2150ba0", "r2150ea0", 1600, 10, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
		if (showMotherAndKid)
			ambient("r2160ba0", "r2160ea0", 1500, 15, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
		if (showMotherAndKidSmall)
			ambient("r2160bb0", "", 1500, 15, 30, AmbientAnim::PAN_LEFT);
		if (showBlondBoy)
			ambient("r2350ba0", "r2350ea0", 1500, 15, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
		if (showOldMan)
			ambient("r2100bb0", "r2100eb0", 1500, 10, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
		if (showStatueMan)
			ambient("r2050ba0", "r2050ea0", 1500, 10, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
		if (showCat) {
			room->addStaticLayer("r2010on0", kMerchantStandZ);
			ambient("r2080bb0", "r2080eb0", kMerchantZ, 10, 30, AmbientAnim::PAN_LEFT);
		}

		if (persistent->_creteShowMerchant) {
			room->addStaticLayer("r2010on0", kMerchantStandZ);
			room->selectFrame(kMerchantAnim, kMerchantZ, 0);
			room->playSFX("G0261mA0");
			if (persistent->_creteShowHorned) {
				room->addStaticLayer(kHorned, 1220);
				room->enableHotzone(kHornedHotzone);
			}
			if (persistent->_creteShowHornless2) {
				room->addStaticLayer(kHornless2, 1190);
				room->setHotzoneEnabled(kHornless2Hotzone,
					persistent->_creteTriedHornless[0]);
			}
			if (persistent->_creteShowHornless1) {
				room->addStaticLayer(kHornless1, 1180);
				room->enableHotzone(kHornless1Hotzone);
			}
			if (persistent->_creteShowHornless3) {
				room->addStaticLayer(kHornless3, 1180);
				room->setHotzoneEnabled(kHornless3Hotzone,
					persistent->_creteTriedHornless[1]);
			}
			if (persistent->_creteShowHornless4) {
				room->addStaticLayer(kHornless4, 1180);
				room->setHotzoneEnabled(kHornless4Hotzone,
					persistent->_creteTriedHornless[2]);
				g_vm->addTimer(kMerchantIdleTimer, 10000, -1);
			}
		}

		if (birdsState == 0) {
			room->addStaticLayer(kTalusImageWithShip, kTalusZ);
			room->enableHotzone(kTalusHotzone);
		} else
			room->addStaticLayer(kTalusImage, kTalusZ);

		// TODO: fix this
		switch (minotaurPosition) {
		case 0:
			ambient("r2060bf0", "r2060ee0", 1800, 3, 4, AmbientAnim::PAN_RIGHT, kOffsetRightRoom); // shouldn't be an ambient
			break;
		case 1:
			ambient("r2060bt0", "r2060ei0", 1650, 3, 4, AmbientAnim::PAN_RIGHT, kOffsetRightRoom); // shouldn't be an ambient
			break;
		case 2:
//			ambient("r1110ea0", "", 15, 30); // wrong
			break;
		case 3:
			ambient("r1110ba0", "r1110eb0", 850, 20, 40, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
			break;
		}

		switch (womanSmellPosition) {
		case 0:
			ambient("r2380ba0", "r2380ea0", 1550, 15, 30, AmbientAnim::PAN_ANY);
			break;
		case 1:
			ambient("r2380bc0", "r2380ec0", 1550, 15, 30, AmbientAnim::PAN_ANY);
			break;
		case 2:
			ambient("r2380bd0", "r2380ed0", 1550, 15, 30, AmbientAnim::PAN_RIGHT, kOffsetRightRoom);
			break;
		}

		switch (birdsState) {
		case 0:
			ambient("r1060ba0", "r1060ea0", 4500, 10, 40, AmbientAnim::PAN_ANY);
			break;
		case 1:
			ambient("r1060bb0","r1060eb0", 4500, 5, 40, AmbientAnim::PAN_ANY);
			break;
		case 2:
			ambient("r1060bc0", "r1060ec0", 4500, 5, 40, AmbientAnim::PAN_ANY);
			break;
		}

		if (dolphinPosition >= 0) {
			Common::String dolphinAnim = Common::String::format(
				"r1150b%c0", 'a' + dolphinPosition);
			Common::String dolphinSound = Common::String::format(
				"r1150e%c0", 'a' + dolphinPosition);

			ambient(dolphinAnim, dolphinSound, 5500, 5, 20, AmbientAnim::PAN_LEFT);
		}

		if (bg1_variant >= 0 && bg2_variant >= 0) {
			Common::String bg2 = Common::String::format(
				"r1010o%c1", 'e' + bg2_variant);
			room->addStaticLayer(bg2, 3500);

			Common::String bg1 = Common::String::format(
				"r1010o%c0", 'e' + bg1_variant);
			room->addStaticLayer(bg1, 1000);
		}

		room->playMusicLoop("R1010eA0");

		if (g_vm->getPreviousRoomId() == kMinosPalaceRoom) {
			room->panRightInstant();
			if (persistent->_creteIntroMerchant) {
				room->disableMouse();
				room->panLeftAnim(kIntroMerchantPanFinished);
				_merchantIsBusy = true;
				persistent->_creteIntroMerchant = false;
			}

			if (persistent->_creteIntroAtlantisBoat) {
				persistent->_creteIntroAtlantisBoat = false;
				room->disableMouse();
				room->playVideo("r1180ba0", 0, 12134, Common::Point(640, 216));
			}

		}

		if (quest == kMedusaQuest && persistent->_medisleShowFates) {
			switch (persistent->_creteSandalsState) {
			case Persistent::SANDALS_NOT_SOLVED:
				for (int i = 0; i < 4; i++)
					room->enableHotzone(Common::String::format("VaseSegment%d", i + 1));
				_vasePos[0] = (g_vm->getRnd().getRandomNumberRng(1, 3) + 2) % 4;
				_vasePos[1] = (g_vm->getRnd().getRandomNumberRng(1, 5) + 3) % 6;
				_vasePos[2] = (g_vm->getRnd().getRandomNumberRng(1, 5) + 2) % 6;
				_vasePos[3] = (g_vm->getRnd().getRandomNumberRng(1, 3) + 3) % 4;
				renderVase();
				break;
			case Persistent::SANDALS_SOLVED:
				showSandals();
				break;
			case Persistent::SANDALS_TAKEN:
				room->selectFrame("r1220ba0", 500, 0);
				break;
			}
		}

		switch (persistent->_quest) {
		case kCreteQuest:
			if (!persistent->_roomVisited[kMinosPalaceRoom]) {
				room->playVideo("r1260ma0", 0);
				break;
			}

			if (persistent->_creteShowMerchant) {
				room->playVideo("g0261ma0", 0);
				break;
			}
			break;
		case kMedusaQuest:
			if (persistent->_medisleShowFates && persistent->_creteSandalsState == Persistent::SANDALS_NOT_SOLVED)
				room->playVideo("r1220ma0", 0);
			break;
		case kRescuePhilQuest:
			if (persistent->_creteStrongBoxState == Persistent::BOX_CLOSED || persistent->_creteStrongBoxState == Persistent::BOX_OPEN)
				room->playVideo("r2230ma0", 0);
			break;
		// To silence warning
		case kTroyQuest:
		case kNoQuest:
		case kEndGame:
		case kNumQuests:
			break;
		}

		if (!persistent->_creteSaidHelenPermanentResident && persistent->_quest == kTroyQuest) {
			persistent->_creteSaidHelenPermanentResident = true;
			room->disableMouse();
			room->playVideo("r1250ba0", 0, 12134, Common::Point(0, 216));
		}
	}

	void frameCallback() override {
		if (_strongBoxPopup && strongBoxIsBusy())
			redrawStrongBox();
		if (_fadingHades) {
			Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
			int val = (256 * (g_vm->getCurrentTime() - _fadingHadesStartTime)) / 2000;
			if (val >= 256) {
				_fadingHades = false;
				val = 256;
				handleEvent(12125);
			}
			room->setColorScale("r2035pa0", 256 - val);
		}
	}

private:
	void showMiniStrongBox() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (persistent->_quest != kRescuePhilQuest)
			return;

		switch(persistent->_creteStrongBoxState) {
		case Persistent::BOX_CLOSED:
			room->enableHotzone("StrongBoxClosed");
			room->selectFrame("r2010ba0", 5000, 0, kOffsetRightRoom);
			break;
		case Persistent::BOX_OPEN:
			room->enableHotzone("StrongBoxOpen");
			room->selectFrame("r2010ba0", 5000, 1, kOffsetRightRoom);
			break;
		case Persistent::BOX_OPEN_POTION:
			room->enableHotzone("StrongBoxOpenPotion");
			room->selectFrame("r2010ba0", 5000, 2, kOffsetRightRoom);
			break;
		case Persistent::BOX_OPEN_NO_POTION:
			room->enableHotzone("StrongBoxOpenNoPotion");
			room->selectFrame("r2010ba0", 5000, 3, kOffsetRightRoom);
			break;
		}
	}

	AmbientAnim ambient(const Common::String &anim, const Common::String &sound,
			    int zValue, int minint, int maxint,
			    AmbientAnim::PanType pan,
			    Common::Point offset = Common::Point(0,0),
			    bool loop = true) {
		AmbientAnim ret = AmbientAnim(anim, sound, zValue, minint * 1000, maxint * 1000,
					      loop ? AmbientAnim::KEEP_LOOP : AmbientAnim::DISAPPEAR,
					      offset, pan);
		ret.start();
		return ret;
	}

	bool randomBool() const {
		return g_vm->getRnd().getRandomBit();
	}

	void renderAtlantisDisks() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->selectFrame(kAtlantisLargeDisk, kAtlantisLargeDiskZ, _atlantisBoatPosition[0]);
		room->selectFrame(kAtlantisMediumDisk, kAtlantisMediumDiskZ, _atlantisBoatPosition[1]);
		room->selectFrame(kAtlantisSmallDisk, kAtlantisSmallDiskZ, _atlantisBoatPosition[2]);
	}

	void advanceAtlantisDisk(int diskNum) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_atlantisBoatPosition[diskNum] = (_atlantisBoatPosition[diskNum] + 1) % 8;
		renderAtlantisDisks();
		room->playSFX(Common::String::format("r1210e%c0", 'e' + diskNum), 12303 + diskNum);
	}

	void hideAtlantisPopupOverlays() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->stopAnim(kAtlantisLargeDisk);
		room->stopAnim(kAtlantisMediumDisk);
		room->stopAnim(kAtlantisSmallDisk);
		room->stopAnim(kAtlantisDiskBackground);
	}

	void renderVase() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for(int i = 0; i < 4; i++)
			room->selectFrame(vaseSegment[i], 1000, _vasePos[i]);
			
	}

	void showSandals() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnim("r1220ba0", 500, PlayAnimParams::loop().partial(9, 17));
		room->enableHotzone("sandals");
	}

	void showStrongBox() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		// User already clicked, no need to play that anim
		persistent->_cretePlayedZeusCheckOutThatBox = true;
		room->pushHotZones("Box.Hot");
		room->playSFX("g0082ea0");
		redrawStrongBox();
		_strongBoxPopup = true;
		switch(persistent->_creteStrongBoxState) {
		case Persistent::BOX_CLOSED:
			room->selectFrame("r2230bb0", 1000, 1, kOffsetRightRoom);
			break;
		case Persistent::BOX_OPEN:
			room->selectFrame("r2230bb0", 1000, 3, kOffsetRightRoom);
			break;
		case Persistent::BOX_OPEN_POTION:
			room->selectFrame("r2230bb0", 1000, 3, kOffsetRightRoom);
			room->selectFrame("r2230bf0", 300, -2, kOffsetRightRoom);
			break;
		case Persistent::BOX_OPEN_NO_POTION:
			room->selectFrame("r2230bb0", 1000, 3, kOffsetRightRoom);
			room->selectFrame("r2230bf0", 300, -1, kOffsetRightRoom);
			break;
		}
	}

	void hideStrongBox() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->stopAnim("r2230bb0");
		room->stopAnim("r2230bf0");
		for (int i = 0; i < 12; i++)
			room->stopAnim(LayerId("r2010om0", i, "pos"));
		room->stopAnim(LayerId("r2010om1", 0, "center"));
		for (int i = 0; i < 4; i++) {
			room->stopAnim(LayerId("r2010om1", i, "side"));
		}
		_strongBoxPopup = false;
	}

	void redrawStrongBox() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		
		room->setHotzoneEnabled("Potion", persistent->_creteStrongBoxState == Persistent::BOX_OPEN_POTION);
		room->setHotzoneEnabled("Latch", persistent->_creteStrongBoxState == Persistent::BOX_CLOSED);
		room->setHotzoneEnabled("ButtonN", persistent->_creteStrongBoxState == Persistent::BOX_OPEN);
		room->setHotzoneEnabled("ButtonS", persistent->_creteStrongBoxState == Persistent::BOX_OPEN);
		room->setHotzoneEnabled("ButtonE", persistent->_creteStrongBoxState == Persistent::BOX_OPEN);
		room->setHotzoneEnabled("ButtonW", persistent->_creteStrongBoxState == Persistent::BOX_OPEN);
		room->setHotzoneEnabled("ButtonC", persistent->_creteStrongBoxState == Persistent::BOX_OPEN);

		switch(persistent->_creteStrongBoxState) {
		case Persistent::BOX_CLOSED:
			break;
		case Persistent::BOX_OPEN:
			room->selectFrame(LayerId("r2010om1", 0, "center"), 400, 0, Common::Point(1060, 308));
			for (int i = 0; i < 4; i++) {
				room->selectFrame(LayerId("r2010om1", i, "side"), 400, 1,
						  strongBoxSideDotOffsets[i].get());
			}
			{
				bool wasMoving = strongBoxIsBusy();
				for (int i = 0; i < 12; i++) {
					_strongBoxTiles[i].show();
				}
				if (wasMoving && !strongBoxIsBusy()) {
					strongBoxCheckSolution();
				}
			}
			break;
		case Persistent::BOX_OPEN_POTION:
		case Persistent::BOX_OPEN_NO_POTION:
			room->selectFrame(LayerId("r2010om1", 0, "center"), 400, 0, Common::Point(1060, 308));
			for (int i = 0; i < 4; i++) {
				room->selectFrame(LayerId("r2010om1", i, "side"), 400, 1,
						  strongBoxSideDotOffsets[i].get());
			}
			for (int i = 0; i < 12; i++) {
				_strongBoxTiles[i].show();
			}
			break;
		}
	}

	bool strongBoxIsBusy() {
		for (int i = 0; i < 12; i++) {
			if (_strongBoxTiles[i].isMoving()) {
				return true;
			}
		}
		return false;
	}

	void strongBoxCheckSolution() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		bool zOk = false, eOk = false, uOk = false, sOk = false;
		for (int i = 0; i < 12; i++) {
			if (_strongBoxTiles[i].getPosition() == 0
			    && _strongBoxTiles[i].getLetter() == StrongBoxTile::kLetterZ
			    && (_strongBoxTiles[i].getOrientation() == 0
				|| _strongBoxTiles[i].getOrientation() == 180))
				zOk = true;
			if (_strongBoxTiles[i].getPosition() == 1
			    && _strongBoxTiles[i].getLetter() == StrongBoxTile::kLetterE
			    && _strongBoxTiles[i].getOrientation() == 0)
				eOk = true;
			if (_strongBoxTiles[i].getPosition() == 2
			    && _strongBoxTiles[i].getLetter() == StrongBoxTile::kLetterU
			    && _strongBoxTiles[i].getOrientation() == 0)
				uOk = true;
			if (_strongBoxTiles[i].getPosition() == 3
			    && _strongBoxTiles[i].getLetter() == StrongBoxTile::kLetterS
			    && (_strongBoxTiles[i].getOrientation() == 0
				|| _strongBoxTiles[i].getOrientation() == 180))
				sOk = true;
		}

		if (zOk && eOk && uOk && sOk) {
			persistent->_creteStrongBoxState = Persistent::BOX_OPEN_POTION;
			room->disableMouse();
			room->enableHotzone("Potion");
			room->disableHotzone("ButtonS");
			room->disableHotzone("ButtonN");
			room->disableHotzone("ButtonE");
			room->disableHotzone("ButtonW");
			room->disableHotzone("ButtonC");
			room->playAnimWithSFX("r2230bf0", "r2230ea0", 300,
					      PlayAnimParams::keepLastFrame().partial(0, -2),
					      12414, kOffsetRightRoom);
		}
	}

	void strongBoxMoveTiles(int p1, int p2, int p3, int p4) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playSFX("r2230ee0");
		for (int i = 0; i < 12; i++) {
			int pos = _strongBoxTiles[i].getPosition();
			if (pos == p1)
				_strongBoxTiles[i].moveTo(p2);
			if (pos == p2)
				_strongBoxTiles[i].moveTo(p3);
			if (pos == p3)
				_strongBoxTiles[i].moveTo(p4);
			if (pos == p4)
				_strongBoxTiles[i].moveTo(p1);
		}
	}

	bool _fadingHades;
	int _fadingHadesStartTime;
	int _tavernCounter;
	int _oneManBandCounter;
	int _atlantisBoatPosition[3];
	bool _merchantIsBusy;
	int _vasePos[4];
	bool _vaseBusy[4];
	bool _strongBoxPopup;
	StrongBoxTile _strongBoxTiles[12];
	Common::Array<Common::String> _tavernTalks;
	AmbientAnim _alchemistAmbient;
};

Common::SharedPtr<Hadesch::Handler> makeCreteHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new CreteHandler());
}

}
