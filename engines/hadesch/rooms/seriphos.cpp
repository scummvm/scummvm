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
static const char *kApolloHighlight = "c7400ba0";
static const char *kArtemisHighlight = "c7410ba0";
static const char *kDemeterHighlight = "c7420ba0";
static const char *kStrawCartFull = "c7100ba0";
static const char *kStrawCartEmpty = "c7100bb0";
static const char *kStrawCartHotzone = "Straw Cart";

static const char *questHovelNames[] = {
	"",
	"HovelsCrete",
	"HovelsTroy",
	"HovelsMedusa",
	"HovelsPhil"
};

// TODO: fill this
static const TranscribedSound seClickTranscript[] = {
	{"c7320wb0", _s("You know, you've got to say one thing for that Oedipus: he loved his mother")},
	{"c7320wc0", _s("Did you ever wonder: if Atlas is holding up the world, what's the heck is he standing on?") },
	{"c7320wd0", _s("Do you know ho many narcisses it takes to screw in an oil lamp? One. He holds the lamp and the world revolves around him") },
	{"c7320we0", _s("My dear, these hovels are small. Even the mice are crammed") },
	{"c7320wf0", _s("Happiness is seeing king Polydectes' picture on the side of a milk bucket") },
	{"c7330xa0", _s("You know what would look really good on king Polydectes? A pitbull")}, // unclear: I'm unable to hear beginning of the utterance
	{"c7330xc0", _s("That Perseus kid: brave, strong, steady as rock. And if he takes one look at Medusa and it's where he's gonna be. Yeah, well, keep your fingers crossed, he's all over in snakes right now.")},
	{"c7340xa0", _s("Did you hear Daedalus is building a huge labyrinth to catch Minotaur? Works great except now he can't find his way out. I heard he's building some wings made of wax. He-he. Good luch getting that idea off the ground")},
	{"c7340xc0", _s("Boy our king is mean. Did you know that when Oedipus went blind the king rearranged the furniture? But at least we're not as bad off as the Crete: they have a rotten king, the dangerous Minotaur and lousy parking. Good luck finding a place for your chariot")},
	{"c7350xa0", _s("That beautiful Helen is still being held captive in Troy. How awful for her. They've got such a lousy shopping there. When is this trojan war going to be over? Then maybe we'll start peloponesean war.")},
	{"c7350xc0", _s("Gee, Odysseus failed to get into the city. Helen is still a prisonner and morale is low. What else can go wrong? He's just found out his chariot needs new shocks. That's gonna be expensive")},
	{"c7360xa0", _s("Oh, it's a good thing Perseus killed Medusa otherwise Polydectes would still be king. Perseus is a much better king than Polydectes was. Now that I think of it, my dog would be a much better king than Polydectes was")},
//	"c7360wc0"
//	"c7360wd0"
//	"c7310xc0"
//	"c7310xa0"
//	"c7310xb0"
//	"c7310xd0"
//	"c7310xe0"
//	"c7310xf0"
//	"c7310xh0"
//	"c7310xg0"
	{ nullptr, nullptr }
};

enum {
	kAnimationCompleted = 26007,
      	// 26008 is the end of statue animation that we handle as functor instead
	kStrawTaken = 26020,
	kStrawTakenCleanup = 26021,

	kIdlesTick = 1026001,
	kHovelsCompleted = 1026002
};

enum {
	kBackgroundZ = 10000,
	kCartZ = 101,
	kStatuesZ = 101,
	kPhilZ = 0
};

class SeriphosHandler : public Handler {
public:
	SeriphosHandler() {
		_hovelsCounter = -1;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		if (name == "Apollo") {
			Common::Array<Common::String> videos;
			videos.push_back("c7400na0");
			videos.push_back("c7400nb0");
			videos.push_back("c7400nc0");

			room->playStatueSMK(kApolloStatue,
					    kApolloHighlight,
					    kStatuesZ,
					    videos, 33, 49);
			return;
		}
		if (name == "Artemis") {
			Common::Array<Common::String> videos;
			videos.push_back("c7410na0");
			videos.push_back("c7410nb0");

			room->playStatueSMK(kArtemisStatue,
					    kArtemisHighlight,
					    kStatuesZ,
					    videos, 25, 46);
			return;
		}
		if (name == "Demeter") {
			Common::Array<Common::String> videos;
			videos.push_back("c7420na0");
			videos.push_back("c7420nb0");

			room->playStatueSMK(kDemeterStatue,
					    kDemeterHighlight,
					    kStatuesZ,
					    videos, 25, 45);
			return;
		}

		if (name == "Argo") {
			if (quest == kMedusaQuest && !persistent->_seriphosPhilWarnedAthena
			    && (!persistent->_athenaSwordTaken || !persistent->_athenaShieldTaken)
			    && persistent->_hintsAreEnabled) {
				room->disableMouse();
				persistent->_seriphosPhilWarnedAthena = true;
				room->playVideo("c7300ba0", 0, 26022, Common::Point(0, 216));
				return;
			}
			g_vm->moveToRoom(kArgoRoom);
			return;
		}

		if (name == "Athena's Temple") {
			g_vm->moveToRoom(kAthenaRoom);
			return;
		}

		if (name == kStrawCartHotzone) {
			room->selectFrame(kStrawCartEmpty, kCartZ, 0);
			_seIdles.hide(kStrawCartFull);
			room->playMusic("c7380mb0");
			g_vm->getHeroBelt()->placeToInventory(kStraw, kStrawTaken);
			room->disableHotzone(kStrawCartHotzone);
			room->disableMouse();
			return;
		}

		if ((name == "Curtains" || name == "Hovels")
		    && (quest == kMedusaQuest
			&& (persistent->_athenaSwordTaken && persistent->_athenaShieldTaken)
			&& g_vm->getPreviousRoomId() == kAthenaRoom
			&& !persistent->_seriphosPhilCurtainsItems)) {
			room->disableMouse();
			persistent->_seriphosPhilCurtainsItems = true;
			room->playVideo("c7370ba0", 0, 26023, Common::Point(0, 216));
			return;
		}

		if (name == "Curtains") {
			_ambients.hide("c7160ba0");

			if (quest < kMedusaQuest) {
				_seClick.playNext("CurtainsBeforeMedusa", kAnimationCompleted);
			} else if (quest == kMedusaQuest) {
				_seClick.playNext("CurtainsDuringMedusa", kAnimationCompleted);
			} else if (quest > kMedusaQuest) {
				_seClick.playNext("CurtainsAfterMedusa", kAnimationCompleted);
			}
			return;
		}

		if (name == "Hovels") {
			int genericidx = -1;
			_hovelsCounter++;
			room->disableMouse();
			room->playAnimWithSFX("c7320ba0", "C7320EA0", 3101, PlayAnimParams::loop());
			switch(persistent->_quest) {
			case kRescuePhilQuest:
				if (_hovelsCounter == 1) {
					_seClick.playChosen(questHovelNames[persistent->_quest],
							    persistent->_gender == kFemale ? 2 : 1, kHovelsCompleted);
					return;
				}
				// Fallthrough
			case kCreteQuest:
			case kTroyQuest:
			case kMedusaQuest:
				if (_hovelsCounter < 2) {
					_seClick.playChosen(questHovelNames[persistent->_quest], _hovelsCounter, kHovelsCompleted);
					return;
				}
				genericidx = _hovelsCounter - 2;
				break;
			default:
				genericidx = _hovelsCounter;
				break;
			}

			_seClick.playChosen("HovelsGeneric", genericidx, kHovelsCompleted);
			if (genericidx == 4)
				_hovelsCounter = -1;
			return;
		}
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		switch (eventId) {
		case 2803:
			_ambients.tick();
			break;
		case 26009:
			room->playMusic("c7290ma0", 26012);
			room->playVideo("c7290ba0", 111, 26010, Common::Point(90, 76));
			break;
		case 26010:
			room->playSFX("c7160ea0", 26013);
			break;
		case 26013:
			room->playVideo("c7290bd0", 111, 26014, Common::Point(92, 76));
			break;
		case 26014:
			room->enableMouse();
			startIdles();
			break;
		case kStrawTaken:
			room->playVideo("c7380ba0", kPhilZ, kStrawTakenCleanup, Common::Point(0, 216));
			break;
		case 26022:
		case 26023:
		case kStrawTakenCleanup:
			room->enableMouse();
			break;
		case kAnimationCompleted:
			room->enableMouse();
			_ambients.unpauseAndFirstFrame("c7160ba0");
			break;
		case kIdlesTick:
			_seIdles.tick();
			break;
		case kHovelsCompleted:
			room->enableMouse();
			room->stopAnim("c7320ba0");
			room->selectFrame("c7320ba0", 3101, 0);
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		room->loadHotZones("Seriphos.HOT", true);
		room->addStaticLayer("c7010pa0", kBackgroundZ);

		Common::String seAmbFn = quest > kMedusaQuest ? "SeAmb2.txt" : "SeAmb.txt";
		TextTable seAmb = TextTable(
			Common::SharedPtr<Common::SeekableReadStream>(room->openFile(seAmbFn)), 9);
		_ambients.readTableFileSFX(seAmb, AmbientAnim::PAN_ANY);

		if (quest == kMedusaQuest && !persistent->_seriphosPlayedMedusa) {
			g_vm->addTimer(26009, 500, 1);
			persistent->_seriphosPlayedMedusa = true;
		} else
			startIdles();
		if (quest > kMedusaQuest) {
			room->selectFrame("c7010oa0", 102, 0);
			room->selectFrame("c7010ta0", 101, 0);
			room->selectFrame("c7010ob0", 3101, 0);
			room->playAnimLoop("c7320bb0", 1101);
		}

		room->playMusicLoop(quest > kMedusaQuest ? "c7010eb0" : "c7010ea0");

		_seClick.readTable(room, "SeClick.txt", seClickTranscript);

		g_vm->getHeroBelt()->setColour(quest > kMedusaQuest ? HeroBelt::kWarm : HeroBelt::kCool);

		TextTable seIdles = TextTable(
				Common::SharedPtr<Common::SeekableReadStream>(room->openFile("SeIdles.txt")), 14);
		_seIdles.readTableFileSFX(seIdles, AmbientAnim::PAN_ANY);
		_seIdles.firstFrame();

		if (quest == kCreteQuest && !persistent->_seriphosStrawCartTaken) {
			_seIdles.unpauseAndFirstFrame(kStrawCartFull);
			room->stopAnim(kStrawCartEmpty);
		} else {
			room->selectFrame(kStrawCartEmpty, kCartZ, 0);
			_seIdles.hide(kStrawCartFull);
			room->disableHotzone(kStrawCartHotzone);
		}

		room->playAnimLoop("c7110bb0", 2101);
		room->playAnimLoop("c7110bc0", 2101);
		room->playAnimLoop("c7180ba0", 3101);
		room->selectFrame("c7320ba0", 3101, 0);
	}
private:
	void startIdles() {
		g_vm->addTimer(2803, 10000, -1);
		_ambients.firstFrame();
		g_vm->addTimer(kIdlesTick, 6000, -1);
		_seIdles.firstFrame();
	}

	AmbientAnimWeightedSet _ambients;
	AmbientAnimWeightedSet _seIdles;
	AnimClickables _seClick;
	int _hovelsCounter;
};

Common::SharedPtr<Hadesch::Handler> makeSeriphosHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new SeriphosHandler());
}

}
