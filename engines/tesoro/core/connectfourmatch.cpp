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
#include "tesoro/core/connectfourmatch.h"
#include "tesoro/core/namekey.h"
#include "tesoro/world.h"

namespace Tesoro {

enum { kResultVariable = 2, kNoResult = 0xff, kPlayerWins = 2 };

void ConnectFourMatch::start(World &host, byte *variables) {
	// Only the counts are cleared: the cells above a count are never read.
	for (int column = 0; column < ConnectFour::kColumns; ++column)
		board.raw()[1 + column * 8] = 0;
	host.showPieces();
	busy = false;
	state = kPlaying;
	moveStamp = 0;

	_savedResult = variables[kResultVariable];
	variables[kResultVariable] = kNoResult;
}

void ConnectFourMatch::finish(World &host, byte result) {
	state = result;
	busy = true;
	moveStamp = host.now();
	blinkStamp = host.now();
}

void ConnectFourMatch::turn(World &host, byte *variables) {
	host.setPointerActive(true);

	if (state != kPlaying) {
		// The game is over: the winning pieces blink for 3 seconds, then the script learns the result.
		if ((uint32)(host.now() - moveStamp) > 3000) {
			variables[kResultVariable] = _savedResult;
			if (state == kPlayerWon)
				variables[kResultVariable] = kPlayerWins;
			host.setPointerActive(false);
		}
		host.showPieces();
		return;
	}

	if (!busy)
		return;

	if (moveStamp != 0) {
		if ((uint32)(host.now() - moveStamp) > 1500)
			moveStamp = 0;
		return;
	}

	busy = false;
	switch (board.choose(host, (int8)board.raw()[0])) {
	case ConnectFour::kPlayerWon:
		finish(host, kPlayerWon);
		break;
	case ConnectFour::kMachineWon:
	case ConnectFour::kDraw:
		finish(host, kMachineWon);
		break;
	default:
		break;
	}
	host.showPieces();
}

void ConnectFourMatch::showOn(ObjectList &objects, int32 now) {
	if (state != kPlaying && now - blinkStamp > 200) {
		blink = blink == 0 ? 1 : 0;
		blinkStamp = now;
	}

	// The first piece is the first object called FICHA; the rest follow it in the list.
	const uint64 key = nameKey64("FICHA");
	uint next = 0;
	while (next < objects.size() && objects[next]->nameKey() != key)
		++next;

	const int32 *winning = board.line();
	for (int column = 0; column < ConnectFour::kColumns; ++column) {
		for (int row = 0; row < ConnectFour::kHeight; ++row) {
			Object *piece = next < objects.size() ? objects[next] : nullptr;
			if (piece) {
				bool hide = false;
				if (state != kPlaying) {
					for (int i = 0; i < 4; ++i) {
						if (winning[i * 2] == column && winning[i * 2 + 1] == row && blink == 0) {
							hide = true;
							break;
						}
					}
				}

				if (hide || row >= board.count(column)) {
					piece->setVisible(false);
				} else {
					piece->setVisible(true);
					piece->animation().selectBank((byte)(board.cell(column, row) - 1), 0, 0, false, false, now);
				}
			}
			++next;
		}
	}
}

} // End of namespace Tesoro
