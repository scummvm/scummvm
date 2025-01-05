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

#include "mediastation/chunk.h"
#include "mediastation/datafile.h"
#include "mediastation/datum.h"

namespace MediaStation {

enum VariableType {
	// This is an invalid type used for initialization only.
	kVariableTypeEmpty = 0x0000,

	// This is an "array", but the IMT sources
	// use the term "collection".
	kVariableTypeCollection = 0x0007,
	kVariableTypeString = 0x0006,
	kVariableTypeAssetId = 0x0005,
	// These seem to be used in Dalmatians, but I don't know what they are
	// used for.
	kVariableTypeUnk1 = 0x0004,
	// These seem to be constants of some sort? This is what some of these
	// IDs look like in PROFILE._ST:
	//  - $downEar 10026
	//  - $sitDown 10027
	// Seems like these can also reference variables:
	//  - var_6c14_bool_FirstThingLev3 315
	//  - var_6c14_NextEncouragementSound 316
	kVariableTypeUnk2 = 0x0003,
	kVariableTypeBoolean = 0x0002,
	kVariableTypeLiteral = 0x0001
};

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
	~Variable();
};

} // End of namespace MediaStation

#endif