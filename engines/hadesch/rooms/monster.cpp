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
#include "hadesch/rooms/monster.h"

namespace Hadesch {
enum {
	kBackgroundZ = 10000,
	kCyclopsZ = 500,
	kZeusLightZ = 500,
	kTyphoonZ = 500
};

static const char *kZeusLight = "V7100BJ0";
static const int kLightningCutoff = kVideoWidth / 2;

class MonsterHandler : public Handler {
public:
	MonsterHandler() {
		_playingShootingSound = false;
		_countOfIntroLightning = 0;
		_battleground = Common::SharedPtr<Battleground>(new Battleground());
		_typhoon = Common::SharedPtr<Typhoon>(new Typhoon(_battleground));
		_cyclops = Common::SharedPtr<Cyclops>(new Cyclops(_battleground));
		_illusion = Common::SharedPtr<Illusion>(new Illusion(_battleground));
	}

	void handleClick(const Common::String &name) override {
		if (_battleground->_isInFight && _battleground->_monsterNum == kTyphoon) {
			_typhoon->handleClick(_typhoon, name);
			return;
		}

		if (_battleground->_isInFight && _battleground->_monsterNum == kIllusion) {
			_illusion->handleClick(name);
			return;
		}
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		_typhoon->handleEvent(eventId);
		_cyclops->handleEvent(eventId);
		_illusion->handleEvent(eventId);

		switch (eventId) {
		case 524:
			g_vm->getHeroBelt()->setBranchOfLifeFrame(0);
			break;
		case 526:
			g_vm->getHeroBelt()->setThunderboltFrame(kLightning3);
			break;
			
		case 15351:
			room->playAnimWithSound(kZeusLight, "G0260MA0", kZeusLightZ,
						PlayAnimParams::keepLastFrame().partial(0, 4),
						15379); // 15379(anim), 15381(-1), 15359(sound)
			break;
		case 15352:
			room->playVideo("V7190BA0", 0, 15386, Common::Point(0, 216));
			break;
		case 15353:
			room->playAnim(kZeusLight, kZeusLightZ,
				       PlayAnimParams::disappear().partial(4, -1),
				       15358);
			break;
		case 15355:
			room->playSkippableSound("V7100WC0", 15364);
			g_vm->getHeroBelt()->setThunderboltFrame(kLightning2);
			g_vm->addTimer(526, 5000);
			break;
		case 15356:
			room->playVideo("V7180BB0", 0, 15361, Common::Point(0, 216));
			break;
		case 15357:
			room->playSkippableSound(g_vm->getRnd().getRandomBit()
						 ? "V7150WC0" : "V7150WB0",
						 15353);
			break;
		case 15358:
			switch (_battleground->_monsterNum) {
			case kCyclops:
				room->playVideo("V7180BB0", 0, 15389, Common::Point(0, 216));
				break;
			case kTyphoon:
				room->playVideo("V7210BB0", 0, 15387, Common::Point(0, 216));
				break;
			case kIllusion:
				room->playVideo("V7220BR0", 0, 15382, Common::Point(0, 216));
				break;
			}
			break;
		case 15361:
			g_vm->addTimer(15391, 100);
			break;
		case 15364:
			room->playSkippableSound("V7100WD0", 15365);
			_battleground->_leavesRemaining = 9;
			g_vm->getHeroBelt()->setBranchOfLifeFrame(1);
			g_vm->addTimer(524, 5000);
			break;
		case 15365:
			room->playSkippableSound("V7100WE0", 15366);
			_countOfIntroLightning = 0;
			introLightning();
			break;
		case 15366:
			room->playSkippableSound("V7100WF0", 15367);
			break;
		case 15367:
			room->playSkippableSound("V7100WH0", 15368);
			break;
		case 15368:
			room->playSkippableSound("V7100WI0", 15369);
			break;
		case 15369:
			room->playAnim(kZeusLight, kZeusLightZ,
				       PlayAnimParams::disappear().partial(4, -1),
				       15356);
			break;
		case 15370:
			if (++_countOfIntroLightning < 4)
				introLightning();
			break;
		case 15374:
			room->playAnimWithSound(kZeusLight, "G0260MA0", kZeusLightZ,
						PlayAnimParams::keepLastFrame().partial(0, 4),
						15375);
			break;
		case 15375:
			room->playSound(g_vm->getRnd().getRandomBit() ? "v7150wd0" : "v7150we0",
				15376);
			replenishPowers();
			break;
		case 15376:
			room->playAnim(kZeusLight, kZeusLightZ,
				       PlayAnimParams::disappear().partial(4, -1),
				       15377);
			break;
		case 15377:
			room->playVideo("V7210BB0", 0, 15387, Common::Point(0, 216));
			break;
		case 15378:
			handleEvent(15390);
			break;
		case 15379:
			room->playSound(g_vm->getRnd().getRandomBit() ? "v7150wd0" : "v7150we0",
				15380);
			replenishPowers();
			break;
		case 15380:
			room->playAnim(kZeusLight, kZeusLightZ,
				       PlayAnimParams::disappear().partial(4, -1),
				       15381);
			break;
		case 15381:
			room->playVideo("V7220BR0", 0, 15388, Common::Point(0, 216));
			break;
		case 15382:
			g_vm->addTimer(15392, 100);
			break;
		case 15383:
			if (persistent->_quest == kRescuePhilQuest)
				g_vm->moveToRoom(kHadesThroneRoom);
			else {
				_battleground->_level++;
				_cyclops->enterCyclops(_battleground->_level);
			}
			break;
		case 15386:
			room->playSound(persistent->_gender == kMale ? "V7190WB0" : "V7190WC0", 15374);
			break;
		case 15387:
			room->playSound(persistent->_gender == kMale ? "V7210WB0" : "V7210WC0", 15378);
			break;
		case 15388:
			room->playSound("V7220WB1", 15382);
			break;
		case 15389:
			room->playSound("V7180WB0", 15361);
			break;
		case 15390:
			_typhoon->enterTyphoon(1);
			break;
		case 15391:
			_cyclops->enterCyclops(1);
			break;
		case 15392:
			_illusion->enterIllusion(1);
			break;
		case 15465:
			_playingShootingSound = false;
			break;
		case kHitReceived:
			_battleground->_leavesRemaining--;
			if (_battleground->_leavesRemaining >= 0)
				g_vm->getHeroBelt()->setBranchOfLifeFrame(10 - _battleground->_leavesRemaining);
			if (_battleground->_leavesRemaining <= 0) {
				_battleground->stopFight();
				room->disableMouse();
				room->playAnimWithSound(kZeusLight, "G0260MA0", kZeusLightZ,
							PlayAnimParams::keepLastFrame().partial(0, 4),
							15357);
			}
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		g_vm->getHeroBelt()->setColour(HeroBelt::kCold);
		room->loadHotZones("Monster.HOT", false);
		room->addStaticLayer("v7010pa0", kBackgroundZ, Common::Point(-10, -10)); // background
		// event 15362+15363

		room->disableMouse();
		_battleground->_monsterNum = kCyclops;
		room->playAnimWithSound(kZeusLight, "G0260MA0", kZeusLightZ,
					PlayAnimParams::keepLastFrame().partial(0, 4),
					15355);
	}

	void frameCallback() override {
		_battleground->tick();
		_illusion->tick();
	}

	void handleAbsoluteClick(Common::Point p) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (!_battleground->_isInFight)
			return;
		_battleground->handleAbsoluteClick(p);

		HeroPower hp = g_vm->getHeroBelt()->getSelectedStrength();
		bool isStrong = hp == kPowerStrength;

		if (p.x < kLightningCutoff) {
			room->playAnim(isStrong ? "v7130ba2" : "v7130ba0", 300, PlayAnimParams::disappear(), EventHandlerWrapper(), p);
		} else {
			room->playAnim(isStrong ? "v7130ba3" : "v7130ba1", 300, PlayAnimParams::disappear(),
				       EventHandlerWrapper(), p - Common::Point(kLightningCutoff, 0));
		}

		if (!_playingShootingSound) {
			switch (hp) {
			case kPowerStealth:
				room->playSound("v7130ea0");
				break;
			case kPowerStrength:
				room->playSound("v7130eb0");
				break;
			case kPowerWisdom:
				room->playSound("v7130ec0");
				break;
			case kPowerNone:
				room->playSound("v7130ee0");
				break;
			}
			_playingShootingSound = true;
			g_vm->addTimer(15465, g_vm->getRnd().getRandomNumberRng(300, 600));
		}

		switch (_battleground->_monsterNum) {
		case kCyclops:
			_cyclops->handleClick(p);
			break;
		case kTyphoon:
			break;
		case kIllusion:
			_illusion->handleAbsoluteClick(p);
			break;
		}
	}

private:
	void replenishPowers() {
		g_vm->getHeroBelt()->setThunderboltFrame(kLightning2);
		g_vm->addTimer(526, 5000);
		_battleground->_leavesRemaining = 9;
		g_vm->getHeroBelt()->setBranchOfLifeFrame(1);
		g_vm->addTimer(524, 5000);
	}

	void introLightning() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Common::Point target = Common::Point(
			g_vm->getRnd().getRandomNumberRng(150, 450),
			g_vm->getRnd().getRandomNumberRng(50, 350));
		if (target.x < kLightningCutoff) {
			room->playAnim("v7130ba0", 300, PlayAnimParams::disappear(), 15370, target);
		} else {
			room->playAnim("v7130ba1", 300, PlayAnimParams::disappear(), 15370,
				       target - Common::Point(kLightningCutoff, 0));
		}
		room->playSound("v7130eb0");
	}

	bool _playingShootingSound;
	int _countOfIntroLightning;
	Common::SharedPtr<Battleground> _battleground;
	Common::SharedPtr<Typhoon> _typhoon;
	Common::SharedPtr<Cyclops> _cyclops;
	Common::SharedPtr<Illusion> _illusion;
};

Common::SharedPtr<Hadesch::Handler> makeMonsterHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new MonsterHandler());
}

}
