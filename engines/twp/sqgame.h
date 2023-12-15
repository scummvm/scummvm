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

#ifndef TWP_SQGAME_H
#define TWP_SQGAME_H

#include <stddef.h>
#include "twp/squirrel/squirrel.h"

namespace Twp {

void regFunc(HSQUIRRELVM v, SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck = 0, const SQChar *typemask = NULL);
void sqgame_register_syslib(HSQUIRRELVM v);
void sqgame_register_objlib(HSQUIRRELVM v);
void sqgame_register_genlib(HSQUIRRELVM v);
void sqgame_register_actorlib(HSQUIRRELVM v);

} // namespace Twp
#endif
