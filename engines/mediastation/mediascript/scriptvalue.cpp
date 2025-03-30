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
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/function.h"

namespace MediaStation {

void ScriptValue::setToParamToken(int i) {
	switch (_type) {
	case kOperandTypeBool:
	case kOperandTypeInt:
	case kOperandTypeParamToken: {
		_u.i = i;
		break;
	}

	case kOperandTypeVariable: {
		_u.variable->_value.i = i;
		break;
	}

	default:
		error("ScriptValue::putInteger(): Attempt to put integer into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

int ScriptValue::asParamToken() {
	switch (_type) {
	case kOperandTypeBool:
	case kOperandTypeInt:
	case kOperandTypeParamToken: {
		return _u.i;
	}

	case kOperandTypeTime: {
		return static_cast<int>(_u.d);
	}

	case kOperandTypeVariable: {
		return _u.variable->_value.i;
	}

	default:
		error("ScriptValue::getInteger(): Attempt to get integer from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void ScriptValue::setToFloat(double d) {
	switch (_type) {
	case kOperandTypeTime:
	case kOperandTypeFloat: {
		_u.d = d;
		break;
	}

	case kOperandTypeVariable: {
		// TODO: Add assertion.
		_u.variable->_value.d = d;
		break;
	}

	default:
		error("ScriptValue::setToFloat(): Attempt to put double into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

double ScriptValue::asFloat() {
	switch (_type) {
	case kOperandTypeTime:
	case kOperandTypeFloat: {
		return _u.d;
	}

	case kOperandTypeBool:
	case kOperandTypeInt: {
		return static_cast<double>(_u.i);
	}

	case kOperandTypeVariable: {
		// TODO: Add assertion that this is the proper type.
		return _u.variable->_value.d;
	}

	default:
		error("ScriptValue::asFloat(): Attempt to get double from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void ScriptValue::setToString(Common::String *string) {
	switch (_type) {
	case kOperandTypeString: {
		_u.string = string;
		break;
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeString);
		_u.variable->_value.string = string;
		break;
	}

	default:
		error("ScriptValue::setToString(): Attempt to put string into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Common::String *ScriptValue::asString() {
	switch (_type) {
	case kOperandTypeString: {
		return _u.string;
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeString);
		return _u.variable->_value.string;
	}

	default:
		error("ScriptValue::asString(): Attempt to get string from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void ScriptValue::putVariable(Variable *variable) {
	switch (_type) {
	case kOperandTypeVariable: {
		_u.variable = variable;
		break;
	}

	default:
		error("ScriptValue::putVariable(): Attempt to put variable into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Variable *ScriptValue::getVariable() {
	switch (_type) {
	case kOperandTypeVariable: {
		return _u.variable;
	}

	default:
		error("ScriptValue::getVariable(): Attempt to get variable from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void ScriptValue::setToFunctionId(uint functionId) {
	switch (_type) {
	case kOperandTypeFunctionId: {
		_u.functionId = functionId;
		break;
	}

	default:
		error("ScriptValue::setToFunctionId(): Attempt to put function ID into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

uint ScriptValue::asFunctionId() {
	switch (_type) {
	case kOperandTypeFunctionId: {
		return _u.functionId;
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeFunctionId);
		return _u.variable->_value.functionId;
	}

	default:
		error("ScriptValue::getFunction(): Attempt to get function ID from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void ScriptValue::setToMethodId(BuiltInMethod methodId) {
	switch (_type) {
	case kOperandTypeMethodId: {
		_u.methodId = methodId;
		break;
	}

	default:
		error("ScriptValue::setToFunctionId(): Attempt to put method ID into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

BuiltInMethod ScriptValue::asMethodId() {
	switch (_type) {
	case kOperandTypeMethodId: {
		return _u.methodId;
	}

	default:
		error("ScriptValue::getFunction(): Attempt to get method ID from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void ScriptValue::setToAssetId(uint32 assetId) {
	switch (_type) {
	case kOperandTypeAssetId: {
		_u.assetId = assetId;
		break;
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeAssetId);
		_u.variable->_value.assetId = assetId;
		break;
	}

	default:
		error("ScriptValue::setToAssetId(): Attempt to put asset ID into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Asset *ScriptValue::getAsset() {
	switch (_type) {
	case kOperandTypeAssetId: {
		if (_u.assetId == 0) {
			return nullptr;
		} else {
			return g_engine->getAssetById(_u.assetId);
		}
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeAssetId);
		return g_engine->getAssetById(_u.variable->_value.assetId);
	}

	default:
		error("ScriptValue::getAsset(): Attempt to get asset from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

uint32 ScriptValue::asAssetId() {
	switch (_type) {
	case kOperandTypeAssetId: {
		return _u.assetId;
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeAssetId);
		return _u.variable->_value.assetId;
	}

	default:
		error("ScriptValue::getAssetId(): Attempt to get asset ID from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void ScriptValue::setToCollection(Common::SharedPtr<Collection> collection) {
	switch (_type) {
	case kOperandTypeCollection: {
		_collection = collection;
		break;
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeCollection);
		_u.variable->_c = collection;
		break;
	}

	default:
		error("ScriptValue::setToCollection(): Attempt to put collection into ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Common::SharedPtr<Collection> ScriptValue::asCollection() {
	switch (_type) {
	case kOperandTypeCollection: {
		return _collection;
	}

	case kOperandTypeVariable: {
		assert(_u.variable->_type == kScriptValueTypeCollection);
		return _u.variable->_c;
	}

	default:
		error("ScriptValue::asCollection(): Attempt to get collection from ScriptValue type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

ScriptValue ScriptValue::getLiteralValue() const {
	// This function dereferences any variable to get the actual
	// "direct" value (a literal asset ID or otherwise).
	if (_type == kOperandTypeVariable) {
		return _u.variable->getValue();
	} else {
		return *this;
	}
}

bool ScriptValue::operator==(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either ScriptValue is a double, perform double comparison.
		double lhsValue = lhs.isDouble() ? lhs.asFloat() : static_cast<double>(lhs.asParamToken());
		double rhsValue = rhs.isDouble() ? rhs.asFloat() : static_cast<double>(rhs.asParamToken());
		return lhsValue == rhsValue;
	} else {
		switch (lhs.getType()) {
		case kOperandTypeBool:
		case kOperandTypeInt:
			return lhs.asParamToken() == rhs.asParamToken();

		case kOperandTypeAssetId:
			if (rhs.getType() == kOperandTypeInt) {
				// This might happen if, for example, a given asset wasn't found
				// in a collection and the script sets the return value to -1.
				return static_cast<int>(lhs.asAssetId()) == rhs.asParamToken();
			} else {
				// If the types are incompatiable, rhs will raise the error.
				return lhs.asAssetId() == rhs.asAssetId();
			}

		case kOperandTypeString:
			return *lhs.asString() == *rhs.asString();

		default:
			error("ScriptValue::operator==(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
		}
	}
}

bool ScriptValue::operator<(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("ScriptValue::operator<(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either ScriptValue is a double, perform double comparison.
		double lhsValue = lhs.isDouble() ? lhs.asFloat() : static_cast<double>(lhs.asParamToken());
		double rhsValue = rhs.isDouble() ? rhs.asFloat() : static_cast<double>(rhs.asParamToken());
		return lhsValue < rhsValue;
	} else {
		// Otherwise, perform integer comparison.
		return lhs.asParamToken() < rhs.asParamToken();
	}
}

bool ScriptValue::operator>(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("ScriptValue::operator>(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either ScriptValue is a double, perform double comparison.
		double lhsValue = lhs.isDouble() ? lhs.asFloat() : static_cast<double>(lhs.asParamToken());
		double rhsValue = rhs.isDouble() ? rhs.asFloat() : static_cast<double>(rhs.asParamToken());
		return lhsValue > rhsValue;
	} else {
		// Otherwise, perform integer comparison.
		return lhs.asParamToken() > rhs.asParamToken();
	}
}

bool ScriptValue::operator||(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();

	// If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error.
	switch (lhs.getType()) {
	case kOperandTypeBool:
	case kOperandTypeInt:
		return lhs.asParamToken() || rhs.asParamToken();

	default:
		error("ScriptValue::operator||(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}
}

bool ScriptValue::operator!() const {
	ScriptValue literalValue = getLiteralValue();

	// If the types being compared end up being incompatible, the respective get
	// method will raise the error.
	switch (literalValue.getType()) {
	case kOperandTypeBool:
	case kOperandTypeInt:
		return !literalValue.asParamToken();

	default:
		error("ScriptValue::operator!(): Unimplemented ScriptValue type %s", operandTypeToStr(literalValue.getType()));
	}
}

bool ScriptValue::operator&&(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();

	// If the types being compared end up being incompatible, the respective get
	// method will raise the error.
	switch (lhs.getType()) {
	case kOperandTypeBool:
	case kOperandTypeInt:
		return lhs.asParamToken() && rhs.asParamToken();

	default:
		error("ScriptValue::operator&&(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}
}

ScriptValue ScriptValue::operator+(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();
	ScriptValue returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("ScriptValue::operator+(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either ScriptValue is a double, perform double addition.
		double lhsValue = lhs.isDouble() ? lhs.asFloat() : static_cast<double>(lhs.asParamToken());
		double rhsValue = rhs.isDouble() ? rhs.asFloat() : static_cast<double>(rhs.asParamToken());
		returnValue.setToFloat(lhsValue + rhsValue);
	} else {
		// Otherwise, perform integer addition.
		returnValue.setToParamToken(lhs.asParamToken() + rhs.asParamToken());
	}

	return returnValue;
}

ScriptValue ScriptValue::operator-(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();
	ScriptValue returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("ScriptValue::operator-(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either ScriptValue is a double, perform double subtraction.
		double lhsValue = lhs.isDouble() ? lhs.asFloat() : static_cast<double>(lhs.asParamToken());
		double rhsValue = rhs.isDouble() ? rhs.asFloat() : static_cast<double>(rhs.asParamToken());
		returnValue.setToFloat(lhsValue - rhsValue);
	} else {
		// Otherwise, perform integer subtraction.
		returnValue.setToParamToken(lhs.asParamToken() - rhs.asParamToken());
	}

	return returnValue;
}

ScriptValue ScriptValue::operator*(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();
	ScriptValue returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("ScriptValue::operator*(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either ScriptValue is a double, perform double multiplication.
		double lhsValue = lhs.isDouble() ? lhs.asFloat() : static_cast<double>(lhs.asParamToken());
		double rhsValue = rhs.isDouble() ? rhs.asFloat() : static_cast<double>(rhs.asParamToken());
		returnValue.setToFloat(lhsValue * rhsValue);
	} else {
		// Otherwise, perform integer subtraction.
		returnValue.setToParamToken(lhs.asParamToken() * rhs.asParamToken());
	}

	return returnValue;
}

ScriptValue ScriptValue::operator/(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();
	ScriptValue returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("ScriptValue::operator/(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either ScriptValue is a double, perform double division.
		double lhsValue = lhs.isDouble() ? lhs.asFloat() : static_cast<double>(lhs.asParamToken());
		double rhsValue = rhs.isDouble() ? rhs.asFloat() : static_cast<double>(rhs.asParamToken());
		returnValue.setToFloat(lhsValue / rhsValue);
	} else {
		// Otherwise, perform integer division.
		returnValue.setToParamToken(lhs.asParamToken() / rhs.asParamToken());
	}

	return returnValue;

}

ScriptValue ScriptValue::operator%(const ScriptValue &other) const {
	ScriptValue lhs = getLiteralValue();
	ScriptValue rhs = other.getLiteralValue();
	ScriptValue returnValue(lhs.getType());

	// If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error.
	switch (lhs.getType()) {
	case kOperandTypeBool:
	case kOperandTypeInt:
		if (rhs.asParamToken() == 0) {
			error("ScriptValue::operator%%(): Attempted mod by zero");
		}
		returnValue.setToParamToken(lhs.asParamToken() % rhs.asParamToken());
		return returnValue;

	default:
		error("ScriptValue::operator/(): Unimplemented ScriptValue types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}
}

ScriptValue ScriptValue::operator-() const {
	ScriptValue literalValue = getLiteralValue();
	ScriptValue returnValue(literalValue.getType());

	// If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error.
	switch (literalValue.getType()) {
	case kOperandTypeBool:
	case kOperandTypeInt:
		returnValue.setToParamToken(-literalValue.asParamToken());
		return returnValue;

	case kOperandTypeTime:
	case kOperandTypeFloat:
		returnValue.setToFloat(-literalValue.asFloat());
		return returnValue;

	default:
		error("ScriptValue::operator-(): Unimplemented ScriptValue type %s", operandTypeToStr(literalValue.getType()));
	}
}

} // End of namespace MediaStation
