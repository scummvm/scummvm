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

#include "common/formats/winexe_pe.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/ai_main.h"
#include "scumm/he/moonbase/map_main.h"

namespace Scumm {

Moonbase::Moonbase(ScummEngine_v100he *vm) : _vm(vm) {
	_exe = new Common::PEResources();

	initFOW();

	_ai = new AI(_vm);
	_map = new Map(_vm);
}

Moonbase::~Moonbase() {
	delete _exe;
	delete _ai;
	delete _map;
}

int Moonbase::readFromArray(int array, int y, int x) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = array;

	return _vm->readArray(_vm->VAR_U32_ARRAY_UNK, y, x);
}

void Moonbase::deallocateArray(int array) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = array;

	return _vm->nukeArray(_vm->VAR_U32_ARRAY_UNK);
}

int Moonbase::callScummFunction(int scriptNumber, int paramCount,...) {
	va_list va_params;
	va_start(va_params, paramCount);
	int args[25];

	memset(args, 0, sizeof(args));

	Common::String str;
	str = Common::String::format("Moonbase::callScummFunction(%d, [", scriptNumber);

	for (int i = 0; i < paramCount; i++) {
		args[i] = va_arg(va_params, int);

		str += Common::String::format("%d ", args[i]);
	}
	str += "])";

	debug(3, "%s", str.c_str());


	va_end(va_params);

	_vm->runScript(scriptNumber, 0, 1, args);

	return _vm->pop();
}

} // End of namespace Scumm
