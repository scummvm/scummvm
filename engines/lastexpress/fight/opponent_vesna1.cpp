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

COpponent2::COpponent2(LastExpressEngine *engine, CFight *fight) : COpponent(engine, fight) {
	_numSeqs = 7;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2002or.seq",   15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2002oal.seq",  15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2002oam.seq",  15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2002oar.seq",  15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2002okr.seq",  15, 0);
	_seqs[5] = _engine->getArchiveManager()->loadSeq("2002okml.seq", 15, 0);
	_seqs[6] = _engine->getArchiveManager()->loadSeq("2002okm.seq",  15, 0);

	_engine->getLogicManager()->playDialog(kCharacterTableA, engine->isDemo() ? "MUS030D" : "MUS030", 16, 0);

	_timer = 30;
}

void COpponent2::timer() {
	if (!_timer && actionAvailable(1) && !_nextSequenceIdx) {
		switch (rnd(6)) {
		case 0:
			doAction(1, 0);
			break;
		case 1:
			doAction(2, 0);
			break;
		case 2:
			doAction(3, 0);
			break;
		case 3:
			doAction(3, 0);
			doAction(1, 2);
			break;
		case 4:
			doAction(1, 0);
			doAction(2, 2);
			break;
		case 5:
			doAction(3, 0);
			doAction(2, 2);
			break;
		default:
			break;
		}

		_timer = rnd(15);
	}

	if (_currentSprite && (_currentSprite->flags & 2) != 0) {
		if (_currentActionIdx == 1)
			_opponent->send(1);

		if (_currentActionIdx == 2)
			_opponent->send(2);

		if (_currentActionIdx == 3)
			_opponent->send(3);

		if (_opponent->isDead()) {
			_engine->getLogicManager()->endDialog(kCharacterTableA);
			send(105);
		}
	}

	COpponent::timer();
}

} // End of namespace LastExpress
