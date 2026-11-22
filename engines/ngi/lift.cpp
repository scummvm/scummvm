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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/objectnames.h"
#include "ngi/constants.h"

#include "ngi/scene.h"
#include "ngi/statics.h"
#include "ngi/messages.h"
#include "ngi/gameloader.h"
#include "ngi/motion.h"

namespace NGI {

int NGIEngine::lift_getButtonIdP(int objid) {
	switch (objid) {
	case ST_LBN_0N:
		return ST_LBN_0P;

	case ST_LBN_1N:
		return ST_LBN_1P;

	case ST_LBN_2N:
		return ST_LBN_2P;

	case ST_LBN_3N:
		return ST_LBN_3P;

	case ST_LBN_4N:
		return ST_LBN_4P;

	case ST_LBN_5N:
		return ST_LBN_5P;

	case ST_LBN_6N:
		return ST_LBN_6P;

	case ST_LBN_7N:
		return ST_LBN_7P;

	case ST_LBN_8N:
		return ST_LBN_8P;

	case ST_LBN_9N:
		return ST_LBN_9P;

	default:
		return 0;
	}
}

int NGIEngine::lift_getButtonIdH(int objid) {
	switch (objid) {
	case ST_LBN_0P:
		return ST_LBN_0H;

	case ST_LBN_1P:
		return ST_LBN_1H;

	case ST_LBN_2P:
		return ST_LBN_2H;

	case ST_LBN_3P:
		return ST_LBN_3H;

	case ST_LBN_4P:
		return ST_LBN_4H;

	case ST_LBN_5P:
		return ST_LBN_5H;

	case ST_LBN_6P:
		return ST_LBN_6H;

	case ST_LBN_7P:
		return ST_LBN_7H;

	case ST_LBN_8P:
		return ST_LBN_8H;

	case ST_LBN_9P:
		return ST_LBN_9H;

	default:
		return 0;
	}
}

int NGIEngine::lift_getButtonIdN(int objid) {
	switch (objid) {
	case ST_LBN_0H:
	case ST_LBN_0N:
	case ST_LBN_0P:
		return ST_LBN_0N;

	case ST_LBN_1H:
	case ST_LBN_1N:
	case ST_LBN_1P:
		return ST_LBN_1N;

	case ST_LBN_2H:
	case ST_LBN_2N:
	case ST_LBN_2P:
		return ST_LBN_2N;

	case ST_LBN_3H:
	case ST_LBN_3N:
	case ST_LBN_3P:
		return ST_LBN_3N;

	case ST_LBN_4H:
	case ST_LBN_4N:
	case ST_LBN_4P:
		return ST_LBN_4N;

	case ST_LBN_5H:
	case ST_LBN_5N:
	case ST_LBN_5P:
		return ST_LBN_5N;

	case ST_LBN_6H:
	case ST_LBN_6N:
	case ST_LBN_6P:
		return ST_LBN_6N;

	case ST_LBN_7H:
	case ST_LBN_7N:
	case ST_LBN_7P:
		return ST_LBN_7N;

	case ST_LBN_8H:
	case ST_LBN_8N:
	case ST_LBN_8P:
		return ST_LBN_8N;

	case ST_LBN_9H:
	case ST_LBN_9N:
	case ST_LBN_9P:
		return ST_LBN_9N;

	default:
		return 0;
	}
}

void NGIEngine::lift_setButton(const char *name, int state) {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (var)
		var->setSubVarAsInt(name, state);
}

void NGIEngine::lift_init(Scene *sc, int enterSeq, int exitSeq) {
	_lastLiftButton = nullptr;

	_liftEnterMQ = sc->findMessageQueue(enterSeq);
	if (!_liftEnterMQ)
		return;

	_liftExitMQ = sc->findMessageQueue(exitSeq);

	if (!_liftExitMQ)
		return;

	Command *ex = _liftEnterMQ->getCommandByIndex(0);

	if (!ex)
		return;

	_liftX = ex->_x;
	_liftY = ex->_y;

	_lift = sc->findAniObjectById(ANI_LIFT, -1);

	for (uint i = 0; i < sc->_aniObjects.size(); i++) {
		AniObject *ani = sc->_aniObjects[i];

		if (ani->_id == ANI_LIFTBUTTON)
			ani->_statics = ani->getStatic(lift_getButtonIdP(ani->_statics->_staticsId));
	}

	GameVar *var = getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);
	if (var) {
		for (var = var->_subVars; var; var = var->_nextVarObj) {
			for (uint i = 0; i < sc->_aniObjects.size(); i++) {
				AniObject *ani = sc->_aniObjects[i];

				if (ani->_id == ANI_LIFTBUTTON) {
					int id = lift_getButtonIdN(ani->_statics->_staticsId);

					if (id == var->_value.intValue)
						ani->_statics = ani->getStatic(id);
				}

			}
		}
	}
}

void NGIEngine::lift_exitSeq(Command *cmd) {
	if (cmd) {
		MessageQueue *mq = _globalMessageQueueList->getMessageQueueById(cmd->_parId);

		if (mq)
			mq->activateExCommandsByKind(34);
	}

	_lift->setStatic(ST_LFT_CLOSED);

	MessageQueue *mq = new MessageQueue(_globalMessageQueueList->compact());

	Command *ex = new Command(_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);

	ex->_z = 256;
	ex->_messageNum = 256;
	ex->_excFlags |= 3;
	mq->addCommandToEnd(ex);

	if (!cmd) {
		ex = new Command(_aniMan->_id, 2, 40, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _aniMan->_copyId;
		ex->_excFlags |= 2;
		mq->addCommandToEnd(ex);
	}

	ex = new Command(_lift->_id, 1, MV_LFT_OPEN, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _lift->_copyId;
	ex->_excFlags |= 2;
	mq->addCommandToEnd(ex);

	ex = new Command(_aniMan->_id, 1, MV_MAN_STARTD, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _aniMan->_copyId;
	ex->_excFlags |= 2;
	mq->addCommandToEnd(ex);

	ex = new Command(_aniMan->_id, 5, -1, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _aniMan->_copyId;
	ex->_z = 10;
	ex->_x = -1;
	ex->_y = -1;
	ex->_excFlags |= 3;
	mq->addCommandToEnd(ex);

	ex = new Command(_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;
	ex->_z = 256;
	ex->_messageNum = 0;
	mq->addCommandToEnd(ex);

	ex = new Command(0, 17, MSG_LIFT_STARTEXITQUEUE, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;
	mq->addCommandToEnd(ex);

	ex = new Command(_lift->_id, 1, MV_LFT_CLOSE, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _lift->_copyId;
	ex->_excFlags |= 2;

	mq->addCommandToEnd(ex);

	mq->run(nullptr);
}

void NGIEngine::lift_closedoorSeq() {
	if (_lift->_movement) {
		if (_lift->_movement->_id == MV_LFT_CLOSE) {
			_lift->setQueue(nullptr);
		} else if (_lift->_movement->_id == MV_LFT_OPEN) {
			int ph = _lift->_movement->_curFrame;

			_lift->setStatic(ST_LFT_OPEN_NEW);
			_lift->startMovement(MV_LFT_CLOSE, 0, -1);

			if (_lift->_movement->_currMovement)
				_lift->_movement->setCurFrame(_lift->_movement->_currMovement->_phases.size() - ph);
			else
				_lift->_movement->setCurFrame(_lift->_movement->_phases.size() - ph);
		} else {
			_lift->setStatic(ST_LFT_OPEN_NEW);

			_lift->startMovement(MV_LFT_CLOSE, 0, -1);
		}
	} else {
		if (_lift->_statics->_staticsId == ST_LFT_CLOSED ) {
			_lift->setStatic(ST_LFT_CLOSED);
		} else {
			_lift->startMovement(MV_LFT_CLOSE, 0, -1);
		}
	}

	MessageQueue *mq = new MessageQueue(_globalMessageQueueList->compact());
	Command *ex = new Command(0, 17, MSG_LIFT_GO, 0, 0, 0, 1, 0, 0, 0);

	ex->_excFlags |= 3;
	mq->addCommandToEnd(ex);

	if (!mq->run(_lift))
		delete mq;
}

void NGIEngine::lift_walkAndGo() {
	MessageQueue *mq;
	Command *ex;

	if (abs(_liftX - _aniMan->_x) > 1 || abs(_liftY - _aniMan->_y) > 1 || _aniMan->_movement || _aniMan->_statics->_staticsId != ST_MAN_UP) {
		mq = getCurrSceneSc2MotionController()->startMove(_aniMan, _liftX, _liftY, 1, ST_MAN_UP);

		if (mq) {
			ex = new Command(0, 17, MSG_LIFT_CLICKBUTTON, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 3;

			mq->addCommandToEnd(ex);
		}
	} else {
		lift_openLift();

		mq = new MessageQueue(_liftEnterMQ, 0, 0);

		mq->setFlags(mq->getFlags() | 1);

		ex = new Command(_aniMan->_id, 2, 15, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _aniMan->_copyId;
		ex->_excFlags |= 2;
		mq->addCommandToFront(ex);

		ex = new Command(_aniMan->_id, 5, -1, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _aniMan->_copyId;
		ex->_z = _lift->_z + 1;
		ex->_x = -1;
		ex->_y = -1;
		ex->_excFlags |= 3;
		mq->addCommandToEnd(ex);

		ex = new Command(0, 17, MSG_LIFT_CLOSEDOOR, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		mq->addCommandToEnd(ex);

		mq->run(nullptr);

		_aniMan->_flags |= 0x100;
	}
}

void NGIEngine::lift_openLift() {
	if (_lift->_movement) {
		if (_lift->_movement->_id == MV_LFT_OPEN) {
			_lift->setQueue(nullptr);
		} else if (_lift->_movement->_id == MV_LFT_CLOSE) {
			int idx = _lift->_movement->_curFrame;

			_lift->setStatic(ST_LFT_CLOSED);
			_lift->startMovement(MV_LFT_OPEN, 0, -1);

			if (_lift->_movement->_currMovement)
				_lift->_movement->setCurFrame(_lift->_movement->_currMovement->_phases.size() - idx);
			else
				_lift->_movement->setCurFrame(_lift->_movement->_phases.size() - idx);
		} else {
			_lift->setStatic(ST_LFT_CLOSED);
			_lift->startMovement(MV_LFT_OPEN, 0, -1);
		}
	} else if (_lift->_statics->_staticsId == ST_LFT_OPEN_NEW) {
		_lift->setStatic(ST_LFT_OPEN_NEW);
	} else {
		_lift->startMovement(MV_LFT_OPEN, 0, -1);
	}
}

void NGIEngine::lift_clickButton() {
	if (_lastLiftButton)
		lift_walkAndGo();
}

void NGIEngine::lift_goAnimation() {
	if (_lastLiftButton) {
		int parentId = _currentScene->_sceneId;
		int buttonId = lift_getButtonIdN(_lastLiftButton->_statics->_staticsId);

		if (!buttonId)
			return;

		int numItems = _gameLoader->_preloadItems.size();

		for (int i = 0; i < numItems; i++) {
			PreloadItem &pre = _gameLoader->_preloadItems[i];

			if (pre.preloadId2 == buttonId && pre.preloadId1 == _currentScene->_sceneId) {
				MessageQueue *mq = new MessageQueue(_globalMessageQueueList->compact());

				Command *ex = new Command(ANI_MAN, 1, (pre.param != LiftDown ? MV_MAN_LIFTDOWN : MV_MAN_LIFTUP), 0, 0, 0, 1, 0, 0, 0);

				ex->_param = -1;
				ex->_wait = 1;
				ex->_excFlags |= 2;

				mq->addCommandToEnd(ex);

				ex = new Command(parentId, 17, 61, 0, 0, 0, 1, 0, 0, 0);

				ex->_param = buttonId;
				ex->_excFlags |= 3;

				mq->addCommandToEnd(ex);

				_aniMan->_flags &= ~0x100;

				if (!mq->run(_aniMan))
					delete mq;

				_aniMan->_flags |= 0x100;

				return;
			}
		}
	}

	lift_exitSeq(nullptr);

	if (_lastLiftButton) {
		_lastLiftButton->_statics = _lastLiftButton->getStatic(lift_getButtonIdN(_lastLiftButton->_statics->_staticsId));
		_lastLiftButton = nullptr;
	}
}

void NGIEngine::lift_animateButton(AniObject *button) {
	int butId = lift_getButtonIdP(button->_statics->_staticsId);

	if (butId && butId != button->_statics->_staticsId) {
		if (button == _lastLiftButton) {
			playSound(SND_CMN_032, 0);
		} else {
			if (_lastLiftButton) {
				int id = lift_getButtonIdN(_lastLiftButton->_statics->_staticsId);

				if (id)
					_lastLiftButton->_statics = _lastLiftButton->getStatic(id);

				_lastLiftButton = nullptr;
			}

			if (_aniMan->isIdle() && !(_aniMan->_flags & 0x100)) {
				_lastLiftButton = button;
				button->_statics = button->getStatic(butId);

				Command *ex = new Command(0, 35, SND_CMN_032, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				ex->_z = 1;
				ex->postMessage();

				int id = lift_getButtonIdH(_lastLiftButton->_statics->_staticsId);

				if (id) {
					_lastLiftButton->_statics = _lastLiftButton->getStatic(id);

					lift_walkAndGo();
				}
			}
		}
	}
}

void NGIEngine::lift_startExitQueue() {
	MessageQueue *mq = new MessageQueue(_liftExitMQ, 0, 0);

	mq->run(nullptr);
}

void NGIEngine::lift_hoverButton(Command *cmd) {
	if (_lastLiftButton) {
		if (!(cmd->_param & 2) || _liftX != cmd->_x || _liftY != cmd->_y) {
			_lastLiftButton->_statics = _lastLiftButton->getStatic(lift_getButtonIdN(_lastLiftButton->_statics->_staticsId));
			_lastLiftButton = nullptr;
		}
	}
}

bool NGIEngine::lift_checkButton(const char *varName) {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (var)
		return lift_getButtonIdP(var->getSubVarByName(varName)->_value.intValue) > 0;

	return false;
}

void NGIEngine::lift_setButtonStatics(Scene *sc, int buttonId) {
	for (uint i = 0; i < sc->_aniObjects.size(); i++) {
		AniObject *ani = sc->_aniObjects[i];

		if (ani->_id == ANI_LIFTBUTTON) {
			int id = lift_getButtonIdN(ani->_statics->_staticsId);

			if (id == buttonId)
				ani->_statics = ani->getStatic(id);
		}
	}
}


} // End of namespace NGI
