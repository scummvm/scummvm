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

#include "lastexpress/fight/fight.h"
#include "lastexpress/fight/fighter.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

int LastExpressEngine::doFight(int fightId) {
	int fightNode;
	TBM *tbm;

	if (_fightSkipCounter >= 5 && (fightId == 2004 || fightId == 2005)) {
		_fightSkipCounter = 0;
		return 0;
	}

	getLogicManager()->displayWaitIcon();
	getMessageManager()->clearClickEvents();

	getLogicManager()->_doubleClickFlag = false;
	mouseSetLeftClicked(false);
	mouseSetRightClicked(false);

	getOtisManager()->wipeAllGSysInfo();

	switch (fightId) {
	case 2001:
		fightNode = 44 - (getLogicManager()->getModel(1) == 2);
		break;
	case 2002:
		fightNode = 45;
		break;
	case 2003:
		fightNode = 46;
		break;
	case 2004:
		fightNode = 47;
		break;
	case 2005:
		fightNode = 48;
		break;
	default:
		fightNode = 820;
		break;
	}

	if (getGraphicsManager()->canDrawMouse()) {
		getGraphicsManager()->setMouseDrawable(false);
		getGraphicsManager()->burstMouseArea();
	}

	char sceneName[12];
	Common::strcpy_s(sceneName, getLogicManager()->_trainData[fightNode].sceneFilename);

	int bgResult = getArchiveManager()->loadBG(sceneName);
	if (bgResult <= 0) {
		if (bgResult)
			return 1;

		tbm = &getGraphicsManager()->_renderBox1;
	} else {
		tbm = &getGraphicsManager()->_renderBox2;
	}

	getLogicManager()->_activeNode = fightNode;

	(_characters->characters[kCharacterCath]).characterPosition = getLogicManager()->_trainData[fightNode].nodePosition;
	getSoundManager()->_scanAnySoundLoopingSection = true;

	if (getGraphicsManager()->acquireSurface()) {
		getGraphicsManager()->copy(getGraphicsManager()->_frontBuffer, (PixMap *)getGraphicsManager()->_screenSurface.getPixels(), tbm->x, tbm->y, tbm->width, tbm->height);
		getGraphicsManager()->unlockSurface();
	}

	getMemoryManager()->freeFX();

	_fight = new CFight(this, fightId);

	_fight->timer(0, 0);

	getGraphicsManager()->burstBox(tbm->x, tbm->y, tbm->width, tbm->height);
	getLogicManager()->restoreEggIcon();

	int outcome = _fight->process();

	delete _fight;
	_fight = nullptr;

	getMemoryManager()->lockFX();

	return outcome;
}

void LastExpressEngine::abortFight() {
	if (_fight)
		_fight->setFightHappening(false);
}

CFight::CFight(LastExpressEngine *engine, int fightId) {
	_engine = engine;

	_currentSeqIdx = 0;

	switch (fightId) {
	case 2001: // Cath vs Milos
		_cath = new CCath1(_engine, this);
		_opponent = new COpponent1(_engine, this);
		break;
	case 2002: // Cath vs Vesna (when saving Anna)
		_cath = new CCath2(_engine, this);
		_opponent = new COpponent2(_engine, this);
		break;
	case 2003: // Cath vs Ivo
		_cath = new CCath3(_engine, this);
		_opponent = new COpponent3(_engine, this);
		break;
	case 2004: // Cath vs Salko
		_cath = new CCath4(_engine, this);
		_opponent = new COpponent4(_engine, this);
		break;
	case 2005: // Cath vs Vesna (final fight)
		_cath = new CCath5(_engine, this);
		_opponent = new COpponent5(_engine, this);
		break;
	default:
		break;
	}

	if (!_cath || !_opponent) {
		error("Out of memory");
	}

	if (_cath->init(_opponent) && _opponent->init(_cath)) {
		_fightIsHappening = true;
		if (_engine->_fightSkipCounter >= 5) {
			switch (fightId) {
			case 2001:
				_opponent->setHitPoints(1);
				_cath->doAction(4, 0);
				_opponent->doAction(0, 0);
				break;
			case 2003:
				_opponent->setHitPoints(1);
				_cath->doAction(3, 0);
				_opponent->doAction(6, 0);
				break;
			case 2005:
				_opponent->setHitPoints(1);
				_cath->doAction(0, 0);
				_cath->doAction(3, 2);
				_opponent->doAction(5, 0);
				break;
			}
		} else {
			_cath->doAction(0, 0);
			_opponent->doAction(0, 0);
		}
	} else {
		endFight(0);
	}

	_savedMouseEventHandle = _engine->getMessageManager()->getEventHandle(1);
	_savedTimerEventHandle = _engine->getMessageManager()->getEventHandle(3);

	_engine->getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::fightMouseWrapper);
	_engine->getMessageManager()->setEventHandle(kEventChannelTimer, &LastExpressEngine::fightTimerWrapper);

	_engine->setEventTickInternal(false);
}

CFight::~CFight() {
	if (_cath) {
		delete _cath;
		_cath = nullptr;
	}

	if (_opponent) {
		delete _opponent;
		_opponent = nullptr;
	}

	_engine->getMessageManager()->setEventHandle(kEventChannelMouse, _savedMouseEventHandle);
	_engine->getMessageManager()->setEventHandle(kEventChannelTimer, _savedTimerEventHandle);
}

int CFight::process() {
	setOutcome(1);

	while (_fightIsHappening) {
		do {
			_engine->getSoundManager()->soundThread();
		} while (_engine->getMessageManager()->process());

		// Only wait and handle events if we've processed all messages, unlike the original which had a separate thread for input...
		_engine->waitForTimer(5);
	}

	return _outcome;
}

void CFight::setOutcome(int outcome) {
	_outcome = outcome;
}

void CFight::endFight(int outcome) {
	_engine->_fightSkipCounter = 0;
	setOutcome(outcome);
	_engine->abortFight();
}

void CFight::timer(Event *event, bool isProcessing) {
	_engine->setEventTickInternal(false);

	if (_engine->_gracePeriodTimer) {
		if ((_engine->getLogicManager()->_globals[kGlobalJacket] < 2 ? 225 : 450) == _engine->_gracePeriodTimer || _engine->_gracePeriodTimer == 900) {
			_eggIconBrightness = 0;
			_eggIconBrightnessStep = 1;
		}

		if (!_engine->_lockGracePeriod) // Gets set to true only by the debugger only...
			_engine->_gracePeriodTimer--;

		if (_engine->_gracePeriodTimer <= 500 ||
			!(_engine->_gracePeriodTimer % 5)) {

			if ((_engine->_gracePeriodTimer <= 500 && !(_engine->_gracePeriodTimer % ((_engine->_gracePeriodTimer + 100) / 100))) ||
				(_engine->_gracePeriodTimer > 500 && !(_engine->_gracePeriodTimer % 5))) {
				if (_eggIconBrightness) {
					_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, _eggIconBrightness);
				} else {
					_engine->getGraphicsManager()->drawItem(_engine->_currentGameFileColorId + 39, 608, 448);
				}

				_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
				_eggIconBrightness += _eggIconBrightnessStep;

				if (!_eggIconBrightness || _eggIconBrightness == 3)
					_eggIconBrightnessStep = -_eggIconBrightnessStep;
			}
		}

		if (_engine->_gracePeriodTimer == 90) {
			_engine->getSoundManager()->playSoundFile("TIMER.SND", kSoundTypeMenu | kVolumeFull, 0, 0);
		}

		if (_engine->_gracePeriodTimer < 90 && !_engine->getLogicManager()->dialogRunning("TIMER"))
			_engine->_gracePeriodTimer = 0;

		if (!_engine->_gracePeriodTimer) {
			if (_engine->_cursorX < 608 || _engine->_cursorY < 448 || _engine->_cursorX >= 640 || _engine->_cursorY >= 480) {
				_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, 1);
			} else {
				_engine->getGraphicsManager()->drawItem(_engine->_currentGameFileColorId + 39, 608, 448);
			}

			_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
			_engine->getVCR()->makePermanent();
		}
	}

	if (!_currentSeqIdx) {
		Link *link = nullptr;
		uint8 location = 0;

		for (Link *i = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].link; i; i = i->next) {
			if (_engine->getLogicManager()->pointIn(_engine->_cursorX, _engine->_cursorY, i) && location <= i->location) {
				location = i->location;
				link = i;
			}
		}

		if (!link || (_engine->_cursorType = link->cursor, !_cath->actionAvailable(link->action)))
			_engine->_cursorType = 0;

		_cath->timer();
		_opponent->timer();

		if (_fightIsHappening) {
			if (isProcessing) {
				_engine->getSpriteManager()->drawCycle();
			} else if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getSpriteManager()->drawCycleSimple((PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
				_engine->getGraphicsManager()->unlockSurface();
			}

			if (_currentSeqIdx) {
				_currentSeqIdx--;
			}
		}
	}
}

void CFight::mouse(Event *event) {
	if (_currentSeqIdx == 0) {
		_engine->mouseSetLeftClicked(false);
		_engine->getGraphicsManager()->setMouseDrawable(false);
		_engine->mouseSetRightClicked(false);

		_engine->getGraphicsManager()->burstMouseArea(false); // The original updated the screen, we don't to avoid flickering...

		_engine->_cursorX = event->x;
		_engine->_cursorY = event->y;

		if (_engine->_cursorX < 608 || _engine->_cursorY < 448 || _engine->_cursorX >= 640 || _engine->_cursorY >= 480) {
			if ((event->flags & kMouseFlagRightDown) != 0) {
				_engine->getLogicManager()->endDialog(kCharacterTableA);
				_engine->abortFight();

				if (_engine->_gracePeriodTimer) {
					_engine->_fightSkipCounter = 0;
				} else {
					_engine->_fightSkipCounter++;
				}

				_engine->mouseSetRightClicked(true);
			}

			if (_lowIconToggle) {
				if (!_engine->_gracePeriodTimer) {
					_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, 1);
					_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
				}

				_lowIconToggle = false;
			}

			uint8 location = 0;
			Link *link = nullptr;

			for (Link *i = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].link; i; i = i->next) {
				if (_engine->getLogicManager()->pointIn(_engine->_cursorX, _engine->_cursorY, i) && location <= i->location) {
					location = i->location;
					link = i;
				}
			}

			if (link) {
				_engine->_cursorType = link->cursor;

				if (_cath->actionAvailable(link->action)) {
					if ((event->flags & kMouseFlagLeftDown) != 0)
						_cath->send(link->action);
				} else {
					_engine->_cursorType = 0;
				}
			} else {
				_engine->_cursorType = 0;
			}
		} else {
			if (!_lowIconToggle) {
				if (!_engine->_gracePeriodTimer) {
					_engine->getGraphicsManager()->drawItem(_engine->_currentGameFileColorId + 39, 608, 448);
					_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
				}

				_lowIconToggle = true;
			}

			if ((event->flags & kMouseFlagLeftDown) != 0) {
				_lowIconToggle = false;
				_engine->getLogicManager()->endDialog(kCharacterTableA);
				endFight(2);
			} else if ((event->flags & kMouseFlagRightDown) != 0 && _engine->_gracePeriodTimer) {
				if (_engine->getLogicManager()->dialogRunning("TIMER"))
					_engine->getLogicManager()->endDialog("TIMER");

				_engine->_gracePeriodTimer = 900;
			}
		}

		_engine->getGraphicsManager()->setMouseDrawable(true);
		_engine->getGraphicsManager()->newMouseLoc();
		_engine->getGraphicsManager()->burstMouseArea();
	}
}

} // End of namespace LastExpress
