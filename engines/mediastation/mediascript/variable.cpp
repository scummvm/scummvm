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

#include "mediastation/mediascript/variable.h"
#include "mediastation/chunk.h"
#include "mediastation/datum.h"
#include "mediastation/datafile.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Variable::Variable(Chunk &chunk) {
	id = Datum(chunk, DatumType::UINT16_1).u.i;
	type = Variable::Type(Datum(chunk, DatumType::UINT8).u.i);
	debugC(5, kDebugLoading, "Variable::Variable(): id = 0x%x, type 0x%x (@0x%llx)", id, type, static_cast<long long int>(chunk.pos()));
	switch ((Type)type) {
	case Type::COLLECTION: {
		uint totalItems = Datum(chunk).u.i;
		value.collection = new Common::Array<Variable *>;
		for (uint i = 0; i < totalItems; i++) {
			debugC(7, kDebugLoading, "Variable::Variable(): COLLECTION: Value %d of %d", i, totalItems);
			Variable *variableDeclaration = new Variable(chunk);
			value.collection->push_back(variableDeclaration);
		}
		break;
	}

	case Type::STRING: {
		// TODO: This copies the string. Can we read it directly from the chunk?
		int size = Datum(chunk).u.i;
		char *buffer = new char[size + 1];
		chunk.read(buffer, size);
		buffer[size] = '\0';
		value.string = new Common::String(buffer);
		delete[] buffer;
		debugC(7, kDebugLoading, "Variable::Variable(): STRING: %s", value.string->c_str());
		break;
	}

	case Type::ASSET_ID: {
		value.assetId = Datum(chunk, DatumType::UINT16_1).u.i;
		debugC(7, kDebugLoading, "Variable::Variable(): ASSET ID: %d", value.assetId);
		break;
	}

	case Type::BOOLEAN: {
		uint rawValue = Datum(chunk, DatumType::UINT8).u.i;
		debugC(7, kDebugLoading, " Variable::Variable(): BOOL: %d", rawValue);
		value.b = (rawValue == 1);
		break;
	}

	case Type::LITERAL: {
		// Client code can worry about extracting the value.
		value.datum = new Datum(chunk);
		debugC(7, kDebugLoading, "Variable::Variable(): LITERAL");
		break;
	}

	default: {
		warning("Variable::Variable(): Got unknown variable value type 0x%x", type);
		value.datum = new Datum(chunk);
	}
	}
}

Variable::~Variable() {
	switch ((Type)type) {
	case Type::ASSET_ID:
	case Type::BOOLEAN: {
		break;
	}

	case Type::COLLECTION: {
		delete value.collection;
		break;
	}

	case Type::STRING: {
		delete value.string;
		break;
	}

	case Type::LITERAL: {
		delete value.datum;
		break;
	}

	default: {
		delete value.datum;
		break;
	}
	}
}

} // End of namespace MediaStation
