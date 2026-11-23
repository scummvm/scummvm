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

namespace Hollywood {

void RunawayRoomScene::startMuseum() {
	_inventory.startChapter(2);
	_entering = true;
	resumeMuseum();
}

void RunawayRoomScene::resumeMuseum() {
	if (_sequence.active()) {
		return;
	}
	resumeAmbientAudio(_entering);
	prepareCustomGameplayLoop();
	if (!_entering) {
		return;
	}
	_entering = false;
	runCustomEntrySequence();
}

void RunawayRoomScene::deferMuseumAction(uint16 action) {
	_roomAction = action;
	_actionFrames.clear();
	_actionStage = 0;
}

void RunawayRoomScene::refreshMuseum() {
	if (!rebuildBackground()) {
		error("Cannot update Runaway museum room %u", _room);
	}
	updateMaps();
}

void RunawayRoomScene::leaveMuseumRoom(uint16 scene, Common::Point position, byte facing, bool door) {
	runExitSideEffectsAfterLoop();
	_transition.scene = scene;
	_transition.position = position;
	_transition.facing = facing;
	_transition.door = door;
}

void RunawayRoomScene::updateMuseum(uint elapsed) {
	advanceCustomGameplayLoop(elapsed);
	updateRoomAnimation();
	if (_sequence.active() || _transition.scene) {
		return;
	}
	updateConversation();
	if (_roomAction && _actionFrames.empty() && !speaking() && !_conversation && !_walkPath.active()) {
		if (updateMuseumInventory()) {
			return;
		}
		const uint16 action = _roomAction;
		_roomAction = 0;
		finishCustomAnimation(action);
	}
}

void RunawayRoomScene::setMuseumInventorySpeechPoses(uint16 cue, byte variation, uint firstLine) {
	byte pose = 0;
	switch (cue) {
	case 2:
		pose = 1;
		break;
	case 4:
	case 5:
	case 11:
	case 21:
		pose = 2 * _random.getRandomNumber(1);
		break;
	case 6:
		pose = _random.getRandomNumber(1);
		break;
	case 17:
		pose = 2;
		break;
	case 20:
		pose = variation == 0 ? 1 : 0;
		break;
	case 52:
		pose = variation == 2 ? 1 : 0;
		break;
	case 115:
	case 116:
	case 117:
		// The demo has an unused smiling speech pose at frames 18-22.
		pose = _restoredContent && variation == 1 ? 3 : 0;
		break;
	case 121:
		pose = _restoredContent && variation == 2 ? 3 : 0;
		break;
	default:
		break;
	}
	for (uint i = firstLine; i < _lines.size(); ++i) {
		_lines[i].inventoryPose = pose;
		// Native 0x428170 indexes poses by the remaining continuation count.
		if (_lines.size() - i == 2) {
			if (cue == 50)
				_lines[i].inventoryPose = 2;
			else if (cue == 52 && variation == 0)
				_lines[i].inventoryPose = 1;
		}
	}
}

bool RunawayRoomScene::sayMuseumInventory(const RunawayInventory::Reply &reply) {
	if (!reply.cue)
		return true;
	if (reply.cue == 102) { // Inspect the crucifix before or after restoration.
		typedef RunawaySequence S;
		const byte restored = _inventory.museum().restoredCrucifix != 0;
		_filmCommands = {{S::kImage, (uint16)(143 + restored)}, {S::kSpeech, 102, restored, restored, 1},
			{S::kWaitInput}, {S::kEnd}};
		if (_game == kRunawayMuseumDemo)
			_filmCommands[0].file = restored ? "am_crlim.obt" : "am_crsuc.obt";
		_inventorySequence = true;
		_sequence.start(_filmCommands.data(), _text, _now, 'D');
		return true;
	}
	if (reply.cue == 108) {
		_inventory.museum().businessCardExamined = 1;
		say(108, _museum.phoneUsed != 0, true);
		return true;
	}
	if (_game == kRunawayMuseumDemo && reply.cue == 52) {
		say(52, _random.getRandomNumber(2), true);
		return true;
	}
	if (reply.cue == 115 && reply.variation == 1) {
		// Inserting the battery precedes Brian's response.
		deferMuseumAction(reply.cue);
		return true;
	}
	say(reply.cue, reply.cue == 112 ? _museum.safeUsed : reply.variation, true);
	if (((reply.cue == 116 || reply.cue == 117) && !reply.variation) ||
			(reply.cue == 121 && reply.variation == 1)) {
		deferMuseumAction(reply.cue);
	}
	return true;
}

bool RunawayRoomScene::updateMuseumInventory() {
	uint16 effect;
	byte volume;
	switch (_roomAction) {
	case 115: // Insert the frozen battery into the recorder.
		effect = 60;
		volume = 230;
		break;
	case 116: // Put the battery in the ladle.
		effect = 40;
		volume = 179;
		break;
	case 117: // Pry the ruby out of the mask.
		effect = 56;
		volume = 230;
		break;
	case 121: // Fill the packet with ground coffee.
		effect = 45;
		volume = 255;
		break;
	default:
		return false;
	}
	if (!_actionStage) {
		_audio.playEffect(effect, volume);
		_actionStarted = _now;
		_actionStage = 1;
		return true;
	}
	// Native inventory actions wait for the effect, up to six quarter-second ticks.
	if (_audio.effectPlaying() && _now - _actionStarted < 1500) {
		return true;
	}
	const uint16 cue = _roomAction;
	_roomAction = 0;
	if (cue == 117) {
		_inventory.add(12);
	}
	say(cue, cue == 121 ? 2 : 1, true);
	return true;
}

void RunawayRoomScene::startMuseumFilm(MuseumFilm film) {
	typedef RunawaySequence S;
	const bool demo = _game == kRunawayMuseumDemo;
	const bool gog = !demo && Common::File::exists("DATAVC00.000");
	_museumFilm = film;
	_filmCommands.clear();
	const uint stage = film == kArrivalFilm || film == kCliveFilm ? 401 : sceneTextNumber();
	if (!_filmText.load("RESOURCE.003", "Runaway museum film", stage, 0, 0x1135)) {
		error("Cannot load Runaway museum film text");
	}
	switch (film) {
	case kArrivalFilm:
		// The original blanks the screen during dialing and waits for the
		// effect, with a limit of 28 quarter-second ticks.
		_filmCommands = {{S::kMusic, 17, 255}, {S::kOpen, 5}, {S::kEffect, 52, 255},
			{S::kPlay, 0, 62}, {S::kSpeech, 40, 0}, {S::kPlay, 63, 889},
			{S::kStill, (uint16)(gog ? 900 : 890)}, {S::kWaitEffect, 7000, 1}, {S::kTalk, 30, 0, 1, 0, 21}, {S::kDelay, 1000},
			{S::kTalk, 30, 1, 1, 0, 21}, {S::kPlay, (uint16)(gog ? 912 : 919), (uint16)(gog ? 919 : 930)},
			{S::kTalk, 30, 2, 3, 0, 21}, {S::kDelay, 2000}, {S::kTalk, 30, 3, 3, 0, 21}, {S::kDelay, 3000}, {S::kTalk, 30, 4, 3, 0, 21}};
		if (demo) {
			_filmCommands = {{S::kOpen, 5}, {S::kPlay, 0, 68}, {S::kSpeech, 40, 0}, {S::kPlay, 69, 912},
				{S::kTalk, 30, 0, 1, 0, 21}, {S::kDelay, 1000},
				{S::kTalk, 30, 1, 1, 0, 21}, {S::kDelay, 3000}, {S::kPlay, 923, 931},
				{S::kTalk, 30, 2, 3, 0, 21}, {S::kDelay, 2000}, {S::kTalk, 30, 3, 3, 0, 21},
				{S::kDelay, 3000}, {S::kPlay, 943, 988}, {S::kTalk, 30, 4, 3, 0, 21}};
		}
		break;
	case kCliveFilm:
		_filmCommands = {{S::kOpen, 1}, {S::kPlay, 57, 107}, {S::kTalk, 1, 0, (byte)(gog ? 5 : 1), 0, 20},
			{S::kPlay, 0, 25}, {S::kTalk, 3, 0, (byte)(gog ? 1 : 5)}, {S::kPlay, 37, 44}, {S::kTalk, 2, 0, (byte)(gog ? 3 : 9), 0, 14}};
		{
			static const byte cues[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 17};
			static const byte colors[] = {20, 11, 20, 14, 20, 11, 20, 11, 14, 11, 20, 11, 20, 14, 11, 14};
			static const byte cdPortraits[] = {3, 7, 1, 9, 3, 5, 3, 5, 9, 7, 1, 5, 1, 11, 5, 9};
			static const byte gogPortraits[] = {9, 7, 5, 3, 9, 1, 9, 1, 3, 7, 5, 1, 5, 11, 1, 3};
			static const byte stills[] = {5, 3, 159, 1, 5, 0, 5, 0, 44, 3, 2, 0, 2, 4, 0, 44};
			for (uint i = 0; i < ARRAYSIZE(cues); ++i) {
				if (i == 2) {
					_filmCommands.push_back(S::Command(S::kPlay, 119, 159));
				} else if (i == 8 || i == 15) {
					_filmCommands.push_back(S::Command(S::kPlay, 37, 44));
				} else if (gog) {
					_filmCommands.push_back(S::Command(S::kStill, stills[i], 1));
				}
				_filmCommands.push_back(S::Command(S::kTalk, cues[i], i == 15 ? 1 : 0, gog ? gogPortraits[i] : cdPortraits[i], 0, colors[i]));
			}
		}
		break;
	case kDrillingFilm:
		_filmCommands = {{S::kOpen, 2}, {S::kEffect, 49, 204}, {S::kPlay, 0, 135}};
		break;
	case kLaserFilm:
		_filmCommands = {{S::kOpen, 3}, {S::kMusic, 22, 255}, {S::kEffect, 50, 204}, {S::kPlay, 0, 127},
			{S::kSpeech, 70, 0, 0, 0, 17}, {S::kPlay, 128, 175}, {S::kMusic, 13, 255}};
		if (demo)
			_filmCommands = {{S::kOpen, 3}, {S::kPlay, 0, 175}};
		break;
	case kRubyFilm:
		_filmCommands = {{S::kOpen, 4}, {S::kEffect, 51, 230}, {S::kPlay, 0, 75}};
		break;
	case kCrucifixFilm:
		_filmCommands = {{S::kImage, 144}, {S::kSpeech, 32, 2, 2}, {S::kWaitSpeech}};
		if (demo)
			_filmCommands[0].file = "am_crlim.obt";
		break;
	case kMobstersFilm:
		_filmCommands = {{S::kOpen, 7}, {S::kMusic, 21, 255, 1}, {S::kEffect, 13, 255}, {S::kPlay, 0, (uint16)(demo ? 211 : 213)},
			{S::kTalk, 93, 0, 1, 0, 22}, {S::kTalk, 94, 0, 1, 1, 21}, {S::kTalk, 93, 1, 1, 0, 22},
			{S::kPlay, (uint16)(demo ? 223 : gog ? 214 : 225), (uint16)(demo ? 236 : gog ? 226 : 237)}, {S::kTalk, 94, 1, 3, 0, 21}};
		if (gog) {
			_filmCommands.push_back(S::Command(S::kStill, 227));
		}
		_filmCommands.push_back(S::Command(S::kTalk, 93, 2, (uint16)(gog ? 1 : 5), 0, 22));
		if (gog) {
			_filmCommands.push_back(S::Command(S::kStill, 228));
		}
		_filmCommands.push_back(S::Command(S::kTalk, 94, 2, 3, 0, 21));
		_filmCommands.push_back(S::Command(S::kMusic, 13, 255));
		break;
	case kPhoneFilm:
		_filmCommands = {{S::kOpen, 8}, {S::kEffect, 57, 153, 1, 1}, {S::kPlay, 0, 31}, {S::kTalk, 91, 0, 1, 0, 19}};
		for (uint i = _museum.phoneUsed ? 7 : 0; i < (_museum.phoneUsed ? 9U : 6U); ++i) {
			_filmCommands.push_back(S::Command(S::kTalk, 82, i, 1, 1));
			_filmCommands.push_back(S::Command(S::kTalk, 92, i, 1, 0, 19));
		}
		_filmCommands.push_back(S::Command(S::kTalk, 82, 6, 1, 1));
		_filmCommands.push_back(S::Command(S::kTalk, 92, 6, 1, 0, 19));
		_filmCommands.push_back(S::Command(S::kPlay, (uint16)(demo ? 54 : gog ? 32 : 43), (uint16)(demo ? 87 : gog ? 62 : 72)));
		break;
	case kScannerFilm:
		_filmCommands = {{S::kOpen, 6}, {S::kMusic, 19, 255}, {S::kEffect, 53, 255}, {S::kPlay, 0, (uint16)(demo ? 694 : 386)}, {S::kMusic, 11, 255}};
		break;
	case kBetrayalFilm:
		_filmCommands = {{S::kOpen, 9}, {S::kMusic, 20, 255}, {S::kPlay, 0, 9}, {S::kEffect, 11, 255},
			{S::kSpeech, 92, 4, 4, 0, 20}, {S::kPlay, 10, 78}, {S::kWaitSpeech}, {S::kTalk, 92, 2, 1, 0, 20},
			{S::kPlay, (uint16)(gog ? 79 : 90), (uint16)(gog ? 86 : 90)}, {S::kSpeech, 92, 3, 3, 0, 20},
			{S::kPlay, (uint16)(gog ? 87 : 91), (uint16)(gog ? 121 : 134)}, {S::kEffect, 12, 255},
			{S::kPlay, (uint16)(gog ? 122 : 135), (uint16)(gog ? 124 : 135)}, {S::kWaitSpeech}, {S::kWaitEffect, 5000}};
		if (demo) {
			_filmCommands = {{S::kOpen, 9}, {S::kPlay, 0, 79}, {S::kTalk, 92, 2, 1, 0, 20},
				{S::kSpeech, 92, 3, 3, 0, 20}, {S::kPlay, 91, 135}, {S::kWaitSpeech}};
		}
		break;
	case kMuseumExitFilm:
		_filmCommands = {{S::kOpen, 10}, {S::kEffect, 57, 153, 1, 1}, {S::kPlay, 0, 12},
			{S::kTalk, 60, 0, 1, 0, 19}, {S::kMusic, 18, 255}, {S::kEffect, 54, 255},
			{S::kSpeech, 60, 1, 1, 0, 19}, {S::kPlay, (uint16)(gog ? 13 : 24), (uint16)(gog ? 130 : 141)}, {S::kWaitSpeech}};
		break;
	}
	if (demo) {
		static const char *const movies[] = {
			nullptr, "Interior museo 1.avi", "susan trabajando.avi", "susan le revienta.avi", "pone rubi.avi",
			"Exterior museo 1.avi", "Escaner-crucifijo.avi", "Exterior museo 2.avi", "Willy telefono.avi",
			"final del museo.avi", "muerte de willy.avi"
		};
		// The demo keeps each twelve-frame talking portrait in the movie itself.
		static const uint16 portraits[][6] = {
			{}, {107, 172, 25, 160, 44, 184}, {}, {}, {}, {912, 931}, {},
			{211, 236, 248}, {31}, {79}, {12}
		};
		Common::Array<S::Command> commands;
		uint movie = 0;
		for (S::Command command : _filmCommands) {
			if (command.operation == S::kOpen) {
				movie = command.a;
				command.file = movies[movie];
			} else if (command.operation == S::kTalk && command.c) {
				const uint16 frame = portraits[movie][(command.c - 1) / 2];
				commands.push_back(S::Command(S::kStill, frame));
				if (!command.d) {
					commands.push_back(S::Command(S::kSpeech, command.a, command.b, command.b, 0, command.speechColor));
					commands.push_back(S::Command(S::kPlay, frame, frame + 11, S::kPortraitDuringSpeech));
					continue;
				}
				command.c = 0;
			}
			commands.push_back(command);
		}
		_filmCommands = commands;
	}
	_filmCommands.push_back(S::Command(S::kEnd));
	if (film == kPhoneFilm) {
		draw();
	}
	_sequence.start(_filmCommands.data(), _filmText, _now, 'D');
	if (film == kPhoneFilm) {
		_sequence.setBackground(_screen, demo ? Common::Rect(106, 316, 394, 604) :
			Common::Rect(116, kRunawayViewportTop + 320, 396, kRunawayViewportTop + 600));
	}
}

} // End of namespace Hollywood
