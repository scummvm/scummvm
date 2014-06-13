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

#include "mortevielle/mortevielle.h"
#include "mortevielle/debugger.h"

namespace Mortevielle {

Debugger::Debugger(MortevielleEngine *vm) : GUI::Debugger() {
	_vm = vm;
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("show_questions", WRAP_METHOD(Debugger, Cmd_showAllQuestions));
	registerCmd("reset_parano", WRAP_METHOD(Debugger, Cmd_resetParano));
}

bool Debugger::Cmd_showAllQuestions(int argc, const char **argv) {
	for (int i = 1; i <= 10; ++i)
		_vm->_coreVar._pctHintFound[i] = '*';

	for (int i = 1; i <= 42; ++i)
		_vm->_coreVar._availableQuestion[i] = '*';

	for (int i = 0; i < 9; i++) {
		_vm->_charAnswerCount[i] = 0;
		_vm->_charAnswerMax[i] = 999;
	}

	return true;
}

bool Debugger::Cmd_resetParano(int argc, const char **argv) {
	_vm->_coreVar._faithScore = 0;

	return true;
}

} // End of namespace Mortevielle
