
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
	CBofString m_sVarName;  // Name of the variable
	CBofString m_sVarValue; // Value of the variable if not a reference
	VARTYPE m_xVarType;     // Type of variable, string
	BOOL m_bGlobal : 1;     // Is the variable a constant
	BOOL m_bConstant : 1;   // Is the variable a constant
	BOOL m_bReference : 1;  // Is the variable a reference to an objects state date
	BOOL m_bTimer : 1;      // Is the variable updated on object timer events
	BOOL m_bRandom : 1;     // Is the variable updated as a random number

public:
	CBagVar();
	CBagVar(CBagVar &xVar);
	CBagVar(const CBofString &sName, const CBofString &sValue, BOOL bAddToList);
	~CBagVar();

	PARSE_CODES SetInfo(bof_ifstream &);

	const CBofString &GetName() { return m_sVarName; }
	//	const CBofString& 	GetValue() 		{ return m_sVarValue; }
	const CBofString &GetValue();
	int GetNumValue();
	BOOL IsGlobal() { return m_bGlobal; }
	BOOL IsConstant() { return m_bConstant; }
	BOOL IsNumeric() { return m_xVarType == NUMBER; }
	BOOL IsBoolean() { return m_xVarType == BOOLEAN; }
	BOOL IsString() { return m_xVarType == STRING; }
	BOOL IsReference() { return m_bReference; }
	BOOL IsTimer() { return m_bTimer; }
	BOOL IsRandom() { return m_bRandom; }
	const VARTYPE GetType() { return m_xVarType; }

	// Whenever setting the name, add this object to the hash table.
	VOID SetName(const CBofString &s);
	VOID SetValue(const CBofString &s);
	VOID SetValue(int nVal);
	VOID SetBoolValue(BOOL bVal);
	VOID SetGlobal(BOOL bVal = TRUE) { m_bGlobal = bVal; }
	VOID SetConstant(BOOL bVal = TRUE) { m_bConstant = bVal; }
	VOID SetReference(BOOL bVal = TRUE) { m_bReference = bVal; }
	VOID SetTimer(BOOL bVal = TRUE) { m_bTimer = bVal; }
	VOID SetRandom(BOOL bVal = TRUE) { m_bRandom = bVal; }
	VOID SetString() { m_xVarType = STRING; }
	VOID SetNumeric() { m_xVarType = NUMBER; }
	VOID SetBoolean() { m_xVarType = BOOLEAN; }
	// VOID SetType(VARTYPE xType)		{ m_xVarType  = xType; }

	VOID Increment();
};

//  This could be templated with the storage device manager
#define VAR_HTABLE_SIZE 131

class CBagVarManager : public CBagParseObject, public CBofObject {
private:
	static int nVarMngrs;
	CBofList<CBagVar *> m_xVarList;

public:
	CBagVarManager();
	~CBagVarManager();

	ERROR_CODE RegisterVariable(CBagVar *pVar);
	ERROR_CODE UnRegisterVariable(CBagVar *pVar);
	ERROR_CODE UpdateRegistration();
	ERROR_CODE ReleaseVariables(BOOL bIncludeGlobals = TRUE);

	ERROR_CODE IncrementTimers();
	CBagVar *GetVariable(const CBofString &sName);
	CBagVar *GetVariable(INT i) { return m_xVarList[i]; }
	INT GetNumVars() { return m_xVarList.GetCount(); }

	// Use a hash table to lookup variables.
	CBofList<CBagVar *> m_xVarHashList[VAR_HTABLE_SIZE];
};

} // namespace Bagel

#endif
