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

#include "twp/sqgame.h"

namespace Twp {

void regFunc(HSQUIRRELVM v, SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck, const SQChar *typemask) {
	sq_pushroottable(v);
	sq_pushstring(v, functionName, -1);
	sq_newclosure(v, f, 0); // create a new function
	sq_setparamscheck(v, nparamscheck, typemask);
	sq_setnativeclosurename(v, -1, functionName);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1); // pops the root table
}

}
