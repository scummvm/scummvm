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

#ifndef MEDIASTATION_MEDIASCRIPT_OPERAND_H
#define MEDIASTATION_MEDIASCRIPT_OPERAND_H

#include "common/array.h"

#include "mediastation/mediascript/variable.h"

namespace MediaStation {

class Function;
class Asset;

enum OperandType {
	kOperandTypeEmpty = 0, // a flag for C++ code, not real operand type.
	// TODO: Figure out the difference between these two.
	kOperandTypeLiteral1 = 151,
	kOperandTypeLiteral2 = 153,
	// TODO: Figure out the difference between these two.
	kOperandTypeFloat1 = 152,
	kOperandTypeFloat2 = 157,
	kOperandTypeString = 154,
	// TODO: This only seems to be used in effectTransition:
	//  effectTransition ( $FadeToPalette )
	// compiles to:
	//  [219, 102, 1]
	//  [155, 301]
	kOperandTypeDollarSignVariable = 155,
	kOperandTypeAssetId = 156,
	kOperandTypeVariableDeclaration = 158,
	kOperandTypeFunction = 160
};

class Operand {
public:
	Operand() : _type(kOperandTypeEmpty) {}
	Operand(OperandType type) : _type(type) {}

	OperandType getType() const {
		return _type;
	}

	void putInteger(int i);
	int getInteger();

	void putDouble(double d);
	double getDouble();

	void putString(Common::String *string);
	Common::String *getString();

	void putVariable(Variable *variable);
	Variable *getVariable();

	void putFunction(Function *function);
	Function *getFunction();

	void putAsset(uint32 assetId);
	Asset *getAsset();
	uint32 getAssetId();

	Operand operator-(const Operand &other) const;

private:
	OperandType _type = kOperandTypeEmpty;
	union {
		uint assetId = 0;
		Common::String *string;
		Variable *variable;
		Function *function;
		int i;
		double d;
	} _u;
};

} // End of namespace MediaStation

#endif