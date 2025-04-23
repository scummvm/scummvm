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

#include "lastexpress/fight/fighter.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

COpponent1::COpponent1(LastExpressEngine *engine, CFight *fight) : COpponent(engine, fight) {
	_numSeqs = 7;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2001or.seq",  15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2001oal.seq", 15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2001oam.seq", 15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2001okl.seq", 15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2001okm.seq", 15, 0);
	_seqs[5] = _engine->getArchiveManager()->loadSeq("2001dbk.seq", 15, 0);
	_seqs[6] = _engine->getArchiveManager()->loadSeq("2001wbk.seq", 15, 0);

	_engine->getLogicManager()->playDialog(kCharacterTableA, "MUS027", 16, 0);

	_timer = 35;
}

void COpponent1::timer() {
	if (!_timer && actionAvailable(1) && !_nextSequenceIdx) {
		if (_opponent->getDodges() >= 2) {
			switch (rnd(5)) {
			case 0:
				doAction(1, 0);
				break;
			case 1:
				doAction(2, 0);
				break;
			case 2:
				doAction(2, 0);
				doAction(1, 2);
				break;
			case 3:
				doAction(1, 0);
				doAction(2, 2);
				break;
			case 4:
				doAction(1, 0);
				doAction(1, 2);
				break;
			default:
				break;
			}
		} else {
			doAction(2, 0);
		}

		if (_opponent->getDodges() < 5) {
			_timer = 6 * (5 - _opponent->getDodges());
		} else {
			_timer = 0;
		}
	}

	if (_currentSprite && (_currentSprite->flags & 2) != 0) {
		if (_currentActionIdx == 1)
			_opponent->send(1);

		if (_currentActionIdx == 2)
			_opponent->send(2);

		if (_opponent->isDead()) {
			_engine->getLogicManager()->endDialog(kCharacterTableA);
			send(105);
		}
	}

	COpponent::timer();
}

void COpponent1::send(int message) {
	if (message == 4) {
		doAction(5, 1);
		_opponent->send(103);
	} else if (message != 131) {
		CFighter::send(message);
	}
}

} // End of namespace LastExpress
