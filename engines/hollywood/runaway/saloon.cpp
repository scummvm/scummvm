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

#include "hollywood/resource.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/saloon.h"

namespace Hollywood {
namespace Runaway {

bool SaloonDoors::load(ChunkArchive &archive) {
	return _animation.load(archive, 8, 71) &&
		archive.readVariableChunk(6, _patches[0]) && archive.readVariableChunk(7, _patches[1]);
}

void SaloonDoors::resume(uint32 now, bool open) {
	_tick = now;
	_frame = open ? 7 : 0;
}

void SaloonDoors::update(uint32 now, bool pressed, RunawayAudio &audio, uint16 openingCue) {
	if (pressed && _frame > 7) {
		_frame = 0;
	}
	if ((pressed && _frame == 7) || (!pressed && !_frame)) {
		_tick = now;
		return;
	}
	if (now - _tick < 80) {
		return;
	}
	_tick = now;
	_frame = (_frame + 1) % 71;
	if (_frame == 1) {
		audio.playEffect(openingCue);
	} else if (_frame == 9) {
		audio.playEffect(openingCue + 1);
	}
}

void SaloonDoors::draw(Graphics::ManagedSurface &viewport, bool pressed) const {
	applyRunawayPatch(_patches[pressed], viewport);
	_animation.draw(viewport, _frame, 0, 0);
}

} // End of namespace Runaway
} // End of namespace Hollywood
