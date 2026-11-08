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

#ifndef TESORO_CORE_ANIMSET_H
#define TESORO_CORE_ANIMSET_H

#include "common/array.h"
#include "tesoro/core/animbank.h"

namespace Tesoro {

/**
 * Animations of one object:
 * holds a player for each bank listed in the object's record, one of them being the current player.
 * */
class AnimSet {
public:
	/**
	 * Makes a player for each entry of indices, a copy of levelPlayers[index].
	 * The first becomes current.
	 * */
	void init(const Common::Array<AnimPlayer> &levelPlayers, const byte *indices, uint count);

	uint bankCount() const { return _players.size(); }
	/** The bank that is playing. */
	uint bankIndex() const { return _bankIndex; }
	/** The bank last chosen with selectBank or switchTo */
	byte selectedBank() const { return _selected; }
	void setSelectedBank(byte bank) { _selected = bank; }

	AnimPlayer &current() { return _players[_bankIndex]; }

	/** Makes a bank current and starts playing the steps first..last. */
	void selectBank(byte bank, uint16 first, int last, bool loop, bool sumDurations, int32 now);

	void tick(int32 now);

	/** Advances the current player and drops the movement it computed. */
	void tickClearMovement(int32 now);

private:
	Common::Array<AnimPlayer> _players;
	uint _bankIndex = 0;
	byte _selected = 0;
	bool _stashed = false;
	uint32 _stashedBank = 0;
	uint32 _stashedFlags = 0;
};

} // End of namespace Tesoro

#endif
