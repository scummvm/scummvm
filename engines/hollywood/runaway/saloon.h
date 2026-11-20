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

#ifndef HOLLYWOOD_RUNAWAY_SALOON_H
#define HOLLYWOOD_RUNAWAY_SALOON_H

#include "hollywood/runaway/graphics.h"

namespace Hollywood {
class RunawayAudio;

namespace Runaway {

// The porch uses the same pressure-operated doors in chapters four and six.
class SaloonDoors {
public:
	bool load(ChunkArchive &archive);
	void resume(uint32 now, bool open);
	void update(uint32 now, bool pressed, RunawayAudio &audio, uint16 openingCue);
	void draw(Graphics::ManagedSurface &viewport, bool pressed) const;

private:
	RunawayAnimation _animation;
	Common::Array<byte> _patches[2];
	byte _frame = 0;
	uint32 _tick = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SALOON_H
