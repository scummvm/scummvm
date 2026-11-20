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

#ifndef HOLLYWOOD_RUNAWAY_OSCAR_H
#define HOLLYWOOD_RUNAWAY_OSCAR_H

#include "common/scummsys.h"

namespace Common {
class RandomSource;
}

namespace Hollywood {
class RunawayAudio;

namespace Runaway {

class OscarAnimation {
public:
	explicit OscarAnimation(bool closeup, byte chapter = 4) : _closeup(closeup), _chapter(chapter) {}
	void resume(uint32 now) { _tick = now; }
	void update(uint32 now, Common::RandomSource &random, RunawayAudio &audio);
	uint frame(bool speaking, byte speechFrame) const;
	void setTalking(bool talking) { _talking = talking; }
	void startIdle();

private:
	const bool _closeup;
	const byte _chapter;
	bool _talking = false;
	uint16 _step = 130;
	byte _idle = 3;
	uint32 _tick = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_OSCAR_H
