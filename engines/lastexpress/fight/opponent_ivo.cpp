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

COpponent3::COpponent3(LastExpressEngine *engine, CFight *fight) : COpponent(engine, fight) {
	_numSeqs = 9;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2003or.seq",  15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2003oal.seq", 15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2003oar.seq", 15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2003odm.seq", 15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2003okl.seq", 15, 0);
	_seqs[5] = _engine->getArchiveManager()->loadSeq("2003okj.seq", 15, 0);
	_seqs[6] = _engine->getArchiveManager()->loadSeq("blank.seq",   15, 0);
	_seqs[7] = _engine->getArchiveManager()->loadSeq("csdr.seq",    15, 0);
	_seqs[8] = _engine->getArchiveManager()->loadSeq("2003l.seq",   15, 0);

	_engine->getLogicManager()->playDialog(kCharacterTableA, "MUS032", 16, 0);

	_hitPoints = 5;
	_timer = 15;
}

void COpponent3::timer() {
	if (!_timer && actionAvailable(1) && !_nextSequenceIdx) {
		switch (rnd(5)) {
		case 0:
			doAction(1, 0);
			break;
		case 1:
			doAction(2, 0);
			break;
		case 2:
			doAction(1, 0);
			doAction(2, 2);
			break;
		case 3:
			doAction(2, 0);
			doAction(1, 2);
			break;
		case 4:
			doAction(1, 0);
			doAction(1, 2);
			break;
		default:
			break;
		}

		_timer = 3 * _hitPoints + rnd(10);
	}

	if (_currentSprite && (_currentSprite->flags & 2) != 0) {
		if (_opponent->isDead()) {
			doAction(7, 1);
			_opponent->doAction(8, 1);
			_engine->getLogicManager()->endDialog(kCharacterTableA);
			send(105);
		}

		if (_currentActionIdx == 1)
			_opponent->send(1);

		if (_currentActionIdx == 2)
			_opponent->send(2);
	}

	COpponent::timer();
}

void COpponent3::send(int action) {
	uint num;

	switch (action) {
	case 3:
		if ((_currentActionIdx != 3 && _currentActionIdx != 1) || (_currentSprite->flags & 4) != 0) {
			doAction(6, 1);
			_opponent->doAction(6, 1);
			_opponent->send(103);
		}

		break;
	case 4:
		if ((_currentActionIdx != 3 && _currentActionIdx != 2) || (_currentSprite->flags & 4) != 0) {
			doAction(6, 1);
			_opponent->doAction(5, 1);
			_opponent->send(103);
		}

		break;
	case 131:
		if (!_currentActionIdx) {
			if (_hitPoints > 2) {
				num = 60;
			} else {
				num = 75;
			}

			if (num >= rnd(100)) {
				doAction(3, 1);
				if (_opponent->getAction() == 4)
					doAction(2, 2);
			}
		}

		break;
	default:
		CFighter::send(action);
		break;
	}
}

} // End of namespace LastExpress
