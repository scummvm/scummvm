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

#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene8230.h"

namespace Hollywood {
namespace {

// Speech poses begin at 40 and 110; the other spans move Rutger around his bench.
const byte kRutgerFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 33, 33, 33, 33, 33, 33,
	33, 34, 35, 36, 37, 38, 33, 34, 35, 36, 37, 38, 33, 33, 33, 33, 33, 33, 33, 33,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
	79, 80, 81, 82, 82, 82, 82, 82, 82, 82, 82, 83, 84, 85, 86, 87, 82, 83, 84, 85,
	86, 87, 82, 82, 82, 82, 82, 82, 82, 82,
	93, 94, 95, 96, 97, 98, 99, 100, 100, 100,
	32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13,
	12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kPipeFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	39, 40, 41, 42, 43, 44, 39, 40, 41, 42, 43, 44, 39, 39, 39, 39, 39, 39, 39, 39,
	46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
	66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
	86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105,
	106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
	126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
	146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165,
	166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 39, 39, 39, 39, 39,
	38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22,
	183, 184, 185, 186, 187, 188, 189, 190, 191, 192
};

const uint kRutgerPuffStart = 165 - 59;
const uint kBrianPuffStart = 64;
const uint kSmokeFrames = 39;

} // End of anonymous namespace

const RunawayRoom kRunawayScene8230 = {
	8230, 1024, 600, nullptr, 0,
	{{100, 100, 100}, {55, 55, 55}, {50, 50, 50}, {45, 45, 45}, {40, 40, 40}}, Common::Point(698, 560),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8230::Scene8230(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8230::loadCustomResources() {
	if (!loadForeground(0, 4) || !_plants.load(_archive, 5, 36) || !_rutger.load(_archive, 6, 101)) {
		return false;
	}
	return _entryScene != 8231 || (_pipeRutger.load(_archive, 8, 193) && _pipeBrian.load(_archive, 9, 109) &&
		_smoke[0].load(_archive, 11, kSmokeFrames) && _smoke[1].load(_archive, 12, kSmokeFrames));
}

float Scene8230::actorHeight() const {
	return _position.y * 1.076923132f - 148.0769196f;
}

void Scene8230::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

void Scene8230::prepareCustomGameplayLoop() {
	_rutgerTick = _now;
}

void Scene8230::resumeAmbientAudio(bool entering) {
	_audio.stopEffect(2);
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(19);
	}
}

void Scene8230::runCustomEntrySequence() {
	_position = Common::Point(698, 560);
	_walkPath.reset(_position);
	_facing = 7;
	walkTo(Common::Point(541, 495), 7);
	moveRutger(0, 33, 40);
	switch (_entryScene) {
	case 8231: // Trade the tomahawk for help with the first medium test.
		_inventory.remove(9);
		deferAction(410);
		break;
	case 8232: // Deliver the yawaskel seeds for the stronger brew.
		_inventory.remove(10);
		deferAction(420);
		break;
	default:
		_finale.rutgerPlantHint = 0;
		deferAction(400);
		break;
	}
}

void Scene8230::moveRutger(uint first, uint last, uint idle) {
	assert(first <= last && last < (_pipe ? ARRAYSIZE(kPipeFrames) : ARRAYSIZE(kRutgerFrames)));
	_rutgerStep = first;
	_rutgerLast = last;
	_rutgerIdle = idle;
	_rutgerTick = _now;
	_movingRutger = true;
}

void Scene8230::advanceCustomGameplayLoop(uint32 delta) {
	while (_movingRutger && _now - _rutgerTick >= 80) {
		_rutgerTick += 80;
		if (_rutgerStep < _rutgerLast) {
			++_rutgerStep;
		} else {
			_movingRutger = false;
			_rutgerStep = _rutgerIdle;
		}
	}
}

bool Scene8230::updateCustomSequence() {
	return _movingRutger;
}

bool Scene8230::shouldDrawActor() const {
	return !_brianPipePose;
}

bool Scene8230::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	if (!drawingRoomAnimation() || (_roomAction != 413 && _roomAction != 415)) {
		return false;
	}
	const bool brian = _roomAction == 415;
	const uint start = brian ? kBrianPuffStart : kRutgerPuffStart;
	const uint smokeFrame = _actionStep >= start && _actionStep < start + kSmokeFrames - 1 ?
		_actionStep - start + 1 : 0;
	// Rutger stays behind Brian. Both smoke clouds pass behind the hanging plants.
	if (brian) {
		_secondaryAnimation.draw(viewport, _secondaryFrames[_actionStep], _camera.x, -_camera.y);
		_actionAnimation.draw(viewport, _actionFrames[_actionStep], _camera.x, -_camera.y);
		_smoke[1].draw(viewport, smokeFrame, _camera.x, -_camera.y);
	} else {
		_actionAnimation.draw(viewport, _actionFrames[_actionStep], _camera.x, -_camera.y);
		_secondaryAnimation.draw(viewport, _secondaryFrames[_actionStep], _camera.x, -_camera.y);
		_smoke[0].draw(viewport, smokeFrame, _camera.x, -_camera.y);
		RunawayActor::RenderParams params;
		params.position = _position;
		params.camera = _camera;
		params.roomWidth = _background.w;
		params.height = actorHeight();
		params.antialias = _antialias;
		const byte light = MIN<byte>(4, findRunawayLight(_position, _background.w, _mask, _maps));
		memcpy(params.light, kRunawayScene8230.light[light], sizeof(params.light));
		_actor.draw(_screen, params, _mask, _maps);
	}
	drawAmbientAnimations(viewport, &_plants, 1);
	drawCustomForegroundComposite(viewport);
	return true;
}

void Scene8230::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, &_plants, 1);
	const bool pipeAction = drawingRoomAnimation() && _roomAction == 411;
	if (!pipeAction) {
		uint step = _rutgerStep;
		if (!_movingRutger && speaker()) {
			step += speechFrame() % 6;
		}
		const RunawayAnimation &animation = _pipe ? _pipeRutger : _rutger;
		animation.draw(viewport, _pipe ? kPipeFrames[step] : kRutgerFrames[step], _camera.x, -_camera.y);
	}
	if (_brianPipePose) {
		const uint frame = 96 + (speaking() && !speaker() ? speechFrame() : 0);
		_pipeBrian.draw(viewport, frame, _camera.x, -_camera.y);
	}
}

void Scene8230::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.y < 521) {
		drawForeground(viewport, 0);
	}
}

void Scene8230::startRutgerConversation() {
	startConversation(1);
	static const DialogueChoice root[] = {
		{1, 0, 3, 6, 6, 1, 255}, {1, 0, 3, 7, 7, 1, 255},
		{0, 0, 1, 8, 8, 10, 255}, {0, 1, 1, 9, 9, 1, 255},
		{0, 2, 1, 10, 10, 11, 255}, {1, 0, 3, 11, 11, 1, 255},
		{0, 0, 3, 12, 12, 1, 255}, {1, 0, 0, 13, 13, 0, 0}
	};
	memcpy(_dialogue[0], root, sizeof(root));
	_dialogue[10][0] = {1, 0, 2, 14, 14, 2, 255};
	_dialogue[11][0] = {1, 0, 2, 17, 17, 1, 255};
	_dialogue[12][0] = {1, 0, 0, 18, 18, 3, 255};
	if (_finale.mediumTests == 1) {
		_dialogue[0][2].enabled = !_finale.rutgerTranceDiscussed;
		_dialogue[0][3].enabled = _finale.rutgerTranceDiscussed;
	} else if (_finale.mediumTests == 2) {
		_dialogue[0][4].enabled = !_finale.rutgerBrewDiscussed;
	}
	_dialogue[0][6].enabled = _finale.mediumTests >= 3 && !_finale.trailerFound;
}

void Scene8230::handleCustomDialogueChoice(byte effect) {
	_finale.rutgerPlantHint = 0;
	switch (effect) {
	case 2: // Rutger asks for something in exchange for a trance-inducing herb.
		for (uint i = 15; i <= 16; ++i) {
			say(80, i);
			say(90, i, false, 1);
		}
		break;
	case 3: // Learn about yawaskel and recognize its pods after leaving the bench.
		_finale.rutgerPlantHint = 1;
		for (uint i = 19; i <= 22; ++i) {
			say(80, i);
			say(90, i, false, 1);
		}
		break;
	case 10: // Remember the request for help with the first medium test.
		_finale.rutgerTranceDiscussed = 1;
		break;
	case 11: // Remember the stronger brew needed for the second medium test.
		_finale.rutgerBrewDiscussed = 1;
		break;
	default:
		break;
	}
}

void Scene8230::finishCustomConversation() {
	if (!findRunawayRoom(8242)) {
		leaveRoom(8242);
		startRutgerConversation();
		return;
	}
	walkTo(Common::Point(698, 560), 3);
	moveRutger(140, 172, 172);
	leaveRoom(8242);
}

void Scene8230::startRutgerPuff() {
	Common::Array<byte> frames, embers;
	for (uint step = 59; step <= 195; ++step) {
		const uint hold = step == 165 ? kSmokeFrames : 1;
		for (uint i = 0; i < hold; ++i) {
			frames.push_back(kPipeFrames[step]);
			embers.push_back(step >= 121 && step <= 150 ? step - 120 : 0);
		}
	}
	beginRoomAnimation(413, 8, 193, frames.data(), frames.size(), false);
	setRoomAnimationOverlay(10, 31, embers.data(), embers.size());
}

void Scene8230::startBrianPuff() {
	Common::Array<byte> brian, rutger;
	for (uint step = 0; step < 80; ++step) {
		const byte frame = step < 7 ? 34 : step < 19 ? 40 - step : 17 + step;
		const uint hold = step == kBrianPuffStart ? kSmokeFrames : 1;
		for (uint i = 0; i < hold; ++i) {
			const uint rutgerStep = 199 + brian.size();
			brian.push_back(frame);
			rutger.push_back(rutgerStep < ARRAYSIZE(kPipeFrames) ? kPipeFrames[rutgerStep] : 0);
		}
	}
	beginRoomAnimation(415, 9, 109, brian.data(), brian.size());
	setRoomAnimationOverlay(8, 193, rutger.data(), rutger.size());
}

RunawayRoomScene::ActionResult Scene8230::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // Introduce Brian or greet Rutger after a previous visit.
		if (!_finale.rutgerMet) {
			for (uint i = 0; i <= 2; ++i) {
				say(80, i);
				say(90, i, false, 1);
			}
			_finale.rutgerMet = 1;
		} else {
			say(80, 3);
			if (_finale.rutgerBrew == 2) {
				_finale.rutgerBrew = 3;
				say(90, 4, false, 1);
				say(80, 5);
				say(90, 5, false, 1);
			} else {
				say(90, 3, false, 1);
			}
		}
		startRutgerConversation();
		break;
	case 410: // Offer Rutger the Hopi axe-pipe.
		say(80, 3);
		say(90, 3, false, 1);
		say(16, 2);
		say(16, 3, false, 1);
		deferAction(411);
		break;
	case 411: { // Hand over the pipe while Rutger reaches for it.
		byte brian[34];
		for (uint i = 0; i < ARRAYSIZE(brian); ++i) {
			brian[i] = i + 1;
		}
		_pipe = true;
		beginRoomAnimation(411, 8, 193, kPipeFrames, ARRAYSIZE(brian));
		setRoomAnimationOverlay(9, 109, brian, ARRAYSIZE(brian));
		break;
	}
	case 1411: // Rutger settles the pipe on his bench.
		moveRutger(34, 39, 40);
		deferAction(412);
		break;
	case 412: // Agree on the exchange before Rutger tries the pipe.
		say(16, 4, false, 1);
		say(16, 5);
		say(16, 6, false, 1);
		deferAction(413);
		break;
	case 413: // Rutger lights the pipe and blows out smoke.
		startRutgerPuff();
		break;
	case 1413: // Rutger invites Brian to try it.
		_rutgerStep = 40;
		for (uint i = 7; i <= 12; ++i) {
			say(16, i, false, i & 1);
		}
		deferAction(415);
		break;
	case 415: // Brian accepts the pipe and takes a puff.
		startBrianPuff();
		break;
	case 1415: // Brian feels dizzy after smoking.
		_pipe = false;
		_brianPipePose = true;
		_rutgerStep = 40;
		say(16, 13);
		deferAction(416);
		break;
	case 416: // Return to Mama Dorita for the second medium test.
		leaveRoom(8102);
		break;
	case 420: // Show Rutger the yawaskel seeds.
		say(80, 3);
		say(90, 3, false, 1);
		for (uint i = 1; i <= 5; ++i) {
			say(17, i, false, !(i & 1));
		}
		deferAction(421);
		break;
	case 421: // Step around Rutger's bench.
		walkTo(Common::Point(564, 549));
		deferAction(422);
		break;
	case 422: // Rutger makes room for Brian to put down the seeds.
		moveRutger(60, 63, 63);
		walkTo(Common::Point(288, 517), 0);
		deferAction(423);
		break;
	case 423: { // Pour the seeds onto the bench.
		byte frames[56];
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			frames[i] = i < 18 ? i : i < 48 ? 17 : i - 30;
		}
		beginRoomAnimation(423, 7, 26, frames, ARRAYSIZE(frames), true, 60);
		break;
	}
	case 1423: // Step away while Rutger examines the seeds.
		moveRutger(64, 103, 110);
		walkTo(Common::Point(564, 549));
		deferAction(424);
		break;
	case 424: // Return to the front of the bench.
		walkTo(Common::Point(541, 495), 7);
		deferAction(425);
		break;
	case 425: // Rutger agrees to make the Hopi brew.
		say(17, 6, false, 1);
		say(17, 7);
		say(17, 8, false, 1);
		deferAction(426);
		break;
	case 426: // Let Rutger get to work.
		say(17, 9);
		moveRutger(130, 137, 40);
		deferAction(427);
		break;
	case 427: // Wait outside for the brew to be prepared.
		_finale.rutgerBrew = 1;
		leaveRoom(8182);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8230::handleAnimationFrameHook() {
	if (_roomAction == 413 && _actionStep == 121 - 59) {
		_audio.playEffect(34, 230);
	} else if ((_roomAction == 413 && _actionStep == kRutgerPuffStart) ||
			(_roomAction == 415 && _actionStep == kBrianPuffStart)) {
		_audio.playEffect(35, 191);
	} else if (_roomAction == 423 && _actionStep == 17) {
		_audio.playEffect(36);
		if (!applyPatch(13)) {
			error("Cannot draw Runaway yawaskel seeds");
		}
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
