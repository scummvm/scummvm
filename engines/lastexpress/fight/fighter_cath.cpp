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

namespace LastExpress {

void CCath::timer() {
	CFighter::timer();

	if (_currentSprite) {
		if (_nextMessage == 102) {
			_currentSprite->hotspotPriority = 2;
		} else {
			_currentSprite->hotspotPriority = 0;
		}
	}
}

CCath1::CCath1(LastExpressEngine *engine, CFight *fight) : CCath(engine, fight) {
	_numSeqs = 7;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2001cr.seq",   15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2001cdl.seq",  15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2001cdr.seq",  15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2001cdm.seq",  15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2001csgr.seq", 15, 0);
	_seqs[5] = _engine->getArchiveManager()->loadSeq("2001csgl.seq", 15, 0);
	_seqs[6] = _engine->getArchiveManager()->loadSeq("2001dbk.seq",  15, 0);
}

void CCath1::timer() {
	if (_currentSprite && (_currentSprite->flags & 2) != 0) {
		if (_opponent->isDead()) {
			doAction(5, 1);
			_opponent->doAction(6, 1);
			_engine->getLogicManager()->endDialog(kCharacterTableA);
			_engine->getLogicManager()->playDialog(kCharacterClerk, "MUS029", 16, 0);
			send(104);
		}

		if (_currentActionIdx == 4) {
			_opponent->send(4);
			_fight->setOutcome(0);
		}
	}

	CCath::timer();
}

bool CCath1::actionAvailable(int action) {
	if (action != 128)
		return CFighter::actionAvailable(action);

	if (_currentActionIdx != 1)
		return CFighter::actionAvailable(action);

	if (!_currentSprite || (_currentSprite->flags & 4) != 0 || _opponent->getAction() != 1)
		return CFighter::actionAvailable(action);

	_engine->_cursorType = 9;

	return true;
}

void CCath1::send(int action) {
	switch (action) {
	case 1:
		if (_currentActionIdx != 1 || (_currentSprite->flags & 4) != 0) {
			doAction(6, 1);
			_opponent->doAction(3, 1);
			_opponent->send(103);
			timer();
		} else {
			_dodges++;
		}

		break;
	case 2:
		if ((_currentActionIdx == 2 || _currentActionIdx == 3) && (_currentSprite->flags & 4) == 0) {
			_dodges++;
		} else {
			doAction(6, 1);
			_opponent->doAction(4, 1);
			_opponent->send(103);
			timer();
		}

		break;
	case 128:
		if (_currentActionIdx == 1 && _currentSprite && (_currentSprite->flags & 4) == 0 && _opponent->getAction() == 1) {
			doAction(4, 1);
			timer();
		} else {
			int opponentAction = _opponent->getAction();
			if (opponentAction == 1) {
				doAction(1, 0);
			} else if (opponentAction == 2) {
				doAction(3, 0);
			} else {
				doAction(rnd(3) + 1, 0);
			}
		}

		break;
	default:
		CFighter::send(action);
		break;
	}
}

CCath2::CCath2(LastExpressEngine *engine, CFight *fight) : CCath(engine, fight) {
	_numSeqs = 5;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2002cr.seq",  15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2002cdl.seq", 15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2002cdr.seq", 15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2002cdm.seq", 15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2002lbk.seq", 15, 0);
}

void CCath2::send(int action) {
	if ((action - 1) > 127) {
		CFighter::send(action);
	} else {
		switch (action) {
		case 1:
			if ((_currentActionIdx == 1 || _currentActionIdx == 3) && (_currentSprite->flags & 4) == 0) {
				_dodges++;
			} else {
				doAction(4, 1);
				_opponent->doAction(4, 1);
				_opponent->send(103);
				timer();
			}
			break;
		case 2:
			if ((_currentActionIdx == 2 || _currentActionIdx == 3) && (_currentSprite->flags & 4) == 0) {
				_dodges++;
			} else {
				doAction(4, 1);
				_opponent->doAction(5, 1);
				_opponent->send(103);
				timer();
			}

			break;
		case 3:
			if ((_currentActionIdx == 2 || _currentActionIdx == 1) && (_currentSprite->flags & 4) == 0) {
				_dodges++;
			} else {
				doAction(4, 1);
				_opponent->doAction(6, 1);
				_opponent->send(103);
				timer();
			}

			break;
		case 128:
		{
			int opponentAction = _opponent->getAction();
			if (opponentAction == 1) {
				doAction(1, 0);
			} else if (opponentAction == 3) {
				doAction(2, 0);
			} else {
				doAction(3, 0);
			}

			break;
		}
		default:
			CFighter::send(action);
			break;
		}
	}

	if (_dodges > 4) {
		_engine->getLogicManager()->endDialog(kCharacterTableA);
		_fight->endFight(0);
	}
}

CCath3::CCath3(LastExpressEngine *engine, CFight *fight) : CCath(engine, fight) {
	_numSeqs = 10;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2003cr.seq",  15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2003car.seq", 15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2003cal.seq", 15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2003cdr.seq", 15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2003cdm.seq", 15, 0);
	_seqs[5] = _engine->getArchiveManager()->loadSeq("2003chr.seq", 15, 0);
	_seqs[6] = _engine->getArchiveManager()->loadSeq("2003chl.seq", 15, 0);
	_seqs[9] = _engine->getArchiveManager()->loadSeq("2003ckr.seq", 15, 0);
	_seqs[7] = _engine->getArchiveManager()->loadSeq("2003lbk.seq", 15, 0);
	_seqs[8] = _engine->getArchiveManager()->loadSeq("2003fbk.seq", 15, 0);

	_hitPoints = 5;
}

void CCath3::timer() {
	if ((_currentActionIdx == 4 || _currentActionIdx == 3) && !_currentSpriteIdx)
		_opponent->send(131);

	if (_currentSprite && (_currentSprite->flags & 2) != 0) {
		if (_opponent->isDead()) {
			doAction(9, 1);
			_opponent->doAction(8, 1);
			_engine->getLogicManager()->endDialog(kCharacterTableA);
			send(104);
			return;
		}

		if (_currentActionIdx == 3)
			_opponent->send(3);

		if (_currentActionIdx == 4)
			_opponent->send(4);
	}

	CCath::timer();
}

bool CCath3::actionAvailable(int action) {
	if (action == 129 || action == 130) {
		return _currentActionIdx != 7;
	} else {
		return CFighter::actionAvailable(action);
	}
}

void CCath3::send(int action) {
	if ((action - 1) > 129) {
		CFighter::send(action);
	} else {
		switch (action) {
		case 1:
			if (_currentActionIdx != 1 || (_currentSprite->flags & 4) != 0) {
				doAction(7, 1);
				_opponent->doAction(4, 1);
				_opponent->send(103);
				timer();
			}

			break;
		case 2:
			if (_currentActionIdx != 2 || (_currentSprite->flags & 4) != 0) {
				doAction(7, 1);
				_opponent->doAction(5, 1);
				_opponent->send(103);
				timer();
			}

			break;
		case 128:
			if (_opponent->getAction() == 2) {
				doAction(2, 0);
			} else {
				doAction(1, 0);
			}

			break;
		case 129:
			if (_currentActionIdx) {
				if (_opponent->getHitPoints() > 1) {
					doAction(4, 2);
				} else {
					doAction(3, 2);
				}
			} else if (_opponent->getHitPoints() > 1) {
				doAction(4, 0);
			} else {
				doAction(3, 0);
			}

			break;
		case 130:
			if (_currentActionIdx) {
				doAction(3, 2);
			} else {
				doAction(3, 0);
			}

			break;
		default:
			CFighter::send(action);
			break;
		}
	}
}

CCath4::CCath4(LastExpressEngine *engine, CFight *fight) : CCath(engine, fight) {
	_numSeqs = 4;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2004cr.seq",  15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2004cdr.seq", 15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2004chj.seq", 15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2004bk.seq",  15, 0);

	_hitPoints = 2;
}

void CCath4::timer() {
	CCath::timer();

	if ((_currentSprite->flags & 2) != 0) {
		if (_opponent->isDead()) {
			_engine->getLogicManager()->endDialog(kCharacterTableA);
			_fight->endFight(0);
		} else if (_currentActionIdx == 2) {
			_opponent->send(2);
		}
	}
}

bool CCath4::actionAvailable(int action) {
	if (action != 131)
		return CFighter::actionAvailable(action);

	if (_currentActionIdx != 1)
		return false;

	if (_opponent->isDead())
		_engine->_cursorType = 9;

	return true;
}

void CCath4::send(int action) {
	if ((action - 1) > 130) {
		CFighter::send(action);
	} else {
		switch (action) {
		case 1:
			if (_currentActionIdx != 1 || (_currentSprite->flags & 4) != 0) {
				_dodges = 0;
				doAction(3, 1);
				_opponent->doAction(3, 1);
				_opponent->send(103);
				timer();
			} else {
				_dodges++;
			}

			break;
		case 2:
			if (_currentActionIdx != 1 || (_currentSprite->flags & 4) != 0) {
				_dodges = 0;
				doAction(3, 1);
				_opponent->doAction(4, 1);
				_opponent->send(103);
				_hitPoints = 0;
				timer();
			} else {
				_dodges++;
			}

			break;
		case 128:
			doAction(1, 0);
			_dodges = 0;
			break;
		case 131:
			if (_currentActionIdx) {
				doAction(2, 2);
			} else {
				doAction(2, 0);
			}

			break;
		default:
			CFighter::send(action);
			break;
		}
	}
}

CCath5::CCath5(LastExpressEngine *engine, CFight *fight) : CCath(engine, fight) {
	_numSeqs = 6;

	_seqs[0] = _engine->getArchiveManager()->loadSeq("2005cr.seq",   15, 0);
	_seqs[1] = _engine->getArchiveManager()->loadSeq("2005cdr.seq",  15, 0);
	_seqs[2] = _engine->getArchiveManager()->loadSeq("2005cbr.seq",  15, 0);
	_seqs[4] = _engine->getArchiveManager()->loadSeq("2005bk.seq",   15, 0);
	_seqs[3] = _engine->getArchiveManager()->loadSeq("2005cdm1.seq", 15, 0);
	_seqs[5] = _engine->getArchiveManager()->loadSeq("2005chl.seq",  15, 0);
}

void CCath5::timer() {
	if (_currentSprite && (_currentSprite->flags & 2) != 0) {
		if (_currentActionIdx == 3)
			_opponent->send(3);

		if (_opponent->isDead()) {
			_engine->getLogicManager()->endDialog(kCharacterTableA);
			_fight->endFight(0);
			return;
		}

		if (_currentActionIdx == 5)
			_opponent->send(5);
	}

	CCath::timer();
}

bool CCath5::actionAvailable(int action) {
	if (action != 128)
		return CFighter::actionAvailable(action);

	if (_currentActionIdx != 1) {
		if (_opponent->getAction() == 5) {
			_engine->_cursorType = 6;
			return true;
		}

		return CFighter::actionAvailable(action);
	}

	if (_opponent->getAction() != 1)
		return false;

	if (!_currentSprite || (_currentSprite->flags & 4) == 0)
		return false;

	_engine->_cursorType = 30;

	return true;
}

void CCath5::send(int action) {
	if ((action - 1) > 131) {
		CFighter::send(action);
	} else {
		switch (action) {
		case 1:
			if (_currentActionIdx != 1) {
				_opponent->send(103);
				timer();
			} else {
				_dodges++;
			}

			break;
		case 2:
			if (_currentActionIdx != 2) {
				_opponent->send(103);
				timer();
			} else {
				_dodges++;
			}

			break;
		case 5:
			if (_currentActionIdx != 3) {
				_opponent->send(103);
				timer();
			}

			break;
		case 128:
			if (_currentActionIdx == 1 && _opponent->getAction() == 1 && _currentSprite && (_currentSprite->flags & 4) != 0) {
				doAction(5, 1);
			} else if (_opponent->getAction() == 5) {
				doAction(3, 0);
			} else {
				doAction(1, 0);
			}

			break;
		case 132:
			doAction(2, 0);
			break;
		default:
			CFighter::send(action);
			break;
		}
	}

	if (_dodges > 10) {
		_opponent->doAction(5, 2);
		_opponent->setHitPoints(1);
		_dodges = 0;
	}
}

} // End of namespace LastExpress
