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
#include "mediastation/mediascript/operand.h"

namespace MediaStation {

Variable::Variable(Chunk &chunk, bool readId) {
	if (readId) {
		_id = Datum(chunk).u.i;
	}
	_type = VariableType(Datum(chunk).u.i);
	debugC(5, kDebugLoading, "Variable::Variable(): id = 0x%x, type %s (%d) (@0x%llx)", 
		_id, variableTypeToStr(_type), static_cast<uint>(_type), static_cast<long long int>(chunk.pos()));
	switch ((VariableType)_type) {
	case kVariableTypeCollection: {
		uint totalItems = Datum(chunk).u.i;
		_value.collection = new Common::Array<Variable *>;
		for (uint i = 0; i < totalItems; i++) {
			debugC(7, kDebugLoading, "Variable::Variable(): %s: Value %d of %d", variableTypeToStr(_type), i, totalItems);
			Variable *variableDeclaration = new Variable(chunk, readId = false);
			_value.collection->push_back(variableDeclaration);
		}
		break;
	}

	case kVariableTypeString: {
		// TODO: This copies the string. Can we read it directly from the chunk?
		int size = Datum(chunk).u.i;
		char *buffer = new char[size + 1];
		chunk.read(buffer, size);
		buffer[size] = '\0';
		_value.string = new Common::String(buffer);
		delete[] buffer;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %s", variableTypeToStr(_type), _value.string->c_str());
		break;
	}

	case kVariableTypeAssetId: {
		_value.assetId = Datum(chunk, kDatumTypeUint16_1).u.i;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %d", variableTypeToStr(_type), _value.assetId);
		break;
	}

	case kVariableTypeBoolean: {
		uint rawValue = Datum(chunk, kDatumTypeUint8).u.i;
		debugC(7, kDebugLoading, " Variable::Variable(): %s: %d", variableTypeToStr(_type), rawValue);
		_value.b = (rawValue == 1);
		break;
	}

	case kVariableTypeFloat: {
		Datum datum = Datum(chunk);
		if ((datum.t != kDatumTypeFloat64_1) && (datum.t != kDatumTypeFloat64_2)) {
			error("Variable::Variable(): Got a non-float datum type 0x%x to put into a float variable", datum.t);
		}
		_value.d = datum.u.f;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %f", variableTypeToStr(_type), _value.d);
		break;
	}

	case kVariableTypeInt: {
		_value.i = Datum(chunk).u.i;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %d", variableTypeToStr(_type), _value.i);
		break;
	}

	default: {
		error("Variable::Variable(): Got unknown variable value type %s (%d)", variableTypeToStr(_type), static_cast<uint>(_type));
	}
	}
}

Variable::~Variable() {
	switch (_type) {
	case kVariableTypeCollection: {
		for (Variable *variable : *(_value.collection)) {
			delete variable;
		}
		delete _value.collection;
		break;
	}

	case kVariableTypeString: {
		delete _value.string;
		break;
	}

	default: {
		break;
	}
	}
}

Operand Variable::getValue() {	
	switch (_type) {
	case kVariableTypeEmpty: {
		error("Variable::getValue(): Attempt to get value from an empty variable");
	}

	case kVariableTypeCollection: {
		// TODO: Determine if any scripts actually try to do this.
		error("Variable::getValue(): Returning a collection is not implemented");
		break;
	}

	case kVariableTypeString: {
		Operand returnValue(kOperandTypeString);
		returnValue.putString(_value.string);
		return returnValue;
	}

	case kVariableTypeAssetId: {
		Operand returnValue(kOperandTypeAssetId);
		returnValue.putAsset(_value.assetId);
		return returnValue;
	}

	case kVariableTypeBoolean: {
		// TODO: Is this value type correct?
		// Shouldn't matter too much, though, since it's still an integer type.
		Operand returnValue(kOperandTypeLiteral1);
		returnValue.putInteger(_value.b);
		return returnValue;
	}

	case kVariableTypeFloat: {
		// TODO: Is this value type correct?
		// Shouldn't matter too much, though, since it's still a floating-point type.
		Operand returnValue(kOperandTypeFloat1);
		returnValue.putDouble(_value.d);
		return returnValue;
	}

	default: {
		error("Variable::getValue(): Attempt to get value from unknown variable type %s (%d)", variableTypeToStr(_type), static_cast<uint>(_type));
	}
	}
}

void Variable::putValue(Operand value) {
	switch (value.getType()) {
	case kOperandTypeEmpty: {
		error("Variable::putValue(): Assigning an empty operand to a variable not supported");
	}

	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
	case kOperandTypeDollarSignVariable: {
		_type = kVariableTypeInt;
		_value.i = value.getInteger();
		break;
	}

	case kOperandTypeFloat1:
	case kOperandTypeFloat2: {
		_type = kVariableTypeFloat;
		_value.d = value.getDouble();
		break;
	}

	case kOperandTypeString: {
		_type = kVariableTypeString;
		_value.string = value.getString();
		break;
	}

	case kOperandTypeAssetId: {
		_type = kVariableTypeAssetId;
		_value.assetId = value.getAssetId();
		break;
	}

	case kOperandTypeVariableDeclaration: {
		putValue(value.getLiteralValue());
		break;
	}

	default: {
		error("Variable::putValue(): Assigning an unknown operand type %s (%d) to a variable not supported",
			operandTypeToStr(value.getType()), static_cast<uint>(value.getType()));
	}
	}
}

Operand Variable::callMethod(BuiltInMethod method, Common::Array<Operand> &args) {
	switch (_type) {
	case kVariableTypeCollection: {
		// TODO: This is just a warning for now so we can get past the
		// IBM/Crayola opening screen.
		warning("Variable::callMethod(): Calling method on a collection not implemented yet");
		return Operand();
		break;
	}

	default: {
		error("Variable::callMethod(): Calling method on unknown variable type %s (%d)", variableTypeToStr(_type), static_cast<uint>(_type));
	}
	}
}

} // End of namespace MediaStation
