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

#include "common/str.h"

#include "mediastation/datafile.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediascript/collection.h"

namespace MediaStation {

class Actor;

class ScriptValue {
public:
	ScriptValue() : _type(kScriptValueTypeEmpty), _collection(nullptr) {}
	ScriptValue(ParameterReadStream *stream);
	ScriptValue(const ScriptValue &other);
	~ScriptValue();

	ScriptValueType getType() const { return _type; }

	void setToFloat(uint i);
	void setToFloat(int i);
	void setToFloat(double d);
	double asFloat() const;
	// Some transitions expect either a float or a time depending on engine version.
	// To keep things simple, we have a method to try both.
	double asFloatOrTime() const;

	void setToBool(bool b);
	bool asBool() const;

	void setToTime(double d);
	double asTime() const;

	void setToParamToken(uint paramToken);
	uint asParamToken() const;

	void setToActorId(uint actorId);
	uint asActorId() const;

	void setToString(const Common::String &string);
	Common::String asString() const;

	void setToCollection(Collection *collection);
	Collection *asCollection() const;

	void setToFunctionId(uint functionId);
	uint asFunctionId() const;

	void setToMethodId(BuiltInMethod methodId);
	BuiltInMethod asMethodId() const;

	Common::String getDebugString() const;

	void operator=(const ScriptValue &other);
	bool operator==(const ScriptValue &other) const;
	bool operator!=(const ScriptValue &other) const;
	bool operator<(const ScriptValue &other) const;
	bool operator>(const ScriptValue &other) const;
	bool operator<=(const ScriptValue &other) const;
	bool operator>=(const ScriptValue &other) const;

	bool operator||(const ScriptValue &other) const;
	bool operator^(const ScriptValue &other) const;
	bool operator&&(const ScriptValue &other) const;

	ScriptValue operator+(const ScriptValue &other) const;
	ScriptValue operator-(const ScriptValue &other) const;
	ScriptValue operator*(const ScriptValue &other) const;
	ScriptValue operator/(const ScriptValue &other) const;
	ScriptValue operator%(const ScriptValue &other) const;
	ScriptValue operator-() const;

private:
	ScriptValueType _type = kScriptValueTypeEmpty;
	union {
		double d = 0;
		bool b;
		uint paramToken;
		uint actorId;
		uint functionId;
		BuiltInMethod methodId;
	} _u;
	Common::String _string;
	Collection *_collection = nullptr;
	void clearCollection();
	void copyFrom(const ScriptValue &other);

	static bool compare(Opcode op, const ScriptValue &left, const ScriptValue &right);
	static bool compareEmptyValues(Opcode op);
	static bool compareStrings(Opcode op, const Common::String &left, const Common::String &right);
	static bool compare(Opcode op, uint left, uint right);
	static bool compare(Opcode op, bool left, bool right);
	static bool compare(Opcode op, double left, double right);
	static bool compare(Opcode op, Collection *left, Collection *right);

	static ScriptValue evalMathOperation(Opcode op, const ScriptValue &left, const ScriptValue &right);
	static double binaryMathOperation(Opcode op, double left, double right);
};

} // End of namespace MediaStation

#endif
