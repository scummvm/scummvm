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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_PUZZLES_KK_TILE_MATCH_MODEL_H
#define RIPPER_PUZZLES_KK_TILE_MATCH_MODEL_H

#include "common/scummsys.h"

namespace Ripper {

class KkTileMatchModel {
public:
	static const uint kTileCount = 16;

	enum MatchResult { kPending, kMatch, kMismatch };

	KkTileMatchModel();

	uint tileState(uint tile) const { return _states[tile]; }
	void setTileState(uint tile, byte state) { _states[tile] = state; }
	bool isActive(uint tile) const { return (_activeMask & (1U << tile)) != 0; }
	uint16 activeMask() const { return _activeMask; }
	void reveal(uint tile);
	void hide(uint tile);
	bool applyMove(const int *sources);
	uint activeTileCount() const;
	uint matchingActiveTileCount() const;
	MatchResult evaluate() const;
	MatchResult applyClick(uint tile, const int *sources);

private:
	byte _states[kTileCount];
	uint16 _activeMask;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KK_TILE_MATCH_MODEL_H
