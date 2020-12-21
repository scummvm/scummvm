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

static const char *kStoneAnim = "g0110ob0";
static const char *kStoneHotzone = "stone";
static const char *snakes[] = {
	"m1220bb0",
	"m1210bb0",
	"m1190bb0",
	"m1180bb0"
};

static const char *greenSnakes[] = {
	"m1160bd0",
	"m1160bc0",
	"m1160bb0",
	"m1160ba0"
};

static const char *itemImages[] = {
	"m1010bb0",
	"m1010bc0",
	"m1010ba0",
	"m1010bd0",
	"m1010be0"
};

static const char *itemImagesGlow[] = {
	"m1010bb1",
	"m1010bc1",
	"m1010ba1",
	"m1010bd1",
	"m1010be1"
};

static const char *itemSounds[] = {
	"m1190ea0",
	"m1180ec0",
	"m1220ea0",
	"m1210ea0",
	"m1230ea0"
};

static const TranscribedSound itemClickSounds[] = {
	{"m1150ne0", _s("If Perseus looks directly at Medusa he will be turned to stone. Using the shield from my temple, he can look at her reflection instead") },
	{"m1150nd0", _s("The sword from my temple will cut off Medusa's head") },
	{"m1150na0", _s("Medusa's head must be brough back inside this magic bag") },
	{"m1150nb0", _s("When Perseus wears the dark helmet, he will be invisible") },
	{"m1150nc0", _s("Putting on the winged sandals will allow you to fly above and move quickly") }
};

static const char *perseusItemAnims[] = {
	"m1190ba0",
	"m1180ba0",
	"m1220ba0",
	"m1210ba0",
	"m1230ba0"
};

static const TranscribedSound perseusItemSounds[] = {
	{ "m1190na0", _s("Thanks. The shield is just what I needed") },
	{ "m1180na0", _s("Yeah. The sword oughta do the trick") },
	{ "m1220na0", _s("Alright. You've got the bag. Excellent") },
	{ "m1210na0", _s("Yes, the helmet. You're doing great") },
	{ "m1230na0", _s("Ok, you've found the sandals. I owe you big time") }
};

static const TranscribedSound eyeInsistSpeech[] = {
	{"m2120wd0", _s("No, no. Where did he go?")},
	{"m2120we0", _s("Feel around here. It's got to be near")},
	{"m2120wf0", _s("If it's lost I'd cry but I don't have an eye")}
};

struct StatuePiece {
	const char *name;
	const char *fullElement;
	const char *emptyElement;
	int statuex;
	int statuey;
	int statuez;
	int brokenx;
	int brokeny;
	int hotspotx;
	int hotspoty;
	int dep1;
	int dep2;

	Common::Point getStatueOffset() const {
		return Common::Point(statuex, statuey);
	}

	Common::Point getBrokenOffset() const {
		return Common::Point(brokenx, brokeny);
	}

	Common::Point getHotspot() const {
		return Common::Point(hotspotx, hotspoty);
	}
};
    
static const StatuePiece statuePieces[] = {
	{"LowerLeg1", "m1010or0", "m1010or2", 519, 282, 2009, 461, 395, 26, 28, -1, -1 },
	{"LowerLeg2", "m1010or1", "m1010or3", 571, 276, 2008, 339, 358, 15, 30, -1, -1 },
	{"UpperLeg1", "m1010ov0", "m1010ov1", 528, 225, 2010, 590, 356, 11, 24,  0, -1 },
	{"UpperLeg2", "m1010ow0", "m1010ow1", 547, 225, 2006, 298, 415, 15, 25,  1, -1 },
	{"Torso"    , "m1010os0", "m1010os1", 518, 159, 2007, 517, 407, 23, 26,  2,  3 },
	{"UpperArm1", "m1010ot0", "m1010ot1", 500, 161, 2005, 582, 453, 13, 10,  4, -1 },
	{"UpperArm2", "m1010ou0", "m1010ou1", 563, 173, 2004, 381, 362, 10, 13,  4, -1 },
	{"LowerArm1", "m1010op0", "m1010op1", 471, 132, 2002, 483, 357, 20, 24,  5, -1 },
	{"LowerArm2", "m1010oq0", "m1010oq1", 565, 193, 2001, 426, 360, 25, 35,  6, -1 },
	{"Head"     , "m1010oo0", "m1010oo1", 539, 107, 2003, 406, 307, 24, 32,  4, -1 }

};

static const char *fatesHotzoneNames[kNumFates] = {
	"Lachesis",
	"Atropos",
	"Clotho"
};

static const char *itemNames[] = {
	"shield",
	"sword",
	"bag",
	"helmet",
	"sandals"
};

static const struct {
	const char *image;
	int minint, maxint;
	int zVal;
} ambientsLeft[] = {
	{ "m1030ba0", 10000, 40000, 4000 },
	{ "m1320ba0", 15000, 30000, 4000 },
	{ "m1320bb0", 10000, 50000, 4000 },
	{ "m1320bc0", 5000, 20000, 4000 },
	{ "m1330ba0", 10000, 40000, 4000 },
	{ "m1330bb0", 5000, 20000, 4000 },
	{ "m1330bc0", 5000, 30000, 4000 },
	{ "m1340ba0", 5000, 20000, 4000 },
	{ "m1340bb0", 15000, 30000, 4000 },
	{ "m1340bc0", 5000, 20000, 4000 }
};

static const struct {
	const char *image;
	const char *sound;
	int minint, maxint;
	int zVal;
	int parallax;
	bool keep;
} ambientsLeftSnakesAndRats[] = {
	{ "m1090ba0", "m1090ea0", 5000, 45000, 300, -200, true },
	{ "m1310ba0", "m1310ea0", 5000, 45000, 300, -200, true },
	{ "m1100ba0", "m1100ea0", 5000, 45000, 4100, 0, true },
	{ "m1300ba0", "m1300ea0", 5000, 45000, 4100, 0, true },
	{ "m1060ba0", "m1060ea0", 5000, 50000, 4200, 0, false },
	{ "m1280ba0", "m1280ea0", 5000, 50000, 1500, -200, false }
};

static const TranscribedSound kICanForetellTheFuture = {
	"m2190wa0",
	_s("I can foretell the future. You know, I always knew I was going to say that")
};

static const TranscribedSound kGiveMeEyeOrIllPunchYouInTheEye = {
	"m2190wb0",
	_s("Give me the eye or I'll punch you in the eye")
};

static const TranscribedSound kPunchingOwnEye = {
	"m2190wc0",
	_s("Oh yeah? Then you'll be punching your own eye")
};

static const TranscribedSound kIllTakeTheEyeYouCanHaveOneOfMyEars = {
	"m2190wd0",
	_s("I'll take the eye you can have one of my ears")
};

static const TranscribedSound kMedusaOnceBeauty = {
	"m2210wa0",
	_s("Medusa once was quite the beauty. Also vain and very snoody. Treated Athena awful bad and mad the goddess awfully mad. To snab the god is all it takes and now her hair is made of snakes")
}; // unclear

static const struct {
	const char *image;
	int minint, maxint;
	int zVal;
} ambientsRight[] = {
	{"m2060ba0", 5000, 40000, 4000},
	{"m2060bb0", 10000, 30000, 4000},
	{"m2280ba0", 5000, 20000, 2000},
	{"m2270ba0", 3000, 20000, 4000}
};

enum {
	kStoneTakenCleanup = 11050,
	kStoneTaken = 1011050
};

enum {
	kBackgroundZ = 10000,
	kStatuesZVal2 = 3000,
	kFatesZ = 1500
};

enum {
	kLoopFatesShadow = 1011001
};

class MedIsleHandler : public Handler {
public:
	MedIsleHandler() {
		_eyeInsistCounter = 0;
		_eyeIsGivenBack = false;
		_eyeIsPickedUp = false;
		_fatesShadowIsActive = false;
		_isFirstFates = false;
		_statueDrag = -1;
		_depProblemState = 0;
		_fatesAreBusy = false;
		_lastClickedItem = -1;
		_hintsCounter = 0;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;

		for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
			if (name == Common::String("D") + statuePieces[i].name) {
				statueDClick(i);
				return;
			}
			if (name == Common::String("S") + statuePieces[i].name) {
				statueSClick(i);
				return;
			}
		}

		if (_statueDrag >= 0) {
			return;
		}
		if (name == kStoneHotzone) {
			room->stopAnim(kStoneAnim);
			g_vm->getHeroBelt()->placeToInventory(kStone, kStoneTaken);
			room->disableHotzone(kStoneHotzone);
			room->playMusic("m1360ma0");
			persistent->_medisleStoneTaken = true;
			room->disableMouse();
			return;
		}

		if (name == "Argo") {
			g_vm->moveToRoom(kArgoRoom);
			return;
		}

		if (name == "Eyeball") {
			room->drag("m2010oa0", 0, Common::Point(21, 20));
			room->stopAnim("m2010oa0");
			room->disableMouse();
			room->playSFX("m2130ea0", 11027);
			_eyeIsPickedUp = true;
			persistent->_medisleEyePosition = kNumFates;
			return;
		}

		if (_eyeIsGivenBack) {
			static const int nextEvent[] = {
				11621, 11623, 11626
			};
			for (FateId i = kLachesis; i < kNumFates; i = (FateId) (i + 1)) {
				if (name == fatesHotzoneNames[i]) {
					moveEye(i, nextEvent[i]);
					return;
				}
			}
		}

		if (_eyeIsPickedUp && !_eyeIsGivenBack) {
			for (FateId i = kLachesis; i < kNumFates; i = (FateId) (i + 1))
				if (name == fatesHotzoneNames[i]) {
					persistent->_medisleEyePosition = i;
					room->disableMouse();
					room->playSFX("m2130ee0", 11029);
					room->disableHotzone("Eyeball");
					room->clearDrag();
					_eyeIsGivenBack = true;
					_isFirstFates = true;
					return;
				}
		}

		if (name == "MagicBag") {
			hideMagicBag();
			persistent->_medisleBagPuzzleState = Persistent::BAG_TAKEN;
			g_vm->getHeroBelt()->placeToInventory(kBag);
			renderFatesAll();
			return;
		}

		for (int i = 0; i < 5; i++) {
			if (name == itemNames[i]) {
				itemGlow(i);
				return;
			}
		}
		
		if (name == "FatesLair") {
			if (showAllFates())
				return;
			room->disableMouse();
			if (showNoFates()) {
				_hintsCounter++;
				if (_hintsCounter == 1 && persistent->_medislePlayedPhilFatesDesc)
					_hintsCounter = 2;
				if (_hintsCounter == 1) {
					if (quest > kMedusaQuest || (quest == kMedusaQuest && persistent->_medisleShowFates)) {
						playFatesLairBackupSound();
						return;
					}
					persistent->_medislePlayedPhilFatesDesc = true;
					room->playVideo("m2210ba0", 0, 11049, Common::Point(640, 216));
					return;
				}

				Common::Array <TranscribedSound> hints;

				switch (quest) {
				case kCreteQuest:
					hints.push_back(TranscribedSound::make("m2220wa0", "The Minotaur's is where you need to be. Trapping him sets Daedalus free."));
					hints.push_back(TranscribedSound::make("m2220wb0", "King Minos is the king of Crete. He's not the nicest man you meet."));
					hints.push_back(TranscribedSound::make("m2220wc0", "The Minotaur is hard to beat. The one who's trying lives in Crete"));
					break;
				case kTroyQuest:
					hints.push_back(TranscribedSound::make("m2230wa0", "If Helen gets this note, our friend, the trojan war will quickly end."));
					hints.push_back(TranscribedSound::make("m2230wb0", "Helen was a husband's joy but she's a prisonner inside Troy."));
					hints.push_back(TranscribedSound::make("m2230wc0", "The trojan horse you cannot ride bust beware what's found inside"));
					break;
				case kMedusaQuest:
					if (persistent->_medisleShowFates) {
						hints.push_back(kMedusaOnceBeauty);
						hints.push_back(TranscribedSound::make("m2250wa0", "Medusa's curse is quite well known: she looks at you and you turn to stone"));
						hints.push_back(TranscribedSound::make("m2250wb0", "So be forewarned, it isn't wise to gaze into Medusa's eyes."));
					}
					break;
				case kRescuePhilQuest:
					hints.push_back(TranscribedSound::make("m2240wa0", "To rescue Phil's an easy fix: just go across the river Styx"));
					hints.push_back(TranscribedSound::make("m2240wb0", "You know Phil, he loves the ladies. He'd be best beware of the evil ladies"));
					break;
				// To silence warning
				case kNoQuest:
				case kEndGame:
				case kNumQuests:
					break;
				}

				if (hints.empty()) {
					playFatesLairBackupSound();
				} else {
					fatesShadowSpeech(hints[(_hintsCounter - 2) % hints.size()], 11632);
				}
				return;
			}

			// TODO: check this

			if (showFate(kLachesis) && showFate(kAtropos) && !showFate(kClotho)) {
				fatesShadowSpeech(kIllTakeTheEyeYouCanHaveOneOfMyEars, 11632);
			}

			if (showFate(kLachesis) && !showFate(kAtropos) && showFate(kClotho)) {
				fatesShadowSpeech(kMedusaOnceBeauty, 11632);
			}

			if (showFate(kLachesis) && !showFate(kAtropos) && !showFate(kClotho)) {
				fatesShadowSpeech(_hintsCounter & 1 ? kIllTakeTheEyeYouCanHaveOneOfMyEars : kMedusaOnceBeauty, 11632);
			}

			if (!showFate(kLachesis) && showFate(kAtropos) && showFate(kClotho)) {
				fatesShadowSpeech(kICanForetellTheFuture, 11632);
			}

			if (!showFate(kLachesis) && showFate(kAtropos) && !showFate(kClotho)) {
				fatesShadowSpeech(_hintsCounter & 1 ? kICanForetellTheFuture : kIllTakeTheEyeYouCanHaveOneOfMyEars, 11632);
			}

			if (!showFate(kLachesis) && !showFate(kAtropos) && showFate(kClotho)) {
				switch (_hintsCounter % 4) {
				case 0:
					fatesShadowSpeech(kGiveMeEyeOrIllPunchYouInTheEye, 11629);
					return;
				case 1:
					fatesShadowSpeech(kPunchingOwnEye, 11632);
					return;
				case 2:
					fatesShadowSpeech(kICanForetellTheFuture, 11632);
					return;
				case 3:
					fatesShadowSpeech(kMedusaOnceBeauty, 11632);
					return;
				}
			}

			return;
		}

	  		/*
TODO (medusa quest):
 MNSH: Perseus
 MNSH: MedusasLair
*/
	}

  	bool handleClickWithItem(const Common::String &name, InventoryItem item) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		for (int i = 0; i < 5; i++) {
			if (name == itemNames[i] && item == kShield + i) {
				itemPlaced(item);
				return true;
			}
		}

		if (name == "Perseus" && (
			    item >= kShield && item <= kSandals)) {
			room->disableMouse();
			playPerseusAnimSpeech("m1240ba0", TranscribedSound::make("m1240na0", "No, you've got to put it in the frieze above me, hurry!"), 11053);
		}

		return false;
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		switch (eventId) {
		case 11002:
			if (!room->isMouseEnabled() || !room->isPanRight())
				break;
			room->disableMouse();
			room->playVideo("m2100ba0", 0, 11003,
					Common::Point(640, 216));
			break;
		case 11003:
			room->playSpeech(TranscribedSound::make(
							  "m2100wa0",
							  "Someone help! I'm over here"),
						  11004);
			room->enableMouse();
			break;
		case kStoneTaken:
			room->playVideo("m1360ba0", 200, kStoneTakenCleanup, Common::Point(0, 216));
			break;
		case 11064: // Right pan
			if (persistent->_medisleShowFatesIntro) {
				persistent->_medisleShowFatesIntro = false;
				persistent->_medisleShowFates = true;
				room->setPannable(false);
				room->disableHotzone("Argo");
				room->playMusic("m2120ma0", 11642);
				g_vm->addTimer(11601, 6500);
				room->disableMouse();
				persistent->_medisleEyeballIsActive = true;
			}
			break;
		case 11601:
			fatesShadowSpeech(TranscribedSound::make("m2120wa0", "Now it's time, the eyeball's mine"), 11602);
			break;
		case 11602:
			fatesShadowSpeech(TranscribedSound::make("m2120wb0", "I can't see, so give it to me"), 11603);
			break;
		case 11603:
			fatesShadowSpeech(TranscribedSound::make("m2120wc0", "You stupid lout. Let's go, look out"), 11604);
			break;
		case 11604:
			fatesShadowSpeechEnd();
			room->playAnimWithSFX("m2120ba0", "m2120ea0", 280, PlayAnimParams::disappear(), 11605, kOffsetRightRoom);
			break;
		case 11605:
			room->enableMouse();
			room->enableHotzone("Eyeball");
			room->selectFrame("m2010oa0", 280, 0, Common::Point(949, 409));
			room->setLayerParallax("m2010oa0", -200);
			g_vm->addTimer(11606, g_vm->getRnd().getRandomNumberRng(10000, 20000), -1);
			break;
		case 11606:
			if (_eyeIsGivenBack || _eyeIsPickedUp)
				break;
			fatesShadowSpeech(eyeInsistSpeech[_eyeInsistCounter % 3], 11608);
			_eyeInsistCounter++;
			break;
		case 11608:
			fatesShadowSpeechEnd();
			break;
		case 11065: // Left pan
			if (persistent->_seriphosPlayedMedusa && !persistent->_medislePlayedPerseusIntro) {
				room->disableMouse();
				room->playVideo("m1140ba0", 0, 11005, Common::Point(0, 216));
			}
			break;
		case 11005:
			playPerseusAnimSpeech("m1140bb0",
					      TranscribedSound::make(
						      "m1140nb0",
						      "This is what happens when you don't follow instructions. "
						      "I should have listened to Athena's warning"),
					      11007);
			persistent->_medislePlayedPerseusIntro = true;
			break;
		case 11006:
			finishPerseusAnim();
			room->enableMouse();
			break;
		case 11007:
			playPerseusAnimSpeech("m1140bc0", TranscribedSound::make(
						      "m1140nc0",
						      "Without that stuff, I'm gonna wind up as part of Medusa's latest rock collection"),
					      11008);
			break;
		case 11008:
			playPerseusAnimSpeech("m1140bd0", TranscribedSound::make("m1140nd0", "You gotta help me"), 11006);
			break;
		case 11009:
			room->playSpeech(itemClickSounds[_lastClickedItem], 11010);
			break;
		case 11010:
			for (int i = 0; i < 5; i++)
				room->stopAnim(itemImagesGlow[i]);
			break;
		case 11012:
			room->playMusic("m1210ma0", 11013);
			break;
		case 11013:
			playPerseusAnimSpeech(perseusItemAnims[_lastPlacedItem - kShield],
					      perseusItemSounds[_lastPlacedItem - kShield], 11014);
			break;
		case 11014: {
			finishPerseusAnim();
			int event = -1;
			switch (getNumberOfBroughtItems()) {
			case 1:
				event = 11015;
				break;
			case 2:
				persistent->_medisleShowFatesIntro = true;
				event = 11017;
				break;
			case 3:
				event = 11020;
				break;
			case 4:
				event = 11024;
				break;
			case 5:
				room->playAnimWithSFX("m1170ba0",
						      "m1190ec1",
						      806,
						      PlayAnimParams::disappear(),
						      11021);
				return;
			}
			int snakeIdx = 4 - getNumberOfBroughtItems();
			room->playAnimWithSFX(snakes[snakeIdx],
					      "m1190ec1", 807 + snakeIdx,
					      PlayAnimParams::disappear(),
					      event);
		}
			break;
		case 11015:
			renderPerseus();
			if (_lastPlacedItem == kSword)
				playPerseusAnimSpeech("m1180bc0", TranscribedSound::make("m1180nb0", "Thanks for the sword. Did you bring anything else?"), 11016);
			else
				playPerseusAnimSpeech("m1190bc0", TranscribedSound::make("m1190nb0", "Thanks for the shield. Did you bring anything else?"), 11016);
			break;
		case 11016:
		case 11025:
		case 11053:
			finishPerseusAnim();
			room->enableMouse();
			break;
		case 11017:
			renderPerseus();
			playPerseusAnimSpeech("m1200ba0", TranscribedSound::make("m1200na0", "You're doing great but I still need three other weapons. The fates will know where they are. Go ask them. They're somewhere on the isle"), 11018);
			break;
		case 11018:
			finishPerseusAnim();
			room->playVideo("m1200ma0", 0, 11019);
			break;
		case 11019:
		case 11037:
		case 11042:
		case 11049:
		case kStoneTakenCleanup:
			room->enableMouse();
			break;
		case 11021:
			for (int i = 0; i < 5; i++)
				room->stopAnim(itemImages[i]);
			for (int i = 0; i < 4; i++)
				room->stopAnim(snakes[i]);
			for (int i = 0; i < 4; i++)
				room->stopAnim(greenSnakes[i]);
			_perseusAnim.hide();
			room->stopAnim("m1010oi0");
			room->playVideo("m1260bh0", 900, 11023, Common::Point(2, 60));
			break;
		case 11023:
			room->selectFrame("m1260bh1", 900, 0);
			// TODO: arcade sequence
			if (0) {
				g_vm->moveToRoom(kMedusaPuzzle);
			} else
				g_vm->moveToRoom(kQuiz);
			break;
		case 11024:
			renderPerseus();
			playPerseusAnimSpeech("m1250bb0", TranscribedSound::make("m1250nb0", "What about the last item?"), 11025);
			break;
		case 11020:
			renderPerseus();
			playPerseusAnimSpeech("m1250ba0",
					      TranscribedSound::make("m1250na0", "What about the other items?"), 11025);
			break;
		case 11027:
			room->playVideo("m2130ba0", kFatesZ, 11609, Common::Point(756, 0));
			break;
		case 11029:
			moveEye(kLachesis, 11621);
			break;
		case 11621:
			renderFatesExcept(kLachesis);
			room->playVideo("m2160ba0", kFatesZ, _isFirstFates ? 11622 : 11627, Common::Point(854, 0));
			break;
		case 11622:
			moveEye(kAtropos, 11623);
			break;
		case 11623:
			renderFatesExcept(kAtropos);
			room->playVideo("m2170ba0", kFatesZ, _isFirstFates ? 11624 : 11627, Common::Point(1002, 96));
			break;
		case 11624:
			if (persistent->_medisleBagPuzzleState < Persistent::BAG_STARTED)
				persistent->_medisleBagPuzzleState = Persistent::BAG_STARTED;
			room->playSFX("m2200ea0");
			startBagPuzzle();
			// Fallthrough
		case 11625:
			moveEye(kClotho, 11626);
			break;
		case 11626:
			renderFatesExcept(kClotho);
			room->playVideo("m2180ba0", kFatesZ, 11627, Common::Point(1090, 68));
			break;
		case 11627:
			renderFatesAll();
			room->enableMouse();
			_isFirstFates = false;
			room->setPannable(true);
			room->enableHotzone("Argo");
			break;
		case 11044:
			if (room->isMouseEnabled() && !_eyeIsGivenBack) {
				room->playVideo("m2130bd0", 0, 11045, Common::Point(640, 216));
			}
			break;
		case 11609:
			renderFatesExcept(kAtropos, kClotho);
			room->playVideo("m2130bb0", kFatesZ, 11610, Common::Point(922, 0));
			break;
		case 11610:
			renderFatesExcept(kClotho);
			room->playVideo("m2130bc0", kFatesZ, 11611, Common::Point(1024, 0));
			break;
		case 11611:
			renderFatesAll();
			room->enableMouse();
			g_vm->addTimer(11044, 5000);
			g_vm->addTimer(11615, g_vm->getRnd().getRandomNumberRng(5000, 10000), -1);
			break;
		case 11615: {
			if (showNoFates() || _fatesAreBusy || !room->isPanRight())
				break;
			_fatesAreBusy = true;

			FateId fate = (FateId) g_vm->getRnd().getRandomNumberRng(0, 2);

			if (!showFate(fate) && fate == kLachesis) {
				room->playSFX(
					g_vm->getRnd().getRandomBit() ? "m2160wb0" : "m2160wa0", 11616);
				break;
			}

			if (!showFate(fate)) {
				_fatesAreBusy = false;
				break;
			}

			// Move eye to another fate
			if (persistent->_medisleEyePosition == fate) {
				int variants = 0;
				for (FateId i = kLachesis; i < kNumFates; i = (FateId) (i + 1))
					if (showFate(i) && i != fate)
						variants++;
				if (variants <= 0) {
					_fatesAreBusy = false;
					break;
				}

				int off = variants == 1 ? 0 : g_vm->getRnd().getRandomBit();

				FateId moveTo = kLachesis;
				int j = 0;

				for (FateId i = kLachesis; i < kNumFates; i = (FateId) (i + 1))
					if (showFate(i) && i != fate) {
						if (j == off) {
							moveTo = i;
							break;
						}
						j++;
					}

				moveEye(moveTo, 11617);
				break;
			}

			switch (fate) {
			case kLachesis:
				room->stopAnim("m2140od0");
				if (g_vm->getRnd().getRandomBit()) {
					room->playAnimWithSFX("m2140ba0", "m2140ea0", 1500,
							      PlayAnimParams::disappear(), 11617, kOffsetRightRoom);
				} else {
					room->playAnimWithSFX("m2140be0", "m2140ee0", 1500,
							      PlayAnimParams::disappear(), 11617, kOffsetRightRoom);
				}
				break;
			case kAtropos:
				room->stopAnim("m2140oe0");
				room->playAnimWithSFX("m2140bg0", "m2140eg0", 1500,
						      PlayAnimParams::disappear(), 11617, kOffsetRightRoom);
				break;
			case kClotho:
				room->stopAnim("m2140of0");
				room->playAnimWithSFX("m2140bh0", "m2140eh0", 1500,
						      PlayAnimParams::disappear(), 11617, kOffsetRightRoom);
				break;
			// To silence warning
			case kNumFates:
				break;
			}
			break;
		}
		case 11616:
		case 11617:
			renderFatesAll();
			break;
		case 11629:
			room->playSpeech(kPunchingOwnEye, 11630);
			break;
		case 11630:
		case 11632:
			room->enableMouse();
			fatesShadowSpeechEnd();
			break;
		case kLoopFatesShadow:
			room->playAnim("m2280bc0", 4000,
				       PlayAnimParams::loop().partial(10, 49), -1, kOffsetRightRoom);
			break;
		case 11035:
			room->playSFX("m1270ea0", 11203);
			_statueDrag = -1;
			renderStatue();
			break;
		case 11203:
			if (isAllPlaced()) {
				room->playAnimWithSFX("m1270bc0", "m1270eb0", 500, PlayAnimParams::disappear(), 11038);
				room->disableMouse();
			}
			break;
		case 11038:
			persistent->_medisleBagPuzzleState = Persistent::BAG_SOLVED;
			showMagicBag();
			room->playVideo("m1270bd0", 0, 11039, Common::Point(0, 216));
			break;
		case 11039:
		case 11040:
			playPerseusAnimSpeech("m1290ba0", TranscribedSound::make("m1290na0", "Hey, that's the magic bag. Put it in the frieze above me"), 11041);
			break;
		case 11041:
			finishPerseusAnim();
			room->playVideo(persistent->_gender == kMale ? "m1290bb0" : "m1290bc0", 0, 11042);
			break;
		case 11036:
			_statueDrag = -1;
			renderStatue();
			if (_depProblemState == 1
			    && persistent->_hintsAreEnabled) {
				room->disableMouse();
				room->playVideo("m1270bb0", 0, 11037, Common::Point(0, 216));
				_depProblemState = 2;
			}
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		room->loadHotZones("MedIsle.HOT", false);
		room->addStaticLayer("m1010pa0", kBackgroundZ);
		room->panRightInstant();
		room->setPannable(true);
		if (quest == kCreteQuest && !persistent->_medisleStoneTaken) {
			room->selectFrame(kStoneAnim, 1000, 0);
			room->enableHotzone(kStoneHotzone);
		}

		room->enableHotzone("Argo");

		if (quest != kMedusaQuest) {
			room->enableHotzone("MedusasLair");
		}

		if (quest == kMedusaQuest && persistent->_seriphosPlayedMedusa) {
			if (!persistent->_medislePlayedPerseusIntro) {
				room->playMusic("m1140ma0", 11056);
				g_vm->addTimer(11002, 5000);
			}
			renderPerseus();
			room->enableHotzone("Perseus");
			room->selectFrame("m1130ba0", 1200, 0);
			_perseusAnim = AmbientAnim("m1130ba0", "", 1200, 5000, 10000,
						   AmbientAnim::KEEP_LOOP, Common::Point(0, 0),
						   AmbientAnim::PAN_LEFT);
			_perseusAnim.start();
			room->enableHotzone("sword");
			room->enableHotzone("shield");
			room->enableHotzone("bag");
			room->enableHotzone("sandals");
			room->enableHotzone("helmet");
		}

		// TODO: figure out all the panning events
		room->setUserPanCallback(-1, 11065, -1, 11064);
		room->selectFrame("m1010pb0", 300, 0);
		room->setLayerParallax("m1010pb0", -200);
		room->selectFrame("m1010pc0", 300, 0, Common::Point(815, 395));
		room->setLayerParallax("m1010pc0", -200);
		room->selectFrame("m1010ta0", 2000, 0);

		for (int i = 0; i < 5; i++) {
			if (persistent->_medislePlacedItems[i])
				room->selectFrame(itemImages[i], 2000, 0);
		}

		room->playMusicLoop("m1010ea0");

		if (persistent->_medisleShowFates) {
			enforceEyeConsistency();
			_eyeIsGivenBack = true;
			renderFatesAll();
			g_vm->addTimer(11615, g_vm->getRnd().getRandomNumberRng(5000, 10000), -1);
		} else
			room->enableHotzone("FatesLair");

		if (quest == kMedusaQuest && persistent->_medisleBagPuzzleState == 2) {
			for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
				_statueState[i] = true;
			}
			renderStatue();
			room->enableHotzone("MagicBag");
			room->playAnimLoop("m1270bg0", 500);
		} else if (quest == kMedusaQuest && persistent->_medisleBagPuzzleState == 1) {
			startBagPuzzle();
		} else {
			for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
				_statueState[i] = true;
			}
			renderStatue();
		}

		g_vm->getHeroBelt()->setColour(HeroBelt::kCool);

		for (unsigned i = 0; i < sizeof(ambientsLeft) / sizeof(ambientsLeft[0]); i++) {
			AmbientAnim(ambientsLeft[i].image, "", ambientsLeft[i].zVal,
				    ambientsLeft[i].minint, ambientsLeft[i].maxint,
				    AmbientAnim::DISAPPEAR, Common::Point(0, 0), AmbientAnim::PAN_LEFT).start();
		}

		for (unsigned i = 0; i < sizeof(ambientsLeftSnakesAndRats) / sizeof(ambientsLeftSnakesAndRats[0]); i++) {
			AmbientAnim(ambientsLeftSnakesAndRats[i].image, ambientsLeftSnakesAndRats[i].sound, ambientsLeftSnakesAndRats[i].zVal,
				    ambientsLeftSnakesAndRats[i].minint, ambientsLeftSnakesAndRats[i].maxint,
				    ambientsLeftSnakesAndRats[i].keep ? AmbientAnim::KEEP_LOOP : AmbientAnim::DISAPPEAR,
				    Common::Point(0, 0), AmbientAnim::PAN_LEFT).start();
		}

		for (unsigned i = 0; i < sizeof(ambientsRight) / sizeof(ambientsRight[0]); i++) {
			AmbientAnim(ambientsRight[i].image, "", ambientsRight[i].zVal,
				    ambientsRight[i].minint, ambientsRight[i].maxint,
				    AmbientAnim::KEEP_LOOP, kOffsetRightRoom, AmbientAnim::PAN_RIGHT).start();
		}
		room->playAnimLoop("M2070BA0", 450, kOffsetRightRoom);
		room->playAnimLoop("m2260ba0", 1500, kOffsetRightRoom);
		room->playAnimLoop("m1040ba0", 450);
	}
private:
	void fatesShadowSpeech(const TranscribedSound &sound, int event) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (!_fatesShadowIsActive) {
			room->playAnim("m2280bc0", 4000,
				       PlayAnimParams::disappear().partial(0, 49), kLoopFatesShadow, kOffsetRightRoom);
			_fatesShadowIsActive = true;
		}
		room->playSpeech(sound, event);
	}

	void fatesShadowSpeechEnd() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_fatesShadowIsActive = false;
		room->playAnim("m2280bc0", 4000,
			       PlayAnimParams::disappear().partial(49, 54), -1, kOffsetRightRoom);
	}

	int getNumberOfBroughtItems() {
		Persistent *persistent = g_vm->getPersistent();
		int brought = 0;
		for (int i = 0; i < 5; i++)
			brought += persistent->_medislePlacedItems[i];
		return brought;
	}

	void renderPerseus() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		room->selectFrame("m1010oi0", 1300, 0);
		int bluestages = MAX(4 - getNumberOfBroughtItems(), 0);
		int i;
		for (i = 0; i < bluestages; i++) {
			room->selectFrame(snakes[i], 807 + i, 0);
		}
		for (; i < 4; i++) {
			room->playAnimLoop(greenSnakes[i], 807 + i);
		}
	}

	void playPerseusAnimSpeech(const Common::String &anim, const TranscribedSound &sound, int callback) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_perseusAnim.hide();
		room->playAnimWithSpeech(anim, sound, 1200, PlayAnimParams::disappear(), callback);
	}

	void finishPerseusAnim() {
		_perseusAnim.unpauseAndFirstFrame();
	}

	void itemPlaced(InventoryItem item) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		int itemIdx = item - kShield;
		Persistent *persistent = g_vm->getPersistent();

		if (persistent->_medislePlacedItems[itemIdx]
			|| persistent->_quest != kMedusaQuest
			|| !persistent->_seriphosPlayedMedusa)
			return;
		persistent->_medislePlacedItems[itemIdx] = true;
		g_vm->getHeroBelt()->removeFromInventory(item);
		_lastPlacedItem = item;
		room->selectFrame(itemImages[itemIdx], 2000, 0);
		room->playSFX(itemSounds[itemIdx], 11012);
		room->disableMouse();
	}

	void itemGlow(int itemIdx) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		if (persistent->_quest != kMedusaQuest
		    || !persistent->_seriphosPlayedMedusa)
			return;
		if (!persistent->_medislePlacedItems[itemIdx])
			room->playAnimLoop(itemImagesGlow[itemIdx], 2000);
		_lastClickedItem = itemIdx;
		room->playSFX("m1150ea0", 11009);
	}

	void renderFatesExcept(FateId except1, FateId except2 = kNumFates) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		enforceEyeConsistency();
		for (FateId i = kLachesis; i < kNumFates; i = (FateId) (i + 1)) {
			if (!showFate(i) || i == except1 || i == except2) {
				room->stopAnim(Common::String::format("m2140o%c0", i + 'd'));
				room->stopAnim(Common::String::format("m2140o%c0", i + 'a'));
			} else if (persistent->_medisleEyePosition == i) {
				room->selectFrame(Common::String::format("m2140o%c0", i + 'a'),
						  kFatesZ, 0, kOffsetRightRoom);
				room->stopAnim(Common::String::format("m2140o%c0", i + 'd'));
			} else {
				room->stopAnim(Common::String::format("m2140o%c0", i + 'a'));
				room->selectFrame(Common::String::format("m2140o%c0", i + 'd'),
						  kFatesZ, 0, kOffsetRightRoom);
			}
		}

		for (FateId i = kLachesis; i < kNumFates; i = (FateId) (i + 1)) {
			room->setHotzoneEnabled(fatesHotzoneNames[i], showFate(i));
		}

		room->setHotzoneEnabled("FatesLair", !showAllFates());

		room->stopAnim("m2140ba0");
		room->stopAnim("m2140be0");
		_fatesAreBusy = except1 != kNumFates || except2 != kNumFates;
	}

	void renderFatesAll() {
		renderFatesExcept(kNumFates);
	}

	void enforceEyeConsistency() {
		Persistent *persistent = g_vm->getPersistent();

		if (persistent->_medisleEyePosition == kNumFates)
			return;

		if (showFate(persistent->_medisleEyePosition))
			return;

		for (persistent->_medisleEyePosition = kLachesis; persistent->_medisleEyePosition < kNumFates
			     && !showFate(persistent->_medisleEyePosition);
		     persistent->_medisleEyePosition = (FateId) (persistent->_medisleEyePosition + 1));
		if (persistent->_medisleEyePosition == kNumFates)
			persistent->_medisleEyePosition = kLachesis;
	}

	bool showFate(FateId fate) {
 		Persistent *persistent = g_vm->getPersistent();
		static const InventoryItem mapping[3] = {kHelmet, kBag, kSandals};

		if (persistent->_quest != kMedusaQuest || !persistent->_medisleShowFates || fate >= kNumFates)
			return false;

		InventoryItem mapped = mapping[fate];

		return !persistent->isInInventory(mapped)
			&& !persistent->_medislePlacedItems[mapped - kShield];
	}

	void moveEye(FateId newPos, int event) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		if (newPos == persistent->_medisleEyePosition) {
			g_vm->handleEvent(event);
			return;
		}

		FateId oldPos = persistent->_medisleEyePosition;
		renderFatesExcept(oldPos, newPos);

		const char *transAnim = nullptr;
		const char *transSound = nullptr;

		if (newPos == kLachesis && oldPos == kAtropos) {
			transAnim = "m2140bb0";
			transSound = "m2140eb0";
		}
		if (newPos == kLachesis && oldPos == kClotho) {
			transAnim = "m2150bc0";
			transSound = "m2150ec0";
		}

		if (newPos == kAtropos && oldPos == kLachesis) {
			transAnim = "m2140bd0";
			transSound = "m2140ed0";
		}

		if (newPos == kAtropos && oldPos == kClotho) {
			transAnim = "m2140bc0";
			transSound = "m2140ec0";
		}

		if (newPos == kClotho && oldPos == kAtropos) {
			transAnim = "m2140bf0";
			transSound = "m2140ef0";
		}

		if (newPos == kClotho && oldPos == kLachesis) {
			transAnim = "m2150bd0";
			transSound = "m2150ed0";
		}

		room->playAnimWithSFX(transAnim, transSound, kFatesZ, PlayAnimParams::disappear(), event, kOffsetRightRoom);
		persistent->_medisleEyePosition = newPos;
		_fatesAreBusy = true;
	}

	void renderStatue() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
			if (_statueState[i]) {
				room->stopAnim(statuePieces[i].emptyElement);
				room->selectFrame(statuePieces[i].fullElement, statuePieces[i].statuez, 0, statuePieces[i].getStatueOffset());
				room->disableHotzone(Common::String("D") + statuePieces[i].name);
			} else if (_statueDrag == i) {
				room->stopAnim(statuePieces[i].fullElement);
				room->selectFrame(statuePieces[i].emptyElement, statuePieces[i].statuez, 0, statuePieces[i].getStatueOffset());
				room->disableHotzone(Common::String("D") + statuePieces[i].name);
			} else {
				room->selectFrame(statuePieces[i].emptyElement, statuePieces[i].statuez, 0, statuePieces[i].getStatueOffset());
				room->selectFrame(statuePieces[i].fullElement, kStatuesZVal2, 0, statuePieces[i].getBrokenOffset());
				room->enableHotzone(Common::String("D") + statuePieces[i].name);
				room->setHotZoneOffset(Common::String("D") + statuePieces[i].name, statuePieces[i].getBrokenOffset());
			}
		}

		if (_statueDrag >= 0) {
			room->drag(statuePieces[_statueDrag].fullElement, 0, statuePieces[_statueDrag].getHotspot());
			room->setPannable(false);
		} else {
			room->clearDrag();
			room->setPannable(true);
		}
	}

	void startBagPuzzle() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
			_statueState[i] = false;
		}
		renderStatue();
		for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
			room->enableHotzone(Common::String("D") + statuePieces[i].name);
			room->enableHotzone(Common::String("S") + statuePieces[i].name);
		}
	}

	void statueDClick(int pieceNum) {
		_statueDrag = pieceNum;
		
		renderStatue();
	}

	bool checkDep(int pieceNum) {
		if (statuePieces[pieceNum].dep1 >= 0
		    && !_statueState[statuePieces[pieceNum].dep1])
			return false;
		if (statuePieces[pieceNum].dep2 >= 0
		    && !_statueState[statuePieces[pieceNum].dep2])
			return false;
		return true;
	}

	void statueSClick(int pieceNum) {
		// TODO: animation
		if (_statueDrag != pieceNum || !checkDep(pieceNum)) {
			if (_depProblemState == 0 && _statueDrag == pieceNum)
				_depProblemState = 1;
			_statueDrag = -1;
			renderStatue();
			g_vm->handleEvent(11036);
			return;
		}

		_statueState[_statueDrag] = true;
		g_vm->handleEvent(11035);
	}

	bool isAllPlaced() {
		for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
			if (!_statueState[i])
				return false;
		}

		return true;
	}

	void showMagicBag() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnimLoop("m1270bg0", 500);
		room->enableHotzone("MagicBag");
		renderStatue();
		for (int i = 0; i < ARRAYSIZE(statuePieces); i++) {
			room->disableHotzone(Common::String("D") + statuePieces[i].name);
			room->disableHotzone(Common::String("S") + statuePieces[i].name);
		}
	}

	void hideMagicBag() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->stopAnim("m1270bg0");
		room->disableHotzone("MagicBag");
	}

	bool showAllFates() {
		return showFate(kLachesis) && showFate(kAtropos) && showFate(kClotho);
	}

	bool showNoFates() {
		return !showFate(kLachesis) && !showFate(kAtropos) && !showFate(kClotho);
	}

	void playFatesLairBackupSound() {
		if (g_vm->getRnd().getRandomBit())
			fatesShadowSpeech(kGiveMeEyeOrIllPunchYouInTheEye, 11629);
		else
			fatesShadowSpeech(kIllTakeTheEyeYouCanHaveOneOfMyEars, 11630);
	}

	AmbientAnim _perseusAnim;
	InventoryItem _lastPlacedItem;
	int _eyeInsistCounter;
	bool _eyeIsGivenBack, _eyeIsPickedUp;
	bool _fatesShadowIsActive;
	bool _isFirstFates;
	bool _statueState[10];
	bool _fatesAreBusy;
	int _depProblemState;
	int _statueDrag;
	int _lastClickedItem;
	int _hintsCounter;
};

Common::SharedPtr<Hadesch::Handler> makeMedIsleHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new MedIsleHandler());
}

}
