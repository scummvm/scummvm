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

#ifndef MTROPOLIS_COROUTINE_MANAGER_H
#define MTROPOLIS_COROUTINE_MANAGER_H

#include "mtropolis/coroutine_protos.h"

namespace MTropolis {

struct CoroutineStackFrame2;
struct CompiledCoroutine;
struct ICoroutineCompiler;
struct CoroutineParamsBase;

struct ICoroutineManager {
	virtual ~ICoroutineManager();

	virtual void registerCoroutine(CompiledCoroutine **compiledCoroPtr) = 0;
	virtual void compileCoroutine(CompiledCoroutine **compiledCoroPtr, CoroutineCompileFunction_t compileFunction, bool isVoidReturn) = 0;

	static ICoroutineManager *create();
};

} // End of namespace MTropolis

#endif
