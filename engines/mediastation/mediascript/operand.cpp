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

	default: {
		error("Operand::putInteger(): Attempt to put unsupported value into operand (type 0x%x)", static_cast<uint>(_type));
	}
	}
}

int Operand::getInteger() {
	switch (_type) {
	case kOperandTypeLiteral1:
	case kOperandTypeLiteral2:
	case kOperandTypeDollarSignVariable: {
		return _u.i;
	}

	case kOperandTypeVariableDeclaration: {
		return _u.variable->_value.i;
	}

	default: {
		error("Operand::getInteger(): Attempt to get unsupported value from operand (type 0x%x)", static_cast<uint>(_type));
	}
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

	default: {
		error("Operand::putDouble(): Attempt to put unsupported value in operand (type 0x%x)", static_cast<uint>(_type));
	}
	}
}

double Operand::getDouble() {
	switch (_type) {
	case kOperandTypeFloat1:
	case kOperandTypeFloat2: {
		return _u.d;
	}

	case kOperandTypeVariableDeclaration: {
		// TODO: Add assertion that this is the proper type.
		return _u.variable->_value.d;
	}

	default: {
		error("Operand::getDouble(): Attempt to get unsupported value from operand (type 0x%x)", static_cast<uint>(_type));
	}
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

	default: {
		error("Operand::putString(): Attempt to put unsupported value into operand (type 0x%x)", static_cast<uint>(_type));
	}
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

	default: {
		error("Operand::getString(): Attempt to get unsupported value from operand (type 0x%x)", static_cast<uint>(_type));
	}
	}
}

void Operand::putVariable(Variable *variable) {
	switch (_type) {
	case kOperandTypeVariableDeclaration: {
		_u.variable = variable;
		break;
	}

	default: {
		error("Operand::putVariable(): Attempt to put unsupported value into operand that is not a variable (type 0x%x)", static_cast<uint>(_type));
	}
	}
}

Variable *Operand::getVariable() {
	switch (_type) {
	case kOperandTypeVariableDeclaration: {
		return _u.variable;
	}

	default: {
		error("Operand::getVariable(): Attempt to get unsupported value from operand that is not a variable (type 0x%x)", static_cast<uint>(_type));
	}
	}
}

void Operand::putFunction(uint functionId) {
	switch (_type) {
	case kOperandTypeFunction: {
		_u.functionId = functionId;
		break;
	}

	default: {
		error("Operand::putFunction(): Attempt to put unsupported value into operand that is not a function (type 0x%x)", static_cast<uint>(_type));
	}
	}
}

uint Operand::getFunctionId() {
	switch (_type) {
	case kOperandTypeFunction: {
		return _u.functionId;
	}

	default: {
		error("Operand::getFunction(): Attempt to get unsupported value from operand that is not a function (type 0x%x)", static_cast<uint>(_type));
	}
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

	default: {
		error("Operand::putAsset(): Attempt to put asset into operand that is not an asset (type 0x%x)", static_cast<uint>(_type));
	}
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

	default: {
		error("Operand::getAsset(): Attempt to get asset from operand that is not an asset (type 0x%x)", static_cast<uint>(_type));
	}
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

	default: {
		error("Operand::getAssetId(): Attempt to get asset ID from operand that is not an asset (type 0x%x)", static_cast<uint>(_type));
	}
	}
}

Operand Operand::getLiteralValue() {
	// This function dereferences any variable to get the actual
	// "direct" value (a literal asset ID or otherwise).
	if (_type == kOperandTypeVariableDeclaration) {
		return _u.variable->getValue();
	} else {
		return *this;
	}
}

bool Operand::operator==(Operand &other) {
	Operand lhs = getLiteralValue();
	Operand rhs = getLiteralValue();
	// TODO: Maybe some better type checking here. If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error. But better might be checking
	// both before we try getting values to report a more descriptive error.
	switch (lhs.getType()) {
	case kOperandTypeLiteral1: 
	case kOperandTypeLiteral2:
		return lhs.getInteger() == rhs.getInteger();

	case kOperandTypeFloat1:
	case kOperandTypeFloat2:
		return lhs.getDouble() == rhs.getDouble();

	case kOperandTypeString:
		return *lhs.getString() == *rhs.getString();

	default:
		error("Operand::operator==(): Unsupported operand types %d and %d", static_cast<uint>(lhs.getType()), static_cast<uint>(rhs.getType()));
	}
}

bool Operand::operator>=(Operand &other) {
	Operand lhs = getLiteralValue();
	Operand rhs = getLiteralValue();
	// If the types being compared end up being incompatible, the respective get
	// method on the rhs will raise the error.
	switch (lhs.getType()) {
	case kOperandTypeLiteral1: 
	case kOperandTypeLiteral2:
		return lhs.getInteger() >= rhs.getInteger();

	case kOperandTypeFloat1:
	case kOperandTypeFloat2:
		return lhs.getDouble() >= rhs.getDouble();

	default:
		error("Operand::operator>=(): Unsupported operand types %d and %d", static_cast<uint>(lhs.getType()), static_cast<uint>(rhs.getType()));
	}
}

Operand Operand::operator-(const Operand &other) const {
	Operand returnValue;
	if (this->_type == kOperandTypeLiteral1 && other._type == kOperandTypeLiteral1) {
		returnValue._type = kOperandTypeLiteral1;
		returnValue._u.i = this->_u.i - other._u.i;
	} else if (this->_type == kOperandTypeFloat1 && other._type == kOperandTypeFloat1) {
		returnValue._type = kOperandTypeFloat1;
		returnValue._u.d = this->_u.d - other._u.d;
	} else {
		error("Operand::operator-(): Unsupported operand types %d and %d", static_cast<uint>(this->_type), static_cast<uint>(other._type));
	}
	return returnValue;
}

} // End of namespace MediaStation
