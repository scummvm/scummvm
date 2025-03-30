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

#ifndef MEDIASTATION_MEDIASCRIPT_SCRIPTVALUE_H
#define MEDIASTATION_MEDIASCRIPT_SCRIPTVALUE_H

#include "common/ptr.h"
#include "common/str.h"

#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediascript/variable.h"

namespace MediaStation {

class Asset;

class ScriptValue {
public:
	ScriptValue() : _type(kOperandTypeEmpty) {}
	ScriptValue(OperandType type) : _type(type) {}

	OperandType getType() const { return _type; }

	void setToParamToken(int i);
	int asParamToken();

	void setToFloat(double d);
	double asFloat();

	void setToString(Common::String *string);
	Common::String *asString();

	void putVariable(Variable *variable);
	Variable *getVariable();

	void setToFunctionId(uint functionId);
	uint asFunctionId();

	void setToMethodId(BuiltInMethod methodId);
	BuiltInMethod asMethodId();

	void setToAssetId(uint32 assetId);
	Asset *getAsset();
	uint32 asAssetId();

	void setToCollection(Common::SharedPtr<Collection> collection);
	Common::SharedPtr<Collection> asCollection();

	ScriptValue getLiteralValue() const;

	bool operator==(const ScriptValue &other) const;
	bool operator<(const ScriptValue &other) const;
	bool operator>(const ScriptValue &other) const;

	bool operator||(const ScriptValue &other) const;
	bool operator!() const;
	bool operator&&(const ScriptValue &other) const;

	ScriptValue operator+(const ScriptValue &other) const;
	ScriptValue operator-(const ScriptValue &other) const;
	ScriptValue operator*(const ScriptValue &other) const;
	ScriptValue operator/(const ScriptValue &other) const;
	ScriptValue operator%(const ScriptValue &other) const;
	ScriptValue operator-() const;

private:
	bool isInteger() { return getType() == kOperandTypeBool || getType() == kOperandTypeInt; };
	bool isDouble() { return getType() == kOperandTypeFloat || getType() == kOperandTypeTime; };
	bool isNumber() { return isInteger() || isDouble(); };

	OperandType _type = kOperandTypeEmpty;
	union {
		uint assetId = 0;
		uint functionId;
		BuiltInMethod methodId;
		Common::String *string;
		Variable *variable;
		int i;
		double d;
	} _u;
	Common::SharedPtr<Collection> _collection;
};

} // End of namespace MediaStation

#endif