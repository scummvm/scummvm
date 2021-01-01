/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ICB_GAME_OBJECT_H
#define ICB_GAME_OBJECT_H

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/px_assert.h"

namespace ICB {

// object run-time status values
enum _object_status {       // possible values of object status field
	OB_STATUS_NOT_HELD, // 0
	OB_STATUS_HELD      // 1 (when an object is held it does not get processed or drawn - it is excluded from the game)
};

#define OB_INIT_SCRIPT 0
#define OB_LOGIC_CONTEXT 1
#define OB_ACTION_CONTEXT 2

#define c_game_object c_compressed_game_object

class c_un_game_object {
	// Only ob_status is made public. To access the other elements use
	// the access functions. This is so that further changes to the structure
	// can be catered for through the access functions, and not needed where
	// any element is specifically referenced

public:
	// Main access functions
	cstr GetName() const;             // Get a pointer to the object name
	uint GetNoLvars() const;          // Get the number of local variables
	uint GetNoScripts() const;        // Get the number of scripts
	cstr GetScriptName(uint32) const; // Get the name of a script
	uint32 GetSize() { return (m_size); }
	uint32 GetLvarInfoOffset() { return (m_var_table_offset); }

	cstr GetScriptVariableName(uint32) const; // gets name
	int32 GetVariable(cstr name) const;       // get's number of named variable

	int32 IsVariableString(uint32) const; // is variable a string (1=string, 0=int)

	void SetIntegerVariable(uint32, int32); // Sets the value of an integer variable

	int32 &GetIntegerVariable(uint32); // Get the value of an integer variable
	cstr GetStringVariable(uint32) const;    // Get the value of a string variable

	cstr GetStringValueOrDefault(const cstr varName, const cstr defaultStr) {
		int32 var;
		var = GetVariable(varName);
		if (var == -1)
			return defaultStr;
		else
			return GetStringVariable(var);
	}

	int32 GetIntegerValueOrDefault(const cstr varName, int32 defaultInt) {
		int32 var;
		var = GetVariable(varName);
		if (var == -1)
			return defaultInt;
		else
			return GetIntegerVariable(var);
	}

	// Let the new compressed object have full access to this one
	friend class c_compressed_game_objectCreator;

protected:       // The object data
	uint32 m_size; // The size of the total data structure

	uint32 m_var_table_offset;

public:
	uint32 ob_status; // low level internal stuff - see enum _object_status

protected: // The rest of the data
	// The offsets to the blocks of data. All offsets
	// are from the start of the object
	uint32 m_script_name_table_offset; // Offset to the script name table
	uint32 m_lvars_offset;             // Offset to the local variable data
	uint32 m_name_offset;              // Offset to the object name

	// Variable and script count
	uint32 m_noLvars;   // How many lvars this object has
	uint32 m_noScripts; // The number of scripts associated with this object
};

inline cstr c_un_game_object::GetName() const {
	// Get a pointer to the object name
	return ((cstr)(((cstr) this) + m_name_offset));
}

inline uint c_un_game_object::GetNoLvars() const {
	// Get the number of local variables
	return (m_noLvars);
}

inline uint c_un_game_object::GetNoScripts() const {
	// Get the number of scripts
	return (m_noScripts);
}

inline cstr c_un_game_object::GetScriptName(uint32 scriptNo) const {
	_ASSERT((scriptNo >= 0) && (scriptNo < m_noScripts));
	return ((cstr)(((const cstr) this) + ((const int *)(((const char *)this) + m_script_name_table_offset))[scriptNo]));
}

inline cstr c_un_game_object::GetScriptVariableName(uint32 varNo) const {
	cstr currentPos;
	const uint32 *table;

	currentPos = (((cstr) this) + m_var_table_offset);
	table = (const uint32 *)currentPos;
	return currentPos + table[varNo * 2];
}

inline int32 c_un_game_object::IsVariableString(uint32 varNo) const {
	cstr currentPos;
	const uint32 *table;

	currentPos = (((cstr) this) + m_var_table_offset);
	table = (const uint32 *)currentPos;
	return table[varNo * 2 + 1];
}

inline int32 c_un_game_object::GetVariable(cstr name) const {
	cstr currentPos;
	const uint32 *table;
	int32 retValue;
	int32 whichVar;

	currentPos = (((cstr) this) + m_var_table_offset);
	table = (const uint32 *)currentPos;

	retValue = -1;

	for (whichVar = 0; whichVar < (int32)m_noLvars; whichVar++) {
		if (strcmp(name, currentPos + table[whichVar * 2]) == 0) {
			retValue = whichVar;
			whichVar = (int32)m_noLvars;
		}
	}

	return retValue;
}

inline void c_un_game_object::SetIntegerVariable(uint32 lvar, int32 val) {
	_ASSERT((lvar >= 0) && (lvar < m_noLvars));
	(((int *)(((char *)this) + m_lvars_offset))[lvar]) = val;
}

inline int32 &c_un_game_object::GetIntegerVariable(uint32 lvar) {
	// Get an lvar value
	_ASSERT((lvar >= 0) && (lvar < m_noLvars));
	return (((int32 *)(((char *)this) + m_lvars_offset))[lvar]);
}

inline cstr c_un_game_object::GetStringVariable(uint32 lvar) const {
	// Get an lvar value
	_ASSERT((lvar >= 0) && (lvar < m_noLvars));
	return (((cstr) this) + ((const int *)(((const char *)this) + m_lvars_offset))[lvar]);
}

class c_compressed_game_object {
	// Only ob_status is made public. To access the other elements use
	// the access functions. This is so that further changes to the structure
	// can be catered for through the access functions, and not needed where
	// any element is specifically referenced

public:
	// Main access functions
	cstr GetName() const;      // Get a pointer to the object name
	uint GetNoLvars() const;   // Get the number of local variables
	uint GetNoScripts() const; // Get the number of scripts
	uint32 GetSize() { return (m_size); }

	// Using the hash system you cannot get names, only hashes
	uint32 GetScriptNameFullHash(uint32) const;
	uint32 GetScriptNamePartHash(uint32) const;

	cstr GetScriptVariableName(uint32) const; // gets name
	int32 GetVariable(cstr name) const;       // get's number of named variable

	int32 IsVariableString(uint32) const; // is variable a string (1=string, 0=int)

	void SetIntegerVariable(uint32, int32); // Sets the value of an integer variable

	int32 &GetIntegerVariable(uint32); // Get the value of an integer variable
	cstr GetStringVariable(uint32) const;    // Get the value of a string variable

	cstr GetStringValueOrDefault(const cstr varName, const cstr defaultStr) {
		int32 var;
		var = GetVariable(varName);
		if (var == -1)
			return defaultStr;
		else
			return GetStringVariable(var);
	}

	int32 GetIntegerValueOrDefault(const cstr varName, int32 defaultInt) {
		int32 var;
		var = GetVariable(varName);
		if (var == -1)
			return defaultInt;
		else
			return GetIntegerVariable(var);
	}

	friend class c_compressed_game_objectCreator;

protected:       // The object data
	uint32 m_size; // The size of the total data structure

	uint32 m_var_table_offset;

public:
	uint32 ob_status; // low level internal stuff - see enum _object_status

protected: // The rest of the data
	// The offsets to the blocks of data. All offsets
	// are from the start of the object

	uint32 m_script_name_hash_table_offset; // Offset to the script name table

	uint32 m_lvars_offset; // Offset to the local variable data
	uint32 m_name_offset;  // Offset to the object name

	// Variable and script count
	uint32 m_noLvars;   // How many lvars this object has
	uint32 m_noScripts; // The number of scripts associated with this object

	/*      This data is then followed by:

	        Null terminated object name
	        Object variable information block
	        Script names information block
	*/
};

inline cstr c_compressed_game_object::GetName() const {
	// Get a pointer to the object name
	return ((cstr)(((cstr) this) + m_name_offset));
}

inline uint c_compressed_game_object::GetNoLvars() const {
	// Get the number of local variables
	return (m_noLvars);
}

inline uint c_compressed_game_object::GetNoScripts() const {
	// Get the number of scripts
	return (m_noScripts);
}

inline uint32 c_compressed_game_object::GetScriptNameFullHash(uint32 scriptNo) const {
	_ASSERT((scriptNo >= 0) && (scriptNo < m_noScripts));
	return (((const int *)(((const char *)this) + m_script_name_hash_table_offset))[scriptNo * 2]);
}

inline uint32 c_compressed_game_object::GetScriptNamePartHash(uint32 scriptNo) const {
	_ASSERT((scriptNo >= 0) && (scriptNo < m_noScripts));
	return (((const int *)(((const char *)this) + m_script_name_hash_table_offset))[scriptNo * 2 + 1]);
}

inline cstr c_compressed_game_object::GetScriptVariableName(uint32 varNo) const {
	cstr currentPos;
	const uint32 *table;

	currentPos = (((cstr) this) + m_var_table_offset);
	table = (const uint32 *)currentPos;
	return ((const char *)this) + table[varNo * 2];
}

inline int32 c_compressed_game_object::IsVariableString(uint32 varNo) const {
	cstr currentPos;
	const uint32 *table;

	currentPos = (((cstr) this) + m_var_table_offset);
	table = (const uint32 *)currentPos;
	return table[varNo * 2 + 1];
}

inline int32 c_compressed_game_object::GetVariable(cstr name) const {
	cstr currentPos;
	const uint32 *table;
	int32 retValue;
	uint32 whichVar;

	currentPos = (((cstr) this) + m_var_table_offset);
	table = (const uint32 *)currentPos;

	retValue = -1;

	for (whichVar = 0; whichVar < m_noLvars; whichVar++) {
		if (strcmp(name, ((cstr) this) + table[whichVar * 2]) == 0) {
			retValue = whichVar;
			whichVar = (int32)m_noLvars;
		}
	}

	return retValue;
}

inline void c_compressed_game_object::SetIntegerVariable(uint32 lvar, int32 val) {
	_ASSERT((lvar >= 0) && (lvar < m_noLvars));
	(((int *)(((char *)this) + m_lvars_offset))[lvar]) = val;
}

inline int32 &c_compressed_game_object::GetIntegerVariable(uint32 lvar) {
	// Get an lvar value
	_ASSERT((lvar >= 0) && (lvar < m_noLvars));
	return (((int32 *)(((char *)this) + m_lvars_offset))[lvar]);
}

inline cstr c_compressed_game_object::GetStringVariable(uint32 lvar) const {
	// Get an lvar value
	_ASSERT((lvar >= 0) && (lvar < m_noLvars));
	return (((cstr) this) + ((const int *)(((const char *)this) + m_lvars_offset))[lvar]);
}

class CSettableGameObject : public c_un_game_object {
public:
	// Setfunctions
	void SetSize(uint32);                  // Set the object size
	void SetVarTable(uint32);              // set var table offset
	void Set_ob_status(uint32);            // Set ob_status
	void SetScriptNameTableOffset(uint32); // Set the name table offset
	void SetLvarDataOffsetTable(uint32);   // Set the lvars offset

	void SetNoLvars(uint32);    // Set the number of variables
	void SetNoScripts(uint32);  // Set the number of scripts
	void SetNameOffset(uint32); // Set the name offset
};

inline void CSettableGameObject::SetSize(uint32 size) {
	// Set the object size
	m_size = size;
	m_var_table_offset = 0xdeadbeef;
}

inline void CSettableGameObject::SetVarTable(uint32 o) {
	// Set the object size
	m_var_table_offset = o;
}

inline void CSettableGameObject::Set_ob_status(uint32 status) {
	// Set ob_status
	ob_status = status;
}

inline void CSettableGameObject::SetNoLvars(uint32 n) {
	// Get the number of local variables
	m_noLvars = n;
}

inline void CSettableGameObject::SetNoScripts(uint32 n) {
	// Get the number of scripts
	m_noScripts = n;
}

inline void CSettableGameObject::SetScriptNameTableOffset(uint32 o) {
	// Set the name table offset
	m_script_name_table_offset = o;
}

inline void CSettableGameObject::SetLvarDataOffsetTable(uint32 o) {
	// Set the lvars offset
	m_lvars_offset = o;
}

inline void CSettableGameObject::SetNameOffset(uint32 o) {
	// Set the name offset
	m_name_offset = o;
}

} // End of namespace ICB

#endif
