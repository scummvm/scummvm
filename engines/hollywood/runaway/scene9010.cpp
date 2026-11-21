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
#include "hollywood/runaway/scene9010.h"

namespace Hollywood {
namespace {

typedef RunawaySequence S;

const uint kDepartureFadeStart = 41 * 80 + 256 * 40 + 3000;
const uint kDepartureDuration = kDepartureFadeStart + 2000;

const S::Command kFarewellSequence[] = {
	{S::kOpen, 3}, {S::kPlay, 0, 99}, {S::kEffect, 12, 115, 1, 1},
	{S::kPlay, 100, 133}, {S::kMusicVolume, 76},
	{S::kTalk, 1, 0}, {S::kTalk, 1, 1}, {S::kTalk, 1, 2}, {S::kTalk, 1, 3},
	{S::kEffect, 13, 128}, {S::kStopEffects, 2}, {S::kPlay, 134, 199},
	{S::kEffect, 12, 115, 1, 1}, {S::kPlay, 200, 220},
	{S::kTalk, 1, 4}, {S::kTalk, 1, 5}, {S::kTalk, 1, 6}, {S::kTalk, 1, 7},
	{S::kEffect, 14, 153}, {S::kStopEffects, 2}, {S::kPlay, 221, 342},
	{S::kSpeech, 1, 10, 10}, {S::kPlay, 343, 517},
	{S::kEffect, 12, 115, 1, 1}, {S::kPlay, 518, 593}, {S::kWaitSpeech},
	{S::kTalk, 1, 11, 1}, {S::kTalk, 1, 12, 3},
	{S::kPlay, 606, 649}, {S::kTalk, 1, 13, 3}, {S::kTalk, 1, 14, 1},
	{S::kTalk, 1, 15, 1, 1}, {S::kTalk, 1, 16, 1}, {S::kTalk, 1, 17, 3},
	{S::kTalk, 1, 18, 5}, {S::kEffect, 15, 153}, {S::kStopEffects, 2},
	{S::kPlay, 674, 772}, {S::kSpeech, 1, 19, 19}, {S::kMusicVolume, 255},
	{S::kPlay, 773, 875}, {S::kWaitSpeech}, {S::kEnd}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene9010 = {
	9010, 2048, 900, nullptr, 0, {{100, 100, 100}}, Common::Point(0, 0),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene9010::Scene9010(RunawayAudio &audio, RunawayInventory &, const RunawayStrings &, RunawayGameType) :
	_audio(audio), _sequence(audio) {}

bool Scene9010::load(uint16 entryScene) {
	_entryScene = entryScene;
	_gog = Common::File::exists("DATAVC00.000");
	_screen.create(kRunawayScreenWidth, kRunawayScreenHeight, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	ChunkArchive archive;
	return archive.open(Common::Path("RESOURCE.I01")) &&
		loadRunawaySurface(archive, 0, 2048, 900, _background) &&
		_car.load(archive, 1, 136) && archive.readVariableChunk(2, _parallax) &&
		_text.loadStage("RESOURCE.003", "Runaway Berkeley farewell", 801) && _credits.load();
}

void Scene9010::start(uint32 now) {
	_now = now;
	resume();
}

void Scene9010::resume() {
	_audio.playMusic(32);
	_audio.setMusicVolume(255);
	_audio.playMusicEffect(35, 255, 2, true);
	enter(kArrival);
}

bool Scene9010::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!entering || !state.valid() || state.game != kRunawayFullGame || state.scene != _entryScene) {
		return false;
	}
	_now = now;
	return true;
}

void Scene9010::captureState(RunawaySaveState &state) const {
	state.scene = _entryScene;
	state.x = state.y = state.camera = state.cameraY = 0;
}

void Scene9010::enter(Step step) {
	_step = step;
	_started = _now;
	switch (step) {
	case kArrival: // Pan from Berkeley to Brian and Gina's car.
		_camera = Common::Point(768, 300);
		_frame = 0;
		break;
	case kCarDoor: // Open the car door before the farewell conversation.
		_camera.x = 0;
		_audio.playEffect(11);
		break;
	case kFarewell: // Gina persuades Brian to reconsider Berkeley.
		startFarewell();
		break;
	case kDeparture: // Follow the departing car and pan up across the bay.
		_frame = 60;
		_camera = Common::Point(0, 300);
		break;
	case kFlight: // The plane takes off, followed by the travel montage.
		_commands.clear();
		_commands.push_back(S::Command(S::kOpen, 4));
		_commands.push_back(S::Command(S::kMusic, 27, 255));
		_commands.push_back(S::Command(S::kEffect, 62, 204));
		_commands.push_back(S::Command(S::kPlay, 0, _gog ? 3068 : 3069));
		_commands.push_back(S::Command(S::kEnd));
		_sequence.start(_commands.data(), _text, _now, 'I');
		break;
	case kCredits: // The original rolls the credits before Brian's epilogue.
		_credits.start(_now);
		break;
	case kMusicTail: // Let the montage's music finish before the epilogue.
		break;
	case kFinished: // Continue with Brian's final address to the player.
		_audio.stopEffect();
		_transition.scene = 9030;
		break;
	}
}

void Scene9010::startFarewell() {
	_commands.clear();
	for (uint i = 0; i < ARRAYSIZE(kFarewellSequence); ++i) {
		S::Command command = kFarewellSequence[i];
		if (_gog) {
			// Native GOG 0x52e470 uses I03.002 for the two close-up stills.
			if (command.operation == S::kTalk && command.b >= 11) {
				_commands.push_back(S::Command(S::kStill, command.b == 18 ? 638 : (command.c - 1) / 2, command.b == 18 ? 0 : 1));
			} else if (command.operation == S::kPlay) {
				switch (command.a) {
				case 606:
					command.a = 594;
					command.b = 637;
					break;
				case 674:
					command.a = 639;
					command.b = 735;
					break;
				case 773:
					command.a = 736;
					command.b = 839;
					break;
				default:
					break;
				}
			}
		}
		_commands.push_back(command);
	}
	// The car sounds and campus ambience continue across the film cuts.
	_sequence.start(_commands.data(), _text, _now, 'I', true);
}

void Scene9010::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	const uint elapsed = now - _started;
	switch (_step) {
	case kArrival:
		_camera.x = 768 - MIN<uint>(768, elapsed > 1000 ? (elapsed - 1000) / 40 * 4 : 0);
		if (!_camera.x) {
			enter(kCarDoor);
		}
		break;
	case kCarDoor:
		_frame = MIN<uint>(12, elapsed > 2000 ? (elapsed - 2000) / 80 : 0);
		if (elapsed >= 3040) {
			enter(kFarewell);
		}
		break;
	case kFarewell:
	case kFlight:
		_sequence.update(now);
		if (!_sequence.active()) {
			enter(_step == kFarewell ? kDeparture : kCredits);
		}
		break;
	case kDeparture: {
		_frame = 60 + MIN<uint>(75, elapsed / 80);
		const uint pan = elapsed > 3280 ? MIN<uint>(256, (elapsed - 3280) / 40) : 0;
		_camera = Common::Point(pan * 4, 300 - pan);
		if (elapsed >= kDepartureDuration) {
			enter(kFlight);
		}
		break;
	}
	case kCredits:
		_credits.update(now);
		if (_credits.finished()) {
			enter(kMusicTail);
		}
		break;
	case kMusicTail:
		if (!_audio.musicPlaying() || elapsed >= 20000) {
			enter(kFinished);
		}
		break;
	case kFinished:
		break;
	}
}

void Scene9010::skipVideo() {
	_sequence.skip();
	_sequence.update(_now);
	_audio.stopMusic();
	enter(kFinished);
}

void Scene9010::draw() {
	if (_sequence.active()) {
		_sequence.draw(_screen);
		return;
	}
	if (_step == kCredits) {
		_credits.draw(_screen);
		return;
	}
	_screen.clear();
	if (_step == kMusicTail || _step == kFinished) {
		return;
	}
	Graphics::ManagedSurface viewport(_screen, Common::Rect(0, kRunawayViewportTop, kRunawayScreenWidth,
		kRunawayViewportTop + kRunawayViewportHeight));
	viewport.blitFrom(_background, Common::Rect(_camera.x, _camera.y, _camera.x + 1024, _camera.y + 600), Common::Point(0, 0));
	_car.draw(viewport, _frame, _camera.x, -_camera.y);
	// The foreground building moves at a different rate from the campus.
	applyRunawayPatch(_parallax, viewport, 300 - _camera.y * 2, (6128 - _camera.x) / 4 - _camera.x);
	if (_step == kArrival) {
		dimRunawayViewport(_screen, MIN<uint>(1000, _now - _started));
	} else if (_step == kDeparture && _now - _started > kDepartureFadeStart) {
		dimRunawayViewport(_screen, kDepartureDuration - MIN<uint32>(kDepartureDuration, _now - _started), 2000);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
