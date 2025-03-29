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

#include "common/ptr.h"
#include "common/str.h"
#include "common/array.h"

#include "mediastation/datafile.h"
#include "mediastation/datum.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class Operand;

class Collection : public Common::Array<Operand> {
public:
	Operand callMethod(BuiltInMethod method, Common::Array<Operand> &args);
};

class Variable {
public:
	uint32 _id = 0;
	VariableType _type = kVariableTypeEmpty;
	union {
		Common::String *string;
		uint functionId;
		int i;
		double d;
		uint assetId;
	} _value;
	Common::SharedPtr<Collection> _c;

	Variable();
	Variable(Chunk &chunk, bool readId = true);
	~Variable();

	Operand getValue();
	void putValue(Operand value);

private:
	void clear();
};

} // End of namespace MediaStation

#endif