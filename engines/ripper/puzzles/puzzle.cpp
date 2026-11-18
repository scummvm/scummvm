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

#include "ripper/puzzles/puzzle.h"

#include "common/system.h"

#include "ripper/input.h"
#include "ripper/media/media.h"
#include "ripper/milestones.h"
#include "ripper/ripper.h"

namespace Ripper {

static const uint kDosTickMillis = 55;

Puzzle::Puzzle(RipperEngine *engine) : Scene(engine) {
}

bool Puzzle::serviceEngineEvents() {
	return !_engine->getInput()->pollEvents();
}

bool Puzzle::waitTicks(uint ticks) {
	const uint32 target = g_system->getMillis() + ticks * kDosTickMillis;
	while (!_engine->shouldQuit() &&
			(int32)(target - g_system->getMillis()) > 0) {
		if (!serviceEngineEvents())
			return false;
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

bool Puzzle::markSolved(uint completionFlag, const char *source) {
	return _engine->getMilestones()->set(completionFlag, true, source);
}

void Puzzle::stopAudioHandles(Audio::SoundHandle *handles, uint count) {
	for (uint i = 0; i < count; ++i)
		_engine->getMedia()->stopSoundEffect(handles[i]);
}

} // End of namespace Ripper
