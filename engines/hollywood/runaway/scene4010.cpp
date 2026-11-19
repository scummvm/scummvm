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
 */

#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {

void RunawayRoomScene::startMuseum() {
	_inventory.startChapter(2);
	_entering = true;
	resumeMuseum();
}

void RunawayRoomScene::resumeMuseum() {
	if (_sequence.active())
		return;
	if (_entering || !_audio.musicPlaying())
		_audio.playMusic(_room == kMuseumSecurity ? 11 : _room == kMuseumLaboratory ? 13 : 12, false);
	_nextLaser = _now + 25000 + _random.getRandomNumber(5000);
	if (!_entering)
		return;
	_entering = false;
	if (_entryScene == 4010) {
		startMuseumFilm(kArrivalFilm);
	} else if (_room == kMuseumWilly) {
		start4070();
	} else if (_room == kMuseumKeypad) {
		start4140();
	} else if (_room == kMuseumLaboratory) {
		if (_museum.susan == 2) {
			_museum.susan = 3;
			_inventory.museum().restoredCrucifix = 1;
			say(32);
			say(32, 1);
			deferMuseumAction(405);
			refreshMuseum();
		} else if (!_museum.susanMet) {
			_museum.susanMet = 1;
			for (uint i = 0; i < 3; ++i) {
				say(80, i);
				say(90, i, false, 1);
			}
		}
	} else if (_room == kMuseumSecurity) {
		if (_inventory.museum().restoredCrucifix) {
			walkTo(Common::Point(906, 470), 0, true);
			deferMuseumAction(409);
		} else {
			beginRoomAnimationRange(412, 21, 13, 12, 0, false);
			_audio.playEffect(32);
		}
	} else if (_room == kMuseumHall) {
		if (_entryScene != 4063 && _entryScene != 4064) {
			_museum.willyInLab = 0;
			updateMaps();
		} else if (_entryScene == 4064 && _museum.willyInLab) {
			changeFloor(false, Common::Point(921, 813));
			if (_museum.willyInLab == 1) {
				walkTo(Common::Point(1029, 842), 7, true);
				say(38, 1);
				deferMuseumAction(460);
			} else {
				say(36, 3);
				deferMuseumAction(462);
			}
		}
		if (_entryScene == 4063) {
			if (_museum.keypad == 1)
				say(26, 2);
			else if (_museum.keypad == 4)
				leaveMuseumRoom(4100, Common::Point(141, 468), 3);
		}
	}
}

void RunawayRoomScene::deferMuseumAction(uint16 action) {
	_roomAction = action;
	_actionFrames.clear();
	_actionStage = 0;
}

void RunawayRoomScene::refreshMuseum() {
	if (!rebuildBackground())
		error("Cannot update Runaway museum room %u", _room);
	updateMaps();
}

void RunawayRoomScene::leaveMuseumRoom(uint16 scene, Common::Point position, byte facing, bool door) {
	if (_room == kMuseumSecurity) {
		_museum.chamberOpen = 0;
		_museum.chamberTime = 0;
	}
	_transition.scene = scene;
	_transition.position = position;
	_transition.facing = facing;
	_transition.door = door;
}

void RunawayRoomScene::updateMuseum(uint elapsed, bool inventoryOpen) {
	if (_room == kMuseumSecurity && _now - _ambientTick >= 80) {
		_ambientTick = _now;
		_computerFrame = speaker() && !_cliveVisible ? _random.getRandomNumber(48) : 48;
		for (uint i = 0; i < 2; ++i) {
			if (_museum.chamberOpen)
				_machinePosition[i] = MIN<uint>(i ? 405 : 512, _machinePosition[i] + 1);
			else if (_machinePosition[i] > 5)
				--_machinePosition[i];
			else if (!_random.getRandomNumber(_machinePosition[i] ? 9 : 49))
				_machinePosition[i] = _machinePosition[i] ? 0 : i ? 5 : 4;
		}
	}
	if (_room == kMuseumLaboratory) {
		if (speaker())
			_ambientFrame = (_museum.susan == 1 ? 52 : 45) + speechFrame();
		else if (_museum.susan == 3)
			_ambientFrame = 1;
		else if (_museum.susan == 1)
			_ambientFrame = 79;
		else
			_ambientFrame = (_now / 80) % 26;
		if (!_museum.susan && !busy() && !inventoryOpen && _position.x < 530 && (int32)(_now - _nextLaser) >= 0) {
			beginRoomAnimation(450, 23, 95, nullptr, 0, false);
			_nextLaser = _now + 25000 + _random.getRandomNumber(5000);
		}
	} else if (_room == kMuseumHall) {
		_ambientFrame = (_now / 80) % 22;
	} else if (_room == kMuseumSecurity && _museum.chamberTime) {
		_museum.chamberTime -= MIN<uint>(elapsed, _museum.chamberTime);
		if (!_museum.chamberTime && !_roomAction)
			closeMuseumChamber();
	}
	if (_room == kMuseumSecurity && _museum.chamberOpen && !_museum.chamberTime && !_roomAction)
		closeMuseumChamber();
	updateRoomAnimation();
	if (_sequence.active() || _transition.scene)
		return;
	updateConversation();
	if (_roomAction && _actionFrames.empty() && !speaking() && !_conversation && _pathIndex == _path.size()) {
		const uint16 action = _roomAction;
		_roomAction = 0;
		finishMuseumAnimation(action);
	}
}

void RunawayRoomScene::museumAnimationFrame() {
	if (_room == kMuseumOffice && _roomAction == 321) {
		if (_actionStep == 21)
			_audio.playEffect(36);
		if (_actionStep == 27)
			_audio.playEffect(_safeOpen ? 39 : 38, 77);
	} else if (_room == kMuseumLaboratory) {
		if ((_roomAction == 328 || _roomAction == 428) && (_actionStep == 4 || _actionStep == 2))
			_audio.playEffect(37, 204);
	} else if (_room == kMuseumHall && _roomAction == 461) {
		if (_actionStep == 156)
			_audio.playEffect(29);
	} else if (_room == kMuseumSecurity) {
		if ((_roomAction == 308 || _roomAction == 340) && _actionStep == 12)
			_audio.playEffect(34, 230);
		if ((_roomAction == 342 && _actionStep == 15) || (_roomAction == 344 && _actionStep == 4))
			_audio.playEffect(37, 204);
		if (_roomAction == 325) {
			if (_actionStep == 12)
				_audio.playEffect(42);
			else if (_actionStep == 20)
				_audio.playEffect(41, 255, 1);
			else if (_actionStep == 105)
				_audio.playEffect(43);
		}
		if (_roomAction == 410 && _actionStep == 35)
			_audio.playEffect(55);
	}
}

bool RunawayRoomScene::sayMuseumInventory(const RunawayInventory::Reply &reply) {
	if (reply.cue == 108)
		_inventory.museum().businessCardExamined = 1;
	say(reply.cue, reply.cue == 112 ? _museum.safeUsed : reply.variation, true);
	if ((reply.cue == 116 && !reply.variation) || (reply.cue == 117 && !reply.variation))
		say(reply.cue, 1, true);
	if (reply.cue == 121 && reply.variation == 1)
		say(121, 2, true);
	return true;
}

void RunawayRoomScene::finishMuseumAnimation(uint16 action) {
	switch (_room) {
	case kMuseumOffice:
		if (action == 321) {
			_safeOpen = !_safeOpen;
			if (_safeOpen && !_museum.safeUsed) {
				_museum.safeUsed = 1;
				say(20, 1);
			}
			refreshMuseum();
		}
		break;
	case kMuseumLaboratory: finish4050(action); break;
	case kMuseumHall: finish4060(action); break;
	case kMuseumWilly: finish4070(action); break;
	case kMuseumSecurity: finish4100(action); break;
	case kMuseumKeypad:
		if (action == 414)
			leaveMuseumRoom(4063, Common::Point(1153, 297), 7);
		break;
	default: break;
	}
}

void RunawayRoomScene::startMuseumFilm(MuseumFilm film) {
	typedef RunawaySequence S;
	const bool gog = Common::File::exists("DATAVC00.000");
	_museumFilm = film;
	_filmCommands.clear();
	const uint stage = film == kArrivalFilm || film == kCliveFilm ? 401 : _room == kMuseumWilly ? 406 : _room / 10;
	if (!_filmText.load("RESOURCE.003", "Runaway museum film", stage, 0, 0x1135))
		error("Cannot load Runaway museum film text");
	switch (film) {
	case kArrivalFilm:
		_filmCommands = {{S::kMusic, 17, 255}, {S::kOpen, 5}, {S::kEffect, 52, 255},
			{S::kPlay, 0, 889}, {S::kStill, (uint16)(gog ? 900 : 889)}, {S::kTalk, 30, 0, 1}, {S::kDelay, 1000},
			{S::kTalk, 30, 1, 1}, {S::kPlay, (uint16)(gog ? 912 : 919), (uint16)(gog ? 919 : 930)},
			{S::kTalk, 30, 2, 3}, {S::kDelay, 2000}, {S::kTalk, 30, 3, 3}, {S::kDelay, 3000}, {S::kTalk, 30, 4, 3}};
		break;
	case kCliveFilm:
		_filmCommands = {{S::kOpen, 1}, {S::kPlay, 57, 107}, {S::kTalk, 1, 0, (byte)(gog ? 5 : 1)},
			{S::kPlay, 0, 25}, {S::kTalk, 3, 0, (byte)(gog ? 1 : 5)}, {S::kPlay, 37, 44}, {S::kTalk, 2, 0, (byte)(gog ? 3 : 9)}};
		{
			static const byte cues[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 17};
			static const byte cdPortraits[] = {3, 7, 1, 9, 3, 5, 3, 5, 9, 7, 1, 5, 1, 11, 5, 9};
			static const byte gogPortraits[] = {9, 7, 5, 3, 9, 1, 9, 1, 3, 7, 5, 1, 5, 11, 1, 3};
			static const byte stills[] = {5, 3, 159, 1, 5, 0, 5, 0, 44, 3, 2, 0, 2, 4, 0, 44};
			for (uint i = 0; i < ARRAYSIZE(cues); ++i) {
				if (i == 2)
					_filmCommands.push_back(S::Command(S::kPlay, 119, 159));
				else if (i == 8 || i == 15)
					_filmCommands.push_back(S::Command(S::kPlay, 37, 44));
				else if (gog)
					_filmCommands.push_back(S::Command(S::kStill, stills[i], 1));
				_filmCommands.push_back(S::Command(S::kTalk, cues[i], i == 15 ? 1 : 0, gog ? gogPortraits[i] : cdPortraits[i]));
			}
		}
		break;
	case kDrillingFilm:
		_filmCommands = {{S::kOpen, 2}, {S::kEffect, 49, 204}, {S::kPlay, 0, 135}};
		break;
	case kLaserFilm:
		_filmCommands = {{S::kOpen, 3}, {S::kMusic, 22, 255}, {S::kEffect, 50, 204}, {S::kPlay, 0, 175}, {S::kMusic, 13, 255}};
		break;
	case kRubyFilm:
		_filmCommands = {{S::kOpen, 4}, {S::kEffect, 51, 230}, {S::kPlay, 0, 75}};
		break;
	case kMobstersFilm:
		_filmCommands = {{S::kOpen, 7}, {S::kMusic, 21, 255, 1}, {S::kEffect, 13, 255}, {S::kPlay, 0, 213},
			{S::kTalk, 93, 0, 1}, {S::kTalk, 94, 0, 1, 1}, {S::kTalk, 93, 1, 1},
			{S::kPlay, (uint16)(gog ? 214 : 225), (uint16)(gog ? 226 : 237)}, {S::kTalk, 94, 1, 3}};
		if (gog)
			_filmCommands.push_back(S::Command(S::kStill, 227));
		_filmCommands.push_back(S::Command(S::kTalk, 93, 2, (uint16)(gog ? 1 : 5)));
		if (gog)
			_filmCommands.push_back(S::Command(S::kStill, 228));
		_filmCommands.push_back(S::Command(S::kTalk, 94, 2, 3));
		_filmCommands.push_back(S::Command(S::kMusic, 13, 255));
		break;
	case kPhoneFilm:
		_filmCommands = {{S::kOpen, 8}, {S::kEffect, 57, 153, 1, 1}, {S::kPlay, 0, 31}, {S::kTalk, 91, 0, 1}};
		for (uint i = _museum.phoneUsed ? 7 : 0; i < (_museum.phoneUsed ? 9U : 6U); ++i) {
			_filmCommands.push_back(S::Command(S::kTalk, 82, i, 1, 1));
			_filmCommands.push_back(S::Command(S::kTalk, 92, i, 1));
		}
		_filmCommands.push_back(S::Command(S::kTalk, 82, 6, 1, 1));
		_filmCommands.push_back(S::Command(S::kTalk, 92, 6, 1));
		_filmCommands.push_back(S::Command(S::kPlay, (uint16)(gog ? 32 : 43), (uint16)(gog ? 62 : 72)));
		break;
	case kScannerFilm:
		_filmCommands = {{S::kOpen, 6}, {S::kMusic, 19, 255}, {S::kEffect, 53, 255}, {S::kPlay, 0, 386}, {S::kMusic, 11, 255}};
		break;
	case kBetrayalFilm:
		_filmCommands = {{S::kOpen, 9}, {S::kMusic, 20, 255}, {S::kPlay, 0, 9}, {S::kEffect, 11, 255},
			{S::kSpeech, 92, 4, 4}, {S::kPlay, 10, 78}, {S::kWaitSpeech}, {S::kTalk, 92, 2, 1},
			{S::kPlay, (uint16)(gog ? 79 : 90), (uint16)(gog ? 86 : 90)}, {S::kSpeech, 92, 3, 3},
			{S::kPlay, (uint16)(gog ? 87 : 91), (uint16)(gog ? 121 : 134)}, {S::kEffect, 12, 255},
			{S::kPlay, (uint16)(gog ? 122 : 135), (uint16)(gog ? 124 : 135)}, {S::kWaitSpeech}, {S::kWaitEffect, 5000}};
		break;
	case kMuseumExitFilm:
		_filmCommands = {{S::kOpen, 10}, {S::kEffect, 57, 153, 1, 1}, {S::kPlay, 0, 12},
			{S::kTalk, 60, 0, 1}, {S::kMusic, 18, 255}, {S::kEffect, 54, 255},
			{S::kSpeech, 60, 1, 1}, {S::kPlay, (uint16)(gog ? 13 : 24), (uint16)(gog ? 130 : 141)}, {S::kWaitSpeech}};
		break;
	}
	_filmCommands.push_back(S::Command(S::kEnd));
	if (film == kPhoneFilm)
		draw();
	_sequence.start(_filmCommands.data(), _filmText, _now, 'D');
	if (film == kPhoneFilm)
		_sequence.setBackground(_screen, Common::Rect(116, kRunawayViewportTop + 320, 396, kRunawayViewportTop + 600));
}

void RunawayRoomScene::finishMuseumFilm() {
	switch (_museumFilm) {
	case kArrivalFilm:
		startMuseumFilm(kCliveFilm);
		return;
	case kCliveFilm:
		_entryScene = 4060;
		changeFloor(true, Common::Point(607, 323));
		_audio.playMusic(12, false);
		break;
	case kDrillingFilm:
		_museum.laserSeen = 1;
		beginRoomAnimation(451, 24, 101, nullptr, 0, false);
		break;
	case kRubyFilm:
		beginRoomAnimationRange(456, 19, 13, 4, 0);
		break;
	case kLaserFilm:
		_museum.susan = 1;
		_museum.laserUpgraded = 1;
		say(30, 2);
		say(33, 1, false, 1);
		refreshMuseum();
		break;
	case kMobstersFilm:
		_museum.susan = 2;
		_museum.restoration = 2;
		leaveMuseumRoom(4061, Common::Point(873, 311), 3, true);
		break;
	case kPhoneFilm:
		_museum.phoneUsed = 1;
		_museum.willyInLab = 1;
		_museum.sawCode = 2;
		if (_museum.keypad == 1)
			_museum.keypad = 2;
		beginRoomAnimationRange(460, 13, 27, 26, 0);
		break;
	case kScannerFilm:
		if (!applyPatch(28))
			error("Cannot update Runaway scanner");
		walkTo(Common::Point(915, 547), 4, true);
		deferMuseumAction(413);
		break;
	case kBetrayalFilm:
		_chapterComplete = true;
		break;
	case kMuseumExitFilm:
		leaveMuseumRoom(4100, Common::Point(141, 468));
		break;
	}
}

} // End of namespace Hollywood
