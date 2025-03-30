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

#include "mediastation/mediastation.h"
#include "mediastation/mediascript/variable.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/codechunk.h"
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

ScriptValue Collection::callMethod(BuiltInMethod method, Common::Array<ScriptValue> &args) {
	switch (method) {
	case kIsEmptyMethod: {
		ScriptValue returnValue(kOperandTypeBool);
		returnValue.setToParamToken(static_cast<uint>(empty()));
		return returnValue;
	}

	case kAppendMethod: {
		for (ScriptValue arg : args) {
			push_back(arg);
		}
		return ScriptValue();
	}

	case kDeleteFirstMethod: {
		ScriptValue returnValue = remove_at(0);
		return returnValue;
	}

	case kDeleteAtMethod: {
		// Find the item in the collection, then remove and return it.
		assert(args.size() == 1);
		for (uint i = 0; i < size(); i++) {
			if (args[0] == operator[](i)) {
				ScriptValue returnValue = remove_at(i);
				return returnValue;
			}
		}

		// The item wasn't found.
		return ScriptValue();
	}

	case kCountMethod: {
		ScriptValue returnValue = ScriptValue(kOperandTypeBool);
		returnValue.setToParamToken(size());
		return returnValue;
	}

	case kGetAtMethod: {
		assert(args.size() == 1);
		ScriptValue returnValue = operator[](args[0].asParamToken());
		return returnValue;
	}

	case kSendMethod: {
		// Call a method on each item in the collection.
		BuiltInMethod methodToSend = static_cast<BuiltInMethod>(args[0].asMethodId());
		Common::Array<ScriptValue> sendArgs;
		for (uint i = 0; i < size(); i++) {
			ScriptValue self = operator[](i);
			CodeChunk::callBuiltInMethod(methodToSend, self, sendArgs);
		}
		return ScriptValue();
	}

	case kSeekMethod: {
		// Find the item in the collection if it exists.
		assert(args.size() == 1);
		for (uint i = 0; i < size(); i++) {
			if (args[0] == operator[](i)) {
				return operator[](i);
			}
		}

		// The item wasn't found.
		ScriptValue returnValue(kOperandTypeBool);
		returnValue.setToParamToken(-1);
		return returnValue;
	}

	case kJumbleMethod: {
		// Scramble the items in the collection.
		for (uint i = size() - 1; i > 0; --i) {
			uint j = g_engine->_randomSource.getRandomNumber(size() - 1);
			SWAP(operator[](i), operator[](j));
		}
		return ScriptValue();
	}

	case kSortMethod: {
		assert(args.empty());
		Common::sort(begin(), end());
		return ScriptValue();
	}

	case kEmptyMethod: {
		clear();
		return ScriptValue();
	}

	default:
		error("Collection::callMethod(): Attempt to call unimplemented method %s (%d)", builtInMethodToStr(method), static_cast<uint>(method));
	}
}

Variable::Variable(Chunk &chunk, bool readId) {
	if (readId) {
		_id = Datum(chunk).u.i;
	}

	_type = static_cast<ScriptValueType>(Datum(chunk).u.i);
	debugC(7, kDebugScript, "Variable::Variable(): id = %d, type %s (%d) (@0x%llx)",
		_id, scriptValueTypeToStr(_type), static_cast<uint>(_type), static_cast<long long int>(chunk.pos()));
	switch (_type) {
	case kScriptValueTypeCollection: {
		uint totalItems = Datum(chunk).u.i;
		_c = Common::SharedPtr<Collection>(new Collection);
		for (uint i = 0; i < totalItems; i++) {
			debugC(7, kDebugLoading, "Variable::Variable(): %s: Value %d of %d", scriptValueTypeToStr(_type), i, totalItems);
			Variable variable = Variable(chunk, readId = false);
			_c->push_back(variable.getValue());
		}
		break;
	}

	case kScriptValueTypeString: {
		// TODO: This copies the string. Can we read it directly from the chunk?
		int size = Datum(chunk).u.i;
		char *buffer = new char[size + 1];
		chunk.read(buffer, size);
		buffer[size] = '\0';
		_value.string = new Common::String(buffer);
		delete[] buffer;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %s", scriptValueTypeToStr(_type), _value.string->c_str());
		break;
	}

	case kScriptValueTypeAssetId: {
		_value.assetId = Datum(chunk, kDatumTypeUint16_1).u.i;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %d", scriptValueTypeToStr(_type), _value.assetId);
		break;
	}

	case kScriptValueTypeBool: {
		uint rawValue = Datum(chunk, kDatumTypeUint8).u.i;
		debugC(7, kDebugLoading, " Variable::Variable(): %s: %d", scriptValueTypeToStr(_type), rawValue);
		_value.i = static_cast<int>(rawValue == 1);
		break;
	}

	case kScriptValueTypeFloat: {
		Datum datum = Datum(chunk);
		if ((datum.t != kDatumTypeFloat64_1) && (datum.t != kDatumTypeFloat64_2)) {
			error("Variable::Variable(): Got a non-float datum type 0x%x to put into a float variable", datum.t);
		}
		_value.d = datum.u.f;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %f", scriptValueTypeToStr(_type), _value.d);
		break;
	}

	case kScriptValueTypeParamToken: {
		_value.i = Datum(chunk).u.i;
		debugC(7, kDebugLoading, "Variable::Variable(): %s: %d", scriptValueTypeToStr(_type), _value.i);
		break;
	}

	default:
		error("Variable::Variable(): Got unknown variable value type %s (%d)", scriptValueTypeToStr(_type), static_cast<uint>(_type));
	}
}

Variable::~Variable() {
	clear();
}

ScriptValue Variable::getValue() {
	switch (_type) {
	case kScriptValueTypeEmpty: {
		error("Variable::getValue(): Attempt to get value from an empty variable");
	}

	case kScriptValueTypeCollection: {
		ScriptValue returnValue(kOperandTypeCollection);
		returnValue.setToCollection(_c);
		return returnValue;
	}

	case kScriptValueTypeString: {
		ScriptValue returnValue(kOperandTypeString);
		returnValue.setToString(_value.string);
		return returnValue;
	}

	case kScriptValueTypeAssetId: {
		ScriptValue returnValue(kOperandTypeAssetId);
		returnValue.setToAssetId(_value.assetId);
		return returnValue;
	}

	case kScriptValueTypeBool: {
		// TODO: Is this value type correct?
		// Shouldn't matter too much, though, since it's still an integer type.
		ScriptValue returnValue(kOperandTypeBool);
		returnValue.setToParamToken(_value.i);
		return returnValue;
	}

	case kScriptValueTypeParamToken: {
		// TODO: Is this value type correct?
		// Shouldn't matter too much, though, since it's still an integer type.
		ScriptValue returnValue(kOperandTypeBool);
		returnValue.setToParamToken(_value.i);
		return returnValue;
	}

	case kScriptValueTypeFloat: {
		// TODO: Is this value type correct?
		// Shouldn't matter too much, though, since it's still a floating-point type.
		ScriptValue returnValue(kOperandTypeTime);
		returnValue.setToFloat(_value.d);
		return returnValue;
	}

	default:
		error("Variable::getValue(): Attempt to get value from unknown variable type %s (%d)", scriptValueTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Variable::putValue(ScriptValue value) {
	clear();

	switch (value.getType()) {
	case kOperandTypeEmpty: {
		error("Variable::putValue(): Assigning an empty ScriptValue to a variable not supported");
	}

	case kOperandTypeBool:
	case kOperandTypeInt:
	case kOperandTypeParamToken: {
		_type = kScriptValueTypeParamToken;
		_value.i = value.asParamToken();
		break;
	}

	case kOperandTypeTime:
	case kOperandTypeFloat: {
		_type = kScriptValueTypeFloat;
		_value.d = value.asFloat();
		break;
	}

	case kOperandTypeString: {
		_type = kScriptValueTypeString;
		_value.string = value.asString();
		break;
	}

	case kOperandTypeAssetId: {
		_type = kScriptValueTypeAssetId;
		_value.assetId = value.asAssetId();
		break;
	}

	case kOperandTypeVariable: {
		putValue(value.getLiteralValue());
		break;
	}

	case kOperandTypeFunctionId: {
		_type = kScriptValueTypeFunctionId;
		_value.functionId = value.asFunctionId();
		break;
	}

	case kOperandTypeCollection: {
		_type = kScriptValueTypeCollection;
		_c = value.asCollection();
		break;
	}

	default:
		error("Variable::putValue(): Assigning an unknown ScriptValue type %s (%d) to a variable not supported",
			operandTypeToStr(value.getType()), static_cast<uint>(value.getType()));
	}
}

void Variable::clear() {
	switch (_type) {
	case kScriptValueTypeCollection: {
		_c.reset();
		break;
	}

	case kScriptValueTypeString: {
		delete _value.string;
		_value.string = nullptr;
		break;
	}

	default:
		break;
	}

	_type = kScriptValueTypeEmpty;
}

} // End of namespace MediaStation
