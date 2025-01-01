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
	case Operand::Type::Literal1:
	case Operand::Type::Literal2:
	case Operand::Type::DollarSignVariable: {
		_u.i = i;
		break;
	}

	case Operand::Type::VariableDeclaration: {
		_u.variable->value.i = i;
		break;
	}

	default: {
		error("Operand::putInteger(): Attempt to put unsupported value into operand (type 0x%x)", _type);
	}
	}
}

int Operand::getInteger() {
	switch (_type) {
	case Operand::Type::Literal1:
	case Operand::Type::Literal2:
	case Operand::Type::DollarSignVariable: {
		return _u.i;
	}

	case Operand::Type::VariableDeclaration: {
		return _u.variable->value.i;
	}

	default: {
		error("Operand::getInteger(): Attempt to get unsupported value from operand (type 0x%x)", _type);
	}
	}
}

void Operand::putDouble(double d) {
	switch (_type) {
	case Operand::Type::Float1:
	case Operand::Type::Float2: {
		_u.d = d;
		break;
	}

	case Operand::Type::VariableDeclaration: {
		// TODO: Add assertion.
		_u.variable->value.d = d;
		break;
	}

	default: {
		error("Operand::putDouble(): Attempt to put unsupported value in operand (type 0x%x)", _type);
	}
	}
}

double Operand::getDouble() {
	switch (_type) {
	case Operand::Type::Float1:
	case Operand::Type::Float2: {
		return _u.d;
	}

	case Operand::Type::VariableDeclaration: {
		// TODO: Add assertion that this is the proper type.
		return _u.variable->value.d;
	}

	default: {
		error("Operand::getDouble(): Attempt to get unsupported value from operand (type 0x%x)", _type);
	}
	}
}

void Operand::putString(Common::String *string) {
	switch (_type) {
	case Operand::Type::String: {
		_u.string = string;
		break;
	}

	case Operand::Type::VariableDeclaration: {
		assert(_u.variable->type == Variable::Type::STRING);
		_u.variable->value.string = string;
		break;
	}

	default: {
		error("Operand::putString(): Attempt to put unsupported value into operand (type 0x%x)", _type);
	}
	}
}

Common::String *Operand::getString() {
	switch (_type) {
	case Operand::Type::String: {
		return _u.string;
	}

	case Operand::Type::VariableDeclaration: {
		assert(_u.variable->type == Variable::Type::STRING);
		return _u.variable->value.string;
	}

	default: {
		error("Operand::getString(): Attempt to get unsupported value from operand (type 0x%x)", _type);
	}
	}
}

void Operand::putVariable(Variable *variable) {
	switch (_type) {
	case Operand::Type::VariableDeclaration: {
		_u.variable = variable;
		break;
	}

	default: {
		error("Operand::putVariable(): Attempt to put unsupported value into operand that is not a variable (type 0x%x)", _type);
	}
	}
}

Variable *Operand::getVariable() {
	switch (_type) {
	case Operand::Type::VariableDeclaration: {
		return _u.variable;
	}

	default: {
		error("Operand::getVariable(): Attempt to get unsupported value from operand that is not a variable (type 0x%x)", _type);
	}
	}
}

void Operand::putFunction(Function *function) {
	switch (_type) {
	case Operand::Type::Function: {
		_u.function = function;
		break;
	}

	default: {
		error("Operand::putFunction(): Attempt to put unsupported value into operand that is not a function (type 0x%x)", _type);
	}
	}
}

Function *Operand::getFunction() {
	switch (_type) {
	case Operand::Type::Function: {
		return _u.function;
	}

	default: {
		error("Operand::getFunction(): Attempt to get unsupported value from operand that is not a function (type 0x%x)", _type);
	}
	}
}

void Operand::putAsset(uint32 assetId) {
	switch (_type) {
	case Operand::Type::AssetId: {
		_u.assetId = assetId;
		break;
	}

	case Operand::Type::VariableDeclaration: {
		assert(_u.variable->type == Variable::Type::ASSET_ID);
		_u.variable->value.assetId = assetId;
		break;
	}

	default: {
		error("Operand::putAsset(): Attempt to put asset into operand that is not an asset (type 0x%x)", _type);
	}
	}
}

Asset *Operand::getAsset() {
	switch (_type) {
	case Operand::Type::AssetId: {
		if (_u.assetId == 0) {
			return nullptr;
		} else {
			return g_engine->_assets.getVal(_u.assetId);
		}
	}

	case Operand::Type::VariableDeclaration: {
		assert(_u.variable->type == Variable::Type::ASSET_ID);
		return g_engine->_assets.getVal(_u.variable->value.assetId);
	}

	default: {
		error("Operand::getAsset(): Attempt to get asset from operand that is not an asset (type 0x%x)", _type);
	}
	}
}

uint32 Operand::getAssetId() {
	switch (_type) {
	case Operand::Type::AssetId: {
		return _u.assetId;
	}

	case Operand::Type::VariableDeclaration: {
		assert(_u.variable->type == Variable::Type::ASSET_ID);
		return _u.variable->value.assetId;
	}

	default: {
		error("Operand::getAssetId(): Attempt to get asset ID from operand that is not an asset (type 0x%x)", _type);
	}
	}
}

Operand Operand::operator-(const Operand &other) const {
	Operand returnValue;
	if (this->_type == Operand::Type::Literal1 && other._type == Operand::Type::Literal1) {
		returnValue._type = Operand::Type::Literal1;
		returnValue._u.i = this->_u.i - other._u.i;
	} else if (this->_type == Operand::Type::Float1 && other._type == Operand::Type::Float1) {
		returnValue._type = Operand::Type::Float1;
		returnValue._u.d = this->_u.d - other._u.d;
	} else {
		error("Operand::operator-(): Unsupported operand types %d and %d", this->_type, other._type);
	}
	return returnValue;
}

} // End of namespace MediaStation
