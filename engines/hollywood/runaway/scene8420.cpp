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
#include "common/util.h"
#include "hollywood/runaway/scene8420.h"

namespace Hollywood {

const RunawayRoom kRunawayScene8420 = {
	8420, 1024, 600, nullptr, 0,
	{{100, 100, 100}, {50, 50, 50}, {60, 60, 60}, {80, 80, 80}, {70, 70, 70}}, Common::Point(1023, 450),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8420::Scene8420(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8420::loadCustomResources() {
	if (!_ambient[0].load(_archive, 4, 30) || !_ambient[1].load(_archive, 5, 30) || !_gina.load(_archive, 7, 74)) {
		return false;
	}
	return _entryScene != 8422 || (_dreamGina.load(_archive, 15, 56) && _johnny.load(_archive, 16, 56));
}

void Scene8420::applyCustomHotspotState() {
	// The bedroom is scripted; the original permits Brian to cross every mask zone.
	memset(_maps.data(), 1, 256);
	memset(_maps.data() + 256, 0, 256);
}

void Scene8420::prepareCustomGameplayLoop() {
	_ambientTick = _ginaTick = _now;
	_ambientFrame[1] = _random.getRandomNumber(29);
}

void Scene8420::resumeAmbientAudio(bool entering) {
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(_entryScene == 8422 ? 25 : 16);
		_audio.setMusicVolume(255);
	}
}

void Scene8420::runCustomEntrySequence() {
	_facing = 5;
	if (_entryScene == 8421) {
		typedef RunawaySequence S;
		static const S::Command wakingUp[] = {
			{S::kOpen, 5}, {S::kPlay, 0, 163}, {S::kEnd}
		};
		_showBrian = false;
		_ginaFrame = 61;
		_film = kWakingUp;
		_sequence.start(wakingUp, _text, _now, 'H');
	} else if (_entryScene == 8422) {
		_showBrian = false;
		_dreaming = true;
		startNightmareMovement(0, 37, 420);
	} else {
		walkTo(Common::Point(851, 480), 5, true);
		deferAction(400);
	}
}

void Scene8420::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	RunawayRoomScene::update(now, scrollDirection, inventoryOpen);
	// Gina's cries use a separate voice channel underneath Johnny's nightmare dialogue.
	updateNightmareAudio();
}

void Scene8420::updateNightmareAudio() {
	if (!_screaming || _audio.effectPlaying(1)) {
		return;
	}
	byte variation;
	do {
		variation = 1 + _random.getRandomNumber(2);
	} while (variation == _lastScream);
	_lastScream = variation;
	_audio.playBackgroundSpeech(_text.stageCue(54, variation).voiceSampleId, 1);
}

void Scene8420::advanceCustomGameplayLoop(uint32 delta) {
	while (_now - _ambientTick >= 80) {
		_ambientTick += 80;
		_ambientFrame[0] = (_ambientFrame[0] + 1) % 30;
		const int change = 1 + _random.getRandomNumber(4);
		_ambientFrame[1] = CLIP<int>(_ambientFrame[1] + (_random.getRandomBit() ? change : -change), 0, 29);
	}
	while (_now - _ginaTick >= 80) {
		_ginaTick += 80;
		if (_dreaming) {
			if (_nightmareMoving) {
				_ginaFrame = MIN<byte>(_nightmareLast, _ginaFrame + 1);
				_johnnyFrame = MIN<byte>(_nightmareLast, _johnnyFrame + 1);
				_nightmareMoving = _ginaFrame != _nightmareLast || _johnnyFrame != _nightmareLast;
			}
		} else if (_ginaFrame < 61) {
			++_ginaFrame;
			if (_ginaFrame == 53) {
				_audio.playEffect(56, 102);
			}
			if (_ginaFrame == 61 && !_waitingForGina) {
				_ginaFrame = 0;
			}
		}
	}
}

bool Scene8420::updateCustomSequence() {
	if (_nightmareMoving) {
		return true;
	}
	if (_waitingForGina) {
		if (_ginaFrame < 61) {
			return true;
		}
		_waitingForGina = false;
	}
	return false;
}

void Scene8420::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_dreaming) {
		const uint johnny = speaker() == 2 ? 37 + speechFrame() % 6 : _johnnyFrame;
		_johnny.draw(viewport, johnny, 0, 0);
		_dreamGina.draw(viewport, _ginaFrame, 0, 0);
	} else {
		const uint gina = speaker() == 1 ? 61 + speechFrame() : _ginaFrame;
		_gina.draw(viewport, gina, 0, 0);
	}
	_ambient[1].draw(viewport, _ambientFrame[1], 0, 0);
	_ambient[0].draw(viewport, _ambientFrame[0], 0, 0);
}

float Scene8420::actorHeight() const {
	return _position.x < 690 ? 521.578918457f - _position.x * 0.263157904f : 340;
}

void Scene8420::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

void Scene8420::exchange(uint16 cue, byte first, byte last) {
	for (uint i = first; i <= last; ++i) {
		say(cue, i);
		say(cue + 10, i, false, 1);
	}
}

void Scene8420::startGinaConversation() {
	startConversation(1);
	_destination = _finale.mediumDiscussed ? 8091 : 8101;
	_dialogue[0][0] = {1, 0, 3, 14, 15, 1, 0};
	_dialogue[0][1] = {byte(_finale.mediumDiscussed && _finale.mediumTests < 3), 0, 1, 17, 17, 0, 0};
	_dialogue[0][2] = {byte(!_finale.mediumDiscussed && _finale.mediumTests < 3), 1, 1, 18, 18, 0, 0};
	_dialogue[0][3] = {byte(_finale.mediumTests >= 3), 2, 1, 21, 21, 0, 0};
	_dialogue[10][0] = {1, 0, 0, 19, 19, 2, 255};
	_dialogue[11][0] = {1, 0, 0, 20, 20, 3, 255};
	_dialogue[12][0] = {1, 0, 0, 20, 20, 2, 255};
	buildDialogueChoices();
}

void Scene8420::handleCustomDialogueChoice(byte effect) {
	if (effect == 2 || effect == 3) {
		_destination = effect == 2 ? 8091 : 8101;
	}
}

void Scene8420::finishCustomConversation() {
	deferAction(430);
}

void Scene8420::startNightmareMovement(byte first, byte last, uint16 nextAction) {
	_ginaFrame = first;
	_johnnyFrame = first ? 43 : 0;
	_nightmareLast = last;
	_nightmareMoving = true;
	_ginaTick = _now;
	deferAction(nextAction);
}

RunawayRoomScene::ActionResult Scene8420::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // Greet Gina from the bedroom entrance.
		say(40, _finale.ginaVisited ? 11 : 0);
		deferAction(401);
		break;
	case 401: // Let Gina finish moving her injured leg before answering.
		_waitingForGina = true;
		deferAction(402);
		break;
	case 402: // Gina recognizes Brian.
		say(50, _finale.ginaVisited ? 11 : 0, false, 1);
		deferAction(403);
		break;
	case 403: // Walk over to the bed.
		walkTo(Common::Point(462, 505), 5, true);
		deferAction(404);
		break;
	case 404: // Explain the séance plan, or report progress on finding a medium.
		if (!_finale.ginaVisited) {
			exchange(40, 1, 8);
			say(40, _finale.mediumDiscussed ? 10 : 9);
			say(50, 10, false, 1);
			_finale.ginaVisited = 1;
		} else if (_finale.mediumDiscussed) {
			const byte variation = _finale.ginaMediumDiscussed ? 13 : 12;
			say(50, variation, false, 1);
			say(40, variation);
			_finale.ginaMediumDiscussed = 1;
		}
		deferAction(405);
		break;
	case 405: // Choose whether to ask about Gina's leg or return downstairs.
		startGinaConversation();
		break;
	case 1410: // Brian gets out of bed after recovering from the séance.
		_showBrian = true;
		_position = Common::Point(690, 475);
		_walkPath.reset(_position);
		_facing = 4;
		say(41);
		deferAction(411);
		break;
	case 411: // Gina explains how long Brian has been unconscious.
		_facing = 5;
		exchange(41, 1, 1);
		deferAction(412);
		break;
	case 412: // Approach Gina to ask who she really contacted in the séance.
		walkTo(Common::Point(462, 505), 5, true);
		deferAction(413);
		break;
	case 413: // Demand an explanation for Gina's deception.
		exchange(41, 2, 3);
		deferAction(414);
		break;
	case 414: // Gina confesses what happened at the Pink Iguana.
		startConfession();
		break;
	case 415: // Agree to help and learn about Johnny's trailer.
		exchange(42, 0, 8);
		_destination = 8091;
		deferAction(430);
		break;
	case 420: // Johnny appears in Gina's nightmare and threatens her.
		say(44, 0, false, 2);
		say(54, 0, false, 1);
		say(44, 1, false, 2);
		deferAction(421);
		break;
	case 421: // Johnny approaches the bed as Gina starts screaming.
		_screaming = true;
		startNightmareMovement(37, 55, 422);
		break;
	case 422: { // Brian's calls interrupt the nightmare movie.
		typedef RunawaySequence S;
		static const S::Command nightmare[] = {
			{S::kOpen, 6}, {S::kMusic, 26, 255},
			{S::kSpeech, 44, 2, 6}, {S::kPlay, 0, 163}, {S::kWaitSpeech},
			{S::kMusic, 16, 255, 1}, {S::kEnd}
		};
		_film = kNightmare;
		_sequence.start(nightmare, _text, _now, 'H');
		break;
	}
	case 423: // Reassure Gina, explain the recording, and rehearse her accusation.
		exchange(43, 0, 1);
		say(43, 2);
		say(43, 3);
		say(53, 2, false, 1);
		say(43, 4);
		say(43, 3);
		say(53, 3, false, 1);
		say(43, 5);
		say(53, 4, false, 1);
		say(43, 6);
		say(53, 5, false, 1);
		say(43, 7);
		deferAction(424);
		break;
	case 424: // Turn away from the bed to operate the recorder.
		walkTo(Common::Point(462, 527), 7, true);
		deferAction(425);
		break;
	case 425: // Take out the recorder and press the record button.
		beginRoomAnimationRange(425, 17, 53, 0, 22);
		break;
	case 1425: // Record Gina's line while Brian holds the recorder.
		say(53, 5, false, 1);
		holdRoomAnimation(426, 17, 53, 22);
		break;
	case 1426: // Stop recording and put the recorder away.
		beginRoomAnimationRange(427, 17, 53, 28, 52);
		break;
	case 1427: // Keep Gina's recording alongside any recording of Gustav.
		_finale.recordings |= 1;
		walkTo(Common::Point(462, 505), 5, true);
		deferAction(428);
		break;
	case 428: // Ask Gina to be ready to leave once the plan is in motion.
		for (uint i = 8; i <= 10; ++i) {
			say(43, i);
			say(53, i - 2, false, 1);
		}
		_destination = 8091;
		deferAction(430);
		break;
	case 430: // Leave Gina resting and return to the house or the street.
		_ginaFrame = 0;
		walkTo(Common::Point(1023, 450), 3, true);
		leaveRoom(_destination);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8420::finishCustomFilm() {
	_ambientTick = _ginaTick = _now;
	if (_film == kWakingUp) {
		beginRoomAnimationRange(410, 6, 18, 0, 17);
	} else if (_film == kConfession) {
		deferAction(415);
	} else if (_film == kNightmare) {
		_screaming = _dreaming = false;
		_showBrian = true;
		_audio.stopEffect(1);
		_ginaFrame = 61;
		_position = Common::Point(462, 505);
		_walkPath.reset(_position);
		_facing = 5;
		deferAction(423);
	}
	_film = kNoFilm;
}

void Scene8420::startConfession() {
	typedef RunawaySequence S;
	const bool gog = Common::File::exists("DATAVC00.000");
	_filmCommands.clear();
	_filmCommands.push_back(S::Command(S::kOpen, 8));
	_filmCommands.push_back(S::Command(S::kStill, 0, gog));
	for (uint i = 4; i <= 6; ++i) {
		_filmCommands.push_back(S::Command(S::kTalk, 51, i, 1));
		if (gog) {
			_filmCommands.push_back(S::Command(S::kStill, i == 6 ? 0 : 1, 1));
		}
		_filmCommands.push_back(S::Command(S::kTalk, 41, i, i == 6 ? 1 : 3, i == 6));
		if (gog) {
			_filmCommands.push_back(S::Command(S::kStill, 0, 1));
		}
	}
	_filmCommands.push_back(S::Command(S::kTalk, 51, 7, 1));
	_filmCommands.push_back(S::Command(S::kMusic, 33, 255));
	if (!gog) {
		_filmCommands.push_back(S::Command(S::kPlay, 24, 24));
	}
	_filmCommands.push_back(S::Command(S::kSpeech, 51, 8, 8));
	_filmCommands.push_back(S::Command(S::kPlay, gog ? 0 : 25, gog ? 237 : 261));
	_filmCommands.push_back(S::Command(S::kMusic, 34, 255, 1));
	_filmCommands.push_back(S::Command(S::kWaitSpeech, 0, 0, 5));
	_filmCommands.push_back(S::Command(S::kEffect, 57, 26));
	_filmCommands.push_back(S::Command(S::kPlay, gog ? 238 : 282, gog ? 244 : 289));
	_filmCommands.push_back(S::Command(S::kSpeech, 51, 9, 9));
	_filmCommands.push_back(S::Command(S::kPlay, gog ? 245 : 290, gog ? 1057 : 1101));
	_filmCommands.push_back(S::Command(S::kWaitSpeech, 0, 0, 3, 1));
	_filmCommands.push_back(S::Command(S::kMusic, 16, 255, 1));
	for (uint i = 9; i <= 20; ++i) {
		if (i != 10 && i != 11) {
			if (gog) {
				_filmCommands.push_back(S::Command(S::kStill, 1, 1));
			}
			_filmCommands.push_back(S::Command(S::kTalk, 41, i, 3));
		}
		const bool offscreen = i == 9 || i == 11 || i == 20;
		if (gog) {
			_filmCommands.push_back(S::Command(S::kStill, offscreen ? 1 : 0, 1));
		}
		_filmCommands.push_back(S::Command(S::kTalk, 51, i + 1, offscreen ? 3 : 1, offscreen));
	}
	_filmCommands.push_back(S::Command(S::kEnd));
	_film = kConfession;
	_sequence.start(_filmCommands.data(), _text, _now, 'H');
}

} // End of namespace Runaway
} // End of namespace Hollywood
