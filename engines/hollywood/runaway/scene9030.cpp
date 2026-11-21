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

#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/util.h"
#include "hollywood/runaway/scene9030.h"

namespace Hollywood {
namespace {

const byte kBrianFrames[] = {
	0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 16, 17, 18, 19, 20, 21, 16, 16, 16, 16, 16, 16, 16, 16,
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 0
};

const byte kWaveLayers[] = {8, 9, 10, 3, 4, 5, 6, 7};

} // End of anonymous namespace

const RunawayRoom kRunawayScene9030 = {
	9030, 1024, 2062, nullptr, 0, {{100, 100, 100}}, Common::Point(0, 0),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene9030::Scene9030(RunawayAudio &audio, RunawayInventory &, const RunawayStrings &, RunawayGameType) :
	_audio(audio), _random("runawayEpilogue"), _speech(audio, _random) {}

bool Scene9030::load(uint16 entryScene) {
	_entryScene = entryScene;
	_screen.create(kRunawayScreenWidth, kRunawayScreenHeight, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	ChunkArchive archive;
	if (!archive.open(Common::Path("RESOURCE.I03")) ||
			!loadRunawaySurface(archive, 0, 1024, 2062, _background) ||
			!_text.loadStage("RESOURCE.003", "Runaway epilogue", 100)) {
		return false;
	}
	const uint frames[] = {73, 22, 79, 501, 501, 343, 501, 501, 501, 501, 501};
	for (uint i = 0; i < ARRAYSIZE(_layers); ++i) {
		if (!_layers[i].load(archive, i + 1, frames[i])) {
			return false;
		}
	}
	return true;
}

void Scene9030::start(uint32 now) {
	_now = now;
	resume();
}

void Scene9030::resume() {
	_request = _ginaStep = 0;
	_frame = _cameraY = 0;
	enter(kExplanation);
}

bool Scene9030::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!entering || !state.valid() || state.game != kRunawayFullGame || state.scene != _entryScene) {
		return false;
	}
	_now = now;
	return true;
}

void Scene9030::captureState(RunawaySaveState &state) const {
	state.scene = _entryScene;
	state.x = state.y = state.camera = state.cameraY = 0;
}

void Scene9030::say(byte variation) {
	_cue = _text.stageCue(10, variation);
	_line = 0;
	nextLine();
}

void Scene9030::nextLine() {
	_speech.stop();
	if (!_cue.valid() || _line >= MAX<uint>(1, _cue.continuationCount)) {
		return;
	}
	const Common::U32String text(_text.largeTextRecord(_cue.textRecordId + _line), Common::kDos850);
	const uint16 sample = _cue.voiceSampleId ? _cue.voiceSampleId + _line : 0;
	++_line;
	_speech.start(text, sample, true, _now, 0);
}

void Scene9030::enter(Step step) {
	_step = step;
	_started = _now;
	switch (step) {
	case kExplanation: // Brian explains his last-minute decision.
		say(0);
		break;
	case kCloseupExit: // Pull back from Brian's address to the player.
		_frame = 6;
		break;
	case kFadeOut: // Fade the close-up before revealing the beach.
		_frame = 72;
		break;
	case kBeachFadeIn: // Reveal Gina and Brian on the Cayman Islands.
		_cameraY = _background.h - kRunawayViewportHeight;
		_frame = 0;
		_beachStarted = _ginaTick = _ambientTick = _now;
		break;
	case kExclamation: // Finish Brian's explanation over the wide shot.
		_audio.playMusic(23, false);
		_audio.setMusicVolume(255);
		_audio.playMusicEffect(37, 102, 2, true);
		say(1);
		break;
	case kPause: // Hold the beach view before Gina calls out.
		break;
	case kGinaRequest: // Brian turns toward Gina as she asks for his help.
		_frame = 19;
		say(_request ? 5 : 2);
		break;
	case kReply: // Answer Gina while facing her.
		_frame = 30;
		say(_request ? 6 : 3);
		break;
	case kTurnBack: // Turn back to finish addressing the player.
		_frame = 49;
		break;
	case kFarewell: // Brian's farewell is interrupted once more by Gina.
		_frame = 0;
		say(4);
		break;
	case kLastWords: // Say goodbye while the camera pans up from the beach.
		_frame = 0;
		say(7);
		break;
	case kMusicTail: // Leave the final view visible until the music ends.
		_audio.stopEffect(2);
		break;
	case kFinished: // End the game after the epilogue.
		_speech.stop();
		_audio.stopEffect();
		_audio.stopMusic();
		break;
	}
}

void Scene9030::updateBeach() {
	while (_now - _ginaTick >= 80) {
		_ginaTick += 80;
		if (_ginaStep < 27 || (_ginaStep >= 30 && _ginaStep < 79)) {
			++_ginaStep;
		} else if (_ginaStep == 27 && !_random.getRandomNumber(2)) {
			_ginaStep = 30;
		} else {
			_ginaStep = 0;
		}
	}
	while (_now - _ambientTick >= 250) {
		_ambientTick += 250;
		if (_cameraY > 100 && !_audio.effectPlaying(1) && !_random.getRandomNumber(9)) {
			_audio.playEffect(70 + _random.getRandomNumber(2), 153, 1);
		}
	}
}

void Scene9030::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	if (speaking() && _speech.finished(now)) {
		nextLine();
	}
	if (_step >= kExclamation && _step < kFinished) {
		updateBeach();
	}
	const uint elapsed = now - _started;
	switch (_step) {
	case kExplanation:
		if (!speaking()) {
			enter(kCloseupExit);
		}
		break;
	case kCloseupExit:
		_frame = 6 + MIN<uint>(66, elapsed / 60);
		if (_frame == 72) {
			enter(kFadeOut);
		}
		break;
	case kFadeOut:
		if (elapsed >= 250) {
			enter(kBeachFadeIn);
		}
		break;
	case kBeachFadeIn:
		if (elapsed >= 250) {
			enter(kExclamation);
		}
		break;
	case kExclamation:
		if (!speaking()) {
			enter(kPause);
		}
		break;
	case kPause:
		if (elapsed >= 2000) {
			enter(kGinaRequest);
		}
		break;
	case kGinaRequest:
		_frame = 19 + MIN<uint>(11, elapsed / 60);
		if (!speaking() && _frame == 30) {
			enter(kReply);
		}
		break;
	case kReply:
		if (!speaking()) {
			enter(kTurnBack);
		}
		break;
	case kTurnBack:
		_frame = 49 + MIN<uint>(11, elapsed / 60);
		if (_frame == 60) {
			enter(_request ? kLastWords : kFarewell);
		}
		break;
	case kFarewell:
		if (!speaking()) {
			_request = 1;
			enter(kGinaRequest);
		}
		break;
	case kLastWords:
		_cameraY = _background.h - kRunawayViewportHeight - MIN<uint>(_background.h - kRunawayViewportHeight, elapsed / 10);
		if (!_cameraY && !speaking()) {
			enter(kMusicTail);
		}
		break;
	case kMusicTail:
		if (!_audio.musicPlaying() || elapsed >= 60000) {
			enter(kFinished);
		}
		break;
	case kFinished:
		break;
	}
}

void Scene9030::skipSpeech() {
	if (speaking()) {
		nextLine();
	}
}

void Scene9030::skipVideo() {
	enter(kFinished);
}

void Scene9030::draw() {
	_screen.clear();
	Graphics::ManagedSurface viewport(_screen, Common::Rect(0, kRunawayViewportTop, kRunawayScreenWidth,
		kRunawayViewportTop + kRunawayViewportHeight));
	viewport.blitFrom(_background, Common::Rect(0, _cameraY, 1024, _cameraY + 600), Common::Point(0, 0));
	if (_step <= kFadeOut) {
		_layers[0].draw(viewport, _step == kExplanation ? _speech.frame(_now) % 6 : _frame, 0, 0);
	} else {
		// Native 0x57fc50 draws the eight wave layers behind Gina and Brian.
		for (uint i = 0; i < ARRAYSIZE(kWaveLayers); ++i) {
			const RunawayAnimation &animation = _layers[kWaveLayers[i]];
			const uint period = (animation.frameCount() - 1) * 2;
			const uint frame = ((_now - _beachStarted) / 160) % period;
			animation.draw(viewport, MIN(frame, period - frame), 0, -(int)_cameraY);
		}
		_layers[2].draw(viewport, _ginaStep >= 30 ? _ginaStep - 2 : _ginaStep, 0, -(int)_cameraY);
		uint brian = _frame;
		if (_step == kReply) {
			brian = 30 + _speech.frame(_now);
		} else if (_step == kExclamation || _step == kFarewell || _step == kLastWords) {
			brian = _speech.frame(_now);
		}
		_layers[1].draw(viewport, kBrianFrames[brian], 0, -(int)_cameraY);
	}
	if (_step == kFadeOut || _step == kBeachFadeIn) {
		// The original flashes to white between the close-up and the beach.
		const uint elapsed = MIN<uint>(250, _now - _started);
		const uint opacity = (_step == kFadeOut ? elapsed : 250 - elapsed) * 255 / 250;
		Graphics::ManagedSurface white(1, 1, viewport.format);
		white.clear(viewport.format.RGBToColor(255, 255, 255));
		viewport.transBlitFrom(white, Common::Rect(1, 1), Common::Rect(viewport.w, viewport.h), 0, false, opacity);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
