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

#ifndef AGS_ENGINE_SCRIPT_NON_BLOCKING_SCRIPT_FUNCTION_H
#define AGS_ENGINE_SCRIPT_NON_BLOCKING_SCRIPT_FUNCTION_H

#include "ags/lib/std/vector.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/script/runtime_script_value.h"

namespace AGS3 {

struct NonBlockingScriptFunction {
	const char *functionName;
	int numParameters;
	//void* param1;
	//void* param2;
	RuntimeScriptValue params[2];
	bool roomHasFunction;
	bool globalScriptHasFunction;
	std::vector<bool> moduleHasFunction;
	bool atLeastOneImplementationExists;

	NonBlockingScriptFunction(const char *funcName, int numParams) {
		this->functionName = funcName;
		this->numParameters = numParams;
		atLeastOneImplementationExists = false;
		roomHasFunction = true;
		globalScriptHasFunction = true;
	}
};

} // namespace AGS3

#endif
