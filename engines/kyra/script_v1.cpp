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
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v1.h"
#include "kyra/screen.h"

namespace Kyra {

int KyraEngine_v1::o1_queryGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_queryGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return queryGameFlag(stackPos(0));
}

int KyraEngine_v1::o1_setGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_setGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return setGameFlag(stackPos(0));
}

int KyraEngine_v1::o1_resetGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_resetGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return resetGameFlag(stackPos(0));
}

int KyraEngine_v1::o1_getRand(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_getRand(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < stackPos(1));
	return _rnd.getRandomNumberRng(stackPos(0), stackPos(1));
}

int KyraEngine_v1::o1_hideMouse(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_hideMouse(%p) ()", (const void *)script);
	screen()->hideMouse();
	return 0;
}

int KyraEngine_v1::o1_showMouse(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_showMouse(%p) ()", (const void *)script);
	screen()->showMouse();
	return 0;
}

int KyraEngine_v1::o1_setMousePos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_setMousePos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_system->warpMouse(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v1::o1_setHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_setHandItem(%p) (%d)", (const void *)script, stackPos(0));
	setHandItem(stackPos(0));
	return 0;
}

int KyraEngine_v1::o1_removeHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_removeHandItem(%p) ()", (const void *)script);
	removeHandItem();
	return 0;
}

int KyraEngine_v1::o1_getMouseState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_getMouseState(%p) ()", (const void *)script);
	return _mouseState;
}

} // end of namespace Kyra

