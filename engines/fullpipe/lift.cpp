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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"

#include "fullpipe/scene.h"
#include "fullpipe/statics.h"
#include "fullpipe/messages.h"
#include "fullpipe/gameloader.h"

namespace Fullpipe {

int FullpipeEngine::lift_getButtonIdP(int objid) {
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

int FullpipeEngine::lift_getButtonIdH(int objid) {
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

int FullpipeEngine::lift_getButtonIdN(int objid) {
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

void FullpipeEngine::lift_setButton(const char *name, int state) {
	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (var)
		var->setSubVarAsInt(name, state);
}

void FullpipeEngine::lift_init(Scene *sc, int enterSeq, int exitSeq) {
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
		StaticANIObject *ani = (StaticANIObject *)sc->_staticANIObjectList1[i];

		if (ani->_id == ANI_LIFTBUTTON)
			ani->_statics = ani->getStaticsById(lift_getButtonIdP(ani->_statics->_staticsId));
	}

	GameVar *var = getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);
	if (var) {
		for (var = var->_subVars; var; var = var->_nextVarObj) {
			for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++) {
				StaticANIObject *ani = (StaticANIObject *)sc->_staticANIObjectList1[i];

				if (ani->_id == ANI_LIFTBUTTON) {
					int id = lift_getButtonIdN(ani->_statics->_staticsId);

					if (id == var->_value.intValue)
						ani->_statics = ani->getStaticsById(id);
				}

			}
		}
	}
}

void FullpipeEngine::lift_exitSeq(ExCommand *cmd) {
	if (cmd)
		_globalMessageQueueList->getMessageQueueById(cmd->_parId)->activateExCommandsByKind(34);

	_lift->changeStatics2(ST_LFT_CLOSED);

	MessageQueue *mq = new MessageQueue(_globalMessageQueueList->compact());

	ExCommand *ex = new ExCommand(_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);

	ex->_field_14 = 256;
	ex->_messageNum = 256;
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);

	if (!ex) {
		ex = new ExCommand(_aniMan->_id, 2, 40, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = _aniMan->_okeyCode;
		ex->_excFlags |= 2;
		mq->addExCommandToEnd(ex);
	}

	ex = new ExCommand(_lift->_id, 1, MV_LFT_OPEN, 0, 0, 0, 1, 0, 0, 0);
	ex->_keyCode = _lift->_okeyCode;
	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_aniMan->_id, 1, MV_MAN_STARTD, 0, 0, 0, 1, 0, 0, 0);
	ex->_keyCode = _aniMan->_okeyCode;
	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_aniMan->_id, 5, -1, 0, 0, 0, 1, 0, 0, 0);
	ex->_keyCode = _aniMan->_okeyCode;
	ex->_field_14 = 10;
	ex->_x = -1;
	ex->_y = -1;
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;
	ex->_field_14 = 256;
	ex->_messageNum = 0;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(0, 17, MSG_LIFT_STARTEXITQUEUE, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(_lift->_id, 1, MV_LFT_CLOSE, 0, 0, 0, 1, 0, 0, 0);
	ex->_keyCode = _lift->_okeyCode;
	ex->_excFlags |= 2;

	mq->addExCommandToEnd(ex);

	mq->chain(0);
}

void FullpipeEngine::lift_closedoorSeq() {
	warning("STUB: FullpipeEngine::lift_closedoorSeq()");
}

void FullpipeEngine::lift_walkAndGo() {
	warning("STUB: FullpipeEngine::lift_walkAndGo()");
}

void FullpipeEngine::lift_clickButton() {
	if (_lastLiftButton)
		lift_walkAndGo();
}

void FullpipeEngine::lift_goAnimation() {	if (_lastLiftButton) {
		int parentId = _currentScene->_sceneId;
		int buttonId = lift_getButtonIdN(_lastLiftButton->_statics->_staticsId);

		if (!buttonId)
			return;

		int numItems = _gameLoader->_preloadItems.size();

		for (int i = 0; i < numItems; i++) {
			PreloadItem *pre = _gameLoader->_preloadItems[i];

			if (pre->preloadId2 == buttonId && pre->preloadId1 == _currentScene->_sceneId) {
				MessageQueue *mq = new MessageQueue(_globalMessageQueueList->compact());

				ExCommand *ex = new ExCommand(ANI_MAN, 1, (pre->keyCode != LiftDown ? MV_MAN_LIFTDOWN : MV_MAN_LIFTUP), 0, 0, 0, 1, 0, 0, 0);

				ex->_keyCode = -1;
				ex->_field_24 = 1;
				ex->_excFlags |= 2;

				mq->addExCommandToEnd(ex);

				ex = new ExCommand(parentId, 17, 61, 0, 0, 0, 1, 0, 0, 0);

				ex->_keyCode = buttonId;
				ex->_excFlags |= 3;

				mq->addExCommandToEnd(ex);

				_aniMan->_flags &= 0xFEFF;

				if (!mq->chain(_aniMan))
					delete mq;

				_aniMan->_flags |= 1;
			}
		}
	}

	lift_exitSeq(0);

	if (_lastLiftButton) {
		_lastLiftButton->_statics = _lastLiftButton->getStaticsById(lift_getButtonIdN(_lastLiftButton->_statics->_staticsId));
		_lastLiftButton = 0;
	}
}

void FullpipeEngine::lift_sub1(StaticANIObject *ani) {
	warning("STUB: FullpipeEngine::lift_sub1()");
}

void FullpipeEngine::lift_startExitQueue() {
	MessageQueue *mq = new MessageQueue(_liftExitMQ, 0, 0);

	mq->chain(0);
}

void FullpipeEngine::lift_sub05(ExCommand *ex) {
	warning("STUB: FullpipeEngine::lift_sub05()");
}

bool FullpipeEngine::lift_checkButton(const char *varName) {
	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (var)
		return lift_getButtonIdP(var->getSubVarByName(varName)->_value.intValue) > 0;

	return false;
}

void FullpipeEngine::lift_setButtonStatics(Scene *sc, int buttonId) {
	for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++) {
		StaticANIObject *ani = (StaticANIObject *)sc->_staticANIObjectList1[i];

		if (ani->_id == ANI_LIFTBUTTON) {
			int id = lift_getButtonIdN(ani->_statics->_staticsId);

			if (id == buttonId)
				ani->_statics = ani->getStaticsById(id);
		}
	}
}


} // End of namespace Fullpipe
