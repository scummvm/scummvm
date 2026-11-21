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

#include "common/util.h"
#include "hollywood/runaway/scene5140.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
};

const byte kBrianFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 52, 53, 54,
	55, 56, 57, 58, 59, 60, 61, 62, 51, 51, 51, 51, 51, 51, 51, 51,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 80, 80, 80, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
	80, 80, 80, 80, 80, 80, 80, 80, 93, 94, 95, 96, 97, 98, 99, 100,
	101, 101, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 101, 101,
	101, 101, 101, 101, 101, 101, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
	124, 125, 126, 127, 128, 80
};

enum Speaker { kBrianCarla, kBrianLula, kBrianMariola, kCarla, kLula, kMariola, kNobody };
const byte kNoLine = 0xff;
const byte kBrianTalkFrames[] = {130, 100, 60};

struct IntroductionLine {
	byte variation;
	Speaker speaker;
	byte firstFrame, lastFrame;
};

const IntroductionLine kIntroduction[] = {
	{0, kMariola, 0, 0}, // Greet Brian.
	{1, kBrianMariola, 0, 0},
	{2, kCarla, 120, 128}, // Turn toward Carla.
	{3, kBrianCarla, 0, 0},
	{4, kLula, 150, 165}, // Turn toward Lula.
	{5, kBrianLula, 0, 0},
	{kNoLine, kNobody, 95, 80}, // Turn back to Mariola.
	{6, kBrianMariola, 0, 0}, // Explain Gina's captivity.
	{7, kMariola, 0, 0},
	{8, kCarla, 120, 128},
	{9, kLula, 150, 165},
	{10, kMariola, 95, 80},
	{11, kCarla, 120, 128},
	{12, kBrianCarla, 0, 0}, // Ask for help.
	{13, kLula, 150, 165},
	{14, kBrianLula, 0, 0},
	{kNoLine, kNobody, 164, 150}, // Turn back to Carla.
	{15, kBrianCarla, 0, 0}, // Introductions.
	{16, kCarla, 0, 0},
	{17, kMariola, 127, 120},
	{18, kLula, 80, 96},
	{19, kBrianLula, 0, 0},
	{20, kMariola, 95, 80},
	{21, kBrianMariola, 0, 0},
	{22, kCarla, 120, 128},
	{23, kLula, 150, 165},
	{24, kMariola, 95, 80},
	{25, kBrianMariola, 0, 0},
	{kNoLine, kNobody, 80, 96},
	{26, kBrianLula, 0, 0}, // Think of a rescue plan.
	{kNoLine, kNobody, 164, 150},
	{27, kBrianCarla, 0, 0},
	{28, kLula, 150, 165},
	{29, kMariola, 95, 80},
	{30, kCarla, 120, 128},
};

const IntroductionLine kFarewell[] = {
	{kNoLine, kNobody, 120, 128}, // Brian finds that the others have left.
	{32, kBrianCarla, 0, 0},
	{33, kMariola, 0, 0}, // Offscreen reply.
	{34, kBrianCarla, 0, 0},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5140 = {
	5140, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {},
	Common::Point(512, 500),
	nullptr, 0,
	nullptr, 0,
	nullptr, 0
};

namespace Runaway {

Scene5140::Scene5140(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5140::loadCustomResources() {
	return _brianAnimation.load(_archive, 1, 130) &&
		_carlaAnimation.load(_archive, 2, 33) &&
		_lulaAnimation.load(_archive, 3, 51) &&
		_mariolaAnimation.load(_archive, 4, 32);
}

void Scene5140::runCustomEntrySequence() {
	_busIntroduction = BusIntroduction();
	_busIntroduction.tick = _now;
	_roomAction = 5140;
}

void Scene5140::updateIntroduction() {
	static const AnimationFootstep arrivalSteps[] = {
		{1, 800}, {1, 100}, {6, 512}, {11, 100}, {14, 800}, {15, 512}, {24, 512}
	};
	BusIntroduction &intro = _busIntroduction;
	if (intro.phase == BusIntroduction::kFilm) {
		intro.phase = BusIntroduction::kReturn;
		intro.frame = 0;
		intro.lastFrame = 51;
		intro.tick = _now;
		intro.nextLine = 0;
		intro.speaker = kNobody;
	}
	const bool arriving = intro.phase == BusIntroduction::kArrival || intro.phase == BusIntroduction::kReturn;
	if (_now - intro.tick >= 80) {
		intro.tick = _now;
		intro.blink = false;
		if (intro.frame != intro.lastFrame) {
			if (intro.frame < intro.lastFrame) {
				++intro.frame;
			} else {
				--intro.frame;
			}
			if (intro.phase == BusIntroduction::kArrival) {
				playAnimationFootsteps(intro.frame, 2, arrivalSteps, ARRAYSIZE(arrivalSteps));
			}
		} else if (intro.speaker >= kCarla && intro.facing != kBrianLula) {
			intro.blink = !_random.getRandomNumber(14);
		}
	}
	if (intro.frame != intro.lastFrame || speaking()) {
		return;
	}
	if (arriving) {
		intro.phase = intro.phase == BusIntroduction::kArrival ? BusIntroduction::kConversation : BusIntroduction::kFarewell;
		intro.frame = intro.lastFrame = kBrianTalkFrames[kBrianMariola];
	}
	const bool farewell = intro.phase == BusIntroduction::kFarewell;
	const IntroductionLine *lines = farewell ? kFarewell : kIntroduction;
	const uint count = farewell ? ARRAYSIZE(kFarewell) : ARRAYSIZE(kIntroduction);
	if (intro.nextLine == count) {
		if (farewell) {
			_roomAction = 0;
			leaveDesertRoom(5060);
			_transition = _hospitalExit;
		} else {
			// Brian pauses to think, then announces that he has a plan.
			typedef RunawaySequence S;
			static const S::Command film[] = {
				{S::kOpen, 6}, {S::kPlay, 0, 0}, {S::kEffect, 20, 255},
				{S::kPlay, 1, 186}, {S::kTalk, 1, 31, 1}, {S::kEnd}
			};
			intro.phase = BusIntroduction::kFilm;
			_sequence.start(film, _text, _now, 'E');
		}
		return;
	}
	const IntroductionLine &line = lines[intro.nextLine++];
	intro.speaker = line.speaker;
	if (line.firstFrame != line.lastFrame) {
		intro.frame = line.firstFrame;
		intro.lastFrame = line.lastFrame;
		intro.facing = line.lastFrame == 80 || line.lastFrame == 120 ? kBrianMariola :
			line.lastFrame == 96 || line.lastFrame == 165 ? kBrianLula : kBrianCarla;
	} else if (line.speaker <= kBrianMariola) {
		intro.facing = line.speaker;
		intro.frame = intro.lastFrame = kBrianTalkFrames[intro.facing];
	}
	intro.tick = _now;
	if (line.variation != kNoLine) {
		say(1, line.variation, false, line.speaker + 1);
	}
}

bool Scene5140::updateCustomSequence() {
	updateIntroduction();
	return true;
}

void Scene5140::finishCustomFilm() {
	updateIntroduction();
}

bool Scene5140::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	const BusIntroduction &intro = _busIntroduction;
	const bool arriving = intro.phase == BusIntroduction::kArrival || intro.phase == BusIntroduction::kReturn;
	uint frames[] = {kBrianFrames[intro.frame], 27, 44, 25};
	if (arriving) {
		frames[1] = MIN<uint>(intro.frame, 27);
		frames[2] = MIN<uint>(intro.frame, 44);
		frames[3] = MIN<uint>(intro.frame, 25);
	} else {
		const byte mouth = speaking() ? speechFrame() : 0;
		if (intro.frame == intro.lastFrame) {
			frames[0] = kBrianFrames[kBrianTalkFrames[intro.facing] +
				(intro.speaker <= kBrianMariola ? mouth : intro.blink ? 6 : 0)];
		}
		if (intro.speaker >= kCarla && intro.speaker <= kMariola) {
			frames[intro.speaker - kCarla + 1] += mouth % 6;
		}
	}
	const bool alone = intro.phase == BusIntroduction::kReturn || intro.phase == BusIntroduction::kFarewell;
	if (!alone) {
		_lulaAnimation.draw(viewport, frames[2], 0, 0);
	}
	_brianAnimation.draw(viewport, frames[0], 0, 0);
	if (!alone) {
		_carlaAnimation.draw(viewport, frames[1], 0, 0);
		_mariolaAnimation.draw(viewport, frames[3], 0, 0);
	}
	return true;
}

bool Scene5140::shouldUpdateActor() const {
	return false;
}

bool Scene5140::loadWalkMap() {
	const RunawayRoom *room = findRunawayRoom(_room);
	// This cutscene contains a background and sprite tracks, without a mask.
	_mask.resize(room->width * room->height, 0);
	_originalMaps.resize(1536, 0);
	return true;
}

} // End of namespace Runaway
} // End of namespace Hollywood
