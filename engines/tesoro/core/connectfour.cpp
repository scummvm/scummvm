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

#include "tesoro/core/connectfour.h"
#include "tesoro/world.h"

namespace Tesoro {

void ConnectFour::clear() {
	for (int i = 0; i < kSize; ++i)
		_mem[i] = 0;
	for (int i = 0; i < 16; ++i)
		_line[i] = 0;
}

int8 ConnectFour::at(int offset) const {
	const int i = kPad + offset;
	return i >= 0 && i < kSize ? (int8)_mem[i] : 0;
}

int ConnectFour::score(int column, int who) {
	// The four ways a line can run: up, up-right, right, down-right.
	static const int kDirections[4][2] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 } };
	int best = 0;
	for (int d = 0; d < 4; ++d) {
		const int dc = kDirections[d][0], dr = kDirections[d][1];
		int c = column;
		int r = at(1 + column * 8) - 1;
		// Back along the line from the last piece while the pieces are the same side's.
		int rr = r - dr;
		if (rr < at(1 + (c - dc) * 8)) {
			do {
				if (rr < 0 || at(2 + (c - dc) * 8 + rr) != who)
					break;
				c -= dc;
				r = rr;
				rr -= dr;
			} while (rr < at(1 + (c - dc) * 8));
		}
		// Then forward, counting and noting the cells.
		int length = 0;
		if (r < at(1 + c * 8)) {
			do {
				if (r < 0 || at(2 + c * 8 + r) != who)
					break;
				_line[length * 2] = c;
				_line[length * 2 + 1] = r;
				c += dc;
				r += dr;
				++length;
			} while (r < at(1 + c * 8));
		}
		if (best < length) {
			best = length;
			if (length > 3)
				return 4;
		}
	}
	return best;
}

int ConnectFour::bestMove(World &host, int &bestScore, int who) {
	int candidates[kColumns];
	int n = 0;
	bestScore = 0;
	for (int col = 0; col < kColumns; ++col) {
		if (count(col) == kHeight)
			continue;
		dropPiece(col, who);
		const int s = score(col, who);
		--_mem[kPad + 1 + col * 8];
		if (s == bestScore) {
			candidates[n++] = col;
		} else if (bestScore < s) {
			bestScore = s;
			n = 1;
			candidates[0] = col;
		}
	}
	if (n == 0)
		return -1;

	// A candidate that lets the other side win at once is left out.
	for (int k = 0; k < n; ++k) {
		const int col = candidates[k];
		dropPiece(col, 2);
		int reply = 0;
		for (; reply < kColumns; ++reply) {
			if (count(reply) == kHeight)
				continue;
			dropPiece(reply, 1);
			const int s = score(reply, 1);
			--_mem[kPad + 1 + reply * 8];
			if (s == 4)
				break;
		}
		--_mem[kPad + 1 + col * 8];
		if (reply == kColumns)
			return col;
	}
	return candidates[host.randomNumber(0, n - 1)];
}

ConnectFour::Outcome ConnectFour::choose(World &host, int lastColumn) {
	if (score(lastColumn, 1) == 4)
		return kPlayerWon;
	for (int col = 0; col < kColumns; ++col) {
		if (count(col) == kHeight)
			continue;
		dropPiece(col, 2);
		if (score(col, 2) == 4)
			return kMachineWon;
		--_mem[kPad + 1 + col * 8];
	}
	int playerScore = 0, machineScore = 0;
	const int blocking = bestMove(host, playerScore, 1);
	if (blocking < 0)
		return kDraw;
	const int attacking = bestMove(host, machineScore, 2);
	dropPiece(machineScore <= playerScore ? blocking : attacking, 2);
	return kMoved;
}

} // End of namespace Tesoro
