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
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"

#include "gui/message.h"


namespace Hadesch {

enum {
	kBackgroundZ = 10000,
	kCharonZ = 701
};

static const int kNumSeats = 10;

enum {
	kHuman          = 1 <<  0,
	kMonster        = 1 <<  1,
	kAnimal         = 1 <<  2,
	kSmoking        = 1 <<  6,
	kFlat           = 1 <<  7,
	kFrozen         = 1 <<  8,
	kPierced        = 1 <<  9,
	kWet            = 1 << 10,
	kDismembered    = 1 << 11,
	kWeaponInjury   = 1 << 12,
	kTridentInjured = 1 << 13,
	kHeadInjury     = 1 << 14,
	kDrownedGuy     = 1 << 15,
	kHorned         = 1 << 16,
	kCrushed        = 1 << 17,
	kSnakeKilled    = 1 << 18,
	kCold           = 1 << 19,
	kTwoHeaded      = 1 << 20,
	kHot            = 1 << 21,
	kHeadless       = 1 << 22,
	kChokedDog      = 1 << 23,
	kWithHoles      = 1 << 24,
	kCat            = 1 << 25,
	kDog            = 1 << 26
};

struct ShadowInfo {
	const char *image;
	const char *nameImage;
	const char *name;
	int waityoffset;
	int priorityleft;
	int priorityright;
	int attributes;
	int waitBubbleX;
	int waitBubbleY;
	int starboardBubbleX;
	int starboardBubbleY;
	int portBubbleX;
	int portBubbleY;
	int portFrame;
	int starboardFrame;
	const char *animSound;
	int animPlayCount;

	Common::Point getWaitBubblePos() const {
		return Common::Point(waitBubbleX, waitBubbleY);
	}

	Common::Point getPortBubblePos() const {
		return Common::Point(portBubbleX, portBubbleY);
	}

	Common::Point getStarboardBubblePos() const {
		return Common::Point(starboardBubbleX, starboardBubbleY);
	}
};

static const ShadowInfo shadows[] = {
	{
	 "xxxxxxxx", ""        , "Chariot Wheel Guy" ,   0,  0, 0   , kHuman   | kFlat | kCrushed,
	 0, 0, 0, 0, 0, 0, 2, 3,
	 nullptr, 0
	},
	{"V9070bD0", "V9520tD0", "Chariot Wheel Dog" ,  40, 20, 5   , kAnimal  | kFlat | kCrushed | kDog,
	 113, 177, 66, 134, 106, 176, 16, 32,
	 nullptr, 0
	},
	{"V9070bE0", "V9520tE0", "Drowned Guy"       ,   0, 10, 15  , kHuman   | kWet | kDrownedGuy,
	 124, 84, 78, 67, 104, 80, 19, 35,
	 "V9070eE0", 1
	},
	{"V9070bI0", "V9520tI0", "Holey Guy"         ,   0, 15, 20  , kHuman   | kPierced | kWeaponInjury | kWithHoles,
	 84, 107, 68, 66, 92, 100, 6, 13,
	 nullptr, 0
	},
	{"V9070bP0", "V9520tO0", "Cyclops"           ,  14, 50, 50  , kMonster | kPierced | kWeaponInjury | kHeadInjury | kCrushed | kTwoHeaded,
	 77, 64, 73, 72, 63, 68, 14, 33,
	 nullptr, 0
	},
	{"V9070bM0", "V9520tM0", "Toasted Guy"       , -13, 40, 50  , kHuman   | kSmoking | kHot,
	 115, 89, 78, 71, 104, 73, 14, 28,
	 nullptr, 0
	},
	{"V9070bJ0", "V9520tJ0", "Minotaur"          ,   0, 25, 20  , kMonster | kHorned,
	 111, 100, 63, 79, 106, 111, 12, 45,
	 nullptr, 0
	},
	{"V9070bA0", "V9520tA0", "Ball Chain Monster",   0, 20, 20  , kMonster | kWeaponInjury | kHeadInjury | kHeadless,
	 79, 92, 85, 91, 83, 102, 13, 26,
	 nullptr, 0
	},
	{"V9070bL0", "V9520tL0", "Snake Man"         ,   0, 25, 40  , kHuman   | kAnimal | kCrushed | kSnakeKilled,
	 82, 65, 93, 62, 74, 45, 4, 8,
	 "V9070eL0", 3
	},
	{"xxxxxxxx", ""	       , "Arrow Guy"         ,   0,  0, 0   , kHuman   | kPierced | kWeaponInjury,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 nullptr, 0 },
	// 10
	{"V9070bC0", "V9520tC0", "Choked Dog"        ,  46, 30, 5   , kAnimal  | kDog | kChokedDog,
	 97, 91, 83, 85, 81, 85, 11, 22,
	 "V9070eC0", 2
	},
	{"V9070bK0", "V9520tK0", "Mounted Aries"     ,  23, 35, 40  , kAnimal  | kHeadInjury | kHorned,
	 121, 62, 75, 54, 113, 52, 6, 12,
	 "V9070eK0", 1
	},
	{"V9070bF0", "V9520tF0", "Flat Cat"          ,  43, 40, 5   , kAnimal  | kFlat | kCrushed | kCat,
	 75, 142, 72, 142, 72, 142, 5, 19,
	 nullptr, 0
	},
	{"V9070bH0", "V9520tH0", "Headless Guy"      , -14,  5, 15  , kHuman   | kDismembered | kWeaponInjury | kHeadInjury | kHeadless,
	 110, 119, 50, 98, 115, 117, 26, 52,
	 nullptr, 0
	},
	{"V9070bG0", "V9520tG0", "Frozen Guy"        ,   0, 15, 40  , kHuman   | kCold | kWet | kFrozen,
	 71, 67, 91, 80, 74, 58, 9, 18,
	 "V9070eG0", 2
	},
	// 15
	{"xxxxxxxx", ""	       , "Caesar"            ,   0,  0, 0   , kHuman   | kPierced | kWeaponInjury,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 nullptr, 0 },
	{"V9070bN0", "V9520tN0", "Trident Guy"       ,   9,  5, 10  , kHuman   | kPierced | kWeaponInjury | kHeadInjury | kTridentInjured,
	 89, 91, 81, 88, 94, 88, 5, 10,
	 nullptr, 0
	},
	{"xxxxxxxx", ""	       , "BeeSting Guy"      ,   0,  0, 0   , kHuman   | kPierced,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 nullptr, 0 },
	{"xxxxxxxx", ""	       , "Half Man"          ,   0,  0, 0   , kHuman   | kDismembered,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 nullptr, 0 },
	{"V9070bQ0", "V9520tP0", "Toasted Cat"       ,  41, 45, 5   , kAnimal  | kSmoking | kHot | kCat,
	 111, 158, 59, 135, 105, 161, 6, 12,
	 "V9070eQ0", 3
	},
	// 20
	{"V9070bR0", "V9520tQ0", "Pillar Guy"        ,   0, 15, 10  , kHuman   | kFlat | kHeadInjury | kCrushed,
	 86, 188, 78, 186, 92, 186, 15, 30,
	 "V9070eR0", 1
	},
	{"xxxxxxxx", ""	       , "Ax-Head Guy"       ,   0,  0, 0   , kHuman   | kWeaponInjury | kHeadInjury,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 nullptr, 0 },
	{"V9070bB0", "V9520tB0", "Boiled Guy"        ,  45,  5, 10  , kHuman   | kHot | kWet,
	 80, 62, 81, 68, 79, 60, 13, 42,
	 nullptr, 0
	}
};

static const struct {
	const char *image;
	TranscribedSound sound;
} thoughts[] = {
	{ "V9140tA0", {"V9140nA0", _hs("He won't sit next to humans") } }, // 0
	{ "V9140tB0", {"V9140nB0", _hs("He won't sit next to animals") } },
	{ "V9140tC0", {"V9140nC0", _hs("He won't sit next to smoking beings") } },
	{ "V9140tD0", {"V9140nD0", _hs("He won't sit next to flat beings") } },
	{ "V9140tE0", {"V9140nE0", _hs("He won't sit next to weapon injuries") } },
	{ "V9140tF0", {"V9140nF0", _hs("He won't sit next to cats") } }, // 5
	{ "V9140tG0", {"V9140nG0", _hs("He won't sit next to two-headed beings") } },
	{ "V9140tH0", {"V9140nH0", _hs("He won't sit next to monsters") } },
	{ "V9140tI0", {"V9140nI0", _hs("He won't sit next to hot beings") } },
	{ "V9140tJ0", {"V9140nJ0", _hs("He won't sit next to headless beings") } },
	{ "V9140tK0", {"V9140nK0", _hs("He won't sit next to horned beings") } }, // 10
	{ "V9140tL0", {"V9140nL0", _hs("He won't sit next to flat monsters") } },
	{ "V9140tM0", {"V9140nM0", _hs("He won't sit next to beings with holes") } },
	{ "V9140tN0", {"V9140nN0", _hs("He won't sit next to pierced beings") } },
	{ "V9140tO0", {"V9140nO0", _hs("He won't sit next to fffreozen beings") } },
	{ "V9140tP0", {"V9140nP0", _hs("He won't sit next to wet beings") } }, // 15
	{ "V9140tQ0", {"V9140nQ0", _hs("He won't sit next to crushed dogs") } },
	{ "V9140tR0", {"V9140nR0", _hs("He won't sit next to dogs") } },
	{ "V9140tS0", {"V9140nS0", _hs("He won't sit next to headless dogs") } },
	{ "V9140tT0", {"V9140nT0", _hs("He won't sit next to dismembered beings") } },
	{ "V9140tU0", {"V9140nU0", _hs("He won't sit next to crushed humans") } }, // 20
	{ "V9140tV0", {"V9140nV0", _hs("He won't sit next to trident-injured beings. And I couldn't blame him") } },
	{ "V9140tW0", {"V9140nW0", _hs("He won't sit next to snake-killed beings") } },
	{ "V9160tA0", {"V9160nA0", _hs("He won't sit across from humans") } },
	{ "V9160tB0", {"V9160nB0", _hs("He won't sit across from animals") } },
	{ "V9160tC0", {"V9160nC0", _hs("He won't sit across from weapon injuries") } }, // 25
	{ "V9160tD0", {"V9160nD0", _hs("He won't sit across from cats") } },
	{ "V9160tE0", {"V9160nE0", _hs("He won't sit across from monsters") } },
	{ "V9160tF0", {"V9160nF0", _hs("He won't sit across from hot beings") } },
	{ "V9160tG0", {"V9160nG0", _hs("He won't sit across from headless beings") } },
	{ "V9160tH0", {"V9160nH0", _hs("He won't sit across from flat beings") } }, // 30
	{ "V9160tI0", {"V9160nI0", _hs("He won't sit across from head injuries") } },
	{ "V9160tJ0", {"V9160nJ0", _hs("He won't sit across from beings with holes") } },
	{ "V9160tK0", {"V9160nK0", _hs("He won't sit across from pierced beings") } },
	{ "V9160tL0", {"V9160nL0", _hs("He won't sit across from wet beings") } },
	{ "V9170tA0", {"V9170nA0", _hs("He simply must sit next to a dog") } }, // 35
	{ "V9170tB0", {"V9170nB0", _hs("He must sit next to a being with holes") } },
	{ "V9170tC0", {"V9170nC0", _hs("He must sit next to a pierced being") } },
	{ "V9170tD0", {"V9170nD0", _hs("He must sit next to a human") } },
	{ "V9170tE0", {"V9170nE0", _hs("He must sit next to a monster") } },
	{ "V9170tF0", {"V9170nF0", _hs("He must sit next to a monster with a head injury") } }, // 40
	{ "V9170tG0", {"V9170nG0", _hs("He must sit next to a dismembered being") } },
	{ "V9170tH0", {"V9170nH0", _hs("He must sit next to a flat human") } },
	{ "V9170tI0", {"V9170nI0", _hs("He must sit next to a smoking being") } },
	{ "V9170tJ0", {"V9170nJ0", _hs("He must sit next to a hot being") } },
	{ "V9170tK0", {"V9170nK0", _hs("He simply must sit next to a horned being") } }, // 45
	{ "V9170tL0", {"V9170nL0", _hs("He must sit next to a flat animal") } },
	{ "V9170tM0", {"V9170nM0", _hs("He must sit next to a flat being") } },
	{ "V9170tN0", {"V9170nN0", _hs("He must sit next to an animal") } },
	{ "V9170tO0", {"V9170nO0", _hs("He must sit next to a headless being") } },
	{ "V9170tP0", {"V9170nP0", _hs("He must sit next to a head being") } }, // 50
	{ "V9170tQ0", {"V9170nQ0", _hs("He must sit next to a weapon injury") } },
	{ "V9170tR0", {"V9170nR0", _hs("He must sit next to a cat") } },
	{ "V9170tS0", {"V9170nS0", _hs("He must sit next to a snake-killed being") } },
	{ "V9170tT0", {"V9170nT0", _hs("He must sit next to a smoking animal") } },
	{ "V9170tU0", {"V9170nU0", _hs("He must sit next to a wet being") } }, // 55
	{ "V9170tV0", {"V9170nV0", _hs("He must sit next to a c-c-cold being") } },
	{ "V9170tY0", {"V9170nY0", _hs("He must sit next to two humans") } },
	{ "V9200tA0", {"V9200nA0", _hs("He must sit across from a being with holes") } },
	{ "V9200tB0", {"V9200nB0", _hs("He must sit across from a pierced being") } },
	{ "V9200tC0", {"V9200nC0", _hs("He must sit across from a human") } }, // 60
	{ "V9200tD0", {"V9200nD0", _hs("He must sit across from a monster") } },
	{ "V9200tE0", {"V9200nE0", _hs("He must sit across from a monster with a head injury") } },
	{ "V9200tF0", {"V9200nF0", _hs("He must sit across from a dismembered injury") } },
	{ "V9200tG0", {"V9200nG0", _hs("He must sit across from a smoking being") } },
	{ "V9200tH0", {"V9200nH0", _hs("He must sit across from a hot being") } }, // 65
	{ "V9200tI0", {"V9200nI0", _hs("He must sit across from a horned being") } },
	{ "V9200tJ0", {"V9200nJ0", _hs("He must sit across from a flat being") } },
	{ "V9200tK0", {"V9200nK0", _hs("He must sit across from an animal") } },
	{ "V9200tL0", {"V9200nL0", _hs("He must sit across from a headless being") } },
	{ "V9200tM0", {"V9200nM0", _hs("He must sit across from the chariot wheel guy") } }, // 70
	{ "V9200tN0", {"V9200nN0", _hs("He must sit across from a head injury") } },
	{ "V9200tO0", {"V9200nO0", _hs("He must sit across from a weapon injury") } },
	{ "V9240tA0", {"V9240nA0", _hs("He must sit in the front or back") } },
	{ "V9210tA0", {"V9210nA0", _hs("He must sit next to and across from only monsters") } },
	{ "V9210tB0", {"V9210nB0", _hs("He must sit next to and across from only animals") } }, // 75
	{ "V9210tC0", {"V9210nC0", _hs("He must sit next to and across from only humans") } },
	{ "V9210tD0", {"V9210nD0", _hs("He must sit next to and across from only weapon injuries") } },
	{ "V9220tA0", {"V9220nA0", _hs("He must not sit next to or across from humans") } },
	{ "V9220tB0", {"V9220nB0", _hs("He must not sit next to or across from pierced beings") } },
	{ "V9220tC0", {"V9220nC0", _hs("He must not sit next to or across from monsters") } }, // 80
	{ "V9220tD0", {"V9220nD0", _hs("He must not sit next to or across from animals") } },
		{ "V9245tA0", {"V9300nE0", _hs("Ha-ha-ha") } }
};

static const struct {
	const char *image;
	const TranscribedSound sound;
} charonRules[] = {
	{ "V9250tA0", { "V9250aA0", _hs("Charon must sit next to horned beings") } }, // 0
	{ "V9250tB0", { "V9250aB0", _hs("Charon must sit next to cats") } },
	{ "V9250tC0", { "V9250aC0", _hs("Charon must sit next to animals") } },
	{ "V9250tD0", { "V9250aD0", _hs("Charon must sit next to smoking beings") } },
	{ "V9260tA0", { "V9260nA0", _hs("Charon can't have any monsters at the head of the boat") } },
	{ "V9260tB0", { "V9260nB0", _hs("No animals at the head of the boat") } }, // 5
	// FIXME: Spelling incorrect. wont should be won't. Fixing changes game data and thus may cause issues
	{ "V9260tC0", { "V9260nC0", _hs("Charon wont sit next to two-headed beings") } },
	{ "V9260tD0", { "V9260nD0", _hs("Charon won't sit next to flat beings") } },
	{ "V9260tE0", { "V9260nE0", _hs("Charon won't sit next to horned beings") } },
	{ "V9260tF0", { "V9260nF0", _hs("Charon won't sit next to cats") } },
	{ "V9260tG0", { "V9260nG0", _hs("Charon won't sit next to smoking beings")  } }, // 10
	{ "V9260tJ0", { "V9260nJ0", _hs("Charon won't sit next to trident-injured beings") } },
	{ "V9270tA0", { "XXXXXXXX", "" } }
};

static const TranscribedSound charonFinishSounds[] = {
	{"V9300nA0", _hs("Well done")},
	{"V9300nH0", _hs("Deadly")},
	{"V9300nI0", _hs("Nice job")}
};

static const TranscribedSound charonNextLevel[] = {
	{"V9280wA0", _hs("Onto level two")},
	{"V9280wB0", _hs("Onto level three")},
	{"V9280wC0", _hs("You have mastered ferryman's folly")}
};

static const char *charonAnims[] = {
	"V9140BA0",
	"V9140BB0",
	"V9140BC0"
};

static TranscribedSound yuckSounds[] = {
	{ "V9290nA0", _hs("Eww!") },
	{ "V9290nB0", _hs("Yuck!") },
	{ "V9290nC0", _hs("Gross!") },
	{ "V9290nD0", _hs("As if!") },
	{ "V9290nE0", _hs("No way!") },
};

struct CharonIdleVideoInfo {
	const char *name;
	int offsetX;
	int offsetY;

	Common::Point getOffset() const {
		return Common::Point(offsetX, offsetY);
	}
};

static CharonIdleVideoInfo charonIdleVideos[] = {
	{"V9140BD0", 418, 40},
	{"V9140BE0", 370, 64}
};

struct Shade {
	int shadowId;
	int thoughtId;
	int currentPos;
	int waitingPos;
	bool positionIsFixed;
	int rowidx;
	int tabNA;
	int tabN;
	int tabO;
	int tabA;
	int tabNN;
};

struct ImagePos {
	int x, y, z;

	Common::Point getPoint() const {
		return Common::Point(x, y);
	}
};

static const ImagePos boatPosition[] = {
	{150, 380, 555},
	{210, 352, 556},
	{266, 328, 557},
	{321, 307, 558},
	{381, 293, 559},
	{240, 452, 550},
	{310, 440, 551},
	{373, 425, 552},
	{434, 405, 553},
	{491, 383, 554}
};

static const ImagePos waitPosition[] = {
	{41,  119, 1058},
	{114,  94, 1059},
	{187,  69, 1060},
	{41,  226, 1051},
	{114, 201, 1052},
	{187, 176, 1053},
	{260, 151, 1054},
	{333, 126, 1055},
	{406, 101, 1056},
	{479,  76, 1057}
};

enum {
	kCharonEndTalk = 24812,
	// 24802 is split into 2 to avoid having an argument
	k24802_arg0 = 1024801,
	k24802_arg1 = 1024802,
	k24017_arg0 = 1024001,
	k24017_arg9 = 1024010,
	k24018_arg0 = 1024011,
	k24018_arg9 = 1024020,
	k24801_arg1 = 1024021,
	k24801_arg2 = 1024022,
	k24801_arg3 = 1024023,
	k24801_arg4 = 1024024,
	k24801_arg5 = 1024025,
	k24801_arg6 = 1024026
};

class FerryHandler : public Handler {
public:
	FerryHandler() {
		_dragged = -1;
		_clickTimer = -1;
		_isPlayingYuck = false;
		_lastCharonAnim = -1;
		_charonIsBusy = false;
		memset(_isInAnim, 0, sizeof(_isInAnim));
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		if (name.matchString("s##")) {
			g_vm->addTimer(24012, 350);
			_clickTimer = typeToIdx(name.substr(1).asUint64());
			return;
		}

		if (name.matchString("f##")) {
			int fPos = (name[1] - '0') * 5 + (name[2] - '0');
			for (unsigned i = 0; i < _shades.size(); i++) {
				if (_shades[i].currentPos == fPos) {
					if (_shades[i].positionIsFixed)
						showThoughtByShadowId(i);
					else {
						_clickTimer = i;
						g_vm->addTimer(24012, 350);
					}
					break;
				}
			}
			return;
		}

		if (name == "Sign" && _charonTID != 12) {
			playCharonSound(charonRules[_charonTID].sound);
			return;
		}

		/*
		  TODO:
 MNSH: Charon
 MNSH: sp##
 MNSH: ss##
*/
	}

	void handleUnclick(const Common::String &name, const Common::Point pnt) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_clickTimer >= 0) {
			g_vm->cancelTimer(24012);
			showThoughtByShadowId(_clickTimer);
			_clickTimer = -1;
		} else if (_dragged >= 0) {
			if (name.matchString("f##")) {
				moveToFerry(_dragged, (name[1] - '0') * 5
					    + (name[2] - '0'));
			} else {
				backToWaiting(_dragged);
			}
			room->stopAnim("v9010bc0");
			_dragged = -1;
			hideThought();
			levelRender();
		}
	}

	void handleMouseOver(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (name.matchString("f##") && _dragged != -1) {
			room->selectFrame("v9010bc0", 800, name[1] == '1' ? (9 - (name[2] - '0')) : (name[2] - '0'));
		}
	}

	void handleMouseOut(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (name.matchString("f##") && _dragged != -1) {
			room->stopAnim("v9010bc0");
		}
	}

	void charonIdle() {
		if (_charonIsBusy)
			return;
		hideCharon();
		int vid = g_vm->getRnd().getRandomNumberRng(0, ARRAYSIZE(charonIdleVideos) - 1);
		g_vm->getVideoRoom()->playVideo(charonIdleVideos[vid].name,
						kCharonZ, 24811, charonIdleVideos[vid].getOffset());
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		switch (eventId) {
		case 24006:
			if (_count24006++ >= 3) {
				hideThought();
				break;
			}
			g_vm->addTimer(24006, 1200);
			// TODO: don't issue duplicates
			showThoughtByShadowId(g_vm->getRnd().getRandomNumberRng(0, 9), true);
			break;
		case 24010:
			_isPlayingYuck = false;
			break;
		case 24012:
			_dragged = _clickTimer;
			_clickTimer = -1;
			levelRender();
			break;
		case k24017_arg0:
		case k24017_arg0 + 1:
		case k24017_arg0 + 2:
		case k24017_arg0 + 3:
		case k24017_arg0 + 4:
		case k24017_arg0 + 5:
		case k24017_arg0 + 6:
		case k24017_arg0 + 7:
		case k24017_arg0 + 8:
		case k24017_arg9:
		{
			int shade = eventId - k24017_arg0;
			g_vm->addTimer(k24017_arg0 + shade, g_vm->getRnd().getRandomNumberRng(10200, 21800));
			if (shade == _dragged)
				return;
			if (_shades[shade].currentPos >= 5) {
				idleAnimShade(shade, boatPosition[_shades[shade].currentPos].z,
					      shadows[_shades[shade].shadowId].portFrame,
					      shadows[_shades[shade].shadowId].starboardFrame - 1);
				return;
			}

			if (_shades[shade].currentPos >= 0) {
				idleAnimShade(shade, boatPosition[_shades[shade].currentPos].z,
					      shadows[_shades[shade].shadowId].starboardFrame, -1);
				return;
			}

			if (_shades[shade].waitingPos >= 0) {
				idleAnimShade(shade, waitPosition[_shades[shade].waitingPos].z,
					      0, shadows[_shades[shade].shadowId].portFrame - 1);
				return;
			}

			break;
		}
		case k24018_arg0:
		case k24018_arg0 + 1:
		case k24018_arg0 + 2:
		case k24018_arg0 + 3:
		case k24018_arg0 + 4:
		case k24018_arg0 + 5:
		case k24018_arg0 + 6:
		case k24018_arg0 + 7:
		case k24018_arg0 + 8:
		case k24018_arg9: {
			uint shade = eventId - k24018_arg0;
			_isInAnim[shade] = false;
			levelRender();
			break;
		}
		case 24019:
			if (persistent->_quest == kRescuePhilQuest) {
				g_vm->moveToRoom(kMonsterPuzzle);
			} else {
				_levelS++;
				if (_levelS < 15) {
					levelClear();
					loadLevel();
					levelRender();
					showCharon();
					break;
				}

				playCharonSound(charonNextLevel[_levelL], 24020);
				_levelL++;
				_levelS = 0;
			}
			break;
		case 24020:
			if (_levelL == 4)
				g_vm->moveToRoom(kWallOfFameRoom);
			break;
		case k24802_arg0:
			playCharonSound(charonFinishSounds[g_vm->getRnd().getRandomNumberRng(0, 2)], 24807);
			break;
		case k24802_arg1:
			hideCharon();
			room->playVideo("V9300bA0", kCharonZ, 24019,
					Common::Point(406, 68));
			break;
		case 24807:
			g_vm->addTimer(k24802_arg1, 500);
			break;
		case 24811:
			showCharon();
			break;
		case kCharonEndTalk:
			_charonIsBusy = false;
			break;
		case 24813: {
			g_vm->addTimer(24813, g_vm->getRnd().getRandomNumberRng(12000, 18000));
			charonIdle();
			break;
		}
		case k24801_arg1:
			playCharonSoundSMK(TranscribedSound::make("V9090NH0", "When you click on a shade you can see and I'll tell you what they think of one another"), k24801_arg2);
			_count24006 = 0;
			g_vm->addTimer(24006, 1200);
			break;
		case k24801_arg2:
			playCharonSoundSMK(TranscribedSound::make("V9090ND0", "If you click on a shade you can drag it into a seat"), k24801_arg3);
			break;
		case k24801_arg3:
			playCharonSoundSMK(TranscribedSound::make("V9090NE0", "If they find a neighbour offensive, they'll return to the dark"), k24801_arg4);
			break;
		case k24801_arg4:
			playCharonSoundSMK(TranscribedSound::make("V9090NI0", "To remove the shade from the boat click and drag it to the dark"), k24801_arg5);
			break;
		case k24801_arg5:
			playCharonSoundSMK(TranscribedSound::make("V9130NA0", "Do try and discover what is bothering each of them"), k24801_arg6);
			break;
		case k24801_arg6:
			room->enableMouse();
			showCharon();
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		room->addStaticLayer("V9010pA0", kBackgroundZ);
		room->selectFrame("V9010oA0", 540, 0);
		room->selectFrame("V9010oB0", 700, 0);
		// TODO: semi-transparency
		room->playAnimLoop("V9060bA0", 540);
		room->playAnimLoop("V9060bB0", 540);
		room->playAnimLoop("V9060bC0", 540);
		room->playVideo("V9010xA0", 0, 24002);
		room->loadHotZones("ff.hot", false);

		room->playMusicLoop("V9010eA0");
		room->playMusicLoop("V9300eA0");

		_levelL = 1;
		if (persistent->_quest == kRescuePhilQuest) {
			_levelS = 0;
		} else {
			_levelS = 1;
		}

		levelClear();
		loadLevel();
		levelRender();

		showCharon();
		g_vm->addTimer(24813, g_vm->getRnd().getRandomNumberRng(12000, 18000));
		for (uint i = 0; i < kNumSeats; i++) {
			g_vm->addTimer(k24017_arg0 + i, g_vm->getRnd().getRandomNumberRng(10200, 21800));
		}

		g_vm->getHeroBelt()->setColour(HeroBelt::kCold);

		if (persistent->_quest == kRescuePhilQuest) {
			room->disableMouse();
			// originally 24800
			playCharonSoundSMK(TranscribedSound::make("V9090NA0", "Oh, the dead are a tiresome lot. Charon can't bear to weed through their assorted woes. If you assist Charon by loading the shades onto the boat, Charon will squeeze you in for the ride across"), k24801_arg1);
		}
	}

	void frameCallback() override {
		if (_dragged != -1) {
			levelRender();
		}
	}

	bool handleCheat(const Common::String &cheat) override {
		if (cheat == "done") {
			win();
			return true;
		}

		if (cheat == "identify") {
			GUI::MessageDialog dialog(Common::String::format("l%ds%02d", _levelL, _levelS));
			dialog.runModal();
			return true;
		}

		if (cheat.matchString("l#s##")) {
			int l = atoi(cheat.substr(1, 1).c_str());
			int s = atoi(cheat.substr(3, 2).c_str());
			if (l < 1 || l > 3 || s < 0 || s > 15)
				return false;
			levelClear();
			_levelL = l;
			_levelS = s;
			loadLevel();
			levelRender();
			showCharon();
			return true;
		}

		if (cheat == "idle") {
			charonIdle();
			return true;
		}

		return false;
	}

private:
	void idleAnimShade(int shade, int z, int startFrame, int endFrame) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		LayerId sl(shadows[_shades[shade].shadowId].image, shade, "shadow");
		if (_isInAnim[shade])
			return;
		_isInAnim[shade] = true;
		// TODO: play sound multiple times if needed
		PlayAnimParams params(PlayAnimParams::keepLastFrame().partial(startFrame, endFrame));
		const char *snd = shadows[_shades[shade].shadowId].animSound;
		if (snd && snd[0] != 0)
			room->playAnimWithSFX(sl, snd, z, params, k24018_arg0 + shade, getShadowPos(shade));
		else
			room->playAnim(sl, z, params, k24018_arg0 + shade, getShadowPos(shade));
	}

	void hideCharon() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (uint i = 0; i < ARRAYSIZE(charonAnims); i++)
			room->stopAnim(charonAnims[i]);
		for (uint i = 0; i < ARRAYSIZE(charonIdleVideos); i++)
			room->stopAnim(charonIdleVideos[i].name);
		_charonIsBusy = true;
	}

	void showCharon() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		hideCharon();
		room->selectFrame(charonAnims[0], kCharonZ, 0);
		_charonIsBusy = false;
	}

	void playCharonSound(const TranscribedSound &sound,
			     EventHandlerWrapper ev = kCharonEndTalk, bool isSMK = false) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		int selected = -2;
		hideCharon();
		do {
			selected = g_vm->getRnd().getRandomNumberRng(0, ARRAYSIZE(charonAnims) - 1);
		} while (selected == _lastCharonAnim);
		_lastCharonAnim = selected;
		room->playAnim(charonAnims[selected], kCharonZ, PlayAnimParams::loop());
		if (isSMK)
			room->playVideo(sound.soundName, 0, ev);
		else
			room->playSpeech(sound, ev);
	}

	void playCharonSoundSMK(const TranscribedSound &sound,
				EventHandlerWrapper ev = kCharonEndTalk) {
		playCharonSound(sound, ev, true);
	}

	void hideThought() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->stopAnim("V9090oA0");
		for (unsigned i = 0; i < ARRAYSIZE(shadows); i++)
			room->stopAnim(shadows[i].nameImage);
		for (unsigned i = 0; i < ARRAYSIZE(thoughts); i++)
			room->stopAnim(thoughts[i].image);
		room->stopAnim("V9150tA0");
	}

	void showThoughtByShadowId(int id, bool silent = false) {
		Common::Point bubblePos;
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		int thoughtId = _shades[id].thoughtId;
		if (_shades[id].positionIsFixed && thoughtId == 82)
			thoughtId = -1;
		hideThought();
		bubblePos = getShadowPos(id) - Common::Point(71, 71);
		if (_shades[id].currentPos >= 0 && _shades[id].currentPos <= 4) {
			bubblePos += shadows[_shades[id].shadowId].getStarboardBubblePos();
		} else if (_shades[id].currentPos >= 5) {
			bubblePos += shadows[_shades[id].shadowId].getPortBubblePos();
		} else
			bubblePos += shadows[_shades[id].shadowId].getWaitBubblePos();
		bubblePos.x = MAX<int>(bubblePos.x, -10);
		bubblePos.y = MAX<int>(bubblePos.y, 0);
		if (!silent) {
			if (thoughtId >= 0)
				playCharonSound(thoughts[thoughtId].sound);
			else if (thoughtId == -1)
				playCharonSound(TranscribedSound::make("V9150nA0", "He won't move from that seat"));
		}
		room->selectFrame("V9090oA0", 112, 0, bubblePos);
		room->selectFrame(shadows[_shades[id].shadowId].nameImage, 111, 0, bubblePos);
		if (thoughtId >= 0)
			room->selectFrame(thoughts[thoughtId].image, 111, 0, bubblePos);
		else if (thoughtId == -1)
			room->selectFrame("V9150tA0", 111, 0, bubblePos);
		g_vm->addTimer(24014, 3000);
	}

	void moveToFerry(int shadeId, int ferryPos) {
		_shades[shadeId].currentPos = ferryPos;

		// First all other who are disgusted move out
		for (unsigned i = 0; i < _shades.size(); i++) {
			if (_shades[i].positionIsFixed || _shades[i].currentPos < 0
			    || (int) i == shadeId)
				continue;
			if (!checkCombinationIsAllowed(i))
				backToWaiting(i);
		}

		// Then the new shade
		if (!checkCombinationIsAllowed(shadeId))
			backToWaiting(shadeId);

		// Then everybody who is fixed can kick out new shade
		for (unsigned i = 0; i < _shades.size(); i++) {
			if (!_shades[i].positionIsFixed)
				continue;
			if (!checkCombinationIsAllowed(i))
				backToWaiting(shadeId);
		}

		bool isWon = true;
		for (unsigned i = 0; i < _shades.size(); i++)
			if (_shades[i].currentPos < 0)
				isWon = false;

		if (isWon) {
			win();
		}
	}

	void win() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		for (unsigned i = 0; i < ARRAYSIZE(shadows); i++)
			room->disableHotzone(Common::String::format("s%02d", i));
		for (unsigned i = 0; i < 2; i++)
			for (unsigned j = 0; j < 5; j++)
				room->disableHotzone(Common::String::format("f%01d%01d", i, j));
		g_vm->addTimer(k24802_arg0, 500);
	}

	void backToWaiting(int shadeId) {
		// TODO: anim
		_shades[shadeId].currentPos = -1;
	}

	bool isAttribMatch(Shade *shade, int attr) {
		if (attr == 0)
			return true;
		if (!shade)
			return false;
		return (shadows[shade->shadowId].attributes & attr) == attr;
	}

	void yuck() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_isPlayingYuck)
			return;
		_isPlayingYuck = true;
		room->playSpeech(yuckSounds[g_vm->getRnd().getRandomNumberRng(0, ARRAYSIZE(yuckSounds))],
					  24010);
	}

	bool checkCombinationIsAllowed(int shadeId) {
		Shade *cur = &_shades[shadeId];
		int curPos = cur->currentPos;
		int side = curPos / 5;
		int pos = curPos % 5;
		Shade *pNextTo1 = pos == 0 ? nullptr : posToShade(curPos-1);
		Shade *pNextTo2 = pos == 4 ? nullptr : posToShade(curPos+1);
		Shade *pAfrontOf = posToShade(pos + (side ? 0 : 5));
		bool pNextTo1Potential = pos != 0 && pNextTo1 == nullptr;
		bool pNextTo2Potential = pos != 4 && pNextTo2 == nullptr;

		// "Must sit next to"
		if (!isAttribMatch(pNextTo1, cur->tabN)
		    && !isAttribMatch(pNextTo2, cur->tabN)
		    && !pNextTo1Potential && !pNextTo2Potential) {
			return false;
		}

		// "Must not sit next to"
		if (cur->tabNN && (isAttribMatch(pNextTo1, cur->tabNN)
				   || isAttribMatch(pNextTo2, cur->tabNN))) {
			yuck();
			return false;
		}

		// "Must sit in front of"
		if (!isAttribMatch(pAfrontOf, cur->tabA)
		    && pAfrontOf != nullptr) {
			return false;
		}

		// "Must not sit in front of"
		if (cur->tabNA && isAttribMatch(pAfrontOf, cur->tabNA)) {
			yuck();
			return false;
		}

		// "Must sit in front or back"
		if ((cur->tabO & 1) && pos != 0 && pos != 4) {
			return false;
		}

		// Charon: only X in front
		if (pos == 4 && !isAttribMatch(cur, _charonN))
			return false;

		// Charon: no X in front
		if (pos == 4 && _charonNN && isAttribMatch(cur, _charonNN)) {
			yuck();
			return false;
		}

		// TODO: sound on "must" path

		return true;
	}

	int typeToIdx(int type) {
		for (unsigned i = 0; i < _shades.size(); i++) {
			if (_shades[i].shadowId == type)
				return i;
		}

		return -1;
	}

	Shade *posToShade(int pos) {
		for (unsigned i = 0; i < _shades.size(); i++) {
			if (_shades[i].currentPos == pos)
				return &_shades[i];
		}

		return nullptr;
	}


	void levelClear() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (unsigned i = 0; i < ARRAYSIZE(charonRules); i++) {
			room->stopAnim(charonRules[i].image);
		}
		for (unsigned i = 0; i < _shades.size(); i++) {
			room->stopAnim(LayerId(shadows[_shades[i].shadowId].image, i, "shadow"));
			room->stopAnim(shadows[_shades[i].shadowId].nameImage);
		}
		room->stopAnim("V9090oA0");
		for (unsigned i = 0; i < ARRAYSIZE(thoughts); i++) {
			room->stopAnim(thoughts[i].image);
		}
	}

	void levelRender() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->selectFrame(charonRules[_charonTID].image, 699, 0);
		room->setHotzoneEnabled("Sign", _charonTID != 12);
		room->enableHotzone("Charon");

		for (unsigned i = 0; i < ARRAYSIZE(shadows); i++)
			room->disableHotzone(Common::String::format("s%02d", i));

		bool ffUsed[10];

		memset(ffUsed, 0, sizeof(ffUsed));

		for (uint i = 0; i < _shades.size(); i++) {
			if ((int) i == _dragged) {
				Common::Point pos = g_vm->getMousePos();
				room->selectFrame(LayerId(shadows[_shades[i].shadowId].image,
							  i, "shadow"), 0, 0, pos - Common::Point(88, 160));
				_isInAnim[i] = false;
				continue;
			}

			if (_isInAnim[i])
				continue;

			if (_shades[i].currentPos >= 0) {
				LayerId sl(shadows[_shades[i].shadowId].image,
					   i, "shadow");
				int frame = _shades[i].currentPos / 5 ? shadows[_shades[i].shadowId].portFrame
					: shadows[_shades[i].shadowId].starboardFrame;
				room->selectFrame(sl, boatPosition[_shades[i].currentPos].z,
						  frame, getShadowPos(i));
				ffUsed[_shades[i].currentPos] = true;
				continue;
			}

			if (_shades[i].waitingPos >= 0) {
				Common::Point pos = getShadowPos(i);
				Common::String hz = Common::String::format("s%02d", _shades[i].shadowId);
				room->enableHotzone(hz);
				room->setHotZoneOffset(hz, waitPosition[_shades[i].waitingPos].getPoint()
						       - Common::Point(88, 160) + Common::Point(0, shadows[_shades[i].shadowId].waityoffset));
				room->selectFrame(LayerId(shadows[_shades[i].shadowId].image,
							  i, "shadow"),
						  waitPosition[_shades[i].waitingPos].z, 0, pos);
				continue;
			}
		}

		for (unsigned i = 0; i < 2; i++)
			for (unsigned j = 0; j < 5; j++)
				room->setHotzoneEnabled(Common::String::format("f%01d%01d", i, j),
							_dragged != -1 ? !ffUsed[i*5 + j] : ffUsed[i*5 + j]);
	}

	Common::Point getShadowPos(unsigned idx) {
		if (_shades[idx].currentPos >= 0) {
			Common::Point delta;
			if (_shades[idx].shadowId == 14 || _shades[idx].shadowId == 20)
				delta = Common::Point(0, -35);
			return boatPosition[_shades[idx].currentPos].getPoint()
				- Common::Point(88, 160) + delta;
		}

		if (_shades[idx].waitingPos >= 0) {
			return waitPosition[_shades[idx].waitingPos].getPoint()
				- Common::Point(88, 160) + Common::Point(0, shadows[_shades[idx].shadowId].waityoffset);
		}

		return Common::Point(0, 0);
	}

	void loadLevel() {
		TextTable _levelTable;
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_levelTable = TextTable(
			Common::SharedPtr<Common::SeekableReadStream>(
				room->openFile(
					Common::String::format("l%ds%02d.ff", _levelL, _levelS))),
			14);
		_shades.clear();
		for (int i = 0; i < _levelTable.size(); i++) {
			if (_levelTable.get(i, "Num") == "") {
				_charonTID = _levelTable.get(i, "TID").asUint64Ext();
				_charonN = _levelTable.get(i, "N").asUint64Ext();
				_charonNN = _levelTable.get(i, "NN").asUint64Ext();
				continue;
			}

			Shade shade;
			shade.shadowId = _levelTable.get(i, "Num").asUint64Ext();
			shade.thoughtId = _levelTable.get(i, "TID").asUint64Ext();
			if (_levelTable.get(i, "BoatPos") == "" || _levelTable.get(i, "BoatPos") == "0") {
				shade.currentPos = -1;
				shade.positionIsFixed = false;
			} else {
				shade.currentPos = _levelTable.get(i, "BoatPos").asUint64Ext() - 1;
				shade.positionIsFixed = true;
			}
			shade.tabNA = _levelTable.get(i, "NA").asUint64Ext();
			shade.tabNN = _levelTable.get(i, "NN").asUint64Ext();
			shade.tabN = _levelTable.get(i, "N").asUint64Ext();
			shade.tabO = _levelTable.get(i, "O").asUint64Ext();
			shade.tabA = _levelTable.get(i, "A").asUint64Ext();
			shade.waitingPos = -1;
			shade.rowidx = _shades.size();
			_shades.push_back(shade);
		}

		for (int j = 0; j < 2; j++) {
			int m = 100000, mpos = -1;
			for (unsigned i = j; i < _shades.size(); i++) {
				if (shadows[_shades[i].shadowId].priorityleft <= m && !_shades[i].positionIsFixed) {
					m = shadows[_shades[i].shadowId].priorityleft;
					mpos = i;
				}
			}
			if (shadows[_shades[j].shadowId].priorityleft <= m && !_shades[j].positionIsFixed)
				continue;
			Shade t = _shades[mpos];
			_shades[mpos] = _shades[j];
			_shades[j] = t;
		}

		for (int j = 0; j < 2; j++) {
			int m = 100000, mpos = -1;
			for (unsigned i = 2; i < _shades.size() - j; i++) {
				if (shadows[_shades[i].shadowId].priorityright <= m && !_shades[i].positionIsFixed) {
					m = shadows[_shades[i].shadowId].priorityright;
					mpos = i;
				}
			}
			if (shadows[_shades[_shades.size() - j - 1].shadowId].priorityright <= m
				&& !_shades[_shades.size() - j - 1].positionIsFixed)
				continue;
			Shade t = _shades[mpos];
			_shades[mpos] = _shades[_shades.size() - j - 1];
			_shades[_shades.size() - j - 1] = t;
		}

		int waitidx = 0;

		for (unsigned i = 0; i < _shades.size(); i++) {
			if (!_shades[i].positionIsFixed)
				_shades[i].waitingPos = waitidx;
			debug("%s, leftpriority %d, rightpriority %d, idx %d, res %d",
			      shadows[_shades[i].shadowId].name,
			      shadows[_shades[i].shadowId].priorityleft,
			      shadows[_shades[i].shadowId].priorityright,
			      _shades[i].rowidx,
			      _shades[i].waitingPos);
			waitidx++;
		}
	}
	int _levelL;
	int _levelS;
	int _charonTID;
	int _charonN;
	int _charonNN;
	int _dragged;
	int _clickTimer;
	int _lastCharonAnim;
	int _count24006;
	bool _isPlayingYuck;
	bool _charonIsBusy;
	Common::Array<Shade> _shades;
	bool _isInAnim[kNumSeats];
};

Common::SharedPtr<Hadesch::Handler> makeFerryHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new FerryHandler());
}

}
