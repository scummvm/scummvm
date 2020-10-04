/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	_lastLiftButton = 0;

	_liftEnterMQ = sc->getMessageQueueById(enterSeq);
	if (!_liftEnterMQ)
		return;

	_liftExitMQ = sc->getMessageQueueById(exitSeq);

	if (!_liftExitMQ)
		return;

	ExCommand *ex = _liftEnterMQ->getExCommandByIndex(0);

	if (!ex)
		return;

	_liftX = ex->_x;
	_liftY = ex->_y;

	_lift = sc->getStaticANIObject1ById(ANI_LIFT, -1);

	for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++) {
		StaticANIObject *ani = sc->_staticANIObjectList1[i];

		if (ani->_id == ANI_LIFTBUTTON)
			ani->_statics = ani->getStaticsById(lift_getButtonIdP(ani->_statics->_staticsId));
	}

	GameVar *var = getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);
	if (var) {
		for (var = var->_subVars; var; var = var->_nextVarObj) {
			for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++) {
				StaticANIObject *ani = sc->_staticANIObjectList1[i];

				if (ani->_id == ANI_LIFTBUTTON) {
					int id = lift_getButtonIdN(ani->_statics->_staticsId);

					if (id == var->_value.intValue)
						ani->_statics = ani->getStaticsById(id);
				}

			}
		}
	}
}

void NGIEngine::lift_exitSeq(ExCommand *cmd) {
	if (cmd) {
		MessageQueue *mq = _globalMessageQueueList->getMessageQueueById(cmd->_parId);

		if (mq)
			mq->activateExCommandsByKind(34);
	}

	_lift->changeStatics2(ST_LFT_CLOSED);

	MessageQueue *mq = new MessageQueue(_globalMessageQueueList->compact());

	ExCommand *ex = new ExCommand(_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);

	ex->_z = 256;
	ex->_messageNum = 256;
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);

	if (!cmd) {
		ex = new ExCommand(_aniMan->_id, 2, 40, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _aniMan->_odelay;
		ex->_excFlags |= 2;
		mq->addExCommandToEnd(ex);
	}

	ex = new ExCommand(_lift->_id, 1, MV_LFT_OPEN, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _lift->_odelay;
	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_aniMan->_id, 1, MV_MAN_STARTD, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _aniMan->_odelay;
	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_aniMan->_id, 5, -1, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _aniMan->_odelay;
	ex->_z = 10;
	ex->_x = -1;
	ex->_y = -1;
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;
	ex->_z = 256;
	ex->_messageNum = 0;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(0, 17, MSG_LIFT_STARTEXITQUEUE, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_lift->_id, 1, MV_LFT_CLOSE, 0, 0, 0, 1, 0, 0, 0);
	ex->_param = _lift->_odelay;
	ex->_excFlags |= 2;

	mq->addExCommandToEnd(ex);

	mq->chain(0);
}

void NGIEngine::lift_closedoorSeq() {
	if (_lift->_movement) {
		if (_lift->_movement->_id == MV_LFT_CLOSE) {
			_lift->queueMessageQueue(0);
		} else if (_lift->_movement->_id == MV_LFT_OPEN) {
			int ph = _lift->_movement->_currDynamicPhaseIndex;

			_lift->changeStatics2(ST_LFT_OPEN_NEW);
			_lift->startAnim(MV_LFT_CLOSE, 0, -1);

			if (_lift->_movement->_currMovement)
				_lift->_movement->setDynamicPhaseIndex(_lift->_movement->_currMovement->_dynamicPhases.size() - ph);
			else
				_lift->_movement->setDynamicPhaseIndex(_lift->_movement->_dynamicPhases.size() - ph);
		} else {
			_lift->changeStatics2(ST_LFT_OPEN_NEW);

			_lift->startAnim(MV_LFT_CLOSE, 0, -1);
		}
	} else {
		if (_lift->_statics->_staticsId == ST_LFT_CLOSED ) {
			_lift->changeStatics2(ST_LFT_CLOSED);
		} else {
			_lift->startAnim(MV_LFT_CLOSE, 0, -1);
		}
	}

	MessageQueue *mq = new MessageQueue(_globalMessageQueueList->compact());
	ExCommand *ex = new ExCommand(0, 17, MSG_LIFT_GO, 0, 0, 0, 1, 0, 0, 0);

	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);

	if (!mq->chain(_lift))
		delete mq;
}

void NGIEngine::lift_walkAndGo() {
	MessageQueue *mq;
	ExCommand *ex;

	if (abs(_liftX - _aniMan->_ox) > 1 || abs(_liftY - _aniMan->_oy) > 1 || _aniMan->_movement || _aniMan->_statics->_staticsId != ST_MAN_UP) {
		mq = getCurrSceneSc2MotionController()->startMove(_aniMan, _liftX, _liftY, 1, ST_MAN_UP);

		if (mq) {
			ex = new ExCommand(0, 17, MSG_LIFT_CLICKBUTTON, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 3;

			mq->addExCommandToEnd(ex);
		}
	} else {
		lift_openLift();

		mq = new MessageQueue(_liftEnterMQ, 0, 0);

		mq->setFlags(mq->getFlags() | 1);

		ex = new ExCommand(_aniMan->_id, 2, 15, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _aniMan->_odelay;
		ex->_excFlags |= 2;
		mq->addExCommand(ex);

		ex = new ExCommand(_aniMan->_id, 5, -1, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _aniMan->_odelay;
		ex->_z = _lift->_priority + 1;
		ex->_x = -1;
		ex->_y = -1;
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(0, 17, MSG_LIFT_CLOSEDOOR, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);

		mq->chain(0);

		_aniMan->_flags |= 0x100;
	}
}

void NGIEngine::lift_openLift() {
	if (_lift->_movement) {
		if (_lift->_movement->_id == MV_LFT_OPEN) {
			_lift->queueMessageQueue(0);
		} else if (_lift->_movement->_id == MV_LFT_CLOSE) {
			int idx = _lift->_movement->_currDynamicPhaseIndex;

			_lift->changeStatics2(ST_LFT_CLOSED);
			_lift->startAnim(MV_LFT_OPEN, 0, -1);

			if (_lift->_movement->_currMovement)
				_lift->_movement->setDynamicPhaseIndex(_lift->_movement->_currMovement->_dynamicPhases.size() - idx);
			else
				_lift->_movement->setDynamicPhaseIndex(_lift->_movement->_dynamicPhases.size() - idx);
		} else {
			_lift->changeStatics2(ST_LFT_CLOSED);
			_lift->startAnim(MV_LFT_OPEN, 0, -1);
		}
	} else if (_lift->_statics->_staticsId == ST_LFT_OPEN_NEW) {
		_lift->changeStatics2(ST_LFT_OPEN_NEW);
	} else {
		_lift->startAnim(MV_LFT_OPEN, 0, -1);
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

				ExCommand *ex = new ExCommand(ANI_MAN, 1, (pre.param != LiftDown ? MV_MAN_LIFTDOWN : MV_MAN_LIFTUP), 0, 0, 0, 1, 0, 0, 0);

				ex->_param = -1;
				ex->_field_24 = 1;
				ex->_excFlags |= 2;

				mq->addExCommandToEnd(ex);

				ex = new ExCommand(parentId, 17, 61, 0, 0, 0, 1, 0, 0, 0);

				ex->_param = buttonId;
				ex->_excFlags |= 3;

				mq->addExCommandToEnd(ex);

				_aniMan->_flags &= ~0x100;

				if (!mq->chain(_aniMan))
					delete mq;

				_aniMan->_flags |= 0x100;

				return;
			}
		}
	}

	lift_exitSeq(0);

	if (_lastLiftButton) {
		_lastLiftButton->_statics = _lastLiftButton->getStaticsById(lift_getButtonIdN(_lastLiftButton->_statics->_staticsId));
		_lastLiftButton = 0;
	}
}

void NGIEngine::lift_animateButton(StaticANIObject *button) {
	int butId = lift_getButtonIdP(button->_statics->_staticsId);

	if (butId && butId != button->_statics->_staticsId) {
		if (button == _lastLiftButton) {
			playSound(SND_CMN_032, 0);
		} else {
			if (_lastLiftButton) {
				int id = lift_getButtonIdN(_lastLiftButton->_statics->_staticsId);

				if (id)
					_lastLiftButton->_statics = _lastLiftButton->getStaticsById(id);

				_lastLiftButton = 0;
			}

			if (_aniMan->isIdle() && !(_aniMan->_flags & 0x100)) {
				_lastLiftButton = button;
				button->_statics = button->getStaticsById(butId);

				ExCommand *ex = new ExCommand(0, 35, SND_CMN_032, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				ex->_z = 1;
				ex->postMessage();

				int id = lift_getButtonIdH(_lastLiftButton->_statics->_staticsId);

				if (id) {
					_lastLiftButton->_statics = _lastLiftButton->getStaticsById(id);

					lift_walkAndGo();
				}
			}
		}
	}
}

void NGIEngine::lift_startExitQueue() {
	MessageQueue *mq = new MessageQueue(_liftExitMQ, 0, 0);

	mq->chain(0);
}

void NGIEngine::lift_hoverButton(ExCommand *cmd) {
	if (_lastLiftButton) {
		if (!(cmd->_param & 2) || _liftX != cmd->_x || _liftY != cmd->_y) {
			_lastLiftButton->_statics = _lastLiftButton->getStaticsById(lift_getButtonIdN(_lastLiftButton->_statics->_staticsId));
			_lastLiftButton = 0;
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
	for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++) {
		StaticANIObject *ani = sc->_staticANIObjectList1[i];

		if (ani->_id == ANI_LIFTBUTTON) {
			int id = lift_getButtonIdN(ani->_statics->_staticsId);

			if (id == buttonId)
				ani->_statics = ani->getStaticsById(id);
		}
	}
}


} // End of namespace NGI
