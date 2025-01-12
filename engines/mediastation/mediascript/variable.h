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

#ifndef MEDIASTATION_MEDIASCRIPT_VARIABLE_DECLARATION_H
#define MEDIASTATION_MEDIASCRIPT_VARIABLE_DECLARATION_H

#include "common/str.h"
#include "common/array.h"

#include "mediastation/chunk.h"
#include "mediastation/datum.h"
#include "mediastation/mediascript/builtins.h"

namespace MediaStation {

class Operand;

class Variable {
public:
	uint32 _id = 0;
	VariableType _type = kVariableTypeEmpty;
	union {
		Datum *datum = nullptr;
		Common::String *string;
		Common::Array<Variable *> *collection;
		bool b;
		int i;
		double d;
		uint assetId;
	} _value;

	Variable();
	Variable(Chunk &chunk, bool readId = true);

	Operand getValue();
	Operand callMethod(BuiltInMethod method, Common::Array<Operand> &args);
	~Variable();
};

} // End of namespace MediaStation

#endif