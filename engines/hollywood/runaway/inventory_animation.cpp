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
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/inventory.h"

namespace Hollywood {

RunawayInventory::Reply RunawayInventory::examine(byte item, const RunawaySaveState &state) {
	assert(!handling());
	const Reply reply = examinationReply(item, state);
	return _fullGame && prepareExamination(reply) ? Reply(0) : reply;
}

bool RunawayInventory::prepareExamination(const Reply &reply) {
	if (reply.cue == 68 && reply.variation != 1) {
		if (reply.variation == 2)
			_handlingSteps.push_back(HandlingStep(kHandlingSpeech, 68, 2));
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, 84, 92));
		_handlingSteps.push_back(HandlingStep(kHandlingEffect, 35, reply.variation ? 1500 : 750, 70));
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, 93, 102, 1));
		_handlingSteps.push_back(HandlingStep(kHandlingSpeech, 68, reply.variation ? 3 : 0));
		return true;
	}
	if (reply.cue == 209 || reply.cue == 252 || reply.cue == 301) {
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, 49, 64));
		_handlingSteps.push_back(HandlingStep(kHandlingCloseup, reply.cue));
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, 65, 81, 1));
		return true;
	}
	if (reply.cue == 306) {
		_handlingSteps.push_back(HandlingStep(kHandlingSpeech, 306, 0));
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, 49, 64));
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, 65, 81, 1));
		_handlingSteps.push_back(HandlingStep(kHandlingSpeech, 306, 1));
		return true;
	}
	return false;
}

void RunawayInventory::collectPods(uint16 stage) {
	assert(!handling());
	_handlingSteps.push_back(HandlingStep(kHandlingStageSpeech, 20, 2, stage));
	_handlingSteps.push_back(HandlingStep(kHandlingFrames, 84, 92));
	_handlingSteps.push_back(HandlingStep(kHandlingEffect, 38, 2000, 100, 1));
	_handlingSteps.push_back(HandlingStep(kHandlingAdd, 20));
	_handlingSteps.push_back(HandlingStep(kHandlingFrames, 93, 102, 1));
	_handlingSteps.push_back(HandlingStep(kHandlingStageSpeech, 20, 3, stage));
	_handlingSteps.push_back(HandlingStep(kHandlingFrames, 84, 92));
	_handlingSteps.push_back(HandlingStep(kHandlingEffect, 41, 2000, 100, 1));
	_handlingSteps.push_back(HandlingStep(kHandlingFrames, 93, 102, 1));
	_handlingSteps.push_back(HandlingStep(kHandlingStageSpeech, 20, 4, stage));
}

RunawayInventory::Reply RunawayInventory::combine(byte first, byte second, const RunawaySaveState::Town &townState) {
	assert(!handling());
	if (_museumDemo)
		return applyCombination(first, second, townState);

	// Resolve the outcome without changing the items displayed during the lead-in.
	RunawayInventory preview(*this);
	const Reply reply = preview.applyCombination(first, second, townState);
	if (!prepareHandling(reply))
		return applyCombination(first, second, townState);
	_handlingFirst = first;
	_handlingSecond = second;
	_handlingTown = townState;
	return Reply(0);
}

bool RunawayInventory::prepareHandling(const Reply &reply) {
	struct Action {
		uint16 cue;
		byte variation, firstFrame;
		uint16 effect;
		byte volume;
		uint16 wait;
		byte before, after; // 0xff means no speech on that side of the action.
	};
	// Native inventory handlers use the portrait cycles at 0x411770-0x411970.
	static const Action actions[] = {
		{ 69, 0, 49, 36,  70, 1500, 0xff,    0}, // Fill the syringe.
		{ 71, 0, 49,  0, 100,    0, 0xff,    0}, // Put the wig on the head.
		{ 72, 0, 49,  0, 100,    0, 0xff,    0}, // Discover that the marker is dry.
		{ 72, 2, 84, 34,  70, 1500, 0xff,    2}, // Write the chart.
		{ 73, 1, 49, 37,  70, 1500, 0xff,    1}, // Restore the marker.
		{115, 1, 84, 60,  90, 1500, 0xff,    1}, // Insert the recorder's battery.
		{116, 0, 49, 40,  70,    0,    0,    1}, // Put the battery in the ladle.
		{117, 0, 84, 56,  90, 1500,    0,    1}, // Pry out the ruby.
		{121, 1, 84, 45, 100, 1500,    1,    2}, // Fill the coffee packet.
		{156, 0, 49, 26,  65, 1500, 0xff,    0}, // Wet the cloth.
		{178, 0, 84, 68,  70, 1500,    0, 0xff}, // Butter the helmet.
		{183, 0, 49,  0, 100,    0,    0,    1}, // Tie the rope to the coat hanger.
		{184, 0, 84,  0, 100,    0,    0,    1}, // Wet one lipstick.
		{185, 0, 84,  0, 100,    0,    0,    1}, // Wet the box of lipsticks.
		{186, 1, 84, 54, 100, 1500,    1,    2}, // Fit one lipstick in the belt.
		{187, 1, 84, 54, 100, 1500,    1,    2}, // Fill the belt with lipsticks.
		{188, 0, 49,  0, 100,    0,    0,    1}, // Patch the basketball.
		{189, 1, 84, 34, 100, 1500,    1,    2}, // Inflate the basketball.
		{190, 1, 84, 68,  70, 1500,    1,    2}, // Add sugar to the helmet.
		{224, 0, 84, 32, 100, 1500,    0,    4}, // Decorate the sculpture; finish with cue 219.
		{225, 1, 84, 65, 100, 1500,    1,    2}, // Prepare chewing tobacco.
		{227, 1, 84, 30, 100, 1500,    1,    2}, // Staple the strap.
		{228, 0, 84, 34, 100, 2000, 0xff,    0}, // Empty a measuring tube.
		{228, 2, 84, 34, 100, 2000,    1,    2}, // Fill the small tube.
		{228, 3, 84, 34, 100, 2000,    1,    3}, // Fill the large tube.
		{231, 2, 84, 19, 100, 2000,    2,    3}, // Fill the bottle with water.
		{234, 2, 84, 34, 100, 2000, 0xff,    2}, // Pour between the tubes.
		{236, 2, 84, 34, 100, 2000, 0xff,    2}, // Obtain forty cc.
		{237, 2, 84, 34, 100, 2000,    2,    3}, // Mix the fuel.
		{240, 1, 84, 29, 100, 2000,    1,    2}, // Sand the sculpture.
		{241, 1, 49,  0, 100,    0,    1,    2}, // Assemble the mortar.
		{261, 0, 84, 20, 100, 1500,    0,    1}, // Split the timber.
		{263, 0, 84, 14, 100, 1500,    0,    1}, // Fit the mining tool's handle.
		{313, 0, 84, 41, 100, 1500,    0,    1}, // Try the cold scalpel.
		{313, 2, 84, 41, 100, 1500,    2,    3}, // Repeat the unsuccessful attempt.
		{313, 4, 84, 40, 100, 1500,    4,    5}  // Cut open the pods.
	};
	if (reply.action || reply.stage)
		return false;
	for (uint i = 0; i < ARRAYSIZE(actions); ++i) {
		const Action &action = actions[i];
		if (reply.cue != action.cue || reply.variation != action.variation)
			continue;
		if (action.before != 0xff)
			_handlingSteps.push_back(HandlingStep(kHandlingSpeech, action.cue, action.before));
		const bool shortCycle = action.firstFrame == 84;
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, action.firstFrame, shortCycle ? 92 : 64));
		if (action.effect) {
			const byte channel = action.cue == 261 || action.cue == 263 ? 1 : 0;
			const uint repeats = action.cue == 187 ? 5 : 1;
			for (uint j = 0; j < repeats; ++j)
				_handlingSteps.push_back(HandlingStep(kHandlingEffect, action.effect, action.wait, action.volume, channel));
		}
		const bool unsuccessful = (action.cue == 72 && !action.variation) || (action.cue == 313 && action.variation < 4);
		if (!unsuccessful)
			_handlingSteps.push_back(HandlingStep(kHandlingCombine, action.cue == 234 || action.cue == 236 ? 2 : 1));
		_handlingSteps.push_back(HandlingStep(kHandlingFrames, shortCycle ? 93 : 65, shortCycle ? 102 : 81, 1));
		if (unsuccessful)
			_handlingSteps.push_back(HandlingStep(kHandlingCombine));
		if (action.cue == 313 && action.variation == 4)
			_handlingSteps.push_back(HandlingStep(kHandlingMusic, 37));
		if (action.after != 0xff)
			_handlingSteps.push_back(HandlingStep(kHandlingSpeech, action.cue == 224 ? 219 : action.cue, action.after));
		if (action.cue == 185 && reply.lastVariation == 2) {
			_handlingSteps.push_back(HandlingStep(kHandlingSpeech, 185, 2));
			_handlingSteps.push_back(HandlingStep(kHandlingFrames, 49, 64));
			_handlingSteps.push_back(HandlingStep(kHandlingFrames, 65, 81, 1));
			_handlingSteps.push_back(HandlingStep(kHandlingRemove, 10));
		}
		return true;
	}
	return false;
}

RunawayInventory::Reply RunawayInventory::updateHandling(uint32 now, RunawayAudio &audio, bool speaking) {
	while (_handlingStep < _handlingSteps.size()) {
		const HandlingStep &step = _handlingSteps[_handlingStep];
		const bool entered = !_handlingEntered;
		if (entered) {
			_handlingStarted = now;
			_handlingEntered = true;
		}
		const uint32 elapsed = now - _handlingStarted;
		switch (step.operation) {
		case kHandlingSpeech:
		case kHandlingStageSpeech:
			if (entered) {
				_handlingFrame = 0xff;
				Reply reply(step.a, step.b);
				if (step.operation == kHandlingStageSpeech)
					reply.stage = step.c;
				return reply;
			}
			if (speaking)
				return Reply(0);
			break;
		case kHandlingCloseup:
			if (entered) {
				Reply reply(step.a);
				reply.closeup = true;
				return reply;
			}
			// The scene resumes this sequence after the close-up has ended.
			break;
		case kHandlingFrames: {
			const uint ticks = elapsed / 60;
			const uint count = step.b - step.a + 1;
			if (entered)
				_handlingFrame = step.c ? step.a - 1 : 0;
			if (ticks)
				_handlingFrame = step.c && ticks > count ? 0 : step.a + MIN<uint>(ticks, count) - 1;
			if (ticks < count + step.c)
				return Reply(0);
			break;
		}
		case kHandlingEffect:
			if (entered)
				audio.playEffect(step.a, step.c, step.d);
			if (audio.effectPlaying(step.d) && elapsed < step.b)
				return Reply(0);
			break;
		case kHandlingCombine:
			applyCombination(_handlingFirst, _handlingSecond, _handlingTown);
			// Native 0x411320 updates the icons without the pickup portrait.
			_newItem = 0;
			for (uint i = 0; i < step.a; ++i)
				audio.playEffect(1, 100, audio.effectPlaying() ? 1 : 0);
			break;
		case kHandlingRemove:
		case kHandlingAdd:
			if (step.operation == kHandlingAdd)
				add(step.a, false);
			else
				remove(step.a);
			_selected = 0;
			audio.playEffect(1, 100, audio.effectPlaying() ? 1 : 0);
			break;
		case kHandlingMusic:
			audio.playMusic(step.a, false);
			break;
		}
		++_handlingStep;
		_handlingEntered = false;
	}
	clearHandling();
	return Reply(0);
}

void RunawayInventory::clearHandling() {
	_handlingSteps.clear();
	_handlingStep = 0;
	_handlingStarted = 0;
	_handlingEntered = false;
	_handlingFrame = 0xff;
	_handlingFirst = _handlingSecond = 0;
}

} // End of namespace Hollywood
