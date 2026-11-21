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
#include "hollywood/runaway/scene9020.h"

namespace Hollywood {
namespace {

typedef RunawaySequence S;

const S::Command kBankSequence[] = {
	{S::kOpen, 1}, {S::kMusic, 28, 255}, {S::kEffect, 58, 255},
	{S::kPlay, 0, 2399}, {S::kWaitEffect, 5000}, {S::kEnd}
};

const S::Command kGangstersSequence[] = {
	{S::kOpen, 2}, {S::kMusic, 34, 255, 1}, {S::kPlay, 0, 67},
	{S::kTalk, 1, 0, 1}, {S::kTalk, 1, 1, 5}, {S::kTalk, 1, 2, 1},
	{S::kTalk, 1, 3, 5}, {S::kTalk, 1, 4, 1}, {S::kTalk, 1, 5, 7},
	{S::kTalk, 1, 6, 3}, {S::kTalk, 1, 7, 1},
	{S::kEffect, 59, 255}, {S::kPlay, 115, 218}, {S::kTalk, 1, 8, 9},
	{S::kEffect, 60, 255}, {S::kPlay, 230, 352}, {S::kDelay, 2000}, {S::kTalk, 1, 9},
	{S::kPlay, 353, 385}, {S::kTalk, 1, 10, 11},
	{S::kEffect, 61, 255}, {S::kPlay, 398, 445},
	{S::kSpeech, 1, 11, 11}, {S::kPlay, 446, 572},
	{S::kSpeech, 1, 12, 12}, {S::kPlay, 573, 747}, {S::kWaitSpeech},
	{S::kTalk, 1, 13, 13}, {S::kPlay, 760, 780}, {S::kTalk, 1, 14, 15},
	{S::kPlay, 793, 829}, {S::kEnd}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene9020 = {
	9020, 1024, 600, nullptr, 0, {{100, 100, 100}}, Common::Point(0, 0),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene9020::Scene9020(RunawayAudio &audio, RunawayInventory &, const RunawayStrings &, RunawayGameType) :
	_sequence(audio), _narrator(audio) {}

bool Scene9020::load(uint16 entryScene) {
	_entryScene = entryScene;
	_screen.create(kRunawayScreenWidth, kRunawayScreenHeight, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	return _text.loadStage("RESOURCE.003", "Runaway ending", 802) && _narrator.loadEndingNarration(7);
}

void Scene9020::start(uint32 now) {
	_now = now;
	resume();
}

void Scene9020::resume() {
	_step = kBankNarration;
	_narrator.start(_now);
}

bool Scene9020::restoreState(const RunawaySaveState &state, uint32 now, bool entering) {
	if (!entering || !state.valid() || state.game != kRunawayFullGame || state.scene != _entryScene) {
		return false;
	}
	_now = now;
	return true;
}

void Scene9020::captureState(RunawaySaveState &state) const {
	state.scene = _entryScene;
	state.x = state.y = state.camera = state.cameraY = 0;
}

bool Scene9020::narrating() const {
	return _step == kBankNarration || _step == kGangstersNarration || _step == kDepartureNarration;
}

const Common::U32String &Scene9020::subtitle() const {
	return narrating() ? _narrator.subtitle() : _sequence.subtitle();
}

void Scene9020::skipVideo() {
	_sequence.skip();
	_sequence.update(_now);
	_narrator.skip(_now);
	_step = kFinished;
	_transition.scene = 9010;
}

void Scene9020::skipSpeech() {
	if (narrating()) {
		_narrator.skipSpeech(_now);
	} else {
		_sequence.skipSpeech();
	}
}

void Scene9020::draw() {
	if (narrating()) {
		_narrator.draw(_screen);
	} else {
		_sequence.draw(_screen);
	}
}

void Scene9020::startNarration(byte cue) {
	if (!_narrator.loadEndingNarration(cue)) {
		error("Cannot load Runaway ending narration %u", cue);
	}
	_narrator.start(_now);
}

void Scene9020::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	_now = now;
	if (narrating()) {
		_narrator.update(now);
		if (_narrator.failed()) {
			error("Cannot play Runaway ending narration");
		}
		if (!_narrator.finished()) {
			return;
		}
	} else {
		_sequence.update(now);
		if (_sequence.active()) {
			return;
		}
	}
	// Native 0x57fa40 interleaves these five sequences before Berkeley (9010).
	switch (_step) {
	case kBankNarration: // Brian introduces the trip to the bank.
		_step = kBankFilm;
		_sequence.start(kBankSequence, _text, now, 'I');
		break;
	case kBankFilm: // Explain how the recordings implicated the gangsters.
		_step = kGangstersNarration;
		startNarration(8);
		break;
	case kGangstersNarration: // The Sandrettis confront Gustav and Feodor.
		_step = kGangstersFilm;
		startGangstersFilm();
		break;
	case kGangstersFilm: // Brian concludes the story before leaving with Gina.
		_step = kDepartureNarration;
		startNarration(9);
		break;
	case kDepartureNarration: // Continue to Brian and Gina's farewell at Berkeley.
		_transition.scene = 9010;
		_step = kFinished;
		break;
	case kFinished: // The engine consumes the pending transition.
		break;
	}
}

void Scene9020::startGangstersFilm() {
	if (!Common::File::exists("DATAVC00.000")) {
		_sequence.start(kGangstersSequence, _text, _now, 'I');
		return;
	}
	// GOG removes the talking frames from I02.001; I02.000 holds the faces.
	// These boundaries and callback frames come from its native 0x52d6a0.
	static const struct {
		uint16 cdFirst, cdLast, gogFirst, gogLast;
	} ranges[] = {
		{0, 67, 0, 67}, {115, 218, 68, 170}, {230, 352, 171, 292},
		{353, 385, 293, 325}, {398, 445, 326, 372}, {446, 572, 373, 499},
		{573, 747, 500, 675}, {760, 780, 676, 696}, {793, 829, 697, 733}
	};
	_commands.clear();
	for (uint i = 0; i < ARRAYSIZE(kGangstersSequence); ++i) {
		S::Command command = kGangstersSequence[i];
		if (command.operation == S::kPlay) {
			for (uint j = 0; j < ARRAYSIZE(ranges); ++j) {
				if (command.a == ranges[j].cdFirst && command.b == ranges[j].cdLast) {
					command.a = ranges[j].gogFirst;
					command.b = ranges[j].gogLast;
					break;
				}
			}
		}
		_commands.push_back(command);
	}
	_sequence.start(_commands.data(), _text, _now, 'I');
}

} // End of namespace Runaway
} // End of namespace Hollywood
