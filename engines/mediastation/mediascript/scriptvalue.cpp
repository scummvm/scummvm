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

#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/function.h"
#include "mediastation/mediastation.h"

#define VALUETYPEMISMATCH(expectedType) warning("%s: Script value type mismatch: Expected %s, got %s", __func__, scriptValueTypeToStr(expectedType), scriptValueTypeToStr(_type))

namespace MediaStation {

ScriptValue::ScriptValue(ParameterReadStream *stream) {
	_type = static_cast<ScriptValueType>(stream->readTypedByte());

	switch (_type) {
	case kScriptValueTypeEmpty:
		break;

	case kScriptValueTypeFloat: {
		double d = stream->readTypedDouble();
		setToFloat(d);
		break;
	}

	case kScriptValueTypeBool: {
		uint rawValue = stream->readTypedByte();
		if (rawValue != 0 && rawValue != 1) {
			error("%s: Got invalid literal bool value %d", __func__, rawValue);
		}
		setToBool(rawValue);
		break;
	}

	case kScriptValueTypeTime: {
		double d = stream->readTypedTime();
		setToFloat(d);
		break;
	}

	case kScriptValueTypeParamToken: {
		uint paramToken = stream->readTypedUint16();
		setToParamToken(paramToken);
		break;
	}

	case kScriptValueTypeActorId: {
		uint actorId = stream->readTypedUint16();
		setToActorId(actorId);
		break;
	}

	case kScriptValueTypeString: {
		uint size = stream->readTypedUint16();
		Common::String string = stream->readString('\0', size);
		setToString(string);
		break;
	}

	case kScriptValueTypeCollection: {
		uint totalItems = stream->readTypedUint16();
		Collection *collection = new Collection;
		for (uint i = 0; i < totalItems; i++) {
			ScriptValue collectionValue = ScriptValue(stream);
			collection->push_back(collectionValue);
		}
		setToCollection(collection);
		break;
	}

	case kScriptValueTypeFunctionId: {
		uint functionId = stream->readTypedUint16();
		setToFunctionId(functionId);
		break;
	}

	case kScriptValueTypeMethodId: {
		BuiltInMethod methodId = static_cast<BuiltInMethod>(stream->readTypedUint16());
		setToMethodId(methodId);
		break;
	}

	default:
		error("%s: Got unknown script value type %s", __func__, scriptValueTypeToStr(_type));
	}
}

void ScriptValue::clearCollection() {
	if (_collection) {
		delete _collection;
		_collection = nullptr;
	}
}

ScriptValue::~ScriptValue() {
	clearCollection();
}

ScriptValue::ScriptValue(const ScriptValue &other) {
	clearCollection();
	copyFrom(other);
}

void ScriptValue::operator=(const ScriptValue &other) {
	clearCollection();
	copyFrom(other);
}

void ScriptValue::copyFrom(const ScriptValue &other) {
	_type = other._type;

	switch (_type) {
	case kScriptValueTypeEmpty:
		// Nothing to copy for empty type.
		break;

	case kScriptValueTypeFloat:
		_u.d = other._u.d;
		break;

	case kScriptValueTypeBool:
		_u.b = other._u.b;
		break;

	case kScriptValueTypeTime:
		_u.d = other._u.d;
		break;

	case kScriptValueTypeParamToken:
		_u.paramToken = other._u.paramToken;
		break;

	case kScriptValueTypeActorId:
		_u.actorId = other._u.actorId;
		break;

	case kScriptValueTypeString:
		_string = other._string;
		break;

	case kScriptValueTypeCollection:
		if (other._collection) {
			// We always need a deep copy.
			_collection = new Collection(*other._collection);
		}
		break;

	case kScriptValueTypeFunctionId:
		_u.functionId = other._u.functionId;
		break;

	case kScriptValueTypeMethodId:
		_u.methodId = other._u.methodId;
		break;

	default:
		error("%s: Got unknown script value type %s", __func__, scriptValueTypeToStr(_type));
	}
}

void ScriptValue::setToFloat(uint i) {
	setToFloat(static_cast<double>(i));
}

void ScriptValue::setToFloat(int i) {
	setToFloat(static_cast<double>(i));
}

void ScriptValue::setToFloat(double d) {
	_type = kScriptValueTypeFloat;
	_u.d = d;
}

double ScriptValue::asFloat() const {
	if (_type == kScriptValueTypeFloat) {
		return _u.d;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeFloat);
		return 0.0;
	}
}

double ScriptValue::asFloatOrTime() const {
	if (_type == kScriptValueTypeFloat || _type == kScriptValueTypeTime) {
		return _u.d;
	} else {
		// Times usually appear as floats (not the other way around), so
		// for the one log we will use the time type.
		VALUETYPEMISMATCH(kScriptValueTypeTime);
		return 0.0;
	}
}

void ScriptValue::setToBool(bool b) {
	_type = kScriptValueTypeBool;
	_u.b = b;
}

bool ScriptValue::asBool() const {
	if (_type == kScriptValueTypeBool) {
		return _u.b;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeBool);
		return false;
	}
}

void ScriptValue::setToTime(double d) {
	_type = kScriptValueTypeTime;
	_u.d = d;
}

double ScriptValue::asTime() const {
	if (_type == kScriptValueTypeTime) {
		return _u.d;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeTime);
		return 0.0;
	}
}

void ScriptValue::setToParamToken(uint paramToken) {
	_type = kScriptValueTypeParamToken;
	_u.paramToken = paramToken;
}

uint ScriptValue::asParamToken() const {
	if (_type == kScriptValueTypeParamToken) {
		return _u.paramToken;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeParamToken);
		return 0;
	}
}

void ScriptValue::setToActorId(uint actorId) {
	_type = kScriptValueTypeActorId;
	_u.actorId = actorId;
}

uint ScriptValue::asActorId() const {
	if (_type == kScriptValueTypeActorId) {
		return _u.actorId;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeActorId);
		return 0;
	}
}

void ScriptValue::setToString(const Common::String &string) {
	_type = kScriptValueTypeString;
	_string = string;
}

Common::String ScriptValue::asString() const {
	if (_type == kScriptValueTypeString) {
		return _string;
	} else {
		return Common::String("");
	}
}

void ScriptValue::setToCollection(Collection *collection) {
	_type = kScriptValueTypeCollection;
	clearCollection();
	_collection = collection;
}

Collection *ScriptValue::asCollection() const {
	if (_type == kScriptValueTypeCollection) {
		return _collection;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeCollection);
		return nullptr;
	}
}

void ScriptValue::setToFunctionId(uint functionId) {
	_type = kScriptValueTypeFunctionId;
	_u.functionId = functionId;
}

uint ScriptValue::asFunctionId() const {
	if (_type == kScriptValueTypeFunctionId) {
		return _u.functionId;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeFunctionId);
		return 0;
	}
}

void ScriptValue::setToMethodId(BuiltInMethod methodId) {
	_type = kScriptValueTypeMethodId;
	_u.methodId = methodId;
}

BuiltInMethod ScriptValue::asMethodId() const {
	if (_type == kScriptValueTypeMethodId) {
		return _u.methodId;
	} else {
		VALUETYPEMISMATCH(kScriptValueTypeMethodId);
		return kInvalidMethod;
	}
}

Common::String ScriptValue::getDebugString() const {
	switch (getType()) {
	case kScriptValueTypeEmpty:
		return "empty";

	case kScriptValueTypeBool:
		return Common::String::format("bool: %s", asBool() ? "TRUE" : "FALSE");

	case kScriptValueTypeFloat:
		return Common::String::format("float: %f", asFloat());

	case kScriptValueTypeActorId: {
		Common::String actorName = g_engine->formatActorName(asActorId(), true);
		return Common::String::format("actor: %s", actorName.c_str());
	}

	case kScriptValueTypeTime:
		return Common::String::format("time: %f", asTime());

	case kScriptValueTypeParamToken: {
		Common::String tokenName = g_engine->formatParamTokenName(asParamToken());
		return Common::String::format("token: %s", tokenName.c_str());
	}

	case kScriptValueTypeString:
		return Common::String::format("string: \"%s\"", asString().c_str());

	case kScriptValueTypeCollection: {
		Collection *collection = asCollection();
		uint itemCount = collection ? collection->size() : 0;
		return Common::String::format("collection: [%d items]", itemCount);
	}

	case kScriptValueTypeFunctionId: {
		Common::String functionName = g_engine->formatFunctionName(asFunctionId());
		return Common::String::format("function: %s", functionName.c_str());
	}

	case kScriptValueTypeMethodId: {
		return Common::String::format("method: %s", builtInMethodToStr(asMethodId()));
	}

	default:
		return Common::String::format("arg type %s", scriptValueTypeToStr(getType()));
	}
}

bool ScriptValue::compare(Opcode op, const ScriptValue &lhs, const ScriptValue &rhs) {
	if (lhs.getType() != rhs.getType()) {
		warning("%s: Attempt to compare mismatched values: %s; %s",
			__func__, lhs.getDebugString().c_str(), rhs.getDebugString().c_str());
	}

	bool result = false;
	switch (lhs.getType()) {
	case kScriptValueTypeEmpty:
		result = compareEmptyValues(op);
		break;

	case kScriptValueTypeFloat:
		result = compare(op, lhs.asFloatOrTime(), rhs.asFloatOrTime());
		break;

	case kScriptValueTypeBool:
		result = compare(op, lhs.asBool(), rhs.asBool());
		break;

	case kScriptValueTypeTime:
		result = compare(op, lhs.asFloatOrTime(), rhs.asFloatOrTime());
		break;

	case kScriptValueTypeParamToken:
		result = compare(op, lhs.asParamToken(), rhs.asParamToken());
		break;

	case kScriptValueTypeActorId:
		result = compare(op, lhs.asActorId(), rhs.asActorId());
		break;

	case kScriptValueTypeString:
		result = compareStrings(op, lhs.asString(), rhs.asString());
		break;

	case kScriptValueTypeCollection:
		result = compare(op, lhs.asCollection(), rhs.asCollection());
		break;

	case kScriptValueTypeFunctionId:
		result = compare(op, lhs.asFunctionId(), rhs.asFunctionId());
		break;

	case kScriptValueTypeMethodId:
		result = compare(op, static_cast<uint>(lhs.asMethodId()), static_cast<uint>(rhs.asMethodId()));
		break;

	default:
		error("%s: Got unknown script value type %d", __func__, lhs.getType());
	}
	return result;
}

bool ScriptValue::compareEmptyValues(Opcode op) {
	// Empty values are considered equal.
	switch (op) {
	case kOpcodeEquals:
		return true;

	case kOpcodeNotEquals:
		return false;

	default:
		warning("%s: Got invalid empty value operation %s", __func__, opcodeToStr(op));
		return false;
	}
}

bool ScriptValue::compareStrings(Opcode op, const Common::String &left, const Common::String &right) {
	switch (op) {
	case kOpcodeEquals:
		return (left == right);

	case kOpcodeNotEquals:
		return (left != right);

	case kOpcodeLessThan:
		return (left < right);

	case kOpcodeGreaterThan:
		return (left > right);

	case kOpcodeLessThanOrEqualTo:
		return (left <= right);

	case kOpcodeGreaterThanOrEqualTo:
		return (left >= right);

	default:
		error("%s: Got invalid string operation %s", __func__, opcodeToStr(op));
	}
}

bool ScriptValue::compare(Opcode op, uint left, uint right) {
	switch (op) {
	case kOpcodeEquals:
		return (left == right);

	case kOpcodeNotEquals:
		return (left != right);

	default:
		error("%s: Got invalid param token operation %s", __func__, opcodeToStr(op));
	}
}

bool ScriptValue::compare(Opcode op, bool left, bool right) {
	switch (op) {
	case kOpcodeEquals:
		return (left == right);

	case kOpcodeNotEquals:
		return (left != right);

	default:
		error("%s: Got invalid bool operation %s", __func__, opcodeToStr(op));
	}
}

bool ScriptValue::compare(Opcode op, double left, double right) {
	switch (op) {
	case kOpcodeEquals:
		return (left == right);

	case kOpcodeNotEquals:
		return (left != right);

	case kOpcodeLessThan:
		return (left < right);

	case kOpcodeGreaterThan:
		return (left > right);

	case kOpcodeLessThanOrEqualTo:
		return (left <= right);

	case kOpcodeGreaterThanOrEqualTo:
		return (left >= right);

	default:
		error("%s: Got invalid float operation %s", __func__, opcodeToStr(op));
	}
}

bool ScriptValue::compare(Opcode op, Collection *left, Collection *right) {
	switch (op) {
	case kOpcodeEquals:
		return (left == right);

	case kOpcodeNotEquals:
		return (left != right);

	default:
		error("%s: Got invalid collection operation %s", __func__, opcodeToStr(op));
	}
}

ScriptValue ScriptValue::evalMathOperation(Opcode op, const ScriptValue &left, const ScriptValue &right) {
	ScriptValue returnValue;
	double result = 0.0;

	switch (left.getType()) {
	case kScriptValueTypeFloat: {
		if (right.getType() == kScriptValueTypeTime) {
			result = binaryMathOperation(op, left.asFloat(), right.asTime());
		} else if (right.getType() == kScriptValueTypeFloat) {
			result = binaryMathOperation(op, left.asFloat(), right.asFloat());
		} else {
			error("%s: Attempted to do math operation on unsupported value type %s", __func__, scriptValueTypeToStr(right.getType()));
		}
		returnValue.setToFloat(result);
		break;
	}

	case kScriptValueTypeTime: {
		if (right.getType() == kScriptValueTypeTime) {
			result = binaryMathOperation(op, left.asTime(), right.asTime());
		} else if (right.getType() == kScriptValueTypeFloat) {
			result = binaryMathOperation(op, left.asTime(), right.asFloat());
		} else {
			error("%s: Attempted to do math operation on unsupported value type %s", __func__, scriptValueTypeToStr(right.getType()));
		}
		returnValue.setToFloat(result);
		break;
	}

	case kScriptValueTypeString: {
		returnValue.setToString(left.asString() + right.asString());
		break;
	}

	default:
		error("%s: Attempted to do math operation on unsupported value type %s", __func__, scriptValueTypeToStr(right.getType()));
	}

	return returnValue;
}

double ScriptValue::binaryMathOperation(Opcode op, double left, double right) {
	switch (op) {
	case kOpcodeAdd:
		return left + right;

	case kOpcodeSubtract:
		return left - right;

	case kOpcodeMultiply:
		return left * right;

	case kOpcodeDivide:
		if (right != 0.0) {
			return left / right;
		} else {
			error("%s: Division by zero", __func__);
		}

	case kOpcodeModulo:
		if (right != 0.0) {
			return fmod(left, right);
		} else {
			error("%s: Division by zero", __func__);
		}

	default:
		error("%s: Got unvalid binary math operation %s", __func__, opcodeToStr(op));
	}
}

bool ScriptValue::operator==(const ScriptValue &other) const {
	return compare(kOpcodeEquals, *this, other);
}

bool ScriptValue::operator!=(const ScriptValue &other) const {
	return compare(kOpcodeNotEquals, *this, other);
}

bool ScriptValue::operator<(const ScriptValue &other) const {
	return compare(kOpcodeLessThan, *this, other);
}

bool ScriptValue::operator>(const ScriptValue &other) const {
	return compare(kOpcodeGreaterThan, *this, other);
}

bool ScriptValue::operator<=(const ScriptValue &other) const {
	return compare(kOpcodeLessThanOrEqualTo, *this, other);
}

bool ScriptValue::operator>=(const ScriptValue &other) const {
	return compare(kOpcodeGreaterThanOrEqualTo, *this, other);
}

bool ScriptValue::operator||(const ScriptValue &other) const {
	if (getType() != kScriptValueTypeBool || other.getType() != kScriptValueTypeBool) {
		error("%s: Expected bools for binary comparison, got %s and %s", __func__, scriptValueTypeToStr(getType()), scriptValueTypeToStr(other.getType()));
	}

	return asBool() || other.asBool();
}

bool ScriptValue::operator^(const ScriptValue &other) const {
	if (getType() != kScriptValueTypeBool || other.getType() != kScriptValueTypeBool) {
		error("%s: Expected bools for binary comparison, got %s and %s", __func__, scriptValueTypeToStr(getType()), scriptValueTypeToStr(other.getType()));
	}

	return asBool() ^ other.asBool();
}

bool ScriptValue::operator&&(const ScriptValue &other) const {
	if (getType() != kScriptValueTypeBool || other.getType() != kScriptValueTypeBool) {
		error("%s: Expected bools for binary comparison, got %s and %s", __func__, scriptValueTypeToStr(getType()), scriptValueTypeToStr(other.getType()));
	}

	return asBool() && other.asBool();
}

ScriptValue ScriptValue::operator+(const ScriptValue &other) const {
	return evalMathOperation(kOpcodeAdd, *this, other);
}

ScriptValue ScriptValue::operator-(const ScriptValue &other) const {
	return evalMathOperation(kOpcodeSubtract, *this, other);
}

ScriptValue ScriptValue::operator*(const ScriptValue &other) const {
	return evalMathOperation(kOpcodeMultiply, *this, other);
}

ScriptValue ScriptValue::operator/(const ScriptValue &other) const {
	return evalMathOperation(kOpcodeDivide, *this, other);
}

ScriptValue ScriptValue::operator%(const ScriptValue &other) const {
	return evalMathOperation(kOpcodeModulo, *this, other);
}

ScriptValue ScriptValue::operator-() const {
	ScriptValue returnValue;
	switch (getType()) {
	case kScriptValueTypeFloat:
		returnValue.setToFloat(-asFloat());
		break;

	case kScriptValueTypeTime:
		returnValue.setToTime(-asTime());
		break;

	default:
		error("%s: Attempted to negate type %s", __func__, scriptValueTypeToStr(getType()));
	}
	return returnValue;
}

} // End of namespace MediaStation
