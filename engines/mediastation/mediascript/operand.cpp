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
#include "mediastation/mediascript/operand.h"
#include "mediastation/mediascript/function.h"

namespace MediaStation {

void Operand::putInteger(int i) {
	switch (_type) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
	case kOperandTypeDollarSignVariable: {
		_u.i = i;
		break;
	}

	case kOperandTypeVariableDeclaration: {
		_u.variable->_value.i = i;
		break;
	}

	default:
		error("Operand::putInteger(): Attempt to put integer into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

int Operand::getInteger() {
	switch (_type) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
	case kOperandTypeDollarSignVariable: {
		return _u.i;
	}

	case kOperandTypeFloat1: {
		return static_cast<int>(_u.d);
	}

	case kOperandTypeVariableDeclaration: {
		return _u.variable->_value.i;
	}

	default:
		error("Operand::getInteger(): Attempt to get integer from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Operand::putDouble(double d) {
	switch (_type) {
	case kOperandTypeFloat1:
	case kOperandTypeFloat2: {
		_u.d = d;
		break;
	}

	case kOperandTypeVariableDeclaration: {
		// TODO: Add assertion.
		_u.variable->_value.d = d;
		break;
	}

	default:
		error("Operand::putDouble(): Attempt to put double into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

double Operand::getDouble() {
	switch (_type) {
	case kOperandTypeFloat1:
	case kOperandTypeFloat2: {
		return _u.d;
	}

	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2: {
		return static_cast<double>(_u.i);
	}

	case kOperandTypeVariableDeclaration: {
		// TODO: Add assertion that this is the proper type.
		return _u.variable->_value.d;
	}

	default:
		error("Operand::getDouble(): Attempt to get double from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Operand::putString(Common::String *string) {
	switch (_type) {
	case kOperandTypeString: {
		_u.string = string;
		break;
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeString);
		_u.variable->_value.string = string;
		break;
	}

	default:
		error("Operand::putString(): Attempt to put string into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Common::String *Operand::getString() {
	switch (_type) {
	case kOperandTypeString: {
		return _u.string;
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeString);
		return _u.variable->_value.string;
	}

	default:
		error("Operand::getString(): Attempt to get string from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Operand::putVariable(Variable *variable) {
	switch (_type) {
	case kOperandTypeVariableDeclaration: {
		_u.variable = variable;
		break;
	}

	default:
		error("Operand::putVariable(): Attempt to put variable into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Variable *Operand::getVariable() {
	switch (_type) {
	case kOperandTypeVariableDeclaration: {
		return _u.variable;
	}

	default:
		error("Operand::getVariable(): Attempt to get variable from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Operand::putFunctionId(uint functionId) {
	switch (_type) {
	case kOperandTypeFunction: {
		_u.functionId = functionId;
		break;
	}

	default:
		error("Operand::putFunctionId(): Attempt to put function ID into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

uint Operand::getFunctionId() {
	switch (_type) {
	case kOperandTypeFunction: {
		return _u.functionId;
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeFunction);
		return _u.variable->_value.functionId;
	}

	default:
		error("Operand::getFunction(): Attempt to get function ID from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Operand::putMethodId(BuiltInMethod methodId) {
	switch (_type) {
	case kOperandTypeMethod: {
		_u.methodId = methodId;
		break;
	}

	default:
		error("Operand::putFunctionId(): Attempt to put method ID into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

BuiltInMethod Operand::getMethodId() {
	switch (_type) {
	case kOperandTypeMethod: {
		return _u.methodId;
	}

	default:
		error("Operand::getFunction(): Attempt to get method ID from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Operand::putAsset(uint32 assetId) {
	switch (_type) {
	case kOperandTypeAssetId: {
		_u.assetId = assetId;
		break;
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeAssetId);
		_u.variable->_value.assetId = assetId;
		break;
	}

	default:
		error("Operand::putAsset(): Attempt to put asset ID into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Asset *Operand::getAsset() {
	switch (_type) {
	case kOperandTypeAssetId: {
		if (_u.assetId == 0) {
			return nullptr;
		} else {
			return g_engine->getAssetById(_u.assetId);
		}
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeAssetId);
		return g_engine->getAssetById(_u.variable->_value.assetId);
	}

	default:
		error("Operand::getAsset(): Attempt to get asset from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

uint32 Operand::getAssetId() {
	switch (_type) {
	case kOperandTypeAssetId: {
		return _u.assetId;
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeAssetId);
		return _u.variable->_value.assetId;
	}

	default:
		error("Operand::getAssetId(): Attempt to get asset ID from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

void Operand::putCollection(Common::SharedPtr<Collection> collection) {
	switch (_type) {
	case kOperandTypeCollection: {
		_collection = collection;
		break;
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeCollection);
		_u.variable->_c = collection;
		break;
	}

	default:
		error("Operand::putCollection(): Attempt to put collection into operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Common::SharedPtr<Collection> Operand::getCollection() {
	switch (_type) {
	case kOperandTypeCollection: {
		return _collection;
	}

	case kOperandTypeVariableDeclaration: {
		assert(_u.variable->_type == kVariableTypeCollection);
		return _u.variable->_c;
	}

	default:
		error("Operand::getCollection(): Attempt to get collection from operand type %s (%d)",
			operandTypeToStr(_type), static_cast<uint>(_type));
	}
}

Operand Operand::getLiteralValue() const {
	// This function dereferences any variable to get the actual
	// "direct" value (a literal asset ID or otherwise).
	if (_type == kOperandTypeVariableDeclaration) {
		return _u.variable->getValue();
	} else {
		return *this;
	}
}

bool Operand::operator==(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either operand is a double, perform double comparison.
		double lhsValue = lhs.isDouble() ? lhs.getDouble() : static_cast<double>(lhs.getInteger());
		double rhsValue = rhs.isDouble() ? rhs.getDouble() : static_cast<double>(rhs.getInteger());
		return lhsValue == rhsValue;
	} else {
		switch (lhs.getType()) {
		case kOperandTypeLiteral1:
		case kOperandTypeLiteral2:
			return lhs.getInteger() == rhs.getInteger();

		case kOperandTypeAssetId:
			if (rhs.getType() == kOperandTypeLiteral2) {
				// This might happen if, for example, a given asset wasn't found
				// in a collection and the script sets the return value to -1.
				return static_cast<int>(lhs.getAssetId()) == rhs.getInteger();
			} else {
				// If the types are incompatiable, rhs will raise the error.
				return lhs.getAssetId() == rhs.getAssetId();
			}

		case kOperandTypeString:
			return *lhs.getString() == *rhs.getString();

		default:
			error("Operand::operator==(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
		}
	}
}

bool Operand::operator<(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("Operand::operator<(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either operand is a double, perform double comparison.
		double lhsValue = lhs.isDouble() ? lhs.getDouble() : static_cast<double>(lhs.getInteger());
		double rhsValue = rhs.isDouble() ? rhs.getDouble() : static_cast<double>(rhs.getInteger());
		return lhsValue < rhsValue;
	} else {
		// Otherwise, perform integer comparison.
		return lhs.getInteger() < rhs.getInteger();
	}
}

bool Operand::operator>(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("Operand::operator>(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either operand is a double, perform double comparison.
		double lhsValue = lhs.isDouble() ? lhs.getDouble() : static_cast<double>(lhs.getInteger());
		double rhsValue = rhs.isDouble() ? rhs.getDouble() : static_cast<double>(rhs.getInteger());
		return lhsValue > rhsValue;
	} else {
		// Otherwise, perform integer comparison.
		return lhs.getInteger() > rhs.getInteger();
	}
}

bool Operand::operator||(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();

	// If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error.
	switch (lhs.getType()) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
		return lhs.getInteger() || rhs.getInteger();

	default:
		error("Operand::operator||(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}
}

bool Operand::operator!() const {
	Operand literalValue = getLiteralValue();

	// If the types being compared end up being incompatible, the respective get
	// method will raise the error.
	switch (literalValue.getType()) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
		return !literalValue.getInteger();

	default:
		error("Operand::operator!(): Unimplemented operand type %s", operandTypeToStr(literalValue.getType()));
	}
}

bool Operand::operator&&(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();

	// If the types being compared end up being incompatible, the respective get
	// method will raise the error.
	switch (lhs.getType()) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
		return lhs.getInteger() && rhs.getInteger();

	default:
		error("Operand::operator&&(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}
}

Operand Operand::operator+(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();
	Operand returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("Operand::operator+(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either operand is a double, perform double addition.
		double lhsValue = lhs.isDouble() ? lhs.getDouble() : static_cast<double>(lhs.getInteger());
		double rhsValue = rhs.isDouble() ? rhs.getDouble() : static_cast<double>(rhs.getInteger());
		returnValue.putDouble(lhsValue + rhsValue);
	} else {
		// Otherwise, perform integer addition.
		returnValue.putInteger(lhs.getInteger() + rhs.getInteger());
	}

	return returnValue;
}

Operand Operand::operator-(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();
	Operand returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("Operand::operator-(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either operand is a double, perform double subtraction.
		double lhsValue = lhs.isDouble() ? lhs.getDouble() : static_cast<double>(lhs.getInteger());
		double rhsValue = rhs.isDouble() ? rhs.getDouble() : static_cast<double>(rhs.getInteger());
		returnValue.putDouble(lhsValue - rhsValue);
	} else {
		// Otherwise, perform integer subtraction.
		returnValue.putInteger(lhs.getInteger() - rhs.getInteger());
	}

	return returnValue;
}

Operand Operand::operator*(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();
	Operand returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("Operand::operator*(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either operand is a double, perform double multiplication.
		double lhsValue = lhs.isDouble() ? lhs.getDouble() : static_cast<double>(lhs.getInteger());
		double rhsValue = rhs.isDouble() ? rhs.getDouble() : static_cast<double>(rhs.getInteger());
		returnValue.putDouble(lhsValue * rhsValue);
	} else {
		// Otherwise, perform integer subtraction.
		returnValue.putInteger(lhs.getInteger() * rhs.getInteger());
	}

	return returnValue;
}

Operand Operand::operator/(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();
	Operand returnValue(lhs.getType());

	if (!lhs.isNumber() || !rhs.isNumber()) {
		error("Operand::operator/(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}

	if (lhs.isDouble() || rhs.isDouble()) {
		// If either operand is a double, perform double division.
		double lhsValue = lhs.isDouble() ? lhs.getDouble() : static_cast<double>(lhs.getInteger());
		double rhsValue = rhs.isDouble() ? rhs.getDouble() : static_cast<double>(rhs.getInteger());
		returnValue.putDouble(lhsValue / rhsValue);
	} else {
		// Otherwise, perform integer division.
		returnValue.putInteger(lhs.getInteger() / rhs.getInteger());
	}

	return returnValue;

}

Operand Operand::operator%(const Operand &other) const {
	Operand lhs = getLiteralValue();
	Operand rhs = other.getLiteralValue();
	Operand returnValue(lhs.getType());

	// If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error.
	switch (lhs.getType()) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
		if (rhs.getInteger() == 0) {
			error("Operand::operator%%(): Attempted mod by zero");
		}
		returnValue.putInteger(lhs.getInteger() % rhs.getInteger());
		return returnValue;

	default:
		error("Operand::operator/(): Unimplemented operand types %s and %s", operandTypeToStr(lhs.getType()), operandTypeToStr(rhs.getType()));
	}
}

Operand Operand::operator-() const {
	Operand literalValue = getLiteralValue();
	Operand returnValue(literalValue.getType());

	// If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error.
	switch (literalValue.getType()) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
		returnValue.putInteger(-literalValue.getInteger());
		return returnValue;

	case kOperandTypeFloat1:
	case kOperandTypeFloat2:
		returnValue.putDouble(-literalValue.getDouble());
		return returnValue;

	default:
		error("Operand::operator-(): Unimplemented operand type %s", operandTypeToStr(literalValue.getType()));
	}
}

} // End of namespace MediaStation
