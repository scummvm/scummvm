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

namespace Fullpipe {

int FullpipeEngine::lift_getButtonIdP(int objid) {
	switch (objid) {
	case ST_LBN_0N:
		return ST_LBN_0P;
		break;
	case ST_LBN_1N:
		return ST_LBN_1P;
		break;
	case ST_LBN_2N:
		return ST_LBN_2P;
		break;
	case ST_LBN_3N:
		return ST_LBN_3P;
		break;
	case ST_LBN_4N:
		return ST_LBN_4P;
		break;
	case ST_LBN_5N:
		return ST_LBN_5P;
		break;
	case ST_LBN_6N:
		return ST_LBN_6P;
		break;
	case ST_LBN_7N:
		return ST_LBN_7P;
		break;
	case ST_LBN_8N:
		return ST_LBN_8P;
		break;
	case ST_LBN_9N:
		return ST_LBN_9P;
		break;
	default:
		return 0;
		break;
	}
}

void FullpipeEngine::lift_setButton(const char *name, int state) {
	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (var)
		var->setSubVarAsInt(name, state);
}

void FullpipeEngine::lift_sub5(Scene *sc, int qu1, int qu2) {
	warning("STUB: FullpipeEngine::lift_sub5()");
}

void FullpipeEngine::lift_exitSeq(ExCommand *ex) {
	warning("STUB: FullpipeEngine::lift_exitSeq()");
}

void FullpipeEngine::lift_closedoorSeq() {
	warning("STUB: FullpipeEngine::lift_closedoorSeq()");
}

void FullpipeEngine::lift_animation3() {
	warning("STUB: FullpipeEngine::lift_animation3()");
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

void FullpipeEngine::lift_sub7(Scene *sc, int buttonId) {
	warning("STUB: lift_sub7()");
}


} // End of namespace Fullpipe
