
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

#ifndef BAGEL_BAGLIB_VAR_H
#define BAGEL_BAGLIB_VAR_H

#include "bagel/baglib/parse_object.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/list.h"

namespace Bagel {

class CBagVar : public CBagParseObject, public CBofObject {
public:
	enum VARTYPE { STRING = 0, NUMBER = 1, BOOLEAN = 2 };

private:
	CBofString _sVarName;  // Name of the variable
	CBofString _sVarValue; // Value of the variable if not a reference
	VARTYPE _xVarType;     // Type of variable, string
	bool _bGlobal : 1;     // Is the variable a constant
	bool _bConstant : 1;   // Is the variable a constant
	bool _bReference : 1;  // Is the variable a reference to an objects state date
	bool _bTimer : 1;      // Is the variable updated on object timer events
	bool _bRandom : 1;     // Is the variable updated as a random number
	bool _freeze = false;

public:
	CBagVar();
	CBagVar(const CBofString &sName, const CBofString &sValue, bool bAddToList);
	virtual ~CBagVar();

	ParseCodes setInfo(CBagIfstream &) override;

	const CBofString &getName() {
		return _sVarName;
	}
	//  const CBofString&   getValue()      { return _sVarValue; }
	const CBofString &getValue();
	int getNumValue();
	bool isGlobal() {
		return _bGlobal;
	}
	bool isConstant() {
		return _bConstant;
	}
	bool isNumeric() {
		return _xVarType == NUMBER;
	}
	bool isBoolean() {
		return _xVarType == BOOLEAN;
	}
	bool isString() {
		return _xVarType == STRING;
	}
	bool isReference() {
		return _bReference;
	}
	bool isTimer() {
		return _bTimer;
	}
	bool isRandom() {
		return _bRandom;
	}
	bool isFrozen() const {
		return _freeze;
	}
	VARTYPE getType() {
		return _xVarType;
	}

	// Whenever setting the name, add this object to the hash table.
	void setName(const CBofString &s);
	void setValue(const CBofString &s);
	void setValue(int nVal);
	void setBoolValue(bool bVal);
	void setGlobal(bool bVal = true) {
		_bGlobal = bVal;
	}
	void setConstant(bool bVal = true) {
		_bConstant = bVal;
	}
	void setReference(bool bVal = true) {
		_bReference = bVal;
	}
	void setTimer(bool bVal = true) {
		_bTimer = bVal;
	}
	void setRandom(bool bVal = true) {
		_bRandom = bVal;
	}
	void setFreeze(bool bVal = true) {
		_freeze = bVal;
	}
	void setString() {
		_xVarType = STRING;
	}
	void setNumeric() {
		_xVarType = NUMBER;
	}
	void setBoolean() {
		_xVarType = BOOLEAN;
	}

	void increment();
};

//  This could be templated with the storage device manager
#define VAR_HASH_TABLE_SIZE 131

class CBagVarManager : public CBagParseObject, public CBofObject {
private:
	static int nVarMngrs;
	CBofList<CBagVar *> _xVarList;

public:
	CBagVarManager();
	virtual ~CBagVarManager();
	static void initialize();

	ErrorCode registerVariable(CBagVar *pVar);
	ErrorCode unRegisterVariable(CBagVar *pVar);
	ErrorCode updateRegistration();
	ErrorCode releaseVariables(bool bIncludeGlobals = true);

	ErrorCode incrementTimers();
	CBagVar *getVariable(const CBofString &sName);
	CBagVar *getVariable(int i) {
		return _xVarList[i];
	}
	int getNumVars() {
		return _xVarList.getCount();
	}

	// Use a hash table to lookup variables.
	CBofList<CBagVar *> _xVarHashList[VAR_HASH_TABLE_SIZE];
};

} // namespace Bagel

#endif
