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

static const char * eyeOfFateVolcanoToStyx[] = {
	"eye of fates too good to be true",
	"eye of fates never ever",
	"eye of fates this place again"
};

enum {
	kBackgroundZ = 10000,
	kHelmetZ = 550
};

class VolcanoHandler : public Handler {
public:
	VolcanoHandler() {
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (name == "argo") {
			room->disableMouse();
			g_vm->moveToRoom(kArgoRoom);
			return;
		}

		if (name == "first boulder") {
			if (persistent->_quest != kMedusaQuest || persistent->_volcanoPuzzleState != Persistent::VOLCANO_NO_BOULDERS_THROWN) {
				_boulder1Anim.play(false);
				return;
			}

			room->disableMouse();
			room->disableHotzone("first boulder");
			room->stopAnim("lever gem");
			room->stopAnim("pain still");
			room->stopAnim("panic still");
			_boulder1Anim.hide();
			room->playAnimWithSFX("first boulder falls", "first boulder falls sound", 400,
					      PlayAnimParams::disappear(), 16023);
			return;
		}

		if (name == "second boulder") {
			for (int i = 1; i <= 3; i++) {
				room->stopAnim(Common::String::format("lava flow %d", i));
			}
			room->stopAnim("second boulder");
			room->stopAnim("lever gem");
			_painAnim.hide();
			_panicAnim.hide();
			room->playVideo("plug volcano movie", 0, 16025);
			persistent->_volcanoPuzzleState = Persistent::VOLCANO_BOULDER_ON_VOLCANO;
			return;
		}

		if (name == "helmet") {
			persistent->_volcanoPuzzleState = Persistent::VOLCANO_HELMET_SHOWN;
			g_vm->getHeroBelt()->placeToInventory(kHelmet);
			room->playSFX("skeleton revealed sfx");
			room->playAnimKeepLastFrame("helmet", kHelmetZ);
			return;
		}

		if (name == "near river styx" || name == "near river styx bg") {
			if (persistent->_quest != kRescuePhilQuest
			    && persistent->_volcanoToStyxCounter < ARRAYSIZE(eyeOfFateVolcanoToStyx)) {
				room->disableMouse();
				room->playVideo(eyeOfFateVolcanoToStyx[persistent->_volcanoToStyxCounter], 0, 16007, Common::Point(0, 216));
				persistent->_volcanoToStyxCounter++;
			} else {
				handleEvent(16007);
			}
			return;
		}
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		switch (eventId) {
		case 16005:
			room->playAnimLoop("black lava pool", 500);
			AmbientAnim("bubble", "bubble sound", 500, 1000, 2000,
				    AmbientAnim::DISAPPEAR, Common::Point(0, 0), AmbientAnim::PAN_ANY).start();
			g_vm->addTimer(16008, 4000); // TODO: pan down
			break;
		case 16007:
			room->stopAnim("gem overlay");
			room->playMusic("morph music", 16009);
			room->playVideo("morphing gems", 500, 16005, Common::Point(10, 10));
			break;
		case 16008:
			g_vm->moveToRoom(kRiverStyxRoom);
			break;
		case 16011:
			room->enableMouse();
			break;
		case 16016:
			if (!persistent->_volcanoHeyKid
			    && persistent->_hintsAreEnabled) {
				persistent->_volcanoHeyKid = true;
				room->playVideo("eye of fates hey kid", 0, 16011, Common::Point(0, 216));
			} else {
				room->enableMouse();
			}
			room->selectFrame("panic still", 424, 0);
			room->selectFrame("pain still", 425, 0);
			break;
		case 16023:
			squashedPanic();
			room->playAnimWithSFX("second boulder", "second boulder sound",
					      401, PlayAnimParams::keepLastFrame(), 16024);
			persistent->_volcanoPuzzleState = Persistent::VOLCANO_SQUASHED_PANIC;
			break;
		case 16024:
			if (persistent->_hintsAreEnabled)
				room->playVideo("eye of fates on ta something", 0, 16011, Common::Point(0, 216));
			else
				room->enableMouse();
			room->enableHotzone("second boulder");
			break;
		case 16025:
			room->playSFX("volcanic rumble", 16028);
			room->selectFrame("volcano plug boulder", 400, 0);
			g_vm->addTimer(16026, 500);
			break;
		case 16026:
			// TODO: screen shake
			g_vm->addTimer(16030, 2500);
			break;
		case 16030:
			room->playSFX("explosion sound", 16033);
			g_vm->addTimer(16031, 1000);
			break;
		case 16031:
			room->playAnim("explosion", 400, PlayAnimParams::disappear(), 16032);
			break;
			// TODO: what's the difference?
		case 16032:
		case 16034:
			room->selectFrame("helmet", kHelmetZ, 0);
			room->enableHotzone("helmet");
			// TODO: 16035 timer
			// TODO: Or is it "lava flow sound 1"
			room->playAnimWithSFX("final lava flow", "final lava flow sound", 500,
					      PlayAnimParams::loop());
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		room->loadHotZones("Volcano.hot", false);
		room->addStaticLayer("background", kBackgroundZ);
		room->selectFrame("gem overlay", 500, 0);
		room->selectFrame("argo", 1000, 0);
		room->enableHotzone("argo");
		if (quest == kMedusaQuest && persistent->_medisleShowFates && !persistent->_volcanoPainAndPanicIntroDone) {
			persistent->_volcanoPainAndPanicIntroDone = true;
			room->playVideo("pain and panic intro movie", 425, 16016, Common::Point(422, 165));
		}

		if (quest < kMedusaQuest) {
			if (!persistent->isRoomVisited(kVolcanoRoom)) {
				room->playVideo("eye of fates we're rich", 0, 16010, Common::Point(0, 216));
			}
		}

		if (quest < kMedusaQuest || (quest == kMedusaQuest && persistent->_volcanoPuzzleState == Persistent::VOLCANO_NO_BOULDERS_THROWN)) {
			room->selectFrame("lever gem", 450, 0);
			_boulder1Anim = AmbientAnim("first boulder", "first boulder sound", 400, 5000, 10000,
						    AmbientAnim::KEEP_LOOP, Common::Point(0, 0),
						    AmbientAnim::PAN_ANY);
			_boulder1Anim.start();
			room->selectFrame("second boulder", 401, 0);
		}

		if (quest < kMedusaQuest || (quest == kMedusaQuest && persistent->_volcanoPuzzleState <= Persistent::VOLCANO_SQUASHED_PANIC)) {
			for (int i = 1; i <= 3; i++) {
				room->playAnimWithSFX(Common::String::format("lava flow %d", i),
						      Common::String::format("lava flow sound %d", i),
						      500, PlayAnimParams::loop());
			}
		}

		if (quest == kMedusaQuest && persistent->_volcanoPuzzleState == Persistent::VOLCANO_NO_BOULDERS_THROWN) {
			room->enableHotzone("first boulder");
		}

		if (quest == kMedusaQuest && persistent->_volcanoPuzzleState == Persistent::VOLCANO_SQUASHED_PANIC) {
			room->enableHotzone("second boulder");
			squashedPanic();
			room->selectFrame("second boulder", 401, -1);
		}

		if (quest == kMedusaQuest && persistent->_volcanoPuzzleState == Persistent::VOLCANO_BOULDER_ON_VOLCANO) {
			room->selectFrame("helmet", kHelmetZ, 0);
			room->enableHotzone("helmet");
			// TODO: Or is it "lava flow sound 1"
			room->playAnimWithSFX("final lava flow", "final lava flow sound", 500,
					      PlayAnimParams::loop());
			room->selectFrame("volcano plug boulder", 400, 0);
		}

		if (quest > kMedusaQuest || (quest == kMedusaQuest && persistent->_volcanoPuzzleState == Persistent::VOLCANO_HELMET_SHOWN)) {
			room->selectFrame("helmet", kHelmetZ, -1);
			room->playAnimWithSFX("final lava flow", "final lava flow sound", 500,
					      PlayAnimParams::loop());
			room->selectFrame("volcano plug boulder", 400, 0);
		}

		for (int i = 1; i <= 3; i++) {
			AmbientAnim(Common::String::format("sparkle %d", i), "", 475, 5000, 10000,
				    AmbientAnim::DISAPPEAR,
				    Common::Point(0,0), AmbientAnim::PAN_ANY).start();
		}

		if (quest == kRescuePhilQuest) {
			room->playMusic("theme music");
		} else {
			room->playMusicLoop("W1010eA0");
		}

		room->playAnimLoop("waves", 900);
		room->enableHotzone("near river styx");
		room->enableHotzone("near river styx bg");

		g_vm->getHeroBelt()->setColour(HeroBelt::kWarm);
	}

private:
	void squashedPanic() {
		// TODO: transcribe and change to speech sound type
		_painAnim = AmbientAnim("squashed pain", "squashed pain sound", 425, 5000, 10000, AmbientAnim::KEEP_LOOP,
					Common::Point(0, 0), AmbientAnim::PAN_ANY);
		_panicAnim = AmbientAnim("squashed panic", "squashed panic sound", 425, 5000, 10000, AmbientAnim::KEEP_LOOP,
					 Common::Point(0, 0), AmbientAnim::PAN_ANY);
		_painAnim.start();
		_panicAnim.start();
	}

	AmbientAnim _boulder1Anim;
	AmbientAnim _painAnim, _panicAnim;
};

Common::SharedPtr<Hadesch::Handler> makeVolcanoHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new VolcanoHandler());
}

}
