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

enum {
	kBackgroundZ = 10000
};

enum {
	kDeadManEndAnim = 28014
};

struct StyxShadeInternal {
	StyxShadeInternal(Common::String name) {
		_name = name;
		_counter = 0;
	}

	void resume() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_ambient.unpause();
		room->enableMouse();
	}
	
	Common::String _name;
	int _counter;
	AmbientAnim _ambient;
	Common::Array<Common::String> _sounds;
};

class StyxShadeEndSound : public EventHandler {
public:
	StyxShadeEndSound(Common::SharedPtr<StyxShadeInternal> internal) {
		_internal = internal;
	}
	void operator()() {
		_internal->resume();
	}
private:
	Common::SharedPtr<StyxShadeInternal> _internal;
};

// TODO: transparency and shimmering
class StyxShade {
public:
	StyxShade(const Common::String &name, int zVal, int minInt, int maxInt,
		  const Common::String &ambient) {
		_internal = makeInternal(name, zVal, minInt, maxInt, ambient);
	}

	StyxShade(const Common::String &name, int zVal, int minInt, int maxInt) {
		_internal = makeInternal(name, zVal, minInt, maxInt, name + " ambient");
	}

	StyxShade() {
	}

	static Common::SharedPtr<StyxShadeInternal> makeInternal(const Common::String &name, int zVal, int minInt, int maxInt,
							     const Common::String &ambient) {
		Common::SharedPtr<StyxShadeInternal> ret(new StyxShadeInternal(name));
		ret->_ambient = AmbientAnim(ambient, ambient + " sound", zVal, minInt, maxInt,
					    AmbientAnim::KEEP_LOOP, Common::Point(0, 0), AmbientAnim::PAN_ANY);
		return ret;
	}

	void start() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_internal->_ambient.start();
		room->enableHotzone(_internal->_name);
	}

	void click() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_internal->_sounds.empty())
			return;
		_internal->_ambient.pause();
		room->playVideo(_internal->_sounds[_internal->_counter % _internal->_sounds.size()],
				800, EventHandlerWrapper(Common::SharedPtr<EventHandler>(new StyxShadeEndSound(_internal))));
		_internal->_counter++;
		room->disableMouse();
	}

	void addSound(const Common::String &snd) {
		_internal->_sounds.push_back(snd);
	}
private:
	Common::SharedPtr<StyxShadeInternal> _internal;
};

class RiverStyxHandler : public Handler {
public:
	RiverStyxHandler() {
		_charonSound = false;
		_cameraMovingDown = false;
		_cameraMovingUp = false;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (name == "volcano top") {
			room->disableMouse();
			room->playAnimWithSFX("morphing gems", "morphing gems sound",
					      1000, PlayAnimParams::keepLastFrame().backwards(), 28018);
			return;
		}

		if (name == "charon") {
			// Originally it goes through event 28002, 1
			if (persistent->_styxCharonUsedPotion) {
				room->playVideo("charon assumes you have gold sound", 0, 28004);
			} else {
				
				room->playVideo(_charonSound ? "charon says away 2 sound" : "charon says away 1 sound", 0, 28004);
			}
			return;
		}

		if (name == "ax head") {
			_axHead.click();
			return;
		}

		if (name == "statue") {
			_statue.click();
			return;
		}

		if (name == "pillar") {
			_pillar.click();
			return;
		}

		if (name == "dog") {
			_dog.click();
			return;
		}

		if (name == "drowned man") {
			_drownedMan.click();
			return;
		}

		if (name == "trojan soldier") {
			_trojanSoldier.click();
			return;
		}

		if (name == "greek soldier") {
			_greekSoldier.click();
			return;
		}

		if (name == "alchemist") {
			_alchemist.click();
			return;
		}
	}

	bool handleClickWithItem(const Common::String &name, InventoryItem item) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		if (name == "charon" && item == kPotion) {
			// Originally event 28002, kPotion
			room->disableMouse();
			g_vm->getHeroBelt()->removeFromInventory(item);
			_charon.hide();
			room->playVideo("charon glow", 549, 28005, Common::Point(516, 93));
			g_vm->addTimer(28006, 2000, 1);
			persistent->_styxCharonUsedPotion = true;
			return true;
		}
		if (name == "charon" && item == kCoin) {
			// Originally event 28002, kCoin
			room->disableMouse();
			g_vm->getHeroBelt()->removeFromInventory(item);
			_charon.hide();
			room->playVideo("change purse", 549, 28010, Common::Point(524, 100));
			g_vm->addTimer(28008, 1000, 1);
			persistent->_styxCharonUsedCoin = true;
			return true;
		}

		return false;
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		switch(eventId) {
		case 28004:
			stopCharonTalk();
			if (persistent->_styxCharonUsedPotion && persistent->_styxCharonUsedCoin) {
				_charon.hide();
				room->playVideo("charon asks for help", 549, 28011, Common::Point(452, 96));
			} else {
				room->enableMouse();
			}
			break;
		case 28005:
			playCharonTalk("charon says quite dead sound", 28004);
			break;
		case 28006:
			room->playMusic("charon glow sting", 28007);
			break;
		case 28008:
			room->playMusic("charon accepts coin sting", 28009);
			break;
		case 28009:
			if (persistent->_styxCharonUsedPotion && persistent->_styxCharonUsedCoin) {
				handleEvent(28004);
			} else {
				playCharonTalk("charon takes an advance sound", 28004);
			}
			break;
		case 28010:
			_charon.unpauseAndFirstFrame();
			break;
		case 28011:
			_charon.unpauseAndFirstFrame();
			g_vm->moveToRoom(kFerrymanPuzzle);
			break;
		case 28017:
			if (persistent->_quest == kRescuePhilQuest && !persistent->_styxAlchemistSaidIntro) {
				persistent->_styxAlchemistSaidIntro = true;
				_alchemist.click();
			} else
				room->enableMouse();
			break;
		case kDeadManEndAnim:
			break;
		case 28018:
			room->selectFrame("overlay fade to volcano top", 1000, 0, Common::Point(0, -50));
			room->stopAnim("overlay fade from volcano top");
			_cameraMovingUp = true;
			_cameraMovingStart = g_vm->getCurrentTime();
			break;
		case 28019:
			g_vm->moveToRoom(kVolcanoRoom);
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		room->loadHotZones("NearRivr.HOT", false);
		room->enableHotzone("volcano top");
		room->addStaticLayer("background", kBackgroundZ, Common::Point(0, -50));
		room->selectFrame("sign text", 900, 0, Common::Point(0, -50));

		AmbientAnim("bats 1", "bats 1 sound", 925, 8000, 12000,
			    AmbientAnim::DISAPPEAR, Common::Point(0, 0), AmbientAnim::PAN_ANY)
			.start();
		AmbientAnim("bats 2", "bats 2 sound", 925, 8000, 12000,
			    AmbientAnim::DISAPPEAR, Common::Point(0, 0), AmbientAnim::PAN_ANY)
			.start();
		AmbientAnim("terminal bats", "terminal bats sound", 925, 8000, 12000,
			    AmbientAnim::BACK_AND_FORTH, Common::Point(0, 0), AmbientAnim::PAN_ANY)
			.start();
		room->playAnimLoop("mist 1", 950);
		room->playAnimLoop("mist 2", 950);
		room->playAnimLoop("water", 951);
		room->playAnimLoop("group of shades", 950);
		room->selectFrame("morphing gems", 1000, -1);

		if (quest == kRescuePhilQuest) {
			room->selectFrame("ferry", 975, 0, Common::Point(0, -50));
			_charon = AmbientAnim("charon", "charon sound", 550,
				    5000, 10000, AmbientAnim::KEEP_LOOP, Common::Point(0, 0),
				    AmbientAnim::PAN_ANY);
			_charon.start();
			room->enableHotzone("charon");
		}

		room->playMusicLoop(quest == kRescuePhilQuest ? "V4010eB0" : "V4010eA0");
		_axHead = StyxShade("ax head", 800, 5000, 10000);
		_axHead.addSound("ax head click sound 1");
		_axHead.addSound("ax head click sound 2");
		_axHead.addSound("ax head click sound 3");
		_axHead.start();

		if (quest == kRescuePhilQuest || quest == kCreteQuest) {
			_pillar = StyxShade("pillar", 550, 8000, 12000);
			if (quest == kRescuePhilQuest)
				_pillar.addSound("pillar quest speech");
			_pillar.addSound("pillar click sound");
			_pillar.start();
		}

		if (quest == kCreteQuest || quest == kTroyQuest || quest == kMedusaQuest) {
			_dog = StyxShade("dog", 600, 5000, 10000);
			if (quest == kCreteQuest)
				_dog.addSound("dog quest speech");
			_dog.addSound("dog click sound 1");
			_dog.addSound("dog click sound 2");
			_dog.start();
		}

		if (quest == kCreteQuest || quest == kTroyQuest) {
			_greekSoldier = StyxShade("greek soldier", 550, 5000, 10000);
			if (quest == kTroyQuest)
				_greekSoldier.addSound("greek soldier quest speech");
			_greekSoldier.addSound("greek soldier click sound");
			_greekSoldier.start();
		}

		if (quest == kTroyQuest) {
			_trojanSoldier = StyxShade("trojan soldier", 650, 5000, 10000);
			_trojanSoldier.addSound("trojan soldier quest speech");
			_trojanSoldier.start();
		}

		if (quest == kMedusaQuest) {
			_statue = StyxShade("statue", 700, 5000, 10000);
			_statue.addSound("statue quest speech");
			_statue.start();

			_drownedMan = StyxShade("drowned man", 550, 5000, 10000);
			_drownedMan.addSound("drowned man click sound 1");
			_drownedMan.addSound("drowned man click sound 2");
			_drownedMan.start();
		}

		if (quest == kRescuePhilQuest) {
			_alchemist = StyxShade("alchemist", 750, 5000, 10000, "alchemist");
                        if (!persistent->_styxAlchemistSaidIntro)
				_alchemist.addSound("alchemist intro");
			if (persistent->_hintsAreEnabled) {
				if ((persistent->isInInventory(kCoin) || persistent->_styxCharonUsedCoin)
				    && (persistent->isInInventory(kPotion) || persistent->_styxCharonUsedPotion)) {
					_alchemist.addSound("alchemist hint 2");
					_alchemist.addSound("alchemist hint 3");
				} else if (persistent->_creteVisitedAfterAlchemistIntro) {
					_alchemist.addSound("alchemist hint 1");
				}
			}
                        if (persistent->_styxAlchemistSaidIntro)
				_alchemist.addSound("alchemist intro");
			_alchemist.addSound("alchemist click");
			_alchemist.start();
			if (!persistent->_styxAlchemistSaidIntro)
				room->disableMouse();
		}

		// TODO: condition it on not restoring from save
		room->setViewportOffset(Common::Point(0, -50));
		_cameraMovingDown = true;
		_cameraMovingStart = g_vm->getCurrentTime();
		room->selectFrame("overlay fade from volcano top", 1000, 0, Common::Point(0, -50));

		g_vm->getHeroBelt()->setColour(HeroBelt::kCold);
	}

	void frameCallback() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		if (_cameraMovingDown) {
			int pos = -50 + 50 * (g_vm->getCurrentTime() - _cameraMovingStart) / 4000;
			if (pos >= 0) {
				handleEvent(28017);
				pos = 0;
			}
			room->setViewportOffset(Common::Point(0, pos));
		}

		if (_cameraMovingUp) {
			int pos = -50 * (g_vm->getCurrentTime() - _cameraMovingStart) / 4000;
			if (pos < -50) {
				handleEvent(28019);
				pos = -50;
			}
			room->setViewportOffset(Common::Point(0, pos));
		}
	}

private:
	void playCharonTalk(const Common::String &name, int event) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playVideo(name, 0, event);
		_charon.hide();
		room->playAnimLoop("charon talks", 550);
	}

	void stopCharonTalk() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_charon.unpauseAndFirstFrame();
		room->stopAnim("charon talks");
	}
	AmbientAnim _charon;
	bool _charonSound;
	bool _cameraMovingDown;
	bool _cameraMovingUp;
	int _cameraMovingStart;

	StyxShade _axHead;
	StyxShade _pillar;
	StyxShade _dog;
	StyxShade _drownedMan;
	StyxShade _statue;
	StyxShade _greekSoldier;
	StyxShade _trojanSoldier;
	StyxShade _alchemist;
};

Common::SharedPtr<Hadesch::Handler> makeRiverStyxHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new RiverStyxHandler());
}

}
