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

void FullpipeEngine::lift_exitSeq(ExCommand *ex) {
	warning("STUB: FullpipeEngine::lift_exitSeq()");
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

void FullpipeEngine::lift_goAnimation() {
	warning("STUB: FullpipeEngine::lift_goAnimation()");
}

void FullpipeEngine::lift_sub1(StaticANIObject *ani) {
	warning("STUB: FullpipeEngine::lift_sub1()");
}

void FullpipeEngine::lift_startExitQueue() {
	warning("STUB: FullpipeEngine::lift_startExitQueue()");
}

void FullpipeEngine::lift_sub05(ExCommand *ex) {
	warning("STUB: FullpipeEngine::lift_sub05()");
}

bool FullpipeEngine::lift_checkButton(const char *varname) {
	warning("STUB: FullpipeEngine::lift_checkButton(%s)", varname);

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
