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

COpponent5::COpponent5(LastExpressEngine *engine, CFight *fight) : COpponent(engine, fight) {
	_numSeqs = 8;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2005or.seq",   15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2005oam.seq",  15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2005oar.seq",  15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2005okml.seq", 15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2005okr.seq",  15, 0);
	_seqs[5] = _engine->getArchiveManager()->loadSeq("2005odm1.seq", 15, 0);
	_seqs[6] = _engine->getArchiveManager()->loadSeq("2005csbm.seq", 15, 0);
	_seqs[7] = _engine->getArchiveManager()->loadSeq("2005oam4.seq", 15, 0);

	_engine->getLogicManager()->playDialog(kCharacterTableA, "MUS038", 16, 0);

	_timer = 30;
	_hitPoints = 4;
}

void COpponent5::timer() {
	if (!_timer && actionAvailable(1) && !_nextSequenceIdx) {
		if (_hitPoints == 1) {
			doAction(5, 0);
		} else {
			switch (rnd(6)) {
			case 0:
				doAction(1, 0);
				break;
			case 1:
				doAction(1, 0);
				doAction(1, 2);
				break;
			case 2:
				doAction(2, 0);
				break;
			case 3:
				doAction(2, 0);
				doAction(2, 2);
				break;
			case 4:
				doAction(1, 0);
				doAction(2, 2);
				break;
			case 5:
				doAction(2, 0);
				doAction(1, 2);
				break;
			default:
				break;
			}
		}
		_timer = 4 * _hitPoints;
	}

	if (!_currentSprite || (_currentSprite->flags & 2) == 0) {
		COpponent::timer();
		return;
	}

	if (_currentActionIdx == 1)
		_opponent->send(1);

	if (_currentActionIdx == 2)
		_opponent->send(2);

	if (_currentActionIdx == 5)
		_opponent->send(5);

	if (_opponent->isDead()) {
		switch (_currentActionIdx) {
		case 1:
			doAction(3, 1);
			break;
		case 2:
			doAction(4, 1);
			break;
		case 5:
			doAction(6, 1);
			break;
		}

		_opponent->doAction(4, 1);

		send(105);
		_opponent->timer();
		timer();

		_engine->getLogicManager()->endDialog(33);
	} else {
		COpponent::timer();
	}
}

void COpponent5::send(int action) {
	switch (action) {
	case 3:
		_opponent->send(103);
		break;
	case 5:
		doAction(7, 1);
		_opponent->send(103);
		if (_hitPoints <= 1)
			_hitPoints = 1;

		break;
	case 131:
		break;
	default:
		CFighter::send(action);
		break;
	}
}

} // End of namespace LastExpress
