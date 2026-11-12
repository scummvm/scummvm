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

#include "tesoro/core/animset.h"

#include "common/textconsole.h"

namespace Tesoro {

void AnimSet::init(const Common::Array<AnimPlayer> &levelPlayers, const byte *indices, uint count) {
	_players.clear();
	for (uint i = 0; i < count; ++i) {
		if (indices[i] >= levelPlayers.size()) {
			warning("AnimSet: bank %u does not exist", indices[i]);
			_players.push_back(AnimPlayer());
		} else {
			_players.push_back(levelPlayers[indices[i]]);
		}
	}

	_bankIndex = 0;
	_selected = 0;
	_stashed = false;
}

void AnimSet::selectBank(byte bank, uint16 first, int last, bool loop, bool sumDurations, int32 now) {
	if (bank >= _players.size()) {
		warning("AnimSet: no bank %u to select", bank);
		return;
	}

	_bankIndex = bank;
	_selected = bank;
	_players[bank].setRange(first, last, loop, sumDurations, now);
	_stashed = false;
}

void AnimSet::switchTo(uint bank) {
	if (bank >= _players.size()) {
		warning("AnimSet: no bank %u to switch to", bank);
		return;
	}
	_bankIndex = bank;
	_selected = (byte)bank;
}

void AnimSet::switchPlayingTo(uint bank) {
	if (bank >= _players.size()) {
		warning("AnimSet: no bank %u to switch to", bank);
		return;
	}
	_bankIndex = bank;
}

void AnimSet::tick(int32 now) {
	current().advance(now);
}

void AnimSet::tickClearMovement(int32 now) {
	current().advance(now);
	current().clearMovement();
}

} // End of namespace Tesoro
