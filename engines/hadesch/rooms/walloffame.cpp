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

static const char *kBackground = "background";
static const char *kHotspots = "HallTrph.hot";
static const char *kArgo = "argo";

static const char *kZeusYooHoo = "zeus yoo-hoo";
static const char *kZeusYellsPhil = "zeus yells phil";
static const char *kZeusVacationIsOver = "zeus vacation's over";
static const char *kZeusFineChoiceHero = "zeus fine choice hero";
static const char *kZeusShaftOfLightLeft = "zeus shaft of light left";

static const char *kRope = "rope";
static const char *kHerculesLintel = "hercules lintel";

static const char *kApplicationHotspots = "applicat.hot";
static const char *kApplication = "application";
static const char *kApplicationHeroine = "application heroine";
static const char *kApplicationHeroineHotzone = "heroine";
static const char *kApplicationHero = "application hero";
static const char *kApplicationHeroHotzone = "hero";
static const char *kApplicationEnterButton = "application enter button";
static const char *kApplicationEnterHotzone = "enter";
static const char *kApplicationBurntHole = "application burnt hole";
static const char *kApplicationText = "application text";
static const char *kApplicationHeroineText = "application heroine text";
static const char *kApplicationHeroText = "application hero text";
static const char *kApplicationNameText = "application name text";
static const char *kApplicationUnfurls = "application unfurls";
static const char *kApplicationUnfurlsSound = "application unfurls sound";
static const char *kApplicationRollsUpHeroine = "application rolls up heroine";
static const char *kApplicationRollsUpHero = "application rolls up hero";
static const char *kApplicationChooseHeroSound = "application choose hero sound";
static const char *kApplicationChooseHeroineSound = "application choose heroine sound";

static const char *kHadesBurstsIn = "hades bursts in";
static const char *kHadesNoMoreHeroes = "hades no more heroes got it";
static const char *kHadesNoMoreHeroines = "hades no more heroines got it";

#define kPhilWalksCenterToLeft "phil walks center to left"
#define kPhilSighsLeft "phil sighs left"

#define kPhilSnores "phil snores"
static const char *kPhilSnoresSound = "phil snores sound";
#define kPhilRollsOver "phil rolls over"
#define kPhilJumpsOffPillow "phil jumps off pillow"
static const char *kPhilOfCourseIdo = "phil of course i do";
static const char *kPhilWhatAHothead = "phil what a hothead";
static const char *kPhilTimeToGetStarted = "phil time to get started";
static const char *kPhilTreasuresOfHercules = "phil treasures of hercules";
static const char *kPhilBasicInformation = "phil basic information";
static const char *kPhilFlakyPlaster = "phil flaky plaster";
static const char *kPhilGettinStarted = "phil gettin started";
static const char *kPhilToolsOfTrade = "phil tools of the trade";
static const char *kPhilHeroBelt = "phil hero belt";
static const char *kPhilHeroBeltBuckle = "phil left belt buckle";
static const char *kPhilClickAnItem = "phil click an item";
static const char *kPhilHeroPowers = "phil hero powers";
static const char *kPhilRightNotches = "phil right notches";
static const char *kPhilEyeOfFates = "phil eye of fates";
static const char *kPhilBadNews = "phil bad news";
static const char *kPhilFirstQuest = "phil first quest";
#define kPhilTakesScroll "phil takes scroll"
#define kPhilDropsScroll "phil drops scroll"
static const char *kPhilDaedalusNeedsHelp = "phil daedalus needs help";
static const char *kPhilOffToCrete = "phil off to crete";
static const char *kPhilArgo = "phil argo";
#define kPhilTapsFootLeft "phil taps foot left"

#define kPhilYapsWithoutSound "phil yaps without sound"

static const char *kMugshot = "mug shot";
static const char *kMugshotSound = "mug shot sound";
static const char *kPanicAndPainSting = "pain and panic sting";

#define kHeroStatue "hero statue"
#define kHeroineStatue "heroine statue"
static const char *kScroll = "scroll";
static const char *kScrollSound = "scroll sound";
	
static const char *kHeroBelt = "hero belt";
static const char *kHeroBeltUpSound = "hero belt up sound";
static const char *kHeroBeltDownSound = "hero belt down sound";

static const char *herculesRoomElements[] = {
	"mares",
	"cattle",
	"girdle",
	"horns",
	"hydra",
	"bull",
	"lion",
	"apples",
	"stables",
	"boar",
	"birds",
	"cerberus",
	"hercules",
};

static const char *allPhils[] = {
	"phil walks left to center",
	kPhilWalksCenterToLeft,
	"phil faces backward left",
	kPhilSighsLeft,
	kPhilTapsFootLeft,
	"phil dusts left",
	"phil dusts center",
	kPhilSnores,
	kPhilRollsOver,
	kPhilJumpsOffPillow,
	kPhilYapsWithoutSound,
	kPhilTakesScroll,
	kPhilDropsScroll,
	kPhilSighsLeft,
	"phil scans left"
};

static const char *powerLevelNames[] = {
	"thunderbolt",
	"trojan horse",
	"owl"
};

static const char *zeusComments[] = {
	"minotaur",
	"medusa",
	"trojan armor",
	"cyclops",
	kHeroStatue,
	kHeroineStatue
};

enum {
	kPhilJokeEvent = 19003,
	kPhilNewQuestScroll = 1019020,
	kPhilBecomesIdle = 1019025,
	kPhilJokeEventCleanup = 1019029,
	kPhilRereadDropScroll = 1019030
};

enum {
	kBackgroundZ = 10000,
	kQuestOverlayZ = 925,
	kMugshotZ = 900,
	kScrollZ = 900,
	kShaftOfLightLeftZ = 500,
	kShaftOfLightRightZ = 500,
	kTrophyZ = 450,
	kPhilZ = 400,
	kApplicationZ = 300,
	kApplicationButtonZ = 299,

	kSoundOnlyZ = 0,

	// TODO: figure proper values
	kHadesVideoZ = 0,
	kHeroBeltZ = 0,
	kGlowZ = 0
};

Common::String nameQuest() {
	Persistent *persistent = g_vm->getPersistent();
	switch(persistent->_quest) {
	case kNoQuest:
		return "";
	case kCreteQuest:
		return "crete quest";
	case kTroyQuest:
		return "troy quest";
	case kMedusaQuest:
		return "medusa quest";
	case kRescuePhilQuest:
		return "rescue phil quest";
	case kEndGame:
		return "finished game";
	// To silence warning
	case kNumQuests:
		return "";
	}

	return "";
}

Common::String getShaftOfLightRight() {
	Persistent *persistent = g_vm->getPersistent();
	if (persistent->_quest == kEndGame)
		return "zeus shaft of light right";
	return "zeus shaft of light " + nameQuest();
}

class WallOfFameHandler : public Handler {
public:
	WallOfFameHandler() {
		bool isNewGame = !g_vm->isRoomVisited(kWallOfFameRoom);
		Persistent *persistent = g_vm->getPersistent();
		if (isNewGame) {
			_gender = kUnknown;
			_philIsOnTheRight = false;
		} else {
			_gender = persistent->_gender;
			_philIsOnTheRight = true;
		}
		_isEnterMouseover = false;
		_philWalkPhase = 0;
		_philIsBusy = isNewGame;
		_endGameOutro = false;
		_applicationIsActive = false;
		_philSaidDoingGreat = false;
	}

	void handleClick(const Common::String &hotname) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		if (hotname == kApplicationHeroineHotzone) {
			room->selectFrame(kApplicationHeroine, kApplicationButtonZ, 1);
			room->selectFrame(kApplicationHero, kApplicationButtonZ, 0);
			room->playSFX(kApplicationChooseHeroineSound);
			_gender = kFemale;
			computeEnter();
			return;
		}
		if (hotname == kApplicationHeroHotzone) {
			room->selectFrame(kApplicationHeroine, kApplicationButtonZ, 0);
			room->selectFrame(kApplicationHero, kApplicationButtonZ, 1);
			room->playSFX(kApplicationChooseHeroSound);
			_gender = kMale;
			computeEnter();
			return;
		}
		if (hotname == kApplicationEnterHotzone) {
			pressEnter();
			return;
		}

		if (hotname == "argo") {
			g_vm->moveToRoom(kArgoRoom);
			return;
		}

		for (unsigned i = 0; i < sizeof (herculesRoomElements) / sizeof(herculesRoomElements[0]); i++)
			if (hotname == herculesRoomElements[i]) {
				room->disableMouse();
				room->playAnimKeepLastFrame(hotname + " glow", kGlowZ);
				room->playSFX("click");
				room->playVideo("zeus " + hotname, kSoundOnlyZ, 1019026);
				return;
			}

		for (unsigned power = 0; power < ARRAYSIZE(powerLevelNames); power++) {
			if (hotname.matchString(Common::String::format("%s#", powerLevelNames[power]))) {
				Common::String pl(powerLevelNames[power]);
				room->playSFX("click");
				if (strcmp(powerLevelNames[power], "thunderbolt") == 0)
					room->playSFX("thunder sound");
				zeusCommentRight("zeus " + pl);
				return;
			}
		}

		for (unsigned i = 0; i < sizeof (zeusComments) / sizeof(zeusComments[0]); i++)
			if (hotname == zeusComments[i]) {
				room->playSFX("click");
				zeusCommentRight(Common::String("zeus ") + hotname);
				return;
			}

		if (hotname == "typhon") {
			room->playSFX("click");
			zeusCommentRight(persistent->_gender == kFemale ? "zeus typhon heroine" : "zeus typhon hero");
			return;
		}

		if (hotname == "phil" && quest >= kCreteQuest && quest <= kMedusaQuest) {
			philBecomesListening();
			playPhilVideo(Common::String("phil reread ") + nameQuest(), kPhilRereadDropScroll,
				      Common::Point(22, 301));
			return;
		}

		if (hotname == "hades note") {
			g_vm->cancelTimer(19048);
			handleEvent(19048);
			return;
		}

		if (hotname == "hades note popup background" ||
		    hotname == "hades note popup background2") {
			room->popHotZones();
			room->setPannable(true);
			room->stopAnim("hades note popup");
			room->stopAnim("hades note text");
			room->stopAnim("hades note");
			if (persistent->_hintsAreEnabled) {
				zeusCommentRight("zeus counting on ya");
			}
			room->setHotzoneEnabled("argo", true);
			return;
		}

		if (hotname == "monster frieze") {
			g_vm->moveToRoom(kMonsterPuzzle);
			return;
		}

		if (hotname == "ferryman frieze") {
			g_vm->moveToRoom(kFerrymanPuzzle);
			return;
		}

		/*
		  TODO:
		  MNSH: medusa frieze
		  MNSH: trojan horse frieze
		  MNSH: labyrinth frieze
		*/
	}

	void handleMouseOver(const Common::String &hotname) override {
		Persistent *persistent = g_vm->getPersistent();

		if (hotname == kApplicationEnterHotzone) {
			_isEnterMouseover = true;
			computeEnter();
			return;
		}
		if (persistent->_quest <= kMedusaQuest
		    && (hotname == "labyrinth frieze"
			|| hotname == "trojan horse frieze")) {
			if (_philIsBusy)
				return;
			_philIsBusy = true;
			if (hotname == "labyrinth frieze")
				playPhilVideo("phil labyrinth frieze", kPhilBecomesIdle, Common::Point(40, 324)); // state 51
			else
				playPhilVideo("phil trojan horse frieze", kPhilBecomesIdle, Common::Point(14, 320)); // state 52
			return;
		}

		if (hotname.hasSuffix(" frieze")) {
			zeusCommentRight("zeus " + hotname, true);
			return;
		}

	}

	void handleMouseOut(const Common::String &hotname) override {
		if (hotname == kApplicationEnterHotzone) {
			_isEnterMouseover = false;
			computeEnter();
			return;
		}
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		switch (eventId) {
		case 19004:
			g_vm->addSkippableTimer(19005, 7000);
			room->playAnim(kZeusShaftOfLightLeft, kShaftOfLightLeftZ,
				       PlayAnimParams::keepLastFrame().speed(500));
			room->playVideo(kZeusYooHoo, kSoundOnlyZ, 19701); // zeus yoo-hoo
			break;
		case 19005:
			playPhilAnimSFX(kPhilRollsOver, 1019001, Common::Point(-26, 2)); // state 1
			break;
		case 19011:
			room->playVideo(kZeusVacationIsOver, kSoundOnlyZ, 19012);
			break;
		case 19012:
			playPhilVideo(kPhilOfCourseIdo, 1019002, Common::Point(30, 304)); // state 5
			break;
		case 1019002:
			displayPhilIdle();
			room->playVideo(kZeusFineChoiceHero, kSoundOnlyZ, 19013);
			break;
		case 19013:
			room->playAnim(kZeusShaftOfLightLeft, kShaftOfLightLeftZ, PlayAnimParams::disappear().backwards().speed(500));
			room->setLayerEnabled(kRope, false);
			playPhilVideo(kPhilBasicInformation, 1019003, Common::Point(-4, 2)); // state 6
			break;
		case 19016:
			room->addStaticLayer(kApplicationBurntHole, kApplicationButtonZ);
			room->playVideo(persistent->_gender == kFemale ? kHadesNoMoreHeroines : kHadesNoMoreHeroes,
					   kHadesVideoZ, 19018,
					   Common::Point(207, 103));
			break;
		case 19018:
			applicationClose();
			break;
		case 19019:
			playPhilVideo(kPhilWhatAHothead, 1019005, Common::Point(40, 327)); // state 7
			break;
		case 1019001:
			cancelAllPhils();
			playPhilAnimSFX(kPhilJumpsOffPillow, -1, Common::Point(-26, 2)); // state 4
			room->playVideo(kZeusYellsPhil, kSoundOnlyZ, 19011);
			break;
		case 1019003:
			room->setLayerEnabled(kRope, true);
			displayPhilIdle();
			room->playAnimWithSFX(
				kApplicationUnfurls, kApplicationUnfurlsSound,
				kApplicationZ, PlayAnimParams::disappear().speed(200), 1019004);
			break;
		case 1019004:
			room->enableMouse();
			startApplication();
			break;
		case 1019005:
			playPhilVideo(kPhilTimeToGetStarted, 1019006,
				      Common::Point(36, 319)); // state 8
			break;
		case 1019006:
			playPhilVideo(kPhilTreasuresOfHercules, 1019007,
				      Common::Point(40, 323)); // state 9
			break;
		case 1019007:
			displayPhilIdle();
			room->panRightAnim(1019008);
			break;
		case 1019008:
			_philIsOnTheRight = true;
			updatePhilHotzone();
			playPhilVideo(kPhilFlakyPlaster, 1019009,
				      Common::Point(0, 0)); //state 10
			break;
		case 1019009:
			playPhilVideo(kPhilGettinStarted, 1019010,
					Common::Point(14, 324)); //state 11
			break;
		case 1019010:
			room->disableHeroBelt();
			playPhilVideo(kPhilToolsOfTrade, 1019011,
				      Common::Point(40, 320)); //state 12
			break;
		case 1019011:
			displayPhilYap();
			room->playAnimWithSFX(
				kHeroBelt, kHeroBeltUpSound, kHeroBeltZ,
				PlayAnimParams::keepLastFrame().partial(0, 9),
				19022, kOffsetRightRoom);
			break;
		case 19022:
			room->playVideo(kPhilHeroBelt, kPhilZ, 1019012); // state 13
			break;
		case 1019012:
			room->playVideo(kPhilHeroBeltBuckle, kPhilZ, 1019013); // state 14
			break;
		case 1019013:
			room->selectFrame(kHeroBelt, kHeroBeltZ, 10,
					     kOffsetRightRoom);
			room->playSFX("hero belt items highlight sound");
			room->playVideo(kPhilClickAnItem, kPhilZ, 1019014); // state 15
			break;
		case 1019014:
			room->selectFrame(kHeroBelt, kHeroBeltZ, 11,
					     kOffsetRightRoom);
			room->playSFX("hero belt items highlight sound");
			room->playVideo(kPhilHeroPowers, kPhilZ, 1019015); // state 16
			break;
		case 1019015:
			room->selectFrame(kHeroBelt, kHeroBeltZ, 12,
					     kOffsetRightRoom);
			room->playSFX("hero belt items highlight sound");
			g_vm->addTimer(19029, 4000);
			room->playVideo(kPhilRightNotches, kPhilZ, 1019016); // state 17
			break;
		case 1019016:
			g_vm->cancelTimer(19029);
			room->playAnimWithSFX(
				kHeroBelt, kHeroBeltDownSound, kHeroBeltZ, PlayAnimParams::disappear().partial(9, 0),
				1019017, kOffsetRightRoom);
			break;
		case 1019017:
			room->enableHeroBelt();
			playPhilVideo(kPhilEyeOfFates, 1019018,
				      Common::Point(-8, 144)); // state 18
			break;
		case 1019018:
			room->playSFX(kPanicAndPainSting);
			playPhilVideo(kPhilBadNews, 1019019, Common::Point(14, 320)); // state 19
			break;
		case 1019019:
			displayPhilIdle();
			room->playAnimWithSFX(kMugshot, kMugshotSound,
					      kMugshotZ, PlayAnimParams::keepLastFrame(),
					      19006, kOffsetRightRoom);
			break;
		case 19006:
			room->playAnimWithSFX(kScroll, kScrollSound,
					      kScrollZ, PlayAnimParams::keepLastFrame(),
					      19009, kOffsetRightRoom);
			break;
		case 19009:
			playPhilVideo(kPhilFirstQuest, kPhilNewQuestScroll, Common::Point(40, 324)); // state 21
			break;
		case kPhilNewQuestScroll:
			room->selectFrame(kScroll, kScrollZ, 0, kOffsetRightRoom);
			room->playMusic("quest intro music");
			playPhilAnimSFX(kPhilTakesScroll, 1019021); // state 22
			break;
		case 1019021:
			switch (quest) {
			default:
			case kCreteQuest:
				playPhilVideo(kPhilDaedalusNeedsHelp, 1019022, Common::Point(28, 312)); // state 24
				break;
			case kTroyQuest:
				playPhilVideo("phil holy hera", 1019022, Common::Point(28, 312)); // state 29
				break;
			case kMedusaQuest:
				playPhilVideo("phil who writes these things", 1019022, Common::Point(28, 312)); // state 36
				break;
			}
			break;
		case 1019022:
			playPhilAnimSFX(kPhilDropsScroll, 1019023); // state 23
			break;
		case 1019023:
			switch (quest) {
			default:
			case kCreteQuest:
				playPhilVideo(kPhilOffToCrete, 1019024, Common::Point(40, 324)); // state 25
				break;
			case kTroyQuest:
				playPhilVideo("phil hourly rates", 1019033, Common::Point(28, 312)); // state 30
				break;
			case kMedusaQuest:
				playPhilVideo("phil off to seriphos", kPhilBecomesIdle, Common::Point(14, 320)); // state 37
			}
			break;
		case 1019024:
			playPhilVideo(kPhilArgo, kPhilBecomesIdle, Common::Point(40, 324)); // state 26
			break;
		case kPhilBecomesIdle:
			philBecomesIdle();
			break;
		case 1019026:
			room->enableMouse();
			for (unsigned i = 0; i < sizeof (herculesRoomElements) / sizeof(herculesRoomElements[0]); i++)
				room->stopAnim(Common::String(herculesRoomElements[i]) + " glow");
			break;
		case 1019027:
			if (!_philIsBusy)
				schedulePhilJoke();
			break;
		case 1019028:
			g_vm->cancelTimer(kPhilJokeEvent);
			break;
		case kPhilJokeEvent: {
			if (_philIsBusy) {
				schedulePhilJoke();
				break;
			}

			if (!_philSaidDoingGreat && quest != kCreteQuest) {
				_philSaidDoingGreat = true;
				// state 50
				playPhilVideo("phil doing great", kPhilJokeEventCleanup,
					      Common::Point(14, 320));
				break;
			}
				
			int rnd = g_vm->getRnd().getRandomNumberRng(1, 10);
			debug("rnd = %d", rnd);
			switch(rnd) {
			case 1:
				// state 40
				playPhilVideo("phil break time over", kPhilJokeEventCleanup,
					      Common::Point(40, 320));
				break;
			case 2:
				// state 41
				playPhilVideo("phil back to work", kPhilJokeEventCleanup,
					Common::Point(14, 320));
				break;
			case 3:
				// state 42
				playPhilVideo("phil lets go", kPhilJokeEventCleanup,
					Common::Point(16, 320));
				break;
			case 4:
				// state 43
				playPhilAnimWithoutSound("phil scans left", kPhilJokeEventCleanup,
							 Common::Point(-26, 2));
				break;
			case 5:
				// state 44, go right
				if (_philWalkPhase == 3) {
					schedulePhilJoke();
					break;
				}
				_philWalkPhase++;
				updatePhilHotzone();
				playPhilAnimSFX("phil walks left to center", kPhilJokeEventCleanup,
						Common::Point(-166, +2));
				break;
			case 6:
				// state 45, go left
				if (_philWalkPhase == 0) {
					schedulePhilJoke();
					break;
				}
				_philWalkPhase--;
				updatePhilHotzone();
				playPhilAnimSFX(kPhilWalksCenterToLeft, kPhilJokeEventCleanup);
				break;
			case 7:
				// state 46
				playPhilAnimSFX("phil faces backward left", kPhilJokeEventCleanup, Common::Point(-640, 0));
				break;
			case 8:
				// state 47
				playPhilAnimSFX(kPhilSighsLeft, kPhilJokeEventCleanup);
				break;
			case 9:
				// state 48
				playPhilAnimSFX(kPhilTapsFootLeft, kPhilJokeEventCleanup);
				break;
			case 10:
				// state 49, dusts
				switch (_philWalkPhase) {
				case 0:
					playPhilAnimSFX("phil dusts left", kPhilJokeEventCleanup);
					break;
				case 1:
				case 2:
					playPhilAnimSFX("phil dusts center", kPhilJokeEventCleanup, Common::Point(-166, 2));
					break;
				case 3:
					schedulePhilJoke();
					break;
				}
				break;
			}
			break;
		}
		case 19029:
			room->selectFrame(kHeroBelt, kHeroBeltZ, 13,
					  kOffsetRightRoom);
			room->playSFX("hero belt items highlight sound");
			break;
		case kPhilJokeEventCleanup:
			schedulePhilJoke();
			break;
		case 19045:
			philBecomesIdle();
			room->playAnim(
				getShaftOfLightRight(),
				kShaftOfLightRightZ,
				PlayAnimParams::disappear().speed(500));
			room->enableMouse();
			break;
		case kPhilRereadDropScroll:
			playPhilAnimSFX(kPhilDropsScroll, 19047);
			break;
		case 19047:
			philBecomesIdle();
			break;
		case 19048:
			room->disableMouse();
			room->disableHotzone("hades note");
			room->enableHotzone("hades note popup");
			room->disableHotzone("hades note popup background");
			room->enableHotzone("hades note popup background2");
			room->selectFrame("hades note popup", 150, 0, kOffsetRightRoom);
			room->selectFrame("hades note text", 149, 0, kOffsetRightRoom);
			room->playVideo("hades reads note", 0, 19049);
			break;
		case 19049:
			room->enableMouse();
			break;
		case 19051:
			_hercules = AmbientAnim("herc blinks", "", 0, 2000, 5000,
						AmbientAnim::KEEP_LOOP, kOffsetRightRoom,
						AmbientAnim::PAN_ANY);
			_hercules.start();
			playPhilVideo("phil that was awesome", 19052,
				      Common::Point(14, 320)); // state 56
			break;
		case 19052:
			philBecomesListening();
			room->fadeOut(250, 19053);
			break;
		case 19053:
			_hercules.hide();
			room->resetFade();
			room->disableHeroBelt();
			room->resetLayers();
			room->disableMouse();
			room->setPannable(false);
			room->addStaticLayer("black background", kBackgroundZ, kOffsetRightRoom);
			room->playVideo("hades eye of fates", 0, 19054, kOffsetRightRoom);
			room->playSFX("herc laughs");
			room->playSFX("phil laughs");
			break;
		case 19054:
			g_vm->moveToRoom(kCreditsRoom);
			break;
		case 1019031:
			_philWalkPhase--;
			updatePhilHotzone();
			playPhilAnimSFX(kPhilWalksCenterToLeft, 1019032);
			break;
		case 1019032:
			playPhilVideo("phil lots more to do", kPhilNewQuestScroll, Common::Point(14, 312)); // state 28
			break;
		case 1019033:
			playPhilVideo(persistent->_gender == kFemale ? "phil go to it heroine" : "phil go to it hero",
				      1019034, Common::Point(40, 324)); // state 32/31
			break;
		case 1019034:
			playPhilVideo("phil off to troy", kPhilBecomesIdle, Common::Point(40, 324)); // state 33
			break;
		case 1019035:
			playPhilVideo("phil sneaky devil", 1019036, Common::Point(40, 324)); // state 34
			break;
		case 1019036:
			_philWalkPhase--;
			updatePhilHotzone();
			playPhilAnimSFX(kPhilWalksCenterToLeft, 1019037);
			break;
		case 1019037:
			playPhilVideo("phil good work", kPhilNewQuestScroll, Common::Point(40, 324)); // state 35
			break;
		case 19900:
			if (_gender != kUnknown && !_heroName.empty()) {
				break;
			}
			if (!_heroName.empty()) {
				room->playVideo("phil pick a statue", 0, 19901);
				break;
			}

			if (g_vm->getRnd().getRandomBit()) {
				room->playVideo("phil not a mind reader", 0, 19901);
			} else {
				room->playVideo("phil type in your name", 0, 19901);
			}
			break;
		case 19901:
			g_vm->addTimer(19900, 12000);
			break;
		}
	}

	void handleKeypress(uint32 code) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_applicationIsActive && room->isMouseEnabled()) {
			if (_heroName.size() < 18 &&
			    (('a' <= code && code <= 'z')
			     || ('A' <= code && code <= 'Z')
			     || code == '.' || code == '-' || code == ' ')
			    && room->computeStringWidth("application sign-in text", _heroName + code) <= 318) {
				_heroName += code;
				room->playSFX("application click");
				renderNameInApplication();
				computeEnter();
				return;
			}

			if (_heroName.size() > 0 && code == '\b') {
				_heroName.deleteLastChar();
				room->playSFX("application click");
				renderNameInApplication();
				computeEnter();
				return;
			}

			if (_heroName.size() > 0 && (code == '\r' || code == '\n')) {
				pressEnter();
				return;
			}
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		bool newGame = !g_vm->isRoomVisited(kWallOfFameRoom);
		Persistent *persistent = g_vm->getPersistent();
		room->loadHotZones(kHotspots, false);
		room->addStaticLayer(kBackground, kBackgroundZ);
		room->addStaticLayer(kHerculesLintel, 900);
		room->addStaticLayer(kRope, 900);
		Quest quest = persistent->_quest; 
		if (quest != kEndGame) {
			room->addStaticLayer(kArgo, 950, kOffsetRightRoom);
			room->enableHotzone("argo");
			room->playAnimLoop("argo ambient", 949, kOffsetRightRoom);
		}
		room->playAnimLoop("water ambient", 975, kOffsetRightRoom);
		room->playAnimLoop("hercules hall ambients", 900);
		room->addStaticLayer(nameQuest() + " overlay", kQuestOverlayZ, kOffsetRightRoom);
		if (quest == kEndGame) {
			const char *heroStatue = persistent->_gender == kFemale ? kHeroineStatue : kHeroStatue;
			room->selectFrame(heroStatue, 450, 0, kOffsetRightRoom);
			room->enableHotzone(heroStatue);
			room->setHotZoneOffset(heroStatue, kOffsetRightRoom);
			room->enableHotzone("monster frieze");
			room->enableHotzone("ferryman frieze");
			room->enableHotzone("cyclops");
			room->enableHotzone("typhon");
		}

		if (quest > kMedusaQuest) {
			room->enableHotzone("medusa frieze");
			room->enableHotzone("medusa");
		}
		if (quest > kTroyQuest) {
			room->enableHotzone("trojan horse frieze");
			room->enableHotzone("trojan armor");
		}
		if (quest > kCreteQuest) {
			room->enableHotzone("labyrinth frieze");
			room->enableHotzone("minotaur");
		}

		if (quest < kRescuePhilQuest) {
			room->enableHotzone("phil");
			room->setHotZoneOffset("phil", kOffsetRightRoom);
		}

		// Hercules room
		for (unsigned i = 0; i < sizeof (herculesRoomElements) / sizeof(herculesRoomElements[0]); i++)
			room->enableHotzone(herculesRoomElements[i]);

		for (unsigned power = 0; power < sizeof(powerLevelNames) / sizeof(powerLevelNames[0]); power++) {
			int level = persistent->_powerLevel[power];
			if (level > 0) {
				int off = (4 - level) * 30;
				Common::String hz = Common::String::format("%s%d", powerLevelNames[power], level);
				room->playAnim(powerLevelNames[power], kTrophyZ,
					       PlayAnimParams::loop().partial(off, off + 29),
					       -1, kOffsetRightRoom);
				room->enableHotzone(hz);
				room->setHotZoneOffset(hz, kOffsetRightRoom);
			}
		}
		
		room->selectFrame(kScroll, kScrollZ, 0, kOffsetRightRoom);
		room->setPannable(true);
		if (newGame) {
			room->disableMouse();
			room->playAnimWithSFX(kPhilSnores, kPhilSnoresSound, kPhilZ, PlayAnimParams::loop()); //state 0
			room->selectFrame(kMugshot, kMugshotZ, 0, kOffsetRightRoom);
			g_vm->addSkippableTimer(19004, 4000);
		} else if (persistent->_doQuestIntro) {
			persistent->_doQuestIntro = false;
			room->disableMouse();
			switch (quest) {
			case kTroyQuest:
				_philWalkPhase = 1;
				playPhilVideo("phil congrats trapped minotaur", 1019031, Common::Point(14, 320)); // state 27
				room->playMusic("done crete quest theme");
				break;
			case kMedusaQuest:
				_philWalkPhase = 1;
				playPhilVideo("phil dances", 1019035, Common::Point(36, 257)); // state 38
				room->playMusic("done troy quest theme");
				break;
			case kRescuePhilQuest:
				g_vm->addTimer(19048, 5000, 1);
				room->pushHotZones("HTNote.HOT", true, kOffsetRightRoom);
				room->disableHotzone("hades note popup background2");
				room->setPannable(false);
				room->enableMouse();
				room->playAnimWithSFX("hades note", "hades note sound", 150, PlayAnimParams::loop(),
						      EventHandlerWrapper(), kOffsetRightRoom);
				room->setHotzoneEnabled("argo", false);
				room->playMusic("done medusa quest theme");
				_endGameOutro = true;
				break;

			case kNumQuests: // To silence warning
			case kEndGame:
			case kNoQuest:
				_philWalkPhase = 1;
				philBecomesListening();
				room->playVideo("herc congratulates", 0, 19051, kOffsetRightRoom);
				break;
			case kCreteQuest:
				break;
			}

			room->panRightInstant();
		} else {
			if (persistent->_quest < kRescuePhilQuest)
				schedulePhilJoke();
			room->panRightInstant();
		}

		renderLintel();

		room->playMusicLoop("C1010eA0");

		g_vm->getHeroBelt()->setColour(HeroBelt::kWarm);
	}

	~WallOfFameHandler() override {}

private:
	void clearNameInApplication() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		// One character more to handle possible backspace clicks.
		room->hideString("application sign-in text", _heroName.size() + 1);
	}

	void renderNameInApplication() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		clearNameInApplication();
		room->renderString("application sign-in text", _heroName, Common::Point(202, 338), 298);
	}

	void philBecomesIdle() {
		_philIsBusy = false;
		schedulePhilJoke();
	}

	void philBecomesBusy() {
		_philIsBusy = true;
		g_vm->cancelTimer(kPhilJokeEvent);
	}

	void philBecomesListening() {
		philBecomesBusy();
		displayPhilIdle();
	}

	void schedulePhilJoke() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		displayPhilIdle();
		room->enableMouse();
		g_vm->addTimer(kPhilJokeEvent, g_vm->getRnd().getRandomNumberRng(10000, 15000));
		room->setUserPanCallback(1019028, -1, -1, 1019027);
	}

	void pressEnter() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		// originally event 19015
		if (_gender != kUnknown /*&& name != ""*/) {
			persistent->_gender = _gender;
			persistent->_heroName = _heroName;
			room->disableMouse();
			// Or should it be music? Unclear to me
			room->playSFX("hades evil intro theme");
			room->playVideo(kHadesBurstsIn, kHadesVideoZ, 19016);
		}
	}

	void zeusCommentRight(const Common::String &id, bool mouseIsEnabled = false) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (!mouseIsEnabled)
			room->disableMouse();
		room->playVideo(id, kSoundOnlyZ, 19045);
		room->playAnim(
			getShaftOfLightRight(), kShaftOfLightRightZ, PlayAnimParams::keepLastFrame().speed(500), -1, kOffsetRightRoom);
		philBecomesListening();
	}

	Common::Point getPhilBase() const {
		Common::Point v;
		if (_philIsOnTheRight)
			v = Common::Point(640, 0);
		else
			v = Common::Point(26, -2);
		if (_philWalkPhase >= 0 && _philWalkPhase <= 3)
			v += Common::Point(166 * _philWalkPhase, -2 * _philWalkPhase);
		return v;
	}
	void playPhilVideo(const Common::String &name, int callback, const Common::Point videoOffset) {
		Persistent *persistent = g_vm->getPersistent();
		cancelAllPhils();
		if (persistent->_quest == kRescuePhilQuest)
			return;
		g_vm->getVideoRoom()->playVideo(name, kPhilZ, callback,
						videoOffset + getPhilBase());
	}

	void playPhilAnimSFX(const Common::String &name,
			     int callback, const Common::Point videoOffset = Common::Point(0, 0)) {
		Persistent *persistent = g_vm->getPersistent();
		cancelAllPhils();
		if (persistent->_quest == kRescuePhilQuest)
			return;
		g_vm->getVideoRoom()->playAnimWithSFX(
			name, name + " sound", kPhilZ, PlayAnimParams::keepLastFrame(), callback,
			videoOffset + getPhilBase());
	}

	void playPhilAnimWithoutSound(const Common::String &name,
				      int callback, const Common::Point videoOffset = Common::Point(0, 0)) {
		Persistent *persistent = g_vm->getPersistent();
		cancelAllPhils();
		if (persistent->_quest == kRescuePhilQuest)
			return;
		g_vm->getVideoRoom()->playAnimKeepLastFrame(
			name, kPhilZ, callback,
			videoOffset + getPhilBase());
	}

	void displayPhilIdle() {
		Persistent *persistent = g_vm->getPersistent();
		cancelAllPhils();
		if (persistent->_quest == kRescuePhilQuest
		    || (!_endGameOutro && persistent->_quest == kEndGame))
			return;
		g_vm->getVideoRoom()->selectFrame(kPhilSighsLeft, kPhilZ, 0,
						  getPhilBase());
	}

	void displayPhilYap() {
		cancelAllPhils();
		g_vm->getVideoRoom()->playAnimLoop(
			kPhilYapsWithoutSound, kPhilZ, Common::Point(640-166, 0));
	}

	void computeEnter() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_gender == kUnknown /*|| name == ""*/) {
			room->selectFrame(kApplicationEnterButton, kApplicationButtonZ, 0);
			room->disableHotzone(kApplicationEnterHotzone);
			return;
		}
		room->selectFrame(kApplicationEnterButton, kApplicationButtonZ, _isEnterMouseover ? 2 : 1);
		room->enableHotzone(kApplicationEnterHotzone);
	}

	void startApplication() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->addStaticLayer(kApplication, kApplicationZ);
		room->selectFrame(kApplicationHeroine, kApplicationButtonZ, 0); // heroine
		room->selectFrame(kApplicationHero, kApplicationButtonZ, 0); // hero
		room->selectFrame(kApplicationEnterButton, kApplicationButtonZ, 0);
		room->addStaticLayer(kApplicationText, kApplicationButtonZ);
		room->addStaticLayer(kApplicationHeroineText, kApplicationButtonZ);
		room->addStaticLayer(kApplicationHeroText, kApplicationButtonZ);
		room->addStaticLayer(kApplicationNameText, kApplicationButtonZ);
		room->pushHotZones(kApplicationHotspots);
		room->disableHotzone(kApplicationEnterHotzone);
		_applicationIsActive = true;
		g_vm->addTimer(19900, 5000);
	}

	void applicationClose() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		room->setLayerEnabled(kApplication, false);
		room->setLayerEnabled(kApplicationHeroine, false);
		room->setLayerEnabled(kApplicationHero, false);
		room->setLayerEnabled(kApplicationEnterButton, false);
		room->setLayerEnabled(kApplicationText, false);
		room->setLayerEnabled(kApplicationHeroineText, false);
		room->setLayerEnabled(kApplicationHeroText, false);
		room->setLayerEnabled(kApplicationNameText, false);
		room->setLayerEnabled(kApplicationBurntHole, false);
		room->setLayerEnabled(kApplicationUnfurls, false);
		room->popHotZones();
		clearNameInApplication();
		_applicationIsActive = false;
		// TODO: does it have any sound?
		room->playAnim(persistent->_gender == kFemale ? kApplicationRollsUpHeroine : kApplicationRollsUpHero,
			       kApplicationZ, PlayAnimParams::disappear(), 19019);
		renderLintel();
	}

	void cancelAllPhils() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		for (uint i = 0; i < ARRAYSIZE(allPhils); i++)
			room->stopAnim(allPhils[i]);
	}

	void updatePhilHotzone() {
		g_vm->getVideoRoom()->setHotZoneOffset(
			"phil", getPhilBase());
	}

	void renderLintel() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		room->renderStringCentered("player lintel text", persistent->_heroName, Common::Point(942, 19), 900, -1);
	}

	Gender _gender;
	bool _isEnterMouseover;
	bool _philIsOnTheRight;
	int _philWalkPhase;
	bool _philIsBusy;
	Common::U32String _heroName;
	AmbientAnim _hercules;
	bool _endGameOutro;
	bool _applicationIsActive;
	bool _philSaidDoingGreat;
};

Common::SharedPtr<Hadesch::Handler> makeWallOfFameHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new WallOfFameHandler());
}

}
