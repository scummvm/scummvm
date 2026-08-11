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

CFighter::CFighter(LastExpressEngine *engine, CFight *fight) {
	_engine = engine;
	_nextMessage = 101;
	_fight = fight;
	_currentSeq = 0;
	_currentSprite = 0;
	_currentSpriteIdx = 0;
	_unusedFlag = 0;
	_currentActionIdx = 0;
	_nextSequenceIdx = 0;
	_hitPoints = 1;
	_dodges = 0;

	for (int i = 0; i < ARRAYSIZE(_seqs); i++) {
		_seqs[i] = nullptr;
	}
}

CFighter::~CFighter() {
	_engine->getSpriteManager()->destroySprite(&_currentSprite, false);

	for (int i = 0; i < _numSeqs; i++) {
		if (_seqs[i]) {
			_engine->getMemoryManager()->freeMem(_seqs[i]->rawSeqData);
			delete _seqs[i];
			_seqs[i] = nullptr;
		}
	}
}

void CFighter::timer() {
	uint8 soundAction;
	uint8 sounds[5];
	char filename[16];

	Sprite *sprite = nullptr;

	if (!_currentSeq) {
		if (_currentSprite) {
			_engine->getSpriteManager()->removeSprite(_currentSprite);

			// The original did this, but at this point sprite is necessarily nullptr...
			// if (!sprite || !sprite->copyScreenAndRedrawFlag)
			_engine->getSpriteManager()->queueErase(_currentSprite);
		}

		_currentSprite = sprite;
		return;
	}

	if (_currentSeq->numFrames <= _currentSpriteIdx) {
		switch (_nextMessage) {
		case 101:
			doAction(_nextSequenceIdx, 1);
			_nextSequenceIdx = 0;
			break;
		case 102:
			_currentSpriteIdx = 0;
			break;
		case 103:
			doAction(0, 1);
			send(101);
			_opponent->doAction(0, 1);

			_opponent->send(101);
			_opponent->timer();
			break;
		case 104:
			_fight->endFight(0);
			break;
		case 105:
			_fight->endFight(1);
			break;
		default:
			break;
		}
	}

	if (_fight->fightHappening()) {
		sprite = &_currentSeq->sprites[_currentSpriteIdx];
		sprite->hotspotPriority = 1;
		if (_currentSprite != sprite) {
			soundAction = sprite->soundAction;
			if (soundAction) {
				switch (soundAction) {
				case 150:
					sounds[0] = 151;
					sounds[1] = 152;
					sounds[2] = 153;
					sounds[3] = 154;
					sounds[4] = 155;
					soundAction = sounds[rnd(5)];
					break;
				case 156:
					sounds[0] = 157;
					sounds[1] = 158;
					sounds[2] = 159;
					sounds[3] = 160;
					sounds[4] = 161;
					soundAction = sounds[rnd(5)];
					break;
				case 162:
					sounds[0] = 163;
					sounds[1] = 164;
					sounds[2] = 165;
					sounds[3] = 166;
					sounds[4] = 167;
					soundAction = sounds[rnd(5)];
					break;
				case 168:
					sounds[0] = 169;
					sounds[1] = 170;
					sounds[2] = 171;
					sounds[3] = 172;
					sounds[4] = 173;
					soundAction = sounds[rnd(5)];
					break;
				case 174:
					sounds[0] = 175;
					sounds[1] = 176;
					sounds[2] = 177;
					soundAction = sounds[rnd(3)];
					break;
				case 180:
					sounds[0] = 181;
					sounds[1] = 182;
					sounds[2] = 183;
					sounds[3] = 184;
					soundAction = sounds[rnd(4)];
					break;
				case 184:
					sounds[0] = 185;
					sounds[1] = 186;
					sounds[2] = 187;
					soundAction = sounds[rnd(3)];
					break;
				case 188:
					sounds[0] = 189;
					sounds[1] = 190;
					sounds[2] = 191;
					sounds[3] = 192;
					sounds[4] = 193;
					soundAction = sounds[rnd(5)];
					break;
				case 194:
					sounds[0] = 195;
					sounds[1] = 196;
					sounds[2] = 197;
					soundAction = sounds[rnd(3)];
					break;
				case 198:
					sounds[0] = 199;
					sounds[1] = 200;
					sounds[2] = 201;
					sounds[3] = 202;
					sounds[4] = 203;
					soundAction = sounds[rnd(5)];
					break;
				default:
					break;
				}

				Common::sprintf_s(filename, "LIB%03d.SND", soundAction);
				_engine->getLogicManager()->playDialog(kCharacterClerk, filename, 16, sprite->soundDelay);
			}

			_engine->getSpriteManager()->drawSprite(sprite);
			_currentSpriteIdx++;

			if (_currentSprite) {
				_engine->getSpriteManager()->removeSprite(_currentSprite);
				if (!sprite || !sprite->copyScreenAndRedrawFlag)
					_engine->getSpriteManager()->queueErase(_currentSprite);
			}

			_currentSprite = sprite;
			return;
		}
	}
}

void CFighter::doAction(int sequenceIndex, int action) {
	if (sequenceIndex >= 0 && _numSeqs >= sequenceIndex) {
		switch (action) {
		case 0:
		default:
			if (!_currentActionIdx) {
				_currentSeq = _seqs[sequenceIndex];
				_currentActionIdx = sequenceIndex;
				newSeq();
			}

			break;
		case 1:
			_currentSeq = _seqs[sequenceIndex];
			_currentActionIdx = sequenceIndex;
			_nextSequenceIdx = 0;
			newSeq();
			break;
		case 2:
			_nextSequenceIdx = sequenceIndex;
			break;
		}
	}
}

void CFighter::newSeq() {
	_engine->getSpriteManager()->destroySprite(&_currentSprite, false);
	_currentSpriteIdx = 0;
	_unusedFlag = 0;
}

bool CFighter::init(CFighter *opponent) {
	_opponent = opponent;

	if (_numSeqs <= 0)
		return true;

	for (int i = 0; i < _numSeqs; i++) {
		if (_seqs[i] == nullptr) {
			return false;
		}
	}

	return true;
}

bool CFighter::actionAvailable(int action) {
	return _nextMessage == 101 && !_currentActionIdx;
}

int CFighter::getAction() {
	return _currentActionIdx;
}

int CFighter::getDodges() {
	return _dodges;
}

int CFighter::getHitPoints() {
	return _hitPoints;
}

void CFighter::setHitPoints(int hitPoints) {
	_hitPoints = hitPoints;
}

bool CFighter::isDead() {
	return _hitPoints <= 0;
}

void CFighter::send(int message) {
	switch (message) {
	case 101:
		_nextMessage = message;
		break;
	case 102:
		--_hitPoints;
		_nextMessage = message;
		break;
	case 103:
		_opponent->send(102);
		_nextMessage = message;
		break;
	case 104:
		_fight->setOutcome(0);
		_opponent->send(102);
		_nextMessage = message;
		break;
	case 105:
		_fight->setOutcome(1);
		_opponent->send(102);
		_nextMessage = message;
		break;
	default:
		return;
	}
}

} // End of namespace LastExpress
