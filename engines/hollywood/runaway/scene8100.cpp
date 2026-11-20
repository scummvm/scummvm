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
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/scene8100.h"

namespace Hollywood {
namespace {

// Walking sequences alternate with idle, speech and blink poses at 20, 80 and 150.
const byte kDoritaFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 15, 15, 15, 15, 15, 15, 15, 15,
	28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 58, 58, 58, 58, 58, 58, 58, 58,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 8, 9,
	10, 11, 12, 13, 14, 15, 15, 15, 15, 15,
	14, 13, 12, 11, 10, 9, 8, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 97, 97, 97,
	97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 97, 97, 97, 97, 97, 97, 97, 97,
	96, 95, 94, 93, 92, 91, 90, 89, 88, 8, 9, 10, 11, 12, 13, 14, 15
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8100 = {
	8100, 1024, 600, nullptr, 0,
	{{100, 100, 100}, {65, 65, 65}, {55, 55, 55}, {75, 75, 75}}, Common::Point(332, 470),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8100::Scene8100(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8100::loadCustomResources() {
	Common::Array<byte> patch;
	_altar.create(kRunawayScreenWidth, kRunawayViewportHeight, _screen.format);
	return _dorita.load(_archive, 4, 110) && _prayerAnimation.load(_archive, 8, 22) &&
		_archive.readVariableChunk(7, patch) && applyRunawayPatch(patch, _altar);
}

bool Scene8100::usesAlternateActor() const {
	return false;
}

byte Scene8100::initialActorFacing() const {
	return 1;
}

float Scene8100::actorHeight() const {
	return _position.y * 0.778894484f - 11.557788849f;
}

void Scene8100::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

void Scene8100::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

void Scene8100::prepareCustomGameplayLoop() {
	_doritaTick = _ambientTick = _now;
}

void Scene8100::runCustomEntrySequence() {
	_facing = 1;
	switch (_entryScene) {
	case 8101: // Return from Gina's room with Mama Dorita.
		_position = Common::Point(0, 428);
		_walkPath.reset(_position);
		walkTo(Common::Point(332, 470), 1);
		moveDorita(100, 125, 20);
		deferAction(402);
		break;
	case 8102: // Retry the medium test after preparing outside.
	case 8103: // The third medium test leads to the séance.
		_mediumTest = _entryScene - 8100;
		_fade = kMediumTestFade;
		_fadeStarted = _now;
		deferAction(409);
		break;
	default:
		if (_finale.doritaMet) {
			_position = Common::Point(0, 599);
			_walkPath.reset(_position);
			walkTo(Common::Point(332, 470), 1);
			moveDorita(0, 15, 20);
		}
		deferAction(401);
		break;
	}
}

void Scene8100::resumeAmbientAudio(bool entering) {
	_audio.stopEffect();
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(22);
	}
}

void Scene8100::moveDorita(uint first, uint last, uint idle) {
	assert(first <= last && last < ARRAYSIZE(kDoritaFrames));
	_doritaStep = first;
	_doritaLast = last;
	_doritaIdle = idle;
	_doritaTick = _now;
	_movingDorita = true;
	_blink = 0;
}

void Scene8100::advanceCustomGameplayLoop(uint32 delta) {
	if (_now - _ambientTick >= 250) {
		_ambientTick = _now;
		if (_fade != kSeanceFade && !_audio.effectPlaying(2) && !_random.getRandomNumber(49)) {
			byte effect;
			do {
				effect = 74 + _random.getRandomNumber(4);
			} while (effect == _lastAmbientEffect);
			_lastAmbientEffect = effect;
			_audio.playEffect(effect, 3, 2);
		}
	}
	if (_now - _doritaTick < 80) {
		return;
	}
	_doritaTick = _now;
	if (_movingDorita) {
		if (_doritaStep < _doritaLast) {
			++_doritaStep;
		} else {
			_movingDorita = false;
			_doritaStep = _doritaIdle;
		}
	} else if (speaker()) {
		_blink = 0;
	} else if (_blink) {
		_blink = _blink == 6 ? 0 : _blink + 1;
	} else if (!_random.getRandomNumber(14)) {
		_blink = 1;
	}
}

bool Scene8100::updateCustomSequence() {
	if (_fade == kMediumTestFade) {
		if (_now - _fadeStarted < 500) {
			return true;
		}
		_fade = kNoFade;
	}
	return _movingDorita;
}

void Scene8100::draw() {
	RunawayRoomScene::draw();
	if (_fade == kMediumTestFade) {
		dimRunawayViewport(_screen, MIN<uint>(500, _now - _fadeStarted), 500);
	} else if (_fade == kSeanceFade) {
		dimRunawayViewport(_screen, 250 - MIN<uint>(250, _now - _fadeStarted), 250);
	}
}

void Scene8100::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	uint step = _doritaStep;
	if (!_movingDorita) {
		step += speaker() ? speechFrame() % 6 : _blink ? 5 + _blink : 0;
	}
	_dorita.draw(viewport, kDoritaFrames[step], _camera.x, -_camera.y);
}

bool Scene8100::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	if (!_showAltar) {
		return false;
	}
	viewport.blitFrom(_altar);
	if (drawingRoomAnimation()) {
		_actionAnimation.draw(viewport, _actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)], 0, 0);
	} else {
		const uint frame = speaking() && !speaker() ? 15 + speechFrame() % 6 : 15;
		_prayerAnimation.draw(viewport, frame, 0, 0);
	}
	return true;
}

RunawayRoomScene::ActionResult Scene8100::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 401: // Introduce Mama Dorita or greet her on a later visit.
		if (!_finale.doritaMet) {
			say(90, 0, false, 1);
			say(80, 1);
			say(90, 1, false, 1);
			say(80, 2);
			say(90, 2, false, 1);
			say(80, 4);
		} else {
			say(80, 3);
			say(90, 3, false, 1);
		}
		startDoritaConversation();
		break;
	case 402: // Resume the conversation after seeing Gina.
		startDoritaConversation();
		break;
	case 403: // Leave the conversation through the selected doorway.
		leaveHouse();
		break;
	case 409: // Greet Mama Dorita once the return visit has faded in.
		say(80, 12 + _mediumTest * 3);
		say(90, 12 + _mediumTest * 3, false, 1);
		startMediumTest(_mediumTest);
		break;
	case 410: // Brian leans forward for Mama Dorita to examine his eyes.
		beginRoomAnimationRange(410, 5, 23, 0, 12);
		break;
	case 1410: { // Play the appropriate medium test and its close-up response.
		typedef RunawaySequence S;
		static const uint16 lastFrames[] = {32, 55, 40};
		const uint16 last = lastFrames[_mediumTest - 1];
		const byte response = 16 + (_mediumTest - 1) * 3;
		_filmCommands.clear();
		_filmCommands.push_back({S::kOpen, _mediumTest});
		_filmCommands.push_back({S::kPlay, 0, last});
		_filmCommands.push_back({S::kStill, last});
		_filmCommands.push_back({S::kSpeech, 90, response, response});
		_filmCommands.push_back(S::Command(S::kWaitSpeech));
		_filmCommands.push_back(S::Command(S::kEnd));
		_sequence.start(_filmCommands.data(), _text, _now, 'H');
		break;
	}
	case 411: // Brian steps back after the close-up.
		beginRoomAnimationRange(411, 5, 23, 22, 0);
		break;
	case 1411: // Record the test result and continue talking.
		_finale.mediumTests = MAX(_finale.mediumTests, _mediumTest);
		say(90, 17 + (_mediumTest - 1) * 3, false, 1);
		if (_mediumTest == 3) {
			_conversation = 0;
			_afterConversation = 0;
			for (uint i = 25; i <= 27; ++i) {
				say(80, i);
				say(90, i, false, 1);
			}
			deferAction(420);
		} else if (!_conversation) {
			startDoritaConversation();
		}
		break;
	case 420: // Follow Mama Dorita to the altar.
		walkTo(Common::Point(518, 492), 1);
		moveDorita(130, 146, 150);
		deferAction(421);
		break;
	case 421: // Prepare to kneel at the altar.
		say(80, 28);
		say(90, 28, false, 1);
		deferAction(433);
		break;
	case 433: // Kneel after Mama Dorita has finished giving her instructions.
		beginRoomAnimationRange(421, 6, 21, 0, 20);
		break;
	case 1421: // Switch to the prayer close-up.
		_showAltar = true;
		beginRoomAnimationRange(424, 8, 22, 0, 15);
		break;
	case 1424: // Mama Dorita dictates the cleansing prayers.
		say(90, 30, false, 1);
		say(80, 30);
		say(90, 32, false, 1);
		startPrayers();
		break;
	case 422: // Finish praying and leave the close-up.
		beginRoomAnimationRange(422, 8, 22, 15, 0);
		break;
	case 1422: // Brian stands back up.
		_showAltar = false;
		beginRoomAnimationRange(423, 6, 21, 20, 0);
		break;
	case 1423: // Ask Mama Dorita where the séance will take place.
		_facing = 7;
		say(80, 40);
		say(90, 40, false, 1);
		say(80, 41);
		say(90, 41, false, 1);
		deferAction(425);
		break;
	case 425: // Approach the well.
		walkTo(Common::Point(143, 435), 0);
		moveDorita(170, 186, 20);
		deferAction(426);
		break;
	case 426: // Mama Dorita walks toward the back of the room.
		moveDorita(40, 70, 80);
		deferAction(427);
		break;
	case 427: // Brian hesitates at the opening.
		say(80, 42);
		say(90, 42, false, 1);
		deferAction(428);
		break;
	case 428: // Ask whether the descent is really necessary.
		_facing = 3;
		say(80, 43);
		deferAction(429);
		break;
	case 429: // Brian climbs down into the séance room.
		_facing = 0;
		beginRoomAnimationRange(429, 9, 26, 0, 25);
		break;
	case 1429: // Fade the room after Brian has climbed down the well.
		_fade = kSeanceFade;
		_fadeStarted = _now;
		beginRoomAnimationRange(430, 9, 26, 25, 25, true, 250);
		break;
	case 1430: // Begin the séance with Gina and Mama Dorita.
		_fade = kNoFade;
		startSeance();
		break;
	case 440: // Continue to Gina after learning about her father.
		_finale.doritaMet = 1;
		leaveRoom(8421);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8100::handleAnimationFrameHook() {
	if (_roomAction == 429 && _actionStep == 3) {
		_audio.playEffect(43);
	}
}

void Scene8100::startDoritaConversation() {
	startConversation(1);
	_visitingGina = false;
	_afterConversation = 403;
	static const DialogueChoice topics[] = {
		{0, 0, 1, 4, 5, 10, 255}, {0, 0, 1, 5, 5, 10, 255},
		{0, 0, 1, 6, 6, 10, 255}, {1, 0, 0, 7, 7, 2, 255},
		{1, 0, 3, 8, 8, 1, 255}, {0, 0, 3, 9, 9, 1, 255},
		{1, 0, 0, 10, 10, 0, 0}
	};
	static const DialogueChoice medium[] = {
		{1, 0, 3, 12, 12, 1, 255}, {1, 0, 3, 13, 13, 1, 255},
		{1, 0, 3, 14, 14, 1, 255}, {1, 0, 3, 15, 15, 3, 255},
		{0, 0, 3, 18, 18, 4, 255}, {0, 0, 0, 21, 21, 5, 0},
		{1, 0, 4, 24, 24, 0, 0}
	};
	memcpy(_dialogue[0], topics, sizeof(topics));
	_dialogue[10][0] = {1, 0, 1, 11, 11, 1, 255};
	memcpy(_dialogue[20], medium, sizeof(medium));
	_dialogue[0][1].enabled = _finale.mediumTests < 3 && !_finale.mediumDiscussed;
	_dialogue[0][2].enabled = _finale.mediumTests < 3 && _finale.mediumDiscussed;
	_dialogue[0][5].enabled = _finale.mediumTests >= 3 && !_finale.trailerFound;
	_dialogue[20][3].enabled = _finale.mediumTests < 2;
}

void Scene8100::handleCustomDialogueChoice(byte effect) {
	if (_conversation == 2) {
		if (effect == 1) { // Ask about finding a different medium.
			_dialogue[0][0].enabled = 1;
			say(80, 30);
			say(90, 32 + _prayer, false, 1);
		} else if (effect >= 2 && effect <= 4) { // Repeat the next prayer.
			++_prayer;
			if (_prayer < 3) {
				_dialogue[0][_prayer + 1].enabled = 1;
			}
		}
		return;
	}
	if (effect == 10) { // Learn why Brian must act as the medium.
		_finale.mediumDiscussed = 1;
	} else if (effect == 2) { // Ask to visit Gina.
		_visitingGina = true;
	} else if (effect >= 3 && effect <= 5) { // Volunteer for a medium test.
		startMediumTest(effect - 2);
	}
}

void Scene8100::startMediumTest(byte test) {
	_mediumTest = test;
	_finale.mediumDiscussed = 1;
	deferAction(410);
}

void Scene8100::startPrayers() {
	startConversation(2);
	_afterConversation = 422;
	_prayer = 0;
	static const DialogueChoice prayers[] = {
		{1, 0, 3, 31, 31, 1, 0}, {1, 0, 3, 32, 33, 2, 0},
		{0, 0, 3, 33, 34, 3, 0}, {0, 0, 0, 34, 35, 4, 0}
	};
	memcpy(_dialogue[0], prayers, sizeof(prayers));
}

void Scene8100::leaveHouse() {
	uint16 destination = 8091;
	if (_visitingGina) {
		if (_finale.mediumTests < 3) {
			destination = 8420;
		} else if (_finale.recordingPlan && (_finale.recordings == 0 || _finale.recordings == 2)) {
			destination = 8422;
		}
	}
	if (!findRunawayRoom(destination)) {
		leaveRoom(destination);
		startDoritaConversation();
		return;
	}
	_finale.doritaMet = 1;
	_finale.ginaVisitPending = _visitingGina && destination == 8091;
	walkTo(Common::Point(0, _visitingGina ? 428 : 599), 5);
	if (_visitingGina) {
		moveDorita(40, 70, 80);
	}
	leaveRoom(destination);
}

void Scene8100::finishCustomFilm() {
	if (_seanceFilm) {
		_seanceFilm = false;
		_finale.seanceComplete = 1;
		_finale.gangsters = 1;
		_audio.stopMusic();
		deferAction(440);
	} else {
		execute(411);
	}
}

void Scene8100::startSeance() {
	typedef RunawaySequence S;
	static const S::Command commands[] = {
		{S::kOpen, 4}, {S::kMusic, 30, 96, 1},
		{S::kPlay, 0, 9}, {S::kSpeech, 20, 0}, {S::kPlay, 10, 58}, {S::kWaitSpeech},
		{S::kTalk, 20, 1, 1},
		{S::kSpeech, 20, 2, 2}, {S::kPlay, 1128, 1253, S::kLoopDuringSpeech},
		{S::kSpeech, 20, 3, 3}, {S::kPlay, 59, 299, S::kLoopDuringSpeech},
		{S::kPlay, 300, 344}, {S::kTalk, 20, 4, 1},
		{S::kSpeech, 21, 4, 4}, {S::kPlay, 345, 583, S::kLoopDuringSpeech},
		{S::kSpeech, 20, 5, 5}, {S::kPlay, 589, 708, S::kLoopDuringSpeech},
		{S::kTalk, 20, 6, 5},
		{S::kSpeech, 20, 7, 7}, {S::kPlay, 589, 708, S::kLoopDuringSpeech},
		{S::kMusic, 31, 96}, {S::kPlay, 709, 749},
		{S::kSpeech, 20, 8, 8}, {S::kPlay, 750, 1107}, {S::kWaitSpeech},
		{S::kMusic, 30, 96, 1},
		{S::kTalk, 20, 9, 5}, {S::kTalk, 20, 10, 3}, {S::kTalk, 20, 11, 5},
		{S::kSpeech, 20, 12, 12}, {S::kPlay, 1266, 1516, S::kLoopDuringSpeech},
		{S::kSpeech, 20, 13, 13}, {S::kPlay, 1517, 1557}, {S::kWaitSpeech},
		{S::kSpeech, 20, 14, 14}, {S::kPlay, 1266, 1516, S::kLoopDuringSpeech},
		{S::kTalk, 20, 15, 5}, {S::kTalk, 20, 16, 3},
		{S::kSpeech, 20, 17, 17}, {S::kPlay, 1517, 1557}, {S::kWaitSpeech},
		{S::kSpeech, 20, 18, 18}, {S::kPlay, 1266, 1516, S::kLoopDuringSpeech},
		{S::kTalk, 20, 19, 5}, {S::kTalk, 20, 20, 3},
		{S::kSpeech, 20, 21, 21}, {S::kPlay, 1517, 1557}, {S::kWaitSpeech},
		{S::kSpeech, 20, 22, 22}, {S::kPlay, 1266, 1516, S::kLoopDuringSpeech},
		{S::kTalk, 20, 23, 5}, {S::kTalk, 20, 24, 1},
		{S::kSpeech, 20, 25, 25}, {S::kPlay, 345, 583, S::kLoopDuringSpeech},
		{S::kTalk, 20, 26, 1}, {S::kPlay, 1558, 1615}, {S::kEnd}
	};
	_seanceFilm = true;
	if (!Common::File::exists("DATAVC00.000")) {
		_sequence.start(commands, _text, _now, 'H');
		return;
	}

	// GOG separates the looping shots from the main movie (native 0x52a2b0).
	static const struct {
		uint16 first, last, gogFirst;
		byte suffix;
	} clips[] = {
		{0, 58, 0, 1}, {59, 299, 0, 3}, {300, 344, 58, 1},
		{345, 583, 0, 4}, {589, 708, 0, 5}, {709, 1107, 103, 1},
		{1128, 1253, 0, 6}, {1266, 1516, 0, 7}, {1517, 1557, 0, 8},
		{1558, 1615, 502, 1}
	};
	_filmCommands.clear();
	byte suffix = 1;
	for (uint i = 0; i < ARRAYSIZE(commands); ++i) {
		S::Command command = commands[i];
		if (command.operation == S::kTalk) {
			_filmCommands.push_back(S::Command(S::kStill, (command.c - 1) / 2, 1));
		} else if (command.operation == S::kPlay) {
			for (uint j = 0; j < ARRAYSIZE(clips); ++j) {
				if (command.a < clips[j].first || command.b > clips[j].last) {
					continue;
				}
				if (suffix != clips[j].suffix) {
					suffix = clips[j].suffix;
					_filmCommands.push_back(S::Command(S::kOpen, 4, suffix));
				}
				command.a = command.a - clips[j].first + clips[j].gogFirst;
				command.b = command.b - clips[j].first + clips[j].gogFirst;
				break;
			}
		}
		_filmCommands.push_back(command);
	}
	_sequence.start(_filmCommands.data(), _text, _now, 'H');
}

} // End of namespace Runaway
} // End of namespace Hollywood
