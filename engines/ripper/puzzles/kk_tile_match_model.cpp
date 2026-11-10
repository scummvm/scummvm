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

#include "ripper/puzzles/kk_tile_match_model.h"

namespace Ripper {

KkTileMatchModel::KkTileMatchModel() : _activeMask(0) {
	for (uint tile = 0; tile < kTileCount; ++tile)
		_states[tile] = 0;
}

void KkTileMatchModel::reveal(uint tile) {
	_activeMask |= 1U << tile;
}

void KkTileMatchModel::hide(uint tile) {
	_activeMask &= ~(1U << tile);
}

bool KkTileMatchModel::applyMove(const int *sources) {
	const KkTileMatchModel snapshot = *this;
	bool moved = false;
	for (uint destination = 0; destination < kTileCount; ++destination) {
		const int source = sources[destination];
		if (source < 0)
			continue;
		_states[destination] = snapshot._states[source];
		hide(destination);
		if (snapshot.isActive(source))
			reveal(destination);
		moved = true;
	}
	return moved;
}

uint KkTileMatchModel::activeTileCount() const {
	uint count = 0;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (isActive(tile))
			++count;
	}
	return count;
}

uint KkTileMatchModel::matchingActiveTileCount() const {
	uint count = 0;
	byte referenceState = 0;
	bool haveReference = false;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (!isActive(tile))
			continue;
		if (!haveReference) {
			referenceState = _states[tile];
			haveReference = true;
		}
		if (_states[tile] == referenceState)
			++count;
	}
	return count;
}

KkTileMatchModel::MatchResult KkTileMatchModel::evaluate() const {
	if (activeTileCount() < 3)
		return kPending;
	return matchingActiveTileCount() == 3 ? kMatch : kMismatch;
}

KkTileMatchModel::MatchResult KkTileMatchModel::applyClick(uint tile, const int *sources) {
	applyMove(sources);
	reveal(tile);
	const MatchResult result = evaluate();
	if (result == kMismatch) {
		for (uint activeTile = 0; activeTile < kTileCount; ++activeTile)
			hide(activeTile);
	}
	return result;
}

} // End of namespace Ripper
