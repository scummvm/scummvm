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

class Variable {
public:
	enum class Type {
		// This is an invalid type used for initialization only.
		EMPTY = 0x0000,

		// This is an "array", but the IMT sources
		// use the term "collection".
		COLLECTION = 0x0007,
		STRING = 0x0006,
		ASSET_ID = 0x0005,
		// These seem to be used in Dalmatians, but I don't know what they are
		// used for.
		UNK1 = 0x0004,
		// These seem to be constants of some sort? This is what some of these
		// IDs look like in PROFILE._ST:
		//  - $downEar 10026
		//  - $sitDown 10027
		// Seems like these can also reference variables:
		//  - var_6c14_bool_FirstThingLev3 315
		//  - var_6c14_NextEncouragementSound 316
		UNK2 = 0x0003,
		BOOLEAN = 0x0002,
		LITERAL = 0x0001
	};

	uint32 id = 0;
	Variable::Type type = Type::EMPTY;
	union {
		Datum *datum = nullptr;
		Common::String *string;
		Common::Array<Variable *> *collection;
		bool b;
		int i;
		double d;
		uint assetId;
	} value;

	Variable();
	Variable(Chunk &chunk);
	~Variable();
};

} // End of namespace MediaStation

#endif